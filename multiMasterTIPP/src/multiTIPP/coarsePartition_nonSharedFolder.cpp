#include "io.h"
#include "coarsePartition_nonSharedFolder.h"
#include <iostream>
#include <fstream>
//Domain contains multiple coarsePartitions, each coarsePartition contains multiple partitions

//===================================================================
coarsePartition::coarsePartition(unsigned int color, unsigned int partId, unsigned int activePartNum, unsigned long long initTriangleSize, unsigned int xPartNum, unsigned int yPartNum, double *infoArr, unsigned *pointNumArr, double *initPointCoorArr, unsigned long long *initPointIdArr, unsigned initPointNum, double *activeTriangleCoorArr_submaster, unsigned long long *activeTriangleIdArr_submaster, unsigned triangleNum){

	vertexRecordSize = 2;

	groupId = color;
	coarsePartId = partId;
	activeCoarsePartNum = activePartNum;
	xCoarsePartNum = xPartNum;
	yCoarsePartNum = yPartNum;
	initTriangleNum = initTriangleSize;

	pointPartInfoArr = NULL;
	initPointArr = NULL;
	triangleList = NULL;
	temporaryTriangleList = NULL;
	triangleArr = NULL;
	trianglePartitionList = NULL;

	//read info from file pointPartInfo.xfdl
	//readPointPartInfo();

	//first line --> read xFinePartNum, yFinePartNum
	xFinePartNum = infoArr[0];
	yFinePartNum = infoArr[1];

	//second line --> 4 coordinates of low point and high point of current partition
	lowPoint.setX(infoArr[2]);
	lowPoint.setY(infoArr[3]);
	highPoint.setX(infoArr[4]);
	highPoint.setY(infoArr[5]);

	geoBound.setLowPoint(lowPoint);
	geoBound.setHighPoint(highPoint);

	xGridCoarsePartSize = (highPoint.getX() - lowPoint.getX())/xFinePartNum;
	yGridCoarsePartSize = (highPoint.getY() - lowPoint.getY())/yFinePartNum;

	//third line --> number of init points
	initPointArrSize = infoArr[6];

	//fourth line --> number of points in fine-grained partitions of current coarse partition
	pointPartInfoArr = new unsigned int[xFinePartNum*yFinePartNum];
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++){
		pointPartInfoArr[i] = pointNumArr[i];
	}

	partArr = new partition[xFinePartNum*yFinePartNum];
	//disable some fine partitions if they have no points
	for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
		if(pointPartInfoArr[finePartId]==0){
			partArr[finePartId].finish = true;
			partArr[finePartId].active = false;
		}
	}

	//no need to read initPoint from files.
	initPointArrSize = initPointNum;
	initPointArr = new point[initPointArrSize];	
	//init point array
	for(unsigned i=0; i<initPointNum; i++){
		initPointArr[i].setX(initPointCoorArr[i*2]);
		initPointArr[i].setY(initPointCoorArr[i*2+1]);
		initPointArr[i].setId(initPointIdArr[i]);
	}

	initTriangleNum = triangleNum;
	//create a list of triangles
	for(unsigned long long index=0; index<initTriangleNum; index++){
		point p1(activeTriangleCoorArr_submaster[index*6], activeTriangleCoorArr_submaster[index*6+1], activeTriangleIdArr_submaster[index*3]);
		point p2(activeTriangleCoorArr_submaster[index*6+2], activeTriangleCoorArr_submaster[index*6+3], activeTriangleIdArr_submaster[index*3+1]);
		point p3(activeTriangleCoorArr_submaster[index*6+4], activeTriangleCoorArr_submaster[index*6+5], activeTriangleIdArr_submaster[index*3+2]);
		triangle *newTriangle = new triangle(p1, p2, p3);
		newTriangle->computeCenterRadius();
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
}


