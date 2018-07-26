#include "distribute.h"
#include "common.h"
#include "boundingBox.h"
#include <iostream>
#include <fstream>
#include <cmath>



//Input: mydatabin.ver, mydatabin.ver.xfdl in folder rawPointData
//Output: generate files

//  - pointDomainInfo.xfdl --> including partition sizes 
//		+ first line: (coarse-graind partition sizes, ex: 4 x 4), 
//		+ second line: and number of init points for domain (not inluding 4 corners)
//	- initDomainPoints.ver --> init points for large domain (poin1, point2,...)

//  - pointPartInfoXX.xfdl --> partition info for each partition in domain 
//		+ first line: fine-grained partition sizes ex: (4 x 4), and
//		+ second line: 4 double numbers represent lowPoint and highPoint of current partition
//		+ third line: number of init points
//		+ fourth line: number of points of child partitions
//		+ fifth line: path to dataset --> initPointPartXX.ver & pointPartXX_YY.ver 

//	- initPointPartXX.ver --> init points for each partition in the domain (poin1, point2,...)

//  - pointPartXX_YY.ver --> points in child partition in parent partition XX, and child partition YY
//  - 
//The result data include two files: triangleIds.tri (contains all indices of points of fullPointPart.ver) and fullPointPart.ver
//fullPointPart.ver contains all points in the domain and gris points (include 4 corner points)
//g++ -std=gnu++11 common.cpp point.cpp distribute.cpp distributedMain.cpp -o distribute
//rm ../dataSources/10Kvertices/delaunayResults/*.*
//./distribute 1 3 4 4 4 4 "10Kvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000 2 20
//==============================================================================
distribute::distribute(double domainsize, unsigned int numberOfNodes, unsigned int xCoarsePartNumber, unsigned int yCoarsePartNumber, unsigned int xFinePartNumber, unsigned int yFinePartNumber, std::string dataNameStr, std::string verInfoFile, std::string verFile, unsigned long long chunksize, unsigned int initcoarsesize, unsigned int initfinesize){
	domainSize = domainsize;
	nodeNum = numberOfNodes;
	lowDomainPoint.setX(0);
	lowDomainPoint.setY(0);
	highDomainPoint.setX(domainSize);
	highDomainPoint.setY(domainSize);

	xCoarsePartNum = xCoarsePartNumber;
	yCoarsePartNum = yCoarsePartNumber;
	xFinePartNum = xFinePartNumber;
	yFinePartNum = yFinePartNumber;

	globalPointIndex = 0;

	dataName = dataNameStr;
	vertexInfofilename = "/data0/" + dataName + "/rawPointData/" + verInfoFile;
	vertexfilename = "/data0/" + dataName + "/rawPointData/" + verFile;
	chunkSize = chunksize;

	//initialize initPointArr for the first phase of delaunay
	initCoarseSize = initcoarsesize;
	initFineSize = initfinesize;

	//allocate memory for initPointSize
	unsigned int initPointSize = initCoarseSize + initFineSize;
	initPointArr = new std::list<point>*[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		initPointArr[coarsePartId] = new std::list<point>[xFinePartNum*yFinePartNum];

	//allocate memory for initPointInfoArr
	initPointInfoArr = new unsigned int*[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		initPointInfoArr[coarsePartId] = new unsigned int[xFinePartNum*yFinePartNum];
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
			initPointInfoArr[coarsePartId][finePartId] = initPointSize;
	}


	//allocate memory for partPointList
	partPointList = new std::list<point>*[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		partPointList[coarsePartId] = new std::list<point>[xFinePartNum*yFinePartNum];

	//allocate memory for pointPartInfoArr
	pointPartInfoArr = new unsigned long int*[xCoarsePartNum*yCoarsePartNum];
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		pointPartInfoArr[coarsePartId] = new unsigned long int[xFinePartNum*yFinePartNum];
	
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++) 
			pointPartInfoArr[coarsePartId][finePartId] = 0;

	gridPoints = NULL;

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

	std::string folderShare;
	//remove all file in all folderShare before create new files
	for(unsigned int nodeId=0; nodeId<nodeNum; nodeId++){
		folderShare = "/data" + toString(nodeId) + "/";
		std::string delCommand = "rm -f " + folderShare + dataName + "/*";
		system(delCommand.c_str());
	}

}
//==============================================================================
//return partition index in a domain
//input: lowPoint (low left point), highPoint (high right point), 
//		 number of partitions for two sides of domain (xPartNum, yPartNum), ex: 4x4 --> 16
//		 The point P.
//output: the number Id of the partition which the point P belong to
//This function works for any level of partitions (coarse-grained partitions or fine-grained partitions)
int distribute::partIndex(point p, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum){
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
void distribute::pointsDistribute(double *pointCoorArr, unsigned long long readingSize){

	double initSize = initCoarseSize + initFineSize;

	//Distribute points into partitions
//	#pragma omp parallel for
	for(unsigned long long index=0; index<readingSize; index++){
		//Each point is assigned to a global index which is the index of orginal point dataset (mydatabin.ver in rawPointData)
		point currPoint(pointCoorArr[index*vertexRecordSize], pointCoorArr[index*vertexRecordSize+1], globalPointIndex);

//		#pragma omp critical
		globalPointIndex++;


		unsigned int currCoarsePartIndex = partIndex(currPoint, lowDomainPoint, highDomainPoint, xCoarsePartNum, yCoarsePartNum);
		boundingBox currBox = findPart(currCoarsePartIndex, lowDomainPoint, highDomainPoint, xCoarsePartNum, yCoarsePartNum);
		unsigned int currFinePartIndex = partIndex(currPoint, currBox.getLowPoint(), currBox.getHighPoint(), xFinePartNum, yFinePartNum);

		//Distribute points into initPointArr
		if(initPointInfoArr[currCoarsePartIndex][currFinePartIndex]>0)
//		#pragma omp critical
		{//need contribute points for initPointArr
			initPointArr[currCoarsePartIndex][currFinePartIndex].push_back(currPoint);
			initPointInfoArr[currCoarsePartIndex][currFinePartIndex]--;
		}
		else
//		#pragma omp critical
		{//push current point to current partition
			pointPartInfoArr[currCoarsePartIndex][currFinePartIndex]++;//update pointPartInfo as new point coming
			partPointList[currCoarsePartIndex][currFinePartIndex].push_back(currPoint);
		}
	}
}


//==============================================================================
//Collect initCoarseSize points from each fine-grained partition, and contribute to initPoints.ver file
//The rest of init points in fine-grained partitions will be stored in initPointsXX.ver (XX range from 0 to number of coarse-grained partitions)
//ex: 4x4 coarse-grained partitions, each of them has 4x4 fine-grained partitions. Alltogether we have 4x4x4x4 = 256 fine-grained partitions
//and 16 coarse-grained partitions --> 16 files initPointPart00.ver, ... initPointPart15.ver
void distribute::processInitPoints(){

	//number of init points for domain
	unsigned int initDomainPointSize = xCoarsePartNum*yCoarsePartNum*xFinePartNum*yFinePartNum*initCoarseSize;
	//number of init points for all sub partition in a partition
	unsigned int initPartPointSize = xFinePartNum*yFinePartNum*initFineSize;
	point *initDomainPointArr = new point[initDomainPointSize];
	point *initPartPointArr = new point[initPartPointSize];

	std::string initPartFileStr;
	std::string folderShare;
	FILE *f;
	unsigned int index1 = 0;
	unsigned int index2;

	//Collect initCoarseSize points from each fine-grained partition
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		index2 = 0;
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
			if(initPointArr[coarsePartId][finePartId].size()<(initCoarseSize+initFineSize)){
				std::cout<<"The distribution stops because partition ["<<coarsePartId<<", "<<finePartId<<"] is empty!!!!\n";
				exit(1);
			}
			else{//take initCoarseSize points from each fine-grained partition
				for(unsigned int k=0; k<initCoarseSize; k++){
					initDomainPointArr[index1] = initPointArr[coarsePartId][finePartId].front();
					index1++;
					initPointArr[coarsePartId][finePartId].pop_front();
				}
				//Collect init points in fine-grained partitions that are belong to a coarse-grained partition
				for(unsigned int k=0; k<initFineSize; k++){	
					initPartPointArr[index2] = initPointArr[coarsePartId][finePartId].front();
					index2++;
					initPointArr[coarsePartId][finePartId].pop_front();
				}
				folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
				//write points leftover of each fine-grained partition to initPointPartXX.ver
				initPartFileStr = generateFileName(coarsePartId, folderShare + dataName + "/initPointPart", xCoarsePartNum*yCoarsePartNum, ".ver");
				f = fopen(initPartFileStr.c_str(), "wb");
				fwrite(initPartPointArr, initPartPointSize, sizeof(point), f);
				fclose(f);
			}
	}

	//write all init points to file initDomainPoints.ver
	std::string initDomainFileStr = "/data0/" + dataName + "/initDomainPoints.ver";
	f = fopen(initDomainFileStr.c_str(), "wb");
	fwrite(initDomainPointArr, initDomainPointSize, sizeof(point), f);
	fclose(f);


	delete [] initDomainPointArr;
	delete [] initPartPointArr;

	//delete initPointArr, initPointInfoArr
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		delete [] initPointArr[coarsePartId];
		delete [] initPointInfoArr[coarsePartId];
	}
	delete [] initPointArr;
	delete [] initPointInfoArr;
}

