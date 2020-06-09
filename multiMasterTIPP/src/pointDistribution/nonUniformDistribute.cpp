#include "nonUniformDistribute.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
//rm -rf ../dataSources/mapData/delaunayResults/
//mkdir ../dataSources/mapData/delaunayResults
//./distribute 4 4 4 4 1 "../dataSources/mapData/" "nc_inundation_v6c.grd" "../dataSources/mapData/delaunayResults/" 5 20
//==============================================================================
distribute::distribute(unsigned int xCoarsePartNumber, unsigned int yCoarsePartNumber, unsigned int xFinePartNumber, unsigned int yFinePartNumber, unsigned long long totalPointNumInput, unsigned int initialCoarsePointNum, unsigned int initialFinePointNum, std::string resultPath){
	xCoarsePartNum = xCoarsePartNumber;
	yCoarsePartNum = yCoarsePartNumber;
	xFinePartNum = xFinePartNumber;
	yFinePartNum = yFinePartNumber;

	domainLowPoint.setX(0);
	domainLowPoint.setY(0);
	domainHighPoint.setX(xCoarsePartNum*xFinePartNum);
	domainHighPoint.setY(yCoarsePartNum*yFinePartNum);

	//scale = mapScale;
	totalPointNum = totalPointNumInput;
	initCoarsePointNum = initialCoarsePointNum;
	initFinePointNum = initialFinePointNum;
	initPointNum = initCoarsePointNum + initFinePointNum;

	std::cout<<"=====================================================================================\n";
	std::cout<<"xCoarsePartNumber: "<<xCoarsePartNum<<", yCoarsePartNumber: "<<yCoarsePartNum<<", xFinePartNumber: "<<xFinePartNum<<", yFinePartNumber: "<<yFinePartNum<<std::endl;
	std::cout<<"======================================================================================\n";

	globalPointIndex = 0;

//	srcPath = sourcePath;
//	dataMapFile = dataMapFileName;
	dataMapFile = "./src/northCarolinaMap/nc_inundation_v6c.grd";
	destPath = resultPath;

	mapPointData = NULL;

	//allocate memory for pointPartInfoArr
	pointPartInfoArr = new unsigned int*[xCoarsePartNum*yCoarsePartNum];
	originalPointPartInfoArr = new unsigned int*[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		pointPartInfoArr[coarsePartId] = new unsigned int[xFinePartNum*yFinePartNum];
		originalPointPartInfoArr[coarsePartId] = new unsigned int[xFinePartNum*yFinePartNum];
	}
	
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			pointPartInfoArr[coarsePartId][finePartId] = 0;
			originalPointPartInfoArr[coarsePartId][finePartId] = 0;
		}

	//allocate memory for init points for each coarse partitions
	initPartPointList = new std::list<point>[xCoarsePartNum*yCoarsePartNum];


	initPointInfoArr = new unsigned int[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		initPointInfoArr[coarsePartId] = 0;

	pointCoorArr = NULL;
	gridPoints = NULL;
	gridPointsSize = 0;
}