//==============================================================================
//triangulate without remove any triangles
//==============================================================================
void coarsePartition::basicTriangulate(point p){
	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;

	triangleNode *preNode = NULL;
	triangleNode *currNode = triangleList;
	bool goNext = true;
	double sweepLine = p.getX();

	while(currNode != NULL){

		//this triangle will never circumcircle the new point p
		if(currNode->tri->getFarestCoorX() < sweepLine){
			//store this triangle to temporaryTriangleList and use it for the next row
				triangleNode *exNode = extractNode(triangleList, preNode, currNode);
				insertFront(temporaryTriangleList, exNode);
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
			if(*(badEdgeNode->ed) == *(currEdgeNode->ed))
				removeNode(polygon, preEdgeNode, currEdgeNode);
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

//===================================================================
//return fine-grained partition Id for a point
//===================================================================
unsigned int coarsePartition::partIndex(point p){
	double x = p.getX();
	double y = p.getY();

	unsigned int gridX = x/xGridCoarsePartSize;
	if(gridX>=xFinePartNum) gridX = gridX-1;

	unsigned int gridY = y/yGridCoarsePartSize;
	if(gridY>=xFinePartNum) gridY = gridY-1;

	return gridY*xFinePartNum + gridX;
}

//===================================================================
//all points are sorted before inserting to the Delaunay Triangulation
void coarsePartition::initTriangulate(){
	//sort initExtendPointArr based on x coordinate of each point
	qsort(initPointArr, initPointArrSize, sizeof(point), coorX_comparison);

	//init triangulate
	for(unsigned long long index=0; index<initPointArrSize; index++){
		basicTriangulate(initPointArr[index]);
//std::cout<<"("<<initPointArr[index].getX()<<", "<<initPointArr[index].getY()<<", "<<initPointArr[index].getId()<<")  ";
	}

	delete [] initPointArr;
}

//=========================================================================================
//transform link list of triangles (triangleList) into array of triangle (triangleArr)
void coarsePartition::triangleTransform(){
	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	triangleArrSize	= size(triangleList);
	if(triangleArrSize>1){
		try {
			triangleArr = new triangle[triangleArrSize];
		} catch (std::bad_alloc&) {
		  // Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!coarsePartition309\n";
			exit(1);
		}

		triangleNode *scanNode = triangleList;
		unsigned long long index = 0;
		while(scanNode!=NULL){
			triangleArr[index] = *(scanNode->tri);
			index++;
			scanNode = scanNode->next;
		}	
	}
	removeLinkList(triangleList);
}

//=========================================================================================
//generate all intersections between each triangle and partitions in the coarsePartition.
//each item trianglePartitionList[i] cotains a linklist of partitionIds 
//which are the intersection with current triangleArr[i]
//input: triangle array (triangleArr), and array of list (trianglePartitionList)
//output: trianglePartitionList which contains linklist of partitions for each of triangleId
void coarsePartition::generateIntersection(){
	double globalLowX = geoBound.getLowPoint().getX();
	double globalLowY = geoBound.getLowPoint().getY();
    double gridElementSizeX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/xFinePartNum;
    double gridElementSizeY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/yFinePartNum;

//	xGridCoarsePartSize = (highPoint.getX() - lowPoint.getX())/xFinePartNum;
//	yGridCoarsePartSize = (highPoint.getY() - lowPoint.getY())/yFinePartNum;


	try {
		trianglePartitionList = new std::list<unsigned int>[triangleArrSize];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!coarsePartition364\n";
		exit(1);
	}

	//generate intersection
	for(unsigned long long index=0; index<triangleArrSize; index++){
		triangle t = triangleArr[index];
		double centerX = t.centerX;
		double centerY = t.centerY;
		double radius = t.radius;
		//Find a bounding box cover around the circle outside triangle
		boundingBox bBox(point(centerX-radius, centerY-radius), point(centerX+radius, centerY+radius));

		//gridBox that intersects with the bounding box of a triangle
		gridBound gb = boundingGrid(bBox);

		/*the coordiante of two corners of bounding grid*/
		unsigned int beginx = gb.getLowGridElement().getX();
		unsigned int beginy = gb.getLowGridElement().getY();
		unsigned int endx = gb.getHighGridElement().getX();
		unsigned int endy = gb.getHighGridElement().getY();
//std::cout<<beginx<<" "<<beginy<<" "<<endx<<" "<<endy<<"\n";
		//Scan bounding grid
		for(unsigned int i = beginy; i<=endy; i++){
			for(unsigned int j = beginx; j<=endx; j++){
				//mapping a partition element into an element in partitionElementList
				unsigned int partitionEletIdx = xFinePartNum * i + j;
if((partitionEletIdx<0)||(partitionEletIdx>=xFinePartNum*yFinePartNum)){
	std::cout<<"partitionEletIdx= "<<partitionEletIdx<<"\n";
	std::cout<<index<<" beginx: "<<beginx<<", beginy: "<<beginy<<", endx: "<<endx<<", endy: "<<endy<<"\n";
	std::cout<<t.p1<<t.p2<<t.p3<<std::endl;
	std::cout<<"centerX: "<<centerX<<", centerY: "<<centerY<<", radius: "<<radius<<std::endl;
exit(1);
}
				//if the current partition is not finish
				if(!partArr[partitionEletIdx].finish){

					//set a grid element or partition bounding box
					boundingBox partitionBbox(gridElementSizeX*j + globalLowX, gridElementSizeY*i + globalLowY, 
						gridElementSizeX*(j+1) + globalLowX, gridElementSizeY*(i+1) + globalLowY);

					//the circumcircle of triangle t will be intersected with the partition bounding box
					if(t.intersect(partitionBbox))
						//add this partition Id to the linklist partitionEletIdx
						trianglePartitionList[index].push_back(partitionEletIdx);
				}
			}
		}
	}
}

//=========================================================================================
//generate confliction for each partition. 
//Means that what partition conflicts with which partition
//input: triangleArr, trianglePartitionList
//output: array of partitions (conflictPartList). Each item in array is a linklist of conflicted partIds
//use this function ()printConflictPartitions() to print the detail
void coarsePartition::generateConflictPartitions(){
	//partTempList is an array of linklist, used to contain all conflictions of each partition in coarsePartition
	//two partitions are conflicted when exist a triangle whose circumcircle intersected with both partitions 
	try {
		conflictPartList = new std::list<unsigned int>[xFinePartNum*yFinePartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!coarsePartition432\n";
		exit(1);
	}

	//scan all triangle, for each triangle, scan all partition intersection with this triangle
	// the purpose is to find all conflictions for each partition

	for(unsigned long long index=0; index<triangleArrSize; index++){
		std::list<unsigned int> partList = trianglePartitionList[index];
		//scan each partList of a triangle, if there is one item in the list
		//means that that triangle stays inside a partition
		unsigned long long listSize = partList.size();

		std::list<unsigned int> currList = trianglePartitionList[index];
		//this triangle triangleArr[index] did not assigned to any partition
		if((!triangleArr[index].delivered)&&(listSize>1)){
			//take the first node in list, while loop
			std::list<unsigned int>::iterator it=currList.begin();
			for(std::list<unsigned int>::iterator it1=currList.begin(); it1 != currList.end(); ++it1){
				unsigned int currPartId = *it1;
				for(std::list<unsigned int>::iterator it2=currList.begin(); it2 != currList.end(); ++it2){
					unsigned int conflictPartId = *it2;
					if(conflictPartId!=currPartId) 
						conflictPartList[currPartId].push_back(conflictPartId);
				}
			}
//std::cout<<"\n";
		}		
	}

	for(int index=0; index<xFinePartNum*yFinePartNum; index++){
		conflictPartList[index].sort();
		conflictPartList[index].unique();
	}
}
//=========================================================================================
void coarsePartition::printConflictPartitions(){
	std::list<unsigned int> currList;
	//show conflictions for each partition
	for(int index=0; index<xFinePartNum*yFinePartNum; index++){
		currList = conflictPartList[index];
		if(partArr[index].finish == false){
			std::cout<<"conflicting partitions of partition "<<index<<": {";
			for (std::list<unsigned int>::iterator it=currList.begin(); it != currList.end(); ++it)
				std::cout<<(*it)<<" ";
			std::cout<<"}\n";
		}
//		std::cout<<currList.size()<<"\n";
	}
}

//=========================================================================================
//remove partitions that are finished out of the confliction list
//output: update conflictPartList
void coarsePartition::updateConflictPartitions(){
	if(currActiveList.empty()) return;

//	std::cout<<"list of fine partitions that are finished of coarsePartId: "<<coarsePartId<<" is [";
	//remove lists that are finished in conflictPartList
	for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
		int finishId = *it;
//		std::cout<<finishId<<" ";
		conflictPartList[finishId].clear();
	}
//	std::cout<<"]"<<std::endl;

	//remove finished partition in each list of conflictPartList

	for(unsigned int index=0; index<xFinePartNum*yFinePartNum; index++){
		if(partArr[index].finish==false){
			for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
				unsigned int finishId = *it;
				conflictPartList[index].remove(finishId);
			}
		}
	}
	//clean up currActiveList before the next stage: find active partitions
	currActiveList.clear();
}

//=========================================================================================
//generate active partitions in the coarsePartition: all triangles in a active partition are not intersected with other active partitions 
//means that seaprating partitions and their intersected circumcircle 
//based on the list of partitions intersected with the circumcircle of the triangles
//generate partition that are active
//input: array of list (conflictPartList)
//output: set of partitions (partIds) (activePartSet)
unsigned int coarsePartition::generateActivePartitions(){

	//Update active-inactive partitions
	//find a shortest list in partTempList, the number in order that is not in the confliced list, 
	//will be picked out as the active partition
	bool *activePartArr;
	try {
		activePartArr = new bool[xFinePartNum*yFinePartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!coarsePartition529\n";
		exit(1);
	}

	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++) activePartArr[i] = false;

	//find the shortest list in partTempList
	unsigned int activePartId;
	unsigned int index = 0;
	//find the first unfinished partition
	while(index<xFinePartNum*yFinePartNum)
		if(partArr[index].finish==false){
			activePartId = index;
			break;
		}
		else index++;


	//find the first active partition
	unsigned int unFinishSize = 1;
	//find the first active partitions based on the shorted conflicting list in conflictPartList.
	for(unsigned int index=activePartId+1; index<xFinePartNum*yFinePartNum; index++){
		if(partArr[index].finish==false){
			unFinishSize++;//count number of unfinished partitions
			//find the shortest conflicted partitions
			if(conflictPartList[activePartId].size()>conflictPartList[index].size())
				activePartId = index;
				activePartArr[activePartId] = true;
		}
	}
//std::cout<<"first active partition Id: "<<activePartId<<"\n";
//std::cout<<"unfinished Size: "<<unFinishSize<<"\n";
	//got the first active partition
	currActiveList.push_back(activePartId);


	unsigned int unFinishCount=1;//1 means found the first active partition
	//from the list of an active partition, update all conflicted partitions to partArr
	//conflictPartList is the array of linklist, each item in the array is the list of partitionIds that is conflictiong with current partitionId
	unsigned int loopCount=0;// loopCount is to check the maximum number of loop equal to number of partitions
	while(unFinishCount<unFinishSize){
		std::list<unsigned int> currList = conflictPartList[activePartId];
//		if(activePartId==0) std::cout<<activePartId<<"\n";
		for (std::list<unsigned int>::iterator it=currList.begin(); it != currList.end(); ++it){
			unsigned int conflictPartId = *it;
			if(partArr[conflictPartId].finish == false){
				if(partArr[conflictPartId].active){
					partArr[conflictPartId].active = false;
					unFinishCount++;
				}
			}
		}
		//find the next active partition Id
		//index = 0;
		//while(index<xFinePartNum*yFinePartNum)
		for(unsigned int index=0; index<xFinePartNum*yFinePartNum; index++)
			//the first item in array not updated yet
			if((partArr[index].finish==false)&&(partArr[index].active==true)&&(activePartArr[index]!=true)){
				activePartId = index;
				activePartArr[activePartId] = true;
				currActiveList.push_back(activePartId);
				unFinishCount++;
				break;
			}
			//else index++;
		loopCount++;
		if(loopCount>=xFinePartNum*yFinePartNum) break;
	}


	//print a current list of active partitions
/*	std::string msg ="<<<<<<<list of current fine active partitions of coarse partition " + toString(coarsePartId) + " is [";//currActiveList
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++)
		if((partArr[i].active)&&(!partArr[i].finish)) msg = msg + " " + toString(i) + " ";
	msg = msg + "]\n";
	std::cout<<msg;
*/
	//clean up and update
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++)
		//if a partition is unfinished and active, then set --> finish
		if(!partArr[i].finish){
			if(partArr[i].active) partArr[i].finish = true;
			else partArr[i].active = true;
		}


	//clear all item in active partition set
	if(!activePartSet.empty()) activePartSet.clear();

	//after determining active partition list, build a set of active partition Ids for later use
	if(!currActiveList.empty())
		//scan all items in currActiveList to build activePartMap	
		for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
			unsigned int activePartId = *it;
			activePartSet.insert(activePartId);
		}

	delete [] activePartArr;
	return activePartSet.size();
}

