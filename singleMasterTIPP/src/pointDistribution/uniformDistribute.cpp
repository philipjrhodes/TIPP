//https://github.com/qhull/qhull/blob/master/html/rbox.man
#include "uniformDistribute.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
//rm -rf ../dataSources/10Kvertices/delaunayResults
//mkdir ../dataSources/10Kvertices/delaunayResults
//./distribute 4 4 10000 "../dataSources/10Kvertices/" 25

//==============================================================================
distribute::distribute(unsigned int xPartNumber, unsigned int yPartNumber, unsigned long long totalNumberPoints, unsigned int initialPointNum, std::string fullPath){
	xPartNum = xPartNumber;
	yPartNum = yPartNumber;

	domainLowPoint.setX(0);
	domainLowPoint.setY(0);
	domainHighPoint.setX(xPartNum);
	domainHighPoint.setY(yPartNum);

	initPointNum = initialPointNum;

	std::cout<<"=======================================================================\n";
	std::cout<<"xPartNumber: "<<xPartNum<<", yPartNumber: "<<yPartNum<<std::endl;
	std::cout<<"=======================================================================\n";

	globalPointIndex = 0;
	path = fullPath;

	totalInputPointNum = totalNumberPoints;
	//number of point in the first fine partition xCoarsePartNum=0, yCoarsePartNum=0, xFinePartNum=0, yFinePartNum=0
	basicPointPartNum = totalNumberPoints/(xPartNum*yPartNum);

	domainSize = xPartNumber;

	basicPointPartCoorArr = NULL;
	gridPoints = NULL;
	gridPointsSize = 0;
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
//==============================================================================
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
void distribute::writePointCoorArr(double *pointCoorArr, unsigned int pointCoorArrSize, std::string fileStr){
	if((pointCoorArr==NULL)||(pointCoorArrSize==0)) return;
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not success to open "<<fileStr<<std::endl;
		exit(1);
	}
	fwrite(pointCoorArr, pointCoorArrSize*2, sizeof(double), f);
	fclose(f);
}

//==============================================================================
void distribute::writePointPartArr(point *pointPartArr, unsigned int pointPartArrSize, std::string fileStr){
	if((pointPartArr==NULL)||(pointPartArrSize==0)) return;
	FILE *f = fopen(fileStr.c_str(), "w");
	if(!f){
		std::cout<<"not success to open "<<fileStr<<std::endl;
		exit(1);
	}
	fwrite(pointPartArr, pointPartArrSize, sizeof(point), f);
	fclose(f);
}

//==============================================================================
void distribute::writePointList(std::list<point> pointPartList, std::string fileStr){
	unsigned int pointPartSize = pointPartList.size();
	if(pointPartSize==NULL) return;

	point *pointPartArr = new point[pointPartSize];
	unsigned int index = 0;
	for(std::list<point>::iterator it=pointPartList.begin(); it!=pointPartList.end(); it++){
		pointPartArr[index] = (*it);
		index++;
	}

	FILE *f = fopen(fileStr.c_str(), "w");
	if(!f){
		std::cout<<"not success to open "<<fileStr<<std::endl;
		exit(1);
	}
	fwrite(pointPartArr, pointPartSize, sizeof(point), f);
	fclose(f);
	delete [] pointPartArr;
}

//==============================================================================
double distribute::drand(const double min, const double max){
    return (max - min) * static_cast<double>(rand()) / static_cast<double>(RAND_MAX)+min;
}


//convert vertext file (coordinates) from text to binary, also create a make up attribute file
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void distribute::readCoorPart(double *&pointCoorArr, unsigned int &pointNum){
	std::string vertexTextFileName = path + "mydata.ver";
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
	/*load content from text file to array data*/
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

std::cout<<fileIndex<<" "<<bufferIndex<<" "<<fileSize<<std::endl;
}

