//Number of core may be larger than number of active partitions

#include "delaunayMPI.h"

//============================================================================
delaunayMPI::delaunayMPI(std::string dataNameStr, unsigned int numberOfNodes){

	finePartArr=NULL;
	pointNumPartArr=NULL;

	tempCoorArr=NULL;
	tempPointIdArr=NULL;
	triangleSizeArr=NULL;
	triangleSizeOffsetArr=NULL;

	pointCoorArr=NULL;
	triangleList = NULL;
	temporaryTriangleList = NULL;
	storeTriangleList = NULL;

	dataName = dataNameStr;
	nodeNum = numberOfNodes;
}

//============================================================================
double delaunayMPI::processMPI(unsigned int my_rank, unsigned int pool_size, MPI_Comm row_comm, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum){
	double currentTime, processStoreTime, processTriangleTime, masterTime = 0;
	int ierr;
	unsigned int triangleSize;//number of triangles of current active partitions
	unsigned int triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long long *idArr=NULL;//all point id of triangles for receivers

	//active partition Id
	unsigned int finePartId;

	bool i_am_the_master = false;
	unsigned int root_process = 0;


	if(my_rank == MASTER_RANK) i_am_the_master = true;

	//read triangle data from file
	if(i_am_the_master){
		currentTime = GetWallClockTime();
		readTriangleData(pool_size, coarsePartId, xCoarsePartNum, yCoarsePartNum);
		masterTime += GetWallClockTime()-currentTime;
	}

	MPI_Barrier(row_comm);
//	if(my_rank==MASTER_RANK) std::cout<<"***************MASTER SENDS INFO TO ALL WORKERS****************\n";

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

//	MPI_Barrier(row_comm);

//	if(my_rank<finePartNum)
//		std::cout<<"finePartId: " + toString(finePartId) + ", pointNumPartSize: " + toString(pointNumPartSize)<<"\n";

//	if(my_rank==MASTER_RANK) std::cout<<"***************MASTER SENDS ACTIVE TRIANGLES TO ALL WORKERS****************\n";


	if(i_am_the_master){
		currentTime = GetWallClockTime();
		//This is the master (root) process
	    //printf("Hello from master process %d on %s of %d\n", my_rank, processor_name, pool_size);
		
        //distribute a portion of the array (triangleArr) to each child process
		for(int process_id=1; process_id<pool_size; process_id++) {
			ierr = MPI_Send(&tempCoorArr[triangleSizeOffsetArr[process_id]*6], triangleSizeArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, row_comm);
			ierr = MPI_Send(&tempPointIdArr[triangleSizeOffsetArr[process_id]*3], triangleSizeArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, row_comm);
		}	
		//pointNum & startId for master
		coorArr = &tempCoorArr[triangleSizeOffset*6];
		idArr = &tempPointIdArr[triangleSizeOffset*3];
		//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
		generateTriangles(my_rank, triangleSize, coorArr, idArr);
		delete [] tempCoorArr;
		delete [] tempPointIdArr;
		delete [] triangleSizeArr;
		delete [] triangleSizeOffsetArr;
		masterTime += GetWallClockTime()-currentTime;
	}
	else{//////////////////////This is the other process/////////////////////////////////
//	    printf("Hello from other process %d on %s of %d\n", my_rank, processor_name, pool_size);

		//allocate coorArr & pointIdArr for slave machines
		try {
			coorArr = new double[triangleSize*6];
		} catch (std::bad_alloc&) {
		  // Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!\n";
			exit(1);
		}

		try {
			idArr = new unsigned long long[triangleSize*3];
		} catch (std::bad_alloc&) {
		  // Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!\n";
			exit(1);
		}

		//receive data from master (process 0)
		ierr = MPI_Recv(coorArr, triangleSize*6, MPI_DOUBLE, root_process, send_data_tag, row_comm, &status);
		ierr = MPI_Recv(idArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, root_process, send_data_tag, row_comm, &status);

		if(my_rank<finePartNum)
			//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
			generateTriangles(my_rank, triangleSize, coorArr, idArr);
		delete [] coorArr;
		delete [] idArr;
	}

	//after we have triangle information triangleList from generateTriangles and (pointCoorArr) from readPointCoordinates we will do delaunay/triangulate

	//read from file pointPart.ver, not parallel
//	if(my_rank==MASTER_RANK) std::cout<<"***************WORKERS READ POINTS FOR THEIR TRIANGLULATION****************\n";
	readPointCoor(my_rank, finePartId, coarsePartId, xCoarsePartNum, yCoarsePartNum);


//	if(my_rank==MASTER_RANK) std::cout<<"***************ALL WORKERS ARE TRIANGULATING****************\n";
	triangulate(coarsePartId, finePartId, my_rank);

//	if(my_rank==MASTER_RANK) std::cout<<"***************WORKERS ARE WRITING FINALIZED & BOUNDARY TRIANGLES BACK TO MASTER***************\n";
	//srore ids of storeTriangleList from triangulate function
	processStoreTime = processStoreTriangles(my_rank, finePartId, pool_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum);

	//store triangleList & temporaryTriangleList from triangulate function
	processTriangleTime = processTriangleList(my_rank, pool_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum);

	if(my_rank==MASTER_RANK){
		delete [] finePartArr;
		delete [] pointNumPartArr;
//		delete [] pointNumPartOffsetArr;
	}
	return masterTime;
}