//=========================================================================================
//deliver triangles to active partitions. For each triangle, find in the list of partitions that are 
//intersected with that triangle, if exist a partition Id belong to the active partition list (currActiveList)
//then set that triangle belong to the active partition. Each triangle belong to one active partition
void coarsePartition::deliverTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnBoundaryTriangleIdArr, double *&returnBoundaryTriangleCoorArr, unsigned long long &returnBoundaryTriangleArrSize){
	std::set<unsigned int>::iterator t;

//std::cout<<"Number of triangles after initial delaunay: "<<size(triangleList)<<"\n";
//std::cout<<"Number of active partitions: "<<activePartSet.size()<<"\n";

	//storeTriangleList used to store all triangles that are not intersected with any partition (because there are some inactive partitions)
	std::list<unsigned long long> storeTriangleIdList;

	//scan all triangles, deliver triangles to active partitions
	for(unsigned long long triangleIndex=0; triangleIndex<triangleArrSize; triangleIndex++){
		std::list<unsigned int> partList = trianglePartitionList[triangleIndex];
		if(partList.empty()){//triangle is free now, need to store
			storeTriangleIdList.push_back(triangleIndex);
			triangleArr[triangleIndex].delivered = true;
		}
		else//triangle intersects with an active partition
			//scan each partition Id in partList that are intersected with a circumcircle of triangle
			for (std::list<unsigned int>::iterator it=partList.begin(); it != partList.end(); ++it){
				unsigned int partId = *it;
				t = activePartSet.find(partId);
				if (t != activePartSet.end()){//found this partId in activePartSet
					partArr[partId].triangleIdList.push_back(triangleIndex);
					//set triangle is delivered, a triangle is set belong to a partition at a time
					triangleArr[triangleIndex].delivered = true;
				}
			}
	}

	//store all point ids of triangleId in storeTriangleList to file
	unsigned long long size = storeTriangleIdList.size();
	if(size==0){
		return;
	}
	
	//storeTriangleIdList contains the triangleIds (triangles T) which do not belong to any partitions because
	//those partitions are finalized. T will have two kinds of triangles (interior and boundary triangles of a coarse partition)
	//Interior triangles (stays wholly inside coarse partition) will be stored to external memory (only store the triangle Ids)
	//Boundary triangles (intersect the border of a coarse partition) will be stored temporary for other coarse partitions.
	//Boundary triangles will be stored the whole triangle (coordinates & Ids,...)

	std::list<unsigned long long> interiorTrangleList;
	std::list<triangle> boundaryTrangleList;
	unsigned long long index = 0;
	for (std::list<unsigned long long>::iterator it=storeTriangleIdList.begin(); it != storeTriangleIdList.end(); ++it){
		unsigned long long id = *(it);//id is index of a triangle in triangleArr
		//check if the current triangle stay wholly in current coarse partition, or intersect border
		if(triangleArr[id].inside(geoBound)){// stays wholly inside
			interiorTrangleList.push_front(triangleArr[id].p1.getId());
			interiorTrangleList.push_front(triangleArr[id].p2.getId());
			interiorTrangleList.push_front(triangleArr[id].p3.getId());
		}
		else {//boundary triangles
			boundaryTrangleList.push_back(triangleArr[id]);
		}
	}

	//store storeTrangleList to returnAllStoreTriangleIdsXX.tri
	unsigned long long storeTrangleIdSize = interiorTrangleList.size();
	unsigned long long *storeTrangleIdArr = new unsigned long long[storeTrangleIdSize];
	index = 0;
	for (std::list<unsigned long long>::iterator it=interiorTrangleList.begin(); it != interiorTrangleList.end(); ++it){
		storeTrangleIdArr[index] = (*it);
		index++;
	}
	interiorTrangleList.clear();

	returnStoreTriangleIdArr = storeTrangleIdArr;
	returnStoreTriangleIdArrSize = storeTrangleIdSize/3;

	//store boundaryTrangleList to boundaryTrianglesXX.tri
	unsigned long long boundaryTrangleSize = boundaryTrangleList.size();
	returnBoundaryTriangleIdArr = new unsigned long long[boundaryTrangleSize*3];
	returnBoundaryTriangleCoorArr = new double[boundaryTrangleSize*6];


	index = 0;
	//collect trinagleIds and triangleCoors out of boundaryTrangleArr
	for (std::list<triangle>::iterator it=boundaryTrangleList.begin(); it != boundaryTrangleList.end(); ++it){
		returnBoundaryTriangleIdArr[index*3] = (*it).p1.getId();
		returnBoundaryTriangleIdArr[index*3+1] = (*it).p2.getId();
		returnBoundaryTriangleIdArr[index*3+2] = (*it).p3.getId();

		returnBoundaryTriangleCoorArr[index*6] = (*it).p1.getX();
		returnBoundaryTriangleCoorArr[index*6+1] = (*it).p1.getY();
		returnBoundaryTriangleCoorArr[index*6+2] = (*it).p2.getX();
		returnBoundaryTriangleCoorArr[index*6+3] = (*it).p2.getY();
		returnBoundaryTriangleCoorArr[index*6+4] = (*it).p3.getX();
		returnBoundaryTriangleCoorArr[index*6+5] = (*it).p3.getY();

		index++;
	}
	boundaryTrangleList.clear();

	returnBoundaryTriangleArrSize = boundaryTrangleSize;
}

