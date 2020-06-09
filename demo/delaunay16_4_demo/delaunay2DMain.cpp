//Multiple masters
//dataset stored in server
#include "domain.h"
#include "coarsePartition.h"
#include "delaunayMPI.h"

#define SUB_MASTER_RANK 0
#define return_data_tag1 2003
#define return_data_tag2 2004

//One node contains N number of triangles, each triangle contains three Ids
struct nodeTriangleIdArr{
	unsigned long long *triangleIdArr;
	unsigned long long triangleNum;
};

struct nodeTriangleCoorArr{
	double *triangleCoorArr;
	unsigned long long triangleNum;
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
void transformTriangleIdArrList(std::list<nodeTriangleIdArr> &triangleIdArrList, unsigned long long *&triangleIdArr, unsigned long long &triangleNum){

	if(triangleIdArrList.size()==0) return;
	unsigned long long totalTriangleSize = 0;
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

	unsigned long long index = 0;
	//populating triangleIdArr from each item of triangleIdArrList
	for(std::list<nodeTriangleIdArr>::iterator it=triangleIdArrList.begin(); it!=triangleIdArrList.end(); it++){
		unsigned long long *subTriangleArr = (*it).triangleIdArr;
		unsigned long long subTriangleArrSize = (*it).triangleNum;
		for(unsigned long long i=0; i<subTriangleArrSize; i++){
			triangleIdArr[index*3] = subTriangleArr[i*3];
			triangleIdArr[index*3+1] = subTriangleArr[i*3+1];
			triangleIdArr[index*3+2] = subTriangleArr[i*3+2];
//std::cout<<triangleIdArr[index*3]<<" ";
			index++;
		}
		//remove subTriangleArr in triangleIdArrList
		delete [] (*it).triangleIdArr;
	}
	triangleIdArrList.clear();
}

//=============================================================================
//transform list of triangleCoorArr (triangleCoorArrList) into flat array (triangleCoorArr)
void transformTriangleCoorArrList(std::list<nodeTriangleCoorArr> &triangleCoorArrList, double *&triangleCoorArr, unsigned long long &triangleNum){

	if(triangleCoorArrList.size()==0) return;
	unsigned long long totalTriangleSize = 0;
	//flattening multiple triangleCoorArr in triangleCoorArrList into one triangleCoorArr (triangleCoorArr)
	for(std::list<nodeTriangleCoorArr>::iterator it=triangleCoorArrList.begin(); it!=triangleCoorArrList.end(); it++){
		totalTriangleSize += (*it).triangleNum;
	}

	//triangleNum is total number of triangles
	try {
		triangleCoorArr = new double[totalTriangleSize*6];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory allocated for triangleCoorArr overflow!!!!!!!!!!!\n";
		exit(1);
	}
	triangleNum = totalTriangleSize;

	unsigned long long index = 0;
	//populating triangleCoorArr from each item of triangleCoorArrList
	for(std::list<nodeTriangleCoorArr>::iterator it=triangleCoorArrList.begin(); it!=triangleCoorArrList.end(); it++){
		double *subTriangleArr = (*it).triangleCoorArr;
		unsigned long long subTriangleArrSize = (*it).triangleNum;
		for(unsigned long long i=0; i<subTriangleArrSize; i++){
			triangleCoorArr[index*6] = subTriangleArr[i*6];
			triangleCoorArr[index*6+1] = subTriangleArr[i*6+1];
			triangleCoorArr[index*6+2] = subTriangleArr[i*6+2];
			triangleCoorArr[index*6+3] = subTriangleArr[i*6+3];
			triangleCoorArr[index*6+4] = subTriangleArr[i*6+4];
			triangleCoorArr[index*6+5] = subTriangleArr[i*6+5];
//std::cout<<triangleCoorArr[index*3]<<" ";
			index++;
		}
		//remove subTriangleArr in triangleCoorArrList
		delete [] (*it).triangleCoorArr;
	}
	triangleCoorArrList.clear();
}


//============================================================================
void storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string currPath){
	if((triangleCoorArrSize==0)||(triangleCoorArr==NULL)) return;
	FILE *f = fopen(currPath.c_str(), "a");
	if(!f){
		std::cout<<"not success to open "<<currPath<<std::endl;
		exit(1);
	}
	fwrite(triangleCoorArr, triangleCoorArrSize*6, sizeof(double), f);
	fclose(f);
}

