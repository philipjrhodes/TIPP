#include <iostream>
#include <string>
#include "drawMesh.h"
#include <fstream>
#include <cmath>

//g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp boundingBox.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph

// ./drawDomain ../dataSources/10Kvertices/ ../dataSources/10Kvertices/ 8 8 1
//16 means domainSize
//8 means xPartNum or yPartNum
//1 means option


//==================================================================================
double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3){
	return fabs((x1-x3)*(y2-y1) - (x1-x2)*(y3-y1))/2;
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
		triangleIdArr = NULL;
		triangleNum = 0;
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

//===================================================================
void drawInitTriangles(std::string srcPath, std::string dstPath, std::string fileName, unsigned int domainSize, unsigned partNumBothSize){
	drawMesh *d = new drawMesh;
	d->	drawGridLines(point(0,0), point(1, 1), partNumBothSize, partNumBothSize, 4);

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + "fullPointPart.ver");

	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	double *triangleCoorArr;

	std::string currPath = dstPath + fileName;
	readTriangleIds(triangleIdArr, triangleNum, currPath);
	if(triangleNum==0) return;
	triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);

	//scale down triangleCoorArr (in doamin 1x1)
	for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
	delete [] triangleIdArr;
	delete [] triangleCoorArr;

	delete d;
	delete [] allPointCoorArr;
}

//===================================================================
void drawOnePartition(std::string srcPath, std::string dstPath, std::string fileName, unsigned partId, unsigned int domainSize, unsigned partNumBothSize){
	drawMesh *d = new drawMesh;
	d->	drawGridLines(point(0,0), point(1, 1), partNumBothSize, partNumBothSize, 4);

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + "fullPointPart.ver");

	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	double *triangleCoorArr;

	std::string currPath = generateFileName(partId, dstPath + fileName, partNumBothSize*partNumBothSize, ".tri");
	readTriangleIds(triangleIdArr, triangleNum, currPath);
	if(triangleNum==0) return;
	triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);

	//scale down triangleCoorArr (in doamin 1x1)
	for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
	delete [] triangleIdArr;
	delete [] triangleCoorArr;

	delete d;
	delete [] allPointCoorArr;

}

//===================================================================
void drawAllPartitions(std::string srcPath, std::string dstPath, unsigned int domainSize, unsigned partNumBothSize, std::string interiorTriangleStr){
	drawMesh *d = new drawMesh;
	d->	drawGridLines(point(0,0), point(1, 1), partNumBothSize, partNumBothSize, 4);

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + "fullPointPart.ver");

	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	double *triangleCoorArr;

	for(unsigned partId=0; partId<partNumBothSize*partNumBothSize; partId++){
		//std::string currPath = generateFileName(partId, dstPath + "/triangleIds", partNumBothSize*partNumBothSize, ".tri");
		std::string currPath = generateFileName(partId, dstPath + "/" + interiorTriangleStr, partNumBothSize*partNumBothSize, ".tri");
		readTriangleIds(triangleIdArr, triangleNum, currPath);
		if(triangleNum==0) continue;
		triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);

		//scale down triangleCoorArr (in doamin 1x1)
		for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

		std::cout<<"draw partition "<<currPath<<"\n";
		d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleIdArr;
		delete [] triangleCoorArr;
	}

	std::string currPath = dstPath + "/allBoundaryTriangleIds.tri";
	readTriangleIds(triangleIdArr, triangleNum, currPath);
	triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);
	for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;
	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 5);//GREEN
	delete [] triangleIdArr;
	delete [] triangleCoorArr;

	delete d;
	delete [] allPointCoorArr;
}


