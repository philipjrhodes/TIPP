//Multiple masters
//dataset stored in server
#include "domain.h"
#include "coarsePartition.h"
#include "delaunayMPI.h"
#include <math.h>

//for memcpy
#include <cstring>

#define SUB_MASTER_RANK 0
#define return_data_tag1 2003
#define return_data_tag2 2004

//=============================================================================
//read activeCoarsePartNum, activeCoarsePartIdArr[i], activeCoarsePartInitSizeArr[i], xCorsePartNum, yCorsePartNum
//maxGroupNum is maximum number of groups, each group has one sub-master
//maxGroupNum may be greater than number of active coarse partitions
//for ex: 17 processes --> 5 groups: 0, 1, 2, 3, 4. However, We have only 4 active coarse partitions.
void readTempTrianglesCoarsePartsInfo(unsigned int &activeCoarsePartNum, unsigned int *&activeCoarsePartIdArr, unsigned int *&activeCoarsePartInitSizeArr, unsigned int groupNum, unsigned int &xCoarsePartNum, unsigned int &yCoarsePartNum, std::string outputPath){

	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = outputPath + "/tempTrianglesCoarseParts.xfdl";
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
	allocateMemory(triangleIdArr, unsigned long long, totalTriangleSize*3);
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
	allocateMemory(triangleCoorArr, double, totalTriangleSize*6);
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
			index++;
		}
		//remove subTriangleArr in triangleCoorArrList
		delete [] (*it).triangleCoorArr;
	}
	triangleCoorArrList.clear();
}


//=============================================================================
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectStoreTriangleIdArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned int *rankIdArr, unsigned long long *triangleIdArr, unsigned long long triangleNum, std::string outputPath){

	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned long long *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned long long *storeTriangleNumOffsetArr = NULL;

	if((world_rank==MASTER_RANK)){//only master node
		storeTriangleNumArr = new unsigned long long[groupNum];
		storeTriangleNumArr[0] = triangleNum;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to superMaster (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag1, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleNumArr[i], 1, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], return_data_tag1, MPI_COMM_WORLD, &status);
	}

        //process offset of each triangleIdArr of each subMaster
        if((world_rank==MASTER_RANK))
                generateOffsetArr(storeTriangleNumArr, storeTriangleNumOffsetArr, activeCoarsePartNum);

	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag2, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		unsigned long long storeTriangleIdArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int i=1; i<activeCoarsePartNum; i++)
			storeTriangleIdArrSize += storeTriangleNumArr[i];
		//add the number of stored triangles from master
		storeTriangleIdArrSize += triangleNum;

		allocateMemory(storeTriangleIdArr, unsigned long long, storeTriangleIdArrSize*3);

                //fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		 memcpy(storeTriangleIdArr, triangleIdArr, triangleNum*3*sizeof(unsigned long long));

		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[i]*3], storeTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], return_data_tag2, MPI_COMM_WORLD, &status);

		std::cout<<">>>>>Writing store triangles to file triangleIds.tri<<<<<<\n";
		std::string currPath = outputPath + "/triangleIds.tri";
		storeTriangleIds(storeTriangleIdArr, storeTriangleIdArrSize, currPath, "a");

	}

        if(row_rank==SUB_MASTER_RANK) releaseMemory(triangleIdArr);

        if(world_rank==MASTER_RANK){ //only master node
                releaseMemory(storeTriangleNumArr);
                releaseMemory(storeTriangleNumOffsetArr);
                releaseMemory(storeTriangleIdArr);
        }

}

