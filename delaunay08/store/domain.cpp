#include "domain.h"
#include "common.h"
#include <iostream>
#include <fstream>

//===================================================================
domain::domain(double lowX, double lowY, double highX, double highY, std::string pathStr){
	lowPoint.setX(lowX);
	lowPoint.setY(lowY);
	highPoint.setX(highX);
	highPoint.setY(highY);

	geoBound.setLowPoint(point(0.0,0.0));
	geoBound.setHighPoint(point(1.0,1.0));

	path = pathStr;
	vertexRecordSize = 2;

	pointPartInfoArr = NULL;
	initPointArr = NULL;
	extensionPointArr = NULL;
	triangleList = NULL;
	triangleArr = NULL;

	trianglePartitionList = NULL;
	partArr = NULL;
	pointIdArr = NULL;


	readPointPartFileInfo();

	partArr = new partition[xPartNum*yPartNum];

	//determine super-triangle (must be large enough to completely contain all the points)
	//determine super-triangle (must be large enough to completely contain all the points)
	//domain is a square ABCD between (0,0) and (1,1), two initial super triangles are ABC and ACD
	//two coners A(0,0) and B(0,1) in convexHull take global indices pointNumMax and pointNumMax+1
	//two other points in convexHull are C(1,1) and D(1,0) with global indices are pointNumMax+2 and pointNumMax+3
	triangle *t1 = new triangle(point(0.0, 0.0, pointNumMax), point(0.0, 1.0, pointNumMax+1), point(1.0, 1.0, pointNumMax+2));
	//second super triangle is triangle ACD
	triangle *t2 = new triangle(point(0.0, 0.0, pointNumMax), point(1.0, 1.0, pointNumMax+2), point(1.0, 0.0, pointNumMax+3));

	triangleNode *n1 = new triangleNode;
	n1->tri = t1;
	triangleNode *n2 = new triangleNode;
	n2->tri = t2;	
	
	//and add the super-triangles
	insertFront(triangleList, n1);
	insertFront(triangleList, n2);

	//remove triangleIds.tri
	std::string delCommand = "rm " + path + "delaunayResults/triangleIds.tri";
	system(delCommand.c_str());
}

domain::domain(double lowX, double lowY, double highX, double highY){
	lowPoint.setX(lowX);
	lowPoint.setY(lowY);
	highPoint.setX(highX);
	highPoint.setY(highY);
}
/*
domain::domain(const point &lPoint, const point &hPoint){
	lowPoint = lPoint;
	highPoint = hPoint;
}
*/

point domain::getLowPoint(){
	return lowPoint;
}
point domain::getHighPoint(){
	return highPoint;
}

void domain::setLowPoint(point p){
	lowPoint = p;
}
void domain::setHighPoint(point p){
	highPoint = p;
}

double domain::getDomainSizeX(){
	return (highPoint.getX() - lowPoint.getX());
}
double domain::getDomainSizeY(){
	return (highPoint.getY() - lowPoint.getY());
}

//===================================================================
void domain::readPointPartFileInfo(){
	//Read information from pointPartInfo.xfdl
	std::string fileInfoStr = path + "delaunayResults/" + "pointPartInfo.xfdl";
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
	int pointSum = 0;

	pointPartInfoArr = new unsigned int[xPartNum*yPartNum];

	//second line: number of points for each partition (without initial points)
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		vertexPartInfoFile >> strItem;
		pointPartInfoArr[i] = atoi(strItem.c_str());
std::cout<<pointPartInfoArr[i]<<" ";
		pointNumMax = pointNumMax + pointPartInfoArr[i];
		pointSum = pointSum + pointPartInfoArr[i];
	}
std::cout<<"\n";

	//skip third line: offset of previous line (number of points in each partition)
	for(unsigned int i=0; i<xPartNum*yPartNum; i++) vertexPartInfoFile >> strItem;

	//fourth line: the size of gridPoints (or total number of initial points)
	vertexPartInfoFile >> strItem;
	initPointArrSize = atoi(strItem.c_str());
	pointNumMax = pointNumMax + initPointArrSize;