//=========================================================================================
//number of active partition left over in activePartSet
unsigned int coarsePartition::activePartitionNumber(){
	return activePartSet.size();
}

//=========================================================================================
//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
//coreNum is the number of cores available in MPI system or PBS
//However, each time, send only coreNum of active partitions to slave  nodes
//Assume if total number of active partitions is 20, but number of core available is 6,
//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
//Output is the total time to run MPI
void coarsePartition::prepareDataForDelaunayMPI(unsigned int coreNum, unsigned long long *&tempPointIdArr, double *&tempCoorArr, unsigned long long &totalTriangleSize, unsigned int *&activePartIdArr, unsigned int *&activePartSizeArr, unsigned int *&activePartSizeOffsetArr, unsigned int *&pointNumPartArr, unsigned int &currActivePartNum){

	int totalActivePartNum = activePartSet.size();
	//activePartNum is current number of active partitions sending to MPI (slave nodes)
	if(coreNum<totalActivePartNum)
		currActivePartNum = coreNum;
	else currActivePartNum = totalActivePartNum;

	activePartIdArr = new unsigned int[currActivePartNum];
	activePartSizeArr = new unsigned int[currActivePartNum];//number of triangles in a partition
	activePartSizeOffsetArr = new unsigned int[currActivePartNum];
	pointNumPartArr = new unsigned int[currActivePartNum];

	unsigned int indexPartId = 0;
	totalTriangleSize = 0;

	std::list<unsigned long long> triangleIdList;
	unsigned long long triangleNum;

	//add activePartId in activePartSet to activePartIdArr
	unsigned int localIndexPartId = 0;
	for (std::set<unsigned int>::iterator it=activePartSet.begin(); it!=activePartSet.end(); ++it){
		//process each active partition
		unsigned int partId = *it;
		activePartIdArr[localIndexPartId] = partId;

		triangleIdList = partArr[partId].triangleIdList;
		//number of triangles belong to current partition
		triangleNum = triangleIdList.size();
		activePartSizeArr[localIndexPartId] = triangleNum;
		totalTriangleSize = totalTriangleSize + triangleNum;

		localIndexPartId++;
		if(localIndexPartId == currActivePartNum) break;
	}

	//take out active partId in activePartSet after add in to activePartIdArr
	for(unsigned int index=0; index<currActivePartNum; index++){
		activePartSet.erase(activePartIdArr[index]);
	}
//std::cout<<"------------ Number of active partition left over: "<<activePartSet.size()<<" ----------\n";
	//store coordinates and pointId array to tempCoor.tri and tempPointId.tri
	storeActivePartitions(activePartIdArr, currActivePartNum, tempPointIdArr, tempCoorArr, totalTriangleSize);

	//update activePartSizeOffsetArr based on activePartSizeArr
	activePartSizeOffsetArr[0]=0;
	for(unsigned int i=1; i<currActivePartNum; i++)
		activePartSizeOffsetArr[i] = activePartSizeOffsetArr[i-1]+activePartSizeArr[i-1];

	for(unsigned int i=0; i<currActivePartNum; i++) pointNumPartArr[i] = pointPartInfoArr[activePartIdArr[i]];
}

