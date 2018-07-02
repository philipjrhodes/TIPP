#include "distribute.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//==============================================================================
distribute::distribute(int xPartNumber, int yPartNumber, std::string sourcePath, std::string verInfoFile, std::string verFile, std::string resultPath, unsigned int chunksize, int initsize){
	xPartNum = xPartNumber;
	yPartNum = yPartNumber;
	std::cout<<"xPartNumber: "<<xPartNum<<"   yPartNumber: "<<yPartNum<<std::endl;

	srcPath = sourcePath;
	vertexInfofilename = sourcePath + verInfoFile;
	vertexfilename = sourcePath + verFile;
	destPath = resultPath;

	xPartSize = 1.0/xPartNum;
	yPartSize = 1.0/yPartNum;

	chunkSize = chunksize;

	pointCoorArr = NULL;
	pointDuplicatedArr = NULL;
	pointGridArr = NULL;
	pointGridArr = NULL;
	partList = new std::list<double>[xPartNum*yPartNum];
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
	vertexRecordCount = atoi(strItem.c_str());
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

	unsigned int readingPos = 0;
	unsigned int readingSize = chunkSize;

	//contain each chunk of points that reads from file
	pointCoorArr = new double[vertexRecordSize*readingSize];
	if(!pointCoorArr){
		std::cout<<"There is not enough memory to allocate for pointCoorArr"<<std::endl;
		exit(1);
	}

	//read points from file
	//-4 because we do not read the last 4 points of 4 corners of the domain
//	while (readingPos < vertexRecordCount-4){ //not read all points in file mydatabin.ver
	while (readingPos < vertexRecordCount){ //not read all points in file mydatabin.ver
		if((readingPos + chunkSize)>vertexRecordCount-1)
			readingSize = vertexRecordCount-readingPos;
			std::cout<<"reading position: "<<readingPos<<" of fileSize: "<<vertexRecordCount<<std::endl;
		//reading points with readingSize
		fread(pointCoorArr, sizeof(double), vertexRecordSize*readingSize, fcoorData);

		//Distribute points into partitions
		for(unsigned int index=0; index<readingSize; index++){
			point currPoint(pointCoorArr[index*vertexRecordSize], pointCoorArr[index*vertexRecordSize+1]);
			int currPartIndex = partIndex(currPoint);
			//Distribute points into initPointArr
			if(initPointInfoArr[currPartIndex]>0){//need contribute points for initPointArr
				initPointArr[currPartIndex*initSize + initSize-initPointInfoArr[currPartIndex]] = currPoint;
				initPointInfoArr[currPartIndex]--;
			}
			else{
				pointPartInfoArr[currPartIndex]++;//update pointPartInfo as new point coming
				//push current point to current partition
				partList[currPartIndex].push_back(currPoint.getX());
				partList[currPartIndex].push_back(currPoint.getY());
			}
		}

		//Write all point coordinates in the same partition into its file

		for(unsigned int i=0; i<xPartNum*yPartNum; i++)
			if(!partList[i].empty()){
				writeListToFile(partList[i], i);
				std::list<double>::iterator it;
				partList[i].clear();
			}
	
		readingPos = readingPos + chunkSize;
	}
    fclose(fcoorData);

	//write all init points to file initPoints.ver
	std::string fileStr = destPath+"initPoints.ver";
	FILE *f = fopen(fileStr.c_str(), "wb");

	double *data = new double[initPointArrSize*vertexRecordSize];
	int index = 0;

	for(int i=0; i<initPointArrSize; i++){
		data[vertexRecordSize*i] = initPointArr[i].getX();
		data[vertexRecordSize*i+1] = initPointArr[i].getY();
//std::cout<<data[vertexRecordSize*i]<<" "<<data[vertexRecordSize*i+1]<<std::endl;
	}
	fwrite(data, initPointArrSize*vertexRecordSize, sizeof(double), f);
	fclose(f);
	delete [] data;


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
void distribute::writeListToFile(std::list<double> pointList, int partitionId){
//std::cout<<"---"<<partitionId<<std::endl;
	unsigned int size = pointList.size();
	double *coorData = new double[size];
	unsigned int index = 0;
	std::list<double>::iterator it;
	for (it = pointList.begin(); it != pointList.end(); it++){
		coorData[index] = (*it);
		index++;
	}
	writeToBinaryFile(destPath + "pointPart", partitionId, coorData, size);
	delete [] coorData;
}

//==============================================================================
//append data with size to a binary file, for ex: with partition1 in 16 partitions, the file name would be
// "pointPartxx.ver" in --> "./dataSources/10vertices/pointPartitions"
void distribute::writeToBinaryFile(std::string fileStr, int partitionId, double *data, unsigned int size){
	fileStr = generateFileName(partitionId, destPath + "pointPart", xPartNum*yPartNum);

	FILE *f = fopen(fileStr.c_str(), "a");//appending 
	if(!f) {std::cout<<"not exist "<<fileStr<<std::endl; exit(1);}
	fwrite(data, size, sizeof(double), f);
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
		std::cout<<"\n\npartition "<<partitionId<<":\n";
		double *data = new double[pointNum*vertexRecordSize];
		fread(data, pointNum*vertexRecordSize, sizeof(double), f);
		for(int i=0; i<pointNum; i++)
			std::cout<<data[i*vertexRecordSize]<<" "<<data[i*vertexRecordSize+1]<<std::endl;
		delete [] data;
	}
	fclose(f);
}

