#include "nonUniformAdaptiveDistribute.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
//rm -rf ../dataSources/mapData/delaunayResults/
//mkdir ../dataSources/mapData/delaunayResults
//./distribute 4 4 4 4 1 "../dataSources/mapData/" "nc_inundation_v6c.grd" "../dataSources/mapData/delaunayResults/" 5 20
//==============================================================================
distribute::distribute(unsigned int xCoarsePartNumber, unsigned int yCoarsePartNumber, unsigned int pointNumberThreshold, unsigned long long totalNumberPoints, unsigned int initialCoarsePointNum, unsigned int initialFinePointNum, std::string resultPath){
	xCoarsePartNum = xCoarsePartNumber;
	yCoarsePartNum = yCoarsePartNumber;

	pointNumThreshold = pointNumberThreshold;
	totalInputPointNum = totalNumberPoints;

	domainLowPoint.setX(0);
	domainLowPoint.setY(0);
	domainHighPoint.setX(xCoarsePartNum);
	domainHighPoint.setY(yCoarsePartNum);


	//scale = mapScale;
	initCoarsePointNum = initialCoarsePointNum;
	initFinePointNum = initialFinePointNum;
	initPointNum = initCoarsePointNum + initFinePointNum;

	std::cout<<"=====================================================================================\n";
	std::cout<<"xCoarsePartNumber: "<<xCoarsePartNum<<", yCoarsePartNumber: "<<yCoarsePartNum<<std::endl;
	std::cout<<"pointNumberThreshold: "<<pointNumberThreshold<<std::endl;

	globalPointIndex = 0;

	//dataMapFile = dataMapFileName;
	dataMapFile = "./src/northCarolinaMap/nc_inundation_v6c.grd";
	destPath = resultPath;

	mapPointData = NULL;

	//allocate memory for pointPartInfoArr
	pointPartInfoArr = new unsigned int*[xCoarsePartNum*yCoarsePartNum];

	//number of points in each coarse partition in domain
	pointCoarsePartInfoArr = new unsigned int[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		pointCoarsePartInfoArr[coarsePartId] = 0;

	rowFinePartNumArr = new unsigned int[xCoarsePartNum*yCoarsePartNum];

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
	std::string mapFileStr = dataMapFile;
	std::ifstream readMapFile(mapFileStr.c_str());
	std::string strItem;
	if(!readMapFile){
		std::cout<<"There is no filename : " << mapFileStr;
		exit(1);
	}

	//skip the first and second item line because it is the file name and the number of triangulation
	readMapFile >> strItem;
	readMapFile >> strItem;

    //third item of input contains number of points
	readMapFile >> strItem;
	mapPointNum = atoll(strItem.c_str());

	scale = (double)totalInputPointNum/mapPointNum;
	std::cout<<"map scale: "<<scale<<"\n";
	std::cout<<"======================================================================================\n";

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
//		mapPointData[i*2] = (mapPointData[i*2]-minX)*xCoarsePartNum*xFinePartNum/deltaX;
//		mapPointData[i*2+1] = (mapPointData[i*2+1]-minY)*yCoarsePartNum*yFinePartNum/deltaY;
		mapPointData[i*2] = (mapPointData[i*2]-minX)*xCoarsePartNum/deltaX;
		mapPointData[i*2+1] = (mapPointData[i*2+1]-minY)*yCoarsePartNum/deltaY;

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
//identify a pointNum if it is closest to a n^2
unsigned int distribute::closeToQuare(unsigned int pointNum, unsigned int pointNumThreshold){
	unsigned int result;
	if(pointNum<pointNumThreshold) return 1;
	else{
		unsigned int num = pointNum/pointNumThreshold;
		unsigned int num_sqrt = sqrt(num);
		unsigned int num_prev = num_sqrt;
		unsigned int num_next = num_sqrt + 1;
	    unsigned int sqr_prev = num_prev * num_prev;
		unsigned int sqr_next = num_next * num_next;
	    if(num - sqr_prev < sqr_next - num){
			//return sqrt(sqr_prev);
			result = sqrt(sqr_prev);
			//if(result%2!=0) result++;
    	    //return result;
		}
    	else{
    	    //return sqrt(sqr_next);
			result = sqrt(sqr_next);
			//if(result%2!=0) result--;
    	    //return result;
		}
	}
	if((result >= 3)&&(result <= 6)) result = 4;
	if((result >= 7)&&(result <= 12)) result = 8;
	if((result > 12)&&(result <= 22)) result = 16;
	if(result > 22) result = 32;

	return result;
}

//==============================================================================
//count number of map points in each partition (pointPartInfoArr) in grid xCoarsePartNum x yCoarsePartNum 
//based on scale and number of points in the map then update point numbers in (pointPartInfoArr)
//==============================================================================
void distribute::calculatePointCoarsePartNum(){
	unsigned int coarsePartIndex;
	unsigned int finePartIndex;
	boundingBox bb;
std::cout<<"mapPointNum: "<<mapPointNum<<"\n";

	//determine number of map points in each fine partition
	for(unsigned int i=0; i<mapPointNum; i++){
		point p(mapPointData[i*2], mapPointData[i*2+1]);
//std::cout<<mapPointData[i*2]<<" "<<mapPointData[i*2+1]<<"  ";
		coarsePartIndex = partIndex(p, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		pointCoarsePartInfoArr[coarsePartIndex]++;	
	}

	unsigned int totalPointNum = 0;
	//scale up map points
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		pointCoarsePartInfoArr[coarsePartId] *= scale;
		totalPointNum += pointCoarsePartInfoArr[coarsePartId];
		rowFinePartNumArr[coarsePartId] = closeToQuare(pointCoarsePartInfoArr[coarsePartId], pointNumThreshold);
//std::cout<<pointCoarsePartInfoArr[coarsePartId]<<" "<<rowFinePartNumArr[coarsePartId]<<"\n";
	}
	std::cout<<"total point number (not including grid points): "<<totalPointNum<<"\n";

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
void distribute::appendPointList(std::list<point> pointPartList, std::string fileStr){
	unsigned int pointPartSize = pointPartList.size();
	if(pointPartSize==NULL) return;

	point *pointPartArr = new point[pointPartSize];
	unsigned int index = 0;
	for(std::list<point>::iterator it=pointPartList.begin(); it!=pointPartList.end(); it++){
		pointPartArr[index] = (*it);
		index++;
	}

	FILE *f = fopen(fileStr.c_str(), "a");
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

std::cout<<fileIndex<<" "<<bufferIndex<<" "<<fileSize<<std::endl;
}


//==============================================================================
void distribute::generatePoints(boundingBox partBox, double *&pointCoorArr, unsigned int &pointPartNum){
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
int int_compar(const void *p1, const void *p2)
{
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//==============================================================================
//generate points for a coarse partition, then distribute them into fine partitions
void distribute::distributeCoarsePartition(unsigned int coarsePartId){
	unsigned int xFinePartNum, yFinePartNum;
	unsigned int coarsePartPointNum;

	coarsePartPointNum = pointCoarsePartInfoArr[coarsePartId];
	xFinePartNum = yFinePartNum = rowFinePartNumArr[coarsePartId];
	pointPartInfoArr[coarsePartId] = new unsigned int[xFinePartNum*yFinePartNum];
	if(coarsePartPointNum==0){
		pointPartInfoArr[coarsePartId][0] = 0;
		return;
	}

	boundingBox currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
	double *pointCoorArr;
	//generate points in the partition currCoarseBox
	generatePoints(currCoarseBox, pointCoorArr, coarsePartPointNum);

	//append to file fullPointPart.ver
	writePointCoorArr(pointCoorArr, coarsePartPointNum, destPath + "fullPointPart.ver");
	
	//listPartPointArr contains an array of list of points
	std::list<point> *listPartPointArr = new std::list<point>[xFinePartNum*yFinePartNum];
	point currentPoint, lowCoarsePoint, highCoarsePoint;
	lowCoarsePoint = currCoarseBox.getLowPoint();
	highCoarsePoint = currCoarseBox.getHighPoint();
	unsigned int partFineIndex;
	//distribute point in the partition currCoarseBox into xFinePartNum x yFinePartNum
	for(unsigned int pointId=0; pointId<coarsePartPointNum; pointId++){
		currentPoint.setX(pointCoorArr[pointId*2]);
		currentPoint.setY(pointCoorArr[pointId*2+1]);
		currentPoint.setId(globalPointIndex);
		globalPointIndex++;
		partFineIndex = partIndex(currentPoint, lowCoarsePoint, highCoarsePoint, xFinePartNum, yFinePartNum);
		listPartPointArr[partFineIndex].push_front(currentPoint);
	}
	delete [] pointCoorArr;

	std::string fileStr;
	std::list<point> initPartPointList;
	//process fine partition
	for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){

		unsigned int pointNum = listPartPointArr[finePartId].size();

		if(pointNum>=initPointNum){

			//collect init points for domain
			for(unsigned int i=0; i<initCoarsePointNum; i++){
				point initPoint = listPartPointArr[finePartId].front();
				initDomainPointList.push_back(initPoint);
				listPartPointArr[finePartId].pop_front();
			}
	
			//collect init points for coarse partitions
			for(unsigned int i=0; i<initFinePointNum; i++){
				point initPoint = listPartPointArr[finePartId].front();
				initPartPointList.push_back(initPoint);
				listPartPointArr[finePartId].pop_front();
			}

			//write points that are leftover (listPartPointArr[finePartId]) to pointPartXX_XX.ver
			unsigned int pointFinePartArrSize = listPartPointArr[finePartId].size();
			point *pointFinePartArr = new point[pointFinePartArrSize];
			unsigned int index = 0;
			for(std::list<point>::iterator it=listPartPointArr[finePartId].begin(); it!=listPartPointArr[finePartId].end(); it++){
				pointFinePartArr[index] = (*it);
				index++;
			}
			listPartPointArr[finePartId].clear();

			qsort(pointFinePartArr, pointFinePartArrSize, sizeof(point), int_compar);
			fileStr = generateFileName(coarsePartId, destPath + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
			fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");
			std::cout<<"write ("<<pointFinePartArrSize<<") points to file "<<fileStr<<"\n";
			writePointPartArr(pointFinePartArr, pointFinePartArrSize, fileStr);
			delete [] pointFinePartArr;
			//update pointPartInfoArr
			pointPartInfoArr[coarsePartId][finePartId] = pointFinePartArrSize;
		}
		else{
			if(pointNum>initCoarsePointNum){
				//collect init points for domain
				for(unsigned int i=0; i<initCoarsePointNum; i++){
					point initPoint = listPartPointArr[finePartId].front();
					initDomainPointList.push_back(initPoint);
					listPartPointArr[finePartId].pop_front();
				}
				//collect init points for coarse partitions
				for(unsigned int i=0; i<(pointNum-initCoarsePointNum); i++){
					point initPoint = listPartPointArr[finePartId].front();
					initPartPointList.push_back(initPoint);
					listPartPointArr[finePartId].pop_front();
				}
				pointPartInfoArr[coarsePartId][finePartId] = 0;
			}
			else{//pointNum<=initCoarsePointNum
				//collect init points for domain only
				for(unsigned int i=0; i<pointNum; i++){
					point initPoint = listPartPointArr[finePartId].front();
					initDomainPointList.push_back(initPoint);
					listPartPointArr[finePartId].pop_front();
				}
				pointPartInfoArr[coarsePartId][finePartId] = 0;
			}
		}
	}

	//store initPartPointList to file initPointPartXX.ver 
	unsigned int iniFinetPartPointSize = initPartPointList.size();
	if(iniFinetPartPointSize>0){
		initPointInfoArr[coarsePartId] = iniFinetPartPointSize;
		fileStr = generateFileName(coarsePartId, destPath + "initPointPart", xCoarsePartNum*yCoarsePartNum, ".ver");
		if(iniFinetPartPointSize>0) writePointList(initPartPointList, fileStr);
		initPartPointList.clear();
	}


	//store pointPartInfo array to pointPartInfoXX.xfdl file
	fileStr = generateFileName(coarsePartId, destPath + "pointPartInfo", xCoarsePartNum*yCoarsePartNum, ".xfdl");
	std::ofstream pointPartInfoFile(fileStr, std::ofstream::out);
	//first line store partition numbers
	pointPartInfoFile<<xFinePartNum<<" "<<yFinePartNum<<"\n";
	//second line stores the coordiantes of lowPoint and highPoint of a coarse partition
	pointPartInfoFile<<currCoarseBox.getLowPoint().getX()<<" "<<currCoarseBox.getLowPoint().getY()<<" "<<currCoarseBox.getHighPoint().getX()<<" "<<currCoarseBox.getHighPoint().getY()<<"\n";
	//third line stores total number init points in a coarse partition
	pointPartInfoFile<<initPointInfoArr[coarsePartId]<<"\n";
	//fourth line stores number of points of fine partitions ia a coarse partition
	for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
		pointPartInfoFile<<pointPartInfoArr[coarsePartId][finePartId]<<" ";
	pointPartInfoFile<<"\n";
	pointPartInfoFile.close();

	delete [] listPartPointArr;
}

//==============================================================================
//generate init points, points for all coarse, fine partition in domains
void distribute::generatePointsAllCoarsePartitions(){
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		distributeCoarsePartition(coarsePartId);
	}

	//store initDomainPointList to initDomainPoints.ver
	writePointList(initDomainPointList, destPath + "initDomainPoints.ver");

	unsigned int initDomainPointSize = initDomainPointList.size();
	initDomainPointList.clear();

	std::ofstream pointPartInfoFile(destPath + "pointDomainInfo.xfdl", std::ofstream::out);
	//first line stores number of coarse partition sizes (ex: 4 x 4)
	pointPartInfoFile<<xCoarsePartNum<<" "<<yCoarsePartNum<<"\n";
	//second line stores number of init points for the domain
	pointPartInfoFile<<initDomainPointSize<<"\n";
	pointPartInfoFile.close();
}



//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(){

/*	unsigned int xMaxFinePartNum, yMaxFinePartNum, maxFinePartNum = 0;
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*xCoarsePartNum; coarsePartId++)
		if(maxFinePartNum < rowFinePartNumArr[coarsePartId]) maxFinePartNum = rowFinePartNumArr[coarsePartId];
	std::cout<<"maxFinePartNum: "<<maxFinePartNum<<"\n";

	xMaxFinePartNum = yMaxFinePartNum = maxFinePartNum;
*/


	//Open initPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = destPath + "initDomainPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	unsigned int xFinePartNum, yFinePartNum;
	std::list<point> gridPointList;

	//add middle points in all fine partitions
/*	//generate other grid points (middle points)
	boundingBox currCoarseBox, currFineBox;
	//add middle points of empty partition
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		if(pointCoarsePartInfoArr[coarsePartId]==0)
			for(unsigned int finePartId=0; finePartId<xMaxFinePartNum*yMaxFinePartNum; finePartId++){
				currFineBox = findPart(finePartId, currCoarseBox.getLowPoint(), currCoarseBox.getHighPoint(), xMaxFinePartNum, yMaxFinePartNum);
				double x = currFineBox.getLowPoint().getX() + (currFineBox.getHighPoint().getX() - currFineBox.getLowPoint().getX())/2.0;
				double y = currFineBox.getLowPoint().getY() + (currFineBox.getHighPoint().getY() - currFineBox.getLowPoint().getY())/2.0;
				point middlePoint;
				middlePoint.setX(x);
				middlePoint.setY(y);
				middlePoint.setId(globalPointIndex);
				globalPointIndex++;
				gridPointList.push_front(middlePoint);
			}
	}
*/

	//add middle points in all coarse partitions
	//generate other grid points (middle points)
	boundingBox currCoarseBox, currFineBox;
	//add middle points of empty partition
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		if(pointCoarsePartInfoArr[coarsePartId]==0){
			currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
			double x = currCoarseBox.getLowPoint().getX() + (currCoarseBox.getHighPoint().getX() - currCoarseBox.getLowPoint().getX())/2.0;
			double y = currCoarseBox.getLowPoint().getY() + (currCoarseBox.getHighPoint().getY() - currCoarseBox.getLowPoint().getY())/2.0;
			point middlePoint;
			middlePoint.setX(x);
			middlePoint.setY(y);
			middlePoint.setId(globalPointIndex);
			globalPointIndex++;
			gridPointList.push_back(middlePoint);
		}
	}


	point pTemp;
	double lowPartPointX, lowPartPointY, highPartPointX, highPartPointY, currCoor;
	unsigned int xFinepartNum, yFinepartNum;
	//collect additional points at bottom side of the domain
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum; coarsePartId++){//collect bottom points
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		lowPartPointX = currCoarseBox.getLowPoint().getX();

		xFinePartNum = rowFinePartNumArr[coarsePartId];
		double delta = 1.0/xFinePartNum;
		if(coarsePartId==0)	currCoor = lowPartPointX + delta;
		else currCoor = lowPartPointX;

		while(currCoor<lowPartPointX+1.0){
		
			pTemp.setX(currCoor); //x coordinate for botton line DA
			pTemp.setY(0.0); //y coordinate for bottom line DA
			pTemp.setId(globalPointIndex);
			globalPointIndex++;
			currCoor += delta;
			gridPointList.push_back(pTemp);
		}
	}

	//collect additional points at top side of the domain
	for(unsigned int coarsePartId=(xCoarsePartNum*yCoarsePartNum-xCoarsePartNum); coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){//collect top points
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		lowPartPointX = currCoarseBox.getLowPoint().getX();

		xFinePartNum = rowFinePartNumArr[coarsePartId];
		double delta = 1.0/xFinePartNum;
		if(coarsePartId==xCoarsePartNum*yCoarsePartNum-xCoarsePartNum) currCoor = lowPartPointX + delta;
		else currCoor = lowPartPointX;

		while(currCoor<lowPartPointX+1.0){
			pTemp.setX(currCoor); //x coordinate for botton line DA
			pTemp.setY(yCoarsePartNum); //y coordinate for bottom line DA
			pTemp.setId(globalPointIndex);
			globalPointIndex++;
			currCoor += delta;
			gridPointList.push_back(pTemp);
		}
	}

	//collect additional points at left side of the domain
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId = coarsePartId + yCoarsePartNum){//collect left points
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		lowPartPointY = currCoarseBox.getLowPoint().getY();

		yFinePartNum = rowFinePartNumArr[coarsePartId];
		double delta = 1.0/yFinePartNum;
		if(coarsePartId==0) currCoor = lowPartPointY + delta;
		else currCoor = lowPartPointY;

		while(currCoor<lowPartPointY+1.0){
			pTemp.setX(0.0); //x coordinate for botton line DA
			pTemp.setY(currCoor); //y coordinate for bottom line DA
			pTemp.setId(globalPointIndex);
			globalPointIndex++;
			currCoor += delta;
			gridPointList.push_back(pTemp);
		}

	}

	//collect additional points at right side of the domain
	for(unsigned int coarsePartId=xCoarsePartNum-1; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId = coarsePartId + xCoarsePartNum){//collect right points
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		lowPartPointY = currCoarseBox.getLowPoint().getY();

		yFinePartNum = rowFinePartNumArr[coarsePartId];
		double delta = 1.0/yFinePartNum;
		if(coarsePartId==xCoarsePartNum-1) currCoor = lowPartPointY + delta;
		else currCoor = lowPartPointY;

		while(currCoor<lowPartPointY+1.0){
			pTemp.setX(xCoarsePartNum); //x coordinate for botton line DA
			pTemp.setY(currCoor); //y coordinate for bottom line DA
			pTemp.setId(globalPointIndex);
			globalPointIndex++;
			currCoor += delta;
			gridPointList.push_back(pTemp);
		}
	}


	//transform gridPointList to gridPoints array
	gridPointsSize = gridPointList.size();
	gridPoints = new point[gridPointsSize];

	for(unsigned int pointId=0; pointId<gridPointsSize; pointId++){
		gridPoints[pointId] = gridPointList.front();
		gridPointList.pop_front();
	}


for(int i=0; i<gridPointsSize; i++)
std::cout<<", ["<<gridPoints[i].getX()<<", "<<gridPoints[i].getY()<<", "<<gridPoints[i].getId()<<"]";

	std::cout<<"gridPointsSize: "<<gridPointsSize<<"\n";
	std::cout<<"globalPointIndex: "<<globalPointIndex<<"\n";

	fwrite(gridPoints, sizeof(point), gridPointsSize, f);//-4 means not including 4 corner points
	fclose(f);

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
		xFinePartNum = yFinePartNum = rowFinePartNumArr[coarsePartId];
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			pointNum += pointPartInfoArr[coarsePartId][finePartId];
//std::cout<<pointPartInfoArr[coarsePartId][finePartId]<<" ";
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
	coorArr[gridPointsSize*2+3] = yCoarsePartNum;
	coorArr[gridPointsSize*2+4] = xCoarsePartNum;
	coorArr[gridPointsSize*2+5] = yCoarsePartNum;
	coorArr[gridPointsSize*2+6] = xCoarsePartNum;
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
	unsigned int xFinePartNum, yFinePartNum;
	fileStr = generateFileName(coarsePartId, destPath + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
	xFinePartNum = yFinePartNum = rowFinePartNumArr[coarsePartId];
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
	unsigned int xFinePartNum, yFinePartNum;
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		xFinePartNum = yFinePartNum = rowFinePartNumArr[coarsePartId];
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			printPointPart(coarsePartId, finePartId);
		}
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
	unsigned int xFinePartNum, yFinePartNum;

	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		boundingBox currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		xFinePartNum = yFinePartNum = rowFinePartNumArr[coarsePartId];
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
				if(pointNum==0) continue;
				pointArr = new point[pointNum];
				fread(pointArr, pointNum, sizeof(point), f);
				fclose(f);

				for(unsigned int pointId=0; pointId<pointNum; pointId++){
					if((pointArr[pointId].getX()>highPartPointX)||(pointArr[pointId].getX()<lowPartPointX)) stop = true;
					if((pointArr[pointId].getY()>highPartPointY)||(pointArr[pointId].getY()<lowPartPointY)) stop = true;
//std::cout<<"["<<pointArr[pointId].getX()<<","<<pointArr[pointId].getY()<<"] ";
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
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		delete [] pointPartInfoArr[coarsePartId];
	}
	delete [] pointPartInfoArr;

	delete [] pointCoarsePartInfoArr;
	delete [] rowFinePartNumArr;

	delete [] mapPointData;
	delete [] initPartPointList;

	delete [] initPointInfoArr;
	delete [] gridPoints;
}