//==============================================================================
//read mapFile to mapData and map to the domain [0,0 - xPartNum, yPartNum] 
void distribute::readMapData(){
	std::string mapFileStr = srcPath + dataMapFile;
	std::ifstream readMapFile(mapFileStr.c_str());
	std::string strItem;
	if(!readMapFile){
		std::cout<<"There is no filename : "<<readMapFile;
		exit(1);
	}

	//skip the first and second item line because it is the file name and the number of triangulation
	readMapFile >> strItem;
	readMapFile >> strItem;

    //third item of input contains number of points
	readMapFile >> strItem;
	mapPointNum = atoll(strItem.c_str());
	scale = (double)totalPointNum/mapPointNum;
	std::cout<<"map scale: "<<scale<<"\n";

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
		mapPointData[i*2] = (mapPointData[i*2]-minX)*xCoarsePartNum*xFinePartNum/deltaX;
		mapPointData[i*2+1] = (mapPointData[i*2+1]-minY)*yCoarsePartNum*yFinePartNum/deltaY;
//std::cout<<mapPointData[i*2]<<" "<<mapPointData[i*2+1]<<"\n";
	}
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
//count number of map points in each partition (pointPartInfoArr) in grid xPartNum x yPartNum 
//based on scale and number of points in the map then update point numbers in (pointPartInfoArr)
//==============================================================================
void distribute::calculatePointPartNum(){
	unsigned int coarsePartIndex;
	unsigned int finePartIndex;
	boundingBox bb;
std::cout<<"mapPointNum: "<<mapPointNum<<"\n";

	//determine number of map points in each fine partition
	for(unsigned int i=0; i<mapPointNum; i++){
		point p(mapPointData[i*2], mapPointData[i*2+1]);
//std::cout<<mapPointData[i*2]<<" "<<mapPointData[i*2+1]<<"  ";

		coarsePartIndex = partIndex(p, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		bb = findPart(coarsePartIndex, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		finePartIndex = partIndex(p, bb.getLowPoint(), bb.getHighPoint(), xFinePartNum, yFinePartNum);
//std::cout<<coarsePartIndex<<" "<<finePartIndex<<"\n";
		pointPartInfoArr[coarsePartIndex][finePartIndex]++;
	}



	unsigned int totalPointNum = 0;
	//scale up map points
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			pointPartInfoArr[coarsePartId][finePartId] *= scale;
//std::cout<<pointPartInfoArr[coarsePartId][finePartId]<<" ";
			originalPointPartInfoArr[coarsePartId][finePartId] = pointPartInfoArr[coarsePartId][finePartId];
			totalPointNum += pointPartInfoArr[coarsePartId][finePartId];
		}
	std::cout<<"total point number (not including grid points): "<<totalPointNum<<"\n";


/*	unsigned int totalPoints = 0;
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		pointPartInfoArr[i] = pointPartInfoArr[i]*scale;
		totalPoints += pointPartInfoArr[i];
	}
	std::cout<<"total points: "<<totalPoints;

	unsigned int pointPartNum, totalInitPointNum = 0;
	//calculate number of init points
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		pointPartNum = pointPartInfoArr[partId];
		if(pointPartNum>=initPointNum)
			totalInitPointNum += initPointNum;
		else if(pointPartNum>0) totalInitPointNum += pointPartNum;
	}
	initPointArr = new point[totalInitPointNum];
	initPointArrSize = totalInitPointNum;
*/
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
void distribute::generatePoints(boundingBox partBox, double *&pointCoorArr, unsigned int &pointPartNum){
	double lowPointX = partBox.getLowPoint().getX();
	double lowPointY = partBox.getLowPoint().getY();

	//generate random points in (0,0) - (1,1)
	std::string command = "./bin/rbox " + toString(pointPartNum) + " n D2 O0.5 > " + destPath + "mydata.ver";
	system(command.c_str());

	readCoorPart(pointCoorArr, pointPartNum);

	//shift to current box
	for(unsigned int pointId=0; pointId<pointPartNum; pointId++){
		pointCoorArr[pointId*2] += lowPointX;
		pointCoorArr[pointId*2+1] += lowPointY;
	}
}

//==============================================================================
int int_compar(const void *p1, const void *p2)
{
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//==============================================================================
//based on the number of map points in pointPartInfoArr, generate new points with scale
void distribute::generateScalePoints(){
	unsigned int pointFinePartNum;

	double lowPartPointX, lowPartPointY, highPartPointX, highPartPointY;
	boundingBox currCoarseBox, currFineBox;
	double x,y;
	unsigned int totalPointNum = 0;

	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			currFineBox = findPart(finePartId, currCoarseBox.getLowPoint(), currCoarseBox.getHighPoint(), xFinePartNum, yFinePartNum);
			lowPartPointX = currFineBox.getLowPoint().getX();
			lowPartPointY = currFineBox.getLowPoint().getY();
			highPartPointX = currFineBox.getHighPoint().getX();
			highPartPointY = currFineBox.getHighPoint().getY();
//std::cout<<lowPartPointX<<" "<<lowPartPointY<<" "<<highPartPointX<<" "<<highPartPointY<<"\n";
			double *pointCoorArr=NULL;
			//number of point of current partition (partId) after scaling
			pointFinePartNum = pointPartInfoArr[coarsePartId][finePartId];
			if(pointFinePartNum>0){
				generatePoints(currFineBox, pointCoorArr, pointFinePartNum);
/*				pointCoorArr = new double[pointFinePartNum*2];
				//generate random points
				for(unsigned int pointId=0; pointId<pointFinePartNum; pointId++){
					x = drand(lowPartPointX, highPartPointX);
					y = drand(lowPartPointY, highPartPointY);
					pointCoorArr[pointId*2] = x;
					pointCoorArr[pointId*2+1] = y;
//std::cout<<"["<<pointCoorArr[pointId*2]<<","<<pointCoorArr[pointId*2+1]<<"]  ";
				}
*/

				//append pointCoorArr to file
				writePointCoorArr(pointCoorArr, pointFinePartNum, destPath + "fullPointPart.ver");
				totalPointNum += pointFinePartNum;

				if(pointFinePartNum>=initPointNum){// there have points for initial points and partition points
					//collect initCoarsePointNum for domain from pointCoorArr	
					for(unsigned int pointId=0; pointId<initCoarsePointNum; pointId++){
						point currPoint(pointCoorArr[pointId*2], pointCoorArr[pointId*2+1], globalPointIndex);
						globalPointIndex++;
						initDomainPointList.push_back(currPoint);
					}
					//collect initFinePointNum for coarsePartitionXX from pointCoorArr	
					for(unsigned int pointId=initCoarsePointNum; pointId<initPointNum; pointId++){
						point currPoint(pointCoorArr[pointId*2], pointCoorArr[pointId*2+1], globalPointIndex);
						globalPointIndex++;
						initPartPointList[coarsePartId].push_back(currPoint);
					}

					//write pointPartArr after collecting init points
					unsigned int pointPartArrSize = pointFinePartNum - initPointNum;
					if(pointPartArrSize>0){
						point *pointPartArr = new point[pointPartArrSize];
						unsigned int index=0;
						for(unsigned int pointId=initPointNum; pointId<pointFinePartNum; pointId++){
							point currPoint(pointCoorArr[pointId*2], pointCoorArr[pointId*2+1], globalPointIndex);
							globalPointIndex++;
							pointPartArr[index] = currPoint;
							index++;
						}
						qsort(pointPartArr, pointPartArrSize, sizeof(point), int_compar);
						std::string fileStr = generateFileName(coarsePartId, destPath + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
						fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");
						std::cout<<"write ("<<pointPartArrSize<<") points to file "<<fileStr<<"\n";
						writePointPartArr(pointPartArr, pointPartArrSize, fileStr);
						delete [] pointPartArr;
					}
					//update pointPartInfoArr
					pointPartInfoArr[coarsePartId][finePartId] = pointPartArrSize;

				}
				else{ 
					if(pointFinePartNum>initCoarsePointNum){//collect init points for domain and coase partition only
						//collect initCoarsePointNum for domain from pointCoorArr	
						for(unsigned int pointId=0; pointId<initCoarsePointNum; pointId++){
							point currPoint(pointCoorArr[pointId*2], pointCoorArr[pointId*2+1], globalPointIndex);
							globalPointIndex++;
							initDomainPointList.push_back(currPoint);
						}
						//collect initFinePointNum for coarsePartitionXX from pointCoorArr
						for(unsigned int pointId=initCoarsePointNum; pointId<pointFinePartNum; pointId++){
							point currPoint(pointCoorArr[pointId*2], pointCoorArr[pointId*2+1], globalPointIndex);
							globalPointIndex++;
							initPartPointList[coarsePartId].push_back(currPoint);
						}
						pointPartInfoArr[coarsePartId][finePartId] = 0;					
					}
					else{ //pointFinePartNum<=initCoarsePointNum
						//collect initCoarsePointNum for domain from pointCoorArr	
						for(unsigned int pointId=0; pointId<pointFinePartNum; pointId++){
							point currPoint(pointCoorArr[pointId*2], pointCoorArr[pointId*2+1], globalPointIndex);
							globalPointIndex++;
							initDomainPointList.push_back(currPoint);
						}
						pointPartInfoArr[coarsePartId][finePartId] = 0;
					}
				}

				delete [] pointCoorArr;
			}
			//store initPartPointList to file initPointPartXX.ver 
			unsigned int initPartPointSize = initPartPointList[coarsePartId].size();
			initPointInfoArr[coarsePartId] = initPartPointSize;
			std::string fileStr = generateFileName(coarsePartId, destPath + "initPointPart", xCoarsePartNum*yCoarsePartNum, ".ver");
			if(initPartPointSize>0) writePointList(initPartPointList[coarsePartId], fileStr);
			initPartPointList[coarsePartId].clear();
		}
	}

std::cout<<"total point number (not including grid points): "<<totalPointNum<<"\n";

	//store initDomainPointList to initDomainPoints.ver
	writePointList(initDomainPointList, destPath + "initDomainPoints.ver");
	unsigned int initDomainPointSize = initDomainPointList.size();
	initDomainPointList.clear();

/*
std::cout<<"#############Number of init domain point: "<<initDomainPointSize<<"\n";
//for(std::list<point>::const_iterator it=initDomainPointList.begin(); it!=initDomainPointList.end(); it++) std::cout<<(*it);
std::string str = destPath + "initDomainPoints.ver";
FILE *f = fopen(str.c_str(), "r");
point *pointArr = new point[initDomainPointSize];
fread(pointArr, initDomainPointSize, sizeof(point), f);
fclose(f);
for(int i=0; i<initDomainPointSize; i++) std::cout<<pointArr[i];
delete [] pointArr;
*/

	std::ofstream pointPartInfoFile(destPath + "pointDomainInfo.xfdl", std::ofstream::out);
	//first line stores number of coarse partition sizes (ex: 4 x 4)
	pointPartInfoFile<<xCoarsePartNum<<" "<<yCoarsePartNum<<"\n";
	//second line stores number of init points for the domain
	pointPartInfoFile<<initDomainPointSize<<"\n";
	pointPartInfoFile.close();


	//store pointPartInfo array to pointPartInfoXX.xfdl file
	//trunc means If the file is opened for output operations and it already existed, 
	//its previous content is deleted and replaced by the new one.	
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		std::string fileStr = generateFileName(coarsePartId, destPath + "pointPartInfo", xCoarsePartNum*yCoarsePartNum, ".xfdl");
		std::ofstream pointPartInfoFile(fileStr, std::ofstream::out);
		//first line store partition numbers
		pointPartInfoFile<<xFinePartNum<<" "<<yFinePartNum<<"\n";
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		//second line stores the coordiantes of lowPoint and highPoint of a coarse partition
		pointPartInfoFile<<currCoarseBox.getLowPoint().getX()<<" "<<currCoarseBox.getLowPoint().getY()<<" "<<currCoarseBox.getHighPoint().getX()<<" "<<currCoarseBox.getHighPoint().getY()<<"\n";
		//third line stores total number init points in a coarse partition
		pointPartInfoFile<<initPointInfoArr[coarsePartId]<<"\n";
		//fourth line stores number of points of fine partitions ia a coarse partition
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
			pointPartInfoFile<<pointPartInfoArr[coarsePartId][finePartId]<<" ";
		pointPartInfoFile<<"\n";
		pointPartInfoFile.close();
	}
}

//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(){

	//Open initPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = destPath + "initDomainPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	//-4 means not including 4 corner points
	gridPointsSize = (xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4;
std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";

	//add a middle point from an empty partition
	//the number of middle point depends on the number of empty partition (no point in the partition)
	unsigned int emptyPartNum = 0;
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
			if(originalPointPartInfoArr[coarsePartId][finePartId]==0) emptyPartNum++;

	gridPointsSize += emptyPartNum;
	std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";

	//grid points on the domain edges are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum*4) including 4 corners.
	gridPoints = new point[gridPointsSize];

	//generate other grid points
	unsigned int index = 0;
	boundingBox currCoarseBox, currFineBox;
	//add middle points of empty partition
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			currFineBox = findPart(finePartId, currCoarseBox.getLowPoint(), currCoarseBox.getHighPoint(), xFinePartNum, yFinePartNum);
			if(originalPointPartInfoArr[coarsePartId][finePartId]==0){
				double x = currFineBox.getLowPoint().getX() + (currFineBox.getHighPoint().getX() - currFineBox.getLowPoint().getX())/2.0;
				double y = currFineBox.getLowPoint().getY() + (currFineBox.getHighPoint().getY() - currFineBox.getLowPoint().getY())/2.0;
				point middlePoint;
				middlePoint.setX(x);
				middlePoint.setY(y);
				middlePoint.setId(globalPointIndex);
				globalPointIndex++;
//std::cout<<"order: "<<index<<", coarsePartId: "<<coarsePartId<<", finePartId: "<<finePartId<<", "<<middlePoint.getX()<<" "<<middlePoint.getY()<<" "<<middlePoint.getId()<<"\n";
				//add midlle point to gridPoints
				gridPoints[index] = middlePoint;
				index++;
			}
		}
	}


	double currentCoor = 1.0;
	while(currentCoor<xCoarsePartNum*xFinePartNum){
		gridPoints[index].setX(currentCoor); //x coordinate for botton line DA
		gridPoints[index].setY(0.0); //y coordinate for bottom line DA
		gridPoints[index].setId(globalPointIndex);

//std::cout<<"globalPointIndex = "<<globalPointIndex<<"\n";
		globalPointIndex++;

		gridPoints[index+1].setX(xCoarsePartNum*xFinePartNum - currentCoor); //x coordinate for top line BC
		gridPoints[index+1].setY(xCoarsePartNum*xFinePartNum); //y coordinate for top line BC
		gridPoints[index+1].setId(globalPointIndex);
		globalPointIndex++;

		gridPoints[index+2].setX(0.0); //x coordinate for left line AB
		gridPoints[index+2].setY(xCoarsePartNum*xFinePartNum - currentCoor); //x coordinate for left line AB
		gridPoints[index+2].setId(globalPointIndex);
		globalPointIndex++;

		gridPoints[index+3].setX(xCoarsePartNum*xFinePartNum); //x coordinate for right line CD
		gridPoints[index+3].setY(currentCoor); //x coordinate for right line CD
		gridPoints[index+3].setId(globalPointIndex);
		globalPointIndex++;
		
		currentCoor += 1;
		index +=4;
	}

	std::cout<<"globalPointIndex: "<<globalPointIndex<<"\n";

	fwrite(gridPoints, sizeof(point), gridPointsSize, f);//-4 means not including 4 corner points
	fclose(f);