void distribute::printAllPartitions(){
	//open main partition file (contains all partitions)
	std::string mainFileStr = destPath + "pointPart.ver";
	FILE *f = fopen(mainFileStr.c_str(), "rb");
	if(!f)	{std::cout<<" can not open "<<mainFileStr<<"\n"; return;}

	unsigned int allPointNum = 0;
	for(int i=0; i<xPartNum*yPartNum; i++) allPointNum = allPointNum + pointPartInfoArr[i];
	double *data = new double[allPointNum*vertexRecordSize];
	fread(data, allPointNum*vertexRecordSize, sizeof(double), f);

	std::cout<<"===================the number of points in the domain===================: "<<allPointNum<<"\n";
	for(unsigned int i=0; i<allPointNum; i++)
		std::cout<<data[i*vertexRecordSize]<<" "<<data[i*vertexRecordSize+1]<<std::endl;
	delete [] data;
	fclose(f);
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
void distribute::sort(){
	//open main partition file (contains all partitions)
	std::string mainFileStr = destPath + "pointPart.ver";
	FILE *f = fopen(mainFileStr.c_str(), "wb");
	if(!f)	{std::cout<<" can not open "<<mainFileStr<<"\n"; return;}

	//scan all partition files, sort them, then add to main data file: pointPart.ver

	for(int partId = 0; partId<xPartNum*yPartNum; partId++){
		//read point data from file to point array x1 y1 x2 y2 ...
		std::string fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum);
		unsigned int pointNum = pointPartInfoArr[partId];
		if(pointNum==0){
			std::cout<<fileStr<<" does not exist\n";
			continue;
		}
		double *data = new double[pointNum*vertexRecordSize];
		FILE *f1 = fopen(fileStr.c_str(), "rb");
		//if(!f1){
		//	std::cout<<fileStr<<" does not exist\n"; return;}
		//else{
			fread(data, pointNum*vertexRecordSize, sizeof(double), f1);
std::cout<<"sort the content in file ("<<pointNum<<") "<<fileStr<<std::endl;
		//}
		fclose(f1);
		//delete this partition file in folder pointPartitions 
		remove(fileStr.c_str());

		//transform into array of points
		point *pointArr = new point[pointNum];
		for(unsigned int index=0; index<pointNum; index++){
			pointArr[index].setX(data[index*vertexRecordSize]);
			pointArr[index].setY(data[index*vertexRecordSize+1]);
		}

		//sort based on coordinate x for array of points
		qsort(pointArr, pointNum, sizeof(point), int_compar);

		//copy back to point array
		for(unsigned int index=0; index<pointNum; index++){
			data[index*vertexRecordSize] = pointArr[index].getX();
			data[index*vertexRecordSize+1] = pointArr[index].getY();
		}
		//write back to pointPart.ver 
		fwrite(data, pointNum*vertexRecordSize, sizeof(double), f);

		//Release some temporary arrays
		delete [] pointArr;
		delete [] data;
	}
	fclose(f);
}

