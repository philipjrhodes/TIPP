//https://github.com/qhull/qhull/blob/master/html/rbox.man
#include "nonUniformDistribute.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//xPartNumber, yPartNumber are number of columns and rows for partitioning all point dataset
//mapScale is a number to scale original map points (31345 points)
//scalePartitionX, scalePartitionY are number of columns and rows to partitiong for scaling up. It means that original points will be partitioned,
//then each partittion will be scaled up using mapScale. Total number of points now can be (can be 500 million)
//The last step is to partitioning the point domian after scaling (500 million points) using xPartNumber, yPartNumber
//===================================================================================================================
distribute::distribute(unsigned int xPartNumber, unsigned int yPartNumber, unsigned long long totalNumberPoints, unsigned int initialPointNum, std::string resultPath){
	xPartNum = xPartNumber;
	yPartNum = yPartNumber;

	domainLowPoint.setX(0);
	domainLowPoint.setY(0);
	domainHighPoint.setX(xPartNum);
	domainHighPoint.setY(yPartNum);

	totalInputPointNum = totalNumberPoints;

	initPointNum = initialPointNum;
	std::cout<<"================================================\n";
	std::cout<<"xPartNumber: "<<xPartNum<<"   yPartNumber: "<<yPartNum<<std::endl;
	std::cout<<"================================================\n";

	globalPointIndex = 0;

	dataMapFile = "./src/northCarolinaMap/nc_inundation_v6c.grd";
	destPath = resultPath;

	mapPointData = NULL;
	scalePartInfoArr = new unsigned int[xPartNum*xPartNum];
	for(int i=0; i<xPartNum*xPartNum; i++) scalePartInfoArr[i] = 0;

	pointPartInfoArr = new unsigned int[xPartNum*yPartNum];
	for(int i=0; i<xPartNum*yPartNum; i++) pointPartInfoArr[i] = 0;

	partList = new std::list<point>[xPartNum*yPartNum];

	pointCoorArr = NULL;
	pointGridArr = NULL;
	gridPoints = NULL;
	gridPointsSize = 0;
}

//==============================================================================
//read mapFile to mapData and map to the domain [0,0 - xPartNum, yPartNum] 
void distribute::readMapData(){
	std::string mapFileStr = dataMapFile;
	std::ifstream readMapFile(mapFileStr.c_str());
	std::string strItem;
	if(!readMapFile){
		std::cout<<"There is no filename : "<<dataMapFile;
		exit(1);
	}

	//skip the first and second item line because it is the file name and the number of triangulation
	readMapFile >> strItem;
	readMapFile >> strItem;

    //third item of input contains number of points
	readMapFile >> strItem;
	mapPointNum = atoll(strItem.c_str());
	scale = (double)totalInputPointNum/mapPointNum;

	double maxX, minX, maxY, minY, x, y;
	readMapFile >> strItem;
	readMapFile >> strItem;
	x = atof(strItem.c_str());
	readMapFile >> strItem;
	y = atof(strItem.c_str());
	readMapFile >> strItem;
	maxX = x; maxY = y;
	minX = x; minY = y;

	mapPointData = new double[mapPointNum*2];
	mapPointData[0] = x;
	mapPointData[1] = y;

	for(unsigned int i=1; i<mapPointNum; i++){
		readMapFile >> strItem;
		readMapFile >> strItem;
		x = atof(strItem.c_str());
		mapPointData[i*2] = x;
		if(maxX < x) maxX = x;
		if(minX > x) minX = x;

		readMapFile >> strItem;
		y = atof(strItem.c_str());
		mapPointData[i*2+1] = y;
		if(maxY < y) maxY = y;
		if(minY > y) minY = y;
		readMapFile >> strItem;
	}

	double deltaX = maxX-minX;
	double deltaY = maxY-minY;
	for(unsigned int i=0; i<mapPointNum; i++){
		mapPointData[i*2] = (mapPointData[i*2]-minX)*xPartNum/deltaX;
		mapPointData[i*2+1] = (mapPointData[i*2+1]-minY)*yPartNum/deltaY;
	}
}