std::cout<<"initPointArrSize: "<<initPointArrSize<<"\n";
std::cout<<"pointSum: "<<pointSum<<", pointNumMax: "<<pointNumMax<<"\n";

	//skip fifth line: number of point in each partition (includes initial points)
	for(unsigned int i=0; i<xPartNum*yPartNum+1; i++) vertexPartInfoFile >> strItem;
	//take the last number of fifth line (for the extension points including 4 corners); 
	extensionPointArrSize = atoi(strItem.c_str());

	pointIdArr = new unsigned long int[xPartNum*yPartNum+1];

	//sixth line: read point Ids (or offset of number of points of fullPointPart.ver)
	for(unsigned int i=0; i<xPartNum*yPartNum+1; i++){
		vertexPartInfoFile >> strItem;
		pointIdArr[i] = atoi(strItem.c_str());
	}

	vertexPartInfoFile.close();


	int initPartSize = initPointArrSize/(xPartNum*yPartNum);

/*
//print pointIdArr[]
for(int i=0; i<xPartNum*yPartNum+1; i++){
	std::cout<<pointIdArr[i]<<" ";
}
std::cout<<"\n";
*/
}

//==============================================================================
void domain::loadInitPoints(){
	std::string dataFileStr = path + "delaunayResults/initPoints.ver";
	FILE *f = fopen(dataFileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		return;
	}
//std::cout<<"initPointArrSize: "<<initPointArrSize<<"\n";
	initPointArr = new double[initPointArrSize*vertexRecordSize];
	fread(initPointArr, sizeof(double), initPointArrSize*vertexRecordSize, f);
	fclose(f);
}

//==============================================================================
//extension points include 4 points of square (0,0), (0,1), (1,1), (1,0) and grid points on the bounding domain in the end of fullPointPartArr.ver
//grid points on the domain are points on 4 edges AB, BC, CD, DA of square domain.
//total number of grid points is xPartNum*yPartNum including 4 corners.
void domain::loadExtensionPoints(){
	std::string dataFileStr = path + "delaunayResults/fullPointPart.ver";
	FILE *f = fopen(dataFileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		return;
	}
	//move cursor to the extension points 
	fseek(f, pointNumMax*sizeof(double)*vertexRecordSize, SEEK_CUR); // move to a specific location
	extensionPointArr = new double[extensionPointArrSize*vertexRecordSize];

	//read all extension points (grid points on 4 edges of domain, incluing 3 corners)
	fread(extensionPointArr, sizeof(double), extensionPointArrSize*vertexRecordSize, f);
	fclose(f);

//std::cout<<"pointNum: "<<pointNum<<", extensionPointArrSize: "<<extensionPointArrSize<<std::endl;
//std::cout.precision(16);
//for(int i=0; i<extensionPointArrSize;i++)
//std::cout<<extensionPointArr[i*2]<<" "<<extensionPointArr[i*2+1]<<std::endl;

}