//==============================================================================
//These points are not belong to original data, they are generated follow the grid of xPartNum x yPartNum, and locaed inside doamin, not on square border
void distribute::generateGridPoints(){
	pointGridArrSize = ((xPartNum+1)*2-1)*((yPartNum+1)*2-1);
	pointGridArr = new double[pointGridArrSize*vertexRecordSize];

	pointDuplicatedArr = new bool[pointGridArrSize];
	//initialize
	for(int i=0; i<pointGridArrSize; i++)
		pointDuplicatedArr[i] = false;

	int pointIndex = 0;

	for(int row=0; row<=xPartNum*2; row++)
		for(int col=0; col<=yPartNum*2; col++){
			pointGridArr[vertexRecordSize*pointIndex] = col*xPartSize/2;
			pointGridArr[vertexRecordSize*pointIndex + 1] = row*yPartSize/2;
//std::cout<<pointGridArr[vertexRecordSize*pointIndex]<<","<<pointGridArr[vertexRecordSize*pointIndex + 1]<<std::endl;
			pointIndex++;
		}
}

//==============================================================================
//check gridPoints to make sure not duplicate with original data.
void distribute::checkDuplication(){
	double epsilon = 0.0000000000001;
/*
	//read data from files
	for(int partId=0; partId<xPartNum*yPartNum; partId++){
		unsigned int pointNum = pointPartInfoArr[partId];
		std::string fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum);	
		if(pointNum==0){
			std::cout<<fileStr<<" does not exist\n";
			return;
		}
		//read data file of a partition
		double *data = new double[pointNum*vertexRecordSize];
		FILE *f = fopen(fileStr.c_str(), "rb");
		if(!f){
			std::cout<<fileStr<<" does not exist\n"; return;}
		else{
			fread(data, pointNum*vertexRecordSize, sizeof(double), f);
			std::cout<<"Open file "<<fileStr<<std::endl;
		}
		fclose(f);

		//check the duplication
		for(int i=0; i<pointNum; i++)
			for(int index=0; index<pointGridArrSize; index++)
				if((fabs(data[i*vertexRecordSize]-pointGridArr[index*vertexRecordSize])<epsilon)&&(fabs(data[i*vertexRecordSize+1]-pointGridArr[index*vertexRecordSize+1])<epsilon)){
					pointDuplicatedArr[index] = true;
				}
		delete [] data;
	}
*/
	//save gridPointArr to file
	std::string fileStr = destPath+"gridPoints.ver";
	FILE *f = fopen(fileStr.c_str(), "wb");
	int size = 0;
	for(int i=0; i<pointGridArrSize; i++)
		if(!pointDuplicatedArr[i]) size++;

std::cout<<"size = "<<size<<std::endl;

	double *data = new double[size*vertexRecordSize];
	int index = 0;
	for(int i=0; i<pointGridArrSize; i++){
		if(!pointDuplicatedArr[i]){
			data[index*vertexRecordSize] = pointGridArr[i*vertexRecordSize];
			data[index*vertexRecordSize+1] = pointGridArr[i*vertexRecordSize+1];
			index++;
		}
	}
//for(int i=0; i<size; i++)
//std::cout<<data[i*vertexRecordSize]<<" "<<data[i*vertexRecordSize+1]<<std::endl;
	//write data to file
	fwrite(data, size*vertexRecordSize, sizeof(double), f);
	fclose(f);
	delete [] data;

	//write size to pointPartInfo.xfdl
	std::ofstream pointPartInfoFile(destPath + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::app);//app means append to the end of file
	pointPartInfoFile<<"\n"<<size;
	pointPartInfoFile.close();

}

