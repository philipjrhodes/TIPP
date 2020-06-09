#include <iostream>
#include <string>
#include "common.h"
#include "drawMesh.h"
#include <fstream>
#include <cmath>

//g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp common.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph 
//edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp

//./drawDomain ../dataSources/10Kvertices/ 1


//==================================================================================
double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3){
	return fabs((x1-x3)*(y2-y1) - (x1-x2)*(y3-y1))/2;
}

//==============================================================================
bool insideBoundingBox(point p, boundingBox b){
	double xVal = p.getX();
	double yVal = p.getY();
	point lowPoint = b.getLowPoint();
	point highPoint = b.getHighPoint();
	double xLow = lowPoint.getX();
	double yLow = lowPoint.getY();
	double xHigh = highPoint.getX();
	double yHigh = highPoint.getY();

	if((xVal>xLow)&&(xVal<xHigh)&&(yVal>yLow)&&(yVal<yHigh)) return true;
	else return false;
}

//==============================================================================
//given an index, find a bounding box of a partition in a domain
//input: + partIndex --> partition index of a partition in the domain 
//		 + lowPoint, highPoint --> the domain points
//		 + xPartNum, yPartNum --> granularity of partitions in the domain
//output: the bounding box of cuurent partition
boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum){
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

//================================================================
void readTriangles(triangle *&triangleArr, unsigned int &triangleNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(sizeof(triangle)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	triangleArr = new triangle[triangleNum];

	fread(triangleArr, triangleNum, sizeof(triangle), f);
	fclose(f);
}

//================================================================
void readTriangleCoors(double *&triangleCoorArr, unsigned int &triangleNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(6*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	triangleCoorArr = new double[triangleNum*6];

	fread(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);
}

//================================================================
void readAllCoors(double *&pointCoorArr, unsigned int &pointNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	pointNum = ftell(f)/(2*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	pointCoorArr = new double[pointNum*2];

	fread(pointCoorArr, pointNum*2, sizeof(double), f);
	fclose(f);
}

//================================================================
void readTriangleIds(unsigned long long *&triangleIdArr, unsigned int &triangleNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(3*sizeof(unsigned long long)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	triangleIdArr = new unsigned long long[triangleNum*3];

	fread(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
}

//================================================================
void readCoorsFromTriangleIds(unsigned long long *triangleIdArr, unsigned int &triangleNum, double *pointCoorArr, double *triangleCoorArr){
	//fill triangleCoorArr from triangleIdArr and pointCoorArr
	for(int triangleId=0; triangleId<triangleNum; triangleId++){
		triangleCoorArr[triangleId*6] = pointCoorArr[triangleIdArr[triangleId*3]*2];
		triangleCoorArr[triangleId*6+1] = pointCoorArr[triangleIdArr[triangleId*3]*2+1];
		triangleCoorArr[triangleId*6+2] = pointCoorArr[triangleIdArr[triangleId*3+1]*2];
		triangleCoorArr[triangleId*6+3] = pointCoorArr[triangleIdArr[triangleId*3+1]*2+1];
		triangleCoorArr[triangleId*6+4] = pointCoorArr[triangleIdArr[triangleId*3+2]*2];
		triangleCoorArr[triangleId*6+5] = pointCoorArr[triangleIdArr[triangleId*3+2]*2+1];
	}
}

//================================================================
void readCoorsFromTriangleArr(triangle *triangleArr, unsigned int &triangleNum, double *triangleCoorArr){
	//fill triangleCoorArr from triangleIdArr and pointCoorArr
	for(unsigned int triangleId=0; triangleId<triangleNum; triangleId++){
		triangleCoorArr[triangleId*6] = triangleArr[triangleId].p1.getX();
		triangleCoorArr[triangleId*6+1] = triangleArr[triangleId].p1.getY();
		triangleCoorArr[triangleId*6+2] = triangleArr[triangleId].p2.getX();
		triangleCoorArr[triangleId*6+3] = triangleArr[triangleId].p2.getY();
		triangleCoorArr[triangleId*6+4] = triangleArr[triangleId].p3.getX();
		triangleCoorArr[triangleId*6+5] = triangleArr[triangleId].p3.getY();
	}
}


//===================================================================
//Based on all triangles ids in triangleIds.tri, draw all triangles (include triangles with grid points)
void drawTriangleArr(std::string path){
	//read fullPointPart.ver to pointCoorArr
	std::string fileStr = path + "delaunayResults/fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int pointNum = ftell(f)/(2*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	double *pointCoorArr = new double[pointNum*2];

	fread(pointCoorArr, pointNum*2, sizeof(double), f);
	fclose(f);

	//read triangle Ids (edges)
	std::string fileStr1 = path + "delaunayResults/triangleIds.tri";
	//std::string fileStr1 = path + "delaunayResults/initTriangles.tri";
	//std::string fileStr1 = path + "delaunayResults/returnAllStoreTriangleIds10.tri";
	f = fopen(fileStr1.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(3*sizeof(unsigned long int)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	unsigned long int *pointIdArr = new unsigned long int[triangleNum*3];
	fread(pointIdArr, triangleNum*3, sizeof(unsigned long int), f);
	fclose(f);


//for(int i=0; i<triangleNum; i++)
//	std::cout<<pointIdArr[i*3]<<" "<<pointIdArr[i*3+1]<<" "<<pointIdArr[i*3+2]<<"\n";

	double triangleAreas = 0;

	//build storeTriangleArr
	triangle *triangleArr = new triangle[triangleNum];
std::cout<<"pointNum: "<<pointNum<<", and number of triangles: "<<triangleNum<<"\n";
	for(int i=0; i<triangleNum; i++){
		unsigned int pointId1 = pointIdArr[i*3];
		unsigned int pointId2 = pointIdArr[i*3+1];
		unsigned int pointId3 = pointIdArr[i*3+2];

		triangleArr[i].p1.setId(pointId1);
		triangleArr[i].p2.setId(pointId2);
		triangleArr[i].p3.setId(pointId3);

		triangleArr[i].p1.setX(pointCoorArr[pointId1*2]);
		triangleArr[i].p1.setY(pointCoorArr[pointId1*2+1]);

		triangleArr[i].p2.setX(pointCoorArr[pointId2*2]);
		triangleArr[i].p2.setY(pointCoorArr[pointId2*2+1]);

		triangleArr[i].p3.setX(pointCoorArr[pointId3*2]);
		triangleArr[i].p3.setY(pointCoorArr[pointId3*2+1]);
//std::cout<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<"\n";
		triangleAreas += triangleArea(triangleArr[i].p1.getX(), triangleArr[i].p1.getY(), triangleArr[i].p2.getX(), triangleArr[i].p2.getY(), triangleArr[i].p3.getX(), triangleArr[i].p3.getY());
	}
	std::cout<<"triangleAreas: "<<triangleAreas<<std::endl;

	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);
	d->drawTriangleArr(triangleArr, triangleNum, 2);//GREEN
	delete d;

	delete [] pointCoorArr;
	delete [] pointIdArr;
	delete [] triangleArr;

/*
	std::string fileStr2 = path + "delaunayResults/boundaryCoors.tri";
	triangleNum = 0;
	double *triangleCoorArr;
	readTriangleCoors(triangleCoorArr, triangleNum, fileStr2);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

	delete d;

	delete [] triangleCoorArr;
*/
}

//================================================================
void drawDomainTriangles(std::string path, int domainSize){
	std::string fileStr = path + "delaunayResults/initDomainTriangles.tri";

	unsigned int triangleNum;
	double *triangleCoorArr;
	readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	delete d;

	delete [] triangleCoorArr;
}

//================================================================
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawDomainActivePartitions(std::string path, unsigned *coarsePartIdArr, unsigned coarsePartNum){

	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);

	unsigned int triangleNum;
	double *triangleCoorArr;
	std::string fileStr;
	for(int i=0; i<coarsePartNum; i++){
		fileStr = generateFileName(coarsePartIdArr[i], path + "delaunayResults/activeCoarseParts", xCoarsePartNum*yCoarsePartNum, ".tri");
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleCoorArr;	
	}
	delete d;

/*	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "delaunayResults/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());

	unsigned int *activeCoarsePartIdArr = new unsigned int[activeCoarsePartNum];
	//second line: read active partition ids (coarsePartition Ids)
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}

	unsigned int *activeCoarsePartSizeArr = new unsigned int[activeCoarsePartNum];
	//third line stores number of init triangles belong to active partitions (coarsePartitions)
	//take number of init triangles of current coarse partition in the array of active coarse partitions
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartSizeArr[i] = atoi(strItem.c_str());
	}
	initTriangleInfoFile.close();

	FILE *f;
	unsigned int coarsePartId;
	unsigned int triangleNum;
	std::string fileStr;
	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);
	//read coordinates and point Ids
	for(int i=0; i<activeCoarsePartNum; i++){
		coarsePartId = activeCoarsePartIdArr[i];
		triangleNum = activeCoarsePartSizeArr[i];

		fileStr = generateFileName(i, path + "delaunayResults/tempCoorCoarseParts", activeCoarsePartNum, ".tri");
		FILE *f = fopen(fileStr.c_str(), "rb");
		if(!f){
			std::cout<<"not exist "<<fileStr<<std::endl;
			return;
		}
		double *triangleCoorArr = new double[triangleNum*6];
		fread(triangleCoorArr, sizeof(double), triangleNum*6, f);
		fclose(f);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

		delete [] triangleCoorArr;
	}

	delete d;
	delete [] activeCoarsePartIdArr;
	delete [] activeCoarsePartSizeArr;
*/
}
//================================================================
//Draw init triangles of all active coarse partitions
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
void drawInitTrianglesOneCoarsePart(int coarsePartId, std::string path){

	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "delaunayResults/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile.close();


	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	drawMesh *d = new drawMesh;
	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);

	std::string fileStr;
	if(coarsePartId==0) fileStr = generateFileName(0, path + "delaunayResults/initTrianglesCoors", activeCoarsePartNum, ".tri");
	if(coarsePartId==2) fileStr = generateFileName(1, path + "delaunayResults/initTrianglesCoors", activeCoarsePartNum, ".tri");
	if(coarsePartId==8) fileStr = generateFileName(2, path + "delaunayResults/initTrianglesCoors", activeCoarsePartNum, ".tri");
	if(coarsePartId==10) fileStr = generateFileName(3, path + "delaunayResults/initTrianglesCoors", activeCoarsePartNum, ".tri");

	unsigned int triangleNum;
	double *triangleCoorArr;
	readTriangleCoors(triangleCoorArr, triangleNum, fileStr);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

	delete [] triangleCoorArr;
	delete d;
}

//================================================================
//Draw init triangles of all active coarse partitions
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawInitTrianglesAllCoarseParts(std::string path){
	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "delaunayResults/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile.close();

	drawMesh *d = new drawMesh;
	d->drawGridLines(point(0, 0), point(1.0, 1.0), 4, 4, 4);
	d->drawGridLines(point(0.5, 0.5), point(0.75, 0.75), 4, 4, 4);

	std::string fileStr;
	for(int i=0; i<activeCoarsePartNum; i++){
		fileStr = generateFileName(i, path + "delaunayResults/initTrianglesCoors", activeCoarsePartNum, ".tri");

		unsigned int triangleNum;
		double *triangleCoorArr;
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleCoorArr;
	}
	delete d;
}

//================================================================
//Draw active fine partitions
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawAllActiveFineParts(std::string path, unsigned *coarsePartIdArr, unsigned coarsePartNum, unsigned *finePartIdArr, unsigned finePartNum){
	drawMesh *d = new drawMesh;

	unsigned xCoarsePartNum = 4;
	unsigned yCoarsePartNum = 4;
	unsigned xFinePartNum = 4;
	unsigned yFinePartNum = 4;

	//draw grid lines for all active partitions
	for(unsigned i=0; i<coarsePartNum; i++){
		boundingBox bb = findPart(coarsePartIdArr[i], point(0.0, 0.0), point(1.0, 1.0), 4, 4);
		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), 4, 4, 4);
	}


	unsigned int triangleNum;
	double *triangleCoorArr;
	std::string fileStr;
	for(int i=0; i<coarsePartNum; i++){
		for(int j=0; j<finePartNum; j++){
			fileStr = generateFileName(coarsePartIdArr[i], path + "delaunayResults/subActiveCoorFineParts", xCoarsePartNum*yCoarsePartNum, "_");
			fileStr = generateFileName(finePartIdArr[j], fileStr, xFinePartNum*yFinePartNum, ".tri");
			std::cout<<fileStr<<"\n";
			readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

			d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
			delete [] triangleCoorArr;	
		}
	}
	delete d;
}

//================================================================
//Draw one active fine partitions for a coarPartId
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
void drawOneActiveFineParts(std::string path, int coarsePartId, unsigned *finePartIdArr, unsigned finePartNum){
	drawMesh *d = new drawMesh;

	unsigned xCoarsePartNum = 4;
	unsigned yCoarsePartNum = 4;
	unsigned xFinePartNum = 4;
	unsigned yFinePartNum = 4;

	//draw grid lines for all active partitions
	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), 4, 4);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), 4, 4, 4);

	unsigned int triangleNum;
	double *triangleCoorArr;
	std::string fileStr;

	for(int i=0; i<finePartNum; i++){
		fileStr = generateFileName(coarsePartId, path + "delaunayResults/subActiveCoorFineParts", xCoarsePartNum*yCoarsePartNum, "_");
		fileStr = generateFileName(finePartIdArr[i], fileStr, xFinePartNum*yFinePartNum, ".tri");
		std::cout<<fileStr<<"\n";
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
		triangleNum = 0;
		delete [] triangleCoorArr;
	}

	delete d;
}

