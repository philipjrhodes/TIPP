#include "common.h"
#include "coarsePartition.h"
#include <iostream>
#include <fstream>
//Domain contains multiple coarsePartitions, each coarsePartition contains multiple partitions

//===================================================================
coarsePartition::coarsePartition(unsigned int color, unsigned int partId, unsigned int activePartNum, unsigned int initTriangleSize, unsigned int xPartNum, unsigned int yPartNum, std::string pathStr){

	path = pathStr;
	vertexRecordSize = 2;

	groupId = color;
	coarsePartId = partId;
	activeCoarsePartNum = activePartNum;
	xCoarsePartNum = xPartNum;
	yCoarsePartNum = yPartNum;
	initTriangleNum = initTriangleSize;

	//read init triangle info
//	readInitTriangleInfo();
	pointPartInfoArr = NULL;
	initPointArr = NULL;
	triangleList = NULL;
	temporaryTriangleList = NULL;
	triangleArr = NULL;
	trianglePartitionList = NULL;

	//read info from file pointPartInfo.xfdl
	readPointPartInfo();
std::cout<<"groupId: " + toString(groupId) + ", activeCoarsePartNum: "+ toString(activeCoarsePartNum) + ", initTriangleNum: " + toString(initTriangleNum)<<std::endl;

	partArr = new partition[xFinePartNum*yFinePartNum];

/*	//remove triangleIds.tri
	std::string delCommand = "rm " + path + "delaunayResults/triangleIds.tri";
	system(delCommand.c_str());
*/
}
/*
//===================================================================
//read info in file tempTriangles.xfdl after each stage of the domain
//the numbner of groupIds will be equal to the number of current active coarse partitions in tempTriangles.xfdl
//a groupId consists of an active coarse partition and all points, triangles belong to for further triangulation
//This function aims to get the corase partition: (Id, number of init triangles, )
//===================================================================
void coarsePartition::readInitTriangleInfo(){
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

	unsigned int *activeCoarsePartIdArr = new unsigned int[activeCoarsePartNum];
	//second line: read active partition ids (coarsePartition Ids)
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}
	//current partId of coarse partition
	coarsePartId = activeCoarsePartIdArr[groupId];
	delete [] activeCoarsePartIdArr;

	unsigned int *activeCoarsePartSizeArr = new unsigned int[activeCoarsePartNum];
	//third line stores number of init triangles belong to active partitions (coarsePartitions)
	//take number of init triangles of current coarse partition in the array of active coarse partitions
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartSizeArr[i] = atoi(strItem.c_str());
	}
	initTriangleNum = activeCoarsePartSizeArr[groupId];
	delete [] activeCoarsePartSizeArr;

	//fourth line: read coarse-grained partition sizes 
	initTriangleInfoFile >> strItem;
	xCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile >> strItem;
	yCoarsePartNum = atoi(strItem.c_str());

	initTriangleInfoFile.close();
}
*/
//===================================================================
//read info about the current coarse partition: 
//===================================================================
void coarsePartition::readPointPartInfo(){

	//Read information from pointPartInfoXX.xfdl
	std::string fileInfoStr = generateFileName(coarsePartId, path + "delaunayResults/pointPartInfo", xCoarsePartNum*yCoarsePartNum, ".xfdl");
	std::ifstream partInfoFile(fileInfoStr.c_str());
	if(!partInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first line --> read xFinePartNum, yFinePartNum
	partInfoFile >> strItem;
	xFinePartNum = atoi(strItem.c_str());
	partInfoFile >> strItem;
	yFinePartNum = atoi(strItem.c_str());

	//second line --> 4 coordinates of low point and high point of current partition
	partInfoFile >> strItem;
	lowPoint.setX(std::stod(strItem.c_str()));
	partInfoFile >> strItem;
	lowPoint.setY(std::stod(strItem.c_str()));
	partInfoFile >> strItem;
	highPoint.setX(std::stod(strItem.c_str()));
	partInfoFile >> strItem;
	highPoint.setY(std::stod(strItem.c_str()));

	geoBound.setLowPoint(lowPoint);
	geoBound.setHighPoint(highPoint);

	xGridCoarsePartSize = (highPoint.getX() - lowPoint.getX())/xFinePartNum;
	yGridCoarsePartSize = (highPoint.getY() - lowPoint.getY())/yFinePartNum;


	//third line --> number of init points
	partInfoFile >> strItem;
	initPointArrSize = atoi(strItem.c_str());

	//fourth line --> number of points in fine-grained partitions of current coarse partition
	pointPartInfoArr = new unsigned int[xFinePartNum*yFinePartNum];
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++){
		partInfoFile >> strItem;
		pointPartInfoArr[i] = atoi(strItem.c_str());
	}
	partInfoFile.close();


//for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++) std::cout<<pointPartInfoArr[i]<<" ";

//std::cout<<lowPoint.getX()<<" "<<lowPoint.getY()<<" "<<highPoint.getX()<<" "<<highPoint.getY()<<std::endl;

//std::cout<<"lowPointX: " + toString(lowPoint.getX()) + ", lowPointY: " + toString(lowPoint.getY()) + ", highPointX: " + toString(highPoint.getX()) + ", highPointY: " + toString(highPoint.getY()) + ", xFinePartNum: " + toString(xFinePartNum) + ", yFinePartNum: " + toString(yFinePartNum)<<std::endl;
}

