//Multiple masters
//dataset stored in server
#include "domain.h"
#include "coarsePartition.h"
#include "delaunayMPI.h"

#define SUB_MASTER_RANK 0

//One node contains N number of triangles, each triangle contains three Ids
struct nodeTriangleIdArr{
	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
};

//=============================================================================
//read activeCoarsePartNum, activeCoarsePartIdArr[i], activeCoarsePartInitSizeArr[i], xCorsePartNum, yCorsePartNum
//maxGroupNum is maximum number of groups, each group has one sub-master
//maxGroupNum may be greater than number of active coarse partitions
//for ex: 17 processes --> 5 groups: 0, 1, 2, 3, 4. However, We have only 4 active coarse partitions.
void readTempTrianglesCoarsePartsInfo(unsigned int &activeCoarsePartNum, unsigned int *&activeCoarsePartIdArr, unsigned int *&activeCoarsePartInitSizeArr, unsigned int groupNum, unsigned int &xCoarsePartNum, unsigned int &yCoarsePartNum, std::string path){

	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "delaunayResults/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	activeCoarsePartNum = atoi(strItem.c_str());

	activeCoarsePartIdArr = new unsigned int[groupNum];
	//second line: read active partition ids (coarsePartition Ids)
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}

	activeCoarsePartInitSizeArr = new unsigned int[groupNum];
	//third line stores number of init triangles belong to active partitions (coarsePartitions)
	//take number of init triangles of current coarse partition in the array of active coarse partitions
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartInitSizeArr[i] = atoi(strItem.c_str());
	}

	//fourth line: read coarse-grained partition sizes 
	initTriangleInfoFile >> strItem;
	xCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile >> strItem;
	yCoarsePartNum = atoi(strItem.c_str());

	initTriangleInfoFile.close();
}

//=============================================================================
//transform list of triangleIdArr (triangleIdArrList) into flat array (triangleIdArr)
void transformTriangleIdArrList(std::list<nodeTriangleIdArr> triangleIdArrList, unsigned long long *&triangleIdArr, unsigned int &triangleNum){

	if(triangleIdArrList.size()==0) return;
	unsigned int totalTriangleSize = 0;
	//flattening multiple triangleIdArr in triangleIdArrList into one triangleIdArr (triangleIdArr)
	for(std::list<nodeTriangleIdArr>::iterator it=triangleIdArrList.begin(); it!=triangleIdArrList.end(); it++){
		totalTriangleSize += (*it).triangleNum;
	}

	//triangleNum is total number of triangles
	try {
		triangleIdArr = new unsigned long long[totalTriangleSize*3];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory allocated for triangleIdArr overflow!!!!!!!!!!!\n";
		exit(1);
	}
	triangleNum = totalTriangleSize;

	unsigned long index = 0;
	//populating triangleIdArr from each item of triangleIdArrList
	for(std::list<nodeTriangleIdArr>::iterator it=triangleIdArrList.begin(); it!=triangleIdArrList.end(); it++){
		unsigned long long *subTriangleArr = (*it).triangleIdArr;
		unsigned int subTriangleArrSize = (*it).triangleNum;
		for(unsigned int i=0; i<subTriangleArrSize; i++){
			triangleIdArr[index*3] = subTriangleArr[i*3];
			triangleIdArr[index*3+1] = subTriangleArr[i*3+1];
			triangleIdArr[index*3+2] = subTriangleArr[i*3+2];
//std::cout<<triangleIdArr[index*3]<<" ";
			index++;
		}
		//remove subTriangleArr in triangleIdArrList
		delete [] subTriangleArr;
	}
	triangleIdArrList.clear();
}

//============================================================================
void storeTriangleIds(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, std::string currPath){
	FILE *f = fopen(currPath.c_str(), "a");
	if(!f){
		std::cout<<"not success to open "<<currPath<<std::endl;
		exit(1);
	}
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
}

