//Number of core may be larger than number of active partitions

#include "delaunayMPI.h"
#include "io.h"
#include "common.h"

//for memcpy
#include <cstring>


//============================================================================
delaunayMPI::delaunayMPI(double dSize, bool shareFolderOptionInput, std::string srcPath, std::string dstPath){
	partArr = NULL;
	pointNumPartArr = NULL;

	tempCoorArr = NULL;
	tempPointIdArr = NULL;
	triangleSizeArr = NULL;
	triangleSizeOffsetArr = NULL;

	pointArr = NULL;
	initialTriangleList = NULL;
	interiorTriangleList = NULL;
	boundaryTriangleList = NULL;

	domainSize = dSize;
	shareFolderOption = shareFolderOptionInput;
	inputPath = srcPath;
	outputPath = dstPath;
}

//============================================================================
void delaunayMPI::processMPI(int my_rank, unsigned int pool_size, double &triangulationTime, double &readTime, double &storeTime){
	double currentTime, processStoreTime, processReadTime, processTriangleTime, amounTime;
	triangulationTime = storeTime = readTime = 0;
	int ierr;
	MPI_Status status;
	int triangleSize;//number of triangles of current active partitions
	int triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long long *idArr=NULL;//all point id of triangles for receivers

	//active partition Id
	unsigned int partId;

	bool i_am_the_master = false;
	int root_process = 0;


	if(my_rank == MASTER_RANK) i_am_the_master = true;

	//read triangle data from file
	if(i_am_the_master)
		readTriangleData(pool_size);

	//broadcast xPartNum & yPartNum to all slave nodes
	MPI_Bcast(&xPartNum, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(&yPartNum, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	//broadcast partNum to all slave nodes
	MPI_Bcast(&partNum, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	//send number of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeArr, 1, MPI_UNSIGNED, &triangleSize, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	//send number of offset of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeOffsetArr, 1, MPI_UNSIGNED, &triangleSizeOffset, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	//send active partId to all nodes (master , slaves)
	MPI_Scatter(partArr, 1, MPI_UNSIGNED, &partId, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	//send number of points in each active partition to all nodes (master , slaves)
	//Scatter pointNumPartArr to pointNumPartSize & pointNumPartOffsetSize from master to all nodes
	//this command is used for readPointCoor()
	MPI_Scatter(pointNumPartArr, 1, MPI_UNSIGNED, &pointNumPartSize, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);


	if(i_am_the_master){
		//This is the master (root) process	
        //distribute a portion of the array (triangleArr) to each child process
		for(int process_id=1; process_id<pool_size; process_id++) {
			ierr = MPI_Send(&tempCoorArr[triangleSizeOffsetArr[process_id]*6], triangleSizeArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, MPI_COMM_WORLD);
			ierr = MPI_Send(&tempPointIdArr[triangleSizeOffsetArr[process_id]*3], triangleSizeArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, MPI_COMM_WORLD);
		}	
		//pointNum & startId for master
		coorArr = &tempCoorArr[triangleSizeOffset*6];
		idArr = &tempPointIdArr[triangleSizeOffset*3];
		//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
		generateInitTriangles(my_rank, triangleSize, coorArr, idArr);
		releaseMemory(tempCoorArr);
		releaseMemory(tempPointIdArr);
		releaseMemory(triangleSizeArr);
		releaseMemory(triangleSizeOffsetArr);
	}
	else{//worker processes

		//allocate coorArr & pointIdArr for slave machines
		allocateMemory(coorArr, double, triangleSize*6);
		allocateMemory(idArr, unsigned long long int, triangleSize*3);

		//receive data from master (process 0)
		ierr = MPI_Recv(coorArr, triangleSize*6, MPI_DOUBLE, root_process, send_data_tag, MPI_COMM_WORLD, &status);
		ierr = MPI_Recv(idArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, root_process, send_data_tag, MPI_COMM_WORLD, &status);

		if(my_rank<partNum)
			//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
			generateInitTriangles(my_rank, triangleSize, coorArr, idArr);
		releaseMemory(coorArr);
		releaseMemory(idArr)
	}

	MPI_Barrier(MPI_COMM_WORLD);
	currentTime = GetWallClockTime();
	//read from file pointPart.ver
	if(shareFolderOption)
		readPointCoor(my_rank, partId);
	else
		scatterPointData(my_rank, pool_size, partId);
	MPI_Barrier(MPI_COMM_WORLD);
	readTime += GetWallClockTime() - currentTime;

	currentTime = GetWallClockTime();
	triangulate(partId, my_rank);
	MPI_Barrier(MPI_COMM_WORLD);
	triangulationTime += GetWallClockTime() - currentTime;

	currentTime = GetWallClockTime();
	//srore ids of interiorTriangleList from triangulate function
	//processInteriorTriangles(my_rank, partId, pool_size);
	processDirectInteriorTriangles(my_rank, partId, pool_size);
	MPI_Barrier(MPI_COMM_WORLD);
	storeTime += GetWallClockTime() - currentTime;

	//send bboundaryTriangleList) back to master
	processBoundaryTriangles(my_rank, pool_size);

	if(my_rank==MASTER_RANK){
		releaseMemory(partArr);
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
void delaunayMPI::processDirectInteriorTriangles(int my_rank, unsigned partId, unsigned int pool_size){
	int ierr;
	unsigned int triangleNum = size(interiorTriangleList);
	if(triangleNum!=0){
		//transform interiorTriangleList to array of ids
		unsigned long long *triangleIdArr=NULL;
		listToTriangleIdArr(interiorTriangleList, triangleIdArr, triangleNum);
		removeLinkList(interiorTriangleList);

		std::string currPath = generateFileName(partId, outputPath + "/interiorTriangleIds", xPartNum*yPartNum, ".tri");
		storeTriangleIds(triangleIdArr, triangleNum, currPath, "w");
		releaseMemory(triangleIdArr);
	}
}

//============================================================================
//process interiorTriangleList (tranform into array triangle Ids and send back to master node)
//collect trianglIdArr from each process (not include master rank) transfer to master rank
//first: gather all trianglIdArrSize from each process to master rank
//second: compute the offset of each trianglIdArr from other rank (not master)
//third: send trianglIdArr from each process (not include master rank) to master.
//master store all trianglIdArr to file TriangleIds.tri
void delaunayMPI::processInteriorTriangles(int my_rank, unsigned partId, unsigned int pool_size){
	int ierr;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;


	if(my_rank==MASTER_RANK){//only master node
		storeTriangleNumArr = new unsigned int[pool_size];
	}

	unsigned int triangleNum = size(interiorTriangleList);
	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	if(my_rank==MASTER_RANK){//only master node
		storeTriangleNumOffsetArr = new unsigned int[pool_size];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<pool_size; i++)
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
	}

	//transform storeTriangleList to array of ids
	unsigned long long *triangleIdArr=NULL;
	if(triangleNum!=0) triangleIdArr = new unsigned long long[triangleNum*3];

	triangleNode *head = interiorTriangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		index++;
		head=head->next;
	}
	removeLinkList(interiorTriangleList);

	if(my_rank!=MASTER_RANK){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, MPI_COMM_WORLD);
	}
	else{//master rank receives all triangleIdArr from other ranks
		unsigned long long storeTriangleIdArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=1; process_id<pool_size; process_id++)
			storeTriangleIdArrSize += storeTriangleNumArr[process_id];
		//add the number of stored triangles from master
		storeTriangleIdArrSize += triangleNum;

		allocateMemory(storeTriangleIdArr, unsigned long long, storeTriangleIdArrSize*3);
		//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			storeTriangleIdArr[index*3] = triangleIdArr[index*3];
			storeTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			storeTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];
		}

		for(int process_id=1; process_id<pool_size; process_id++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, MPI_COMM_WORLD, &status);

		//store storeTriangleIdArr to triangleIds.tri (append)
		std::string currPath = outputPath + "/triangleIds.tri";
		if((storeTriangleIdArr!=NULL)||(storeTriangleIdArrSize!=0))
			storeTriangleIds(storeTriangleIdArr, storeTriangleIdArrSize, currPath, "a");
	}

//	std::string currPath = generateFileName(partId, outputPath + "/triangleIds", xPartNum*yPartNum, ".tri");
//	storeTriangleIds(triangleIdArr, triangleNum, currPath, "a");

	if(triangleIdArr!=NULL) delete [] triangleIdArr;
	if(my_rank==MASTER_RANK){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
		if(storeTriangleIdArr!=NULL) delete [] storeTriangleIdArr;
	}
}