//==============================================================================
//After reading mapData, all points in the map domain are partitioned 
//number of points after scaling store in scalePartInfoArr
void distribute::scaleMapData(){
	unsigned int partionIndex;

	std::cout<<"mapPointNum: "<<mapPointNum<<"\n";
	for(unsigned int i=0; i<mapPointNum; i++){
		partionIndex = partIndex(point(mapPointData[i*2], mapPointData[i*2+1]), domainLowPoint, domainHighPoint, xPartNum, yPartNum);
		scalePartInfoArr[partionIndex]++;
	}

	std::cout<<"number of original points in map for each scale partition\n";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		std::cout<<scalePartInfoArr[i]<<" ";
	}

	std::cout<<"\n\nnumber of points after scaling in each of scale partition:\n";
	unsigned int emptyPartNum = 0;
	unsigned long long totalPoints = 0;
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		scalePartInfoArr[i] = scalePartInfoArr[i]*scale;
		totalPoints += scalePartInfoArr[i];
		std::cout<<scalePartInfoArr[i]<<" ";
		if(scalePartInfoArr[i]==0) emptyPartNum++;
	}
	std::cout<<"\ntotal number of points in domain: "<<totalPoints<<"\n";
	std::cout<<"number of empty partitions in domain: "<<emptyPartNum<<"\n";
	std::cout<<"the percent of empty over non-empty partitions: "<<emptyPartNum*100/(xPartNum*yPartNum)<<"%\n";
}

//==============================================================================
//return partition index in a domain
//input: lowPoint (low left point), highPoint (high right point), 
//		 number of partitions for two sides of domain (xPartNum, yPartNum), ex: 4x4 --> 16
//		 The point P.
//output: the number Id of the partition which the point P belong to
//This function works for any level of partitions (coarse-grained partitions or fine-grained partitions)
//==============================================================================
unsigned int distribute::partIndex(point p, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum){
	double x = p.getX();
	double y = p.getY();

	double lowPointX = lowPoint.getX();
	double lowPointY = lowPoint.getY();
	double highPointX = highPoint.getX();
	double highPointY = highPoint.getY();

	double xPartSize = (highPointX - lowPointX)/xPartNum;
	double yPartSize = (highPointY - lowPointY)/yPartNum;

	unsigned int gridX = (x - lowPointX)/xPartSize;
	if(gridX>=xPartNum) gridX = gridX-1;

	unsigned int gridY = (y - lowPointY)/yPartSize;
	if(gridY>=xPartNum) gridY = gridY-1;

	return gridY*xPartNum + gridX;
}

//==============================================================================
//given an index, find a bounding box of a partition in a domain
//input: + partIndex --> partition index of a partition in the domain 
//		 + lowPoint, highPoint --> the domain points
//		 + xPartNum, yPartNum --> granularity of partitions in the domain
//output: the bounding box of cuurent partition
boundingBox distribute::findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum){
	double lowPointX = lowPoint.getX();
	double lowPointY = lowPoint.getY();
	double highPointX = highPoint.getX();
	double highPointY = highPoint.getY();

	double xPartSize = (highPointX - lowPointX)/xPartNum;
	double yPartSize = (highPointY - lowPointY)/yPartNum;

	unsigned int gridX = partIndex % xPartNum;
	unsigned int gridY = partIndex / yPartNum;

	point returnLowPoint(lowPointX+gridX*xPartSize, lowPointY+gridY*yPartSize);
	point returnHighPoint(returnLowPoint.getX() + xPartSize, returnLowPoint.getY() + yPartSize);
	return boundingBox(returnLowPoint, returnHighPoint);
}


//==============================================================================
double distribute::drand(const double min, const double max){
    return (max - min) * static_cast<double>(rand()) / static_cast<double>(RAND_MAX)+min;
}