//==============================================================================
int int_compar(const void *p1, const void *p2){
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//==============================================================================
void distribute::generateBasicData(unsigned int &basicPointPartNum){

	//generate random points in (0,0) - (1,1)
	std::string command = "./bin/rbox " + toString(basicPointPartNum) + " n D2 O0.5 > " + path + "mydata.ver";
	std::cout<<command<<"\n";
	system(command.c_str());

	//get random points between (0,0) - (1,1) (not equal 0 or 1)
	readCoorPart(basicPointPartCoorArr, basicPointPartNum);
	//qsort(basicPointPartCoorArr, basicPointPartNum, sizeof(point), int_compar);
	if(basicPointPartNum<initPointNum){
		std::cout<<"!!!!!Number of points in a partition has to be greater than number of init point!!!!\n";
		exit(1);
	}
}

//==============================================================================
//based on the number of map points in pointPartInfoArr, generate new points with scale
void distribute::generateAllPartitionPoints(){
	double lowPartPointX, lowPartPointY;
	boundingBox currBox;
	std::string fileStr;
	//generate basic points in the range (0,0) - (1,1) to array basicPointCoorArr (partition 0, 0)
	generateBasicData(basicPointPartNum);

	double *pointPartCoorArr = new double[basicPointPartNum*2];

	unsigned int initPointDomainNum = xPartNum*yPartNum*initPointNum;
	point *initPointDomainArr = new point[initPointDomainNum];
	unsigned int initPointDomainIndex = 0;

	//pointPartNum is the number of point in fine partition after collecting all init points out 
	pointPartNum = basicPointPartNum - initPointNum;
	point *pointPartArr = new point[pointPartNum*2];

	//copy the basic points (basicPointCoorArr) to all other partitions
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		currBox = findPart(partId, domainLowPoint, domainHighPoint, xPartNum, yPartNum);
		lowPartPointX = currBox.getLowPoint().getX();
		lowPartPointY = currBox.getLowPoint().getY();

		//copy and shift point coordinates from basicPointPartCoorArr to current partition
		for(unsigned int pointId=0; pointId<basicPointPartNum; pointId++){
			pointPartCoorArr[pointId*2] = basicPointPartCoorArr[pointId*2] + lowPartPointX;
			pointPartCoorArr[pointId*2+1] = basicPointPartCoorArr[pointId*2+1] + lowPartPointY;
		}
		//append pointPartCoorArr to file fullPointPart.ver
		writePointCoorArr(pointPartCoorArr, basicPointPartNum, path + "fullPointPart.ver");


		//collects init domain points
		for(unsigned int pointId=0; pointId<initPointNum; pointId++){
			initPointDomainArr[initPointDomainIndex].setX(pointPartCoorArr[pointId*2]);
			initPointDomainArr[initPointDomainIndex].setY(pointPartCoorArr[pointId*2+1]);
			initPointDomainArr[initPointDomainIndex].setId(globalPointIndex);
			initPointDomainIndex++;
			globalPointIndex++;
		}

		//The points leftover are used for partition
		unsigned int pointPartIndex = 0;
		for(unsigned int pointId=initPointNum; pointId<basicPointPartNum; pointId++){
			pointPartArr[pointPartIndex].setX(pointPartCoorArr[pointId*2]);
			pointPartArr[pointPartIndex].setY(pointPartCoorArr[pointId*2+1]);
			pointPartArr[pointPartIndex].setId(globalPointIndex);
			pointPartIndex++;
			globalPointIndex++;
		}
		//write pointPartArr to pointPartXXX.tri
		qsort(pointPartArr, pointPartNum, sizeof(point), int_compar);
		fileStr = generateFileName(partId, path + "pointPart", xPartNum*yPartNum, ".ver");
		std::cout<<"write ("<<pointPartNum<<") points to file "<<fileStr<<"\n";
		writePointPartArr(pointPartArr, pointPartNum, fileStr);
	}

	//store initPointDomainArr to initDomainPoints.ver
	writePointPartArr(initPointDomainArr, initPointDomainNum, path + "initPoints.ver");

	//store pointPartInfo array to pointPartInfo.xfdl file
	//write to file pointPartInfo.txt
	//trunc means If the file is opened for output operations and it already existed, 
	//its previous content is deleted and replaced by the new one.
	std::ofstream pointPartInfoFile(path + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::trunc);
	pointPartInfoFile<<xPartNum<<" "<<yPartNum<<"\n";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		pointPartInfoFile<<pointPartNum<<" ";
	pointPartInfoFile<<"\n"<<initPointDomainNum;
std::cout<<"initPointDomainNum: "<<initPointDomainNum<<"\n";

	delete [] pointPartCoorArr;
	delete [] initPointDomainArr;
	delete [] pointPartArr;
}