//=============================================================================
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectBoundaryTriangleArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned int *rankIdArr, unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, std::string outputPath){

	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned long long *boundaryTriangleNumArr = NULL;
	unsigned long long *boundaryTriangleIdArr = NULL;
	double *boundaryTriangleCoorArr = NULL;
	//based on boundaryTriangleNumArr, compute boundaryTriangleNumOffsetArr
	unsigned long long *boundaryTriangleNumOffsetArr = NULL;

	if((world_rank==MASTER_RANK)){//only master node
		allocateMemory(boundaryTriangleNumArr, unsigned long long, groupNum);
		boundaryTriangleNumArr[0] = triangleNum;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to superMaster (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag1, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&boundaryTriangleNumArr[i], 1, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], return_data_tag1, MPI_COMM_WORLD, &status);
	}

        //process offset of each triangleIdArr of each subMaster
        if((world_rank==MASTER_RANK))
                generateOffsetArr(boundaryTriangleNumArr,  boundaryTriangleNumOffsetArr, activeCoarsePartNum);

	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, return_data_tag2, MPI_COMM_WORLD);
		ierr = MPI_Send(triangleCoorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, return_data_tag2, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		unsigned long long boundaryTriangleArrSize = 0;
		//calculate the size of boundaryTriangleIdArr that collect from other rank
		for(int i=1; i<activeCoarsePartNum; i++)
			boundaryTriangleArrSize += boundaryTriangleNumArr[i];
		//add the number of stored triangles from master
		boundaryTriangleArrSize += triangleNum;

                allocateMemory(boundaryTriangleIdArr, unsigned long long, boundaryTriangleArrSize*3);
                //fill triangleIds from triangleIdArr of master to boundaryTriangleIdArr
                memcpy(boundaryTriangleIdArr, triangleIdArr, triangleNum*3*sizeof(unsigned long long));

                allocateMemory(boundaryTriangleCoorArr, double, boundaryTriangleArrSize*6);
                //fill triangleIds from triangleIdArr of master to storeTriangleIdArr
                memcpy(boundaryTriangleCoorArr, triangleCoorArr, triangleNum*6*sizeof(double));

		for(int i=1; i<groupNum; i++){
			ierr = MPI_Recv(&boundaryTriangleIdArr[boundaryTriangleNumOffsetArr[i]*3], boundaryTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], return_data_tag2, MPI_COMM_WORLD, &status);
			ierr = MPI_Recv(&boundaryTriangleCoorArr[boundaryTriangleNumOffsetArr[i]*6], boundaryTriangleNumArr[i]*6, MPI_DOUBLE, rankIdArr[i], return_data_tag2, MPI_COMM_WORLD, &status);
		}

		std::cout<<">>>>>Writing boundary triangles to file boundaryIds.tri and boundaryCoors.tri <<<<<<\n";

		std::string currPath1 = outputPath + "/boundaryIds.tri";
		std::string currPath2 = outputPath + "/boundaryCoors.tri";
		storeTriangleIds(boundaryTriangleIdArr, boundaryTriangleArrSize, currPath1, "a");
		storeTriangleCoors(boundaryTriangleCoorArr, boundaryTriangleArrSize, currPath2, "a");


		releaseMemory(boundaryTriangleIdArr);
                releaseMemory(boundaryTriangleCoorArr);
        }

        if(row_rank==SUB_MASTER_RANK){
                releaseMemory(triangleIdArr);
                releaseMemory(triangleCoorArr);
        }

        if(world_rank==MASTER_RANK){ //only master node
                releaseMemory(boundaryTriangleNumArr);
                releaseMemory(boundaryTriangleNumOffsetArr);
                releaseMemory(boundaryTriangleIdArr);
        }
}