//==============================================================================
//Create pointPartInfo array to pointDomainInfo.xfdl and pointPartInfoXX.xfdl files
void distribute::createInfoFiles(){
	//store pointPartInfo array to pointDomainInfo.xfdl file
	//write to file pointPartInfo.txt
	//trunc means If the file is opened for output operations and it already existed, 
	//its previous content is deleted and replaced by the new one.
	std::ofstream pointPartInfoFile("/data0/" + dataName + "/pointDomainInfo.xfdl", std::ofstream::out | std::ofstream::trunc);
	//first line stores number of coarse partition sizes (ex: 4 x 4)
	pointPartInfoFile<<xCoarsePartNum<<" "<<yCoarsePartNum<<"\n";
	unsigned int initDomainPointSize = xCoarsePartNum*yCoarsePartNum*xFinePartNum*yFinePartNum*initCoarseSize;
	//second line stores number of init points for the domain
	pointPartInfoFile<<initDomainPointSize<<"\n";
	pointPartInfoFile.close();



	std::string fileStr;
	std::string	folderShare;
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
		fileStr = generateFileName(coarsePartId, folderShare + dataName + "/pointPartInfo", xCoarsePartNum*yCoarsePartNum, ".xfdl");
		//its previous content is deleted and replaced by the new one.
		pointPartInfoFile.open(fileStr,  std::ofstream::out | std::ofstream::trunc);

		//first line: fine-grained partition sizes ex: (4 x 4), and
		pointPartInfoFile<<xFinePartNum<<" "<<yFinePartNum<<"\n";

		//second line: 4 double numbers represent lowPoint and highPoint of current partition with coarsePArtId
		boundingBox bb = findPart(coarsePartId, lowDomainPoint, highDomainPoint, xCoarsePartNum, yCoarsePartNum);
		pointPartInfoFile<<bb.getLowPoint().getX()<<" "<<bb.getLowPoint().getY()<<" "<<bb.getHighPoint().getX()<<" "<<bb.getHighPoint().getY()<<"\n";

		//third line: number of init points for each partition in the domain
		//this number is similar to all partitions
		pointPartInfoFile<<xFinePartNum*yFinePartNum*initFineSize<<"\n";

		//fourth line: number of points of child partitions
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
			pointPartInfoFile<<pointPartInfoArr[coarsePartId][finePartId]<<" ";
		pointPartInfoFile<<"\n";

		//fifth line: path to dataset --> initPointPartXX.ver & pointPartXX_YY.ver
		//pointPartInfoFile<<destPath<<"\n";
 
		pointPartInfoFile.close();
	}
}

