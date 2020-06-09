#include "domain.h"
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
	triangleArr = NULL;

	trianglePartitionList = NULL;
	partArr = NULL;
	activePartArr = NULL;

	//read info from file pointPartInfo.xfdl
	readPointPartFileInfo();

	allocateMemory(partArr, partition, xPartNum*yPartNum);

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

	//fifth line --> number of point in each coarse partitions
	pointCoarsePartNum = new unsigned int[xPartNum*yPartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xPartNum*yPartNum; coarsePartId++){
		vertexPartInfoFile >> strItem;
		pointCoarsePartNum[coarsePartId] = atoi(strItem.c_str());
	}


std::cout<<"Number of initial points: "<<initPointNum<<"\n";
std::cout<<"Total number of points in domain: "<<pointNum<<"\n";

	vertexPartInfoFile.close();

	initPointArrSize = initPointNum + gridPointNum;

std::cout<<"Number of grid points (not include 4 corners): "<<gridPointNum<<"\n";
	
}

//==============================================================================
//load initial points (from each partition) and grid points (NOT include 4 corners) in file initDomainPoints.ver
//each item of initDomainPoints.ver is an object point
//load grid points on the domain are points on 4 edges AB, BC, CD, DA of square domain NOT including 4 corners.
//4 corner points of square domain are (0,0), (0,1), (1,1), (1,0)
void domain::loadInitPoints(){

	std::string fileStr = inputPath + "/initDomainPoints.ver";
	readPoints(initPointArr, initPointArrSize, fileStr);
}

//===================================================================
//all points are sorted before inserting to the Delaunay Triangulation
void domain::initTriangulate(){

	//sort initExtendPointArr based on x coordinate of each point
	qsort(initPointArr, initPointArrSize, sizeof(point), coorX_comparison);

	//init triangulate
	triangulateDomain(initPointArr, initPointArrSize, triangleList);
	releaseMemory(initPointArr);
}