//=============================================================================
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectStoreTriangleIdArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned long long *triangleIdArr, unsigned int triangleNum, std::string path){
	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;
	unsigned int storeTriangleNumOffset;

	if((world_rank==MASTER_RANK)){//only master node
		storeTriangleNumArr = new unsigned int[groupNum];
//		storeTriangleNumArr = new unsigned int[world_size];
//std::cout<<"AAAAAAAAAAAAAAAAA: "<<activeCoarsePartNum<<" "<<groupNum<<"\n";
		storeTriangleNumArr[0] = triangleNum;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to superMaster (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
std::cout<<"BBBBBBBBBB world rank: "<<world_rank<<"\n";
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED, MASTER_RANK, send_data_tag, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleNumArr[i], 1, MPI_UNSIGNED, i*row_size, send_data_tag, MPI_COMM_WORLD, &status);
	}

//	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	//process offset of each triangleIdArr of each subMaster
	if((world_rank==MASTER_RANK)){//only master node
		storeTriangleNumOffsetArr = new unsigned int[groupNum];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<activeCoarsePartNum; i++){
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
//std::cout<<storeTriangleNumOffsetArr[i]<<" ";
		}
	}

	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		unsigned int storeTriangleIdArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int i=1; i<activeCoarsePartNum; i++)
			storeTriangleIdArrSize += storeTriangleNumArr[i];
		//add the number of stored triangles from master
		storeTriangleIdArrSize += triangleNum;

		//allocate memory for storeTriangleIdArr
		try {
			storeTriangleIdArr = new unsigned long long[storeTriangleIdArrSize*3];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		for(unsigned int index=0; index<triangleNum; index++){
			storeTriangleIdArr[index*3] = triangleIdArr[index*3];
			storeTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			storeTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];
		}

		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[i]*3], storeTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, i*row_size, send_data_tag, MPI_COMM_WORLD, &status);

		std::cout<<"Writing store triangles to file triangleIds.tri\n";
		std::string currPath = path + "delaunayResults/triangleIds.tri";
		storeTriangleIds(storeTriangleIdArr, storeTriangleIdArrSize, currPath);
	}

	if(row_rank==SUB_MASTER_RANK) delete [] triangleIdArr;

	if(world_rank==MASTER_RANK){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
		delete [] storeTriangleIdArr;
	}
//if(groupNum >= activeCoarsePartNum) std::cout<<"AAAAAAAAAAAAAAAAAAAAAAAAAAA\n";
}

