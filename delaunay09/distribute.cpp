#include "distribute.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//This program generate 4 files initPoints.ver, pointCoorPart.ver, pointIdPart.ver, and pointPartInfo.xfdl
//initPoints.ver contains all init points (which is collected from all partitions) and grid points (NOT include 4 corners)
//pointCoorPart.ver contains all points in all partitions (after extracting out init points, the init point which are extrated out are equal for all partitions)
//pointCoorPart.ver --> {partition 0: (x1, y1) (x2, y2),..} {partition 1: (x3, y4) (x5, y6),..} .... {partition 15: (xn, yn) (xn+1, yn+1),..}
//pointIdPart.ver --> {partition 0: (id1) (id2),..} {partition 1: (id3) (id4),..} .... {partition 15: (idn) (idn+1),..}
//pointPartInfo.xfdl:
//- first line: size of the domain (how many partitions) ex: (4 x 4)
//- second line: number of points in each partition not including init points
//- third line: offsets of number of points in second line
//- fouth line: total number of init points, each partition has the same number of init point as the others
//- fifth line: number of grid points (not include 4 corner points) of domain
/*for ex, with 1Kvertices
4 4
46 48 41 49 27 51 41 45 34 51 52 40 36 39 38 42 
0 46 94 135 184 211 262 303 348 382 433 485 525 561 600 638 
320
12
*/
//The result data include two files: triangleIds.tri (contains all indices of points of fullPointPart.ver) and fullPointPart.ver
//fullPointPart.ver contains all points in the domain and gris points (include 4 corner points)
//g++ -std=gnu++11 common.cpp point.cpp distribute.cpp distributedMain.cpp -o distribute
//rm ../dataSources/500Mvertices/delaunayResults/*.*
//./distribute 128 128 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 25
//==============================================================================
distribute::distribute(double domainsize, double distance, int xPartNumber, int yPartNumber, std::string sourcePath, std::string verInfoFile, std::string verFile, std::string resultPath, unsigned long long chunksize, int initsize){
	domainSize = domainsize;
	xPartNum = xPartNumber;
	yPartNum = yPartNumber;
	std::cout<<"xPartNumber: "<<xPartNum<<"   yPartNumber: "<<yPartNum<<std::endl;

	interval = distance;
	globalPointIndex = 0;

	srcPath = sourcePath;
	vertexInfofilename = sourcePath + verInfoFile;
	vertexfilename = sourcePath + verFile;
	destPath = resultPath;

	xPartSize = domainSize/xPartNum;
	yPartSize = domainSize/yPartNum;
//std::cout<<xPartSize<<" "<<yPartSize<<" "<<domainSize<<"\n";
	chunkSize = chunksize;

	pointCoorArr = NULL;
	pointGridArr = NULL;
	pointGridArr = NULL;
	gridPoints = NULL;
	partList = new std::list<point>[xPartNum*yPartNum];
	pointPartInfoArr = new unsigned int[xPartNum*yPartNum];
	for(int i=0; i<xPartNum*yPartNum; i++) pointPartInfoArr[i] = 0;

	//initialize initPointArr for the first phase of delaunay
	initSize = initsize;
	initPointArrSize = initSize*xPartNum*yPartNum;
	initPointArr = new point[initPointArrSize];
	initPointInfoArr = new int[xPartNum*yPartNum];
	for(int i=0; i<xPartNum*yPartNum; i++)
		initPointInfoArr[i] = initSize;


//	pointPartInfoArr = new unsigned int[xPartNum];
//	for(int i=0; i<xPartNum; i++) pointPartInfoArr[i] = 0;

   //Read information from binary vertex file mydatabin.ver.xfdl 
	std::ifstream vertexInfofile(vertexInfofilename.c_str());
	if(!vertexInfofile){
		std::cout<<"non exist "<<vertexInfofilename.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	vertexInfofile >> strItem;
	vertexRecordSize = atoi(strItem.c_str());
	vertexInfofile >> strItem;

	vertexRecordCount = atoll(strItem.c_str());
std::cout<<strItem<<" "<<vertexRecordCount<<"\n";
	vertexInfofile.close();

	std::cout.precision(16);

	//remove all file in pointPartitions before create new files
	std::string delCommand = "rm -f " + destPath + "*.*";
	system(delCommand.c_str());

}
/*
//==============================================================================
//partition a point into 1x2^k partitions
int distribute::partIndex1(point p){
	double x = p.getX();

	int gridX = x/xPartSize;//std::cout<<x<<" ";
	if(gridX>=xPartNum) gridX = gridX-1;

	return gridX;
}
*/

//==============================================================================
//partition a point into 2^k x 2^l partitions
int distribute::partIndex(point p){
	double x = p.getX();
	double y = p.getY();

	int gridX = x/xPartSize;
	if(gridX>=xPartNum) gridX = gridX-1;
	int gridY = y/yPartSize;
	if(gridY>=xPartNum) gridY = gridY-1;

	return gridY*xPartNum + gridX;
}

//==============================================================================
//read billion points from file, partition them into partitions, 
//for ex: a quare domain can be partitioned int to 4 column and 4 rows --> 16 partitions
//store each partition of points to file
void distribute::pointsDistribute(){
	FILE *fcoorData=fopen(vertexfilename.c_str(), "rb");
	if(!fcoorData) {std::cout<<"not exist "<<vertexfilename<<std::endl; exit(1);}

	unsigned long long readingPos = 0;
	unsigned long long readingSize = chunkSize;

	//contain each chunk of points that reads from file
	pointCoorArr = new double[vertexRecordSize*readingSize];
	if(!pointCoorArr){
		std::cout<<"There is not enough memory to allocate for pointCoorArr"<<std::endl;
		exit(1);
	}

	//read points from file
	while (readingPos < vertexRecordCount){ //not read all points in file mydatabin.ver
		if((readingPos + chunkSize)>vertexRecordCount-1)
			readingSize = vertexRecordCount-readingPos;
		std::cout<<"reading position: "<<readingPos<<" of fileSize: "<<vertexRecordCount<<std::endl;
		//reading points with readingSize
		fread(pointCoorArr, sizeof(double), vertexRecordSize*readingSize, fcoorData);

		//Distribute points into partitions
		for(unsigned long long index=0; index<readingSize; index++){
			//Each point is assigned to a global index which is the index of orginal point dataset (mydatabin.ver in rawPointData)
			point currPoint(pointCoorArr[index*vertexRecordSize], pointCoorArr[index*vertexRecordSize+1], globalPointIndex);
			globalPointIndex++;
			int currPartIndex = partIndex(currPoint);

			//Distribute points into initPointArr
			if(initPointInfoArr[currPartIndex]>0){//need contribute points for initPointArr
				initPointArr[currPartIndex*initSize + initSize-initPointInfoArr[currPartIndex]] = currPoint;
				initPointInfoArr[currPartIndex]--;
			}
			else{
				pointPartInfoArr[currPartIndex]++;//update pointPartInfo as new point coming
				//push current point to current partition
				partList[currPartIndex].push_back(currPoint);
			}
		}

		//Write all point coordinates in the same partition into its file

		for(unsigned int i=0; i<xPartNum*yPartNum; i++)
			if(!partList[i].empty()){
				writeListToFile(partList[i], i);
//				std::list<double>::iterator it;
				partList[i].clear();
			}
	
		readingPos = readingPos + chunkSize;
	}
    fclose(fcoorData);
	delete [] pointCoorArr;

	//write all init points to file initPoints.ver
	std::string fileStr = destPath+"initPoints.ver";
	FILE *f = fopen(fileStr.c_str(), "wb");
	fwrite(initPointArr, initPointArrSize, sizeof(point), f);
	fclose(f);

	//store pointPartInfo array to pointPartInfo.xfdl file
	//write to file pointPartInfo.txt
	//trunc means If the file is opened for output operations and it already existed, 
	//its previous content is deleted and replaced by the new one.
	std::ofstream pointPartInfoFile(destPath + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::trunc);
	pointPartInfoFile<<xPartNum<<" "<<yPartNum<<"\n";
	for(unsigned int i=0; i<xPartNum*yPartNum; i++)
		pointPartInfoFile<<pointPartInfoArr[i]<<" ";
	pointPartInfoFile<<"\n";

	//generate pointNumOffset for number of point of each partition 
	unsigned int pointNumOffset = 0;
	pointPartInfoFile<<pointNumOffset<<" ";
	for(int i=1; i<xPartNum*yPartNum; i++){
		//pointNumOffset = pointNumOffset + pointPartInfoArr[i-1] + initSize;
		pointNumOffset = pointNumOffset + pointPartInfoArr[i-1];
		pointPartInfoFile<<pointNumOffset<<" ";
	}

	pointPartInfoFile<<"\n"<<initPointArrSize;
std::cout<<"initPointArrSize: "<<initPointArrSize<<"\n";
	pointPartInfoFile.close();
}

//==============================================================================
void distribute::writeListToFile(std::list<point> pointList, int partitionId){
//std::cout<<"---"<<partitionId<<std::endl;
	unsigned int size = pointList.size();
	point *pointArr = new point[size];
	unsigned int index = 0;
	std::list<point>::iterator it;
	for (it = pointList.begin(); it != pointList.end(); it++){
		pointArr[index] = (*it);
		index++;
	}
	writeToBinaryFile(destPath + "pointPart", partitionId, pointArr, size);
	delete [] pointArr;
}

//==============================================================================
//append data with size to a binary file, for ex: with partition1 in 16 partitions, the file name would be
// "pointPartxx.ver" in --> "./dataSources/10vertices/pointPartitions"
void distribute::writeToBinaryFile(std::string fileStr, int partitionId, point *data, unsigned int size){
	fileStr = generateFileName(partitionId, destPath + "pointPart", xPartNum*yPartNum);

	FILE *f = fopen(fileStr.c_str(), "a");//appending 
	if(!f) {std::cout<<"not exist "<<fileStr<<std::endl; exit(1);}
	fwrite(data, size, sizeof(point), f);
	fclose(f);
}

//==============================================================================
void distribute::printPointPartitions(int partitionId){

	//form a file name from partitionId (partitionId = 5 --> pointPart05.ver)
	std::string fileStr = generateFileName(partitionId, destPath + "pointPart", xPartNum*yPartNum);

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

void distribute::printAllPartitions(){//not include init points
	//open main partition file (contains all partitions)
	std::string fileStr1 = destPath + "pointCoorPart.ver";
	std::string fileStr2 = destPath + "pointIdPart.ver";
	FILE *f1 = fopen(fileStr1.c_str(), "rb");
	if(!f1)	{std::cout<<" can not open "<<fileStr1<<"\n"; return;}
	FILE *f2 = fopen(fileStr2.c_str(), "rb");
	if(!f2)	{std::cout<<" can not open "<<fileStr2<<"\n"; return;}

	unsigned int allPointNum = 0;
	for(int i=0; i<xPartNum*yPartNum; i++) allPointNum = allPointNum + pointPartInfoArr[i];
	double *coorArr = new double[allPointNum*vertexRecordSize];
	unsigned long int *idArr = new unsigned long int[allPointNum];

	fread(coorArr, allPointNum*vertexRecordSize, sizeof(double), f1);
	fread(idArr, allPointNum, sizeof(unsigned long int), f2);
	fclose(f1);
	fclose(f2);

	std::cout<<"===================the number of points in the domain===================: "<<allPointNum<<"\n";
	for(unsigned int i=0; i<allPointNum; i++)
		std::cout<<coorArr[i*vertexRecordSize]<<" "<<coorArr[i*vertexRecordSize+1]<<" "<<idArr[i]<<std::endl;

	delete [] coorArr;
	delete [] idArr;
}

void distribute::printInitPointsAndPartitions(){
	//open main partition file (contains all partitions)
	std::string mainFileStr = destPath + "fullPointPart.ver";
	FILE *f = fopen(mainFileStr.c_str(), "rb");
	if(!f)	{std::cout<<" can not open "<<mainFileStr<<"\n"; return;}

	unsigned int allPointNum = 0;
	for(int i=0; i<xPartNum*yPartNum; i++) allPointNum = allPointNum + pointPartInfoArr[i] + initSize;
	double *data = new double[allPointNum*vertexRecordSize];
	fread(data, allPointNum*vertexRecordSize, sizeof(double), f);

	std::cout<<"===================the number of points in the domain===================: "<<allPointNum<<"\n";
//	for(unsigned int i=0; i<allPointNum; i++)
//		std::cout<<data[i*vertexRecordSize]<<" "<<data[i*vertexRecordSize+1]<<std::endl;
	delete [] data;
	fclose(f);
}
//==============================================================================
int int_compar(const void *p1, const void *p2)
{
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}
//==============================================================================
//use qsort of C++
//sort all points in all partition, then add them to one main file: pointPart.ver
void distribute::sortAdd(){
	//open main partition file (contains all partitions)
//	std::string mainFileStr = destPath + "pointPart.ver";
//	FILE *f = fopen(mainFileStr.c_str(), "wb");
//	if(!f)	{std::cout<<" can not open "<<mainFileStr<<"\n"; return;}
//	fclose(f);

	//scan all partition files, sort them, then add to main data file: pointPart.ver
	for(int partId = 0; partId<xPartNum*yPartNum; partId++){
		//read point data from file to point array x1 y1 x2 y2 ...
		std::string fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum);
		unsigned int pointNum = pointPartInfoArr[partId];
		if(pointNum==0){
			std::cout<<fileStr<<" does not exist\n";
			continue;
		}
		point *pointArr = new point[pointNum];
		FILE *f1 = fopen(fileStr.c_str(), "rb");
		if(!f1){
			std::cout<<fileStr<<" does not exist\n"; return;}
		else{
			fread(pointArr, pointNum, sizeof(point), f1);
std::cout<<"sort the content in file ("<<pointNum<<") "<<fileStr<<std::endl;
		}
		fclose(f1);
		//delete this partition file in folder pointPartitions 
		remove(fileStr.c_str());

		//sort based on coordinate x for array of points
		qsort(pointArr, pointNum, sizeof(point), int_compar);

		//write back to pointPart.ver 
//		fwrite(pointArr, pointNum, sizeof(point), f);

		appendTwoFiles(pointArr, pointNum);

		//Release some temporary arrays
		delete [] pointArr;
	}
	
}

//==============================================================================
//From a an point array of each partition (not include init points)
//separate coordinates and Ids and store to two files pointCoorPart.ver and pointIdPart.ver
void distribute::appendTwoFiles(point *pointArr, unsigned int pointNum){
	if(pointNum<=0) return;
	double *pointCoorArr = new double[pointNum*vertexRecordSize];
	unsigned long int *pointIdArr = new unsigned long int[pointNum];

	//separate coordinates and Ids
	for(unsigned int index=0; index<pointNum; index++){
		pointCoorArr[index*vertexRecordSize] = pointArr[index].getX();
		pointCoorArr[index*vertexRecordSize+1] = pointArr[index].getY();
		pointIdArr[index] = pointArr[index].getId();
	}

	//Write coordinates and Ids to files pointCoorPart.ver and pointIdPart.ver
	std::string fileStr1 = destPath + "pointCoorPart.ver";
	FILE *f1 = fopen(fileStr1.c_str(), "a");
	fwrite(pointCoorArr, pointNum*vertexRecordSize, sizeof(double), f1);
	fclose(f1);

	std::string fileStr2 = destPath + "pointIdPart.ver";
	FILE *f2 = fopen(fileStr2.c_str(), "a");
	fwrite(pointIdArr, pointNum, sizeof(unsigned long int), f2);
	fclose(f2);

	delete [] pointCoorArr;
	delete [] pointIdArr;
}

//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(){

	//Open initPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = destPath + "initPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	unsigned int gridPointsSize = (domainSize/interval)*4-4;
std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";
	//grid points on the domain edges are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum*4) including 4 corners.
	gridPoints = new point[gridPointsSize];//-4 means not including 4 corner points

	//generate other grid points
	int index = 0;
	double currentCoor = interval;
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
		
		currentCoor += interval;
		index +=4;
	}
	fwrite(gridPoints, sizeof(point), xPartNum*4-4, f);//-4 means not including 4 corner points
	fclose(f);