//==============================================================================
int int_compar(const void *p1, const void *p2)
{
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//convert vertext file (coordinates) from text to binary, also create a make up attribute file
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void distribute::readCoorPart(double *&pointCoorArr, unsigned int &pointNum){
	std::string vertexTextFileName = destPath + "mydata.ver";
	std::ifstream instreamVertex(vertexTextFileName.c_str());
	std::string strItem, strItem1, strItem2;

	if(!instreamVertex){
		std::cout<<"There is no filename : "<<vertexTextFileName;
		return;
	}

    // First line of input contains dimensionality
	instreamVertex >> strItem;

	// Second line contains number of records
	instreamVertex >> strItem;
	unsigned long long recordCount = atoll(strItem.c_str());
	unsigned long long fileSize = recordCount*2;

	pointCoorArr = new double[recordCount*2];

//cout.precision(16);

	double valX, valY;
	double epsilon = 0.000001;
	//load content from text file to array data
	unsigned long long int fileIndex = 0;
	unsigned int bufferIndex = 0;

	while(!instreamVertex.eof()&&(fileIndex<fileSize)){
		instreamVertex >> strItem1;
		instreamVertex >> strItem2;
		//read coordinates x and y of a point
		valX = std::stod(strItem1.c_str());
		valY = std::stod(strItem2.c_str());
		if((1.0-valX>epsilon)&&(1.0-valY>epsilon)&&(valX>epsilon)&&(valY>epsilon)){
			pointCoorArr[bufferIndex] = valX;
			bufferIndex++;
			pointCoorArr[bufferIndex] = valY;
			bufferIndex++;
		}
		else { 
			std::cout<<"Closed pointX = "<<valX<<", closed pointY = "<<valY<<std::endl;
		}
		fileIndex +=2;
	}
	instreamVertex.close();
	pointNum = bufferIndex/2;

//std::cout<<fileIndex<<" "<<bufferIndex<<" "<<fileSize<<std::endl;
}

//==============================================================================
void distribute::generatePoints(boundingBox partBox, double *&pointCoorArr, unsigned int &pointPartNum){
	if(pointPartNum==0) return;
	double lowPointX = partBox.getLowPoint().getX();
	double lowPointY = partBox.getLowPoint().getY();

	std::string command = "./bin/rbox " + toString(pointPartNum) + " n D2 O0.5 > " + destPath + "mydata.ver";
	std::cout<<command<<"\n";
	system(command.c_str());

	readCoorPart(pointCoorArr, pointPartNum);

	//shift to current box
	for(unsigned int pointId=0; pointId<pointPartNum; pointId++){
		pointCoorArr[pointId*2] += lowPointX;
		pointCoorArr[pointId*2+1] += lowPointY;
//std::cout<<"["<<pointCoorArr[pointId*2]<<","<<pointCoorArr[pointId*2+1]<<"] "; 
	}
}

//==============================================================================
void distribute::writeListToFile(std::list<point> pointList, std::string fileStr){
	unsigned int size = pointList.size();
	point *pointArr = new point[size];
	unsigned int index = 0;
	std::list<point>::iterator it;
	for (it = pointList.begin(); it != pointList.end(); it++){
		pointArr[index] = (*it);
		index++;
	}
	storePointPartArr(pointArr, size, fileStr, "a");
	delete [] pointArr;
}

//==============================================================================
//read billion points from file, partition them into partitions, 
//for ex: a quare domain can be partitioned int to 4 column and 4 rows --> 16 partitions
//store each partition of points to file
void distribute::pointsDistribute(){
//	std::string fileStr;
	double *pointCoorArr;
	unsigned int pointCoorArrSize;
	boundingBox scaleBox;
	std::string fileStr;
	int vertexRecordSize = 2;

	//read points from file
	for(unsigned int scalePartId=0; scalePartId<xPartNum*yPartNum; scalePartId++){
		scaleBox = findPart(scalePartId, domainLowPoint, domainHighPoint, xPartNum, xPartNum);
		pointCoorArrSize = scalePartInfoArr[scalePartId];
		if(pointCoorArrSize==0) continue;
		generatePoints(scaleBox, pointCoorArr, pointCoorArrSize);
		//append to file fullPointPart.ver
		storePointCoorArr(pointCoorArr, pointCoorArrSize, destPath + "fullPointPart.ver", "a");

		//Distribute points into partitions
		for(unsigned long long index=0; index<pointCoorArrSize; index++){
			//Each point is assigned to a global index which is the index of orginal point dataset (mydatabin.ver in rawPointData)
			point currPoint(pointCoorArr[index*vertexRecordSize], pointCoorArr[index*vertexRecordSize+1], globalPointIndex);
			globalPointIndex++;
			unsigned int currPartIndex = partIndex(currPoint, domainLowPoint, domainHighPoint, xPartNum, yPartNum);
			pointPartInfoArr[currPartIndex]++;//update pointPartInfo as new point coming
			//push current point to current partition
			partList[currPartIndex].push_back(currPoint);
		}

		//Write all point coordinates in the same partition into its file
		for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++)
			if(!partList[partId].empty()){
				fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum,".ver");
				writeListToFile(partList[partId], fileStr);
				partList[partId].clear();
			}
		delete [] pointCoorArr;
	}