/*std::cout<<"list of grid points around the domain: \n";
for(int i=0; i<gridPointsSize; i++)
std::cout<<", ["<<gridPoints[i].getX()<<", "<<gridPoints[i].getY()<<", "<<gridPoints[i].getId()<<"]";
*/


	//write number of grid points (including middle points) to pointPartInfo.xfdl
	//app means append to the end of file
	std::ofstream pointPartInfoFile(destPath + "pointDomainInfo.xfdl", std::ofstream::out | std::ofstream::app);
	//third line stores number of grid points (including extra point in empty fine partitions)
	pointPartInfoFile<<gridPointsSize<<"\n";
	//fourth line stores total number of point in domain (except 4 corner points)
	pointPartInfoFile<<globalPointIndex<<"\n";
	//fifth line store number of points in coarse partitions
	unsigned int pointNum=0;
	unsigned int *pointNumCoarsePartArr = new unsigned int[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			pointNum += pointPartInfoArr[coarsePartId][finePartId];
		}
		pointNumCoarsePartArr[coarsePartId] = pointNum;
		pointNum = 0;
	}
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		pointPartInfoFile<<pointNumCoarsePartArr[coarsePartId]<<" ";

	delete [] pointNumCoarsePartArr;
	pointPartInfoFile.close();
}

