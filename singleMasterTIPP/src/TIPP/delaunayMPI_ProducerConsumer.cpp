#include "delaunayMPI_ProducerConsumer.h"

//for memcpy
#include <cstring>

//============================================================================
delaunayMPI_ProducerConsumer::delaunayMPI_ProducerConsumer(double dSize, bool shareFolderOptionInput, std::string srcPath, std::string dstPath){
	partArr=NULL;
	pointNumPartArr=NULL;

	tempCoorArr=NULL;
	tempPointIdArr=NULL;
	triangleSizeArr=NULL;
	triangleSizeOffsetArr=NULL;

	pointArr=NULL;
	triangleList = NULL;
	storeTriangleList = NULL;
	borderTriangleList = NULL;

	interiorTriangleList = NULL;
	boundaryTriangleList = NULL;

	domainSize = dSize;
	shareFolderOption = shareFolderOptionInput;
	inputPath = srcPath;
	outputPath = dstPath;
}

//============================================================================
void delaunayMPI_ProducerConsumer::processMPI(int world_rank, unsigned int world_size){
	int ierr;
	int triangleSize;//number of triangles of current active partitions
	int triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long long *idArr=NULL;//all point id of triangles for receivers

	//active partition Id
	unsigned int partId;

	bool i_am_the_master = false;
	int root_process = 0;

	if(world_rank == MASTER_RANK) i_am_the_master = true;

	//read triangle data from file
	if(world_rank == MASTER_RANK)
		readTriangleData(world_size);

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(&xPartNum, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(&yPartNum, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(&partNum, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    int tag = 0;
    MPI_Status status;

	if(shareFolderOption==false) scatterPointData(world_rank, world_size, partId);

    if(world_rank == MASTER_RANK){
        int buf[1];
        int replybuf[1]={32};
        int numWorkItems = partNum;
        int workItemCount = numWorkItems;
        int notificationCount = world_size-1;
		int workItemId;
        
        while(notificationCount > 0){
            printf("Master waiting for messages...");
            // receive message from any source
            MPI_Recv(buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("got message from %d\n",status.MPI_SOURCE);

            // send reply back to sender of the message received above
            if(workItemCount > 0){          
                printf("\tSending work item to %d...", status.MPI_SOURCE);
                replybuf[0] = numWorkItems - workItemCount;
				workItemId = numWorkItems - workItemCount;

                MPI_Send(replybuf, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
				MPI_Send(&triangleSizeArr[workItemId], 1, MPI_UNSIGNED, status.MPI_SOURCE, send_data_tag, MPI_COMM_WORLD);
				MPI_Send(&partArr[workItemId], 1, MPI_UNSIGNED, status.MPI_SOURCE, send_data_tag, MPI_COMM_WORLD);
				MPI_Send(&pointNumPartArr[workItemId], 1, MPI_UNSIGNED, status.MPI_SOURCE, send_data_tag, MPI_COMM_WORLD);

				MPI_Send(&tempCoorArr[triangleSizeOffsetArr[workItemId]*6], triangleSizeArr[workItemId]*6, MPI_DOUBLE, status.MPI_SOURCE, send_data_tag, MPI_COMM_WORLD);
				MPI_Send(&tempPointIdArr[triangleSizeOffsetArr[workItemId]*3], triangleSizeArr[workItemId]*3, MPI_UNSIGNED_LONG_LONG, status.MPI_SOURCE, send_data_tag, MPI_COMM_WORLD);

                printf("done.\n");
                workItemCount--;
            } else {
            
                printf("\tSending NOWORK message to %d...",status.MPI_SOURCE);
                replybuf[0] = -1;
                MPI_Send(replybuf, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD); 
                printf("done.\n");
                notificationCount--;
            }
        }
    } else {
        int buf[1];
        int replybuf[1];
        int done = 0;
        int workItemId;
        do{
            buf[0]=world_rank;
            printf("Process %d sending request...", world_rank);
            MPI_Send(buf, 1, MPI_INT, MASTER_RANK, tag, MPI_COMM_WORLD);
            printf("sent. Waiting for reply...");
            MPI_Recv(replybuf, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			workItemId = replybuf[0];
            printf("received work item %d\n", replybuf[0]);

			if(workItemId!=-1){
				MPI_Recv(&triangleSize, 1, MPI_UNSIGNED, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
				MPI_Recv(&partId, 1, MPI_UNSIGNED, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
				MPI_Recv(&pointNumPartSize, 1, MPI_UNSIGNED, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);

				coorArr = new double[triangleSize*6];
				idArr = new unsigned long long[triangleSize*3];
				MPI_Recv(coorArr, triangleSize*6, MPI_DOUBLE, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
				MPI_Recv(idArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);

				//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
				generateInitTriangles(world_rank, triangleSize, coorArr, idArr);
				releaseMemory(coorArr);
				releaseMemory(idArr);

				if(shareFolderOption==true) readPointCoor(world_rank, partId);
				triangulate(partId, world_rank);
				processInteriorTriangles(world_rank, partId, world_size);
			}
			else //if(workItemId == -1)
				done = 1;

        } while(!done);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	processBoundaryTrangleList(world_rank, world_size);

	if(world_rank == MASTER_RANK){
		releaseMemory(partArr);
		releaseMemory(triangleSizeArr);
		releaseMemory(triangleSizeOffsetArr);
		releaseMemory(tempCoorArr);
		releaseMemory(tempPointIdArr);
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
double delaunayMPI_ProducerConsumer::processInteriorTriangles(int my_rank, unsigned partId, unsigned int pool_size){
	int ierr;
	unsigned int triangleNum = size(interiorTriangleList);
	if(triangleNum!=0){
		//transform interiorTriangleList to array of ids
		unsigned long long *triangleIdArr=NULL;
		listToTriangleIdArr(interiorTriangleList, triangleIdArr, triangleNum);
		removeLinkList(interiorTriangleList);

		std::string currPath = generateFileName(partId, outputPath + "/interiorTriangleIds", xPartNum*yPartNum, ".tri");
		storeTriangleIds(triangleIdArr, triangleNum, currPath, "w");
	std::cout<<"store "<<currPath<<"\n";
		releaseMemory(triangleIdArr);
	}
	return 0;
}

//============================================================================
//process boundaryTriangleList (tranform into array of triangles and send back to master node)
void delaunayMPI_ProducerConsumer::processBoundaryTrangleList(int my_rank, unsigned int pool_size){
	int ierr;
	unsigned long long *storeTriangleIdArr = NULL;
	double *storeTriangleCoorArr = NULL;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	//based on triangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;


	if(my_rank==MASTER_RANK) 
		allocateMemory(storeTriangleNumArr, unsigned int, pool_size);
	unsigned int triangleNum = size(boundaryTriangleList);

	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	if(my_rank==MASTER_RANK)
		generateOffsetArr(storeTriangleNumArr, storeTriangleNumOffsetArr, pool_size);

	unsigned long long *triangleIdArr;
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	double *triangleCoorArr;
	allocateMemory(triangleCoorArr, double, triangleNum*6);
	listToTriangleIdCoorArr(boundaryTriangleList, triangleIdArr, triangleCoorArr, triangleNum);
	removeLinkList(boundaryTriangleList);


	if(my_rank!=MASTER_RANK){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, MPI_COMM_WORLD);
		ierr = MPI_Send(triangleCoorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, send_data_tag, MPI_COMM_WORLD);
	}
	else{//master rank receives all triangleIdArr from other ranks
		//total size
		unsigned long long storeTriangleArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(unsigned int process_id=1; process_id<pool_size; process_id++)
			storeTriangleArrSize += storeTriangleNumArr[process_id];
		//add the number of stored triangles from master
		storeTriangleArrSize += triangleNum;

		allocateMemory(storeTriangleIdArr, unsigned long long, storeTriangleArrSize*3);
		allocateMemory(storeTriangleCoorArr, double, storeTriangleArrSize*6);

		//fill triangleIds & triangleCoors from triangleIdArr of master to storeTriangleIdArr and storeTriangleCoorArr
		memcpy(storeTriangleIdArr, triangleIdArr, triangleNum*3*sizeof(unsigned long long));
		memcpy(storeTriangleCoorArr, triangleCoorArr, triangleNum*6*sizeof(double));

		for(unsigned int process_id=1; process_id<pool_size; process_id++){
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, MPI_COMM_WORLD, &status);
			ierr = MPI_Recv(&storeTriangleCoorArr[storeTriangleNumOffsetArr[process_id]*6], storeTriangleNumArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, MPI_COMM_WORLD, &status);
		}

		std::string currPath1 = outputPath + "/returnTriangleIds.tri";
		storeTriangleIds(storeTriangleIdArr, storeTriangleArrSize, currPath1, "w");
		std::string currPath2 = outputPath + "/returnTriangleCoors.tri";
		storeTriangleCoors(storeTriangleCoorArr, storeTriangleArrSize, currPath2, "w");
	}

	releaseMemory(triangleIdArr);
	releaseMemory(triangleCoorArr);

	if(my_rank==MASTER_RANK){
		releaseMemory(storeTriangleNumArr);
		releaseMemory(storeTriangleNumOffsetArr);
		releaseMemory(storeTriangleIdArr);
		releaseMemory(storeTriangleCoorArr);
	}
}

//============================================================================
//read point coordinates from pointPart.ver 
//Depending on partId, read only coordinates belong tos that partition
void delaunayMPI_ProducerConsumer::readPointCoor(int my_rank, unsigned int partId){
	std::string fileStr = generateFileName(partId, inputPath + "/pointPart", xPartNum*yPartNum, ".ver");
	readPoints(pointArr, pointNumPartSize, fileStr);
}

//=============================================================================
//master read coordinate points from all files pointPartXX.ver in a active partitions
void delaunayMPI_ProducerConsumer::readCoorPointInActivePartitions(double *&pointCoorArr, unsigned long long *&pointIdArr){
	std::string fileStr;
	point *allPointArr = NULL;
	unsigned pointNum = 0;
	unsigned xFinePartNum, yFinePartNum;


	//calculate number of points in a coarse partition
	for(int i=0; i<partNum; i++)
		pointNum += pointNumPartArr[i];

	//allocate coorArr & pointIdArr for slave machines
	allocateMemory(allPointArr, point, pointNum);

	unsigned int offset = 0;
	for(int partId=0; partId<partNum; partId++){
		fileStr = generateFileName(partArr[partId], inputPath + "/pointPart", xPartNum*yPartNum, ".ver");
		readPoints_NoAllocation(allPointArr + offset, pointNumPartArr[partId], fileStr);
		offset += pointNumPartArr[partId];
	}

	pointCoorArr = new double[pointNum*2];
	pointIdArr = new unsigned long long[pointNum];
	//transform point into  (double, double)
	for(unsigned int i=0; i<pointNum; i++){
		pointCoorArr[i*2] = allPointArr[i].getX();
		pointCoorArr[i*2+1] = allPointArr[i].getY();
		pointIdArr[i] = allPointArr[i].getId();
	}
	releaseMemory(allPointArr);
}

//============================================================================
//master read all point active partitions, then scatter to worker processes for Delaunay triangulation
//master process read all point coordinates from multiple files pointPartXX.ver 
//and distribute them to worker (pointArr)
void delaunayMPI_ProducerConsumer::scatterPointData(int my_rank, int pool_size, unsigned int partId){
//    if(my_rank>=partNum) return;

	double *readPointCoorArr=NULL;
	unsigned long long *readPointIdArr=NULL;

	int *pointCoorNumPartArr = NULL;
	int *pointIdNumPartArr = NULL;
	int *pointNumPartIdOffsetArr=NULL;
	int *pointNumPartCoorOffsetArr=NULL;

	double *tempPointCoorArr=new double[pointNumPartSize*2];
	unsigned long long *tempPointIdArr=new unsigned long long[pointNumPartSize];

	if(my_rank==MASTER_RANK){
		readCoorPointInActivePartitions(readPointCoorArr, readPointIdArr);

		pointNumPartIdOffsetArr = new int[pool_size];
		pointNumPartCoorOffsetArr = new int[pool_size];
		pointCoorNumPartArr = new int[pool_size];
		pointIdNumPartArr = new int[pool_size];

		for(partId=0; partId<pool_size; partId++){
			pointIdNumPartArr[partId] = pointNumPartArr[partId];
			pointCoorNumPartArr[partId] = pointNumPartArr[partId]*2;
		}

		pointNumPartIdOffsetArr[0] = 0;
		for(partId=1; partId<pool_size; partId++){
			pointNumPartIdOffsetArr[partId] = pointNumPartIdOffsetArr[partId-1] + pointNumPartArr[partId-1];
		}
		pointNumPartCoorOffsetArr[0] = 0;
		for(partId=1; partId<pool_size; partId++){
			pointNumPartCoorOffsetArr[partId] = pointNumPartCoorOffsetArr[partId-1] + pointNumPartArr[partId-1]*2;
		}
	}

	//distribute a portion of the array (readPointCoorArr, readPointIdArr) to each worker process
	//Each process receive a different size of data (portion of readPointCoorArr, readPointIdArr)
	//pointCoorNumPartArr, pointIdNumPartArr --> number of point coordinates and point Ids size (send_counts)
	//pointNumPartIdOffsetArr, pointNumPartCoorOffsetArr --> send_displacements
	MPI_Scatterv(readPointCoorArr, pointCoorNumPartArr, pointNumPartCoorOffsetArr, MPI_DOUBLE, tempPointCoorArr, pointNumPartSize*2, MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Scatterv(readPointIdArr, pointIdNumPartArr, pointNumPartIdOffsetArr, MPI_UNSIGNED_LONG_LONG, tempPointIdArr, pointNumPartSize, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, MPI_COMM_WORLD);

	//transform to point objects
	pointArr = new point[pointNumPartSize];
	for(unsigned pointId=0; pointId<pointNumPartSize; pointId++){
		pointArr[pointId].setX(tempPointCoorArr[pointId*2]);
		pointArr[pointId].setY(tempPointCoorArr[pointId*2+1]);
		pointArr[pointId].setId(tempPointIdArr[pointId]);
	}

	delete [] tempPointCoorArr;
	delete [] tempPointIdArr;

	if(my_rank==MASTER_RANK){
		delete [] readPointCoorArr;
		delete [] readPointIdArr;

		delete [] pointCoorNumPartArr;
		delete [] pointIdNumPartArr;
		delete [] pointNumPartIdOffsetArr;
		delete [] pointNumPartCoorOffsetArr;
	}
}

//============================================================================
//based on data received from master, each slave generate all initial triangles for the partitions
void delaunayMPI_ProducerConsumer::generateInitTriangles(int my_rank, unsigned long long triangleNum, double *coorArr, unsigned long long *pointIdArr){
	for(unsigned long long index=0; index<triangleNum; index++){
		point p1(coorArr[index*6], coorArr[index*6+1], pointIdArr[index*3]);
		point p2(coorArr[index*6+2], coorArr[index*6+3], pointIdArr[index*3+1]);
		point p3(coorArr[index*6+4], coorArr[index*6+5], pointIdArr[index*3+2]);

		triangle *newTriangle = new triangle(p1, p2, p3);
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
}
//============================================================================
//read all triangle data for active partitions, and other partition information
void delaunayMPI_ProducerConsumer::readTriangleData(int pool_size){

	//read meta data for temprary files
	std::string fileInfoStr = outputPath + "/tempTriangles.xfdl";
	std::ifstream triangleInfoFile(fileInfoStr.c_str());
	if(!triangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	//read first line (1 number only) --> number of active partitions
	std::string strItem;
	triangleInfoFile >> strItem;
	partNum = atoi(strItem.c_str());

	//second line is active partition Ids
	partArr = new unsigned int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		partArr[i] = atoi(strItem.c_str());
	}

	//third line is the numbers of triangles which belong to active partitions
	triangleSizeArr = new unsigned int[partNum];//number of triangles belong to partitions
	totalTriangleSize = 0;
	for(unsigned int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
	}

	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new unsigned int[partNum];
	for(unsigned int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}

	//fifth line is the numbers of points in each active partition
	pointNumPartArr = new unsigned int[partNum];
	for(unsigned int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartArr[i] = atoi(strItem.c_str());
	}
	//sixth line is xPartNum & yPartNum
	triangleInfoFile >> strItem;
	xPartNum = atoi(strItem.c_str());
	triangleInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());
	triangleInfoFile.close();


	//read all coordinates of triangles	
	std::string dataFileStr1 = outputPath + "/tempCoor.tri";
	readTriangleCoors(tempCoorArr, totalTriangleSize, dataFileStr1);

	//read all point ids of triangles
	std::string dataFileStr2 = outputPath + "/tempPointId.tri";
	readTriangleIds(tempPointIdArr, totalTriangleSize, dataFileStr2);
}
//==============================================================================
//determine boundingbox of current partition based on partId
boundingBox delaunayMPI_ProducerConsumer::partBox(unsigned int partId){
	double xPartSize = domainSize/xPartNum;
	double yPartSize = domainSize/yPartNum;
	unsigned int gridPartX = partId % xPartNum;
	unsigned int gridPartY = partId / yPartNum;
	point lowPoint(gridPartX*xPartSize, gridPartY*yPartSize);
	point highPoint((gridPartX+1)*xPartSize, (gridPartY+1)*yPartSize);
	return boundingBox(lowPoint, highPoint);
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//===========================================================================
void delaunayMPI_ProducerConsumer::triangulate(unsigned int partId, int my_rank){

	if((pointArr==NULL) ||(pointNumPartSize==0)) return;
	unsigned int count=0;
	MPI_Get_processor_name(processor_name, &namelen);

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);
	triangulatePartition(currPartBBox, pointArr, pointNumPartSize, triangleList, storeTriangleList, borderTriangleList);
	releaseMemory(pointArr);

	if(storeTriangleList!=NULL) addLinkList(storeTriangleList, interiorTriangleList);
	if(borderTriangleList!=NULL) addLinkList(borderTriangleList, boundaryTriangleList);	


	std::cout<<"done triangulation with partId = " + std::to_string(partId) + ", rank = " + std::to_string(my_rank) + ", interiorTriangleSize: " + std::to_string(size(interiorTriangleList)) + ", boundaryTriangleSize: " + std::to_string(size(boundaryTriangleList)) + ", bad triangles: " + std::to_string(count) + ", from node " + processor_name + "\n";
}



