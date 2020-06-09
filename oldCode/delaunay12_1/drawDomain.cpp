#include <iostream>
#include <string>
#include "drawMesh.h"
#include <fstream>
#include <cmath>

//g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph 
//edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp

//./drawDomain ../dataSources/1Kvertices/


//==================================================================================
double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3){
	return fabs((x1-x3)*(y2-y1) - (x1-x2)*(y3-y1))/2;
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
	d->	drawGridLines(4, 4);
	d->drawTriangleArr(triangleArr, triangleNum, 2);//GREEN
	delete d;


	delete [] pointCoorArr;
	delete [] pointIdArr;
	delete [] triangleArr;
}

//================================================================
int main(int argc, char **argv){

	if(argc==1)// no arguments
		std::cout<<"You need to provide two arguments: path\n";
	else{
		std::string path = argv[1];
		drawTriangleArr(path);
	}

	return 0;
}