//===================================================================
//transform link list of triangles (triangleList) into array of triangle (triangleArr)
void domain::triangleTransform(){

	triangleArrSize	= size(triangleList);
	if(triangleArrSize>1){
		allocateMemory(triangleArr, triangle, triangleArrSize);
		triangleNode *scanNode = triangleList;
		unsigned long long index = 0;
		while(scanNode!=NULL){
			triangleArr[index] = *(scanNode->tri);
			index++;
			scanNode = scanNode->next;
		}	
	}
	removeLinkList(triangleList);

/*	//This code is written for visualization
	double *domainTriangleCoorArr;
	copyTriangleCoorsFromTriangleArr(domainTriangleCoorArr, triangleArrSize, triangleArr);
	std::string fileStr = outputPath + "/domainTriangles.tri";
	storeTriangleCoors(domainTriangleCoorArr, triangleArrSize, fileStr, "w");
	releaseMemory(domainTriangleCoorArr);
*/
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

	allocateMemory(trianglePartitionList, std::list<unsigned int>, triangleArrSize);

	//generate intersection
	for(unsigned int index=0; index<triangleArrSize; index++){
		triangle t = triangleArr[index];
		double centerX = t.centerX;
		double centerY = t.centerY;
		double radius = t.radius;
		//Find a bounding box cover around the circle outside triangle
		boundingBox bBox(point(centerX-radius, centerY-radius), point(centerX+radius, centerY+radius));

		//gridBox that intersects with the bounding box of a triangle
		gridBound gb = boundingGrid(bBox, geoBound, xPartNum, yPartNum);

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

/*
	//print intersections for each triangle
	for(unsigned int index=0; index<triangleArrSize; index++){
		std::list<unsigned int> currList = trianglePartitionList[index];
		if(currList.size()<=4) continue;
		std::cout<<"partitions intersec with triangle "<<index<<": ";
		std::cout<<triangleArr[index].p1<<triangleArr[index].p2<<triangleArr[index].p3;
		std::cout<<"%%%%"<<partIndex(triangleArr[index].p1)<<" "<<partIndex(triangleArr[index].p2)<<" "<<partIndex(triangleArr[index].p3)<<"\n";
		triangle t = triangleArr[index];
		double centerX = t.centerX;
		double centerY = t.centerY;
		double radius = t.radius;
		std::cout<<index<<" "<<triangleArrSize<<" lowPoint: "<<centerX-radius<<" "<<centerY-radius<<", highPoint: "<<centerX+radius<<" "<<centerY+radius<<std::endl;
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
//use this function (printConflictPartitions) to print the detail
void domain::generateConflictPartitions(){
	//partTempList is an array of linklist, used to contain all conflictions of each partition in domain
	//two partitions are conflicted when exist a triangle whose circumcircle intersected with both partitions 
	allocateMemory(conflictPartList, std::list<unsigned int>, xPartNum*yPartNum);

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
}

//=========================================================================================
//generate active partitions in the domain: all triangles in a active partition are not intersected with other active partitions 
//means that seaprating partitions and their intersected circumcircle 
//based on the list of partitions intersected with the circumcircle of the triangles
//generate partition that are active
//input: array of list (conflictPartList)
//output: set of partitions (partIds) (activePartSet)
unsigned int domain::generateActivePartitions(){

	//Update active-inactive partitions
	//find a shortest list in partTempList, the number in order that is not in the confliced list, 
	//will be picked out as the active partition
	bool *localActivePartArr;
	allocateMemory(localActivePartArr, bool, xPartNum*yPartNum);

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


	//print a current list of active partitions
	std::string msg = "[[[[[[[[list of current coarse active partitions: [";//currActiveList
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if((partArr[i].active)&&(!partArr[i].finish)) msg = msg + " " + toString(i) + " ";
	msg = msg + "]\n"; 
	std::cout<<msg;

	//clean up and update
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		//if a partition is unfinished and active, then set --> finish
		if(!partArr[i].finish){
			if(partArr[i].active) partArr[i].finish = true;
			else partArr[i].active = true;
		}

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
	return activePartSet.size();

}

//=========================================================================================
//deliver triangles to active partitions. For each triangle, find in the list of partitions that are 
//intersected with that triangle, if exist a partition Id belong to the active partition list (currActiveList)
//then set that triangle belong to the active partition. Each triangle belong to one active partition
void domain::deliverTriangles(){
	std::set<unsigned>::iterator t;

	//storeTriangleList used to store all triangles that are not intersected with any partition (because there are some inactive partitions)
	std::list<unsigned> storeTriangleIdList;

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

	//store all point ids of triangleId in storeTriangleList to file
	unsigned long long *tempTriangleIdArr;
	unsigned tempTriangleIdArrSize = storeTriangleIdList.size();
	if(tempTriangleIdArrSize==0) return;

	copyTriangleIdArr(tempTriangleIdArr, storeTriangleIdList, triangleArr);

	//store data to file
//	storeTriangleIds(tempTriangleIdArr, tempTriangleIdArrSize, outputPath + "/allBoundaryTriangleIds.tri", "w");
	storeTriangleIds(tempTriangleIdArr, tempTriangleIdArrSize, outputPath + "/triangleIds.tri", "a");
	releaseMemory(tempTriangleIdArr);
	storeTriangleIdList.clear();

}

//=========================================================================================
//number of active partition left over in activePartSet
unsigned int domain::activePartitionNumber(){
	return activePartSet.size();
}

//=========================================================================================
//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
//coreNum is the number of cores available in MPI system or PBS
//However, each time, send only coreNum of active partitions to slave  nodes
//Assume if total number of active partitions is 20, but number of core available is 6,
//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
//Output: current number of active partitions
unsigned int domain::prepareDataForDelaunayMPI(unsigned int processNum, unsigned int *&activePartPointSizeArr){

	unsigned int *activePartSizeArr;
	unsigned int *activePartIdArr;

	unsigned int totalActivePartNum = activePartSet.size();
	//activePartNum is current number of active partitions sending to MPI (slave nodes)
	if(processNum<totalActivePartNum)
		currActivePartNum = processNum;
	else currActivePartNum = totalActivePartNum;

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
	for(unsigned int index=0; index<currActivePartNum; index++)
		activePartPointSizeArr[index] = pointCoarsePartNum[activePartIdArr[index]];
	
	//take out active partId in activePartSet after add in to activePartIdArr
	for(unsigned int index=0; index<currActivePartNum; index++)
		activePartSet.erase(activePartIdArr[index]);

	//store coordinates and pointId array to tempCoor.tri and tempPointId.tri
	std::cout<<"Number of active partition: "<<currActivePartNum<<"\n";
	storeActivePartitions(activePartIdArr, currActivePartNum);

	//store info of active partitions to tempTriangles.xfdl
	storeActivePartitionInfo(currActivePartNum, activePartIdArr, activePartSizeArr);

	releaseMemory(activePartIdArr);
	releaseMemory(activePartSizeArr);

	return currActivePartNum;
}

//=========================================================================================
//this function stores all triangles belong to current active partition into file for further delaunay on cluster
//the file structure: (triangle1, triangle2, ...) --> {(p1, p2, p3), (p2, p5, p6), ....}
//each point: (x, y, Id)

//activePartIdArr contains all current active partition Ids
//activePartNum numer of current active partitions
void domain::storeActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum){


	//tempTriangleArr will be send to nodes using MPI
	//scan all active partitions
	for(unsigned int activePartId=0; activePartId<activePartNum; activePartId++){

		//for each active partition, take out all triangles and store to files tempCoorXX.tri and tempPointIdXX.tri, XX is activePartId
		std::list<unsigned long long> triangleIdList = partArr[activePartIdArr[activePartId]].triangleIdList;
		double *tempCoorArr;
		unsigned long long *tempPointIdArr;
		//based on triangleIdList and triangleArr, copy triangleIds and triangleCoors to 
		copyTriangles(tempPointIdArr, tempCoorArr, triangleIdList, triangleArr);

		unsigned long long triangleNum = triangleIdList.size();
		//store tempCoorArr to file tempCoorCoarsePartsXX.tri
		std::string fileStr = generateFileName(activePartId, outputPath + "/tempCoorCoarseParts", activePartNum, ".tri");
		storeTriangleCoors(tempCoorArr, triangleNum, fileStr, "w");

		//store tempPointIdArr to file tempPointIdCoarsePartsXX.tri
		fileStr = generateFileName(activePartId, outputPath + "/tempPointIdCoarseParts", activePartNum, ".tri");
		storeTriangleIds(tempPointIdArr, triangleNum, fileStr, "w");

		releaseMemory(tempCoorArr);
		releaseMemory(tempPointIdArr);
	}
}

//=========================================================================================
//store info of active partitions to tempTriangles.xfdl
//The info consists of:
//	- number of current active partitions, 
//	- active partition ids
//	- number of triangles belong to active partitions
void domain::storeActivePartitionInfo(unsigned int activePartNum, unsigned int *activePartIdArr, unsigned int *activePartSizeArr){

	//stores meta data
	std::string fileInfoStr = outputPath + "/" + "tempTrianglesCoarseParts.xfdl";
	std::ofstream infoFile(fileInfoStr, std::ofstream::out);
	//first line store number of active partition
	infoFile<<activePartNum<<"\n";
	//second line stores active partition ids
	for(unsigned int i=0; i<activePartNum; i++) 	infoFile<<activePartIdArr[i]<<" ";
	infoFile<<"\n";
	//third line stores number of triangles belong to active partitions
	for(unsigned int i=0; i<activePartNum; i++) 	infoFile<<activePartSizeArr[i]<<" ";
	infoFile<<"\n";
	//fourth line stores xPartNum, yPartNum
	infoFile<<xPartNum<<" "<<yPartNum<<"\n";

	infoFile.close();

}

//=========================================================================================
//get returned triangles which are processed from MPI of each coarse-grained partition, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//add triangles in file boundaryTrianglesXX.tri (XX are active partition Ids) to triangleArr.
//and append storedTriangleIdsXX.tri to main file triangleIds.tri
//use activePartArr (contains all active partition IDs for a stage) to find all XX files
void domain::updateTriangleArr(){

	//determine number of undelivered triangles in triangleArr
	unsigned long long count=0;
	for(unsigned long long index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;

	std::string fileStr1 = outputPath + "/boundaryIds.tri";
	std::string fileStr2 = outputPath + "/boundaryCoors.tri";

	unsigned long long *returnBoundaryTriangleIdArr;
	double *returnBoundaryTriangleCoorArr;
	unsigned long long boundaryTriangleNum = 0;

	readTriangleIds(returnBoundaryTriangleIdArr, boundaryTriangleNum, fileStr1);
	readTriangleCoors(returnBoundaryTriangleCoorArr, boundaryTriangleNum, fileStr2);

	command("rm -rf " + fileStr1);
	command("rm -rf " + fileStr2);

	unsigned long long totalBoundaryTriangleArrSize = 0;
	totalBoundaryTriangleArrSize = boundaryTriangleNum;

	//triangleNum is total number of triangles
	triangle *triangleNewArr;
	allocateMemory(triangleNewArr, triangle, totalBoundaryTriangleArrSize + count);

	//copy undelivered triangle from triangleArr to triangleNewArr
	unsigned long long index=0;
	for(unsigned long long i=0; i<triangleArrSize; i++)
		if(triangleArr[i].delivered==false){
			triangleNewArr[index] = triangleArr[i];
			index++;
		}

	//populate triangles for the rest of triangles in triangleNewArr
	generateTriangleArr(returnBoundaryTriangleIdArr, returnBoundaryTriangleCoorArr, boundaryTriangleNum, &triangleNewArr[index]);
	releaseMemory(returnBoundaryTriangleIdArr);
	releaseMemory(returnBoundaryTriangleCoorArr);

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(partArr[i].finish) partArr[i].triangleIdList.clear();

	//process triangleArr
	releaseMemory(triangleArr);

	triangleArr = triangleNewArr;

	//clean up trianglePartitionList
	for(unsigned long long i=0; i<triangleArrSize; i++) trianglePartitionList[i].clear();
	releaseMemory(trianglePartitionList);

	triangleArrSize = totalBoundaryTriangleArrSize + count;

	//clean currActiveList
	if(!currActiveList.empty()) currActiveList.clear();

	//clean activePartSet
	if(!activePartSet.empty()) activePartSet.clear();
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
	allocateMemory(tempPointIdArr, unsigned long long, triangleArrSize*3);

	//copy ids of triangles left over from triangleArr to tempPointIdArr
	for(unsigned long long index=0; index<triangleArrSize; index++){
		tempPointIdArr[index*3] = triangleArr[index].p1.getId();
		tempPointIdArr[index*3+1] = triangleArr[index].p2.getId();
		tempPointIdArr[index*3+2] = triangleArr[index].p3.getId();
	}
	//store tempPointIdArr to triangleIds.tri
	//open triangleIds.tri for appending
	std::string fileStr = outputPath + "/triangleIds.tri";
	storeTriangleIds(tempPointIdArr,triangleArrSize, fileStr, "a");
	releaseMemory(tempPointIdArr);
}

//===================================================================
//count number of minor partitions (fine partitions)
unsigned int domain::countFinePartitions(){
	unsigned int count = 0;
	unsigned int xFinePartNum;
	for(unsigned int coarsePartId=0; coarsePartId<xPartNum*yPartNum; coarsePartId++){
		if(pointCoarsePartNum[coarsePartId]!=0){
			//read number of partitions
			std::string fileInfoStr = generateFileName(coarsePartId, inputPath + "/pointPartInfo", xPartNum*yPartNum, ".xfdl");
			std::ifstream vertexPartInfoFile(fileInfoStr.c_str());
			if(!vertexPartInfoFile){
				std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
				exit(1);
			}
			std::string strItem;
			//first line --> read xPartNum, yPartNum
			vertexPartInfoFile >> strItem;
			xFinePartNum = atoi(strItem.c_str());
			vertexPartInfoFile.close();
			count += xFinePartNum*xFinePartNum;
		}
	}

	return count;
}
//===================================================================
domain::~domain(){

	releaseMemory(pointCoarsePartNum);

	//remove conflictPartList
	for(unsigned int index=0; index<xPartNum*yPartNum; index++){
		std::list<unsigned int> currList = conflictPartList[index];
		currList.clear();
	}
	releaseMemory(conflictPartList);

	if(!activePartSet.empty()) activePartSet.clear();
	releaseMemory(activePartArr);

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		partArr[i].triangleIdList.clear();
	releaseMemory(partArr);

	//process triangleArr
	releaseMemory(triangleArr);

	//remove temporary files
	std::string delCommand = "rm " + outputPath + "/temp*";
	system(delCommand.c_str());
}