//==============================================================================
//merge all partition files into one files
void distribute::mergeAllPartitions(){
	//open main partition file (contains all partitions)
	std::string mainFileStr = destPath + "pointPart.ver";
	FILE *f = fopen(mainFileStr.c_str(), "wb");
	if(!f)	{std::cout<<" can not open "<<mainFileStr<<"\n"; return;}


	for(int index = 0; index<xPartNum*yPartNum; index++){
		int partId = index;
		std::string fileStr = generateFileName(partId, destPath + "pointPart", xPartNum*yPartNum);
		//number of points in a partition
		unsigned int pointNum = pointPartInfoArr[partId];
		if(pointNum==0){
			std::cout<<fileStr<<" does not exist\n";
			continue;
		}
		double *data = new double[pointNum*vertexRecordSize];

		//open current partition file
		FILE *f1 = fopen(fileStr.c_str(), "rb");
		if(!f1){
			std::cout<<fileStr<<" does not exist\n"; continue;}
		else{
			fread(data, pointNum*vertexRecordSize, sizeof(double), f1);
			std::cout<<"read the content in file ("<<pointNum<<") "<<fileStr<<std::endl;
		}
		fclose(f1);

		//write buffer data to main file
		fwrite(data, pointNum*vertexRecordSize, sizeof(double), f);
		delete [] data;
	}
	fclose(f);
}


//==============================================================================
//merge all partition files and init points and 4 points of domain square (0,0), (0,1), (1,1), (1,0) into one files
//intPoints-Part0 & pointPart0 & intPoints-Part1 & pointPart1 & ... & 4 points of square
//4 points of domain square will be placed in the end of file
void distribute::mergeAllPartitionsAndInitPoints(){

	//Open initPoints.ver to intPointsArr
	std::string intiFileStr = destPath + "initPoints.ver";
	FILE *f1 = fopen(intiFileStr.c_str(), "rb");
	if(!f1)	{std::cout<<" can not open "<<intiFileStr<<"\n"; return;}
	double *initPointArr = new double[initPointArrSize*vertexRecordSize];
	fread(initPointArr, initPointArrSize*vertexRecordSize, sizeof(double), f1);
	fclose(f1);

	//open main partition file (contains all partitions & initPoints and 4 square points)
	std::string mainFileStr = destPath + "fullPointPart.ver";
	FILE *f = fopen(mainFileStr.c_str(), "wb");
	if(!f)	{std::cout<<" can not open "<<mainFileStr<<"\n"; return;}

/*	//compute fullPointPartArrSize
	unsigned int fullPointPartArrSize = 0;
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		fullPointPartArrSize = fullPointPartArrSize + pointPartInfoArr[i];
	}
	fullPointPartArrSize = fullPointPartArrSize + initPointArrSize + 4;
*/
	//open pointPart.ver, read each chunk (part size) then append to fullPointPart.ver
	std::string pointPartFileStr = destPath + "pointPart.ver";
	FILE *f2 = fopen(pointPartFileStr.c_str(), "rb");
	if(!f2)	{std::cout<<" can not open "<<pointPartFileStr<<"\n"; return;}

	//write size to pointPartInfo.xfdl
	std::ofstream pointPartInfoFile(destPath + "pointPartInfo.xfdl", std::ofstream::out | std::ofstream::app);//app means append to the end of file
	pointPartInfoFile<<"\n";

	//generate pointTotralNum array for full partition sizes (include init points)
	unsigned int *pointTotalNumArr = new unsigned int[xPartNum*yPartNum];


	//read point coordinates from each partition and from initPointArr and copy to fullPointPartArr.ver

	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		//based on part sizes in pointPartInfoArr and the initSize, append data to fullPointPartArr.ver

		//write the init points of each partition to fullPointPartArr
		fwrite(&initPointArr[partId*initSize*vertexRecordSize], initSize*vertexRecordSize, sizeof(double), f);

		//number of points in a partition
		unsigned int pointNum = pointPartInfoArr[partId];
		
		pointTotalNumArr[partId] = pointNum + initSize;
		//write partition size (pointNum + initSize) for each partition
		pointPartInfoFile<<pointTotalNumArr[partId]<<" ";
		
		if(pointNum==0)	continue;

		double *data = new double[pointNum*vertexRecordSize];
		//read point coordinates from pointPart.ver
		fread(data, pointNum*vertexRecordSize, sizeof(double), f2);
		//write the partition points of each partition to fullPointPartArr
		fwrite(data, pointNum*vertexRecordSize, sizeof(double), f);
		delete [] data;
	}

	//write 4 points of square (0,0), (0,1), (1,1), (1,0) and grid points on the bounding domain in the end of fullPointPartArr.ver
	//grid points on the domian are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum+1)*(yPartNum+1) including 4 corners.
	double interval = 1.0/xPartNum;
	double *squarePoints = new double[xPartNum*4*vertexRecordSize];

	//4 corners of square domain
	squarePoints[0] = 0.0; //coordinate x of point A
	squarePoints[1] = 0.0; //coordinate y of point A

	squarePoints[2] = 0.0; //coordinate x of point B
	squarePoints[3] = 1.0; //coordinate y of point B

	squarePoints[4] = 1.0; //coordinate x of point C
	squarePoints[5] = 1.0; //coordinate y of point C

	squarePoints[6] = 1.0; //coordinate x of point D
	squarePoints[7] = 0.0; //coordinate y of point D

	//generate other grid points
	int index = 4;//after 4 corner points
	double currentCoor = interval;
	for(int i=0; i<xPartNum-1; i++){
		squarePoints[index*vertexRecordSize] = currentCoor; //x coordinate for botton line DA
		squarePoints[index*vertexRecordSize+1] = 0.0; //y coordinate for bottom line DA

		squarePoints[index*vertexRecordSize+2] = 1.0 - currentCoor; //x coordinate for top line BC
		squarePoints[index*vertexRecordSize+3] = 1; //x coordinate for top line BC

		squarePoints[index*vertexRecordSize+4] = 0.0; //x coordinate for left line AB
		squarePoints[index*vertexRecordSize+5] = 1.0 - currentCoor; //x coordinate for left line AB

		squarePoints[index*vertexRecordSize+6] = 1.0; //x coordinate for right line CD
		squarePoints[index*vertexRecordSize+7] = currentCoor; //x coordinate for right line CD
		
		currentCoor += interval;
		index +=4;
	}

