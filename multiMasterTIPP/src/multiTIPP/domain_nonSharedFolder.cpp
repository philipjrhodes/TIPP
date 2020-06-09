#include "domain_nonSharedFolder.h"
#include "io.h"
#include <iostream>
#include <fstream>

//===================================================================
domain::domain(double lowX, double lowY, double highX, double highY, std::string srcPath, std::string dstPath){
	lowPoint.setX(lowX);
	lowPoint.setY(lowY);
	highPoint.setX(highX);
	highPoint.setY(highY);

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=3, then the domain is square between 0,0 - 3,3
	domainSize = getDomainSizeX();
	currActivePartNum = 0;

	geoBound.setLowPoint(point(lowX,lowY));
	geoBound.setHighPoint(point(highX,highY));

	inputPath = srcPath;
	outputPath = dstPath;
	vertexRecordSize = 2;

	pointCoarsePartNum = NULL;
	initPointArr = NULL;
	triangleList = NULL;
	temporaryTriangleList = NULL;
	triangleArr = NULL;

	trianglePartitionList = NULL;
	partArr = NULL;
	activePartArr = NULL;

	//read info from file pointPartInfo.xfdl
	readPointPartFileInfo();

	try {
		partArr = new partition[xPartNum*yPartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain41\n";
		exit(1);
	}

	//disable some partitions that have no points
	updateEmptyPartitions();

	//determine super-triangle (must be large enough to completely contain all the points)
	//determine super-triangle (must be large enough to completely contain all the points)
	//domain is a square ABCD between (0,0) and (1,1), two initial super triangles are ABC and ACD
	//two coners A(0,0) and B(0,1) in convexHull take global indices pointNumMax and pointNumMax+1
	//two other points in convexHull are C(1,1) and D(1,0) with global indices are pointNumMax+2 and pointNumMax+3
	triangle *t1 = new triangle(point(lowX, lowY, pointNumMax), point(lowX, highY, pointNumMax+1), point(highX, highY, pointNumMax+2));
	//second super triangle is triangle ACD
	triangle *t2 = new triangle(point(lowX, lowY, pointNumMax), point(highX, highY, pointNumMax+2), point(highX, lowY, pointNumMax+3));

	triangleNode *n1 = new triangleNode;
	n1->tri = t1;
	triangleNode *n2 = new triangleNode;
	n2->tri = t2;	
	
	//and add the super-triangles
	insertFront(triangleList, n1);
	insertFront(triangleList, n2);

	//remove triangleIds.tri
	std::string delCommand = "rm " + outputPath + "/*.tri";
	system(delCommand.c_str());
	delCommand = "rm " + outputPath + "/temp*";
	system(delCommand.c_str());
}

domain::domain(double lowX, double lowY, double highX, double highY){
	lowPoint.setX(lowX);
	lowPoint.setY(lowY);
	highPoint.setX(highX);
	highPoint.setY(highY);
}

double domain::getDomainSizeX(){
	return (highPoint.getX() - lowPoint.getX());
}
double domain::getDomainSizeY(){
	return (highPoint.getY() - lowPoint.getY());
}

//===================================================================
void domain::readPointPartFileInfo(){
	//Read information from pointDomainInfo.xfdl
	std::string fileInfoStr = inputPath + "/" + "pointDomainInfo.xfdl";
	std::ifstream vertexPartInfoFile(fileInfoStr.c_str());
	if(!vertexPartInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first line --> read xPartNum, yPartNum
	vertexPartInfoFile >> strItem;
	xPartNum = atoi(strItem.c_str());

	vertexPartInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());

	pointNumMax = 0;
	pointNum = 0;
	unsigned int initPointNum = 0;
	unsigned int gridPointNum = 0;

	//second line --> number of init points for the domain
	vertexPartInfoFile >> strItem;
	initPointNum = atoi(strItem.c_str());

	//third line --> number of grid points (artificial points all around domain to avoid sliver triangles)
	vertexPartInfoFile >> strItem;
	gridPointNum = atoi(strItem.c_str());

	//fourth line --> total number of points including grid points
	vertexPartInfoFile >> strItem;
	pointNumMax = atoll(strItem.c_str());
	pointNum = pointNumMax;

	//fifth line --> total number of points in a coarse partition not including grid points
	pointCoarsePartNum = new unsigned int[xPartNum*yPartNum];
	unsigned int coarePartPointNum;
	for(unsigned int coarsePartId=0; coarsePartId<xPartNum*yPartNum; coarsePartId++){
		vertexPartInfoFile >> strItem;
		coarePartPointNum = atoll(strItem.c_str());
		pointCoarsePartNum[coarsePartId] = coarePartPointNum;
	}


std::cout<<"Number of initial points: "<<initPointNum<<"\n";
std::cout<<"Total number of points in domain: "<<pointNum<<"\n";

	vertexPartInfoFile.close();

	initPointArrSize = initPointNum + gridPointNum;

std::cout<<"Number of grid points (not include 4 corners): "<<gridPointNum<<"\n";
	
}

//=========================================================================================
//find all partitions that have no points (not exist files pointPartXX*), then set finish --> true, active --> false
void domain::updateEmptyPartitions(){
	for(unsigned int coarsePartId=0; coarsePartId<xPartNum*yPartNum; coarsePartId++){
		if(pointCoarsePartNum[coarsePartId]==0){
			partArr[coarsePartId].finish = true;
			partArr[coarsePartId].active = false;
		}
	}
}

//==============================================================================
//load initial points (from each partition) and grid points (NOT include 4 corners) in file initDomainPoints.ver
//each item of initDomainPoints.ver is an object point
//load grid points on the domain are points on 4 edges AB, BC, CD, DA of square domain NOT including 4 corners.
//4 corner points of square domain are (0,0), (0,1), (1,1), (1,0)
void domain::loadInitPoints(){
	std::string dataFileStr = inputPath + "/initDomainPoints.ver";
	FILE *f = fopen(dataFileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		fclose(f);
		return;
	}
	try {
		initPointArr = new point[initPointArrSize];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain179\n";
		exit(1);
	}

	fread(initPointArr, sizeof(point), initPointArrSize, f);
	fclose(f);
}

//==============================================================================
void domain::basicTriangulate(point p){
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
//std::cout<<"done Delaunay!!!\n";
}


//===================================================================
//return partition Id for a point
//partition a point into 2^k x 2^l partitions
unsigned int domain::partIndex(point p){
	double x = p.getX();
	double y = p.getY();

	double xPartSize = domainSize/xPartNum;
	double yPartSize = domainSize/yPartNum;

	unsigned int gridX = x/xPartSize;
	if(gridX>=xPartNum) gridX = gridX-1;
	unsigned int gridY = y/yPartSize;
	if(gridY>=xPartNum) gridY = gridY-1;

	return gridY*xPartNum + gridX;
}

//===================================================================
//all points are sorted before inserting to the Delaunay Triangulation
void domain::initTriangulate(){
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
void domain::triangleTransform(){
	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	triangleArrSize	= size(triangleList);
	if(triangleArrSize>1){
		try {
			triangleArr = new triangle[triangleArrSize];
		} catch (std::bad_alloc&) {
		  // Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!domain351\n";
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
//generate all intersections between each triangle and partitions in the domain.
//each item trianglePartitionList[i] cotains a linklist of partitionIds 
//which are the intersection with current triangleArr[i]
//input: triangle array (triangleArr), and array of list (trianglePartitionList)
//output: trianglePartitionList which contains linklist of partitions for each of triangleId
void domain::generateIntersection(){

	double globalLowX = geoBound.getLowPoint().getX();
	double globalLowY = geoBound.getLowPoint().getY();
    double gridElementSizeX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/xPartNum;
    double gridElementSizeY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/yPartNum;
//std::cout<<"gridElementSizeX: "<<gridElementSizeX<<", gridElementSizeY: "<<gridElementSizeY<<"\n";
//std::cout<<"xPartNum: "<<xPartNum<<", yPartNum: "<<yPartNum<<std::endl;

	try {
		trianglePartitionList = new std::list<unsigned int>[triangleArrSize];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain384\n";
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
//std::cout<<index<<" "<<triangleArrSize<<" lowPoint: "<<centerX-radius<<" "<<centerY-radius<<", highPoint: "<<centerX+radius<<" "<<centerY+radius<<std::endl;

		//gridBox that intersects with the bounding box of a triangle
		gridBound gb = boundingGrid(bBox);

		/*the coordiante of two corners of bounding grid*/
		unsigned int beginx = gb.getLowGridElement().getX();
		unsigned int beginy = gb.getLowGridElement().getY();
		unsigned int endx = gb.getHighGridElement().getX();
		unsigned int endy = gb.getHighGridElement().getY();
//std::cout<<index<<" beginx: "<<beginx<<", beginy: "<<beginy<<", endx: "<<endx<<", endy: "<<endy<<"\n";
		//Scan bounding grid
		for(unsigned int i = beginy; i<=endy; i++){
			for(unsigned int j = beginx; j<=endx; j++){
				//mapping a partition element into an element in partitionElementList
				unsigned int partitionEletIdx = xPartNum * i + j;
if((partitionEletIdx<0)||(partitionEletIdx>=xPartNum*yPartNum)){
	std::cout<<"partitionEletIdx= "<<partitionEletIdx<<"\n";
	std::cout<<index<<" beginx: "<<beginx<<", beginy: "<<beginy<<", endx: "<<endx<<", endy: "<<endy<<"\n";
	std::cout<<t.p1<<t.p2<<t.p3<<std::endl;
	std::cout<<"centerX: "<<centerX<<", centerY: "<<centerY<<", radius: "<<radius<<std::endl;
}
				//if the current partition is not finish
				if(!partArr[partitionEletIdx].finish){
//std::cout<<partitionEletIdx<<" ";
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
//std::cout<<"\n\n";
	}

	unsigned int count = 0;
	//count number of interior and boundary triangles
	for(unsigned int i=0; i<triangleArrSize; i++)
		if(trianglePartitionList[i].size()==1) count++;
	std::cout<<"&&&& Number of interior triangles: "<<count<<"\n";
	std::cout<<"&&&& Number of boundary triangles: "<<triangleArrSize - count<<"\n";
	std::cout<<"&&&& Total number of triangles: "<<triangleArrSize<<"\n";
	std::cout<<"&&&& Percent interior triangles over total triangles: "<<count*100/triangleArrSize<<"%\n";

}

//=========================================================================================
//generate confliction for each partition. 
//Means that what partition conflicts with which partition
//input: triangleArr, trianglePartitionList
//output: array of partitions (conflictPartList). Each item in array is a linklist of conflicted partIds
//use this function printConflictPartitions() to print the detail
void domain::generateConflictPartitions(){
	//double	currentTime = GetWallClockTime();
	//partTempList is an array of linklist, used to contain all conflictions of each partition in domain
	//two partitions are conflicted when exist a triangle whose circumcircle intersected with both partitions 
	try {
		conflictPartList = new std::list<unsigned int>[xPartNum*yPartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain462\n";
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

	for(unsigned int index=0; index<xPartNum*yPartNum; index++){
		conflictPartList[index].sort();
		conflictPartList[index].unique();
	}
	//std::cout<<"Amount of time of generateConflictPartitions: "<<GetWallClockTime()-currentTime<<"\n";	
}
//=========================================================================================
void domain::printConflictPartitions(){
	std::list<unsigned int> currList;
	//show conflictions for each partition
	for(int index=0; index<xPartNum*yPartNum; index++){
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
void domain::updateConflictPartitions(){
//	double	currentTime = GetWallClockTime();
	if(currActiveList.empty()) return;

//	std::cout<<"list of coarse partitions that are finished: [";
	//remove lists that are finished in conflictPartList
	for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
		unsigned int finishId = *it;
//		std::cout<<finishId<<" ";
		conflictPartList[finishId].clear();
	}
//	std::cout<<"]"<<std::endl;

	//remove finished partition in each list of conflictPartList

	for(unsigned int index=0; index<xPartNum*yPartNum; index++){
		if(partArr[index].finish==false){
			for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
				int finishId = *it;
				conflictPartList[index].remove(finishId);
			}
		}
	}
	//clean up currActiveList before the next stage: find active partitions
	currActiveList.clear();
//	std::cout<<"Amount of time of updateConflictPartitions: "<<GetWallClockTime()-currentTime<<"\n";	
}

//=========================================================================================
//generate active partitions in the domain: all triangles in a active partition are not intersected with other active partitions 
//means that seaprating partitions and their intersected circumcircle 
//based on the list of partitions intersected with the circumcircle of the triangles
//generate partition that are active
//input: array of list (conflictPartList)
//output: set of partitions (partIds) (activePartSet)
unsigned int domain::generateActivePartitions(){
	//double	currentTime = GetWallClockTime();
	//Update active-inactive partitions
	//find a shortest list in partTempList, the number in order that is not in the confliced list, 
	//will be picked out as the active partition
	bool *localActivePartArr;
	try {
		localActivePartArr = new bool[xPartNum*yPartNum];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain559\n";
		exit(1);
	}

	for(unsigned int i=0; i<xPartNum*yPartNum; i++) localActivePartArr[i] = false;

	//find the shortest list in partTempList
	unsigned int activePartId;
	unsigned int index = 0;
	//find the first unfinished partition
	while(index<xPartNum*yPartNum)
		if(partArr[index].finish==false){
			activePartId = index;
			break;
		}
		else index++;


	//find the first active partition
	unsigned int unFinishSize = 1;
	//find the first active partitions based on the shorted conflicting list in conflictPartList.
	for(unsigned int index=activePartId+1; index<xPartNum*yPartNum; index++){
		if(partArr[index].finish==false){
			unFinishSize++;//count number of unfinished partitions
			//find the shortest conflicted partitions
			if(conflictPartList[activePartId].size()>conflictPartList[index].size())
				activePartId = index;
				localActivePartArr[activePartId] = true;
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
		//while(index<xPartNum*yPartNum)
		for(unsigned int index=0; index<xPartNum*yPartNum; index++)
			//the first item in array not updated yet
			if((partArr[index].finish==false)&&(partArr[index].active==true)&&(localActivePartArr[index]!=true)){
				activePartId = index;
				localActivePartArr[activePartId] = true;
				currActiveList.push_back(activePartId);
				unFinishCount++;
				break;
			}
			//else index++;
		loopCount++;
		if(loopCount>=xPartNum*yPartNum) break;
	}

/*
	//print a current list of active partitions
	std::string msg = "[[[[[[[[list of current coarse active partitions: [";//currActiveList
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if((partArr[i].active)&&(!partArr[i].finish)) msg = msg + " " + toString(i) + " ";
	msg = msg + "]\n"; 
	std::cout<<msg;
*/
	//clean up and update
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		//if a partition is unfinished and active, then set --> finish
		if(!partArr[i].finish){
			if(partArr[i].active) partArr[i].finish = true;
			else partArr[i].active = true;
		}

/*	//print a current list of unfinished partitions
	std::cout<<"list of unfinished coarse partitions: [";
	for(int i=0; i<xPartNum*yPartNum; i++)
		if(!partArr[i].finish) std::cout<<i<<" ";
	std::cout<<"]\n\n";
*/

	//clear all item in active partition set
	if(!activePartSet.empty()) activePartSet.clear();



	//after determining active partition list, build a set of active partition Ids for later use
	if(!currActiveList.empty()){
		activePartArrSize = currActiveList.size();
		if(activePartArr!=NULL) delete [] activePartArr;
		activePartArr = new unsigned int[activePartArrSize];
		unsigned int index = 0;
		//scan all items in currActiveList to build activePartMap	
		for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
			unsigned int activePartId = *it;
			activePartSet.insert(activePartId);
			activePartArr[index] = activePartId;
			index++;
		}
	}

	delete [] localActivePartArr;
	//std::cout<<"Amount of time of generateActivePartitions: "<<GetWallClockTime()-currentTime<<"\n";
	return activePartSet.size();
}

//=========================================================================================
//deliver triangles to active partitions. For each triangle, find in the list of partitions that are 
//intersected with that triangle, if exist a partition Id belong to the active partition list (currActiveList)
//then set that triangle belong to the active partition. Each triangle belong to one active partition
void domain::deliverTriangles(double &masterTime, double &storeTime){
	std::set<unsigned int>::iterator t;
//	double	currentTime1 = GetWallClockTime();
	masterTime = 0;
	storeTime = 0;
	double currentTime = GetWallClockTime();

	//storeTriangleList used to store all triangles that are not intersected with any partition (because there are some inactive partitions)
	std::list<unsigned int> storeTriangleIdList;

	std::list<triangle> visualTriangleList;

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
				unsigned int partId = *it;
				t = activePartSet.find(partId);
				if (t != activePartSet.end()){//found this partId in activePartSet
					partArr[partId].triangleIdList.push_back(triangleIndex);
					//set triangle is delivered, a triangle is set belong to a partition at a time
					triangleArr[triangleIndex].delivered = true;
				}
			}
	}

	masterTime += GetWallClockTime()-currentTime;

	//store all point ids of triangleId in storeTriangleList to file
	unsigned long long size = storeTriangleIdList.size();
	if(size==0) return;

	currentTime = GetWallClockTime();
	unsigned long long *data;
	try {
		data = new unsigned long long[size*3];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain726\n";
		exit(1);
	}
	unsigned long long index = 0;
	for (std::list<unsigned int>::iterator it=storeTriangleIdList.begin(); it != storeTriangleIdList.end(); ++it){
		unsigned int id = *(it);//id is index of a triangle in triangleArr
		data[index*3] = triangleArr[id].p1.getId();
		data[index*3+1] = triangleArr[id].p2.getId();
		data[index*3+2] = triangleArr[id].p3.getId();
		index++;
	}
	masterTime += GetWallClockTime()-currentTime;

	currentTime = GetWallClockTime();
	//store data to file
	std::string fileStr = outputPath + "/triangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(f==NULL){
		std::cout<<"not exist "<<fileStr<<std::endl;
		fclose(f);
		exit(1);
	}
	fwrite(data, size*3, sizeof(unsigned long long), f);
	fclose(f);
	delete [] data;
	storeTime += GetWallClockTime()-currentTime;
	//std::cout<<"Amount of time of deliverTriangles: "<<GetWallClockTime()-currentTime1<<"\n";
}

//=========================================================================================
//number of active partition left over in activePartSet
unsigned int domain::activePartitionNumber(){
	return activePartSet.size();
}

//=========================================================================================
//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
//processNum is the number of cores available in MPI system or PBS
//However, each time, send only processNum of active partitions to slave  nodes
//Assume if total number of active partitions is 20, but number of core available is 6,
//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
//Output: current number of active partitions
unsigned int domain::prepareDataForDelaunayMPI(unsigned processNum, unsigned *&activePartPointSizeArr, unsigned &xCoarsePartNum, unsigned &yCoarsePartNum, unsigned *&activePartIdArr, unsigned *&activePartSizeArr, unsigned &activePartNum, double *&activeTriangleCoorArr, unsigned long long *&activeTriangleIdArr, double &masterTime){

	double currentTime;
	masterTime = 0;
	currentTime = GetWallClockTime();

	xCoarsePartNum = xPartNum;
	yCoarsePartNum = yPartNum;

	unsigned int totalActivePartNum = activePartSet.size();
	//activePartNum is current number of active partitions sending to MPI (slave nodes)
	if(processNum<totalActivePartNum)
		currActivePartNum = processNum;
	else currActivePartNum = totalActivePartNum;

	activePartNum = currActivePartNum;
	activePartIdArr = new unsigned int[currActivePartNum];
	activePartSizeArr = new unsigned int[currActivePartNum];
	activePartPointSizeArr = new unsigned int[currActivePartNum];

	int indexPartId = 0;

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

		localIndexPartId++;
		if(localIndexPartId == currActivePartNum) break;
	}

	//update activePartPointSizeArr
	for(unsigned int index=0; index<currActivePartNum; index++){
		activePartPointSizeArr[index] = pointCoarsePartNum[activePartIdArr[index]];
	}
	

	//take out active partId in activePartSet after add in to activePartIdArr
	for(unsigned int index=0; index<currActivePartNum; index++){
		activePartSet.erase(activePartIdArr[index]);
	}
//std::cout<<"------------ Number of active partition left over: "<<activePartSet.size()<<" ----------\n";

	std::cout<<"Number of active partition: "<<currActivePartNum<<"\n";

	collectActiveTriangles(activePartIdArr, activePartSizeArr, currActivePartNum, activeTriangleCoorArr, activeTriangleIdArr);

	masterTime += GetWallClockTime()-currentTime;
	return currActivePartNum;
}

//=========================================================================================
//this function collect all triangles belong to current active partitions and send them to sub-masters
//the file structure: (triangle1, triangle2, ...) --> {(p1, p2, p3), (p2, p5, p6), ....}
//each point: (x, y, Id)
//activePartIdArr contains all current active partition Ids
//activePartNum numer of current active partitions
void domain::collectActiveTriangles(unsigned int *activePartIdArr, unsigned *activePartSizeArr, unsigned int activePartNum, double *&activeTriangleCoorArr, unsigned long long *&activeTriangleIdArr){
//	double	currentTime = GetWallClockTime();

	unsigned long allTriangleNum = 0;
	//Determine all active triangles
	for(unsigned int activePartId=0; activePartId<activePartNum; activePartId++)
		allTriangleNum += activePartSizeArr[activePartId];

	activeTriangleCoorArr = new double[allTriangleNum*6];//contain three point coordinates
	activeTriangleIdArr = new unsigned long long[allTriangleNum*3];//contain three point Ids

	unsigned long long triangleNum;
	unsigned long long triangleIndex = 0;
	//scan all active partitions
	for(unsigned int activePartId=0; activePartId<activePartNum; activePartId++){
		//for each active partition, take out all triangles and store to  boundaryTriangleCoorArr and boundaryTriangleIdArr
		std::list<unsigned long long> triangleIdList = partArr[activePartIdArr[activePartId]].triangleIdList;
		triangleNum = activePartSizeArr[activePartId];

		for (std::list<unsigned long long>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			//distribute info of triangle into two arrays tempCoorArr (all point coordinates) and tempPointIdArr (point Ids)
			activeTriangleCoorArr[triangleIndex*6] = triangleArr[*it].p1.getX();
			activeTriangleCoorArr[triangleIndex*6+1] = triangleArr[*it].p1.getY();
			activeTriangleCoorArr[triangleIndex*6+2] = triangleArr[*it].p2.getX();
			activeTriangleCoorArr[triangleIndex*6+3] = triangleArr[*it].p2.getY();
			activeTriangleCoorArr[triangleIndex*6+4] = triangleArr[*it].p3.getX();
			activeTriangleCoorArr[triangleIndex*6+5] = triangleArr[*it].p3.getY();

			activeTriangleIdArr[triangleIndex*3] = triangleArr[*it].p1.getId();
			activeTriangleIdArr[triangleIndex*3+1] = triangleArr[*it].p2.getId();
			activeTriangleIdArr[triangleIndex*3+2] = triangleArr[*it].p3.getId();

			triangleIndex++;
		}
	}
}

//=========================================================================================
void domain::writeCoorsToFile(double *coorArr, unsigned long long triangleNum, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "w");
	if(f==NULL){
		std::cout<<"not exist "<<fileStr<<" from domain"<<std::endl;
		fclose(f);
		exit(1);
	}
	fwrite(coorArr, triangleNum*6, sizeof(double), f);
	fclose(f);
}
//=========================================================================================
void domain::writePointIdsToFile(unsigned long long *pointIdArr, unsigned long long triangleNum, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "w");
	if(f==NULL){
		std::cout<<"not exist "<<fileStr<<" from domain"<<std::endl;
		fclose(f);
		exit(1);
	}
	fwrite(pointIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
}


//=========================================================================================
void domain::addFile(std::string fullFileName1, std::string fullFileName2){
	std::string command;
	command = "cat " + fullFileName1 + " >> " + fullFileName2;
	system(command.c_str());
}

//=========================================================================================
void domain::removeFile(std::string fullPath){
	std::string delCommand = "rm -f " + fullPath;
	system(delCommand.c_str());
}

/*
//=========================================================================================
//add return stored triangles from MPI to returnAllStoreTriangleIds.tri and returnAllTriangleCoors.tri
//and merge to current triangleArr in memory
void domain::addReturnStoreTriangles(){
	std::string fullStoreFile;
	for(unsigned int index=0; index<activePartArrSize; index++){
		fullStoreFile = generateFileName(activePartArr[index], outputPath + "/triangleIds", xPartNum*yPartNum, ".tri");
		//add triangles in returnAllStoreTriangleIdsXX.tri to file triangleIds.tri, 
		addFile(fullStoreFile, outputPath + "/triangleIds.tri");
		std::cout<<"Adding " + fullStoreFile + " to triangleIds.tri\n";
		removeFile(fullStoreFile);
	}
}
*/


//=========================================================================================
//get returned triangles which are processed from MPI of each coarse-grained partition, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//add triangles in file boundaryTrianglesXX.tri (XX are active partition Ids) to triangleArr.
//and append storedTriangleIdsXX.tri to main file triangleIds.tri
//use activePartArr (contains all active partition IDs for a stage) to find all XX files
void domain::updateTriangleArr(){
	double	currentTime = GetWallClockTime();
	//determine number of undelivered triangles in triangleArr
	unsigned long long count=0;
	for(unsigned long long index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;

	std::string fileStr1 = outputPath + "/boundaryIds.tri";
	unsigned long long *returnBoundaryTriangleIdArr;
	double *returnBoundaryTriangleCoorArr;
	unsigned long long boundaryTriangleNum = 0;
	std::string fileStr2 = outputPath + "/boundaryCoors.tri";
	readTriangleIds(returnBoundaryTriangleIdArr, boundaryTriangleNum, fileStr1);
	readTriangleCoors(returnBoundaryTriangleCoorArr, boundaryTriangleNum, fileStr2);
	removeFile(fileStr1);
	removeFile(fileStr2);

	unsigned long long totalBoundaryTriangleArrSize = 0;
	totalBoundaryTriangleArrSize = boundaryTriangleNum;

	//triangleNum is total number of triangles
	triangle *triangleNewArr;
	try {
		triangleNewArr = new triangle[totalBoundaryTriangleArrSize + count];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain1035\n";
		exit(1);
	}

	//copy undelivered triangle from triangleArr to triangleNewArr
	unsigned long long index=0;
	for(unsigned long long i=0; i<triangleArrSize; i++)
		if(triangleArr[i].delivered==false){
			triangleNewArr[index] = triangleArr[i];
			index++;
		}

	//populate triangles for the rest of triangles in triangleNewArr
	for(unsigned long long i=0; i<boundaryTriangleNum; i++){
		triangleNewArr[index].p1.setX(returnBoundaryTriangleCoorArr[i*6]);
		triangleNewArr[index].p1.setY(returnBoundaryTriangleCoorArr[i*6+1]);
		triangleNewArr[index].p2.setX(returnBoundaryTriangleCoorArr[i*6+2]);
		triangleNewArr[index].p2.setY(returnBoundaryTriangleCoorArr[i*6+3]);
		triangleNewArr[index].p3.setX(returnBoundaryTriangleCoorArr[i*6+4]);
		triangleNewArr[index].p3.setY(returnBoundaryTriangleCoorArr[i*6+5]);

		triangleNewArr[index].p1.setId(returnBoundaryTriangleIdArr[i*3]);
		triangleNewArr[index].p2.setId(returnBoundaryTriangleIdArr[i*3+1]);
		triangleNewArr[index].p3.setId(returnBoundaryTriangleIdArr[i*3+2]);
		triangleNewArr[index].delivered = false;
		triangleNewArr[index].computeCenterRadius();

		index++;
	}

	delete [] returnBoundaryTriangleIdArr;
	delete [] returnBoundaryTriangleCoorArr;

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(partArr[i].finish) partArr[i].triangleIdList.clear();

	//process triangleArr
	delete [] triangleArr;
	triangleArr = triangleNewArr;

	//clean up trianglePartitionList
	for(unsigned long long i=0; i<triangleArrSize; i++) trianglePartitionList[i].clear();
	delete [] trianglePartitionList;
	triangleArrSize = totalBoundaryTriangleArrSize + count;

	//clean currActiveList
	if(!currActiveList.empty()) currActiveList.clear();

	//clean activePartSet
	if(!activePartSet.empty()) activePartSet.clear();
	std::cout<<"Amount of time of updateTriangleArr: "<<GetWallClockTime()-currentTime<<"\n";
}

//=========================================================================================
void domain::printTriangleArray(){
	for(unsigned long long i=0; i<triangleArrSize; i++)
		std::cout<<i<<" "<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p1.getId()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p2.getId()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<" "<<triangleArr[i].p3.getId()<<"\n";

	std::cout<<"triangleArrSize: "<<triangleArrSize<<"\n";
}

//=========================================================================================
//return number of partitions that are unfinished
unsigned int domain::unfinishedPartNum(){
	unsigned int count = 0;
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++)
		if(partArr[partId].finish==false) count++;
	return count;
}

//=========================================================================================
//return number of partitions that are unfinished
unsigned int domain::unDeliveredTriangleNum(){
	unsigned int count = 0;
	for(unsigned int partId=0; partId<triangleArrSize; partId++)
		if(triangleArr[partId].delivered==false) count++;
	return count;
}

//=========================================================================================
//When all partitions are processed via MPI, we need to store all trianlges left over
void domain::storeAllTriangles(){

	//generate a tempPointIdArr containing those triangles left over (not belong to any active partition)
	//triangleArrSize is the number of triangles left over
	unsigned long long *tempPointIdArr;
	try {
		tempPointIdArr = new unsigned long long[triangleArrSize*3];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow!!!!!!!!!!!domain1126\n";
		exit(1);
	}

	//copy ids of triangles left over from triangleArr to tempPointIdArr
	for(unsigned long long index=0; index<triangleArrSize; index++){
		tempPointIdArr[index*3] = triangleArr[index].p1.getId();
		tempPointIdArr[index*3+1] = triangleArr[index].p2.getId();
		tempPointIdArr[index*3+2] = triangleArr[index].p3.getId();
	}

	//store tempPointIdArr to triangleIds.tri
	//open triangleIds.tri for appending
	std::string fileStr = outputPath + "/triangleIds.tri";
	storeTriangleIds(tempPointIdArr, triangleArrSize, fileStr, "a");

	delete [] tempPointIdArr;
}

//===================================================================
//find xFinePartNum and yFinePartNum
void domain::readFinePartitionSize(unsigned int &xFinePartNum, unsigned int &yFinePartNum){
	//Read information from pointDomainInfo.xfdl
	std::string fileInfoStr = generateFileName(0, inputPath + "/pointPartInfo", xPartNum*yPartNum, ".xfdl");
	std::ifstream vertexPartInfoFile(fileInfoStr.c_str());
	if(!vertexPartInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first line --> read xPartNum, yPartNum
	vertexPartInfoFile >> strItem;
	xFinePartNum = atoi(strItem.c_str());

	vertexPartInfoFile >> strItem;
	yFinePartNum = atoi(strItem.c_str());
	vertexPartInfoFile.close();
}
//===================================================================
domain::~domain(){
//	if(initPointArr!=NULL) delete [] initPointArr;

	delete [] pointCoarsePartNum;

	//remove conflictPartList
	for(unsigned int index=0; index<xPartNum*yPartNum; index++){
		std::list<unsigned int> currList = conflictPartList[index];
		currList.clear();
	}
	delete [] conflictPartList;

	if(!activePartSet.empty()) activePartSet.clear();
	if(activePartArr!=NULL) delete [] activePartArr;

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		partArr[i].triangleIdList.clear();
	delete [] partArr;

	//process triangleArr
	if(triangleArr!=NULL) delete [] triangleArr;

	//remove temporary files
	std::string delCommand = "rm " + outputPath + "/temp*";
	system(delCommand.c_str());
}

//=========================================================================================
//return a boundingGrid which inclides lower left corner and higher right corner of a boundingBox
gridBound domain::boundingGrid(boundingBox bBox){
	float gridElementSizeX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/xPartNum;
    float gridElementSizeY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/yPartNum;
//std::cout<<gridElementSizeX<<" "<<gridElementSizeY<<"\n";

    gridElement lowGridElement(mapHigh(bBox, gridElementSizeX, gridElementSizeY));
    gridElement highGridElement(mapLow(bBox, gridElementSizeX, gridElementSizeY));

    return gridBound(lowGridElement, highGridElement);
}

//=========================================================================================
/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the higher index. Ex: 2.5/2 = 2 --> column/row 0, 1, 2; 2.0/2 = 1 */
gridElement domain::mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY){
	int gridx, gridy;

	if(bBox.getLowPoint().getX()<0) gridx = 0;
	else gridx = (bBox.getLowPoint().getX() - geoBound.getLowPoint().getX())/gridElementSizeX;
	if(bBox.getLowPoint().getY()<0) gridy = 0;
	else gridy = (bBox.getLowPoint().getY() - geoBound.getLowPoint().getY())/gridElementSizeY;

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
gridElement domain::mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY){
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

	//check in special case
//	if(gridx<0) gridx=0;
//	if(gridy<0) gridy=0;

	return 	gridElement(gridx, gridy);
}