//=============================================================================
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectStoreTriangleIdArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned long long *triangleIdArr, unsigned long long triangleNum, std::string path, double &masterTime, double &storeTime){
	double currentTime;
	masterTime = 0;
	storeTime = 0;
	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned long long *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned long long *storeTriangleNumOffsetArr = NULL;
//	unsigned int storeTriangleNumOffset;

	if((world_rank==MASTER_RANK)){//only master node
		currentTime = GetWallClockTime();
		storeTriangleNumArr = new unsigned long long[groupNum];
//		storeTriangleNumArr = new unsigned int[world_size];
//std::cout<<"AAAAAAAAAAAAAAAAA: "<<activeCoarsePartNum<<" "<<groupNum<<"\n";
		storeTriangleNumArr[0] = triangleNum;
		masterTime += GetWallClockTime() - currentTime;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to superMaster (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
//std::cout<<"BBBBBBBBBB world rank: "<<world_rank<<"\n";
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag1, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleNumArr[i], 1, MPI_UNSIGNED_LONG_LONG, i*row_size, return_data_tag1, MPI_COMM_WORLD, &status);
		masterTime += GetWallClockTime() - currentTime;
	}

//	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	//process offset of each triangleIdArr of each subMaster
	if((world_rank==MASTER_RANK)){//only master node
		currentTime = GetWallClockTime();
		storeTriangleNumOffsetArr = new unsigned long long[groupNum];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<activeCoarsePartNum; i++){
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
		masterTime += GetWallClockTime() - currentTime;
		}
	}

	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag2, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		currentTime = GetWallClockTime();
		unsigned long long storeTriangleIdArrSize = 0;
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
			std::cout<<"Memory overflow!!!!!!!!!!!!\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			storeTriangleIdArr[index*3] = triangleIdArr[index*3];
			storeTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			storeTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];
		}

		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[i]*3], storeTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, i*row_size, return_data_tag2, MPI_COMM_WORLD, &status);

		masterTime += GetWallClockTime() - currentTime;

		currentTime = GetWallClockTime();
		std::cout<<">>>>>Writing store triangles to file triangleIds.tri<<<<<<\n";
		std::string currPath = path + "delaunayResults/triangleIds.tri";
		storeTriangleIds(storeTriangleIdArr, storeTriangleIdArrSize, currPath, true);

		delete [] storeTriangleIdArr;
		storeTriangleIdArr = NULL;
		storeTime += GetWallClockTime() - currentTime;
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
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectBoundaryTriangleArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, unsigned coarsePartId, std::string path, double &masterTime, double &storeTime){
	double currentTime;
	masterTime = 0;
	storeTime = 0;
	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned long long *boundaryTriangleNumArr = NULL;
	unsigned long long *boundaryTriangleIdArr = NULL;
	double *boundaryTriangleCoorArr = NULL;
	//based on boundaryTriangleNumArr, compute boundaryTriangleNumOffsetArr
	unsigned long long *boundaryTriangleNumOffsetArr = NULL;

	if((world_rank==MASTER_RANK)){//only master node
		currentTime = GetWallClockTime();
		boundaryTriangleNumArr = new unsigned long long[groupNum];
		boundaryTriangleNumArr[0] = triangleNum;
		masterTime += GetWallClockTime() - currentTime;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to superMaster (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag1, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&boundaryTriangleNumArr[i], 1, MPI_UNSIGNED_LONG_LONG, i*row_size, return_data_tag1, MPI_COMM_WORLD, &status);
		masterTime += GetWallClockTime() - currentTime;
	}

	//process offset of each triangleIdArr of each subMaster
	if((world_rank==MASTER_RANK)){//only master node
		currentTime = GetWallClockTime();
		boundaryTriangleNumOffsetArr = new unsigned long long[groupNum];
		boundaryTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<activeCoarsePartNum; i++){
			boundaryTriangleNumOffsetArr[i] = boundaryTriangleNumOffsetArr[i-1] + boundaryTriangleNumArr[i-1];
		masterTime += GetWallClockTime() - currentTime;
		}
	}


	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag2, MPI_COMM_WORLD);
		ierr = MPI_Send(triangleCoorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, return_data_tag2, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		currentTime = GetWallClockTime();
		unsigned long long boundaryTriangleArrSize = 0;
		//calculate the size of boundaryTriangleIdArr that collect from other rank
		for(int i=1; i<activeCoarsePartNum; i++)
			boundaryTriangleArrSize += boundaryTriangleNumArr[i];
		//add the number of stored triangles from master
		boundaryTriangleArrSize += triangleNum;

		//allocate memory for boundaryTriangleIdArr
		try {
			boundaryTriangleIdArr = new unsigned long long[boundaryTriangleArrSize*3];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!!\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to boundaryTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			boundaryTriangleIdArr[index*3] = triangleIdArr[index*3];
			boundaryTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			boundaryTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];
		}

		//allocate memory for boundaryTriangleCoorArr
		try {
			boundaryTriangleCoorArr = new double[boundaryTriangleArrSize*6];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!!\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			boundaryTriangleCoorArr[index*6] = triangleCoorArr[index*6];
			boundaryTriangleCoorArr[index*6+1] = triangleCoorArr[index*6+1];
			boundaryTriangleCoorArr[index*6+2] = triangleCoorArr[index*6+2];
			boundaryTriangleCoorArr[index*6+3] = triangleCoorArr[index*6+3];
			boundaryTriangleCoorArr[index*6+4] = triangleCoorArr[index*6+4];
			boundaryTriangleCoorArr[index*6+5] = triangleCoorArr[index*6+5];
		}

		for(int i=1; i<groupNum; i++){
			ierr = MPI_Recv(&boundaryTriangleIdArr[boundaryTriangleNumOffsetArr[i]*3], boundaryTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, i*row_size, return_data_tag2, MPI_COMM_WORLD, &status);
			ierr = MPI_Recv(&boundaryTriangleCoorArr[boundaryTriangleNumOffsetArr[i]*6], boundaryTriangleNumArr[i]*6, MPI_DOUBLE, i*row_size, return_data_tag2, MPI_COMM_WORLD, &status);
		}

		masterTime += GetWallClockTime() - currentTime;

		currentTime = GetWallClockTime();
		std::cout<<">>>>>Writing boundary triangles to file boundaryIds.tri and boundaryCoors.tri <<<<<<\n";

		std::string currPath1 = path + "delaunayResults/boundaryIds.tri";
		std::string currPath2 = path + "delaunayResults/boundaryCoors.tri";

		storeTriangleIds(boundaryTriangleIdArr, boundaryTriangleArrSize, currPath1, true);
		storeTriangleCoors(boundaryTriangleCoorArr, boundaryTriangleArrSize, currPath2);

		delete [] boundaryTriangleIdArr;
		delete [] boundaryTriangleCoorArr;
		boundaryTriangleIdArr = NULL;
		boundaryTriangleCoorArr = NULL;
		storeTime += GetWallClockTime() - currentTime;		
	}

	if(row_rank==SUB_MASTER_RANK){
		delete [] triangleIdArr;
		delete [] triangleCoorArr;
	}

	if(world_rank==MASTER_RANK){ //only master node
		delete [] boundaryTriangleNumArr;
		delete [] boundaryTriangleNumOffsetArr;
		delete [] boundaryTriangleIdArr;
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
void delaunayPartition(unsigned int world_rank, unsigned int world_size, unsigned int groupNum, unsigned int color, unsigned int row_rank, unsigned int row_size, MPI_Comm row_comm, std::string path, double &masterTime, double &storeTime){
	double currentTime, masterAmountTime, storeAmountTime;
	masterTime = 0;
	storeTime = 0;

	std::list<nodeTriangleIdArr> storeTriangleIdArrList;
	unsigned long long *storeTriangleIdArr = NULL;
	unsigned long long storeTriangleNum = 0;
	nodeTriangleIdArr storeTriangleIdArrNode;
	storeTriangleIdArrNode.triangleIdArr = NULL;
	storeTriangleIdArrNode.triangleNum = 0;

	std::list<nodeTriangleIdArr> returnTriangleIdArrList;
	unsigned long long *returnTriangleIdArr = NULL;
	unsigned long long returnTriangleNum = 0;
	nodeTriangleIdArr returnTriangleIdArrNode;
	returnTriangleIdArrNode.triangleIdArr = NULL;
	returnTriangleIdArrNode.triangleNum = 0;

	std::list<nodeTriangleCoorArr> returnTriangleCoorArrList;
	double *returnTriangleCoorArr = NULL;
	nodeTriangleCoorArr returnTriangleCoorArrNode;
	returnTriangleCoorArrNode.triangleCoorArr = NULL;
	returnTriangleCoorArrNode.triangleNum = 0;

	std::list<nodeTriangleIdArr> boundaryTriangleIdArrList;
	unsigned long long *boundaryTriangleIdArr = NULL;
	unsigned long long boundaryTriangleNum = 0;
	nodeTriangleIdArr boundaryTriangleIdArrNode;
	boundaryTriangleIdArrNode.triangleIdArr = NULL;
	boundaryTriangleIdArrNode.triangleNum = 0;

	std::list<nodeTriangleCoorArr> boundaryTriangleCoorArrList;
	double *boundaryTriangleCoorArr = NULL;
	nodeTriangleCoorArr boundaryTriangleCoorArrNode;
	boundaryTriangleCoorArrNode.triangleCoorArr = NULL;
	boundaryTriangleCoorArrNode.triangleNum = 0;



	unsigned int groupId = color;//process Id, multiple processes are divided into many groups
	unsigned int coarsePartId;// partition Id of an active partition
	unsigned int xCoarsePartNum;
	unsigned int yCoarsePartNum;
	unsigned int activeCoarsePartNum;//number of active partitions (coarse)
	unsigned int initTriangleNum;//number of init triangles of an active partition

	unsigned int *activeCoarsePartIdArr;//partition Id for each coarse active partition
	unsigned int *activeCoarsePartInitSizeArr;//number of init triangles for each coarse active partition 
	MPI_Status status;

//if(row_rank==SUB_MASTER_RANK) std::cout<<"row_size: " + toString(row_size) + ", row_rank: " + toString(row_rank) + ", groupId: " + toString(groupId)<<"\n";

	//master rank reads info from tempTrianglesCoarseParts.xfdl
	if(world_rank == MASTER_RANK){
		currentTime = GetWallClockTime();
		readTempTrianglesCoarsePartsInfo(activeCoarsePartNum, activeCoarsePartIdArr, activeCoarsePartInitSizeArr, groupNum, xCoarsePartNum, yCoarsePartNum, path);
		//current partId & number of init triangles of master coarse partition
		coarsePartId = activeCoarsePartIdArr[groupId];
		initTriangleNum = activeCoarsePartInitSizeArr[groupId];
		masterTime += GetWallClockTime() - currentTime;
	}

	MPI_Barrier(MPI_COMM_WORLD);

	//then send to all other sub master (row_rank = 0)
	//copy activeCoarsePartNum, xCoarsePartNum, yCoarsePartNum from world_rank to ther ranks
	MPI_Bcast(&activeCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&xCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&yCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//copy second. third values ... in array activeCoarsePartIdArr, activeCoarsePartInitSizeArr to coarsePartId, initTriangleNum of each rank with row_rank==0
	if(world_rank == MASTER_RANK){
		currentTime = GetWallClockTime();
		for(int i=1; i<groupNum; i++){
			MPI_Send(&activeCoarsePartIdArr[i], 1, MPI_INT, i*row_size, send_data_tag, MPI_COMM_WORLD);
			MPI_Send(&activeCoarsePartInitSizeArr[i], 1, MPI_INT, i*row_size, send_data_tag, MPI_COMM_WORLD);
		}
		masterTime += GetWallClockTime() - currentTime;
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

	delaunayMPI *pMPI = new delaunayMPI(row_rank, row_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum, path);
	coarsePartition *c;

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
			c->deliverTriangles(storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, boundaryTriangleIdArrNode.triangleIdArr, boundaryTriangleCoorArrNode.triangleCoorArr, boundaryTriangleIdArrNode.triangleNum);
			if(storeTriangleIdArrNode.triangleIdArr!=NULL){
				storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
				storeTriangleIdArrNode.triangleIdArr = NULL;
				storeTriangleIdArrNode.triangleNum = 0;
			}
			boundaryTriangleCoorArrNode.triangleNum = boundaryTriangleIdArrNode.triangleNum;
			if(boundaryTriangleIdArrNode.triangleIdArr!=NULL){
				boundaryTriangleIdArrList.push_back(boundaryTriangleIdArrNode);
				boundaryTriangleIdArrNode.triangleIdArr = NULL;
				boundaryTriangleIdArrNode.triangleNum = 0;
			}
			if(boundaryTriangleCoorArrNode.triangleCoorArr!=NULL){
				boundaryTriangleCoorArrList.push_back(boundaryTriangleCoorArrNode);
				boundaryTriangleCoorArrNode.triangleCoorArr = NULL;
				boundaryTriangleCoorArrNode.triangleNum = 0;
			}

			if(c->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
		}
//delaunayStop=true;
		//sychronize all processes in a group
		MPI_Barrier(row_comm);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);


		unsigned int currActivePartNumLeftOver=0;
		unsigned long long *tempPointIdArr = NULL;
		double *tempCoorArr = NULL;
		unsigned int *activePartIdArr = NULL;
		unsigned int *activePartSizeArr = NULL;
		unsigned int *activePartSizeOffsetArr = NULL;
		unsigned int *pointNumPartArr = NULL;
		unsigned int currActivePartNum;
		unsigned long long totalTriangleSize=0;
		double coarseLowX = 0;
		double coarseLowY = 0;
		double coarseHighX = 0;
		double coarseHighY = 0;
		unsigned int xFinePartNum = 0;
		unsigned int yFinePartNum = 0;

		while(!activePartStop){
			if(row_rank==SUB_MASTER_RANK){
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				c->prepareDataForDelaunayMPI(row_size, tempPointIdArr, tempCoorArr, totalTriangleSize, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr, pointNumPartArr, currActivePartNum);
				coarseLowX = c->lowPoint.getX();
				coarseLowY = c->lowPoint.getY();
				coarseHighX = c->highPoint.getX();
				coarseHighY = c->highPoint.getY();
				xFinePartNum = c->xFinePartNum;
				yFinePartNum = c->yFinePartNum;

				//currActivePartNum is the number of active partition leftover for the next loop.
				//it means that if currActivePartNum==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				currActivePartNumLeftOver = c->activePartitionNumber();
				if(currActivePartNumLeftOver<=0) activePartStop = true;

//				std::cout<<"Number of active partitions leftover: "<<currActivePartNumLeftOver<<"\n";
			}
//return;
			//sychronize all processes
			MPI_Barrier(row_comm);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);

			//process MPI
			pMPI->processMPI(coarseLowX, coarseLowY, coarseHighX, coarseHighY, xFinePartNum, yFinePartNum, tempPointIdArr, tempCoorArr, totalTriangleSize, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr, pointNumPartArr, currActivePartNum, storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, returnTriangleIdArrNode.triangleIdArr, returnTriangleCoorArrNode.triangleCoorArr, returnTriangleIdArrNode.triangleNum);

			if(row_rank==SUB_MASTER_RANK){
				delete [] tempPointIdArr;
				delete [] tempCoorArr;
				delete [] activePartIdArr;
				delete [] activePartSizeArr;
				delete [] activePartSizeOffsetArr;
				delete [] pointNumPartArr;
				currActivePartNum = 0;
				totalTriangleSize = 0;
			}


			//sychronize all processes
			MPI_Barrier(row_comm);

			if(row_rank==SUB_MASTER_RANK){
				if(storeTriangleIdArrNode.triangleIdArr!=NULL){
					storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
					storeTriangleIdArrNode.triangleIdArr = NULL;
					storeTriangleIdArrNode.triangleNum = 0;	
				}

					returnTriangleCoorArrNode.triangleNum = returnTriangleIdArrNode.triangleNum;
					returnTriangleIdArrList.push_back(returnTriangleIdArrNode);
					returnTriangleCoorArrList.push_back(returnTriangleCoorArrNode);
					returnTriangleIdArrNode.triangleIdArr = NULL;
					returnTriangleIdArrNode.triangleNum = 0;
					returnTriangleCoorArrNode.triangleCoorArr = NULL;
					returnTriangleCoorArrNode.triangleNum = 0;

//				c->addReturnTriangles();
			}

			//sychronize all processes
			MPI_Barrier(row_comm);
		}

		if(row_rank==SUB_MASTER_RANK){
			transformTriangleIdArrList(returnTriangleIdArrList, returnTriangleIdArr, returnTriangleNum);
			transformTriangleCoorArrList(returnTriangleCoorArrList, returnTriangleCoorArr, returnTriangleNum);
			c->updateTriangleArr(returnTriangleIdArr, returnTriangleCoorArr, returnTriangleNum);
		}
		//sychronize all processes
		MPI_Barrier(row_comm);
	}
	delete pMPI;

	if(row_rank==SUB_MASTER_RANK){
		c->storeAllTriangles(storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, boundaryTriangleIdArrNode.triangleIdArr, boundaryTriangleCoorArrNode.triangleCoorArr, boundaryTriangleIdArrNode.triangleNum);

		if(storeTriangleIdArrNode.triangleIdArr!=NULL){
			storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
			storeTriangleIdArrNode.triangleIdArr = NULL;
			storeTriangleIdArrNode.triangleNum = 0;	
		}
		boundaryTriangleCoorArrNode.triangleNum = boundaryTriangleIdArrNode.triangleNum;
		if(boundaryTriangleIdArrNode.triangleIdArr!=NULL){
			boundaryTriangleIdArrList.push_back(boundaryTriangleIdArrNode);
			boundaryTriangleIdArrNode.triangleIdArr = NULL;
			boundaryTriangleIdArrNode.triangleNum = 0;
		}
		if(boundaryTriangleCoorArrNode.triangleCoorArr!=NULL){
			boundaryTriangleCoorArrList.push_back(boundaryTriangleCoorArrNode);
			boundaryTriangleCoorArrNode.triangleCoorArr = NULL;
			boundaryTriangleCoorArrNode.triangleNum = 0;
		}
	}
}

	//transform list of triangleIdArr (storeTriangleCoorArr) into flat array (triangleIdArr)
	//storeTriangleCoorArr will be ersased after inside function
	//triangleIdArr contains all triangle in a coarsePartition after a stage
	transformTriangleIdArrList(storeTriangleIdArrList, storeTriangleIdArr, storeTriangleNum);
