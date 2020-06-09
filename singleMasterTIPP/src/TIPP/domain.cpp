#include "domain.h"
#include "io.h"

#include <iostream>
#include <fstream>


//==============================================================================
domain::domain(double lowX, double lowY, double highX, double highY, std::string srcPath, std::string dstPath){
//==============================================================================
	lowPoint.setX(lowX);
	lowPoint.setY(lowY);
	highPoint.setX(highX);
	highPoint.setY(highY);

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=4, then the domain is square between 0,0 - 4,4
	domainSize = getDomainSizeX();
	currActivePartNum = 0;

	geoBound.setLowPoint(point(lowX,lowY));
	geoBound.setHighPoint(point(highX,highY));

	inputPath = srcPath;
	outputPath = dstPath;

	vertexRecordSize = 2;

	pointPartInfoArr = NULL;
	initPointArr = NULL;
	triangleList = NULL;
	boundaryTriangleList = NULL;
	triangleArr = NULL;

	trianglePartitionList = NULL;
	partArr = NULL;

	//read info from file pointPartInfo.xfdl
	readPointPartFileInfo();


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

	//remove *.tri
	command("rm " + outputPath + "/*.tri");
	command("rm " + outputPath + "/temp*");
}

//==============================================================================
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

//==============================================================================
//read partition info 
//==============================================================================
void domain::readPointPartFileInfo(){
	//Read information from pointPartInfo.xfdl
	std::string fileInfoStr = inputPath + "/" + "pointPartInfo.xfdl";
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
	std::cout<<"=======================================================\n";
	std::cout<<"Triangualation "<<xPartNum<<" x "<<yPartNum<<" = "<<xPartNum*yPartNum<<", "<<inputPath<<std::endl;
	std::cout<<"=======================================================\n";

	pointNumMax = 0;
	pointNum = 0;
	int initPointNum = 0;
	int gridPointNum = 0;

	allocateMemory(pointPartInfoArr, unsigned int, xPartNum*yPartNum);
	//second line: number of points for each partition (without initial points)
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		vertexPartInfoFile >> strItem;
		pointPartInfoArr[i] = atoi(strItem.c_str());
if(pointPartInfoArr[i]!=0) std::cout<<pointPartInfoArr[i]<<" ";
		pointNumMax = pointNumMax + pointPartInfoArr[i];
	}

	allocateMemory(partArr, partition, xPartNum*yPartNum);

	//disable some fine partitions if they have no points
	std::cout<<"partitions that haven't any insertion points: ";
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		if(pointPartInfoArr[partId]==0){
			std::cout<<partId<<" ";
			partArr[partId].finish = true;
			partArr[partId].active = false;
		}
	}


std::cout<<"\n";
std::cout<<"Number of partition points (not include initial points): "<<pointNumMax<<"\n";

	//third line: the total number of initial points
	vertexPartInfoFile >> strItem;
	initPointNum = atoi(strItem.c_str());
	pointNumMax = pointNumMax + initPointNum;
	pointNum = pointNumMax;

std::cout<<"Number of initial points: "<<initPointNum<<"\n";
std::cout<<"Total number of points in domain: "<<pointNum<<"\n";

	//fourth line: number of grid points (not includes 4 corner points)
	vertexPartInfoFile >> strItem;
	gridPointNum = atoi(strItem.c_str());
	pointNumMax = pointNumMax + gridPointNum;
	vertexPartInfoFile.close();

	initPointArrSize = initPointNum + gridPointNum;

std::cout<<"Number of grid points (not include 4 corners): "<<gridPointNum<<"\n";
	
}

//==============================================================================
//load initial points (from each partition) and grid points (NOT include 4 corners) in file initPoints.ver
//each item of initPoints.ver is an object point
//load grid points on the domain are points on 4 edges AB, BC, CD, DA of square domain NOT including 4 corners.
//4 corner points of square domain are (0,0), (0,1), (1,1), (1,0)
//==============================================================================
void domain::loadInitPoints(){
	std::string fileStr = inputPath + "/initPoints.ver";
	readPoints(initPointArr, initPointArrSize, fileStr);
}