//============================================================================
//process boundary triangles (tranform into array of triangles and send back to master node)
void delaunayMPI::processBoundaryTriangles(int my_rank, unsigned int pool_size){
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

		//allocate memory for storeTriangleIdArr
		allocateMemory(storeTriangleIdArr, unsigned long long, storeTriangleArrSize*3);

		//allocate memory for storeTriangleCoorArr
		allocateMemory(storeTriangleCoorArr, double, storeTriangleArrSize*6);

		//fill triangleIds & triangleCoors from triangleIdArr of master to storeTriangleIdArr and storeTriangleCoorArr
		memcpy(storeTriangleIdArr, triangleIdArr, triangleNum*3*sizeof(unsigned long long));
		memcpy(storeTriangleCoorArr, triangleCoorArr, triangleNum*6*sizeof(double));

		for(unsigned int process_id=1; process_id<pool_size; process_id++){
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, MPI_COMM_WORLD, &status);
			ierr = MPI_Recv(&storeTriangleCoorArr[storeTriangleNumOffsetArr[process_id]*6], storeTriangleNumArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, MPI_COMM_WORLD, &status);
		}

		//store storeTriangleIdArr to triangleIds.tri
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
void delaunayMPI::readPointCoor(int my_rank, unsigned int partId){
    if(my_rank>=partNum) return;
	std::string fileStr = generateFileName(partId, inputPath + "/pointPart", xPartNum*yPartNum, ".ver");
	readPoints(pointArr, pointNumPartSize, fileStr);
}