//=========================================================================================
//this function stores all triangles belong to current active partitions into files for further delaunay on cluster
//the file structure: (triangle1, triangle2, ...) --> {(p1, p2, p3), (p2, p5, p6), ....}
//each point: (x, y, Id)

//activePartIdArr contains all current active partition Ids
//activePartNum numer of current active partitions
//totalTriangleSize is number of triangles in all current active partitions (activePartNum)
void coarsePartition::storeActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum, 	unsigned long long *&tempPointIdArr, double *&tempCoorArr, unsigned long long totalTriangleSize){

	//total triangles belong to all active partitions
	//tempTriangleArr will be send to nodes using MPI

	tempCoorArr = new double[totalTriangleSize*6];//contain three point coordinates
	tempPointIdArr = new unsigned long long[totalTriangleSize*3];//contain three point Ids

	unsigned int triangleIndex = 0;

	//scan all active partitions
	for(int activePartId=0; activePartId<activePartNum; activePartId++){

		std::list<unsigned long long> triangleIdList = partArr[activePartIdArr[activePartId]].triangleIdList;
		for (std::list<unsigned long long>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			//distribute info of triangle into two arrays tempCoorArr (all point coordinates) and tempPointIdArr (point Ids)
			tempCoorArr[triangleIndex*6] = triangleArr[*it].p1.getX();
			tempCoorArr[triangleIndex*6+1] = triangleArr[*it].p1.getY();
			tempCoorArr[triangleIndex*6+2] = triangleArr[*it].p2.getX();
			tempCoorArr[triangleIndex*6+3] = triangleArr[*it].p2.getY();
			tempCoorArr[triangleIndex*6+4] = triangleArr[*it].p3.getX();
			tempCoorArr[triangleIndex*6+5] = triangleArr[*it].p3.getY();

			tempPointIdArr[triangleIndex*3] = triangleArr[*it].p1.getId();
			tempPointIdArr[triangleIndex*3+1] = triangleArr[*it].p2.getId();
			tempPointIdArr[triangleIndex*3+2] = triangleArr[*it].p3.getId();
			triangleIndex++;
		}
	}
}