//============================================================================
void delaunayMPI::storeTriangleIds(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, std::string currPath){
	FILE *f = fopen(currPath.c_str(), "a");
	if(!f){
		std::cout<<"not success to open "<<currPath<<std::endl;
		exit(1);
	}
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
}

//============================================================================
//process storeTriangleList (tranform into array triangle Ids and send back to master node)
//collect trianglIdArr from each process (not include master rank) transfer to master rank
//first: gather all trianglIdArrSize from each process to master rank
//second: compute the offset of each trianglIdArr from other rank (not master)
//third: send trianglIdArr from each process (not include master rank) to master.
//master store all trianglIdArr to file triangleIdsXX.tri
double delaunayMPI::processStoreTriangles(unsigned int my_rank, unsigned int finePartId, unsigned int pool_size, MPI_Comm row_comm, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum){
	double currentTime, masterTime = 0;
	int ierr;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;
	unsigned int storeTriangleNumOffset;


	if(my_rank==MASTER_RANK){//only master node
		storeTriangleNumArr = new unsigned int[pool_size];
	}

	unsigned int triangleNum = size(storeTriangleList);
	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK){//only master node
		currentTime = GetWallClockTime();
		storeTriangleNumOffsetArr = new unsigned int[pool_size];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<pool_size; i++)
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
		masterTime += GetWallClockTime()-currentTime;
	}

	//transform storeTriangleList to array of ids
	unsigned long long *triangleIdArr;
	triangleIdArr = new unsigned long long[triangleNum*3];
	triangleNode *head = storeTriangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		index++;
		head=head->next;
	}
	removeLinkList(storeTriangleList);


	if(my_rank!=MASTER_RANK){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, row_comm);
	}
	else{//master rank receives all triangleIdArr from other ranks
		currentTime = GetWallClockTime();
		unsigned int storeTriangleIdArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=1; process_id<pool_size; process_id++)
			storeTriangleIdArrSize += storeTriangleNumArr[process_id];
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

		for(int process_id=1; process_id<pool_size; process_id++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, row_comm, &status);

		std::string	folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
		std::string currPath = generateFileName(coarsePartId, folderShare + dataName + "/triangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		//store storeTriangleIdArr to dataK/triangleIdsXX.tri (append)
		storeTriangleIds(storeTriangleIdArr, storeTriangleIdArrSize, currPath);
		masterTime += GetWallClockTime()-currentTime;
	}

	delete [] triangleIdArr;

	if(my_rank==MASTER_RANK){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
		delete [] storeTriangleIdArr;
	}
	return masterTime;
}

//============================================================================
void delaunayMPI::storeTriangleCoors(double *triangleCoorArr, unsigned int triangleCoorArrSize, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not success to open "<<fileStr<<std::endl;
		exit(1);
	}
	fwrite(triangleCoorArr, triangleCoorArrSize*6, sizeof(double), f);
	fclose(f);
}