for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++) std::cout<<pointPartInfoArr[partId]<<" ";
std::cout<<"\n";
}


//==============================================================================
//use qsort of C++
//sort all points in all partition, then write them back to file: pointPartxxx.ver
void distribute::sortUpdate(){
	std::list<point> initPointList;
	std::string fileStr;
	//scan all partition files, sort them, then add to main data file: pointPart.ver
	for(unsigned int partId = 0; partId<xPartNum*yPartNum; partId++){
		//read point data from file to point array x1 y1 x2 y2 ...
		fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum, ".ver");
		unsigned long int pointNum = pointPartInfoArr[partId];
		if(pointNum==0)	continue;

		point *pointArr;
		readPoints(pointArr, pointNum, fileStr);

		//collect init points
		if(pointNum>=initPointNum){
			for(unsigned int i=0; i<initPointNum; i++) initPointList.push_back(pointArr[i]);
			pointNum -= initPointNum;
			pointPartInfoArr[partId] = pointNum;
		}
		else{ 
			for(unsigned int i=0; i<pointNum; i++) initPointList.push_back(pointArr[i]);
			pointNum = 0;
			pointPartInfoArr[partId] = 0;
			continue;
		}

		//sort based on coordinate x for array of points
		qsort(&pointArr[initPointNum], pointNum, sizeof(point), int_compar);
//		std::cout<<"["<<pointNum<<" points are sorted] ";
		std::cout<<"sort the content in file ("<<pointNum<<") "<<fileStr<<std::endl;
		storePointPartArr(&pointArr[initPointNum], pointNum, fileStr, "w");
		delete [] pointArr;
	}


	//store initPointList to initPoints.ver
	std::string initFileStr = destPath + "initPoints.ver";
	unsigned int totalInitPointNum = initPointList.size();
std::cout<<"totalInitPointNum: "<<totalInitPointNum<<"\n";
	writeListToFile(initPointList, initFileStr);


	//store pointPartInfo array to pointPartInfo.xfdl file
	//write to file pointPartInfo.txt
	//trunc means If the file is opened for output operations and it already existed, 
	//its previous content is deleted and replaced by the new one.
	std::ofstream pointPartInfoFile(destPath + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::trunc);
	pointPartInfoFile<<xPartNum<<" "<<yPartNum<<"\n";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++) pointPartInfoFile<<pointPartInfoArr[i]<<" ";
	pointPartInfoFile<<"\n";

	pointPartInfoFile<<initPointList.size()<<"\n";
	pointPartInfoFile.close();
	initPointList.clear();
std::cout<<"done for sortUpdate()\n";
}