//==============================================================================
//copy mydatabin.ver from rawmapPointData to fullPointPart.ver in delaunatResults
//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
void distribute::createFullPointCoorData(){

	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	//coorArr contains all coordinates of grid points including 4 corner point
	//coorArr does not contain Ids
	unsigned int totalPointNum = gridPointsSize+4; //inclunding 4 corner points of the domain

	double *coorArr = new double[totalPointNum*2];

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
	coorArr[gridPointsSize*2] = 0;
	coorArr[gridPointsSize*2+1] = 0;
	coorArr[gridPointsSize*2+2] = 0;
	coorArr[gridPointsSize*2+3] = yCoarsePartNum*yFinePartNum;
	coorArr[gridPointsSize*2+4] = xCoarsePartNum*xFinePartNum;
	coorArr[gridPointsSize*2+5] = yCoarsePartNum*yFinePartNum;
	coorArr[gridPointsSize*2+6] = xCoarsePartNum*xFinePartNum;
	coorArr[gridPointsSize*2+7] = 0;

std::cout<<"total point in the domain: "<<totalPointNum<<"\n";
for(unsigned int i=0; i<totalPointNum; i++)
	std::cout<<"["<<coorArr[i*2]<<","<<coorArr[i*2+1]<<"]  ";	


	//add coorArr to fullPointPart.ver
	//That mean fullPointPart.ver will have all points in the domain and all grid points (include 4 corner points)
	std::string fileStr = destPath + "fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<fileStr<<"\n"; return;}
	fwrite(coorArr, sizeof(double), totalPointNum*2, f);
	fclose(f);
	delete [] coorArr;
}