//============================================================================
//process triangleList & temporaryTriangleList (tranform into array of triangles and send back to master node)
double delaunayMPI::processTriangleList(unsigned int my_rank, unsigned int pool_size, MPI_Comm row_comm, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum){
	double currentTime, masterTime = 0;
	int ierr;
	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	unsigned long long *storeTriangleIdArr = NULL;
	double *storeTriangleCoorArr = NULL;
	//number of store triangles of each process
	int *storeTriangleNumArr = NULL;
	//based on triangleNumArr, compute storeTriangleNumOffsetArr
	int *storeTriangleNumOffsetArr = NULL;

	if(my_rank==MASTER_RANK) storeTriangleNumArr = new int[pool_size];
	unsigned int triangleNum = size(triangleList);

	MPI_Gather(&triangleNum, 1, MPI_INT, storeTriangleNumArr, 1, MPI_INT, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK){//only master node
		currentTime = GetWallClockTime();
		storeTriangleNumOffsetArr = new int[pool_size];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<pool_size; i++){
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
		masterTime += GetWallClockTime()-currentTime;
		}
	}

	unsigned long long *triangleIdArr = new unsigned long long[triangleNum*3];
	//this is local pointCoorArr (different from the global one)
	double *pointCoorArr = new double[triangleNum*6];

	triangleNode *head = triangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		pointCoorArr[index*6] = head->tri->p1.getX();
		pointCoorArr[index*6+1] = head->tri->p1.getY();
		pointCoorArr[index*6+2] = head->tri->p2.getX();
		pointCoorArr[index*6+3] = head->tri->p2.getY();
		pointCoorArr[index*6+4] = head->tri->p3.getX();
		pointCoorArr[index*6+5] = head->tri->p3.getY();

		index++;
		head=head->next;
	}
	removeLinkList(triangleList);


	if(my_rank!=MASTER_RANK){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, row_comm);
		ierr = MPI_Send(pointCoorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, send_data_tag, row_comm);
	}
	else{//master rank receives all triangleIdArr from other ranks
		currentTime = GetWallClockTime();
		//total size
		unsigned int storeTriangleArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=1; process_id<pool_size; process_id++)
			storeTriangleArrSize += storeTriangleNumArr[process_id];
		//add the number of stored triangles from master
		storeTriangleArrSize += triangleNum;

		//allocate memory for storeTriangleIdArr
		try {
			storeTriangleIdArr = new unsigned long long[storeTriangleArrSize*3];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!\n";
			exit(1);
		}

		//allocate memory for storeTriangleCoorArr
		try {
			storeTriangleCoorArr = new double[storeTriangleArrSize*6];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!\n";
			exit(1);
		}

		//fill triangleIds & triangleCoors from triangleIdArr of master to storeTriangleIdArr and storeTriangleCoorArr
		for(unsigned int index=0; index<triangleNum; index++){
			storeTriangleIdArr[index*3] = triangleIdArr[index*3];
			storeTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			storeTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];

			storeTriangleCoorArr[index*6] = pointCoorArr[index*6];
			storeTriangleCoorArr[index*6+1] = pointCoorArr[index*6+1];
			storeTriangleCoorArr[index*6+2] = pointCoorArr[index*6+2];
			storeTriangleCoorArr[index*6+3] = pointCoorArr[index*6+3];
			storeTriangleCoorArr[index*6+4] = pointCoorArr[index*6+4];
			storeTriangleCoorArr[index*6+5] = pointCoorArr[index*6+5];
		}

		for(int process_id=1; process_id<pool_size; process_id++){
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[process_id]*3], storeTriangleNumArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, row_comm, &status);
			ierr = MPI_Recv(&storeTriangleCoorArr[storeTriangleNumOffsetArr[process_id]*6], storeTriangleNumArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, row_comm, &status);
		}

		//store storeTriangleIdArr to triangleIds.tri (append)
		std::string	folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
		std::string currPath1 = generateFileName(coarsePartId, folderShare + dataName + "/returnTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		std::string currPath2 = generateFileName(coarsePartId, folderShare + dataName + "/returnTriangleCoors", xCoarsePartNum*yCoarsePartNum, ".tri");

		storeTriangleIds(storeTriangleIdArr, storeTriangleArrSize, currPath1);
		storeTriangleCoors(storeTriangleCoorArr, storeTriangleArrSize, currPath2);
		masterTime += GetWallClockTime()-currentTime;
	}

	//release memory
	delete [] triangleIdArr;
	delete [] pointCoorArr;

	if(my_rank==MASTER_RANK){
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
		delete [] storeTriangleIdArr;
		delete [] storeTriangleCoorArr;
	}
	return masterTime;
}

//============================================================================
//read point coordinates from pointPart.ver 
//Depending on finePartId, read only coordinates belong tos that partition
void delaunayMPI::readPointCoor(unsigned int my_rank, unsigned int finePartId, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum){
    if(my_rank>=finePartNum) return;
	std::string	folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
	std::string fileStr = generateFileName(coarsePartId, folderShare + dataName + "/pointPart", xCoarsePartNum*yCoarsePartNum, "");
	fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum,".ver");
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	//pointCoorArr is an array of points
	pointCoorArr = new point[pointNumPartSize];
	fread(pointCoorArr, pointNumPartSize, sizeof(point), f);
	fclose(f);

