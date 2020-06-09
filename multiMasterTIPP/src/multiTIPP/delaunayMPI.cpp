//Number of core may be larger than number of active partitions

#include "delaunayMPI.h"

//for memcpy
#include <cstring>

//============================================================================
delaunayMPI::delaunayMPI(unsigned int myrank, unsigned int poolsize, MPI_Comm rowcomm, unsigned int coarsePartitionId, unsigned int xCoarsePartionNum, unsigned int yCoarsePartionNum, std::string srcPath, std::string dstPath){

	my_rank = myrank;
	pool_size = poolsize;
	row_comm = rowcomm;
	coarsePartId = coarsePartitionId;
	xCoarsePartNum = xCoarsePartionNum;
	yCoarsePartNum = yCoarsePartionNum;

	finePartArr=NULL;
	pointNumPartArr=NULL;

	pointCoorArr=NULL;
	triangleList = NULL;
	boundaryTriangleList = NULL;
	interiorTriangleList = NULL;

	inputPath = srcPath;
	outputPath = dstPath;
}

//============================================================================
void delaunayMPI::processMPI(double lowX, double lowY, double highX, double highY, unsigned int xPartNum, unsigned int yPartNum, unsigned long long *inputPointIdArr, double *inputCoorArr, unsigned long long inputTriangleSize, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr, unsigned int *pointNumPartitionArr, unsigned int &currActivePartNum, unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleIdArrSize){

	returnStoreTriangleIdArr = NULL;
	returnStoreTriangleIdArrSize = 0;
	returnTriangleIdArr = NULL;
	returnTriangleCoorArr = NULL;
	returnTriangleIdArrSize = 0;

	int ierr;
	unsigned long long triangleSize;//number of triangles of current active partitions
	unsigned long triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long long *idArr=NULL;//all point id of triangles for receivers

	double *tempCoorArr=NULL;
	unsigned long long *tempPointIdArr=NULL;

	unsigned int *triangleSizeArr;//number of triangle belong to each partition
	unsigned int *triangleSizeOffsetArr;

	//active partition Id
	unsigned int finePartId;

	bool i_am_the_master = false;
	unsigned int root_process = 0;

	if(my_rank == MASTER_RANK){
		i_am_the_master = true;

		finePartNum = currActivePartNum;
		totalTriangleSize = inputTriangleSize;

		coarseLowX = lowX;
		coarseLowY = lowY;
		coarseHighX = highX;
		coarseHighY = highY;

		xFinePartNum = xPartNum;
		yFinePartNum = yPartNum;

		finePartArr = new unsigned int[pool_size];
		for(unsigned int i=0; i<finePartNum; i++) finePartArr[i] = activePartIdArr[i];

		pointNumPartArr = new unsigned int[pool_size];
		for(unsigned int i=0; i<pool_size; i++) pointNumPartArr[i]=0;
		for(unsigned int i=0; i<finePartNum; i++) pointNumPartArr[i] = pointNumPartitionArr[i];

		triangleSizeArr = new unsigned int[pool_size];//number of triangles belong to partitions
		for(unsigned int i=0; i<pool_size; i++) triangleSizeArr[i]=0;
		for(unsigned int i=0; i<finePartNum; i++) triangleSizeArr[i] = activePartSizeArr[i];

		if(finePartNum<pool_size)
			for(unsigned int i=finePartNum; i<pool_size; i++) triangleSizeArr[i] = 1;

		triangleSizeOffsetArr = new unsigned int[pool_size];
		for(unsigned int i=0; i<pool_size; i++) triangleSizeOffsetArr[i]=0;
		for(unsigned int i=0; i<finePartNum; i++) triangleSizeOffsetArr[i] = activePartSizeOffsetArr[i];
//for(unsigned int i=0; i<finePartNum; i++) std::cout<<triangleSizeOffsetArr[i]<<" ";

		//in case number of active partitions is less than number of MPI threads
		//in this case triangleSizeOffsetArr[i] will be fake
		if(finePartNum<pool_size)
			for(unsigned int i=finePartNum; i<pool_size; i++)
				triangleSizeOffsetArr[i] = triangleSizeOffsetArr[i-1] + triangleSizeArr[i];

		unsigned int extraTriangleNum = 0;
		if(finePartNum<pool_size) extraTriangleNum = pool_size - finePartNum;

		tempPointIdArr = new unsigned long long[(totalTriangleSize + extraTriangleNum)*3];
		memcpy(tempPointIdArr, inputPointIdArr, totalTriangleSize*3*sizeof(unsigned long long));

		tempCoorArr = new double[(totalTriangleSize + extraTriangleNum)*6];
		memcpy(tempCoorArr, inputCoorArr, totalTriangleSize*6*sizeof(double));
	}

	MPI_Barrier(row_comm);

	//broadcast coarsePartId from sub master (my_rank==0) to other ranks in row_comm
	MPI_Bcast(&coarsePartId, 1, MPI_UNSIGNED, 0, row_comm);

	//broadcast coarseLowX, coarseLowY, coarseHighX, coarseHighY from sub master (my_rank==0) to other ranks in row_comm
	MPI_Bcast(&coarseLowX, 1, MPI_DOUBLE, 0, row_comm);
	MPI_Bcast(&coarseLowY, 1, MPI_DOUBLE, 0, row_comm);
	MPI_Bcast(&coarseHighX, 1, MPI_DOUBLE, 0, row_comm);
	MPI_Bcast(&coarseHighY, 1, MPI_DOUBLE, 0, row_comm);

	//broadcast xFinePartNum & yFinePartNum to all slave nodes
	MPI_Bcast(&xFinePartNum, 1, MPI_UNSIGNED, 0, row_comm);
	MPI_Bcast(&yFinePartNum, 1, MPI_UNSIGNED, 0, row_comm);

	//broadcast finePartNum to all slave nodes
	MPI_Bcast(&finePartNum, 1, MPI_UNSIGNED, 0, row_comm);

	//send number of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeArr, 1, MPI_UNSIGNED, &triangleSize, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	//send number of offset of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeOffsetArr, 1, MPI_UNSIGNED, &triangleSizeOffset, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	//send active finePartId to all nodes (master , slaves)
	MPI_Scatter(finePartArr, 1, MPI_UNSIGNED, &finePartId, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	//send number of points and offsets in each active partition to all nodes (master , slaves)
	//Scatter pointNumPartArr & pointNumPartOffsetArr to pointNumPartSize & pointNumPartOffsetSize from master to all nodes
	//this command is used for readPointCoor()
	MPI_Scatter(pointNumPartArr, 1, MPI_UNSIGNED, &pointNumPartSize, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);



	if(i_am_the_master){
		//This is the master (root) process
        //distribute a portion of the array (triangleArr) to each child process
		for(unsigned int process_id=1; process_id<pool_size; process_id++) {
			ierr = MPI_Send(&tempCoorArr[triangleSizeOffsetArr[process_id]*6], triangleSizeArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, row_comm);
			ierr = MPI_Send(&tempPointIdArr[triangleSizeOffsetArr[process_id]*3], triangleSizeArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, row_comm);
		}	
		//pointNum & startId for master
		coorArr = &tempCoorArr[triangleSizeOffset*6];
		idArr = &tempPointIdArr[triangleSizeOffset*3];
		//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
		generateTriangles(triangleSize, coorArr, idArr);

		releaseMemory(tempCoorArr);
		releaseMemory(tempPointIdArr);
		releaseMemory(triangleSizeArr);
		releaseMemory(triangleSizeOffsetArr);
	}
	else{//////////////////////This is the other process/////////////////////////////////
		allocateMemory(coorArr, double, triangleSize*6);
		allocateMemory(idArr, unsigned long long, triangleSize*3);

		//receive data from master (process 0)
		ierr = MPI_Recv(coorArr, triangleSize*6, MPI_DOUBLE, root_process, send_data_tag, row_comm, &status);
		ierr = MPI_Recv(idArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, root_process, send_data_tag, row_comm, &status);

		if(my_rank<finePartNum)
			//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
			generateTriangles(triangleSize, coorArr, idArr);
		releaseMemory(coorArr);
		releaseMemory(idArr);
	}

	//read from file pointPart.ver, not parallel
	readPointCoor(finePartId);

	triangulate(finePartId);

	//store ids of interiorTriangleList from triangulate function
	processInteriorTriangles(returnStoreTriangleIdArr, returnStoreTriangleIdArrSize);
	//processDirectInteriorTriangles(returnStoreTriangleIdArr, returnStoreTriangleIdArrSize);

	//store boundaryTriangleList from triangulate function
	processBoundaryTriangles(returnTriangleIdArr, returnTriangleCoorArr, returnTriangleIdArrSize);

	if(my_rank==MASTER_RANK){
		releaseMemory(finePartArr);
		releaseMemory(pointNumPartArr);
	}
}


//============================================================================
//process interiorTriangleList (tranform into array triangle Ids and send back to master node)
//collect trianglIdArr from each process (not include master rank) transfer to master rank
//first: gather all trianglIdArrSize from each process to master rank
//second: compute the offset of each trianglIdArr from other rank (not master)
//third: send trianglIdArr from each process (not include master rank) to master.
//master store all trianglIdArr to file TriangleIds.tri
void delaunayMPI::processDirectInteriorTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize){
	int ierr;
	unsigned int triangleNum = size(interiorTriangleList);

	//transform interiorTriangleList to array of ids
	unsigned long long *triangleIdArr=NULL;
	listToTriangleIdArr(interiorTriangleList, triangleIdArr, triangleNum);
	removeLinkList(interiorTriangleList);

	if(triangleNum!=0){
		std::string currPath = generateFileName(coarsePartId, outputPath + "/triangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		storeTriangleIds(triangleIdArr, triangleNum, currPath, "a");

	}
	releaseMemory(triangleIdArr);

	returnStoreTriangleIdArr = NULL;
	returnStoreTriangleIdArrSize = 0;
}


//============================================================================
//process interiorTriangleList (tranform into array triangle Ids and send back to master node)
void delaunayMPI::processInteriorTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize){
	int ierr;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;

	if(my_rank==MASTER_RANK)
		allocateMemory(storeTriangleNumArr, unsigned int, pool_size);

	unsigned long long triangleNum = size(interiorTriangleList);
	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK)
		generateOffsetArr(storeTriangleNumArr, storeTriangleNumOffsetArr, pool_size);


	//transform interiorTriangleList to array of ids
	unsigned long long *triangleIdArr = NULL;
	extractTriangleIds(interiorTriangleList, triangleIdArr, triangleNum);
	removeLinkList(interiorTriangleList);

	if(my_rank!=MASTER_RANK)//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, row_comm);
	else{//master rank receives all triangleIdArr from other ranks
		unsigned long long storeTriangleIdArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=1; process_id<pool_size; process_id++)
			storeTriangleIdArrSize += storeTriangleNumArr[process_id];
		//add the number of stored triangles from master
		storeTriangleIdArrSize += triangleNum;

		if(storeTriangleIdArrSize!=0){
			allocateMemory(storeTriangleIdArr, unsigned long long, storeTriangleIdArrSize*3);
			//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
			memcpy(storeTriangleIdArr, triangleIdArr, triangleNum*3*sizeof(unsigned long long));
		}
		for(int process_id=1; process_id<pool_size; process_id++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, row_comm, &status);

		returnStoreTriangleIdArr = storeTriangleIdArr;
		returnStoreTriangleIdArrSize = storeTriangleIdArrSize;
	}

