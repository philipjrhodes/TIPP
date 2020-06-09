//Number of core may be larger than number of active partitions

#include "delaunayMPI_nonSharedFolder.h"

//============================================================================
delaunayMPI::delaunayMPI(unsigned int myrank, unsigned int poolsize, MPI_Comm rowcomm, unsigned int coarsePartitionId, unsigned int xCoarsePartionNum, unsigned int yCoarsePartionNum, double *pointCoorArr_submaster, unsigned long long *pointIdArr_submaster, unsigned *pointNumberArr, unsigned chunkNum){


	my_rank = myrank;
	pool_size = poolsize;
	row_comm = rowcomm;
	coarsePartId = coarsePartitionId;
	xCoarsePartNum = xCoarsePartionNum;
	yCoarsePartNum = yCoarsePartionNum;


	allPointCoorArr = pointCoorArr_submaster;//all point coordinate in current coarse partition
	allPointIdArr = pointIdArr_submaster;//all point Ids in current coarse partition
	pointNumArr = pointNumberArr;//number of points in each fine partition in curretn coarse partition
	allFineParNum = chunkNum;//number of fine partition in curretn coarse partition

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

}

//============================================================================
void delaunayMPI::processMPI(double lowX, double lowY, double highX, double highY, unsigned int xPartNum, unsigned int yPartNum, unsigned long long *inputPointIdArr, double *inputCoorArr, unsigned long long inputTriangleSize, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr, unsigned int *pointNumPartitionArr, unsigned int &currActivePartNum, unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleIdArrSize){

	int ierr;
	unsigned long long triangleSize;//number of triangles of current active partitions
	unsigned long triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long long *idArr=NULL;//all point id of triangles for receivers

	//active partition Id
	unsigned int finePartId;

	bool i_am_the_master = false;
	unsigned int root_process = 0;


	if(my_rank == MASTER_RANK){//rank in groups, SUB_MASTER_RANK
		i_am_the_master = true;

		finePartNum = currActivePartNum;
		totalTriangleSize = inputTriangleSize;
//std::cout<<"*********totalTriangleSize: "<<totalTriangleSize<<"\n";
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
//for(unsigned int i=0; i<finePartNum; i++) std::cout<<triangleSizeArr[i]<<" ";
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
		for(unsigned int i=0; i<totalTriangleSize; i++){
			tempPointIdArr[i*3] = inputPointIdArr[i*3];
			tempPointIdArr[i*3+1] = inputPointIdArr[i*3+1];
			tempPointIdArr[i*3+2] = inputPointIdArr[i*3+2];
//std::cout<<inputPointIdArr[i*3]<<" "<<inputPointIdArr[i*3+1]<<" "<<inputPointIdArr[i*3+2]<<" ";
		}

		tempCoorArr = new double[(totalTriangleSize + extraTriangleNum)*6];
		for(unsigned int i=0; i<totalTriangleSize; i++){
			tempCoorArr[i*6] = inputCoorArr[i*6];
			tempCoorArr[i*6+1] = inputCoorArr[i*6+1];
			tempCoorArr[i*6+2] = inputCoorArr[i*6+2];
			tempCoorArr[i*6+3] = inputCoorArr[i*6+3];
			tempCoorArr[i*6+4] = inputCoorArr[i*6+4];
			tempCoorArr[i*6+5] = inputCoorArr[i*6+5];
//std::cout<<inputCoorArr[i*6]<<" "<<inputCoorArr[i*6+1]<<" "<<inputCoorArr[i*6+2]<<" "<<inputCoorArr[i*6+2]<<" "<<inputCoorArr[i*6+3]<<" ";
		}
	}

	//read triangle data from file
//	if(i_am_the_master)	readTriangleData();

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

	//send active finePartId to all nodes (master , slaves)
	MPI_Scatter(finePartArr, 1, MPI_UNSIGNED, &finePartId, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	//send number of points and offsets in each active partition to all nodes (master , slaves)
	//Scatter pointNumPartArr & pointNumPartOffsetArr to pointNumPartSize & pointNumPartOffsetSize from master to all nodes
	//this command is used for readPointCoor()
	MPI_Scatter(pointNumPartArr, 1, MPI_UNSIGNED, &pointNumPartSize, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);


//	if(my_rank<finePartNum)
//		std::cout<<"finePartId: " + toString(finePartId) + ", pointNumPartSize: " + toString(pointNumPartSize)<<"\n";



	//allocate coorArr & pointIdArr for slave machines
	coorArr = new double[triangleSize*6];
	idArr = new unsigned long long[triangleSize*3];

	int *triangleSizeCoorArr;
	int *triangleSizeOffsetCoorArr;

	//update real size of triangleSizeArr, triangleSizeOffsetArr for tempCoorArr
	if(my_rank == MASTER_RANK){
		triangleSizeCoorArr = new int[pool_size];//number of coordinatesd in all triangles belong to partitions
		triangleSizeOffsetCoorArr = new int[pool_size];//number of coordinatesd in all triangles belong to partitions

		for(unsigned int process_id=0; process_id<pool_size; process_id++){		
			triangleSizeCoorArr[process_id] = triangleSizeArr[process_id]*6;
			triangleSizeOffsetCoorArr[process_id] = triangleSizeOffsetArr[process_id]*6;
		}
	}

	//distribute a portion of the array (triangleArr) to each child process
	//Each process receive a different size of triangles
	//triangleSizeArr[process_id] number of triangles (coordinates) --> send_counts
	//triangleSizeOffsetArr --> send_displacements
	MPI_Scatterv(tempCoorArr, triangleSizeCoorArr, triangleSizeOffsetCoorArr, MPI_DOUBLE, coorArr, triangleSize*6, MPI_DOUBLE, MASTER_RANK, row_comm);

	//update real size of triangleSizeArr, triangleSizeOffsetArr for tempCoorArr
	if(my_rank == MASTER_RANK){
		for(unsigned int process_id=0; process_id<pool_size; process_id++){		
			triangleSizeCoorArr[process_id] /= 2;
			triangleSizeOffsetCoorArr[process_id] /= 2;
		}
	}

	MPI_Scatterv(tempPointIdArr, triangleSizeCoorArr, triangleSizeOffsetCoorArr, MPI_UNSIGNED_LONG_LONG, idArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, row_comm);

	//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
	if(my_rank<finePartNum)
		//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
		generateTriangles(triangleSize, coorArr, idArr);
	delete [] coorArr;
	delete [] idArr;

	if(my_rank==MASTER_RANK){
		delete [] tempCoorArr;
		delete [] tempPointIdArr;
		delete [] triangleSizeArr;
		delete [] triangleSizeOffsetArr;
		delete [] triangleSizeCoorArr;
		delete [] triangleSizeOffsetCoorArr;
	}
	//after we have triangle information triangleList from generateTriangles and (pointCoorArr) from readPointCoordinates we will do delaunay/triangulate

	//read from file pointPart.ver, not parallel
//	readPointCoor(finePartId);

	//Collect coordinate points from sub-master (allPointCoorArr)
	collectPointCoor(finePartId);

	triangulate(finePartId);

	//store ids of storeTriangleList from triangulate function
	processStoreTriangles(returnStoreTriangleIdArr, returnStoreTriangleIdArrSize);

	//store triangleList & temporaryTriangleList from triangulate function
	processTriangleList(returnTriangleIdArr, returnTriangleCoorArr, returnTriangleIdArrSize);

	if(my_rank==MASTER_RANK){
		delete [] finePartArr;
		delete [] pointNumPartArr;
	}
}

//============================================================================
//process storeTriangleList (tranform into array triangle Ids and send back to master node)
void delaunayMPI::processStoreTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize){
/*	returnStoreTriangleIdArr=NULL;
	returnStoreTriangleIdArrSize = 0;
	removeLinkList(storeTriangleList);
	return;
*/

	int ierr;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;//number of triangles
	int *storeTriangleIdNumArr = NULL;//number of triangles x 3
	unsigned long long *storeTriangleIdArr = NULL;
	unsigned long long storeTriangleArrSize = 0;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	int *storeTriangleIdNumOffsetArr = NULL;


	if(my_rank==MASTER_RANK){//only master node
		storeTriangleNumArr = new unsigned int[pool_size];
		storeTriangleIdNumArr = new int[pool_size];
	}

	unsigned int triangleNum = size(storeTriangleList);
	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK){//only master node
		storeTriangleIdNumOffsetArr = new int[pool_size];
		storeTriangleIdNumOffsetArr[0] = 0;
		storeTriangleIdNumArr[0] = storeTriangleNumArr[0]*3;
		for(int i=1; i<pool_size; i++){
			storeTriangleIdNumArr[i] = storeTriangleNumArr[i]*3;
			storeTriangleIdNumOffsetArr[i] = storeTriangleIdNumOffsetArr[i-1] + storeTriangleIdNumArr[i-1];
		}


		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=0; process_id<pool_size; process_id++)
			storeTriangleArrSize += storeTriangleNumArr[process_id];

		if(storeTriangleArrSize!=0){
			//allocate memory for storeTriangleIdArr
			try {
				storeTriangleIdArr = new unsigned long long[storeTriangleArrSize*3];
			} catch (std::bad_alloc&) {
			  //Handle error
				std::cout<<"Memory overflow!!!!!!!!!!!!!\n";
				exit(1);
			}
		}
	}

	//transform storeTriangleList to array of ids
	unsigned long long *triangleIdArr;
	if(triangleNum!=0) triangleIdArr = new unsigned long long[triangleNum*3];
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

	MPI_Gatherv(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, storeTriangleIdArr, storeTriangleIdNumArr, storeTriangleIdNumOffsetArr, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK){ //only master node
		returnStoreTriangleIdArr = storeTriangleIdArr;
		returnStoreTriangleIdArrSize = storeTriangleArrSize;
	}

	if(triangleNum!=0) delete [] triangleIdArr;

	if(my_rank==MASTER_RANK){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleIdNumArr;
		delete [] storeTriangleIdNumOffsetArr;
	}
}