//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(double stepInterval){

	//Open initPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = destPath + "initPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	//-4 means not including 4 corner points
	gridPointsSize = (unsigned)(xPartNum/stepInterval)*4-4;


	//add a middle point from an empty partition
	//the number of middle point depends on the number of empty partition (no point in the partition)
	unsigned int emptyPartNum = 0;
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		if(pointPartInfoArr[i]==0) emptyPartNum++;

	gridPointsSize += emptyPartNum;
	std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";

	//grid points on the domain edges are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum*4) including 4 corners.
	gridPoints = new point[gridPointsSize];

	//generate other grid points
	unsigned int index = 0;

	//add middle points of empty partition
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		if(pointPartInfoArr[partId]==0){
			boundingBox bb = findPart(partId, domainLowPoint, domainHighPoint, xPartNum, yPartNum);
			double x = bb.getLowPoint().getX() + (bb.getHighPoint().getX() - bb.getLowPoint().getX())/2.0;
			double y = bb.getLowPoint().getY() + (bb.getHighPoint().getY() - bb.getLowPoint().getY())/2.0;
			point middlePoint;
			middlePoint.setX(x);
			middlePoint.setY(y);
			middlePoint.setId(globalPointIndex);
			globalPointIndex++;
//std::cout<<"partId: "<<partId<<" "<<middlePoint.getX()<<" "<<middlePoint.getY()<<" "<<middlePoint.getId()<<"\n";
			//add midlle point to gridPoints
			gridPoints[index] = middlePoint;
			index++;
		}
	}

	double currentCoor = stepInterval;
	std::cout<<"stepInterval = "<<stepInterval<<"\n";
	while(currentCoor<xPartNum){
		gridPoints[index].setX(currentCoor); //x coordinate for botton line DA
		gridPoints[index].setY(0.0); //y coordinate for bottom line DA
		gridPoints[index].setId(globalPointIndex);

		globalPointIndex++;

		gridPoints[index+1].setX(xPartNum - currentCoor); //x coordinate for top line BC
		gridPoints[index+1].setY(xPartNum); //y coordinate for top line BC
		gridPoints[index+1].setId(globalPointIndex);
		globalPointIndex++;

		gridPoints[index+2].setX(0.0); //x coordinate for left line AB
		gridPoints[index+2].setY(xPartNum - currentCoor); //x coordinate for left line AB
		gridPoints[index+2].setId(globalPointIndex);
		globalPointIndex++;

		gridPoints[index+3].setX(xPartNum); //x coordinate for right line CD
		gridPoints[index+3].setY(currentCoor); //x coordinate for right line CD
		gridPoints[index+3].setId(globalPointIndex);
		globalPointIndex++;
		
		currentCoor += stepInterval;
		index +=4;
	}

	std::cout<<"globalPointIndex: "<<globalPointIndex<<"\n";

	fwrite(gridPoints, sizeof(point), gridPointsSize, f);
	fclose(f);


std::cout<<"list of grid points around the domain: \n";
for(int i=0; i<gridPointsSize; i++)
std::cout<<" ["<<gridPoints[i].getX()<<", "<<gridPoints[i].getY()<<"]";


	//write number of grid points to pointPartInfo.xfdl
	//app means append to the end of file
	std::ofstream pointPartInfoFile(destPath + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::app);
	pointPartInfoFile<<gridPointsSize<<"\n";
	pointPartInfoFile.close();
}

//==============================================================================
//copy mydatabin.ver from rawmapPointData to fullPointPart.ver in delaunatResults
//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
void distribute::createFullPointCoorData(){

	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	//coorArr contains all coordinates of grid points including 4 corner point
	//coorArr does not contain Ids
	double *coorArr = new double[(gridPointsSize+4)*2];
	//copy coordinates from gridPoints to coorArr
	//note that gridPoints contains all grid points except 4 corner points
	unsigned int index = 0;
	for(index=0; index<gridPointsSize; index++){
		coorArr[index*2] = gridPoints[index].getX();
		coorArr[index*2+1] = gridPoints[index].getY();
	}
	//Assign 4 corner coordinates of domain to coorArr
	//note that: domain is a square ABCD between (0,0) and (1,1)
	//four corners A(0,0), B(0,1), C(1,1) and D(1,0)
	coorArr[index*2] = 0;
	coorArr[index*2+1] = 0;
	coorArr[index*2+2] = 0;
	coorArr[index*2+3] = yPartNum;
	coorArr[index*2+4] = xPartNum;
	coorArr[index*2+5] = yPartNum;
	coorArr[index*2+6] = xPartNum;
	coorArr[index*2+7] = 0;

std::cout<<"gridPointsSize+4 = "<<gridPointsSize+4<<"\n";
	//add coorArr to fullPointPart.ver
	//That mean fullPointPart.ver will have all points in the domain and all grid points (include 4 corner points)
	std::string fileStr = destPath + "fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<fileStr<<"\n"; return;}
	fwrite(coorArr, sizeof(double), (gridPointsSize+4)*2, f);
	fclose(f);
	delete [] coorArr;
}

