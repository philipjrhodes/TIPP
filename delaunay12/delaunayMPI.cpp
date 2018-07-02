//Number of core may be larger than number of active partitions

#include "delaunayMPI.h"

//============================================================================
delaunayMPI::delaunayMPI(double dSize, std::string srcPath){
	partArr=NULL;
	pointNumPartArr=NULL;
	pointNumPartOffsetArr=NULL;

	tempCoorArr=NULL;
	tempPointIdArr=NULL;
	triangleSizeArr=NULL;
	triangleSizeOffsetArr=NULL;

	pointCoorArr=NULL;
//	pointIdArr=NULL;
	triangleList = NULL;
	temporaryTriangleList = NULL;
	storeTriangleList = NULL;

	domainSize = dSize;
	path = srcPath;
}

//============================================================================
void delaunayMPI::processMPI(int my_rank, int pool_size){

	int ierr;
	int triangleSize;//number of triangles of current active partitions
	int triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long long *idArr=NULL;//all point id of triangles for receivers

	//active partition Id
	int partId;

	bool i_am_the_master = false;
	int root_process = 0;


	if(my_rank == MASTER_RANK) i_am_the_master = true;

	//read triangle data from file
	if(i_am_the_master)	readTriangleData(pool_size);

	MPI_Barrier(MPI_COMM_WORLD);
	if(my_rank==MASTER_RANK) std::cout<<"***************MASTER SENDS INFO TO ALL WORKERS****************\n";

	//broadcast xPartNum & yPartNum to all slave nodes
	MPI_Bcast(&xPartNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&yPartNum, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//broadcast partNum to all slave nodes
	MPI_Bcast(&partNum, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//send number of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeArr, 1, MPI_INT, &triangleSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send number of offset of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeOffsetArr, 1, MPI_INT, &triangleSizeOffset, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send active partId to all nodes (master , slaves)
	MPI_Scatter(partArr, 1, MPI_INT, &partId, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send number of points and offsets in each active partition to all nodes (master , slaves)
	//Scatter pointNumPartArr & pointNumPartOffsetArr to pointNumPartSize & pointNumPartOffsetSize from master to all nodes
	//this command is used for readPointCoor()
	MPI_Scatter(pointNumPartArr, 1, MPI_INT, &pointNumPartSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Scatter(pointNumPartOffsetArr, 1, MPI_INT, &pointNumPartOffsetSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

//	MPI_Barrier(MPI_COMM_WORLD);

	if(my_rank<partNum)
		std::cout<<"partId: "<<partId<<", pointNumPartSize: "<<pointNumPartSize<<", pointNumPartOffsetSize: "<<pointNumPartOffsetSize<<"\n";

	if(my_rank==MASTER_RANK) std::cout<<"***************MASTER SENDS ACTIVE TRIANGLES TO ALL WORKERS****************\n";



	if(i_am_the_master){
		//This is the master (root) process
	    //printf("Hello from master process %d on %s of %d\n", my_rank, processor_name, pool_size);
		
        //distribute a portion of the array (triangleArr) to each child process
		for(int process_id=1; process_id<pool_size; process_id++) {
			ierr = MPI_Send(&tempCoorArr[triangleSizeOffsetArr[process_id]*6], triangleSizeArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, MPI_COMM_WORLD);
			ierr = MPI_Send(&tempPointIdArr[triangleSizeOffsetArr[process_id]*3], triangleSizeArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, MPI_COMM_WORLD);
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
		ierr = MPI_Recv(coorArr, triangleSize*6, MPI_DOUBLE, root_process, send_data_tag, MPI_COMM_WORLD, &status);
		ierr = MPI_Recv(idArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, root_process, send_data_tag, MPI_COMM_WORLD, &status);

		if(my_rank<partNum)
			//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
			generateTriangles(my_rank, triangleSize, coorArr, idArr);
		delete [] coorArr;
		delete [] idArr;
	}

	//after we have triangle information triangleList from generateTriangles and (pointCoorArr) from readPointCoordinates we will do delaunay/triangulate

	//read from file pointPart.ver, not parallel
	if(my_rank==MASTER_RANK) std::cout<<"***************WORKERS READ POINTS FOR THEIR TRIANGLULATION****************\n";
	readPointCoor(my_rank, partId);
//	MPI_Barrier(MPI_COMM_WORLD);

	if(my_rank==MASTER_RANK) std::cout<<"***************ALL WORKERS ARE TRIANGULATING****************\n";
	triangulate(partId, my_rank);

	if(my_rank==MASTER_RANK) std::cout<<"***************WORKERS ARE WRITING FINALIZED & BOUNDARY TRIANGLES BACK TO MASTER***************\n";
	//srore ids of storeTriangleList from triangulate function
	processStoreTriangles(my_rank, pool_size);

	//store triangleList & temporaryTriangleList from triangulate function
	processTriangleList(my_rank, pool_size);

	if(my_rank==MASTER_RANK){
		delete [] partArr;
		delete [] pointNumPartArr;
		delete [] pointNumPartOffsetArr;
	}
}

//============================================================================
//process storeTriangleList (tranform into array triangle Ids and send back to master node)
void delaunayMPI::processStoreTriangles(int my_rank, int pool_size){
	//number of store triangles of each process
	unsigned int *storeTriangleNumArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned int *storeTriangleNumOffsetArr = NULL;
	unsigned int storeTriangleNumOffset;


	if(my_rank==MASTER_RANK) //only master node
		storeTriangleNumArr = new unsigned int[pool_size];

	unsigned int triangleNum = size(storeTriangleList);

	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);


	if(my_rank==MASTER_RANK){//only master node
		storeTriangleNumOffsetArr = new unsigned int[pool_size];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<pool_size; i++)
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
	}
	MPI_Scatter(storeTriangleNumOffsetArr, 1, MPI_UNSIGNED, &storeTriangleNumOffset, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);


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




//compute triangle areas for each processes and gather all numbers back to master process
	double triangleAreas = 0;
	double *triangleAreaArr = NULL;

	if(my_rank==MASTER_RANK) //only master node
		triangleAreaArr = new double[pool_size];

	head = storeTriangleList;
	while(head!=NULL){
		triangleAreas += head->tri->area();
		head=head->next;
	}

	MPI_Gather(&triangleAreas, 1, MPI_DOUBLE, triangleAreaArr, 1, MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);
	//compute the total areas for all triangles in domain
	if(my_rank == MASTER_RANK){
		double totalTriangleAreas = 0;
		for(unsigned int i=0; i<pool_size; i++) 
			totalTriangleAreas += triangleAreaArr[i];
		std::cout<<"************* totalTriangleAreas: "<<totalTriangleAreas<<"\n";
	}
	delete [] triangleAreaArr;




	removeLinkList(storeTriangleList);
	std::string currPath = path + "delaunayResults/returnStoreTriangleIds.tri";
	//store array storeTriangleIds of each processes to a single files
	MPI_File_open(MPI_COMM_WORLD, currPath.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, storeTriangleNumOffset * 3 * sizeof(unsigned long long), MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, "native", MPI_INFO_NULL);
	MPI_File_write(fh, triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);
	
	delete [] triangleIdArr;

	if(my_rank==MASTER_RANK){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
	}
//if(my_rank==0) std::cout<<"store triangleNum: "<<triangleNum<<"\n";
}

//============================================================================
//process triangleList & temporaryTriangleList (tranform into array of triangles and send back to master node)
void delaunayMPI::processTriangleList(int my_rank, int pool_size){
	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	//number of store triangles of each process
	int *triangleNumArr = NULL;
	//based on triangleNumArr, compute triangleNumOffsetArr
	int *triangleNumOffsetArr = NULL;
	int triangleNumOffset;

	//only master node
	if(my_rank==MASTER_RANK) triangleNumArr = new int[pool_size];
	unsigned int triangleNum = size(triangleList);

	MPI_Gather(&triangleNum, 1, MPI_INT, triangleNumArr, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	if(my_rank==MASTER_RANK){//only master node
		triangleNumOffsetArr = new int[pool_size];
		triangleNumOffsetArr[0] = 0;
		for(int i=1; i<pool_size; i++){
			triangleNumOffsetArr[i] = triangleNumOffsetArr[i-1] + triangleNumArr[i-1];
//			std::cout<<triangleNumOffsetArr[i]<<"\n";
		}
	}
	MPI_Scatter(triangleNumOffsetArr, 1, MPI_INT, &triangleNumOffset, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);


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

	//save to single file
	std::string currPath = path+ "delaunayResults/returnTriangleIds.tri";
	//store array storeTriangleIds of each processes to a single files
	MPI_File_open(MPI_COMM_WORLD, currPath.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, triangleNumOffset * 3 * sizeof(unsigned long long), MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, "native", MPI_INFO_NULL);
	MPI_File_write(fh, triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);

	std::string currPath1 = path + "delaunayResults/returnTriangleCoors.tri";
	//store array storeTriangleIds of each processes to a single files
	MPI_File_open(MPI_COMM_WORLD, currPath1.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
	MPI_File_set_view(fh, triangleNumOffset * 6 * sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
	MPI_File_write(fh, pointCoorArr, triangleNum*6, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&fh);

	//release memory
	delete [] triangleIdArr;
	delete [] pointCoorArr;

	if(my_rank==MASTER_RANK){
		delete [] triangleNumArr;
		delete [] triangleNumOffsetArr;
	}
}

//============================================================================
//read point coordinates from pointPart.ver 
//Depending on partId, read only coordinates belong tos that partition
void delaunayMPI::readPointCoor(int my_rank, int partId){
    if(my_rank>=partNum) return;

	std::string fileStr = generateFileName(partId, path + "delaunayResults/pointPart", xPartNum*yPartNum);
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
	std::cout<<partId<<" "<<fileStr<<"\n";
	for(int i=0; i<pointNumPartSize; i++)
			std::cout<<pointCoorArr[i].getX()<<" "<<pointCoorArr[i].getY()<<" "<<pointCoorArr[i].getId()<<std::endl;
	}
*/
}

//============================================================================
//based on data received from master, each slave generate all initial triangles for the partitions
void delaunayMPI::generateTriangles(int my_rank, int triangleNum, double *coorArr, unsigned long long *pointIdArr){
	if(my_rank>=partNum) return;
	for(int index=0; index<triangleNum; index++){
		point p1(coorArr[index*6], coorArr[index*6+1], pointIdArr[index*3]);
		point p2(coorArr[index*6+2], coorArr[index*6+3], pointIdArr[index*3+1]);
		point p3(coorArr[index*6+4], coorArr[index*6+5], pointIdArr[index*3+2]);
//std::cout<<pointIdArr[index*3]<<" "<<pointIdArr[index*3+1]<<" "<<pointIdArr[index*3+2]<<"\n";
		triangle *newTriangle = new triangle(p1, p2, p3);
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
}
//============================================================================
//read all triangle data for active partitions, and other partition information
void delaunayMPI::readTriangleData(int pool_size){

	//read meta data for temprary files
	std::string fileInfoStr = path + "delaunayResults/tempTriangles.xfdl";
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
	for(int i=0; i<pool_size; i++) triangleSizeArr[i]=0;
	totalTriangleSize = 0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
	}


	//extraTriangleNum is the fake number add up to the totalTriangleSize
	//for threads with my_rank >= partNum
	int extraTriangleNum = 0;
	if(partNum<pool_size) extraTriangleNum = pool_size - partNum;

	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeArr[i] will be 1 (fake)
	if(partNum<pool_size){
		for(int i=partNum; i<pool_size; i++)
			triangleSizeArr[i] = 1;
	}


	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new unsigned long int[pool_size];
	for(int i=0; i<pool_size; i++) triangleSizeOffsetArr[i]=0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}
	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeOffsetArr[i] will be fake
	if(partNum<pool_size){
		for(int i=partNum; i<pool_size; i++)
			triangleSizeOffsetArr[i] = triangleSizeOffsetArr[i-1] + triangleSizeArr[i];
	}


	//fifth line is the numbers of points in each active partition
	pointNumPartArr = new unsigned int[pool_size];
	for(int i=0; i<pool_size; i++) pointNumPartArr[i]=0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartArr[i] = atoi(strItem.c_str());
	}
	//sixth line is the offset of previous line (point numbers of each active partition)
	//pointNumPartOffsetArr = new unsigned int[partNum];
	pointNumPartOffsetArr = new unsigned long long[pool_size];
	for(int i=0; i<pool_size; i++) pointNumPartOffsetArr[i]=0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartOffsetArr[i] = atoi(strItem.c_str());
	}
	//seventh line is xPartNum & yPartNum
	triangleInfoFile >> strItem;
	xPartNum = atoi(strItem.c_str());
	triangleInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());
	triangleInfoFile.close();


	//read all coordinates of triangles
	std::string dataFileStr1 = path + "delaunayResults/tempCoor.tri";
	FILE *f1 = fopen(dataFileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<dataFileStr1<<std::endl;
		return;
	}
	tempCoorArr = new double[(totalTriangleSize + extraTriangleNum)*6];
	fread(tempCoorArr, sizeof(double), totalTriangleSize*6, f1);
	fclose(f1);

	//read all point ids of triangles
	std::string dataFileStr2 = path + "delaunayResults/tempPointId.tri";
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
boundingBox delaunayMPI::partBox(int partId){
	double xPartSize = domainSize/xPartNum;
	double yPartSize = domainSize/yPartNum;
	int gridPartX = partId % xPartNum;
	int gridPartY = partId / yPartNum;
	point lowPoint(gridPartX*xPartSize, gridPartY*yPartSize);
	point highPoint((gridPartX+1)*xPartSize, (gridPartY+1)*yPartSize);
	return boundingBox(lowPoint, highPoint);
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void delaunayMPI::triangulate(unsigned int partId, int my_rank){

	if((pointCoorArr==NULL) || (my_rank>=partNum)) return;
	unsigned int count=0;

	MPI_Get_processor_name(processor_name, &namelen);
//	std::cout<<"start triangulation from partId = "<<partId<<std::endl;

	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;
	double sweepLine = 0;

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);


	//sequentially insert point into delaunay
	for(unsigned int localPointIndex=0; localPointIndex<pointNumPartSize; localPointIndex++){
//		p.setX(pointCoorArr[localPointIndex].getX());
//		p.setY(pointCoorArr[localPointIndex].getY());
//		p.setId(pointCoorArr[localPointIndex].getId());
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
	std::cout<<"done triangulation with partId = "<<partId<<", rank = "<<my_rank<<", storeTriangleSize: "<<size(storeTriangleList)<<", triangleSize: "<<
size(triangleList)<<", tempTriangleSize: "<<size(temporaryTriangleList)<<", bad triangles:"<<count<<", from node "<<processor_name<<std::endl;
}

//===========================================================================
void delaunayMPI::printTriangleList(int partId){
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