/*	if(my_rank==0){
	std::cout<<finePartId<<" "<<fileStr<<"\n";
	for(int i=0; i<pointNumPartSize; i++)
			std::cout<<pointCoorArr[i].getX()<<" "<<pointCoorArr[i].getY()<<" "<<pointCoorArr[i].getId()<<std::endl;
	}
*/
}

//============================================================================
//based on data received from master, each slave generate all initial triangles for the partitions
void delaunayMPI::generateTriangles(unsigned int my_rank, unsigned int triangleNum, double *coorArr, unsigned long long *pointIdArr){
	if(my_rank>=finePartNum) return;
	for(unsigned int index=0; index<triangleNum; index++){
		point p1(coorArr[index*6], coorArr[index*6+1], pointIdArr[index*3]);
		point p2(coorArr[index*6+2], coorArr[index*6+3], pointIdArr[index*3+1]);
		point p3(coorArr[index*6+4], coorArr[index*6+5], pointIdArr[index*3+2]);
//std::cout<<pointIdArr[index*3]<<" "<<pointIdArr[index*3+1]<<" "<<pointIdArr[index*3+2]<<"\n";
		triangle *newTriangle = new triangle(p1, p2, p3);
		newTriangle->computeCenterRadius();
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
}
//============================================================================
//read all triangle data for active partitions, and other partition information
void delaunayMPI::readTriangleData(unsigned int pool_size, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum){

	std::string	folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
	//read meta data for temprary files
	std::string fileInfoStr = generateFileName(coarsePartId, folderShare + dataName + "/tempTrianglesFineParts", xCoarsePartNum*yCoarsePartNum, ".xfdl");
	std::ifstream triangleInfoFile(fileInfoStr.c_str());
	if(!triangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	//read first line (1 number only) --> number of active partitions
	std::string strItem;
	triangleInfoFile >> strItem;
	finePartNum = atoi(strItem.c_str());

	//second line is active partition Ids
	finePartArr = new unsigned int[pool_size];
	for(int i=0; i<finePartNum; i++){
		triangleInfoFile >> strItem;
		finePartArr[i] = atoi(strItem.c_str());
	}

	//third line is the numbers of triangles which belong to active partitions
	triangleSizeArr = new unsigned int[pool_size];//number of triangles belong to partitions
	for(int i=0; i<pool_size; i++) triangleSizeArr[i]=0;
	totalTriangleSize = 0;
	for(int i=0; i<finePartNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
	}


	//extraTriangleNum is the fake number add up to the totalTriangleSize
	//for threads with my_rank >= finePartNum
	int extraTriangleNum = 0;
	if(finePartNum<pool_size) extraTriangleNum = pool_size - finePartNum;

	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeArr[i] will be 1 (fake)
	if(finePartNum<pool_size){
		for(int i=finePartNum; i<pool_size; i++)
			triangleSizeArr[i] = 1;
	}

	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new unsigned int[pool_size];
	for(int i=0; i<pool_size; i++) triangleSizeOffsetArr[i]=0;
	for(int i=0; i<finePartNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}
	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeOffsetArr[i] will be fake
	if(finePartNum<pool_size){
		for(int i=finePartNum; i<pool_size; i++)
			triangleSizeOffsetArr[i] = triangleSizeOffsetArr[i-1] + triangleSizeArr[i];
	}


	//fifth line is the numbers of points in each active partition
	pointNumPartArr = new unsigned int[pool_size];
	for(int i=0; i<pool_size; i++) pointNumPartArr[i]=0;
	for(int i=0; i<finePartNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartArr[i] = atoi(strItem.c_str());
	}

	//sixth line is xFinePartNum & yFinePartNum
	triangleInfoFile >> strItem;
	xFinePartNum = atoi(strItem.c_str());
	triangleInfoFile >> strItem;
	yFinePartNum = atoi(strItem.c_str());

	//seventh line is read the coordinates of current coarse partition (lowX, lowY, highX, highY)
	triangleInfoFile >> strItem;
	coarseLowX = atof(strItem.c_str());

	triangleInfoFile >> strItem;
	coarseLowY = atof(strItem.c_str());
	triangleInfoFile >> strItem;
	coarseHighX = atof(strItem.c_str());
	triangleInfoFile >> strItem;
	coarseHighY = atof(strItem.c_str());
	triangleInfoFile.close();

	xFinePartSize = (coarseHighX- coarseLowX)/xFinePartNum;
	yFinePartSize = (coarseHighY- coarseLowY)/yFinePartNum;


	//read all coordinates of triangles
	std::string dataFileStr1 = generateFileName(coarsePartId, folderShare + dataName + "/tempCoorFineParts", xCoarsePartNum*yCoarsePartNum, ".tri");
	FILE *f1 = fopen(dataFileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<dataFileStr1<<std::endl;
		return;
	}
	tempCoorArr = new double[(totalTriangleSize + extraTriangleNum)*6];
	fread(tempCoorArr, sizeof(double), totalTriangleSize*6, f1);
	fclose(f1);

	//read all point ids of triangles
	std::string dataFileStr2 = generateFileName(coarsePartId, folderShare + dataName + "/tempPointIdFineParts", xCoarsePartNum*yCoarsePartNum, ".tri");
	FILE *f2 = fopen(dataFileStr2.c_str(), "rb");
	if(!f2){
		std::cout<<"not exist "<<dataFileStr2<<std::endl;
		return;
	}
	tempPointIdArr = new unsigned long long[(totalTriangleSize + extraTriangleNum)*3];
	fread(tempPointIdArr, sizeof(unsigned long long), totalTriangleSize*3, f2);
	fclose(f2);
}

//==============================================================================
//given an index, find a bounding box of a partition in a domain
//input: + partIndex --> partition index of a partition in the domain 
//		 + lowPoint, highPoint --> the partition points
//		 + xPartNum, yPartNum --> granularity of partitions in the domain
//output: the bounding box of cuurent partition
boundingBox delaunayMPI::findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xFinePartNum, unsigned int yFinePartNum){
	double lowPointX = lowPoint.getX();
	double lowPointY = lowPoint.getY();
	double highPointX = highPoint.getX();
	double highPointY = highPoint.getY();

	double xPartSize = (highPointX - lowPointX)/xFinePartNum;
	double yPartSize = (highPointY - lowPointY)/yFinePartNum;

	unsigned int gridX = partIndex % xFinePartNum;
	unsigned int gridY = partIndex / yFinePartNum;

	point returnLowPoint(lowPointX+gridX*xPartSize, lowPointY+gridY*yPartSize);
	point returnHighPoint(returnLowPoint.getX() + xPartSize, returnLowPoint.getY() + yPartSize);
	return boundingBox(returnLowPoint, returnHighPoint);
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void delaunayMPI::triangulate(unsigned int coarsePartId, unsigned int finePartId, unsigned int my_rank){

	if((pointCoorArr==NULL) || (my_rank>=finePartNum)) return;
	unsigned int count=0;

	MPI_Get_processor_name(processor_name, &namelen);
//	std::cout<<"start triangulation from finePartId = "<<finePartId<<std::endl;

	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;
	double sweepLine = 0;

	//determine boundingbox of current partition based on finePartId
	point lowPoint(coarseLowX, coarseLowY);
	point highPoint(coarseHighX, coarseHighY);
	boundingBox currPartBBox = findPart(finePartId, lowPoint, highPoint, xFinePartNum, yFinePartNum);
//if(finePartId==10) std::cout<<"lowX: "<< currPartBBox.getLowPoint().getX() <<", lowY: " << currPartBBox.getLowPoint().getY() << "highX: " << currPartBBox.getHighPoint().getX() << "highY: " << currPartBBox.getHighPoint().getY()<<"\n"; 

//std::cout<<"coarseLowX: " + toString(coarseLowX) + ", coarseLowY: " + toString(coarseLowY) + ", coarseHighX: " + toString(coarseHighX) + ", coarseHighY: " + toString(coarseHighY)<<std::endl;


	//sequentially insert point into delaunay
	for(unsigned int localPointIndex=0; localPointIndex<pointNumPartSize; localPointIndex++){
		p = pointCoorArr[localPointIndex];

		sweepLine = p.getX();

		triangleNode *preNode = NULL;
		triangleNode *currNode = triangleList;
		bool goNext = true;
		while(currNode != NULL){
			//this triangle will never circumcircle the new point p
			if(currNode->tri->getFarestCoorX() < sweepLine){
				//if this triangle is within partition area, store it to storeTriangleList
				if(currNode->tri->inside(currPartBBox)){
 					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(storeTriangleList, exNode);
				}
				//store this triangle to temporaryTriangleList and use it for the next row
				else{
					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(temporaryTriangleList, exNode);
				}	
				goNext = false;
			}
			else//the circumcircle of this triangle can contain the new point
			if(currNode->tri->circumCircleContains(p)){
				edge *edge1 = new edge(currNode->tri->getEdge1());
				edge *edge2 = new edge(currNode->tri->getEdge2());
				edge *edge3 = new edge(currNode->tri->getEdge3());

				edgeNode *eNode1 = createNewNode(edge1);
				edgeNode *eNode2 = createNewNode(edge2);
				edgeNode *eNode3 = createNewNode(edge3);

				insertFront(polygon, eNode1);
				insertFront(polygon, eNode2);
				insertFront(polygon, eNode3);

				removeNode(triangleList, preNode, currNode);
				goNext = false;
			}

			//go to the next element in the liskList
			if(goNext){
				preNode = currNode;
				currNode = currNode->next;
			}
			goNext = true;
		}

		//Find all bad edges from polygon, then remove bad edges later
		edgeNode *currEdgeNode1 = polygon;
		edgeNode *currEdgeNode2 = polygon;
		while(currEdgeNode1!=NULL){
			while(currEdgeNode2!=NULL){
				//two different edges in polygon but same geological edge
				if((currEdgeNode1!=currEdgeNode2)&&(*(currEdgeNode1->ed) == *(currEdgeNode2->ed))){
					edge *newEdge = new edge(*currEdgeNode1->ed);
					edgeNode *newEdgeNode = createNewNode(newEdge);
					insertFront(badEdges, newEdgeNode);
				}
				currEdgeNode2 = currEdgeNode2->next;
			}
			currEdgeNode1 = currEdgeNode1->next;
			currEdgeNode2 = polygon;
		}

		//remove all bad edges from polygon
		edgeNode *badEdgeNode = badEdges;
		edgeNode *preEdgeNode = NULL;
		edgeNode *currEdgeNode = polygon;
		while(badEdgeNode!=NULL){
			while(currEdgeNode!=NULL){
				if(*(badEdgeNode->ed) == *(currEdgeNode->ed)){
					removeNode(polygon, preEdgeNode, currEdgeNode);
					break;
				}
				else{
					preEdgeNode = currEdgeNode;
					currEdgeNode = currEdgeNode->next;
				}
			}
			badEdgeNode = badEdgeNode->next;
			
			preEdgeNode = NULL;
			currEdgeNode = polygon;
		}

		//remove all bad edges
		removeLinkList(badEdges);

		//polygon now is a hole. With insert point p, form new triangles and insert into triangleList
		edgeNode *scanEdgeNode = polygon;
		while(scanEdgeNode!=NULL){
			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			newTriangle->computeCenterRadius();
			if(newTriangle->colinear()||newTriangle->isBad())
				delete newTriangle;
			else{
				triangleNode *newTriangleNode = createNewNode(newTriangle);
				insertFront(triangleList, newTriangleNode);
			}
		scanEdgeNode = scanEdgeNode->next;
		}

		//delete polygon, badTriangle
		removeLinkList(polygon);
	}
	delete [] pointCoorArr;
	std::cout<<"coarsePartId = " + toString(coarsePartId) + ", finePartId = " + toString(finePartId) + ", rank = " + toString(my_rank) + ", storeTriangleSize: " + toString(size(storeTriangleList)) + ", triangleSize: " + toString(size(triangleList)) + ", tempTriangleSize: " + toString(size(temporaryTriangleList)) + ", bad triangles:" + toString(count) + ", from node " + processor_name <<std::endl;
}

//===========================================================================
void delaunayMPI::printTriangleList(unsigned int finePartId){
	std::cout<<"triangleList size:"<<size(triangleList)<<"\n";
	std::cout<<"triangle Id List:"<<"\n";
	triangleNode *head = triangleList;
	while(head!=NULL){
		std::cout<<head->tri->p1.getX()<<" "<<head->tri->p1.getY()<<" " <<head->tri->p1.getId()<<"\n";
		std::cout<<head->tri->p2.getX()<<" "<<head->tri->p2.getY()<<" " <<head->tri->p2.getId()<<"\n";
		std::cout<<head->tri->p3.getX()<<" "<<head->tri->p3.getY()<<" " <<head->tri->p3.getId()<<"\n";
		head = head->next;
	}
}