//==============================================================================
//read billion points from file, partition them into partitions, 
//for ex: a quare domain can be partitioned int to 4 column and 4 rows --> 16 partitions
//store each partition of points to file
void distribute::processDistribution(){
	FILE *fcoorData=fopen(vertexfilename.c_str(), "rb");
	if(!fcoorData) {std::cout<<"not exist "<<vertexfilename<<std::endl; exit(1);}

	unsigned long long readingPos = 0;
	unsigned long long readingSize = chunkSize;
	double *pointCoorArr = NULL;//x1 y1 x2 y2,...xn,yn

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
		pointsDistribute(pointCoorArr, readingSize);

		//Write all point coordinates in the same partition into its file
		for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
			for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
				if(!partPointList[coarsePartId][finePartId].empty()){
					writeListToFile(partPointList[coarsePartId][finePartId], coarsePartId, finePartId);
//					std::list<double>::iterator it;
					partPointList[coarsePartId][finePartId].clear();
				}
		readingPos = readingPos + chunkSize;
	}

    fclose(fcoorData);
	delete [] pointCoorArr;

	//store initPoints
	processInitPoints();

	//store pointPartInfo array to pointDomainInfo.xfdl and pointPartInfoXX.xfdl files
	createInfoFiles();
}

//==============================================================================
void distribute::writeListToFile(std::list<point> pointList, unsigned int coarsePartId, unsigned int finePartId){
	unsigned int size = pointList.size();
	point *pointArr = new point[size];
	unsigned int index = 0;
	std::list<point>::iterator it;
	for (it = pointList.begin(); it != pointList.end(); it++){
		pointArr[index] = (*it);
		index++;
	}
	std::string	folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
	writeToBinaryFile(folderShare + dataName + "/pointPart", coarsePartId, finePartId, pointArr, size);
	delete [] pointArr;
}