//	std::string currPath = generateFileName(coarsePartId, outputPath + "/triangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	//store storeTriangleIdArr to /triangleIdsXX.tri (append)
//	storeTriangleIds(triangleIdArr, triangleNum, currPath, "a");

	if((triangleIdArr!=NULL)||(triangleNum!=0)) 
		releaseMemory(triangleIdArr);

	if(my_rank==MASTER_RANK){ //only master node
		releaseMemory(storeTriangleNumArr);
		releaseMemory(storeTriangleNumOffsetArr);
	}
}

//============================================================================
//process boundaryTriangleList (tranform into array of triangles and send back to master node)
void delaunayMPI::processBoundaryTriangles(unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleArrSize){
	int ierr;

	unsigned long long *storeTriangleIdArr = NULL;
	double *storeTriangleCoorArr = NULL;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	//based on triangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;

	//only master node
	if(my_rank==MASTER_RANK) storeTriangleNumArr = new unsigned int[pool_size];
	unsigned long long triangleNum = size(boundaryTriangleList);

	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK)
		generateOffsetArr(storeTriangleNumArr, storeTriangleNumOffsetArr, pool_size);

	unsigned long long *triangleIdArr;
	double *triangleCoorArr;
	extractTriangleIdsCoors(boundaryTriangleList, triangleIdArr, triangleCoorArr, triangleNum);
	removeLinkList(boundaryTriangleList);


	if(my_rank!=MASTER_RANK){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, row_comm);
		ierr = MPI_Send(triangleCoorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, send_data_tag, row_comm);
	}
	else{//master rank receives all triangleIdArr from other ranks
		//total size
		unsigned long long storeTriangleArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=1; process_id<pool_size; process_id++)
			storeTriangleArrSize += storeTriangleNumArr[process_id];
		//add the number of stored triangles from master
		storeTriangleArrSize += triangleNum;

		//allocate memory for storeTriangleIdArr
		allocateMemory(storeTriangleIdArr, unsigned long long, storeTriangleArrSize*3);

		//allocate memory for storeTriangleCoorArr
		allocateMemory(storeTriangleCoorArr, double, storeTriangleArrSize*6);

		//fill triangleIds & triangleCoors from triangleIdArr of master to storeTriangleIdArr and storeTriangleCoorArr
		memcpy(storeTriangleIdArr, triangleIdArr, triangleNum*3*sizeof(unsigned long long));
		memcpy(storeTriangleCoorArr, triangleCoorArr, triangleNum*6*sizeof(double));

		for(unsigned int process_id=1; process_id<pool_size; process_id++){
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, row_comm, &status);
			ierr = MPI_Recv(&storeTriangleCoorArr[storeTriangleNumOffsetArr[process_id]*6], storeTriangleNumArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, row_comm, &status);
		}

		returnTriangleIdArr = storeTriangleIdArr;
		returnTriangleCoorArr = storeTriangleCoorArr;
		returnTriangleArrSize = storeTriangleArrSize;
	}

	releaseMemory(triangleIdArr);
	releaseMemory(triangleCoorArr);

	if(my_rank==MASTER_RANK){
		releaseMemory(storeTriangleNumArr);
		releaseMemory(storeTriangleNumOffsetArr);
	}
}