//=========================================================================================
//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//two files returnAllTriangleIds.tri and returnAllTriangleCoors.tri will be used to create an array of triangles
//and add those triangles to the current main triangle array triangleArr
void coarsePartition::updateTriangleArr(unsigned long long *returnTriangleIdArr, double *returnTriangleCoorArr, unsigned long long returnTriangleIdArrSize){

	//determine number of undelivered triangles in triangleArr
	unsigned long long count=0;
	for(unsigned long long index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;

//std::cout<<">>>>>>>triangleNum: " + toString(triangleNum)<<"\n";

	//generate a new triangleArr including those triangles left over (not belong to any active partition) and
	//and return triangles that came from delaynayMPI

	//triangleNum is total number of triangles
	triangle *triangleNewArr;
	try {
		triangleNewArr = new triangle[returnTriangleIdArrSize + count];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!coarsePartition1001\n";
		exit(1);
	}

	//copy undelivered triangle from triangleArr to triangleNewArr
	unsigned long long index=0;
	for(unsigned long long i=0; i<triangleArrSize; i++)
		if(triangleArr[i].delivered==false){
			triangleNewArr[index] = triangleArr[i];
			index++;
		}

	//based on two new arrays tempCoorArr & tempPointIdArr, populate info for the rest of triangles in triangleNewArr
	for(int i=0; i<returnTriangleIdArrSize; i++){
		point p1(returnTriangleCoorArr[i*6], returnTriangleCoorArr[i*6+1], returnTriangleIdArr[i*3]);
		point p2(returnTriangleCoorArr[i*6+2], returnTriangleCoorArr[i*6+3], returnTriangleIdArr[i*3+1]);
		point p3(returnTriangleCoorArr[i*6+4], returnTriangleCoorArr[i*6+5], returnTriangleIdArr[i*3+2]);

		//assign new info
		triangleNewArr[index].delivered = false;
		triangleNewArr[index].p1.set(p1);
		triangleNewArr[index].p2.set(p2);
		triangleNewArr[index].p3.set(p3);

		triangleNewArr[index].computeCenterRadius();
		index++;
	}

	delete [] returnTriangleIdArr;
	delete [] returnTriangleCoorArr;

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++)
		if(partArr[i].finish) partArr[i].triangleIdList.clear();

	//process triangleArr
	delete [] triangleArr;
	triangleArr = triangleNewArr;

	//clean up trianglePartitionList
	for(unsigned int i=0; i<triangleArrSize; i++) trianglePartitionList[i].clear();
	delete [] trianglePartitionList;
	triangleArrSize = returnTriangleIdArrSize + count;

	//clean currActiveList
	if(!currActiveList.empty()) currActiveList.clear();

	//clean activePartSet
	if(!activePartSet.empty()) activePartSet.clear();
}