//=============================================================================
//master read coordinate points from all files pointPartXX.ver in a active partitions
void delaunayMPI::readCoorPointInActivePartitions(double *&pointCoorArr, unsigned long long *&pointIdArr){
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
//and distribute them to worker (pointCoorArr)
void delaunayMPI::scatterPointData(int my_rank, int pool_size, unsigned int partId){
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
void delaunayMPI::generateInitTriangles(int my_rank, unsigned long long triangleNum, double *coorArr, unsigned long long *pointIdArr){
	if(my_rank>=partNum) return;
	for(unsigned long long index=0; index<triangleNum; index++){
		point p1(coorArr[index*6], coorArr[index*6+1], pointIdArr[index*3]);
		point p2(coorArr[index*6+2], coorArr[index*6+3], pointIdArr[index*3+1]);
		point p3(coorArr[index*6+4], coorArr[index*6+5], pointIdArr[index*3+2]);
		triangle *newTriangle = new triangle(p1, p2, p3);
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(initialTriangleList, newTriangleNode);
	}
}

//============================================================================
//read all triangle data for active partitions, and other partition information
void delaunayMPI::readTriangleData(int pool_size){

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
	partArr = new unsigned int[pool_size];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		partArr[i] = atoi(strItem.c_str());
	}

	//third line is the numbers of triangles which belong to active partitions
	triangleSizeArr = new unsigned int[pool_size];//number of triangles belong to partitions
	for(unsigned int i=0; i<pool_size; i++) triangleSizeArr[i]=0;
	totalTriangleSize = 0;
	for(unsigned int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
	}


	//extraTriangleNum is the fake number add up to the totalTriangleSize
	//for threads with my_rank >= partNum
	unsigned int extraTriangleNum = 0;
	if(partNum<pool_size) extraTriangleNum = pool_size - partNum;

	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeArr[i] will be 1 (fake)
	if(partNum<pool_size){
		for(unsigned int i=partNum; i<pool_size; i++)
			triangleSizeArr[i] = 1;
	}


	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new unsigned int[pool_size];
	for(unsigned int i=0; i<pool_size; i++) triangleSizeOffsetArr[i]=0;
	for(unsigned int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}
	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeOffsetArr[i] will be fake
	if(partNum<pool_size){
		for(unsigned int i=partNum; i<pool_size; i++)
			triangleSizeOffsetArr[i] = triangleSizeOffsetArr[i-1] + triangleSizeArr[i];
	}


	//fifth line is the numbers of points in each active partition
	pointNumPartArr = new unsigned int[pool_size];
	for(unsigned int i=0; i<pool_size; i++) pointNumPartArr[i]=0;
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
	FILE *f1 = fopen(dataFileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<dataFileStr1<<std::endl;
		return;
	}
	tempCoorArr = new double[(totalTriangleSize + extraTriangleNum)*6];
	fread(tempCoorArr, sizeof(double), totalTriangleSize*6, f1);
	fclose(f1);

	//read all point ids of triangles
	std::string dataFileStr2 = outputPath + "/tempPointId.tri";
	FILE *f2 = fopen(dataFileStr2.c_str(), "rb");
	if(!f2){
		std::cout<<"not exist "<<dataFileStr2<<std::endl;
		return;
	}
	tempPointIdArr = new unsigned long long[(totalTriangleSize + extraTriangleNum)*3];
	fread(tempPointIdArr, sizeof(double), totalTriangleSize*3, f2);
	fclose(f2);
}

//==============================================================================
//determine boundingbox of current partition based on partId
boundingBox delaunayMPI::partBox(unsigned int partId){
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
//input: an array of point (pointArr), domain/partition (squareBBox), initialTriangleList
//output: a list of triangles which are triangulated: boundaryTriangleList, interiorTriangleList
//initialTriangleList is all triangles in the partition as initialization
//interiorTriangleList contains triangles stay wholly inside partition and are finalized
//==============================================================================
void delaunayMPI::triangulate(unsigned int partId, int my_rank){

	if((pointArr==NULL) ||(pointNumPartSize==0) || (my_rank>=partNum)) return;
	unsigned int count=0;
	MPI_Get_processor_name(processor_name, &namelen);

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);
	triangulatePartition(currPartBBox, pointArr, pointNumPartSize, initialTriangleList, interiorTriangleList, boundaryTriangleList);

	releaseMemory(pointArr);
	std::cout<<"done triangulation with partId = " + std::to_string(partId) + ", rank = " + std::to_string(my_rank) + ", interiorTriangleSize: " + std::to_string(size(interiorTriangleList)) + ", boundaryTriangleSize: " + std::to_string(size(boundaryTriangleList)) + ", bad triangles: " + std::to_string(count) + ", from node " + processor_name + "\n";
}