//==============================================================================
//init points are generated by distribute.cpp for current coarse partition
//==============================================================================
void coarsePartition::loadInitPoints(){
	std::string fileStr = generateFileName(coarsePartId, path + "delaunayResults/initPointPart", xCoarsePartNum*yCoarsePartNum, ".ver");
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	initPointArr = new point[initPointArrSize];
	fread(initPointArr, sizeof(point), initPointArrSize, f);
	fclose(f);
}

//==============================================================================
//init triangles are generated from a stage of the domain
//these triangles have circumcircles intersecting with current coarse partition
//==============================================================================
void coarsePartition::loadInitTriangles(){
	std::string fileStr = generateFileName(groupId, path + "delaunayResults/tempCoorCoarseParts", activeCoarsePartNum, ".tri");
	FILE *f1 = fopen(fileStr.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	double *tempCoorArr = new double[initTriangleNum*6];
	fread(tempCoorArr, sizeof(double), initTriangleNum*6, f1);
	fclose(f1);

	//read all point ids of triangles
	fileStr = generateFileName(groupId, path + "delaunayResults/tempPointIdCoarseParts", activeCoarsePartNum, ".tri");
	FILE *f2 = fopen(fileStr.c_str(), "rb");
	if(!f2){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	unsigned long long *tempPointIdArr = new unsigned long long[initTriangleNum*3];
	fread(tempPointIdArr, sizeof(unsigned long long), initTriangleNum*3, f2);
	fclose(f2);

	//create a list of triangles
	for(int index=0; index<initTriangleNum; index++){
		point p1(tempCoorArr[index*6], tempCoorArr[index*6+1], tempPointIdArr[index*3]);
		point p2(tempCoorArr[index*6+2], tempCoorArr[index*6+3], tempPointIdArr[index*3+1]);
		point p3(tempCoorArr[index*6+4], tempCoorArr[index*6+5], tempPointIdArr[index*3+2]);
//std::cout<<tempPointIdArr[index*3]<<" "<<tempPointIdArr[index*3+1]<<" "<<tempPointIdArr[index*3+2]<<"\n";
		triangle *newTriangle = new triangle(p1, p2, p3);
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
	delete [] tempCoorArr;
	delete [] tempPointIdArr;

//	printLinkList(triangleList);

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

//===================================================================
//return fine-grained partition Id for a point
//===================================================================
int coarsePartition::partIndex(point p){
	double x = p.getX();
	double y = p.getY();

	int gridX = x/xGridCoarsePartSize;
	if(gridX>=xFinePartNum) gridX = gridX-1;

	int gridY = y/yGridCoarsePartSize;
	if(gridY>=xFinePartNum) gridY = gridY-1;

	return gridY*xFinePartNum + gridX;
}

/*//===================================================================
int coarsePartition::coorX_comparison(const void *p1, const void *p2)
{
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}
*/
//===================================================================
//all points are sorted before inserting to the Delaunay Triangulation
void coarsePartition::initTriangulate(){
	//sort initExtendPointArr based on x coordinate of each point
	qsort(initPointArr, initPointArrSize, sizeof(point), coorX_comparison);

	//init triangulate
	for(unsigned int index=0; index<initPointArrSize; index++){
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
			std::cout<<"Memory overflow!!!!!!!!!!!\n";
			exit(1);
		}

		triangleNode *scanNode = triangleList;
		unsigned int index = 0;
		while(scanNode!=NULL){
			triangleArr[index] = *(scanNode->tri);
			index++;
			scanNode = scanNode->next;
		}	
	}
	removeLinkList(triangleList);

	//store triangleArr coordinates for visualization
	double *coorTriangleArr = new double[triangleArrSize*6];
	int index = 0;
//std::cout<<"triangleArrSize: "<<triangleArrSize<<std::endl;
	for(int i=0; i<triangleArrSize; i++){
		coorTriangleArr[index*6] = triangleArr[i].p1.getX();
		coorTriangleArr[index*6+1] = triangleArr[i].p1.getY();
		coorTriangleArr[index*6+2] = triangleArr[i].p2.getX();
		coorTriangleArr[index*6+3] = triangleArr[i].p2.getY();
		coorTriangleArr[index*6+4] = triangleArr[i].p3.getX();
		coorTriangleArr[index*6+5] = triangleArr[i].p3.getY();
		index++;
	}
	std::string fileStr = generateFileName(groupId, path + "delaunayResults/initTrianglesCoors", activeCoarsePartNum, ".tri");
	FILE *f = fopen(fileStr.c_str(), "wb");
	fwrite(coorTriangleArr, sizeof(double), triangleArrSize*6, f);
	fclose(f);
	
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
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	//generate intersection
	for(unsigned int index=0; index<triangleArrSize; index++){
		triangle t = triangleArr[index];
		double centerX = t.centerX;
		double centerY = t.centerY;
		double radius = t.radius;
		//Find a bounding box cover around the circle outside triangle
		boundingBox bBox(point(centerX-radius, centerY-radius), point(centerX+radius, centerY+radius));

		//gridBox that intersects with the bounding box of a triangle
		gridBound gb = boundingGrid(bBox);

		/*the coordiante of two corners of bounding grid*/
		int beginx = gb.getLowGridElement().getX();
		int beginy = gb.getLowGridElement().getY();
		int endx = gb.getHighGridElement().getX();
		int endy = gb.getHighGridElement().getY();
//std::cout<<beginx<<" "<<beginy<<" "<<endx<<" "<<endy<<"\n";
		//Scan bounding grid
		for(int i = beginy; i<=endy; i++){
			for(int j = beginx; j<=endx; j++){
				//mapping a partition element into an element in partitionElementList
				int partitionEletIdx = xFinePartNum * i + j;
if((partitionEletIdx<0)||(partitionEletIdx>=xFinePartNum*yFinePartNum)){
	std::cout<<"partitionEletIdx= "<<partitionEletIdx<<"\n";
	std::cout<<index<<" beginx: "<<beginx<<", beginy: "<<beginy<<", endx: "<<endx<<", endy: "<<endy<<"\n";
	std::cout<<t.p1<<t.p2<<t.p3<<std::endl;
	std::cout<<"centerX: "<<centerX<<", centerY: "<<centerY<<", radius: "<<radius<<std::endl;
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

std::cout<<"====================================================\n";

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
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	//scan all triangle, for each triangle, scan all partition intersection with this triangle
	// the purpose is to find all conflictions for each partition

	for(unsigned int index=0; index<triangleArrSize; index++){
		std::list<unsigned int> partList = trianglePartitionList[index];
		//scan each partList of a triangle, if there is one item in the list
		//means that that triangle stays inside a partition
		int listSize = partList.size();

		std::list<unsigned int> currList = trianglePartitionList[index];
		//this triangle triangleArr[index] did not assigned to any partition
		if((!triangleArr[index].delivered)&&(listSize>1)){
			//take the first node in list, while loop
			std::list<unsigned int>::iterator it=currList.begin();
			for(std::list<unsigned int>::iterator it1=currList.begin(); it1 != currList.end(); ++it1){
				int currPartId = *it1;
				for(std::list<unsigned int>::iterator it2=currList.begin(); it2 != currList.end(); ++it2){
					int conflictPartId = *it2;
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

	std::cout<<"list of partitions that are finished: [";
	//remove lists that are finished in conflictPartList
	for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
		int finishId = *it;
		std::cout<<finishId<<" ";
		conflictPartList[finishId].clear();
	}
	std::cout<<"]"<<std::endl;

	//remove finished partition in each list of conflictPartList

	for(int index=0; index<xFinePartNum*yFinePartNum; index++){
		if(partArr[index].finish==false){
			for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
				int finishId = *it;
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
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	for(int i=0; i<xFinePartNum*yFinePartNum; i++) activePartArr[i] = false;

	//find the shortest list in partTempList
	int activePartId;
	int index = 0;
	//find the first unfinished partition
	while(index<xFinePartNum*yFinePartNum)
		if(partArr[index].finish==false){
			activePartId = index;
			break;
		}
		else index++;


	//find the first active partition
	int unFinishSize = 1;
	//find the first active partitions based on the shorted conflicting list in conflictPartList.
	for(int index=activePartId+1; index<xFinePartNum*yFinePartNum; index++){
		if(partArr[index].finish==false){
			unFinishSize++;//count number of unfinished partitions
			//find the shortest conflicted partitions
			if(conflictPartList[activePartId].size()>conflictPartList[index].size())
				activePartId = index;
				activePartArr[activePartId] = true;
		}
	}
std::cout<<"first active partition Id: "<<activePartId<<"\n";
std::cout<<"unfinished Size: "<<unFinishSize<<"\n";
	//got the first active partition
	currActiveList.push_back(activePartId);


	int unFinishCount=1;//1 means found the first active partition
	//from the list of an active partition, update all conflicted partitions to partArr
	//conflictPartList is the array of linklist, each item in the array is the list of partitionIds that is conflictiong with current partitionId
	unsigned int loopCount=0;// loopCount is to check the maximum number of loop equal to number of partitions
	while(unFinishCount<unFinishSize){
		std::list<unsigned int> currList = conflictPartList[activePartId];
//		if(activePartId==0) std::cout<<activePartId<<"\n";
		for (std::list<unsigned int>::iterator it=currList.begin(); it != currList.end(); ++it){
			int conflictPartId = *it;
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
		for(int index=0; index<xFinePartNum*yFinePartNum; index++)
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
	std::cout<<"list of current active partitions: [";//currActiveList
	for(int i=0; i<xFinePartNum*yFinePartNum; i++)
		if((partArr[i].active)&&(!partArr[i].finish)) std::cout<<i<<" ";
	std::cout<<"]\n";

	//clean up and update
	for(int i=0; i<xFinePartNum*yFinePartNum; i++)
		//if a partition is unfinished and active, then set --> finish
		if(!partArr[i].finish){
			if(partArr[i].active) partArr[i].finish = true;
			else partArr[i].active = true;
		}

	//print a current list of unfinished partitions
	std::cout<<"list of unfinished partitions: [";
	for(int i=0; i<xFinePartNum*yFinePartNum; i++)
		if(!partArr[i].finish) std::cout<<i<<" ";
	std::cout<<"]\n\n";



	//clear all item in active partition set
	if(!activePartSet.empty()) activePartSet.clear();

	//after determining active partition list, build a set of active partition Ids for later use
	if(!currActiveList.empty())
		//scan all items in currActiveList to build activePartMap	
		for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
			int activePartId = *it;
			activePartSet.insert(activePartId);
		}

	delete [] activePartArr;
	return activePartSet.size();
}

//=========================================================================================
//deliver triangles to active partitions. For each triangle, find in the list of partitions that are 
//intersected with that triangle, if exist a partition Id belong to the active partition list (currActiveList)
//then set that triangle belong to the active partition. Each triangle belong to one active partition
void coarsePartition::deliverTriangles(){
	std::set<unsigned int>::iterator t;

//std::cout<<"Number of triangles after initial delaunay: "<<size(triangleList)<<"\n";
std::cout<<"Number of active partitions: "<<activePartSet.size()<<"\n";

	//storeTriangleList used to store all triangles that are not intersected with any partition (because there are some inactive partitions)
	std::list<unsigned int> storeTriangleIdList;

	//scan all triangles, deliver triangles to active partitions
	for(unsigned int triangleIndex=0; triangleIndex<triangleArrSize; triangleIndex++){
		std::list<unsigned int> partList = trianglePartitionList[triangleIndex];
		if(partList.empty()){//triangle is free now, need to store
			storeTriangleIdList.push_back(triangleIndex);
			triangleArr[triangleIndex].delivered = true;
		}
		else//triangle intersects with an active partition
			//scan each partition Id in partList that are intersected with a circumcircle of triangle
			for (std::list<unsigned int>::iterator it=partList.begin(); it != partList.end(); ++it){
				int partId = *it;
				t = activePartSet.find(partId);
				if (t != activePartSet.end()){//found this partId in activePartSet
					partArr[partId].triangleIdList.push_back(triangleIndex);
					//set triangle is delivered, a triangle is set belong to a partition at a time
					triangleArr[triangleIndex].delivered = true;
				}
			}
	}

	//store all point ids of triangleId in storeTriangleList to file
	unsigned int size = storeTriangleIdList.size();
	if(size==0){
//std::cout<<"triangleIdList is empty\n";
		return;
	}
	
	unsigned long long *data;
	try {
		data = new unsigned long long[size*3];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}
	unsigned int index = 0;
	for (std::list<unsigned int>::iterator it=storeTriangleIdList.begin(); it != storeTriangleIdList.end(); ++it){
		unsigned int id = *(it);//id is index of a triangle in triangleArr
		data[index*3] = triangleArr[id].p1.getId();
		data[index*3+1] = triangleArr[id].p2.getId();
		data[index*3+2] = triangleArr[id].p3.getId();
		index++;
	}
	//store data to file
	std::string fileStr = generateFileName(coarsePartId, "storedTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	//std::string fileStr = path + "delaunayResults/triangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	fwrite(data, size*3, sizeof(unsigned long long), f);
	fclose(f);

	delete [] data;
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
void coarsePartition::prepareDataForDelaunayMPI(unsigned int coreNum){
	unsigned int *activePartIdArr;
	unsigned int *activePartSizeArr;
	unsigned int *activePartSizeOffsetArr;
	unsigned int currActivePartNum;

	int totalActivePartNum = activePartSet.size();
	//activePartNum is current number of active partitions sending to MPI (slave nodes)
	if(coreNum<totalActivePartNum)
		currActivePartNum = coreNum;
	else currActivePartNum = totalActivePartNum;

	//array of active partition Ids 
	try {
		activePartIdArr = new unsigned int[currActivePartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	try {
		activePartSizeArr = new unsigned int[currActivePartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	try {
		activePartSizeOffsetArr = new unsigned int[currActivePartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	int indexPartId = 0;
	int totalTriangleSize = 0;
//	double MPITime, totalMPITime = 0;

	std::list<unsigned int> triangleIdList;
	unsigned int triangleNum;

	//add activePartId in activePartSet to activePartIdArr
	unsigned int localIndexPartId = 0;
	for (std::set<unsigned int>::iterator it=activePartSet.begin(); it!=activePartSet.end(); ++it){
		//process each active partition
		int partId = *it;
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
	storeActivePartitions(activePartIdArr, currActivePartNum, totalTriangleSize);

	std::cout<<"total active TriangleSize: "<<totalTriangleSize<<"\n";
	//update activePartSizeOffsetArr based on activePartSizeArr
	activePartSizeOffsetArr[0]=0;
	for(unsigned int i=1; i<currActivePartNum; i++)
		activePartSizeOffsetArr[i] = activePartSizeOffsetArr[i-1]+activePartSizeArr[i-1];

	//store info of active partitions to tempTriangles.xfdl
	storeActivePartitionInfo(currActivePartNum, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr);

	delete [] activePartIdArr;
	delete [] activePartSizeArr;
	delete [] activePartSizeOffsetArr;
}

//=========================================================================================
//this function stores all triangles belong to current active partitions into files for further delaunay on cluster
//the file structure: (triangle1, triangle2, ...) --> {(p1, p2, p3), (p2, p5, p6), ....}
//each point: (x, y, Id)

//activePartIdArr contains all current active partition Ids
//activePartNum numer of current active partitions
//totalTriangleSize is number of triangles in all current active partitions (activePartNum)
void coarsePartition::storeActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum, unsigned int totalTriangleSize){
std::cout<<"totalTriangleSize: " <<totalTriangleSize<<"\n";
	//total triangles belong to all active partitions
	//tempTriangleArr will be send to nodes using MPI
	double *tempCoorArr;
	try {
		tempCoorArr = new double[totalTriangleSize*6];//contain three point coordinates
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	unsigned long long *tempPointIdArr;
	try {
		tempPointIdArr = new unsigned long long[totalTriangleSize*3];//contain three point Ids
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	int triangleIndex = 0;

	//scan all active partitions
	for(int activePartId=0; activePartId<activePartNum; activePartId++){

//		std::cout<<"list of trangle Ids that are belong to active partitions: \n";
//		std::cout<<"partition Id: "<<activePartIdArr[activePartId]<<"\n";
//		std::cout<<"triangle Id list: ";

		std::list<unsigned int> triangleIdList = partArr[activePartIdArr[activePartId]].triangleIdList;
		for (std::list<unsigned int>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
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
/*
std::cout<<tempPointIdArr[triangleIndex*3]<<" "<<tempPointIdArr[triangleIndex*3+1]<<" "<<tempPointIdArr[triangleIndex*3+2]<<"========"<<tempCoorArr[triangleIndex*6]<<" "<<tempCoorArr[triangleIndex*6+1]<<" "<<tempCoorArr[triangleIndex*6+2]<<" "<<tempCoorArr[triangleIndex*6+3]<<" "<<tempCoorArr[triangleIndex*6+4]<<" "<<tempCoorArr[triangleIndex*6+5]<<"\n";
*/

			triangleIndex++;
//			std::cout<<(*it)<<" ";
		}
//		std::cout<<"\n\n";
	}

std::cout<<"number of triangles sends to MPI nodes: "<<totalTriangleSize<<"\n";

	//store tempCoorArr to file tempCoorFineParts.tri
//	std::string fileStr = path + "delaunayResults/tempCoorFineParts.tri";
	std::string fileStr = generateFileName(coarsePartId, path + "delaunayResults/tempCoorFineParts", xCoarsePartNum*yCoarsePartNum, ".tri");
	FILE *f1 = fopen(fileStr.c_str(), "wb");
	if(!f1){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(tempCoorArr, totalTriangleSize*6, sizeof(double), f1);
	fclose(f1);

	//store tempPointIdArr to file tempPointIdFineParts.tri
	std::string fileStr2 = generateFileName(coarsePartId, path + "delaunayResults/tempPointIdFineParts", xCoarsePartNum*yCoarsePartNum, ".tri");
//	std::string fileStr2 = path + "delaunayResults/tempPointIdFineParts.tri";
	FILE *f2 = fopen(fileStr2.c_str(), "wb");
	if(!f2){
		std::cout<<"not exist "<<fileStr2<<std::endl;
		return;
	}
	fwrite(tempPointIdArr, totalTriangleSize*3, sizeof(unsigned long long), f2);
	fclose(f2);
	delete [] tempCoorArr;
	delete [] tempPointIdArr;
}

//=========================================================================================
//store info of active partitions to tempTrianglesFineParts.xfdl
//The info consists of:
//	- number of current active partitions, 
//	- active partition ids
//	- number of triangles belong to active partitions
//	- offsets of number of triangles
//	- number of new points (not inserted yet) in each active partition
//	- offset of number of new points
//	- xFinePartNum, yFinePartNum --> subPartition size
//	- startIds for all active partitions
void coarsePartition::storeActivePartitionInfo(unsigned int activePartNum, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr){
	//stores meta data
	std::string fileInfoStr = generateFileName(coarsePartId, path + "delaunayResults/tempTrianglesFineParts", xCoarsePartNum*yCoarsePartNum, ".xfdl");
//	std::string fileInfoStr = path + "delaunayResults/" + "tempTrianglesFineParts.xfdl";
	std::ofstream infoFile(fileInfoStr, std::ofstream::out);
	//first line store number of active partition
	infoFile<<activePartNum<<"\n";
	//second line stores active partition ids
	for(unsigned int i=0; i<activePartNum; i++) infoFile<<activePartIdArr[i]<<" ";
	infoFile<<"\n";
	//third line stores number of triangles belong to active partitions
	for(unsigned int i=0; i<activePartNum; i++) infoFile<<activePartSizeArr[i]<<" ";
	infoFile<<"\n";
	//fourth line stores offsets of third line (number of triangles)
	for(unsigned int i=0; i<activePartNum; i++) infoFile<<activePartSizeOffsetArr[i]<<" ";
	infoFile<<"\n";

	//fifth line stores number of points in each active partition
	for(unsigned int i=0; i<activePartNum; i++) infoFile<<pointPartInfoArr[activePartIdArr[i]]<<" ";
	infoFile<<"\n";

	//sixth line stores xFinePartNum, yFinePartNum
	infoFile<<xFinePartNum<<" "<<yFinePartNum<<"\n";

	//seventh line is stored the coordinates of coarse partition (lowX, lowY, highX, highY)
	infoFile<<lowPoint.getX()<<" "<<lowPoint.getY()<<" "<<highPoint.getX()<<" "<<highPoint.getY()<<"\n";

	infoFile.close();
}

//=========================================================================================
void coarsePartition::addFile(std::string path, std::string fileName1, std::string fileName2){
	std::string command;
	command = "cat " + path + "delaunayResults/" + fileName1 + " >> " + path + "delaunayResults/" + fileName2;
	system(command.c_str());
	command = "rm " + path + "delaunayResults/" + fileName1;
	system(command.c_str());
}

//=========================================================================================
//add return_triangles from MPI to returnAllTriangleIdsXX.tri and returnAllTriangleCoorsXX.tri
//and merge to current triangleArr in memory
void coarsePartition::addReturnTriangles(){

	std::string returnStoreName = generateFileName(coarsePartId, "returnStoreTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	std::string returnAllStoreName = generateFileName(coarsePartId, "returnAllStoreTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	//add triangles in returnStoreTriangleIdsXX.tri to file returnAllStoreTriangleIdsXX.tri, 
	addFile(path, returnStoreName, returnAllStoreName);

	std::string returnTriangleIdsName = generateFileName(coarsePartId, "returnTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	std::string returnAllTriangleIdsName = generateFileName(coarsePartId, "returnAllTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	//add triangles in returnTriangleIdsXX.tri to file returnAllTriangleIdsXX.tri, 
	addFile(path, returnTriangleIdsName, returnAllTriangleIdsName);

	std::string returnTriangleCoorsName = generateFileName(coarsePartId, "returnTriangleCoors", xCoarsePartNum*yCoarsePartNum, ".tri");
	std::string returnAllTriangleCoorsName = generateFileName(coarsePartId, "returnAllTriangleCoors", xCoarsePartNum*yCoarsePartNum, ".tri");
	//and add returnTriangleCoorsXX.tri to file returnAllTriangleCoorsXX.tri
	addFile(path, returnTriangleCoorsName, returnAllTriangleCoorsName);
}

//=========================================================================================
//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//two files returnTriangleIds.tri and returnTriangleCoors.tri will be used to create an array of triangles
//and add those triangles to the current main triangle array triangleArr
void coarsePartition::updateTriangleArr(){

	//determine number of undelivered triangles in triangleArr
	unsigned int count=0;
	for(unsigned int index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;
//std::cout<<"undelivered triangles: "<<count<<"\n";
	//determine number of triangles that came from delaunayMPI

	//read triangles from two files returnTriangleCoors.tri, returnTriangleIds.tri 
	//and add to the remaining trianglesList
	std::string fileStr = generateFileName(coarsePartId, path + "delaunayResults/returnAllTriangleCoors", xCoarsePartNum*yCoarsePartNum, ".tri");
//	std::string fileStr = path + "delaunayResults/returnAllTriangleCoors.tri";

	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(6*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
std::cout<<"triangleNum: "<<triangleNum<<"\n";

	double *tempCoorArr;
	try {
		tempCoorArr = new double[triangleNum*6];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	fread(tempCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);

	std::string fileStr1 = generateFileName(coarsePartId, path + "delaunayResults/returnAllTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
//	std::string fileStr1 = path + "delaunayResults/returnAllTriangleIds.tri";
	FILE *f1 = fopen(fileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}

	unsigned long long *tempPointIdArr;
	try {
		tempPointIdArr = new unsigned long long[triangleNum*3];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

	fread(tempPointIdArr, triangleNum*3, sizeof(unsigned long long), f1);
	fclose(f1);

	//remove returnTriangleCoors.tri, and returnTriangleIds.tri
//	std::string delCommand = "rm -f " + path + "delaunayResults/returnAllTriangleCoors.tri";
	std::string delCommand = "rm -f " + fileStr;
	system(delCommand.c_str());
//	delCommand = "rm -f " + path + "delaunayResults/returnAllTriangleIds.tri";
	delCommand = "rm -f " + path + fileStr1;
	system(delCommand.c_str());

	//generate a new triangleArr including those triangles left over (not belong to any active partition) and
	//and return triangles that came from delaynayMPI

	//triangleNum is total number of triangles
	triangle *triangleNewArr;
	try {
		triangleNewArr = new triangle[triangleNum + count];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!\n";
		exit(1);
	}

//triangle *triangleNewArr = new triangle[triangleNum];

	//copy undelivered triangle from triangleArr to triangleNewArr
	unsigned int index=0;
	for(unsigned int i=0; i<triangleArrSize; i++)
		if(triangleArr[i].delivered==false){
			triangleNewArr[index] = triangleArr[i];
			index++;
		}

	//based on two new arrays tempCoorArr & tempPointIdArr, populate info for the rest of triangles in triangleNewArr
	for(int i=0; i<triangleNum; i++){
		point p1(tempCoorArr[i*6], tempCoorArr[i*6+1], tempPointIdArr[i*3]);
		point p2(tempCoorArr[i*6+2], tempCoorArr[i*6+3], tempPointIdArr[i*3+1]);
		point p3(tempCoorArr[i*6+4], tempCoorArr[i*6+5], tempPointIdArr[i*3+2]);

		//assign new info
		triangleNewArr[index].delivered = false;
		triangleNewArr[index].p1.set(p1);
		triangleNewArr[index].p2.set(p2);
		triangleNewArr[index].p3.set(p3);

		triangleNewArr[index].computeCenterRadius();
		index++;
	}

	delete [] tempCoorArr;
	delete [] tempPointIdArr;

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(int i=0; i<xFinePartNum*yFinePartNum; i++)
		if(partArr[i].finish) partArr[i].triangleIdList.clear();

	//process triangleArr
	delete [] triangleArr;
	triangleArr = triangleNewArr;

	//clean up trianglePartitionList
	for(int i=0; i<triangleArrSize; i++) trianglePartitionList[i].clear();
	delete [] trianglePartitionList;
	triangleArrSize = triangleNum + count;
//	triangleArrSize = triangleNum;

std::cout<<"number of triangle after delaunay: "<<triangleArrSize<<"\n";

	//clean currActiveList
	if(!currActiveList.empty()) currActiveList.clear();

	//clean activePartSet
	if(!activePartSet.empty()) activePartSet.clear();
}

//=========================================================================================
//run MPI command to process Delaunay Triangulation on slave nodes
//Process delaunay for active partitions on cluster. Each partition will be processed (delaunay) on a processor
//atctivePartNum is the number of active partitions sending to slave nodes
void coarsePartition::runDelaunayMPI(int subPartitionSize, int activePartNum){
	//int activePartNum = activePartSet.size();
	std::string mpiCommand = "mpiexec -n " + std::to_string(activePartNum) + " -f machinefile ./delaunayMPI " + std::to_string(subPartitionSize) + " " + path + "delaunayResults/"; 
	//std::string mpiCommand = "mpiexec -n " + std::to_string(activePartNum) + "  ./delaunayMPI " + std::to_string(subPartitionSize) + " " + path + "delaunayResults/";
std::cout<<mpiCommand<<std::endl;
	system(mpiCommand.c_str());
}

//=========================================================================================
void coarsePartition::printTriangleArray(){
	for(int i=0; i<triangleArrSize; i++)
		std::cout<<i<<" "<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p1.getId()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p2.getId()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<" "<<triangleArr[i].p3.getId()<<"\n";

	std::cout<<"triangleArrSize: "<<triangleArrSize<<"\n";

}

//=========================================================================================
//return number of partitions that are unfinished
int coarsePartition::unfinishedPartNum(){
	int count = 0;
	for(int partId=0; partId<xFinePartNum*yFinePartNum; partId++)
		if(partArr[partId].finish==false) count++;
	return count;
}

//=========================================================================================
//return number of partitions that are unfinished
int coarsePartition::unDeliveredTriangleNum(){
	int count = 0;
	for(int partId=0; partId<triangleArrSize; partId++)
		if(triangleArr[partId].delivered==false) count++;
	return count;
}

//=========================================================================================
//When all fine-grained partitions in current coarse-grained partition are processed via MPI, 
//There are two kind of triangles: circumcircles intersect with the border of current coarse-grained partition and wholly inside
//we need to store all trianlges (circumcircles wholly inside) to file, and send back to domain.
//triangles that are leftover (circumcircles intersect with partition border) will be add to triangleIds.tri
void coarsePartition::storeAllTriangles(){

	std::list<triangle> boundaryTriangleList;
	std::list<triangle> insideTriangleList;
	//separate trialges into 2 groups: wholly inside current coarse-grained partition, and boundary triangles
	for(unsigned int index=0; index<triangleArrSize; index++){
		//geoBound is the bounding box of current coarse-grained partition
		if(triangleArr[index].inside(geoBound))//if triangle stay wholly inside current coarse-grained partition
			insideTriangleList.push_back(triangleArr[index]);
		else 
			boundaryTriangleList.push_back(triangleArr[index]);
	}

	//process insideTriangleList: store triangle Ids to storedTriangleIdsXX.tri
	//later domain will collect these files add directly to file triangleIds.tri
	unsigned int storedTriangleSize = insideTriangleList.size();
	unsigned long long *tempPointIdArr = new unsigned long long[storedTriangleSize*3];
	unsigned int index = 0;
	//copy ids of triangles from insideTriangleList to tempPointIdArr
	for(std::list<triangle>::iterator it=insideTriangleList.begin(); it!=insideTriangleList.end(); it++){
		tempPointIdArr[index*3] = (*it).p1.getId();
		tempPointIdArr[index*3+1] = (*it).p2.getId();
		tempPointIdArr[index*3+2] = (*it).p3.getId();
		index++;
	}
	//store tempPointIdArr to storedTriangleIdsXX.tri
	std::string fileStr = generateFileName(coarsePartId, path + "delaunayResults/storedTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(tempPointIdArr, storedTriangleSize*3, sizeof(unsigned long long), f);
	fclose(f);
	delete [] tempPointIdArr;


	//process boundaryTriangleList: store triangle Ids to boundaryTrianglesXX.tri
	//later domain will collect these files add to the domain triangle array
	unsigned int boundaryTriangleSize = boundaryTriangleList.size();
	triangle *tempTriangledArr = new triangle[boundaryTriangleSize];
	index = 0;
	for(std::list<triangle>::iterator it=boundaryTriangleList.begin(); it!=boundaryTriangleList.end(); it++){
		tempTriangledArr[index] = (*it);
		index++;
	}
	//store tempTriangledArr to boundaryTrianglesXX.tri
	fileStr = generateFileName(coarsePartId, path + "delaunayResults/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
	f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(tempTriangledArr, boundaryTriangleSize, sizeof(triangle), f);
	fclose(f);
	delete [] tempTriangledArr;
}


//===================================================================
coarsePartition::~coarsePartition(){
	//remove conflictPartList
	for(int index=0; index<xFinePartNum*yFinePartNum; index++){
		std::list<unsigned int> currList = conflictPartList[index];
		currList.clear();
	}
	delete [] conflictPartList;

	if(!activePartSet.empty()) activePartSet.clear();

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(int i=0; i<xFinePartNum*yFinePartNum; i++)
		partArr[i].triangleIdList.clear();
	delete [] partArr;

	//process triangleArr
	if(triangleArr!=NULL) delete [] triangleArr;

	delete [] pointPartInfoArr;

	//remove temporary files
	std::string delCommand = "rm " + path + "delaunayResults/temp*";
	system(delCommand.c_str());
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
	int gridx, gridy;

	//-1 because start from zero
	int gridMaxX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX -1;
	int gridMaxY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY -1;

	if(bBox.getHighPoint().getX() > geoBound.getHighPoint().getX())
		gridx = gridMaxX;
	else{
		double v = (bBox.getHighPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX;
		if((v-(int)v)>0) gridx = (int)v;
		else gridx = (int)v - 1;
	}

	if(bBox.getHighPoint().getY() > geoBound.getHighPoint().getY())
		gridy = gridMaxY;
	else{
		double v = (bBox.getHighPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY;
		if((v-(int)v)>0) gridy = (int)v;
		else gridy = (int)v - 1;
	}

	return 	gridElement(gridx, gridy);
}