//=========================================================================================
void coarsePartition::printTriangleArray(){
	for(unsigned int i=0; i<triangleArrSize; i++)
		std::cout<<i<<" "<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p1.getId()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p2.getId()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<" "<<triangleArr[i].p3.getId()<<"\n";

	std::cout<<"triangleArrSize: "<<triangleArrSize<<"\n";

}

//=========================================================================================
//return number of partitions that are unfinished
unsigned int coarsePartition::unfinishedPartNum(){
	unsigned int count = 0;
	for(unsigned int partId=0; partId<xFinePartNum*yFinePartNum; partId++)
		if(partArr[partId].finish==false) count++;
	return count;
}

//=========================================================================================
//return number of partitions that are unfinished
unsigned int coarsePartition::unDeliveredTriangleNum(){
	unsigned int count = 0;
	for(unsigned int partId=0; partId<triangleArrSize; partId++)
		if(triangleArr[partId].delivered==false) count++;
	return count;
}

//=========================================================================================
//When all fine-grained partitions in current coarse-grained partition are processed via MPI, 
//There are two kind of triangles: circumcircles intersect with the border of current coarse-grained partition and wholly inside
//we need to store all trianlges (circumcircles wholly inside) to file, and send back to domain.
//triangles that are leftover (circumcircles intersect with partition border) will be add to triangleIds.tri
void coarsePartition::storeAllTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnBoundaryTriangleIdArr, double *&returnBoundaryTriangleCoorArr, unsigned long long &returnBoundaryTriangleArrSize){

	std::list<triangle> boundaryTriangleList;
	std::list<triangle> insideTriangleList;
	//separate trialges into 2 groups: wholly inside current coarse-grained partition, and boundary triangles
	for(unsigned long long index=0; index<triangleArrSize; index++){
		//geoBound is the bounding box of current coarse-grained partition
		if(triangleArr[index].inside(geoBound))//if triangle stay wholly inside current coarse-grained partition
			insideTriangleList.push_back(triangleArr[index]);
		else 
			boundaryTriangleList.push_back(triangleArr[index]);
	}

	//process insideTriangleList: store triangle Ids to storedTriangleIdsXX.tri
	//later domain will collect these files add directly to file triangleIds.tri
	unsigned long long storedTriangleSize = insideTriangleList.size();
	unsigned long long *tempPointIdArr = new unsigned long long[storedTriangleSize*3];
	unsigned long long index = 0;
	//copy ids of triangles from insideTriangleList to tempPointIdArr
	for(std::list<triangle>::iterator it=insideTriangleList.begin(); it!=insideTriangleList.end(); it++){
		tempPointIdArr[index*3] = (*it).p1.getId();
		tempPointIdArr[index*3+1] = (*it).p2.getId();
		tempPointIdArr[index*3+2] = (*it).p3.getId();
		index++;
	}
	insideTriangleList.clear();

	returnStoreTriangleIdArr = tempPointIdArr;
	returnStoreTriangleIdArrSize = storedTriangleSize;



	//process boundaryTriangleList: store triangle Ids to boundaryTrianglesXX.tri
	//later domain will collect these files add to the domain triangle array
	unsigned long long boundaryTriangleSize = boundaryTriangleList.size();
	returnBoundaryTriangleArrSize = boundaryTriangleSize;
	returnBoundaryTriangleIdArr = new unsigned long long[boundaryTriangleSize*3];
	returnBoundaryTriangleCoorArr = new double[boundaryTriangleSize*6];
	index = 0;
	//collect trinagleIds and triangleCoors out of boundaryTrangleArr
	for(std::list<triangle>::iterator it=boundaryTriangleList.begin(); it!=boundaryTriangleList.end(); it++){

		returnBoundaryTriangleIdArr[index*3] = (*it).p1.getId();
		returnBoundaryTriangleIdArr[index*3+1] = (*it).p2.getId();
		returnBoundaryTriangleIdArr[index*3+2] = (*it).p3.getId();

		returnBoundaryTriangleCoorArr[index*6] = (*it).p1.getX();
		returnBoundaryTriangleCoorArr[index*6+1] = (*it).p1.getY();
		returnBoundaryTriangleCoorArr[index*6+2] = (*it).p2.getX();
		returnBoundaryTriangleCoorArr[index*6+3] = (*it).p2.getY();
		returnBoundaryTriangleCoorArr[index*6+4] = (*it).p3.getX();
		returnBoundaryTriangleCoorArr[index*6+5] = (*it).p3.getY();

		index++;
	}
	boundaryTriangleList.clear();
}