//=============================================================================
//Each active partition in domain will be Delaunay processed as a master in a group
//color is the group Id
//row_rank is the rank in each group
//row_size is the number of processes in each group
//row_comm is communication for that group
//=============================================================================
void delaunayPartition(unsigned int world_rank, unsigned int world_size, unsigned int groupNum, unsigned int color, unsigned int row_rank, unsigned int row_size, MPI_Comm row_comm, std::string path){
	std::list<nodeTriangleIdArr> triangleIdArrList;
	unsigned long long *triangleIdArr = NULL;
	unsigned int triangleNum = 0;
	nodeTriangleIdArr triangleIdArrNode;
	triangleIdArrNode.triangleIdArr = NULL;
	triangleIdArrNode.triangleNum = 0;


	unsigned int groupId = color;//process Id, multiple processes are divided into many groups
	unsigned int coarsePartId;// partition Id of an active partition
	unsigned int xCoarsePartNum;
	unsigned int yCoarsePartNum;
	unsigned int activeCoarsePartNum;//number of active partitions (coarse)
	unsigned int initTriangleNum;//number of init triangles of an active partition

	unsigned int *activeCoarsePartIdArr;//partition Id for each coarse active partition
	unsigned int *activeCoarsePartInitSizeArr;//number of init triangles for each coarse active partition 
	MPI_Status status;

	delaunayMPI *pMPI = new delaunayMPI(path);
	coarsePartition *c;
//if(row_rank==SUB_MASTER_RANK) std::cout<<"row_size: " + toString(row_size) + ", row_rank: " + toString(row_rank) + ", groupId: " + toString(groupId)<<"\n";

	//master rank reads info from tempTrianglesCoarseParts.xfdl
	if(world_rank == MASTER_RANK){
		readTempTrianglesCoarsePartsInfo(activeCoarsePartNum, activeCoarsePartIdArr, activeCoarsePartInitSizeArr, groupNum, xCoarsePartNum, yCoarsePartNum, path);
		//current partId & number of init triangles of master coarse partition
		coarsePartId = activeCoarsePartIdArr[groupId];
		initTriangleNum = activeCoarsePartInitSizeArr[groupId];
	}

	MPI_Barrier(MPI_COMM_WORLD);

	//then send to all other sub master (row_rank = 0)
	//copy activeCoarsePartNum, xCoarsePartNum, yCoarsePartNum from world_rank to ther ranks
	MPI_Bcast(&activeCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&xCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&yCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//copy second. third values ... in array activeCoarsePartIdArr, activeCoarsePartInitSizeArr to coarsePartId, initTriangleNum of each rank with row_rank==0
	if(world_rank == MASTER_RANK)
//		for(int i=1; i<activeCoarsePartNum; i++){
		for(int i=1; i<groupNum; i++){
			MPI_Send(&activeCoarsePartIdArr[i], 1, MPI_INT, i*row_size, send_data_tag, MPI_COMM_WORLD);
			MPI_Send(&activeCoarsePartInitSizeArr[i], 1, MPI_INT, i*row_size, send_data_tag, MPI_COMM_WORLD);
		}
	else if(row_rank==0){
		MPI_Recv(&coarsePartId, 1, MPI_INT, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
		MPI_Recv(&initTriangleNum, 1, MPI_INT, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
	}

	if(world_rank == MASTER_RANK){
		delete [] activeCoarsePartIdArr;
		delete [] activeCoarsePartInitSizeArr;
	}

//if(row_rank == SUB_MASTER_RANK) std::cout<<"world_rank: " + toString(world_rank) + ", coarsePartId: " + toString(coarsePartId) + ", initTriangleNum: " + toString(initTriangleNum) + ", groupId: " + toString(groupId) + ", activeCoarsePartNum: " + toString(activeCoarsePartNum) + ", xCoarsePartNum: " + toString(xCoarsePartNum) + ", yCoarsePartNum: " + toString(yCoarsePartNum)<<"\n";

	MPI_Barrier(MPI_COMM_WORLD);

	//if number of processes is greater than needed, skip it
//	if(groupId >= activeCoarsePartNum) return;
if(groupId < activeCoarsePartNum){

	if(row_rank == SUB_MASTER_RANK){
		c = new coarsePartition(groupId, coarsePartId, activeCoarsePartNum, initTriangleNum, xCoarsePartNum, yCoarsePartNum, path);
		c->loadInitPoints();
		c->loadInitTriangles();
		c->initTriangulate();
		c->triangleTransform();
	}

	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage
	//Number of current active partitions
	unsigned int activePartNum;

	while(!delaunayStop){
		if(row_rank==SUB_MASTER_RANK){
			c->generateIntersection();
			c->generateConflictPartitions();
			activePartNum = c->generateActivePartitions();
			c->updateConflictPartitions();
			c->deliverTriangles(triangleIdArrNode.triangleIdArr, triangleIdArrNode.triangleNum);
			if(triangleIdArrNode.triangleIdArr!=NULL){
				triangleIdArrList.push_back(triangleIdArrNode);
				triangleIdArrNode.triangleIdArr = NULL;
				triangleIdArrNode.triangleNum = 0;	
			}

			if(c->unfinishedPartNum()<=0) delaunayStop = true;
//delaunayStop = true;
			activePartStop = false;
		}
		//sychronize all processes in a group
		MPI_Barrier(row_comm);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);

		int currActivePartNum;
		while(!activePartStop){
			if(row_rank==SUB_MASTER_RANK){
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				c->prepareDataForDelaunayMPI(row_size);

				//currActivePartNum is the number of active partition leftover for the next loop.
				//it means that if currActivePartNum==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				currActivePartNum = c->activePartitionNumber();
				if(currActivePartNum<=0) activePartStop = true;
//activePartStop = true;
//				std::cout<<"Number of active partitions leftover: "<<currActivePartNum<<"\n";
			}
			//sychronize all processes
			MPI_Barrier(row_comm);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);

			//process MPI
			pMPI->processMPI(row_rank, row_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum, triangleIdArrNode.triangleIdArr, triangleIdArrNode.triangleNum);

			//sychronize all processes
			MPI_Barrier(row_comm);

			if(row_rank==SUB_MASTER_RANK){
				if(triangleIdArrNode.triangleIdArr!=NULL){
					triangleIdArrList.push_back(triangleIdArrNode);
					triangleIdArrNode.triangleIdArr = NULL;
					triangleIdArrNode.triangleNum = 0;	
				}

				c->addReturnTriangles();
			}

			//sychronize all processes
			MPI_Barrier(row_comm);
		}
		if(row_rank==SUB_MASTER_RANK){
			c->updateTriangleArr();
		}
		//sychronize all processes
		MPI_Barrier(row_comm);
	}
	delete pMPI;

	if(row_rank==SUB_MASTER_RANK){
		c->storeAllTriangles();
	}
}
	//transform list of triangleIdArr (triangleIdArrList) into flat array (triangleIdArr)
	//triangleIdArrList will be ersased after inside function
	transformTriangleIdArrList(triangleIdArrList, triangleIdArr, triangleNum);

//for(int i=0; i<triangleNum; i++)
//std::cout<<triangleIdArr[i*3]<<" "<<triangleIdArr[i*3+1]<<" "<<triangleIdArr[i*3+2]<<"  ";
	//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
	collectStoreTriangleIdArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, triangleIdArr, triangleNum, path);
}