//============================================================================
void delaunayMPI::storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string fileStr){
	if((triangleCoorArr==NULL)||(triangleCoorArrSize==0)) return;
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
void delaunayMPI::processTriangleList(unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleArrSize){
	int ierr;

	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	unsigned long long *storeTriangleIdArr = NULL;
	double *storeTriangleCoorArr = NULL;
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;//number of triangles
	int *storeTriangleIdNumArr = NULL;//number of triangles x 3
	int *storeTriangleCoorNumArr = NULL;//number of triangles x 6

	//based on triangleNumArr, compute storeTriangleNumOffsetArr
	int *storeTriangleIdNumOffsetArr = NULL;
	int *storeTriangleCoorNumOffsetArr = NULL;
	unsigned long long storeTriangleArrSize = 0;

	//only master node
	if(my_rank==MASTER_RANK){
		storeTriangleNumArr = new unsigned int[pool_size];
		storeTriangleIdNumArr = new int[pool_size];
		storeTriangleCoorNumArr = new int[pool_size];
	}
	unsigned int triangleNum = size(triangleList);

	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, row_comm);


	if(my_rank==MASTER_RANK){//only master node
		storeTriangleIdNumOffsetArr = new int[pool_size];
		storeTriangleCoorNumOffsetArr = new int[pool_size];

		storeTriangleIdNumOffsetArr[0] = 0;
		storeTriangleCoorNumOffsetArr[0] = 0;
		storeTriangleIdNumArr[0] = storeTriangleNumArr[0]*3;
		storeTriangleCoorNumArr[0] = storeTriangleNumArr[0]*6;

		for(int i=1; i<pool_size; i++){
			storeTriangleIdNumArr[i] = storeTriangleNumArr[i]*3;
			storeTriangleCoorNumArr[i] = storeTriangleNumArr[i]*6;

			storeTriangleIdNumOffsetArr[i] = storeTriangleIdNumOffsetArr[i-1] + storeTriangleIdNumArr[i-1];
			storeTriangleCoorNumOffsetArr[i] = storeTriangleCoorNumOffsetArr[i-1] + storeTriangleCoorNumArr[i-1];
		}


		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int process_id=0; process_id<pool_size; process_id++)
			storeTriangleArrSize += storeTriangleNumArr[process_id];

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

	}

	unsigned long long *triangleIdArr = new unsigned long long[triangleNum*3];
	//this is local pointCoorArr (different from the global one)
	double *triangleCoorArr = new double[triangleNum*6];

	triangleNode *head = triangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		triangleCoorArr[index*6] = head->tri->p1.getX();
		triangleCoorArr[index*6+1] = head->tri->p1.getY();
		triangleCoorArr[index*6+2] = head->tri->p2.getX();
		triangleCoorArr[index*6+3] = head->tri->p2.getY();
		triangleCoorArr[index*6+4] = head->tri->p3.getX();
		triangleCoorArr[index*6+5] = head->tri->p3.getY();

		index++;
		head=head->next;
	}
	removeLinkList(triangleList);


	MPI_Gatherv(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, storeTriangleIdArr, storeTriangleIdNumArr, storeTriangleIdNumOffsetArr, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, row_comm);
	MPI_Gatherv(triangleCoorArr, triangleNum*6, MPI_DOUBLE, storeTriangleCoorArr, storeTriangleCoorNumArr, storeTriangleCoorNumOffsetArr, MPI_DOUBLE, MASTER_RANK, row_comm);

	if(my_rank==MASTER_RANK){
		returnTriangleIdArr = storeTriangleIdArr;
		returnTriangleCoorArr = storeTriangleCoorArr;
		returnTriangleArrSize = storeTriangleArrSize;
	}

	//release memory
	delete [] triangleIdArr;
	delete [] triangleCoorArr;

	if(my_rank==MASTER_RANK){
		delete [] storeTriangleNumArr;
		delete [] storeTriangleIdNumArr;
		delete [] storeTriangleIdNumOffsetArr;
		delete [] storeTriangleCoorNumArr;
		delete [] storeTriangleCoorNumOffsetArr;
	}
}