for(int i=0; i<xPartNum*4-4;i++)
std::cout<<gridPoints[i].getX()<<" "<<gridPoints[i].getY()<<std::endl;


	//write number of grid points to pointPartInfo.xfdl
	//app means append to the end of file
	std::ofstream pointPartInfoFile(destPath + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::app);
	pointPartInfoFile<<"\n";
	pointPartInfoFile<<xPartNum*4-4<<"\n";
	pointPartInfoFile.close();

/*	std::string initPointFileStr = destPath + "initPoints.ver";
	FILE *f1 = fopen(initFileStr.c_str(), "rb");
	point *initPointArr = new point[320+12];
	fread(initPointArr, sizeof(point), 320+12, f1);
	fclose(f1);
	for(int i=0; i<320+12; i++)
		std::cout<<initPointArr[i].getX()<<" "<<initPointArr[i].getY()<<" "<<initPointArr[i].getId()<<"\n";
	delete [] initPointArr;
*/
}
//==============================================================================
//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
void distribute::createFullPointCoorData(){
	std::cout<<"Create full point partitions and grid points ...\n";
	//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
	std::string cpCommand = "cp " + srcPath + "mydatabin.ver " + destPath + "fullPointPart.ver";
	system(cpCommand.c_str());

	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	//coorArr contains all coordinates of grid points including 4 corner point
	//coorArr does not contain Ids
	double *coorArr = new double[xPartNum*4*vertexRecordSize];
	//copy coordinates from gridPoints to coorArr
	//note that gridPoints contains all grid points except 4 corner points
	int index = 0;
	for(index=0; index<xPartNum*4-4; index++){
		coorArr[index*vertexRecordSize] = gridPoints[index].getX();
		coorArr[index*vertexRecordSize+1] = gridPoints[index].getY();
	}
	//Assign 4 corner coordinates of domain to coorArr
	//note that: domain is a square ABCD between (0,0) and (1,1)
	//four corners A(0,0), B(0,1), C(1,1) and D(1,0)
	coorArr[index*vertexRecordSize] = 0;
	coorArr[index*vertexRecordSize+1] = 0;
	coorArr[index*vertexRecordSize+2] = 0;
	coorArr[index*vertexRecordSize+3] = domainSize;
	coorArr[index*vertexRecordSize+4] = domainSize;
	coorArr[index*vertexRecordSize+5] = domainSize;
	coorArr[index*vertexRecordSize+6] = domainSize;
	coorArr[index*vertexRecordSize+7] = 0;

	//add coorArr to fullPointPart.ver
	//That mean fullPointPart.ver will have all points in the domain and all grid points (include 4 corner points)
	std::string fileStr = destPath + "fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<fileStr<<"\n"; return;}
	fwrite(coorArr, sizeof(double), xPartNum*4*vertexRecordSize, f);
	fclose(f);
	delete [] coorArr;
}

//==============================================================================
distribute::~distribute(){
	delete [] pointPartInfoArr;
	if(pointGridArr!=NULL) delete [] pointGridArr;
	delete [] initPointArr;
	delete [] initPointInfoArr;
	delete [] gridPoints;
	delete [] partList;
}