//=============================================================================
//Each active partition in domain will be Delaunay processed as a master in a group
//color is the group Id
//row_rank is the rank in each group
//row_size is the number of processes in each group
//row_comm is communication for that group
//=============================================================================
void delaunayPartition(unsigned int world_rank, unsigned int world_size, unsigned int groupNum, unsigned int *rankIdArr, unsigned int color, unsigned int row_rank, unsigned int row_size, MPI_Comm row_comm, std::string inputPath, std::string outputPath){

	std::list<nodeTriangleIdArr> storeTriangleIdArrList;
	unsigned long long *storeTriangleIdArr = NULL;
	unsigned long long storeTriangleNum = 0;
	nodeTriangleIdArr storeTriangleIdArrNode;

	std::list<nodeTriangleIdArr> returnTriangleIdArrList;
	unsigned long long *returnTriangleIdArr = NULL;
	unsigned long long returnTriangleNum = 0;
	nodeTriangleIdArr returnTriangleIdArrNode;

	std::list<nodeTriangleCoorArr> returnTriangleCoorArrList;
	double *returnTriangleCoorArr = NULL;
	nodeTriangleCoorArr returnTriangleCoorArrNode;

	std::list<nodeTriangleIdArr> boundaryTriangleIdArrList;
	unsigned long long *boundaryTriangleIdArr = NULL;
	unsigned long long boundaryTriangleNum = 0;
	nodeTriangleIdArr boundaryTriangleIdArrNode;

	std::list<nodeTriangleCoorArr> boundaryTriangleCoorArrList;
	double *boundaryTriangleCoorArr = NULL;
	nodeTriangleCoorArr boundaryTriangleCoorArrNode;

	unsigned int groupId = color;//process Id, multiple processes are divided into many groups
	unsigned int coarsePartId;// partition Id of an active partition
	unsigned int xCoarsePartNum, yCoarsePartNum;
	unsigned int activeCoarsePartNum;//number of active partitions (coarse)
	unsigned int initTriangleNum;//number of init triangles of an active partition

	unsigned int *activeCoarsePartIdArr=NULL;//partition Id for each coarse active partition
	unsigned int *activeCoarsePartInitSizeArr=NULL;//number of init triangles for each coarse active partition 
	MPI_Status status;


	//master rank reads info from tempTrianglesCoarseParts.xfdl
	if(world_rank == MASTER_RANK){
		readTempTrianglesCoarsePartsInfo(activeCoarsePartNum, activeCoarsePartIdArr, activeCoarsePartInitSizeArr, groupNum, xCoarsePartNum, yCoarsePartNum, outputPath);
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
	if(world_rank == MASTER_RANK){
		for(int i=1; i<groupNum; i++){
			MPI_Send(&activeCoarsePartIdArr[i], 1, MPI_INT, rankIdArr[i], send_data_tag, MPI_COMM_WORLD);
			MPI_Send(&activeCoarsePartInitSizeArr[i], 1, MPI_INT, rankIdArr[i], send_data_tag, MPI_COMM_WORLD);
		}
	}
	else if((row_rank==0)&&(world_rank!=MASTER_RANK)){
		MPI_Recv(&coarsePartId, 1, MPI_INT, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
		MPI_Recv(&initTriangleNum, 1, MPI_INT, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
	}

	if(world_rank == MASTER_RANK){
		delete [] activeCoarsePartIdArr;
		delete [] activeCoarsePartInitSizeArr;
	}

	delaunayMPI *pMPI = new delaunayMPI(row_rank, row_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum, inputPath, outputPath);
	coarsePartition *c;

	MPI_Barrier(MPI_COMM_WORLD);

	if(row_rank == SUB_MASTER_RANK){
		c = new coarsePartition(groupId, coarsePartId, activeCoarsePartNum, initTriangleNum, xCoarsePartNum, yCoarsePartNum, inputPath, outputPath);
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
			if(storeTriangleIdArrNode.triangleIdArr!=NULL)
				storeTriangleIdArrList.push_back(storeTriangleIdArrNode);

			boundaryTriangleCoorArrNode.triangleNum = boundaryTriangleIdArrNode.triangleNum;
			if(boundaryTriangleIdArrNode.triangleIdArr!=NULL)
				boundaryTriangleIdArrList.push_back(boundaryTriangleIdArrNode);

			if(boundaryTriangleCoorArrNode.triangleCoorArr!=NULL)
				boundaryTriangleCoorArrList.push_back(boundaryTriangleCoorArrNode);

			if(c->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
		}

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
		double coarseLowX, coarseLowY = 0, coarseHighX = 0, coarseHighY = 0;
		unsigned int xFinePartNum = 0, yFinePartNum = 0;

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

			}

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);
			//sychronize all processes
			MPI_Barrier(row_comm);

			//process MPI
			double t1, t2;
			pMPI->processMPI(coarseLowX, coarseLowY, coarseHighX, coarseHighY, xFinePartNum, yFinePartNum, tempPointIdArr, tempCoorArr, totalTriangleSize, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr, pointNumPartArr, currActivePartNum, storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, returnTriangleIdArrNode.triangleIdArr, returnTriangleCoorArrNode.triangleCoorArr, returnTriangleIdArrNode.triangleNum);

			MPI_Barrier(row_comm);

			if(row_rank==SUB_MASTER_RANK){
				releaseMemory(tempPointIdArr);
				releaseMemory(tempCoorArr);
				releaseMemory(activePartIdArr);
				releaseMemory(activePartSizeArr);
				releaseMemory(activePartSizeOffsetArr);
				releaseMemory(pointNumPartArr);
				currActivePartNum = 0;
				totalTriangleSize = 0;
			}

			if(row_rank==SUB_MASTER_RANK){
				if(storeTriangleIdArrNode.triangleIdArr!=NULL){
					storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
				}
				if(returnTriangleIdArrNode.triangleIdArr!=NULL){
					returnTriangleCoorArrNode.triangleNum = returnTriangleIdArrNode.triangleNum;
					returnTriangleIdArrList.push_back(returnTriangleIdArrNode);
					returnTriangleCoorArrList.push_back(returnTriangleCoorArrNode);
				}
			}

			MPI_Barrier(row_comm);
		}//end of outter while loop

		if(row_rank==SUB_MASTER_RANK){
			transformTriangleIdArrList(returnTriangleIdArrList, returnTriangleIdArr, returnTriangleNum);
			transformTriangleCoorArrList(returnTriangleCoorArrList, returnTriangleCoorArr, returnTriangleNum);
			c->updateTriangleArr(returnTriangleIdArr, returnTriangleCoorArr, returnTriangleNum);
		}
		MPI_Barrier(row_comm);
	}
	delete pMPI;

	if(row_rank==SUB_MASTER_RANK){
		c->storeAllTriangles(storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, boundaryTriangleIdArrNode.triangleIdArr, boundaryTriangleCoorArrNode.triangleCoorArr, boundaryTriangleIdArrNode.triangleNum);
		if(storeTriangleIdArrNode.triangleIdArr!=NULL){
			storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
		}
		boundaryTriangleCoorArrNode.triangleNum = boundaryTriangleIdArrNode.triangleNum;
		if(boundaryTriangleIdArrNode.triangleIdArr!=NULL){
			boundaryTriangleIdArrList.push_back(boundaryTriangleIdArrNode);
		}
		if(boundaryTriangleCoorArrNode.triangleCoorArr!=NULL){
			boundaryTriangleCoorArrList.push_back(boundaryTriangleCoorArrNode);
		}
	}

	//transform list of triangleIdArr (storeTriangleCoorArr) into flat array (triangleIdArr)
	//storeTriangleCoorArr will be ersased after inside function
	//triangleIdArr contains all triangle in a coarsePartition after a stage
	transformTriangleIdArrList(storeTriangleIdArrList, storeTriangleIdArr, storeTriangleNum);

	transformTriangleIdArrList(boundaryTriangleIdArrList, boundaryTriangleIdArr, boundaryTriangleNum);
	transformTriangleCoorArrList(boundaryTriangleCoorArrList, boundaryTriangleCoorArr, boundaryTriangleNum);

	MPI_Barrier(MPI_COMM_WORLD);


	//store triangleIdArr to file from each sub master
	if(row_rank == SUB_MASTER_RANK){
		std::string currPath = generateFileName(coarsePartId, outputPath + "triangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		//store storeTriangleIdArr to dataK/triangleIdsXX.tri (append)
		storeTriangleIds(storeTriangleIdArr, storeTriangleNum, currPath, "w");
	}


	double amountWorkTime, amountWaitTime;
	//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
//	collectStoreTriangleIdArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, rankIdArr, storeTriangleIdArr, storeTriangleNum, outputPath);

	//collect boundary triangles (boundaryTriangleIdArr, boundaryTriangleCoorArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
	collectBoundaryTriangleArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, rankIdArr, boundaryTriangleIdArr, boundaryTriangleCoorArr, boundaryTriangleNum, outputPath);
}