for(int i=0; i<xPartNum*4;i++)
std::cout<<squarePoints[i*2]<<" "<<squarePoints[i*2+1]<<std::endl;


//	double squarePoints[] = {0.0,0.0, 0.0,1.0, 1.0,1.0, 1.0,0.0};
//	fwrite(squarePoints, 4*vertexRecordSize, sizeof(double), f);
	fwrite(squarePoints, sizeof(double), xPartNum*4*vertexRecordSize, f);
//	pointPartInfoFile<<4<<"\n";
	pointPartInfoFile<<xPartNum*yPartNum<<"\n";

	//generate pointNumOffsetArr offset for partition sizes (includes init points), 
	//+1 mean the last part contains 4 points of square domain and other grid points on 4 edges AB, BC, CD, DA
	//update pointIdArr
	unsigned int pointNumOffset = 0;
	pointPartInfoFile<<pointNumOffset<<" ";
	for(int i=1; i<xPartNum*yPartNum+1; i++){
		pointNumOffset = pointNumOffset + pointTotalNumArr[i-1];
		pointPartInfoFile<<pointNumOffset<<" ";
	}
	pointPartInfoFile.close();

	fclose(f);
	fclose(f2);

	delete [] initPointArr;
	delete [] pointTotalNumArr;
	delete [] squarePoints;
//	delete [] pointNumOffsetArr;
}
//==============================================================================
distribute::~distribute(){
	if(pointCoorArr!=NULL) delete [] pointCoorArr;
	delete [] pointPartInfoArr;
	if(pointGridArr!=NULL) delete [] pointGridArr;
	if(pointDuplicatedArr!=NULL) delete [] pointDuplicatedArr;
	delete [] initPointArr;
	delete [] initPointInfoArr;
}