//================================================================
//Draw returnStoredTriangles and returnTriangles
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
void drawSubBoundaryInteriorTriangles(std::string path, int coarsePartId, unsigned *finePartIdArr, unsigned finePartNum){
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	unsigned xFinePartNum = 4;
	unsigned yFinePartNum = 4;

	drawMesh *d = new drawMesh;

	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);
	//d->drawGridLines(point(0.5, 0.5), point(0.75, 0.75), 4, 4, 4);

	unsigned int triangleNum1, triangleNum2;
	double *triangleCoorArr1, *triangleCoorArr2;
	std::string fileStr1, fileStr2;

	for(int i=0; i<finePartNum; i++){
		fileStr1 = generateFileName(coarsePartId, path + "delaunayResults/subBoundary", xCoarsePartNum*yCoarsePartNum, "_");
		fileStr1 = generateFileName(finePartIdArr[i], fileStr1, xFinePartNum*yFinePartNum, ".tri");
		readTriangleCoors(triangleCoorArr1, triangleNum1, fileStr1);
		d->drawCoordinateTriangles(triangleCoorArr1, triangleNum1, 2);//GREEN
		triangleNum1 = 0;
		delete [] triangleCoorArr1;

		fileStr2 = generateFileName(coarsePartId, path + "delaunayResults/subInterior", xCoarsePartNum*yCoarsePartNum, "_");
		fileStr2 = generateFileName(finePartIdArr[i], fileStr2, xFinePartNum*yFinePartNum, ".tri");
		readTriangleCoors(triangleCoorArr2, triangleNum2, fileStr2);
		d->drawCoordinateTriangles(triangleCoorArr2, triangleNum2, 3);//CYAN
		triangleNum2 = 0;
		delete [] triangleCoorArr2;
	}

	delete d;
}

