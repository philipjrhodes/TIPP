//Multiple masters
//dataset stored in server
#include "domain.h"
#include "coarsePartition.h"
#include "delaunayMPI.h"

#define SUB_MASTER_RANK 0

//=============================================================================
//read activeCoarsePartNum, activeCoarsePartIdArr[i], activeCoarsePartInitSizeArr[i], xCorsePartNum, yCorsePartNum
void readTempTrianglesCoarsePartsInfo(unsigned int &activeCoarsePartNum, unsigned int *&activeCoarsePartIdArr, unsigned int *&activeCoarsePartInitSizeArr, unsigned int &xCoarsePartNum, unsigned int &yCoarsePartNum, std::string path){

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

	activeCoarsePartIdArr = new unsigned int[activeCoarsePartNum];
	//second line: read active partition ids (coarsePartition Ids)
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}

	activeCoarsePartInitSizeArr = new unsigned int[activeCoarsePartNum];
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
//Each active partition in domain will be Delaunay processed as a master in a group
//color is the group Id
//row_rank is the rank in each group
//row_size is the number of processes in each group
//row_comm is communication for that group
//=============================================================================
void delaunayPartition(unsigned int world_rank, unsigned int world_size, unsigned int color, unsigned int row_rank, unsigned int row_size, MPI_Comm row_comm, std::string path){
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
		readTempTrianglesCoarsePartsInfo(activeCoarsePartNum, activeCoarsePartIdArr, activeCoarsePartInitSizeArr, xCoarsePartNum, yCoarsePartNum, path);
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
		for(int i=1; i<activeCoarsePartNum; i++){
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
			c->deliverTriangles();
			if(c->unfinishedPartNum()<=0) delaunayStop = true;
delaunayStop = true;
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
activePartStop = true;
				std::cout<<"Number of active partitions leftover: "<<currActivePartNum<<"\n";
			}
			//sychronize all processes
			MPI_Barrier(row_comm);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);

			//process MPI
			pMPI->processMPI(row_rank, row_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum);

			//sychronize all processes
			MPI_Barrier(row_comm);

			if(row_rank==SUB_MASTER_RANK){
				c->addReturnTriangles();
			}

			//sychronize all processes
			MPI_Barrier(row_comm);
		}
		if(row_rank==SUB_MASTER_RANK){
//			c->updateTriangleArr();
		}
		//sychronize all processes
		MPI_Barrier(row_comm);
	}
	delete pMPI;

//	if(row_rank==SUB_MASTER_RANK)
//		c->storeAllTriangles();
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
	unsigned int coreNum;

	path = argv[1];
	domainSize = atof(argv[2]);

	domain *d;

	double masterTime = 0;
	double amountTime = 0;
	double currentTime = GetWallClockTime();	
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
	coreNum = world_size;

	if(world_rank==MASTER_RANK){	
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
			d->generateIntersection();
			d->generateConflictPartitions();
//			d->printConflictPartitions();
			activePartNum = d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();
			if(d->unfinishedPartNum()<=0) delaunayStop = true;
delaunayStop = true;
			activePartStop = false;
			std::cout<<"----------------stage: "<<stage<<"-------------------\n";

			amountTime = GetWallClockTime()-currentTime;
			masterTime += amountTime;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

		int currActivePartNum, activePartNumLeftOver;
		while(!activePartStop){
			if(world_rank==MASTER_RANK){
				currentTime = GetWallClockTime();
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				currActivePartNum = d->prepareDataForDelaunayMPI(coreNum);

				amountTime = GetWallClockTime()-currentTime;
				masterTime += amountTime;
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
			MPI_Bcast(&currActivePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

			//Split the world communicator based on the currActivePartNum into multiple communicatiors
			MPI_Comm row_comm;
			int row_rank, row_size;
			int color = world_rank / currActivePartNum;

			MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);

			MPI_Comm_rank(row_comm, &row_rank);
			MPI_Comm_size(row_comm, &row_size);

//std::cout<<"world_rank: " + toString(world_rank) + ", row_rank: " + toString(row_rank) + ", color: " + toString(color) + ", currActivePartNum: " + toString(currActivePartNum)<<"\n";

			//Processes that are out of scope --> do nothing
			//for ex: 17 processes --> 4 groups: 0, 1, 2, 3 (do not consider group 4, with world_rank=16)
			if(color < currActivePartNum)
				//process multiple masters, each master process a coarsePartition
				delaunayPartition(world_rank, world_size, color, row_rank, row_size, row_comm, path);

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Comm_free(&row_comm);

			if(world_rank==MASTER_RANK){
				currentTime = GetWallClockTime();
//				d->addReturnTriangles();

				amountTime = GetWallClockTime()-currentTime;
				masterTime += amountTime;
//				d->printTriangleArray();
			}

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);
		}
		if(world_rank==MASTER_RANK){
			currentTime = GetWallClockTime();
//			d->updateTriangleArr();
			amountTime = GetWallClockTime()-currentTime;
			masterTime += amountTime;
			stage++;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);
	}


MPI_Finalize();
return 0;


	if(world_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d->storeAllTriangles();

		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;

		std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		overAllTime = MPI_Wtime()-overAllTime;
		std::cout<<"done!!!"<<std::endl;
		std::cout<<"datasources: "<<path<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;
		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"MPI time: "<<overAllTime - masterTime<<"\n";
		std::cout<<"Total time: "<<overAllTime<<"\n";

		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
		resultFile<<"\n\ndatasources: "<<path<<"\n";
		resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<"\n";
		resultFile<<"Master time: "<<masterTime<<"\n";
		resultFile<<"MPI time: "<<overAllTime - masterTime<<"\n";
		resultFile<<"Total time: "<<overAllTime<<"\n";
		resultFile.close();
	}
	if(world_rank==MASTER_RANK) delete d;
    MPI_Finalize();
}