//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(){

	//Open initPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = path + "initPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
//	unsigned int gridPointsSize = domainSize*4-4;
	unsigned int gridPointsSize = domainSize*8-4;
std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";
	//grid points on the domain edges are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum*4) including 4 corners.
	gridPoints = new point[gridPointsSize];//-4 means not including 4 corner points

	//generate other grid points
	unsigned int index = 0;
//	double currentCoor = 1.0;
	double currentCoor = 0.5;
//	for(int i=0; i<xPartNum-1; i++){//-1 means not including corner points for an edge
	while(currentCoor<domainSize){
		gridPoints[index].setX(currentCoor); //x coordinate for botton line DA
		gridPoints[index].setY(0.0); //y coordinate for bottom line DA
		gridPoints[index].setId(globalPointIndex);

//std::cout<<"globalPointIndex = "<<globalPointIndex<<"\n";
		globalPointIndex++;

		gridPoints[index+1].setX(domainSize - currentCoor); //x coordinate for top line BC
		gridPoints[index+1].setY(domainSize); //y coordinate for top line BC
		gridPoints[index+1].setId(globalPointIndex);
		globalPointIndex++;

		gridPoints[index+2].setX(0.0); //x coordinate for left line AB
		gridPoints[index+2].setY(domainSize - currentCoor); //x coordinate for left line AB
		gridPoints[index+2].setId(globalPointIndex);
		globalPointIndex++;

		gridPoints[index+3].setX(domainSize); //x coordinate for right line CD
		gridPoints[index+3].setY(currentCoor); //x coordinate for right line CD
		gridPoints[index+3].setId(globalPointIndex);
		globalPointIndex++;
		
//		currentCoor += 1.0;
		currentCoor += 0.5;
		index +=4;
	}
//	fwrite(gridPoints, sizeof(point), xPartNum*4-4, f);//-4 means not including 4 corner points
	fwrite(gridPoints, sizeof(point), xPartNum*8-4, f);//-4 means not including 4 corner points
	fclose(f);

std::cout<<"list of grid points around the domain: \n";
//for(int i=0; i<xPartNum*4-4;i++)
for(int i=0; i<xPartNum*8-4;i++)
std::cout<<" ["<<gridPoints[i].getX()<<", "<<gridPoints[i].getY()<<"]";


	//write number of grid points to pointPartInfo.xfdl
	//app means append to the end of file
	std::ofstream pointPartInfoFile(path + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::app);
	pointPartInfoFile<<"\n";
//	pointPartInfoFile<<xPartNum*4-4<<"\n";
	pointPartInfoFile<<xPartNum*8-4<<"\n";
	pointPartInfoFile.close();

}
//==============================================================================
//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
void distribute::createFullPointCoorData(){
	std::cout<<"\nCreate full point partitions and grid points ...\n";

	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	//coorArr contains all coordinates of grid points including 4 corner point
	//coorArr does not contain Ids
//	double *coorArr = new double[xPartNum*4*2];
	double *coorArr = new double[xPartNum*8*2];
	//copy coordinates from gridPoints to coorArr
	//note that gridPoints contains all grid points except 4 corner points
	unsigned int index = 0;
//	for(index=0; index<xPartNum*4-4; index++){
	for(index=0; index<xPartNum*8-4; index++){
		coorArr[index*2] = gridPoints[index].getX();
		coorArr[index*2+1] = gridPoints[index].getY();
	}
	//Assign 4 corner coordinates of domain to coorArr
	//note that: domain is a square ABCD between (0,0) and (1,1)
	//four corners A(0,0), B(0,1), C(1,1) and D(1,0)
	coorArr[index*2] = 0;
	coorArr[index*2+1] = 0;
	coorArr[index*2+2] = 0;
	coorArr[index*2+3] = domainSize;
	coorArr[index*2+4] = domainSize;
	coorArr[index*2+5] = domainSize;
	coorArr[index*2+6] = domainSize;
	coorArr[index*2+7] = 0;

	//add coorArr to fullPointPart.ver
	//That mean fullPointPart.ver will have all points in the domain and all grid points (include 4 corner points)
	std::string fileStr = path + "fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<fileStr<<"\n"; return;}
//	fwrite(coorArr, sizeof(double), xPartNum*4*2, f);
	fwrite(coorArr, sizeof(double), xPartNum*8*2, f);
	fclose(f);
	delete [] coorArr;
}