//================================================================
//Draw interior triangles (returnAllStoreTriangleIdsXX.tri) and boundary triangles (boundaryTrianglesXX.tri) after the first stage of domain triangulation
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp (for coarsePartId = 10 only)
void drawOneInteriorBoundary(std::string path, unsigned int coarsePartId){
	drawMesh *d = new drawMesh;

	//draw interior triangles of a coarse partition
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	unsigned int xFinePartNum = 4;
	unsigned int yFinePartNum = 4;

	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xFinePartNum, xFinePartNum, 4);

	unsigned int triangleNum1, triangleNum2;
	double *triangleCoorArr1, *triangleCoorArr2;
	std::string fileStr1, fileStr2;
	
	fileStr1 = generateFileName(coarsePartId, path + "delaunayResults/boundary", xCoarsePartNum*yCoarsePartNum, ".tri");
	readTriangleCoors(triangleCoorArr1, triangleNum1, fileStr1);
	d->drawCoordinateTriangles(triangleCoorArr1, triangleNum1, 2);//GREEN
	delete [] triangleCoorArr1;




	//read fullPointPart.ver to pointCoorArr
	double *pointCoorArr;
	unsigned int pointNum;
	readAllCoors(pointCoorArr, pointNum, path + "delaunayResults/fullPointPart.ver");

	unsigned int triangleNum;
	unsigned long long *triangleIdArr;
	fileStr2 = generateFileName(coarsePartId, path + "delaunayResults/interiorIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	readTriangleIds(triangleIdArr, triangleNum, fileStr2);

	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

	//write interior triangleCoorArr to file interiorXX.tri
	fileStr2 = generateFileName(coarsePartId, path + "delaunayResults/interior", xCoarsePartNum*yCoarsePartNum, ".tri");
	storeTriangleCoors(triangleCoorArr, triangleNum, fileStr2, false);

	delete [] triangleCoorArr;
	delete [] triangleIdArr;
	delete [] pointCoorArr;

	delete d;
}