//for(int i=0; i<triangleNum; i++)
//std::cout<<triangleIdArr[i*3]<<" "<<triangleIdArr[i*3+1]<<" "<<triangleIdArr[i*3+2]<<"  ";

	transformTriangleIdArrList(boundaryTriangleIdArrList, boundaryTriangleIdArr, boundaryTriangleNum);
	transformTriangleCoorArrList(boundaryTriangleCoorArrList, boundaryTriangleCoorArr, boundaryTriangleNum);

	//for visualization
	if(row_rank == SUB_MASTER_RANK){
		std::string currPath = generateFileName(coarsePartId, path + "delaunayResults/boundary", xCoarsePartNum*yCoarsePartNum, ".tri");
		storeTriangleCoors(boundaryTriangleCoorArr, boundaryTriangleNum, currPath, false);

		currPath = generateFileName(coarsePartId, path + "delaunayResults/interiorIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		storeTriangleIds(storeTriangleIdArr, storeTriangleNum, currPath, false);
	}


	//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
	collectStoreTriangleIdArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, storeTriangleIdArr, storeTriangleNum, path, masterAmountTime, storeAmountTime);

	if(world_rank==MASTER_RANK){
		masterTime += masterAmountTime;
		storeTime += storeAmountTime;		
	}
	//collect boundary triangles (boundaryTriangleIdArr, boundaryTriangleCoorArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
	collectBoundaryTriangleArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, boundaryTriangleIdArr, boundaryTriangleCoorArr, boundaryTriangleNum, coarsePartId, path, masterAmountTime, storeAmountTime);

	if(world_rank==MASTER_RANK){
		delete c;
		masterTime += masterAmountTime;
		storeTime += storeAmountTime;
	}
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

	double initialTime = 0;
	double masterTime = 0;
	double updateTime = 0;
	double storeTime = 0;
	double amountTime = 0;
	double currentTime;
	double overAllTime;
	unsigned int xFinePartNum, yFinePartNum;

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
		initialTime += amountTime;

		d->readFinePartitionSize(xFinePartNum, yFinePartNum);
		std::cout<<"=======================================================\n";
		std::cout<<"Triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<std::endl;
		std::cout<<"=======================================================\n";

		std::cout<<"Time for loadInitPoints() is "<<amountTime<<std::endl;

		currentTime = GetWallClockTime();
		d->initTriangulate();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		initialTime += amountTime;
		std::cout<<"Time for initTriangulate() is "<<amountTime<<std::endl;

		currentTime = GetWallClockTime();
		d->triangleTransform();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		initialTime += amountTime;
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
//Not go over stage 2, (only stage 1) for visualization
delaunayStop = true;
			std::cout<<"*****************************[[[ STAGE: "<<stage<<" ]]]**********************, activePartNum: "<<activePartNum<<std::endl;

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
			double amountTime1, amountTime2;
			delaunayPartition(world_rank, world_size, groupNum, color, row_rank, row_size, row_comm, path, amountTime1, amountTime2);
			if(world_rank==MASTER_RANK){
				masterTime += amountTime1; 
				storeTime += amountTime2;
			}

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Comm_free(&row_comm);
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