//==============================================================================
//append data with size to a binary file, for ex: with partition1 in 16 partitions, the file name would be
// "pointPartxx.ver" in --> "./dataSources/10vertices/pointPartitions"
void distribute::writeToBinaryFile(std::string fileStr, unsigned int coarsePartId, unsigned int finePartId, point *data, unsigned int size){
	fileStr = generateFileName(coarsePartId, fileStr, xCoarsePartNum*yCoarsePartNum, "");
	fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");

	FILE *f = fopen(fileStr.c_str(), "a");//appending 
	if(!f) {std::cout<<"not exist "<<fileStr<<std::endl; exit(1);}
	fwrite(data, size, sizeof(point), f);
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
//sort all points in all partition, then write them back to file: pointPartxxx.ver
void distribute::sortUpdate(){
	std::string folderShare;
	//scan all fine-grained partition files, sort them, then write back to that file (pointPartXX_XX.ver)
//	#pragma omp parallel for
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
			std::string fileStr = generateFileName(coarsePartId, folderShare + dataName + "/pointPart", xCoarsePartNum*yCoarsePartNum, "");
			fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");

			unsigned long int pointNum = pointPartInfoArr[coarsePartId][finePartId];
			if(pointNum==0){
				std::cout<<fileStr<<" does not exist\n";
				continue;
			}
			point *pointArr = new point[pointNum];
			FILE *f = fopen(fileStr.c_str(), "rb");
			if(!f){
				std::cout<<fileStr<<" does not exist\n"; continue;}
			else{
				fread(pointArr, pointNum, sizeof(point), f);
			}
			fclose(f);

			//sort based on coordinate x for array of points
			qsort(pointArr, pointNum, sizeof(point), int_compar);
std::cout<<"sort the content in file ("<<pointNum<<") "<<fileStr<<std::endl;		
			f = fopen(fileStr.c_str(), "wb");
			//write back to pointPartxxx.ver 
			fwrite(pointArr, pointNum, sizeof(point), f);
			fclose(f);
			//Release some temporary arrays
			delete [] pointArr;
		}	
}

//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(){

	//Open initDomainPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = "/data0/" + dataName + "/initDomainPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	//-4 means not including 4 corner points
	unsigned int gridPointsSize = (xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4;
std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";

	//grid points on the domain edges are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum*4) including 4 corners.
	gridPoints = new point[gridPointsSize];

	//generate other grid points
	unsigned int index = 0;
	double interval = domainSize/(xCoarsePartNum*xFinePartNum);
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
	fwrite(gridPoints, sizeof(point), (xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4, f);//-4 means not including 4 corner points
	fclose(f);

for(int i=0; i<xCoarsePartNum*xFinePartNum*4-4;i++)
std::cout<<gridPoints[i].getX()<<" "<<gridPoints[i].getY()<<std::endl;


	//write number of grid points to pointDomainInfo.xfdl
	//app means append to the end of file
	std::ofstream pointPartInfoFile("/data0/" + dataName + "/pointDomainInfo.xfdl", std::ofstream::out | std::ofstream::app);
	//third line stores number of grid points (artificial points all around domain to avoid sliver triangles)
	pointPartInfoFile<<(xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4<<"\n";
	//fourth line stores total number of points including grid points
	pointPartInfoFile<<globalPointIndex<<"\n";
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
	std::string cpCommand = "cp /data0/" + dataName + "/rawPointData/mydatabin.ver /data0/" + dataName + "/fullPointPart.ver";
	system(cpCommand.c_str());

	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	//coorArr contains all coordinates of grid points including 4 corner point
	//coorArr does not contain Ids
	double *coorArr = new double[(xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2*vertexRecordSize];
	//copy coordinates from gridPoints to coorArr
	//note that gridPoints contains all grid points except 4 corner points
	unsigned int index = 0;
	for(index=0; index<xCoarsePartNum*xFinePartNum*4-4; index++){
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
	std::string fileStr = "/data0/" + dataName + "/fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<fileStr<<"\n"; return;}
	fwrite(coorArr, sizeof(double), xCoarsePartNum*xFinePartNum*4*vertexRecordSize, f);
	fclose(f);
	delete [] coorArr;
}



//==============================================================================
void distribute::printPointPart(unsigned int coarsePartId, unsigned int finePartId){

	//form a file name from coarsePartId = 5 of 4x4, finePartId = 4 of 4x4--> pointPart05_04.ver)
	std::string fileStr;
	std::string folderShare = "/data" + toString(coarsePartId % nodeNum) + "/";
	fileStr = generateFileName(coarsePartId, folderShare + dataName + "/pointPart", xCoarsePartNum*yCoarsePartNum, "");
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
distribute::~distribute(){

	delete [] gridPoints;

	//delete pointPartArr, pointPartInfoArr
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		delete [] pointPartInfoArr[coarsePartId];
		delete [] partPointList[coarsePartId];
	}
	delete [] pointPartInfoArr;
	delete [] partPointList;

}