//================================================================
//Draw all interior and boundary triangles of all active coarse partitions
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawAllInteriorBoundary(std::string path){
	unsigned int activePartArr[] = {0, 2, 8, 10};

	drawMesh *d = new drawMesh;

	//draw interior triangles of a coarse partition
	unsigned int triangleNum;
	unsigned long long *triangleIdArr;
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	std::string fileStr;

	d->drawGridLines(point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum, 4);

	for(int i=0; i<4; i++){
		unsigned int coarsePartId = activePartArr[i];
		boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);



		//comment out this part if you do not want to draw the interior
		fileStr = generateFileName(coarsePartId, path + "delaunayResults/returnAllStoreTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		readTriangleIds(triangleIdArr, triangleNum, fileStr);

		//read fullPointPart.ver to pointCoorArr
		double *pointCoorArr;
		unsigned int pointNum;
		readAllCoors(pointCoorArr, pointNum, path + "delaunayResults/fullPointPart.ver");
		double *triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

		delete [] triangleCoorArr;
		delete [] triangleIdArr;
		delete [] pointCoorArr;




		//draw boundary triangles of a coarse partition
		fileStr = generateFileName(coarsePartId, path + "delaunayResults/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
		triangle *triangleArr;
		readTriangles(triangleArr, triangleNum, fileStr);
//		double *triangleCoorArr = new double[triangleNum*6];
		triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

		delete [] triangleCoorArr;
		delete [] triangleArr;
	}

	delete d;
}

//================================================================
//Draw inactive and the returned triangles for the next stage of domain
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawNewDomainTrianglesInNextStage(std::string path){
	triangle *triangleArr;
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	unsigned int triangleNum;

	//draw inactive triangles
	std::string fileStr = path + "delaunayResults/inActiveTrangle.tri";
	readTriangles(triangleArr, triangleNum, fileStr);
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

	drawMesh *d = new drawMesh;
	d->drawGridLines(point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum, 4);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	delete [] triangleCoorArr;
	delete [] triangleArr;



	//you can comment out this part if you do not need to draw the boundary triangles return from coarse partitions
	//draw returned triangles (bounadry triangles)
	unsigned int activePartArr[] = {0, 2, 8, 10};
	for(int i=0; i<4; i++){
		unsigned int coarsePartId = activePartArr[i];
		boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
//		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);

		//draw boundary triangles of a coarse partition
		fileStr = generateFileName(coarsePartId, path + "delaunayResults/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
		triangle *triangleArr;
		readTriangles(triangleArr, triangleNum, fileStr);
		double *triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

		delete [] triangleCoorArr;
		delete [] triangleArr;
	}

	delete d;
}

//================================================================
void testBoundaryData(std::string path){
	std::string fileStr = path + "delaunayResults/returnAllTriangleCoors10.tri";

	unsigned int triangleNum;
	double *triangleCoorArr;
	readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	std::cout<<"number of triangles: "<<triangleNum<<"\n";
	delete d;

	delete [] triangleCoorArr;
}

//================================================================
void testTriangleIds(std::string path){
	std::string fileStr = path + "delaunayResults/initTriangles.tri";

	unsigned int triangleNum;
	unsigned long long *triangleIdArr;
	//comment out this part if you do not want to draw the interior
	readTriangleIds(triangleIdArr, triangleNum, fileStr);
std::cout<<"triangleNum: "<<triangleNum<<"\n";
	//read fullPointPart.ver to pointCoorArr
	double *pointCoorArr;
	unsigned int pointNum;
	readAllCoors(pointCoorArr, pointNum, path + "delaunayResults/fullPointPart.ver");
std::cout<<"pointNum: "<<pointNum<<"\n";
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);

	drawMesh *d = new drawMesh;
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

	delete [] triangleCoorArr;
	delete [] triangleIdArr;
	delete [] pointCoorArr;
}


//================================================================
int main(int argc, char **argv){

	if(argc<=2)// no arguments
		std::cout<<"You need to provide two arguments: path and domainSize\n";
	else{
		std::string path = argv[1];
		int domainSize = atoi(argv[2]);

	std::cout<<"Data in directory: "<<path<<"\n";

		//draw the Delaunay result
		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
//		drawTriangleArr(path);

		//draw init triangle for the first domain (figure 9a_1)
		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
//		drawDomainTriangles(path, domainSize);


		//draw init triangle for the first domain and active coarse partitions (figure 9a_2)
		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
		unsigned coarsePartIdArr[] = {0, 2, 8, 10};
//		drawDomainActivePartitions(path, coarsePartIdArr, 4);

		//draw init triangle of active coarse partitions after refinement (already initialization from each coarse partition) (figure 9b)
		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
//		drawInitTrianglesAllCoarseParts(path);


		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
		unsigned coarsePartIdArr1[] = {0, 2, 8, 10};
		unsigned finePartIdArr[] = {0, 3, 9, 15};
//		drawAllActiveFineParts(path, coarsePartIdArr1, 4, finePartIdArr, 4);


		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
//		drawAllInteriorBoundary(path);

		//You need to remove "return" in delaunay2DMain.cpp
		//change (scale = 350; originalX = 20; originalY = 20;) in drawMesh.cpp
//		drawNewDomainTrianglesInNextStage(path);





		//draw large coarse partition 10 including all init trangles (figure 9c)
		//change (scale = 1000; originalX = -450; originalY = -450;) in drawMesh.cpp, coarsePartId = 0, 2, 8, 10
//		drawInitTrianglesOneCoarsePart(10, path);


		//draw 4 active sub-partitions {0, 3, 9, 15} of a coarse partition 10 (figure 9d)
		//change (scale = 1000; originalX = -450; originalY = -450;) in drawMesh.cpp, coarsePartId = 0, 2, 8, 10
		unsigned finePartIdArr1[] = {0, 3, 9, 15};
//		drawOneActiveFineParts(path, 10, finePartIdArr1, 4);


		//Draw active sub partitions {0, 3, 9, 15} in coarse partition 10 after triangulation for sub-partitions (figure 9e)
		//change (scale = 1000; originalX = -450; originalY = -450;) in drawMesh.cpp, coarsePartId = 0, 2, 8, 10
		//unsigned finePartIdArr2[] = {0, 3, 9, 15};
		unsigned finePartIdArr2[] = {1, 3, 8, 15};
//		drawSubBoundaryInteriorTriangles(path, 10, finePartIdArr2, 4);


		//change (scale = 1000; originalX = -450; originalY = -450;) in drawMesh.cpp, coarsePartId = 0, 2, 8, 10
		drawOneInteriorBoundary(path, 10);

//testBoundaryData(path);
//testTriangleIds(path);
	}

	return 0;
}
