#include "uniformDistribute.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

//g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
//rm -rf ../dataSources/mapData/delaunayResults/
//mkdir ../dataSources/mapData/delaunayResults
//./distribute 4 4 4 4 1 "../dataSources/mapData/" "nc_inundation_v6c.grd" "../dataSources/mapData/delaunayResults/" 5 20
//==============================================================================
distribute::distribute(unsigned int xCoarsePartNumber, unsigned int yCoarsePartNumber, unsigned int xFinePartNumber, unsigned int yFinePartNumber, unsigned long long totalNumberPoints, unsigned int initialCoarsePointNum, unsigned int initialFinePointNum, std::string fullPath){
	xCoarsePartNum = xCoarsePartNumber;
	yCoarsePartNum = yCoarsePartNumber;
	xFinePartNum = xFinePartNumber;
	yFinePartNum = yFinePartNumber;

	domainLowPoint.setX(0);
	domainLowPoint.setY(0);
	domainHighPoint.setX(xCoarsePartNum*xFinePartNum);
	domainHighPoint.setY(yCoarsePartNum*yFinePartNum);

	initCoarsePointNum = initialCoarsePointNum;
	initFinePointNum = initialFinePointNum;

	std::cout<<"=====================================================================================\n";
	std::cout<<"xCoarsePartNumber: "<<xCoarsePartNum<<", yCoarsePartNumber: "<<yCoarsePartNum<<", xFinePartNumber: "<<xFinePartNum<<", yFinePartNumber: "<<yFinePartNum<<std::endl;
	std::cout<<"======================================================================================\n";

	globalPointIndex = 0;
	path = fullPath;

	totalInputPointNum = totalNumberPoints; 
	//number of point in the first fine partition xCoarsePartNum=0, yCoarsePartNum=0, xFinePartNum=0, yFinePartNum=0
	basicPointPartNum = totalNumberPoints/(xCoarsePartNumber*yCoarsePartNumber*xFinePartNumber*yFinePartNumber);

	domainSize = xCoarsePartNumber*xFinePartNumber;

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
void distribute::generateBasicData(unsigned int &basicPointPartNum){

	//generate random points in (0,0) - (1,1)
	std::string command = "./bin/rbox " + toString(basicPointPartNum) + " n D2 O0.5 > " + path + "mydata.ver";
	system(command.c_str());

	//get random points between (0,0) - (1,1) (not equal 0 or 1)
	readCoorPart(basicPointPartCoorArr, basicPointPartNum);

	if(basicPointPartNum<initCoarsePointNum+initFinePointNum){
		std::cout<<"!!!!!Nnumber of points i a partition has to be greater than number of init point!!!!\n";
		exit(1);
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
void distribute::generateAllPartitionPoints(){
	double lowPartPointX, lowPartPointY, highPartPointX, highPartPointY;
	boundingBox currCoarseBox, currFineBox;
	std::string fileStr;
	//generate basic points in the range (0,0) - (1,1) to array basicPointCoorArr (partition 0, 0)
	generateBasicData(basicPointPartNum);

	double *pointPartCoorArr = new double[basicPointPartNum*2];

	unsigned int initPointDomainNum = xCoarsePartNum*yCoarsePartNum*xFinePartNum*yFinePartNum*initCoarsePointNum;
	point *initPointDomainArr = new point[initPointDomainNum];
	unsigned int initPointDomainIndex = 0;

	unsigned int initPointPartNum = xFinePartNum*yFinePartNum*initFinePointNum;
	point *initPointPartArr = new point[initPointPartNum];

	//pointPartNum is the number of point in fine partition after collecting all init points out 
	pointPartNum = basicPointPartNum - (initCoarsePointNum + initFinePointNum);
	point *pointFinePartArr = new point[pointPartNum*2];

	//copy the basic points (basicPointCoorArr) to all other partitions
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		unsigned int initPointPartIndex = 0;
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			currFineBox = findPart(finePartId, currCoarseBox.getLowPoint(), currCoarseBox.getHighPoint(), xFinePartNum, yFinePartNum);
			lowPartPointX = currFineBox.getLowPoint().getX();
			lowPartPointY = currFineBox.getLowPoint().getY();
//			highPartPointX = currFineBox.getHighPoint().getX();
//			highPartPointY = currFineBox.getHighPoint().getY();

			//copy and shift point coordinates from basicPointPartCoorArr to current partition
			for(unsigned int pointId=0; pointId<basicPointPartNum; pointId++){
				pointPartCoorArr[pointId*2] = basicPointPartCoorArr[pointId*2] + lowPartPointX;
				pointPartCoorArr[pointId*2+1] = basicPointPartCoorArr[pointId*2+1] + lowPartPointY;
			}
			//append pointPartCoorArr to file fullPointPart.ver
			writePointCoorArr(pointPartCoorArr, basicPointPartNum, path + "fullPointPart.ver");


			//collects init domain points
			for(unsigned int pointId=0; pointId<initCoarsePointNum; pointId++){
				initPointDomainArr[initPointDomainIndex].setX(pointPartCoorArr[pointId*2]);
				initPointDomainArr[initPointDomainIndex].setY(pointPartCoorArr[pointId*2+1]);
				initPointDomainArr[initPointDomainIndex].setId(globalPointIndex);
				initPointDomainIndex++;
				globalPointIndex++;
			}

			//collects init coarse partition points for each coarse partition
			unsigned int initPointNum = initCoarsePointNum+initFinePointNum;
			for(unsigned int pointId=initCoarsePointNum; pointId<initPointNum; pointId++){
				initPointPartArr[initPointPartIndex].setX(pointPartCoorArr[pointId*2]);
				initPointPartArr[initPointPartIndex].setY(pointPartCoorArr[pointId*2+1]);
				initPointPartArr[initPointPartIndex].setId(globalPointIndex);
				initPointPartIndex++;
				globalPointIndex++;
			}

			//The points leftover are used for fine partition
			unsigned int pointPartIndex = 0;
			for(unsigned int pointId=initPointNum; pointId<basicPointPartNum; pointId++){
				pointFinePartArr[pointPartIndex].setX(pointPartCoorArr[pointId*2]);
				pointFinePartArr[pointPartIndex].setY(pointPartCoorArr[pointId*2+1]);
				pointFinePartArr[pointPartIndex].setId(globalPointIndex);
				pointPartIndex++;
				globalPointIndex++;
			}
			//write pointFinePartArr to pointPartXX_XX.tri
			qsort(pointFinePartArr, pointPartNum, sizeof(point), int_compar);
			fileStr = generateFileName(coarsePartId, path + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
			fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");
			std::cout<<"write ("<<pointPartNum<<") points to file "<<fileStr<<"\n";
			writePointPartArr(pointFinePartArr, pointPartNum, fileStr);

			
		}
		//wrrite init point partition to file initPointPartXX.tri
		fileStr = generateFileName(coarsePartId, path + "initPointPart", xCoarsePartNum*yCoarsePartNum, ".ver");
		writePointPartArr(initPointPartArr, initPointPartNum, fileStr);
	}

	//store initPointDomainArr to initDomainPoints.ver
	writePointPartArr(initPointDomainArr, initPointDomainNum, path + "initDomainPoints.ver");

	std::ofstream pointPartInfoFile(path + "pointDomainInfo.xfdl", std::ofstream::out);
	//first line stores number of coarse partition sizes (ex: 4 x 4)
	pointPartInfoFile<<xCoarsePartNum<<" "<<yCoarsePartNum<<"\n";
	//second line stores number of init points for the domain
	pointPartInfoFile<<initPointDomainNum<<"\n";
	pointPartInfoFile.close();


	//store pointPartInfo array to pointPartInfoXX.xfdl file
	//trunc means If the file is opened for output operations and it already existed, 
	//its previous content is deleted and replaced by the new one.	
	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		std::string fileStr = generateFileName(coarsePartId, path + "pointPartInfo", xCoarsePartNum*yCoarsePartNum, ".xfdl");
		std::ofstream pointPartInfoFile(fileStr, std::ofstream::out);
		//first line store partition numbers
		pointPartInfoFile<<xFinePartNum<<" "<<yFinePartNum<<"\n";
		currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		//second line stores the coordiantes of lowPoint and highPoint of a coarse partition
		pointPartInfoFile<<currCoarseBox.getLowPoint().getX()<<" "<<currCoarseBox.getLowPoint().getY()<<" "<<currCoarseBox.getHighPoint().getX()<<" "<<currCoarseBox.getHighPoint().getY()<<"\n";
		//third line stores total number init points in a coarse partition
		pointPartInfoFile<<initPointPartNum<<"\n";
		//fourth line stores number of points of fine partitions in a coarse partition
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++)
			pointPartInfoFile<<pointPartNum<<" ";
		pointPartInfoFile<<"\n";
		pointPartInfoFile.close();
	}

	delete [] pointFinePartArr;
	delete [] initPointPartArr;
	delete [] initPointDomainArr;
}