//============================================================================
//read point coordinates from pointPart.ver 
//Depending on finePartId, read only coordinates belong tos that partition
void delaunayMPI::readPointCoor(unsigned int finePartId){
    if((my_rank>=finePartNum) || (pointNumPartSize==0)) return;

	std::string fileStr = generateFileName(coarsePartId, inputPath + "/pointPart", xCoarsePartNum*yCoarsePartNum, "");
	fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum,".ver");
	readPoints(pointCoorArr, pointNumPartSize, fileStr);
}

//============================================================================
//based on data received from master, each slave generate all initial triangles for the partitions
void delaunayMPI::generateTriangles(unsigned long long triangleNum, double *coorArr, unsigned long long *pointIdArr){
	if(my_rank>=finePartNum) return;
	for(unsigned long long index=0; index<triangleNum; index++){
		point p1(coorArr[index*6], coorArr[index*6+1], pointIdArr[index*3]);
		point p2(coorArr[index*6+2], coorArr[index*6+3], pointIdArr[index*3+1]);
		point p3(coorArr[index*6+4], coorArr[index*6+5], pointIdArr[index*3+2]);

		triangle *newTriangle = new triangle(p1, p2, p3);
		newTriangle->computeCenterRadius();
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void delaunayMPI::triangulate(unsigned int finePartId){

	if((pointCoorArr==NULL) || (pointNumPartSize==0) || (my_rank>=finePartNum)) return;
	unsigned int count=0;
	MPI_Get_processor_name(processor_name, &namelen);

	//determine boundingbox of current partition based on finePartId
	point lowPoint(coarseLowX, coarseLowY);
	point highPoint(coarseHighX, coarseHighY);
	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = findPart(finePartId, lowPoint, highPoint, xFinePartNum, yFinePartNum);
	triangulatePartition(currPartBBox, pointCoorArr, pointNumPartSize, triangleList, interiorTriangleList, boundaryTriangleList);
	releaseMemory(pointCoorArr);
}