//==============================================================================
void distribute::printPointPartitions(unsigned int partitionId){

	//form a file name from partitionId (partitionId = 5 --> pointPart05.ver)
	std::string fileStr = generateFileName(partitionId, destPath + "pointPart", xPartNum*yPartNum, ".ver");
	
	unsigned int pointNum = pointPartInfoArr[partitionId];
	if(pointNum==0){
		std::cout<<fileStr<<" does not exist\n";
		return;
	}

	FILE *f = fopen(fileStr.c_str(), "rb");//appending 
	if(!f){
		std::cout<<fileStr<<" does not exist\n"; 
		return;
	}
	else{
		std::cout<<"\n\npartition = "<<partitionId<<", number of point = "<<pointNum<<":\n";
		point *data = new point[pointNum];
		fread(data, pointNum, sizeof(point), f);
		for(int i=0; i<pointNum; i++)
			std::cout<<data[i].getX()<<" "<<data[i].getY()<<" "<<data[i].getId()<<std::endl;
		delete [] data;
	}
	fclose(f);
}

//==============================================================================
//test data each pointpartXX_XX.ver inside fine partition?
void distribute::testData(){
	point *pointArr=NULL;
	unsigned int pointNum=0;
	std::string fileStr;
	FILE *f;
	double lowPartPointX;
	double lowPartPointY;
	double highPartPointX;
	double highPartPointY;
	unsigned int xFinePartNum, yFinePartNum;

	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		boundingBox currBox = findPart(partId, domainLowPoint, domainHighPoint, xPartNum, yPartNum);
		lowPartPointX = currBox.getLowPoint().getX();
		lowPartPointY = currBox.getLowPoint().getY();
		highPartPointX = currBox.getHighPoint().getX();
		highPartPointY = currBox.getHighPoint().getY();
		bool stop=false;
		if(pointPartInfoArr[partId]!=0){
			fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum, ".ver");
			f = fopen(fileStr.c_str(), "r");
			if(!f){
				std::cout<<"not success to open "<<fileStr<<std::endl;
				exit(1);
			}
			pointNum = pointPartInfoArr[partId];
			if(pointNum==0) continue;
			pointArr = new point[pointNum];
			fread(pointArr, pointNum, sizeof(point), f);
			fclose(f);

			for(unsigned int pointId=0; pointId<pointNum; pointId++){
				if((pointArr[pointId].getX()>highPartPointX)||(pointArr[pointId].getX()<lowPartPointX)){
					stop = true;
					std::cout<<"["<<pointArr[pointId].getX()<<","<<pointArr[pointId].getY()<<"] ";
				}
				if((pointArr[pointId].getY()>highPartPointY)||(pointArr[pointId].getY()<lowPartPointY)){
					stop = true;
					std::cout<<"["<<pointArr[pointId].getX()<<","<<pointArr[pointId].getY()<<"] ";
				}

			}
			delete [] pointArr;
			std::cout<<"done testing fine partition "<<fileStr<<"\n";
		}
		if(stop){
			std::cout<<"The data in partId = "<<partId<<" is wrong!!!!!!!\n";
			std::cout<<"lowPartPointX: "<<lowPartPointX<<", lowPartPointY: "<<lowPartPointY<<", highPartPointX: "<<highPartPointX<<", highPartPointY: "<<highPartPointY<<"\n";
			//printPointPart(coarsePartId,finePartId);
			continue;
		}
	}
}

//==============================================================================
void distribute::info(){
	unsigned int emptyPartNum = 0;
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		std::cout<<scalePartInfoArr[i]<<" ";
		if(scalePartInfoArr[i]==0) emptyPartNum++;
	}
	std::cout<<"\nnumber of empty scaling partitions in domain: "<<emptyPartNum<<"\n";
	std::cout<<"the percent of empty over non-empty partitions: "<<emptyPartNum*100/(xPartNum*yPartNum)<<"%\n";

	emptyPartNum = 0;
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		if(pointPartInfoArr[partId]==0) emptyPartNum++;
		std::cout<<pointPartInfoArr[partId]<<" ";
	}
	std::cout<<"\npercent empty partitions in the map domain: "<<emptyPartNum*100/(xPartNum*yPartNum)<<"%\n";
}

//==============================================================================
distribute::~distribute(){
	delete [] pointPartInfoArr;
	delete [] mapPointData;
	delete [] scalePartInfoArr;

//	if(pointGridArr!=NULL) delete [] pointGridArr;
//	delete [] gridPoints;
	delete [] partList;
}