//===================================================================
coarsePartition::~coarsePartition(){
	//remove conflictPartList
	for(unsigned int index=0; index<xFinePartNum*yFinePartNum; index++){
		std::list<unsigned int> currList = conflictPartList[index];
		currList.clear();
	}
	delete [] conflictPartList;

	if(!activePartSet.empty()) activePartSet.clear();

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++)
		partArr[i].triangleIdList.clear();
	delete [] partArr;

	//process triangleArr
	if(triangleArr!=NULL) delete [] triangleArr;

	delete [] pointPartInfoArr;
}

//=========================================================================================
//return a boundingGrid which inclides lower left corner and higher right corner of a boundingBox
gridBound coarsePartition::boundingGrid(boundingBox bBox){
	float gridElementSizeX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/xFinePartNum;
    float gridElementSizeY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/yFinePartNum;
//std::cout<<gridElementSizeX<<" "<<gridElementSizeY<<"\n";

    gridElement lowGridElement(mapHigh(bBox, gridElementSizeX, gridElementSizeY));
    gridElement highGridElement(mapLow(bBox, gridElementSizeX, gridElementSizeY));

    return gridBound(lowGridElement, highGridElement);
}

//=========================================================================================
/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the higher index. Ex: 2.5/2 = 2 --> column/row 0, 1, 2; 2.0/2 = 1 */
gridElement coarsePartition::mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY){
	int gridx, gridy;

	gridx = (bBox.getLowPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX;
	if(gridx < 0) gridx = 0;

	gridy = (bBox.getLowPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY;
	if(gridy < 0) gridy = 0;

//std::cout<<bBox.getLowPoint().getX()<<" "<<bBox.getLowPoint().getY()<<" "<<geoBound.getLowPoint().getX()<<"\n";

//	int gridMaxX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX -1;
//	int gridMaxY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY -1;

	//check in special case gridx and gridy greater than number of grids
//	if(gridx > gridMaxX) gridx = gridMaxX;
//	if(gridy > gridMaxY) {gridy = gridMaxY;std::cout<<"aaaaaa\n";}

    return gridElement(gridx, gridy);
}

//=========================================================================================
/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the lower index. 2.5/2 = 2 --> column/row 0, 1, 2; 4.0/2 = 2, need to be subtract by 1 --> 1
 if point p fall on the grid line (partition line) --> take lower element.*/
gridElement coarsePartition::mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY){
	unsigned int gridx, gridy;

	//-1 because start from zero
	unsigned int gridMaxX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX -1;
	unsigned int gridMaxY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY -1;

	if(bBox.getHighPoint().getX() > geoBound.getHighPoint().getX())
		gridx = gridMaxX;
	else{
		double v = (bBox.getHighPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX;
		if((v-(unsigned int)v)>0) gridx = (unsigned int)v;
		else gridx = (unsigned int)v - 1;
	}

	if(bBox.getHighPoint().getY() > geoBound.getHighPoint().getY())
		gridy = gridMaxY;
	else{
		double v = (bBox.getHighPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY;
		if((v-(unsigned int)v)>0) gridy = (unsigned int)v;
		else gridy = (int)v - 1;
	}

	return 	gridElement(gridx, gridy);
}