//===================================================================
int coorX_comparison(const void *p1, const void *p2){
	double x1 = ((point*)p1)->getX();
	double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//===================================================================
//all points are sorted before inserting to the Delaunay Triangulation
//===================================================================
void domain::initTriangulate(){
	//sort initExtendPointArr based on x coordinate of each point
	qsort(initPointArr, initPointArrSize, sizeof(point), coorX_comparison);
	std::cout<<"initPointArrSize: "<<initPointArrSize<<"\n";

	//init triangulate
	triangulateDomain(initPointArr, initPointArrSize, triangleList);
	releaseMemory(initPointArr);

	triangleNode *interiorTriangleList=NULL;
	unsigned long long *interiorTriangleIdArr=NULL;
	unsigned interiorTriangleNum = 0;


	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++)
		if(pointPartInfoArr[partId]==0){
			boundingBox currPartBBox = findPart(partId, lowPoint, highPoint, xPartNum, yPartNum);
			extractInteriorTriangles(currPartBBox, triangleList, interiorTriangleList);
			if(interiorTriangleList==NULL) continue;
			listToTriangleIdArr(interiorTriangleList, interiorTriangleIdArr, interiorTriangleNum);
			//write interiorTriangleList to file interiorTriangleIdsXX.tri
			std::string currPath = generateFileName(partId, outputPath + "/interiorTriangleIds", xPartNum*yPartNum, ".tri");
			storeTriangleIds(interiorTriangleIdArr, interiorTriangleNum, currPath, "a");
			removeLinkList(interiorTriangleList);
		}

}

//=========================================================================================
//transform link list of triangles (triangleList) into array of triangle (triangleArr)
void domain::triangleTransform(){

	//extract initial triangles in partitions that have no insert points
	triangleArrSize	= size(triangleList);

	if(triangleArrSize>1){
		allocateMemory(triangleArr, triangle, triangleArrSize);
		triangleNode *scanNode = triangleList;
		unsigned int index = 0;
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
	std::string fileStr = outputPath + "/initialDomainTriangles.tri";
	storeTriangleCoors(domainTriangleCoorArr, triangleArrSize, fileStr, "w");
	releaseMemory(domainTriangleCoorArr);
*/
}

//=========================================================================================
//generate all intersections between each triangle and partitions in the domain.
//each item trianglePartitionList[i] cotains a linklist of partitionIds 
//which are the intersection with current triangleArr[i]
//input: triangle array (triangleArr), and array of list (trianglePartitionList)
//output: trianglePartitionList which contains linklist of partitions for each of triangleId
//=========================================================================================
void domain::generateIntersection(){
	double globalLowX = geoBound.getLowPoint().getX();
	double globalLowY = geoBound.getLowPoint().getY();
    double gridElementSizeX = (geoBound.getHighPoint().getX() - geoBound.getLowPoint().getX())/xPartNum;
    double gridElementSizeY = (geoBound.getHighPoint().getY() - geoBound.getLowPoint().getY())/yPartNum;

	allocateMemory(trianglePartitionList, std::list<unsigned int>, triangleArrSize);

	//generate intersection
	for(unsigned long long index=0; index<triangleArrSize; index++){
		triangle t = triangleArr[index];
		double centerX = t.centerX;
		double centerY = t.centerY;
		double radius = t.radius;
		//Find a bounding box cover around the circle outside triangle
		boundingBox bBox(point(centerX-radius, centerY-radius), point(centerX+radius, centerY+radius));
//std::cout<<index<<" "<<triangleArrSize<<" lowPoint: "<<centerX-radius<<" "<<centerY-radius<<", highPoint: "<<centerX+radius<<" "<<centerY+radius<<std::endl;

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
//=========================================================================================
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
				int currPartId = *it1;
				for(std::list<unsigned int>::iterator it2=currList.begin(); it2 != currList.end(); ++it2){
					int conflictPartId = *it2;
					if(conflictPartId!=currPartId) 
						conflictPartList[currPartId].push_back(conflictPartId);
				}
			}

		}		
	}

	for(int index=0; index<xPartNum*yPartNum; index++){
		conflictPartList[index].sort();
		conflictPartList[index].unique();
	}

	std::cout<<"the status of partitions at first: [";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(!((partArr[i].finish==true)&&(partArr[i].active==false)))
		std::cout<<i<<"("<<(partArr[i].finish?"true":"false")<<","<<(partArr[i].active?"true":"false")<<")"<<"  ";
	std::cout<<"]\n";

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
//=========================================================================================
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
bool comparePartNode(partitionNode n1, partitionNode n2){
	return (n1.pointNum > n2.pointNum);
}