void delaunayMPI::testData(unsigned coarsePartId, unsigned finePartId, point *pointCoorArr, unsigned int pointNumPartSize){

	//determine boundingbox of current partition based on finePartId
	point lowPoint(coarseLowX, coarseLowY);
	point highPoint(coarseHighX, coarseHighY);
	boundingBox currFineBox = findPart(finePartId, lowPoint, highPoint, xFinePartNum, yFinePartNum);

	double lowPartPointX = currFineBox.getLowPoint().getX();
	double lowPartPointY = currFineBox.getLowPoint().getY();
	double highPartPointX = currFineBox.getHighPoint().getX();
	double highPartPointY = currFineBox.getHighPoint().getY();

	bool stop=false;
	for(unsigned int pointId=0; pointId<pointNumPartSize; pointId++){
		if((pointCoorArr[pointId].getX()>highPartPointX)||(pointCoorArr[pointId].getX()<lowPartPointX)) stop = true;
		if((pointCoorArr[pointId].getY()>highPartPointY)||(pointCoorArr[pointId].getY()<lowPartPointY)) stop = true;
	}

	//std::cout<<"done testing fine partition "<<"\n";

	if(stop){
		std::cout<<"The data in coarsePartId = "<<coarsePartId<<", finePartId = "<<finePartId<<" is wrong!!!!!!!\n";
		std::cout<<"lowPartPointX: "<<lowPartPointX<<", lowPartPointY: "<<lowPartPointX<<", highPartPointX: "<<highPartPointX<<", highPartPointY: "<<highPartPointY<<"\n";
	}
}