//==============================================================================
void distribute::printPointPart(unsigned int partId){

	//form a file name from coarsePartId = 5 of 4x4, finePartId = 4 of 4x4--> pointPart05_04.ver)
	std::string fileStr;
	fileStr = generateFileName(partId, path + "pointPart", xPartNum*yPartNum, ".ver");

	unsigned int pointNum = pointPartNum;
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
		std::cout<<"\n\npartition Id = "<<partId<<", number of point = "<<pointNum<<":\n";
		point *data = new point[pointNum];
		fread(data, pointNum, sizeof(point), f);
		for(int i=0; i<pointNum; i++)
			std::cout<<data[i].getX()<<" "<<data[i].getY()<<" "<<data[i].getId()<<std::endl;
		delete [] data;
	}
	fclose(f);
}
//==============================================================================
//print all partition points, not include init and grid points 
void distribute::printAllPartitions(){
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++)
		printPointPart(partId);
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
	pointArr = new point[pointPartNum];

	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		boundingBox currBox = findPart(partId, domainLowPoint, domainHighPoint, xPartNum, yPartNum);
		lowPartPointX = currBox.getLowPoint().getX();
		lowPartPointY = currBox.getLowPoint().getY();
		highPartPointX = currBox.getHighPoint().getX();
		highPartPointY = currBox.getHighPoint().getY();
		bool stop=false;
		fileStr = generateFileName(partId, path + "pointPart", xPartNum*yPartNum, ".ver");

		f = fopen(fileStr.c_str(), "r");
		if(!f){
			std::cout<<"not success to open "<<fileStr<<std::endl;
			exit(1);
		}

		fread(pointArr, pointNum, sizeof(point), f);
		fclose(f);

		for(unsigned int pointId=0; pointId<pointNum; pointId++){
			if((pointArr[pointId].getX()>highPartPointX)||(pointArr[pointId].getX()<lowPartPointX)) stop = true;
			if((pointArr[pointId].getY()>highPartPointY)||(pointArr[pointId].getY()<lowPartPointY)) stop = true;
		}

		std::cout<<"done testing fine partition "<<fileStr<<"\n";

		if(stop){
			std::cout<<"The data in partId = "<<partId<<" is wrong!!!!!!!\n";
			std::cout<<"lowPartPointX: "<<lowPartPointX<<", lowPartPointY: "<<lowPartPointX<<", highPartPointX: "<<highPartPointX<<", highPartPointY: "<<highPartPointY<<"\n";
			//printPointPart(coarsePartId,finePartId);
			continue;
		}

	}
	delete [] pointArr;
}

//==============================================================================
void distribute::info(){

	std::cout<<"number of points in a partition: "<<basicPointPartNum<<" and "<<pointPartNum<<" after colleting init points\n";
	std::cout<<"total number of points on input: "<<totalInputPointNum<<"\n";
	std::cout<<"total number of points in the fullPointPart.tri: "<<basicPointPartNum*(xPartNum*yPartNum)+gridPointsSize+4<<"\n";

}
//==============================================================================
distribute::~distribute(){
	//delete pointPartArr, pointPartInfoArr
	delete [] gridPoints;
	delete [] basicPointPartCoorArr;
}