//==============================================================================
//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
void distribute::addGridPointsToInitPoints(){

	//Open initDomainPoints.ver, append grid points to the end of initPoints.ver
	std::string initFileStr = path + "initDomainPoints.ver";
	FILE *f = fopen(initFileStr.c_str(), "a");
	if(!f)	{std::cout<<" can not open "<<initFileStr<<"\n"; return;}

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	//-4 means not including 4 corner points
	gridPointsSize = (xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4;
std::cout<<"gridPointsSize = "<<gridPointsSize<<"\n";

	//grid points on the domain edges are points on 4 edges AB, BC, CD, DA of square domain.
	//total number of grid points is (xPartNum*4) including 4 corners.
	gridPoints = new point[gridPointsSize];

	//generate other grid points
	unsigned int index = 0;
	double currentCoor = 1.0;

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
		
		currentCoor += 1.0;
		index +=4;
	}
	fwrite(gridPoints, sizeof(point), (xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4, f);//-4 means not including 4 corner points
	fclose(f);

std::cout<<"list of grid points around the domain: \n";
for(int i=0; i<xCoarsePartNum*xFinePartNum*4-4;i++)
std::cout<<" ["<<gridPoints[i].getX()<<", "<<gridPoints[i].getY()<<"]";


	//write number of grid points to pointDomainInfo.xfdl
	//app means append to the end of file
	std::ofstream pointPartInfoFile(path + "pointDomainInfo.xfdl", std::ofstream::out | std::ofstream::app);
	//third line stores number of grid points (artificial points all around domain to avoid sliver triangles)
	pointPartInfoFile<<(xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*2-4<<"\n";
	//fourth line stores total number of points including grid points (not inlcuding 4 corner points)
	pointPartInfoFile<<globalPointIndex<<"\n";
	//fifth line stores totalPointPartNum (number of point in each coarse partition = total all pointPartNum, not including initPoint and gridPoints)
	unsigned long totalPointPartNum = pointPartNum*xFinePartNum*yFinePartNum;

	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++)
		pointPartInfoFile<<totalPointPartNum<<" ";

	pointPartInfoFile.close();

}
//==============================================================================
//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
void distribute::createFullPointCoorData(){
/*	std::cout<<"\nCreate full point partitions and grid points ...\n";
	//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
	std::string cpCommand = "cp " + srcPath + "mydatabin.ver " + destPath + "fullPointPart.ver";
	system(cpCommand.c_str());
*/
	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	//coorArr contains all coordinates of grid points including 4 corner point
	//coorArr does not contain Ids
	double *coorArr = new double[(xCoarsePartNum*xFinePartNum+yCoarsePartNum*yFinePartNum)*4];
	//copy coordinates from gridPoints to coorArr
	//note that gridPoints contains all grid points except 4 corner points
	unsigned int index = 0;
	for(index=0; index<xCoarsePartNum*xFinePartNum*4-4; index++){
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
	fwrite(coorArr, sizeof(double), xCoarsePartNum*xFinePartNum*8, f);
	fclose(f);
	delete [] coorArr;
}

//==============================================================================
void distribute::printPointPart(unsigned int coarsePartId, unsigned int finePartId){

	//form a file name from coarsePartId = 5 of 4x4, finePartId = 4 of 4x4--> pointPart05_04.ver)
	std::string fileStr;
	fileStr = generateFileName(coarsePartId, path + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
	fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");

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
	pointArr = new point[pointPartNum];

	for(unsigned int coarsePartId=0; coarsePartId<xCoarsePartNum*yCoarsePartNum; coarsePartId++){
		boundingBox currCoarseBox = findPart(coarsePartId, domainLowPoint, domainHighPoint, xCoarsePartNum, yCoarsePartNum);
		for(unsigned int finePartId=0; finePartId<xFinePartNum*yFinePartNum; finePartId++){
			boundingBox currFineBox = findPart(finePartId, currCoarseBox.getLowPoint(), currCoarseBox.getHighPoint(), xFinePartNum, yFinePartNum);
			lowPartPointX = currFineBox.getLowPoint().getX();
			lowPartPointY = currFineBox.getLowPoint().getY();
			highPartPointX = currFineBox.getHighPoint().getX();
			highPartPointY = currFineBox.getHighPoint().getY();
			bool stop=false;

			fileStr = generateFileName(coarsePartId, path + "pointPart", xCoarsePartNum*yCoarsePartNum, "");
			fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum, ".ver");
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
				std::cout<<"The data in coarsePartId = "<<coarsePartId<<", finePartId = "<<finePartId<<" is wrong!!!!!!!\n";
				std::cout<<"lowPartPointX: "<<lowPartPointX<<", lowPartPointY: "<<lowPartPointX<<", highPartPointX: "<<highPartPointX<<", highPartPointY: "<<highPartPointY<<"\n";
				//printPointPart(coarsePartId,finePartId);
				continue;
			}
		}
	}
	delete [] pointArr;
}

//==============================================================================
void distribute::info(){

	std::cout<<"number of points in a fine partition: "<<basicPointPartNum<<"\n";
	std::cout<<"total number of points on input: "<<totalInputPointNum<<"\n";
	std::cout<<"total number of points in the fullPointPart.tri: "<<basicPointPartNum*(xCoarsePartNum*yCoarsePartNum*xFinePartNum*yFinePartNum)+gridPointsSize+4<<"\n";

}
//==============================================================================
distribute::~distribute(){
	//delete pointPartArr, pointPartInfoArr
	delete [] gridPoints;
	delete [] basicPointPartCoorArr;
}