//===================================================================
//Based on all triangles ids in triangleIds.tri, draw all triangles (include triangles with grid points)
void drawTriangleArr(std::string srcPath, std::string dstPath, unsigned int domainSize, unsigned partNumBothSize, std::string triangleIdFile){
	//read fullPointPart.ver to pointCoorArr
	std::string fileStr = srcPath + "/fullPointPart.ver";
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
	std::string fileStr1 = dstPath + "/" + triangleIdFile;
	f = fopen(fileStr1.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(3*sizeof(unsigned long long)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	unsigned long long *pointIdArr = new unsigned long long[triangleNum*3];
	fread(pointIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);


//for(int i=0; i<triangleNum; i++)
//	std::cout<<pointIdArr[i*3]<<" "<<pointIdArr[i*3+1]<<" "<<pointIdArr[i*3+2]<<"\n";

	double triangleAreas = 0;

	//build storeTriangleArr
	triangle *triangleArr = new triangle[triangleNum];
std::cout<<"pointNum: "<<pointNum<<", and number of triangles: "<<triangleNum<<"\n";
	for(int i=0; i<triangleNum; i++){
		unsigned long long pointId1 = pointIdArr[i*3];
		unsigned long long pointId2 = pointIdArr[i*3+1];
		unsigned long long pointId3 = pointIdArr[i*3+2];
//std::cout<<pointId1<<" "<<pointId2<<" "<<pointId3<<"\n";
		triangleArr[i].p1.setId(pointId1);
		triangleArr[i].p2.setId(pointId2);
		triangleArr[i].p3.setId(pointId3);

		triangleArr[i].p1.setX(pointCoorArr[pointId1*2]/domainSize);
		triangleArr[i].p1.setY(pointCoorArr[pointId1*2+1]/domainSize);

		triangleArr[i].p2.setX(pointCoorArr[pointId2*2]/domainSize);
		triangleArr[i].p2.setY(pointCoorArr[pointId2*2+1]/domainSize);

		triangleArr[i].p3.setX(pointCoorArr[pointId3*2]/domainSize);
		triangleArr[i].p3.setY(pointCoorArr[pointId3*2+1]/domainSize);
//std::cout<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<"\n";
		triangleAreas += triangleArea(triangleArr[i].p1.getX(), triangleArr[i].p1.getY(), triangleArr[i].p2.getX(), triangleArr[i].p2.getY(), triangleArr[i].p3.getX(), triangleArr[i].p3.getY());
	}
	std::cout<<"triangleAreas: "<<triangleAreas<<std::endl;

	drawMesh *d = new drawMesh;
	d->	drawGridLines(point(0,0), point(1, 1), partNumBothSize, partNumBothSize, 4);
	d->drawTriangleArr(triangleArr, triangleNum, 2);//GREEN
	delete d;


	delete [] pointCoorArr;
	delete [] pointIdArr;
	delete [] triangleArr;
}

//================================================================
int main(int argc, char **argv){

	if(argc<=5){// no arguments
		std::cout<<"You need to provide four arguments: source path, destination path, domain size, partition number both sides, and option\n";
		exit(1);
	}
	else{
		std::string srcPath = argv[1];
		std::string dstPath = argv[2];
		unsigned int domainSize = atoi(argv[3]);
		unsigned xPartNum = atoi(argv[4]);
		unsigned yPartNum = atoi(argv[4]);
		unsigned option = atoi(argv[5]);
		unsigned partId=0;

		switch (option){
			//draw all boundary triangles in domain
			case 0:	drawInitTriangles(srcPath, dstPath, "initialDomainTriangles.tri", domainSize, xPartNum);break;
			case 1: drawTriangleArr(srcPath, dstPath, domainSize, xPartNum, "triangleIds.tri");break;
			case 2: std::cout<<"Please input a partition id (from 0 to "<<xPartNum*yPartNum<<"): "; 
					std::cin >> partId;
					//drawOnePartition(srcPath, dstPath, "triangleIds", partId, domainSize, xPartNum);
					//drawOnePartition(srcPath, dstPath, "interiorTriangleIds", partId, domainSize, xPartNum);
					//drawOnePartition(srcPath, dstPath, "boundaryTrianglePart", partId, domainSize, xPartNum);
					drawOnePartition(srcPath, dstPath, "interiorTriangleIds", partId, domainSize, xPartNum);
					break;
			case 3: drawAllPartitions(srcPath, dstPath, domainSize, xPartNum, "interiorTriangleIds");
		}		
	}

	return 0;
}