//=========================================================================================
//generate active partitions in the domain: all triangles in a active partition are not intersected with other active partitions 
//means that seaprating partitions and their intersected circumcircle 
//based on the list of partitions intersected with the circumcircle of the triangles
//generate partition that are active
//input: array of list (conflictPartList)
//output: set of partitions (partIds) (activePartSet)
//=========================================================================================
unsigned int domain::generateActivePartitions(){

	//Update active-inactive partitions
	//find a shortest list in partTempList, the number in order that is not in the confliced list, 
	//will be picked out as the active partition
	bool *activePartArr;
	allocateMemory(activePartArr, bool, xPartNum*yPartNum);

	//initialize activePartArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++) activePartArr[i] = false;

	//find the shortest list in partTempList
	unsigned int activePartId;
	unsigned int index = 0;
	//find the first unfinished partition, store in activePartId
	while(index<xPartNum*yPartNum)
		if(partArr[index].finish==false){
			activePartId = index;
			break;
		}
		else index++;


	//find the first active partition
	unsigned int unFinishSize = 1;
	//find the first active partitions based on the shortest conflicting list in conflictPartList.
	//find shortest conflicting list in conflictPartList --> high chance to have more active partitiions
	for(unsigned int index=activePartId+1; index<xPartNum*yPartNum; index++){
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
//	activePartArr[activePartId] = true;


	unsigned int unFinishCount=1;//1 means found the first active partition
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
		for(unsigned int index=0; index<xPartNum*yPartNum; index++)
			//the first item in array not updated yet
			if((partArr[index].finish==false)&&(partArr[index].active==true)&&(activePartArr[index]!=true)){
				activePartId = index;
				activePartArr[activePartId] = true;
				currActiveList.push_back(activePartId);
				unFinishCount++;
				break;
			}
		loopCount++;
		if(loopCount>=xPartNum*yPartNum) break;
	}


	std::cout<<"list of current active partitions based on currActiveList: [";
	for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it)
		std::cout<<(*it)<<" ";
	std::cout<<"]\n";

/*	//print a current list of active partitions
	std::cout<<"list of current active partitions based on partitions: [";//currActiveList
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if((partArr[i].active)&&(!partArr[i].finish)) std::cout<<i<<" ";
	std::cout<<"]\n";


	std::cout<<"the status of partitions: [";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(!((partArr[i].finish==true)&&(partArr[i].active==false)))
		std::cout<<i<<"("<<(partArr[i].finish?"true":"false")<<","<<(partArr[i].active?"true":"false")<<")"<<"  ";
	std::cout<<"]\n";
*/

	//clean up and update
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		//if a partition is unfinished and active, then set --> finish
		if(!partArr[i].finish){
			if(partArr[i].active) partArr[i].finish = true;
			else partArr[i].active = true;
		}


	//print a current list of unfinished partitions
	std::cout<<"list of unfinished partitions: [";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(!partArr[i].finish) std::cout<<i<<" ";
	std::cout<<"]\n\n";



	//clear all item in active partition set
	if(!activePartSet.empty()) activePartSet.clear();
	if(!activePartList.empty()) activePartList.clear();

	partitionNode partition;
	//after determining active partition list, build a set of active partition Ids for later use
	if(!currActiveList.empty())
		//scan all items in currActiveList to build activePartMap	
		for (std::list<unsigned int>::iterator it=currActiveList.begin(); it != currActiveList.end(); ++it){
			unsigned int activePartId = *it;
			activePartSet.insert(activePartId);
			partition.partId = activePartId;
			partition.pointNum = pointPartInfoArr[activePartId];
			activePartList.push_back(partition);
		}

	//sort currActiveList
	activePartList.sort(comparePartNode);

	delete [] activePartArr;
	return activePartSet.size();
}