//==============================================================================
void distribute::printPointPart(unsigned int coarsePartId, unsigned int finePartId){

	//form a file name from coarsePartId = 5 of 4x4, finePartId = 4 of 4x4--> pointPart05_04.ver)
	std::string fileStr;
	fileStr = generateFileName(coarsePartId, destPath + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
	fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");

	unsigned int pointNum = pointPartInfoArr[coarsePartId][finePartId];
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
		std::cout<<"\n\nCoarse Partition Id = "<<coarsePartId<<", Fine Partition Id = "<<finePartId<<", number of point = "<<pointNum<<":\n";
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
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			printPointPart(coarsePartId, finePartId);
		}
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

	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		boundingBox currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			boundingBox currFineBox = findPart(finePartId, currCoarseBox.getLowPoint(), currCoarseBox.getHighPoint(), xFinePartNum, yFinePartNum);
			lowPartPointX = currFineBox.getLowPoint().getX();
			lowPartPointY = currFineBox.getLowPoint().getY();
			highPartPointX = currFineBox.getHighPoint().getX();
			highPartPointY = currFineBox.getHighPoint().getY();
			bool stop=false;
			if(pointPartInfoArr[coarsePartId][finePartId]!=0){
				fileStr = generateFileName(coarsePartId, destPath + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
				fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");
				f = fopen(fileStr.c_str(), "r");
				if(!f){
					std::cout<<"not success to open "<<fileStr<<std::endl;
					exit(1);
				}
				pointNum = pointPartInfoArr[coarsePartId][finePartId];
				pointArr = new point[pointNum];
				fread(pointArr, pointNum, sizeof(point), f);
				fclose(f);

				for(unsigned int pointId=0; pointId<pointNum; pointId++){
					if((pointArr[pointId].getX()>highPartPointX)||(pointArr[pointId].getX()<lowPartPointX)) stop = true;
					if((pointArr[pointId].getY()>highPartPointY)||(pointArr[pointId].getY()<lowPartPointY)) stop = true;
				}
				delete [] pointArr;
				std::cout<<"done testing fine partition "<<fileStr<<"\n";
			}
			if(stop){
				std::cout<<"The data in coarsePartId = "<<coarsePartId<<", finePartId = "<<finePartId<<" is wrong!!!!!!!\n";
				std::cout<<"lowPartPointX: "<<lowPartPointX<<", lowPartPointY: "<<lowPartPointX<<", highPartPointX: "<<highPartPointX<<", highPartPointY: "<<highPartPointY<<"\n";
				//printPointPart(coarsePartId,finePartId);
				continue;
			}
		}
	}
}

//==============================================================================
void distribute::readFullPointPartData(){
	std::string fileStr = destPath + "fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "rb");
	unsigned int pointNum = 31620;
	double *pointCoorArr = new double[pointNum*2];
	if(!f)	{std::cout<<" can not open "<<fileStr<<"\n"; return;}
	fread(pointCoorArr, sizeof(double), pointNum*2, f);
	fclose(f);
//	for(unsigned int i=0; i<pointNum; i++){
//		std::cout<<"["<<pointCoorArr[i*2]<<","<<pointCoorArr[i*2+1]<<"]  ";	
//	}
	delete [] pointCoorArr;
}
//==============================================================================
distribute::~distribute(){
	//delete pointPartArr, pointPartInfoArr
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		delete [] pointPartInfoArr[coarsePartId];
	delete [] pointPartInfoArr;
	delete [] originalPointPartInfoArr;

	delete [] mapPointData;
	delete [] initPartPointList;

	delete [] initPointInfoArr;
	delete [] gridPoints;
}