//=============================================================================
int main (int argc, char** argv){
//=============================================================================

    int world_rank, world_size;
	enum processDistributionType{even, adaptive};

	if(argc<=4){
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first and second argument are source and result paths to the dataset\n";
		std::cout<<"The third argument is the domain size (both sides)\n";
		std::cout<<"The fourth argument is the adaptive process distribution or equal process distribution\n";
		return 0;
	}
	std::string inputPath = argv[1];
	std::string outputPath = argv[2];

	//number of partitions/segments on both sides of domain
	double domainSize = atof(argv[3]);

	//process distribution styles (adaptive or equal)
	//adaptive: processes distributed for each group based on number of point in that coase partition
	//equal process distribution: processes distributed are equal to all groups
	bool processDistribution = atoi(argv[4]);

	domain *d;

	double initialTime = 0, loadInitPointTime = 0, masterTime = 0, workerTime = 0, currentTime, overAllTime=0;

	unsigned int xFinePartNum, yFinePartNum;
	unsigned int *colorArr;// each item is a groupId array
	unsigned int *rankIdArr;//each item isa world_rank such that row_rank==0
	unsigned int color;

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
		d = new domain(0.0, 0.0, domainSize, domainSize, inputPath, outputPath);
		d->loadInitPoints();
		loadInitPointTime += GetWallClockTime() - currentTime;
		std::cout<<"=======================================================\n";
		std::cout<<"Triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<std::endl;
		std::cout<<"=======================================================\n";
		std::cout<<"Time for loadInitPoints() is "<<loadInitPointTime<<std::endl;

		currentTime = GetWallClockTime();
		d->initTriangulate();
		initialTime += GetWallClockTime()-currentTime;
		std::cout<<"Time for initTriangulate() is "<<initialTime<<std::endl;

		d->triangleTransform();
	}

	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage

	int stage = 1;
	//Number of current active partitions
	unsigned int activePartNum;
	while(!delaunayStop){
		if(world_rank==MASTER_RANK){
			d->generateIntersection();
			d->generateConflictPartitions();

			//Number of real active partitions
			activePartNum = d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();

			if(d->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
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
				unsigned int *activePartPointSizeArr;//point numbers of active partitions
				currActivePartNum = d->prepareDataForDelaunayMPI(world_size, activePartPointSizeArr);

				//number of processes in a group
				groupSize = (unsigned)(world_size/currActivePartNum);
				groupNum = currActivePartNum;

				if(processDistribution==1)
					apdaptiveAllocateGroupProcesses(activePartPointSizeArr, world_size, currActivePartNum, rankIdArr, colorArr);
				else equalAllocateGroupProcesses(world_size, currActivePartNum, rankIdArr, colorArr);
std::cout<<"active partition sizes\n";
for(int i=0; i<currActivePartNum; i++) std::cout<<activePartPointSizeArr[i]<<" ";

				std::cout<<"groupSize: " + toString(groupSize) + ", groupNum: " + toString(groupNum)<<"\n";
				releaseMemory(activePartPointSizeArr);

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

			//send groupId to all processes
			MPI_Scatter(colorArr, 1, MPI_UNSIGNED, &color, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

			//Split the world communicator based on the currActivePartNum into multiple communicatiors
			MPI_Comm row_comm;
			int row_rank, row_size;

			MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);

			MPI_Comm_rank(row_comm, &row_rank);
			MPI_Comm_size(row_comm, &row_size);

			MPI_Barrier(MPI_COMM_WORLD);
			currentTime = GetWallClockTime();
			//Processes that are out of scope --> do nothing
			//for ex: 17 processes --> 5 groups: 0, 1, 2, 3, 4 (group 4 work but will stop very soon, with world_rank=16)
			//process multiple masters, each master process a coarsePartition
			delaunayPartition(world_rank, world_size, groupNum, rankIdArr, color, row_rank, row_size, row_comm, inputPath, outputPath);

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);
			workerTime += GetWallClockTime() - currentTime;

			MPI_Comm_free(&row_comm);
		}

		if(world_rank==MASTER_RANK){
			releaseMemory(colorArr);
			releaseMemory(rankIdArr);

			d->updateTriangleArr();

			stage++;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if(world_rank==MASTER_RANK){
		d->storeAllTriangles();//store triangles leftover in domain

		std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		overAllTime = MPI_Wtime()-overAllTime;
		std::cout<<"done!!!"<<std::endl;
		unsigned int finePartNum = 0;
		finePartNum = d->countFinePartitions();
		std::cout<<"====================================================================\n";
		std::cout<<"done!!!"<<std::endl;
		std::cout<<"Multiple-Masters version\n";
		std::string distributeStr = (processDistribution==even)?"equal process distribution\n":"adaptive process distribution\n";
		std::cout<<"process distribution type: "<<distributeStr;
		std::cout<<"datasources: "<<inputPath<<std::endl;
		std::cout<<"Delaunay output: "<<outputPath<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<std::endl;
//		unsigned int  pointPartNum = d->pointNumMax/(d->xPartNum*d->yPartNum*xFinePartNum*yFinePartNum);
//		std::cout<<"#average points/sub-partition: "<<pointPartNum<<"\n\n";

		std::cout<<"Load initial points time: "<<loadInitPointTime<<"\n";
		std::cout<<"Initial time: "<<initialTime<<"\n";
		masterTime = overAllTime - workerTime;
		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"Workers time: "<<workerTime<<"\n";
		std::cout<<"Total time: "<<overAllTime<<"\n";
		std::cout<<"====================================================================\n";


		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
		resultFile<<"process distribution type: "<<distributeStr;
		resultFile<<"\n\Multi-Master"<<", "<<inputPath<<", "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<" "<<", masterTime: "<<masterTime<<" "<<", workerTime: "<<workerTime<<"overAllTime: "<<overAllTime<<"\n";
		resultFile.close();
	}
	if(world_rank==MASTER_RANK) delete d;
    MPI_Finalize();
	return 0;
}