//Stop at stage 1 for visualization
MPI_Finalize();
return 0;

	if(world_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d->storeAllTriangles();//store triangles leftover in domain

		amountTime = GetWallClockTime()-currentTime;
		storeTime += amountTime;

		std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		overAllTime = MPI_Wtime()-overAllTime;
		std::cout<<"done!!!"<<std::endl;

		std::cout<<"====================================================================\n";
		std::cout<<"done!!!"<<std::endl;
		std::cout<<"One-Master version\n";
		std::cout<<"datasources: "<<path<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<std::endl;
		std::cout<<"Initial time: "<<initialTime<<"\n\n";
		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"Update time: "<<updateTime<<"\n";
		std::cout<<"Master + Update time: "<<masterTime + updateTime<<"\n";
		std::cout<<"Store time: "<<storeTime<<"\n";
		std::cout<<"Master + Update + Store time: "<<masterTime + updateTime + storeTime<<"\n";
		std::cout<<"Workers time: "<<overAllTime - (masterTime + updateTime + storeTime)<<"\n";
		std::cout<<"Total time: "<<overAllTime<<"\n";
		std::cout<<"====================================================================\n";


		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
/*		resultFile<<"\n\nMultiple-Master version\n";
		resultFile<<"datasources: "<<path<<"\n";
		resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<"\n";
		resultFile<<"Initial time: "<<initialTime<<"\n";
		resultFile<<"Master time: "<<masterTime<<"\n";
		resultFile<<"Update time: "<<updateTime<<"\n";
		resultFile<<"Master + Update + store time: "<<masterTime+updateTime+storeTime<<"\n";
		resultFile<<"Store time: "<<storeTime<<"\n";
		resultFile<<"MPI time: "<<overAllTime - (masterTime + updateTime + storeTime)<<"\n";
		resultFile<<"Total time: "<<overAllTime<<"\n";
*/
		resultFile<<"\n\Multi-Master"<<", "<<path<<", "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<" "<<masterTime<<" "<<updateTime<<" "<<masterTime+updateTime<<" "<<storeTime<<" "<<masterTime+updateTime+storeTime<<" "<<overAllTime - (masterTime + updateTime + storeTime)<<" "<<overAllTime<<"\n";

		resultFile.close();
	}
	if(world_rank==MASTER_RANK) delete d;
    MPI_Finalize();
}