//=========================================================================================
//deliver triangles to active partitions. For each triangle, find in the list of partitions that are 
//intersected with that triangle, if exist a partition Id belong to the active partition list (currActiveList)
//then set that triangle belong to the active partition. Each triangle belong to one active partition
void domain::deliverTriangles(){
	std::set<unsigned int>::iterator t;
std::cout<<"Number of active partitions: "<<activePartSet.size()<<"\n";

	//storeTriangleList used to store all triangles that are not intersected with any partition (because there are some inactive partitions)
	std::list<unsigned long long> storeTriangleIdList;

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
	unsigned int tempTriangleIdArrSize = storeTriangleIdList.size();
	if(tempTriangleIdArrSize==0) return;

	copyTriangleIds(tempTriangleIdArr, storeTriangleIdList, triangleArr);

	storeTriangleIds(tempTriangleIdArr, tempTriangleIdArrSize, outputPath + "/allBoundaryTriangleIds.tri", "a");
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
//processNum is the number of cores available in MPI system or PBS
//However, each time, send only processNum of active partitions to slave  nodes
//Assume if total number of active partitions is 20, but number of core available is 6,
//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
//Output is the total time to run MPI
//=========================================================================================
void domain::prepareDataForDelaunayMPI(unsigned int processNum){
	unsigned int *activePartIdArr;
	unsigned int *activePartSizeArr;
	unsigned int *activePartSizeOffsetArr;

	unsigned int totalActivePartNum = activePartSet.size();
	//activePartNum is current number of active partitions sending to MPI (slave nodes)
	if(processNum<totalActivePartNum)
		currActivePartNum = processNum;
	else currActivePartNum = totalActivePartNum;
	

	//array of active partition Ids 
	allocateMemory(activePartIdArr, unsigned int, currActivePartNum);
	allocateMemory(activePartSizeArr, unsigned int, currActivePartNum);
	allocateMemory(activePartSizeOffsetArr, unsigned int, currActivePartNum);

	unsigned int indexPartId = 0;
	unsigned long long totalTriangleSize = 0;

	std::list<unsigned long long> triangleIdList;
	unsigned long long triangleNum;

	//add activePartId in activePartSet to activePartIdArr
	unsigned int localIndexPartId = 0;
	bool stop = false;
	partitionNode part;
	while(!activePartList.empty()&&!stop){
		part = activePartList.front();
		unsigned int partId = part.partId;
		activePartIdArr[localIndexPartId] = partId;

		triangleIdList = partArr[partId].triangleIdList;
		//number of triangles belong to current partition
		triangleNum = triangleIdList.size();
		activePartSizeArr[localIndexPartId] = triangleNum;
		totalTriangleSize = totalTriangleSize + triangleNum;

		activePartList.pop_front();

		localIndexPartId++;
		if(localIndexPartId == currActivePartNum) break;
	}

	//take out active partId in activePartSet after add in to activePartIdArr
	for(unsigned int index=0; index<currActivePartNum; index++){
		activePartSet.erase(activePartIdArr[index]);
	}

	//store coordinates and pointId array to tempCoor.tri and tempPointId.tri
	storeActivePartitions(activePartIdArr, currActivePartNum, totalTriangleSize);

	std::cout<<"total active TriangleSize: "<<totalTriangleSize<<"\n";
	//update activePartSizeOffsetArr based on activePartSizeArr
	generateOffsetArr(activePartSizeArr, activePartSizeOffsetArr, currActivePartNum);
	//store info of active partitions to tempTriangles.xfdl
	storeActivePartitionInfo(currActivePartNum, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr);

	releaseMemory(activePartIdArr);
	releaseMemory(activePartSizeArr);
	releaseMemory(activePartSizeOffsetArr);
}


//=========================================================================================
//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
//coreNum is the number of cores available in MPI system or PBS
//However, each time, send only coreNum of active partitions to slave  nodes
//Assume if total number of active partitions is 20, but number of core available is 6,
//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
//Output is the total time to run MPI
void domain::prepareDataForDelaunayMPI_ProducerConsumer(){
	unsigned int *activePartIdArr;
	unsigned int *activePartSizeArr;
	unsigned int *activePartSizeOffsetArr;


	unsigned int totalActivePartNum = activePartSet.size();
	currActivePartNum = totalActivePartNum;

	allocateMemory(activePartIdArr, unsigned int, currActivePartNum);
	allocateMemory(activePartSizeArr, unsigned int, currActivePartNum);
	allocateMemory(activePartSizeOffsetArr, unsigned int, currActivePartNum);

	unsigned int indexPartId = 0;
	unsigned long long totalTriangleSize = 0;

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

	//store coordinates and pointId array to tempCoor.tri and tempPointId.tri
	storeActivePartitions(activePartIdArr, currActivePartNum, totalTriangleSize);

	std::cout<<"total active TriangleSize: "<<totalTriangleSize<<"\n";
	//update activePartSizeOffsetArr based on activePartSizeArr
	activePartSizeOffsetArr[0]=0;
	for(unsigned int i=1; i<currActivePartNum; i++)
		activePartSizeOffsetArr[i] = activePartSizeOffsetArr[i-1]+activePartSizeArr[i-1];

	//store info of active partitions to tempTriangles.xfdl
	storeActivePartitionInfo(currActivePartNum, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr);

	releaseMemory(activePartIdArr);
	releaseMemory(activePartSizeArr);
	releaseMemory(activePartSizeOffsetArr);
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
//=========================================================================================
void domain::storeActivePartitionInfo(unsigned int activePartNum, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr){
	//stores meta data
	std::string fileInfoStr = outputPath + "/tempTriangles.xfdl";
	std::ofstream infoFile(fileInfoStr, std::ofstream::out);
	//first line store number of active partition
	infoFile<<activePartNum<<"\n";
	//second line stores active partition ids
	for(unsigned int i=0; i<activePartNum; i++) 	infoFile<<activePartIdArr[i]<<" ";
	infoFile<<"\n";
	//third line stores number of triangles belong to active partitions
	for(unsigned int i=0; i<activePartNum; i++) 	infoFile<<activePartSizeArr[i]<<" ";
	infoFile<<"\n";
	//fourth line stores offsets of third line (number of triangles)
	for(unsigned int i=0; i<activePartNum; i++) 	infoFile<<activePartSizeOffsetArr[i]<<" ";
	infoFile<<"\n";
	//fifth line stores number of points in each active partition
	for(unsigned int i=0; i<activePartNum; i++) 	infoFile<<pointPartInfoArr[activePartIdArr[i]]<<" ";
	infoFile<<"\n";
	//sixth line stores xPartNum, yPartNum
	infoFile<<xPartNum<<" "<<yPartNum<<"\n";

	infoFile.close();
}


//=========================================================================================
//this function stores all triangles belong to current active partitions into files for further delaunay on cluster
//the file structure: (triangle1, triangle2, ...) --> {(p1, p2, p3), (p2, p5, p6), ....}
//each point: (x, y, Id)

//activePartIdArr contains all current active partition Ids
//activePartNum numer of current active partitions
//totalTriangleSize is number of triangles in all current active partitions (activePartNum)
//=========================================================================================
void domain::storeActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum, unsigned long long totalTriangleSize){

	//total triangles belong to all active partitions
	//tempTriangleArr will be send to nodes using MPI
	double *tempCoorArr;
	unsigned long long *tempPointIdArr;
	//copyTriangles(tempPointIdArr, tempCoorArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr);

	allocateMemory(tempCoorArr, double, totalTriangleSize*6);
	allocateMemory(tempPointIdArr, unsigned long long, totalTriangleSize*3);

	unsigned long long triangleIndex = 0;

	//scan all active partitions
	for(unsigned int activePartId=0; activePartId<activePartNum; activePartId++){
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

std::cout<<"number of triangles sends to MPI nodes: "<<totalTriangleSize<<"\n";

	//store tempCoorArr to file tempCoor.tri
	storeTriangleCoors(tempCoorArr, totalTriangleSize, outputPath + "/tempCoor.tri", "w");
	//store tempPointIdArr to file tempPointId.tri
	storeTriangleIds(tempPointIdArr, totalTriangleSize, outputPath + "/tempPointId.tri", "w");

	releaseMemory(tempCoorArr);
	releaseMemory(tempPointIdArr);
}

//=========================================================================================
//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//two files returnTriangleIds.tri and returnTriangleCoors.tri will be used to create an array of triangles
//and add those triangles to the current main triangle array triangleArr
//=========================================================================================
void domain::updateTriangleArr(){

	//determine number of undelivered triangles in triangleArr
	unsigned long long count=0;
	for(unsigned long long index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;

	//determine number of triangles that came from delaunayMPI
	//read triangles from two files returnTriangleCoors.tri, returnTriangleIds.tri 
	//and add to the remaining trianglesList
	std::string fileStr = outputPath + "/returnAllTriangleCoors.tri";
	unsigned long long triangleNum;
	double *tempCoorArr;
	readTriangleCoors(tempCoorArr, triangleNum, outputPath + "/returnAllTriangleCoors.tri");


	unsigned long long *tempPointIdArr;
	readTriangleIds(tempPointIdArr, triangleNum, outputPath + "/returnAllTriangleIds.tri");

	//remove returnTriangleCoors.tri, and returnTriangleIds.tri
	exeCommand("rm -f " + outputPath + "/returnAllTriangleCoors.tri");
	exeCommand("rm -f " + outputPath + "/returnAllTriangleIds.tri");

	//generate a new triangleArr including those triangles left over (not belong to any active partition) and
	//and return triangles that came from delaynayMPI

	//triangleNum is total number of triangles
	triangle *triangleNewArr;
	unsigned long long triangleNewArrSize = triangleNum + count;
	allocateMemory(triangleNewArr, triangle, triangleNewArrSize);

	//copy undelivered triangle from triangleArr to triangleNewArr
	unsigned long long index=0;
	for(unsigned long long i=0; i<triangleArrSize; i++)
		if(triangleArr[i].delivered==false){
			triangleNewArr[index] = triangleArr[i];
			index++;
		}

	generateTriangleArr(tempPointIdArr, tempCoorArr, triangleNum, &triangleNewArr[index]);
	releaseMemory(tempCoorArr);
	releaseMemory(tempPointIdArr);

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
	triangleArrSize = triangleNum + count;

std::cout<<"number of triangle after delaunay: "<<triangleArrSize<<"\n";

	//clean currActiveList
	if(!currActiveList.empty()) currActiveList.clear();

	//clean activePartSet
	if(!activePartSet.empty()) activePartSet.clear();

}

//=========================================================================================
void domain::addReturnTriangles(){
	//add triangles in returnTriangleIds.tri to file returnAllTriangleIds.tri, 
	addFile(outputPath, "returnTriangleIds.tri", "returnAllTriangleIds.tri");
	//and add returnTriangleCoors.tri to file returnAllTriangleCoors.tri
	addFile(outputPath, "returnTriangleCoors.tri", "returnAllTriangleCoors.tri");
}



//=========================================================================================
//process all independent partitions (for serial version)
//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
//coreNum is the number of cores available in MPI system or PBS
//=========================================================================================
void domain::prepareAndDelaunayIndependentPartitions(double &storeTime){
	storeTime = 0;
	unsigned int *activePartIdArr;
	unsigned int *activePartSizeArr;
	unsigned int *activePartSizeOffsetArr;

	unsigned int totalActivePartNum = activePartSet.size();
	currActivePartNum = totalActivePartNum;

	//array of active partition Ids 
	allocateMemory(activePartIdArr, unsigned int, currActivePartNum);
	allocateMemory(activePartSizeArr, unsigned int, currActivePartNum);
	allocateMemory(activePartSizeOffsetArr, unsigned int, currActivePartNum);


	unsigned int indexPartId = 0;
	unsigned long long totalTriangleSize = 0;

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

	refineActivePartitions(activePartIdArr, currActivePartNum, storeTime);

	releaseMemory(activePartIdArr);
	releaseMemory(activePartSizeArr);
	releaseMemory(activePartSizeOffsetArr);
}

//=========================================================================================
//triangle all independent partitions (for serial version only)
//each call pSerial->processSerial triangulate one partitions with input (activePartId, tempTriangleList)
//and output (returnInteriorTriangleList, returnBoundaryTriangleList)
//returnInteriorTriangleList, returnBoundaryTriangleList will be accumulated to interiorTriangleList, and boundaryTriangleList
//Output ofthis function are interiorTriangleList, and boundaryTriangleList
//=========================================================================================
void domain::refineActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum, double &storeTime){
	storeTime = 0;
	double currentTime;
	unsigned activePartId;
	delaunaySerial *pSerial = new delaunaySerial(xPartNum, yPartNum, domainSize, inputPath);
	triangleNode *interiorTriangleList=NULL, *tempTriangleList=NULL;
	triangleNode *returnInteriorTriangleList=NULL, *returnBoundaryTriangleList=NULL;


std::cout<<"activePartNum: "<<activePartNum<<"\n";
	//scan all active partitions
	for(unsigned int i=0; i<activePartNum; i++){
		activePartId = activePartIdArr[i];
		std::list<unsigned long long> triangleIdList = partArr[activePartId].triangleIdList;
		for(std::list<unsigned long long>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			triangle *newTriangle = new triangle(triangleArr[*it]);
			triangleNode *newTriangleNode = createNewNode(newTriangle);
			insertFront(tempTriangleList, newTriangleNode);
		}
		pSerial->processSerial(activePartId, tempTriangleList, returnInteriorTriangleList, returnBoundaryTriangleList);
		removeLinkList(tempTriangleList);

		if(returnInteriorTriangleList!=NULL) addLinkList(returnInteriorTriangleList, interiorTriangleList);
		if(returnBoundaryTriangleList!=NULL) addLinkList(returnBoundaryTriangleList, boundaryTriangleList);

		unsigned long interiorTriangleNum = size(interiorTriangleList);
		if(interiorTriangleNum>1000000){
			currentTime = GetWallClockTime();
			storeTriangleIdsList(interiorTriangleList);
			storeTime += GetWallClockTime() - currentTime;
			removeLinkList(interiorTriangleList);
		}
	}
	if(interiorTriangleList != NULL){
		currentTime = GetWallClockTime();
		storeTriangleIdsList(interiorTriangleList);
		storeTime += GetWallClockTime() - currentTime;
		removeLinkList(interiorTriangleList);
	}

/*	//store interior triangles (interiorTriangleList --> local)
	unsigned long interiorTriangleNum = size(interiorTriangleList);
	if(interiorTriangleNum <= 0) return;

	unsigned long long *interiorTriangleIdArr;
	allocateMemory(interiorTriangleIdArr, unsigned long long, interiorTriangleNum*3);

	unsigned long index = 0;
	triangleNode *currNode = interiorTriangleList;
	while(currNode != NULL){
		interiorTriangleIdArr[index*3] = currNode->tri->p1.getId();
		interiorTriangleIdArr[index*3+1] = currNode->tri->p2.getId();
		interiorTriangleIdArr[index*3+2] = currNode->tri->p3.getId();
		index++;
		currNode = currNode->next;
	}
	removeLinkList(interiorTriangleList);

	//store to triangleIds.tri
	double currentTime = GetWallClockTime();
	storeTriangleIds(interiorTriangleIdArr, interiorTriangleNum, outputPath + "/triangleIds.tri", "a");
	storeTime += GetWallClockTime() - currentTime;
	releaseMemory(interiorTriangleIdArr);
*/
	//process boundaryTriangleList (global) in updateTriangleArr()
}

//=========================================================================================
//append a triangleIdsList to triangleIds.tri
void domain::storeTriangleIdsList(triangleNode *triangleIdList){
	//store interior triangles (interiorTriangleList --> local)
	unsigned long interiorTriangleNum = size(triangleIdList);
	if(interiorTriangleNum <= 0) return;

	unsigned long long *interiorTriangleIdArr;
	allocateMemory(interiorTriangleIdArr, unsigned long long, interiorTriangleNum*3);

	unsigned long index = 0;
	triangleNode *currNode = triangleIdList;
	while(currNode != NULL){
		interiorTriangleIdArr[index*3] = currNode->tri->p1.getId();
		interiorTriangleIdArr[index*3+1] = currNode->tri->p2.getId();
		interiorTriangleIdArr[index*3+2] = currNode->tri->p3.getId();
		index++;
		currNode = currNode->next;
	}
	storeTriangleIds(interiorTriangleIdArr, interiorTriangleNum, outputPath + "/triangleIds.tri", "a");
	releaseMemory(interiorTriangleIdArr);
}

//=========================================================================================
//for serial version only
//get returned triangles which are processed from Serial, remove those triangles that are delivered, 
//update to the main triangleArr, and ready for the next stages of triangulation
//two files returnTriangleIds.tri and returnTriangleCoors.tri will be used to create an array of triangles
//and add those triangles to the current main triangle array triangleArr
//=========================================================================================
void domain::updateTriangleArrForSerial(){

	//determine number of undelivered triangles in triangleArr
	unsigned long long count=0;
	for(unsigned long long index=0; index<triangleArrSize; index++)
		if(triangleArr[index].delivered==false) count++;


	//process boundaryTriangleList (global)
	unsigned long long boundaryTriangleNum = size(boundaryTriangleList);
std::cout<<"boundary triangle num: "<<boundaryTriangleNum<<"\n";

	//triangleNum is total number of triangles
	triangle *triangleNewArr;
	allocateMemory(triangleNewArr, triangle, boundaryTriangleNum + count)

	//copy undelivered triangle from triangleArr to triangleNewArr
	unsigned long long index=0;
	for(unsigned long long i=0; i<triangleArrSize; i++)
		if(triangleArr[i].delivered==false){
			triangleNewArr[index] = triangleArr[i];
			index++;
		}

	triangleNode *currNode = boundaryTriangleList;
	while(currNode != NULL){
		triangleNewArr[index].delivered = false;
		triangleNewArr[index].p1.set(currNode->tri->p1); 
		triangleNewArr[index].p2.set(currNode->tri->p2); 
		triangleNewArr[index].p3.set(currNode->tri->p3); 
		triangleNewArr[index].computeCenterRadius();
		index++;

		currNode = currNode->next;
	}
	removeLinkList(boundaryTriangleList);


	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(partArr[i].finish) partArr[i].triangleIdList.clear();

	releaseMemory(triangleArr);
	triangleArr = triangleNewArr;

	//clean up trianglePartitionList
	for(unsigned  long long i=0; i<triangleArrSize; i++) trianglePartitionList[i].clear();
	releaseMemory(trianglePartitionList);
	triangleArrSize = boundaryTriangleNum + count;

std::cout<<"number of triangle after delaunay: "<<triangleArrSize<<"\n";

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
	copyTriangleIdsFromTriangleArr(tempPointIdArr, triangleArrSize, triangleArr);

	//open allBoundaryTriangleIds.tri for appending
	storeTriangleIds(tempPointIdArr, triangleArrSize, outputPath + "/allBoundaryTriangleIds.tri", "a");
	releaseMemory(tempPointIdArr);
}

//===================================================================
//combine many small file results (triangleIdXX.tri --> triangleIds.tri)
//Add all *.tri files into one big file triangleIds.tri
void domain::combineFiles(){
	std::string currPath1, currPath2;
	currPath2 = outputPath + "/triangleIds.tri";
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		currPath1 = generateFileName(partId, outputPath + "/interiorTriangleIds", xPartNum*yPartNum, ".tri");
		appendFile1(currPath1, currPath2);
	}
	appendFile(outputPath + "/allBoundaryTriangleIds.tri", currPath2);
	
}

//===================================================================
domain::~domain(){
	//remove conflictPartList
	for(int index=0; index<xPartNum*yPartNum; index++){
		std::list<unsigned int> currList = conflictPartList[index];
		currList.clear();
	}
	releaseMemory(conflictPartList);

	if(!activePartSet.empty()) activePartSet.clear();
	if(!activePartList.empty()) activePartList.clear();

	//clean up active partitions: partArr, currActiveList, activePartSet, triangleArr and trianglePartitionList
	//clean partArr
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		partArr[i].triangleIdList.clear();

	releaseMemory(partArr);
	releaseMemory(triangleArr);
	releaseMemory(pointPartInfoArr);

	//remove temporary files
	exeCommand("rm " + outputPath + "/temp*");
}