//============================================================================
//workers processes collect point coordinates from sub-masters 
//Depending on finePartId, read only coordinates belong to that partition
void delaunayMPI::collectPointCoor(unsigned int finePartId){
    if(my_rank>=finePartNum) return;

	unsigned *pointNumOffsetArr;
	if(my_rank == MASTER_RANK){//SUB_MASTER_RANK)
		//set offset for pointNumArr (number of points in each fine partition)
		pointNumOffsetArr = new unsigned[allFineParNum];//all fine partitions in current coarse active partition
//std::cout<<"my_rank: "+toString(my_rank)+"finePartId: "+toString(finePartId)+"\n";
		pointNumOffsetArr[0] = 0;
		for(int i=1; i<allFineParNum; i++){
			pointNumOffsetArr[i] = pointNumOffsetArr[i-1] + pointNumArr[i-1];

		}

		//set data for sub-master
		pointCoorArr = new point[pointNumPartSize];//pointNumPartSize number of points in current fine partition
		unsigned currOffset = pointNumOffsetArr[finePartArr[0]];
		for(unsigned pointId=0; pointId<pointNumPartSize; pointId++){
			pointCoorArr[pointId].setX(allPointCoorArr[(currOffset+pointId)*2]);
			pointCoorArr[pointId].setY(allPointCoorArr[(currOffset+pointId)*2+1]);
			pointCoorArr[pointId].setId(allPointIdArr[currOffset+pointId]);
		}

		//distribute coordinate points to workers
		unsigned int address;
		for(unsigned processId=1; processId<finePartNum; processId++){
			address = pointNumOffsetArr[finePartArr[processId]];
			MPI_Send(&allPointCoorArr[address*2], pointNumArr[finePartArr[processId]]*2, MPI_DOUBLE, processId, send_data_tag, row_comm);
			MPI_Send(&allPointIdArr[address], pointNumArr[finePartArr[processId]], MPI_UNSIGNED_LONG_LONG, processId, send_data_tag, row_comm);
		}
	}else{
		double *tempPointCoorArr = new double[pointNumPartSize*2];
		unsigned long long *tempPointIdArr = new unsigned long long[pointNumPartSize];

		MPI_Recv(tempPointCoorArr, pointNumPartSize*2, MPI_DOUBLE, MASTER_RANK, send_data_tag, row_comm, &status);
		MPI_Recv(tempPointIdArr, pointNumPartSize, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, row_comm, &status);

		//transform to point objects
		pointCoorArr = new point[pointNumPartSize];
		for(unsigned pointId=0; pointId<pointNumPartSize; pointId++){
			pointCoorArr[pointId].setX(tempPointCoorArr[pointId*2]);
			pointCoorArr[pointId].setY(tempPointCoorArr[pointId*2+1]);
			pointCoorArr[pointId].setId(tempPointIdArr[pointId]);
		}
		delete [] tempPointCoorArr;
		delete [] tempPointIdArr;
	}
	if(my_rank == MASTER_RANK)
		delete [] pointNumOffsetArr;
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
void delaunayMPI::triangulate(unsigned int finePartId){
	if((pointCoorArr==NULL) || (pointNumPartSize==0) || (my_rank>=finePartNum)) return;
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

	//sequentially insert point into delaunay
	for(unsigned long long localPointIndex=0; localPointIndex<pointNumPartSize; localPointIndex++){
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
			if(newTriangle->collinear()||newTriangle->isBad())
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
	//std::cout<<"coarsePartId = " + toString(coarsePartId) + ", finePartId = " + toString(finePartId) + ", rank = " + toString(my_rank) + ", storeTriangleSize: " + toString(size(storeTriangleList)) + ", triangleSize: " + toString(size(triangleList)) + ", tempTriangleSize: " + toString(size(temporaryTriangleList)) + ", bad triangles:" + toString(count) + ", from node " + processor_name <<std::endl;
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

//===========================================================================
delaunayMPI::~delaunayMPI(){
}