//=============================================================================
int main (int argc, char** argv){
//=============================================================================

	if(argc<=2){// no arguments
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first argument is the path to the dataset\n";
		std::cout<<"The second argument is the domain size\n";
		return 0;
	}
    int world_rank, world_size;//world_rank --> world_rank
	double domainSize;
	std::string path;

	path = argv[1];
	domainSize = atof(argv[2]);

	domain *d;

	double masterTime = 0;
	double updateTime = 0;
	double storeTime = 0;
	double amountTime = 0;
	double currentTime;
	double overAllTime;

	//ACTION - start do to parallel
	//Initialize MPI.
    MPI_Init(&argc, &argv);

	overAllTime = MPI_Wtime();

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	//world_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if(world_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d = new domain(0.0,0.0, domainSize, domainSize, path);
		d->loadInitPoints();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		std::cout<<"Time for loadInitPoints() is "<<amountTime<<std::endl;

		currentTime = GetWallClockTime();
		d->initTriangulate();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		std::cout<<"Time for initTriangulate() is "<<amountTime<<std::endl;

		currentTime = GetWallClockTime();
		d->triangleTransform();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		std::cout<<"Time for triangleTransform() is "<<amountTime<<std::endl;
	}


	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage

	currentTime = GetWallClockTime();

	int stage = 1;
	//Number of current active partitions
	unsigned int activePartNum;
	while(!delaunayStop){
		if(world_rank==MASTER_RANK){
			double amountTime1, amountTime2;
			currentTime = GetWallClockTime();
			d->generateIntersection();
			d->generateConflictPartitions();
			//Number of real active partitions
			activePartNum = d->generateActivePartitions();
			d->updateConflictPartitions();

			d->deliverTriangles(amountTime1, amountTime2);
			masterTime += amountTime1;
			storeTime += amountTime2;

			if(d->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
			std::cout<<"----------------stage: "<<stage<<", activePartNum: "<<activePartNum<<std::endl;

		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

		int currActivePartNum, activePartNumLeftOver;
		//groupSize: number of processes in a group, groupNum: number of groups
		int groupSize, groupNum;
		while(!activePartStop){
			if(world_rank==MASTER_RANK){
				double amountTime1, amountTime2;
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				//number of sub masters
				currActivePartNum = d->prepareDataForDelaunayMPI(world_size, amountTime1, amountTime2);
				masterTime += amountTime1;
				updateTime += amountTime2;

				//number of processes in a group
				groupSize = world_size/currActivePartNum;
				//groupNum = (world_size%currActivePartNum==0)?(currActivePartNum):(currActivePartNum+1);
				//groupNum = (world_size%currActivePartNum==0)?(currActivePartNum):(world_size/groupSize+(world_size%groupSize==0)?0:1);
				//groupNum = world_size/groupSize + (world_size % groupSize==0) ? (0) : (1);
				if(world_size%currActivePartNum==0)
					groupNum = currActivePartNum;
				else{
					if(world_size%groupSize==0)
						groupNum = world_size/groupSize;
					else groupNum = world_size/groupSize + 1;
				}

std::cout<<"groupSize: " + toString(groupSize) + ", groupNum: " + toString(groupNum)<<"\n";

				//activePartNumLeftOver is then umber of active partition leftover for the next loop.
				//it means that if activePartNumLeftOver==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				activePartNumLeftOver = d->activePartitionNumber();
				if(activePartNumLeftOver<=0) activePartStop = true;
				std::cout<<"Number of current active partitions: "<<currActivePartNum<<"\n";
				std::cout<<"Number of active partitions leftover: "<<activePartNumLeftOver<<"\n";
			}
			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
			MPI_Bcast(&groupSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
			MPI_Bcast(&groupNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

			//Split the world communicator based on the currActivePartNum into multiple communicatiors
			MPI_Comm row_comm;
			int row_rank, row_size;
			int color = world_rank / groupSize;

			MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);

			MPI_Comm_rank(row_comm, &row_rank);
			MPI_Comm_size(row_comm, &row_size);

//std::cout<<"world_rank: " + toString(world_rank) + ", row_rank: " + toString(row_rank) + ", color: " + toString(color) + ", rowSize: " + toString(row_size)<<"\n";

			//Processes that are out of scope --> do nothing
			//for ex: 17 processes --> 5 groups: 0, 1, 2, 3, 4 (group 4 work but will stop very soon, with world_rank=16)
			//process multiple masters, each master process a coarsePartition
			delaunayPartition(world_rank, world_size, groupNum, color, row_rank, row_size, row_comm, path);

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Comm_free(&row_comm);
		}

		if(world_rank==MASTER_RANK){
			currentTime = GetWallClockTime();
			d->addReturnStoreTriangles();

			amountTime = GetWallClockTime()-currentTime;
			storeTime += amountTime;
		}

		if(world_rank==MASTER_RANK){
			currentTime = GetWallClockTime();
			d->updateTriangleArr();
			amountTime = GetWallClockTime()-currentTime;
			updateTime += amountTime;

			stage++;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);
	}

	if(world_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d->storeAllTriangles();//store triangles leftover in domain

		amountTime = GetWallClockTime()-currentTime;
		storeTime += amountTime;

		std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		overAllTime = MPI_Wtime()-overAllTime;
		std::cout<<"done!!!"<<std::endl;

		std::cout<<"Multiple-Master version\n";
		std::cout<<"datasources: "<<path<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<" coarse partitions"<<std::endl;
		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"Update time: "<<updateTime<<"\n";
		std::cout<<"Master + Update time: "<<masterTime + updateTime<<"\n";
		std::cout<<"Store time: "<<storeTime<<"\n";
		std::cout<<"MPI time: "<<overAllTime - (masterTime + updateTime + storeTime)<<"\n";
		std::cout<<"Total time: "<<overAllTime<<"\n";

		unsigned int xFinePartNum, yFinePartNum;
		d->readFinePartitionSize(xFinePartNum, yFinePartNum);

		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
		resultFile<<"\n\nMultiple-Master version\n";
		resultFile<<"datasources: "<<path<<"\n";
		resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<"\n";
		resultFile<<"Master time: "<<masterTime<<"\n";
		resultFile<<"Update time: "<<updateTime<<"\n";
		resultFile<<"Master + Update time: "<<masterTime+updateTime<<"\n";
		resultFile<<"Store time: "<<storeTime<<"\n";
		resultFile<<"MPI time: "<<overAllTime - (masterTime + updateTime + storeTime)<<"\n";
		resultFile<<"Total time: "<<overAllTime<<"\n";

		resultFile.close();
	}
	if(world_rank==MASTER_RANK) delete d;
    MPI_Finalize();
}