//==============================================================================
//triangulate without remove any triangles
//==============================================================================
void domain::basicTriangulate(point p){
	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;

	triangleNode *preNode = NULL;
	triangleNode *currNode = triangleList;
	bool goNext = true;
	while(currNode != NULL){
		if(currNode->tri->circumCircleContains(p)){
//std::cout<<currNode->tri->p1<<currNode->tri->p2<<currNode->tri->p3;
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
//	edgeNode *preEdgeNode1 = NULL;
	edgeNode *currEdgeNode1 = polygon;
//	edgeNode *preEdgeNode2 = NULL;
	edgeNode *currEdgeNode2 = polygon;
	while(currEdgeNode1!=NULL){
		while(currEdgeNode2!=NULL){
			//two different edges in polygon but same geological edge
			if((currEdgeNode1!=currEdgeNode2)&&(*(currEdgeNode1->ed) == *(currEdgeNode2->ed))){
				edge *newEdge = new edge(*currEdgeNode1->ed);
				edgeNode *newEdgeNode = createNewNode(newEdge);
				insertFront(badEdges, newEdgeNode);
			}
			//preEdgeNode2 = currEdgeNode2;
			currEdgeNode2 = currEdgeNode2->next;
		}
		//preEdgeNode1 = currEdgeNode1;
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
//std::cout<<"done Delaunay!!!\n";
}

//===================================================================
//return partition Id for a point
//partition a point into 2^k x 2^l partitions
int domain::partIndex(point p){
	double x = p.getX();
	double y = p.getY();

	double xPartSize = 1.0/xPartNum;
	double yPartSize = 1.0/yPartNum;

	int gridX = x/xPartSize;
	if(gridX>=xPartNum) gridX = gridX-1;
	int gridY = y/yPartSize;
	if(gridY>=xPartNum) gridY = gridY-1;

	return gridY*xPartNum + gridX;
}

//===================================================================
void domain::initTriangulate(){
	unsigned long int startPointIndex = 0;
	unsigned long int extensionPointIndex = pointNumMax+4;

	//triangulate extension points (not including 4 corner points because they are in 2 big triangles ABC & ACD)
	//4 means  not include 4 corners
	for(int index=4; index<extensionPointArrSize; index++){
		point p;
		p.setX(extensionPointArr[index*vertexRecordSize]);
		p.setY(extensionPointArr[index*vertexRecordSize+1]);
		p.setId(extensionPointIndex);
//std::cout<<extensionPointIndex<<" "<<extensionPointArr[index*vertexRecordSize]<<" "<<extensionPointArr[index*vertexRecordSize+1]<<std::endl;
		basicTriangulate(p);
		extensionPointIndex++;
	}	

	//triagulate init points
	for(int index=0; index<initPointArrSize; index++){
		point p;
		p.setX(initPointArr[index*vertexRecordSize]);
		p.setY(initPointArr[index*vertexRecordSize+1]);
//std::cout<<initPointArr[index*vertexRecordSize]<<" "<<initPointArr[index*vertexRecordSize+1]<<std::endl;
		int currPartId = partIndex(p);
		startPointIndex = pointIdArr[currPartId];
		pointIdArr[currPartId]++;//this Id for the next point
		p.setId(startPointIndex);
		basicTriangulate(p);
//std::cout<<currPartId<<" "<<startPointIndex<<" "<<initPointArr[index*vertexRecordSize]<<" "<<initPointArr[index*vertexRecordSize+1]<<"\n";
	}
}

//=========================================================================================
//transform link list of triangles (triangleList) into array of triangle (triangleArr)
void domain::triangleTransform(){
	triangleArrSize	= size(triangleList);
	if(triangleArrSize>1){
		triangleArr = new triangle[triangleArrSize];
		triangleNode *scanNode = triangleList;
		unsigned int index = 0;
		while(scanNode!=NULL){
			triangleArr[index] = *(scanNode->tri);
			index++;
			scanNode = scanNode->next;
		}	
	}
	removeLinkList(triangleList);
//	for(int i=0; i<triangleArrSize; i++) std::cout<<triangleArr[i].p1<<std::endl;
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

	trianglePartitionList = new std::list<unsigned int>[triangleArrSize];
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
		int beginx = gb.getLowGridElement().getX();
		int beginy = gb.getLowGridElement().getY();
		int endx = gb.getHighGridElement().getX();
		int endy = gb.getHighGridElement().getY();
//std::cout<<index<<" beginx: "<<beginx<<", beginy: "<<beginy<<", endx: "<<endx<<", endy: "<<endy<<"\n";
		//Scan bounding grid
		for(int i = beginy; i<=endy; i++){
			for(int j = beginx; j<=endx; j++){
				//mapping a partition element into an element in partitionElementList
				int partitionEletIdx = xPartNum * i + j;
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

/*
	//print intersections for each triangle
	for(unsigned int index=0; index<triangleArrSize; index++){
		std::list<unsigned int> currList = trianglePartitionList[index];
		std::cout<<"partitions intersec with triangle "<<index<<": ";
		for (std::list<unsigned int>::iterator it=currList.begin(); it != currList.end(); ++it)
	    	std::cout << ' ' << *it;
		std::cout<<std::endl;
	}
*/
std::cout<<"====================================================\n";

}

//=========================================================================================
//generate confliction for each partition. 
//Means that what partition conflicts with which partition
//input: triangleArr, trianglePartitionList
//output: array of partitions (conflictPartList). Each item in array is a linklist of conflicted partIds
//use this function ()printConflictPartitions() to print the detail
void domain::generateConflictPartitions(){
	//partTempList is an array of linklist, used to contain all conflictions of each partition in domain
	//two partitions are conflicted when exist a triangle whose circumcircle intersected with both partitions 
	conflictPartList = new std::list<unsigned int>[xPartNum*yPartNum];

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

	for(int index=0; index<xPartNum*yPartNum; index++){
		conflictPartList[index].sort();
		conflictPartList[index].unique();
	}
}
//=========================================================================================
void domain::printConflictPartitions(){
	std::list<unsigned int> currList;
	//show conflictions for each partition
	for(int index=0; index<xPartNum*yPartNum; index++){
//		conflictPartList[index].sort();
//		conflictPartList[index].unique();
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

	for(int index=0; index<xPartNum*yPartNum; index++){
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
//generate active partitions in the domain: all triangles in a active partition are not intersected with other active partitions 
//means that seaprating partitions and their intersected circumcircle 
//based on the list of partitions intersected with the circumcircle of the triangles
//generate partition that are active
//input: array of list (conflictPartList)
//output: set of partitions (partIds) (activePartSet)
void domain::generateActivePartitions(){

	//Update active-inactive partitions
	//find a shortest list in partTempList, the number in order that is not in the confliced list, 
	//will be picked out as the active partition
	bool *activePartArr = new bool[xPartNum*yPartNum];
	for(int i=0; i<xPartNum*yPartNum; i++) activePartArr[i] = false;


	//find the shortest list in partTempList
	int activePartId;
	int index = 0;
	//find the first unfinished partition
	while(index<xPartNum*yPartNum)
		if(partArr[index].finish==false){
			activePartId = index;
			break;
		}
		else index++;


	//find the first active partition
	int unFinishSize = 1;

	for(int index=activePartId+1; index<xPartNum*yPartNum; index++){
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


	int unFinishCount=1;//found the first active partition
	//from the list of an active partition, update all conflicted partitions to partArr
	while(unFinishCount<unFinishSize){
		std::list<unsigned int> currList = conflictPartList[activePartId];
		if(activePartId==0) std::cout<<activePartId<<"\n";	
		for (std::list<unsigned int>::iterator it=currList.begin(); it != currList.end(); ++it){
			int conflictPartId = *it;
			if(partArr[conflictPartId].finish == false){
				if(partArr[conflictPartId].active){
					 partArr[conflictPartId].active = false;
					unFinishCount++;
				}
			}
		}
		index = 0;
		//find the next active partition Id
		while(index<xPartNum*yPartNum)
			//the first item in array not updated yet
			if((partArr[index].finish==false)&&(partArr[index].active==true)&&(activePartArr[index]!=true)){
				activePartId = index;
				activePartArr[index] = true;
				currActiveList.push_back(activePartId);
				unFinishCount++;
//std::cout<<"\nactivePartId: "<<activePartId<<"\n";
				break;
			}
			else index++;
	}


	//print a current list of active partitions
	std::cout<<"list of current active partitions: [";//currActiveList
	for(int i=0; i<xPartNum*yPartNum; i++)
		if((partArr[i].active)&&(!partArr[i].finish)) std::cout<<i<<" ";
	std::cout<<"]\n";

	//clean up and update
	for(int i=0; i<xPartNum*yPartNum; i++)
		//if a partition is unfinished and active, then set --> finish
		if(!partArr[i].finish){
			if(partArr[i].active) partArr[i].finish = true;
			else partArr[i].active = true;
		}

	//print a current list of unfinished partitions
	std::cout<<"list of unfinished partitions: [";
	for(int i=0; i<xPartNum*yPartNum; i++)
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
}

//=========================================================================================
//deliver triangles to active partitions. For each triangle, find in the list of partitions that are 
//intersected with that triangle, if exist a partition Id belong to the active partition list (currActiveList)
//then set that triangle belong to the active partition. Each triangle belong to one active partition
void domain::deliverTriangles(){
	std::set<int>::iterator t;

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
std::cout<<"triangleIdList is empty\n"		;
		return;
	}
	unsigned long int *data = new unsigned long int[size*3];
	unsigned int index = 0;
	for (std::list<unsigned int>::iterator it=storeTriangleIdList.begin(); it != storeTriangleIdList.end(); ++it){
		unsigned int id = *(it);//id is index of a triangle in triangleArr
		data[index*3] = triangleArr[id].p1.getId();
		data[index*3+1] = triangleArr[id].p2.getId();
		data[index*3+2] = triangleArr[id].p3.getId();
		index++;
	}
	//store data to file
	std::string fileStr = path + "delaunayResults/triangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	fwrite(data, size*3, sizeof(unsigned long int), f);
	fclose(f);
	delete [] data;

/*	//print delivered triangles
	std::cout<<"total number of initial trangles: "<<triangleArrSize<<"\n";
	std::cout<<"list of trangles that are delivered: \n";
	for(unsigned int triangleIndex=0; triangleIndex<triangleArrSize; triangleIndex++){
		if(triangleArr[triangleIndex].delivered) std::cout<<triangleIndex<<" ";
	}
	std::cout<<"\n";
*/
}

//=========================================================================================
//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
//coreNum is the number of cores available in MPI system or PBS
//However, each time, send only coreNum of active partitions to slave  nodes
//Assume if total number of active partitions is 20, but number of core available is 6,
//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
//void domain::distributeDelaunayMPI(int coreNum){
void domain::distributeDelaunayMPI(){
	int *activePartIdArr;
	int *activePartSizeArr;
	int *activePartSizeOffsetArr;

	int totalActivePartNum = activePartSet.size();
	//activePartNum is current number of active partitions sending to MPI (slave nodes)
	int activePartNum = totalActivePartNum;
//	if(coreNum<totalActivePartNum)
//		activePartNum = coreNum;
//	else activePartNum = totalActivePartNum;


	//array of active partition Ids 
	activePartIdArr = new int[activePartNum];
	activePartSizeArr = new int[activePartNum];
	activePartSizeOffsetArr = new int[activePartNum];

	int indexPartId = 0;
	int totalTriangleSize = 0;

	//scan all active partitions to update activePartIdArr, activePartSizeArr
	for (std::set<int>::iterator it=activePartSet.begin(); it!=activePartSet.end(); ++it){
/*		if(indexPartId % activePartNum == 0){
			totalTriangleSize = 0;
			
		}
*/
		//process each active partition
		int partId = *it;
		activePartIdArr[indexPartId] = partId;

		std::list<unsigned int> triangleIdList = partArr[partId].triangleIdList;
		//number of triangles belong to current partition
		int triangleNum = triangleIdList.size();
		activePartSizeArr[indexPartId] = triangleNum;
		indexPartId++;

		totalTriangleSize = totalTriangleSize + triangleNum;
	}
	//store coordinates and pointId array to tempCoor.tri and tempPointId.tri
	storeActiveParitions(activePartIdArr, activePartNum, totalTriangleSize);

	std::cout<<"total active TriangleSize: "<<totalTriangleSize<<"\n";
	//update activePartSizeOffsetArr based on activePartSizeArr
	activePartSizeOffsetArr[0]=0;
	for(int i=1; i<activePartNum; i++)
		activePartSizeOffsetArr[i] = activePartSizeOffsetArr[i-1]+activePartSizeArr[i-1];

	//store info of active partitions to tempTriangles.xfdl
	storeActiveParitionInfo(activePartNum, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr);

	delete [] activePartIdArr;
	delete [] activePartSizeArr;
	delete [] activePartSizeOffsetArr;
}
//=========================================================================================
//activePartIdArr contains all current active partition Ids
//activePartNum numer of current active partitions
//totalTriangleSize is number of triangles in all current active partitions (activePartNum)
//this function stores all triangles belong to current active partitions into files for further delaunay on cluster
//the file structure: (triangle1, triangle2, ...) --> {(p1, p2, p3), (p2, p5, p6), ....}
//each point: (x, y, Id)
void domain::storeActiveParitions(int *activePartIdArr, int activePartNum, unsigned int totalTriangleSize){

	//total triangles belong to all active partitions
	//tempTriangleArr will be send to nodes using MPI
	double *tempCoorArr = new double[totalTriangleSize*6];//contain three point coordinates
	unsigned long int *tempPointIdArr = new unsigned long int[totalTriangleSize*3];//contain three point Ids
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

	//store tempCoorArr to file tempCoor.tri
	std::string fileStr = path + "delaunayResults/tempCoor.tri";
	FILE *f1 = fopen(fileStr.c_str(), "wb");
	if(!f1){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(tempCoorArr, totalTriangleSize*6, sizeof(double), f1);
	fclose(f1);

	//store tempPointIdArr to file tempPointId.tri
	std::string fileStr2 = path + "delaunayResults/tempPointId.tri";
	FILE *f2 = fopen(fileStr2.c_str(), "wb");
	if(!f2){
		std::cout<<"not exist "<<fileStr2<<std::endl;
		return;
	}
	fwrite(tempPointIdArr, totalTriangleSize*3, sizeof(unsigned long int), f2);
	fclose(f2);

	delete [] tempCoorArr;
	delete [] tempPointIdArr;
}

//=========================================================================================
//store info of active partitions to tempTriangles.xfdl
//The info consists of:
//	- number of current active partitions, 
//	- active partition ids
//	- number of triangles belong to active partitions
//	- offsets of number of triangles
//	- number of new points (not inserted yet) in each active partition
//	- offset of number of new points
//	- xPartNum, yPartNum --> domain size
//	- startIds for all active partitions
void domain::storeActiveParitionInfo(int activePartNum, int *activePartIdArr, int *activePartSizeArr, int *activePartSizeOffsetArr){
	//stores meta data
	std::string fileInfoStr = path + "delaunayResults/" + "tempTriangles.xfdl";
	std::ofstream infoFile(fileInfoStr, std::ofstream::out);
	//first line store number of active partition
	infoFile<<activePartNum<<"\n";
	//second line stores active partition ids
	for(int i=0; i<activePartNum; i++) 	infoFile<<activePartIdArr[i]<<" ";
	infoFile<<"\n";
	//third line stores number of triangles belong to active partitions
	for(int i=0; i<activePartNum; i++) 	infoFile<<activePartSizeArr[i]<<" ";
	infoFile<<"\n";
	//fourth line stores offsets of third line (number of triangles)
	for(int i=0; i<activePartNum; i++) 	infoFile<<activePartSizeOffsetArr[i]<<" ";
	infoFile<<"\n";
	//fifth line stores number of points in each active partition
	for(int i=0; i<activePartNum; i++) 	infoFile<<pointPartInfoArr[activePartIdArr[i]]<<" ";
	infoFile<<"\n";

	//compute the offset of pointPartInfoArr
	int *pointPartInfoOffsetArr = new int[xPartNum*yPartNum];
	pointPartInfoOffsetArr[0] = 0;
	for (int i=1; i<xPartNum*yPartNum; i++) pointPartInfoOffsetArr[i] = pointPartInfoOffsetArr[i-1]+pointPartInfoArr[i-1];

	//sixth line stores the offset of number of points in each active partition
	for(int i=0; i<activePartNum; i++) 	infoFile<<pointPartInfoOffsetArr[activePartIdArr[i]]<<" ";
	infoFile<<"\n";
	//seventh line stores xPartNum, yPartNum
	infoFile<<xPartNum<<" "<<yPartNum<<"\n";
	//eighth line is stored startIds for all active partitions
	for(int i=0; i<activePartNum; i++) 	infoFile<<pointIdArr[activePartIdArr[i]]<<" ";

	infoFile.close();
	delete [] pointPartInfoOffsetArr;
}

//=========================================================================================
//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//append the content of returnStoreTriangleIds.tri to triangleIds.tri
void domain::updateTriangleArr(){

	//determine number of undelivered triangles in triangleArr
	unsigned int count=0;
	for(unsigned int index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;
std::cout<<"undelivered triangles: "<<count<<"\n";
	//determine number of triangles that came from delaunayMPI

	//read triangles from two files returnTriangleCoors.tri, returnTriangleIds.tri 
	//and add to the remaining trianglesList
	std::string fileStr = path + "delaunayResults/returnTriangleCoors.tri";

	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(6*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
std::cout<<"triangleNum: "<<triangleNum<<"\n";
	double *tempCoorArr = new double[triangleNum*6];
	fread(tempCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);


	std::string fileStr1 = path + "delaunayResults/returnTriangleIds.tri";
	FILE *f1 = fopen(fileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}
	unsigned long int *tempPointIdArr = new unsigned long int[triangleNum*3];
	fread(tempPointIdArr, triangleNum*3, sizeof(unsigned long int), f1);
	fclose(f1);

	//remove returnTriangleCoors.tri, and returnTriangleIds.tri
	std::string delCommand = "rm -f " + path + "delaunayResults/returnTriangleCoors.tri";
	system(delCommand.c_str());
	delCommand = "rm -f " + path + "delaunayResults/returnTriangleIds.tri";
	system(delCommand.c_str());


	//generate a new triangleArr including those triangles left over (not belong to any active partition) and
	//and return triangles that came from delaynayMPI

	//triangleNum is total number of triangles
	triangle *triangleNewArr = new triangle[triangleNum + count];
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
	for(int i=0; i<xPartNum*yPartNum; i++)
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
//append the content of returnStoreTriangleIds.tri (from MPI processing) to triangleIds.tri
void domain::collectStoreTriangleIds(){
	//append the content of returnStoreTriangleIds.tri to triangleIds.tri
	std::string fileStr = path + "delaunayResults/returnStoreTriangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(3*sizeof(unsigned long int)); 
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	unsigned long int *pointIdArr = new unsigned long int[triangleNum*3];
	fread(pointIdArr, triangleNum*3, sizeof(unsigned long int), f);
	fclose(f);

	//remove returnTriangleCoors.tri, and returnTriangleIds.tri
	std::string delCommand = "rm -f " + path + "delaunayResults/returnStoreTriangleIds.tri";
	system(delCommand.c_str());


	//open triangleIds.tri for appending
	fileStr = path + "delaunayResults/triangleIds.tri";
	FILE *f1 = fopen(fileStr.c_str(), "a");
	if(!f1){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(pointIdArr, triangleNum*3, sizeof(unsigned long int), f1);
	fclose(f1);

	delete [] pointIdArr;
}

//=========================================================================================
//Process delaunay for active partitions on cluster. Each partition will be processed (delaunay) on a processor
//master node copies pointPartxx.ver and pointTriangle.tri for each partition over network
void domain::distributeDelaunay(){
	int activePartNum = activePartSet.size();
	std::string mpiCommand = "mpiexec -n " + std::to_string(activePartNum) + " -f machinefile ./delaunayMPI " + path + "delaunayResults/"; 
std::cout<<mpiCommand<<std::endl;
	system(mpiCommand.c_str());
}

//=========================================================================================
void domain::printTriangleArray(){
	for(int i=0; i<triangleArrSize; i++)
		std::cout<<i<<" "<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p1.getId()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p2.getId()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<" "<<triangleArr[i].p3.getId()<<"\n";

	std::cout<<"triangleArrSize: "<<triangleArrSize<<"\n";

}

//=========================================================================================
//return number of partitions that are unfinished
int domain::unfinishedPartNum(){
	int count = 0;
	for(int partId=0; partId<xPartNum*yPartNum; partId++)
		if(partArr[partId].finish==false) count++;
	return count;
}

//=========================================================================================
//return number of partitions that are unfinished
int domain::unDeliveredTriangleNum(){
	int count = 0;
	for(int partId=0; partId<triangleArrSize; partId++)
		if(triangleArr[partId].delivered==false) count++;
	return count;
}

//=========================================================================================
//When all partitions are processed via MPI, we need to store all trianlges left
void domain::storeAllTriangles(){

	//generate a tempPointIdArr containing those triangles left over (not belong to any active partition)
	//triangleArrSize is the number of triangles left over
	unsigned long int *tempPointIdArr = new unsigned long int[triangleArrSize*3];

	//copy ids of triangles left over from triangleArr to tempPointIdArr
	for(unsigned int index=0; index<triangleArrSize; index++){
		tempPointIdArr[index*3] = triangleArr[index].p1.getId();
		tempPointIdArr[index*3+1] = triangleArr[index].p2.getId();
		tempPointIdArr[index*3+2] = triangleArr[index].p3.getId();
	}
	//store tempPointIdArr to triangleIds.tri
	//open triangleIds.tri for appending
	std::string fileStr = path + "delaunayResults/triangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(tempPointIdArr, triangleArrSize*3, sizeof(unsigned long int), f);
	fclose(f);

	delete [] tempPointIdArr;
//	delete [] triangleArr;
//	triangleArr = NULL;
}

//===================================================================
void domain::drawTriangles(){
/*
The different color codes are
0   BLACK
1   BLUE
2   GREEN
3   CYAN
4   RED
5   MAGENTA
6   BROWN
7   LIGHTGRAY
8   DARKGRAY
9   LIGHTBLUE
10  LIGHTGREEN
11  LIGHTCYAN
12  LIGHTRED
13  LIGHTMAGENTA
14  YELLOW
15  WHITE
*/
	drawMesh *d = new drawMesh;
	d->	drawGridLines(xPartNum, yPartNum);
	d->drawTriangles(triangleList, 3);
	delete d;
}
//===================================================================
void domain::drawTriangleArr(){
	//read fullPointPart.ver to pointCoorArr
	unsigned int size = (pointNumMax+extensionPointArrSize);
	double *pointCoorArr = new double[size*2];
	std::string fileStr = path + "delaunayResults/fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fread(pointCoorArr, size*2, sizeof(double), f);
	fclose(f);

	//read triangle Ids (edges)
	std::string fileStr1 = path + "delaunayResults/triangleIds.tri";
	FILE *f1 = fopen(fileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}
	fseek(f1, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f1)/(3*sizeof(unsigned long int)); // get current file pointer
	fseek(f1, 0, SEEK_SET); // seek back to beginning of file

	unsigned long int *pointIdArr = new unsigned long int[triangleNum*3];
	fread(pointIdArr, triangleNum*3, sizeof(unsigned long int), f1);
	fclose(f1);

/*
for(int i=0; i<triangleNum; i++)
	std::cout<<pointIdArr[i*3]<<" "<<pointIdArr[i*3+1]<<" "<<pointIdArr[i*3+2]<<"\n";
*/
	//build storeTriangleArr
	triangle *storeTriangleArr = new triangle[triangleNum];
	for(int i=0; i<triangleNum; i++){
		unsigned int pointId1 = pointIdArr[i*3];
		unsigned int pointId2 = pointIdArr[i*3+1];
		unsigned int pointId3 = pointIdArr[i*3+2];

		storeTriangleArr[i].p1.setId(pointId1);
		storeTriangleArr[i].p2.setId(pointId2);
		storeTriangleArr[i].p3.setId(pointId3);

		storeTriangleArr[i].p1.setX(pointCoorArr[pointId1*2]);
		storeTriangleArr[i].p1.setY(pointCoorArr[pointId1*2+1]);

		storeTriangleArr[i].p2.setX(pointCoorArr[pointId2*2]);
		storeTriangleArr[i].p2.setY(pointCoorArr[pointId2*2+1]);

		storeTriangleArr[i].p3.setX(pointCoorArr[pointId3*2]);
		storeTriangleArr[i].p3.setY(pointCoorArr[pointId3*2+1]);
//std::cout<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<"\n";
		storeTriangleArr[i].computeCenterRadius();
	}

	drawMesh *d = new drawMesh;
	d->	drawGridLines(xPartNum, yPartNum);
	d->drawTriangleArr(storeTriangleArr, triangleNum, 2);//GREEN
//	if(triangleArr!=NULL) d->drawTriangleArr(triangleArr, triangleArrSize, 3);//CYAN

	delete d;

	delete [] pointCoorArr;
	delete [] pointIdArr;
	delete [] storeTriangleArr;
}

//===================================================================
void domain::drawTriangleArr1(){
	//read fullPointPart.ver to pointCoorArr
	unsigned int size = (pointNumMax+4);
	double *pointCoorArr = new double[size*2];
	std::string fileStr = path + "delaunayResults/fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fread(pointCoorArr, size*2, sizeof(double), f);
	fclose(f);

	drawMesh *d = new drawMesh;
	d->	drawGridLines(xPartNum, yPartNum);
	if(triangleArr!=NULL) d->drawTriangleArr(triangleArr, triangleArrSize, 3);//CYAN

	delete d;

	delete [] pointCoorArr;
}
//===================================================================
void domain::drawActivePartTriangles(){
	//triangle list
	triangleNode *triList = NULL;
	//prepare active triangles
	//scan all active partitions to update activePartIdArr, activePartSizeArr
	for (std::set<int>::iterator it=activePartSet.begin(); it!=activePartSet.end(); ++it){
		//process each active partition
		int partId = *it;
//std::cout<<"partition Id:"<<partId<<"\n";
//		if(partId==9){
			std::list<unsigned int> triangleIdList = partArr[partId].triangleIdList;
			for (std::list<unsigned int>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
				unsigned int index = *(it);
//std::cout<<"triangle index:"<<index<<"\n";
				triangle *newTriangle = new triangle(triangleArr[index]);
				triangleNode *newTriangleNode = createNewNode(newTriangle);
				insertFront(triList, newTriangleNode);
			}
//		}
	}


//std::cout<<"number of active partitions: "<<activePartSet.size()<<"\n";
//std::cout<<"number of triangles for all active partitions: "<<size(triList)<<"\n";

/*
	for(unsigned int triangleIndex=0; triangleIndex<triangleArrSize; triangleIndex++){
		if(triangleArr[triangleIndex].delivered){
			std::cout<<triangleIndex<<" ";
			triangle *newTriangle = new triangle(triangleArr[triangleIndex]);
			triangleNode *newTriangleNode = createNewNode(newTriangle);
			insertFront(triList, newTriangleNode);
		}
	}
*/
	drawMesh *d = new drawMesh;
	d->	drawGridLines(xPartNum, yPartNum);
	d->drawTriangles(triList, 3);
	delete d;
	removeLinkList(triList);
}
//===================================================================
domain::~domain(){
	if(initPointArr!=NULL) delete [] initPointArr;

	delete [] pointIdArr;

	//remove conflictPartList
	for(int index=0; index<xPartNum*yPartNum; index++){
		std::list<unsigned int> currList = conflictPartList[index];
		currList.clear();
	}
	delete [] conflictPartList;

	if(!activePartSet.empty()) activePartSet.clear();

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(int i=0; i<xPartNum*yPartNum; i++)
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

