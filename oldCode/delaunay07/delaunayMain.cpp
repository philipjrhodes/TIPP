/*
 * main.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */

//g++ common.cpp point.cpp edge.cpp triangle.cpp delaunay.cpp delaunayMain.cpp -o delaunay -lgraph

#include <iostream>
#include <graphics.h>
#include <fstream>

#include "delaunay.h"
#include "common.h"


int scale = 350;
int originalX = 100;
int originalY = 50;

//=================================================================
void drawGridLines(){
	int gBound[10] = {0,0, 1,0, 1,1, 0,1, 0,0};	
	//draw square & gridlines
	for(int i=0; i<10; i++){
		gBound[i]*scale;
		if(i%2==0)
			gBound[i] = gBound[i] * scale + originalX;
		else gBound[i] = gBound[i] * scale + originalY;
	}
	setcolor(RED);
	drawpoly(5, gBound);  // number of points, pointer to array 
	line(0 * scale + originalX,0.25 * scale + originalY,1 * scale + originalX,0.25 * scale + originalY);
	line(0 * scale + originalX,0.5 * scale + originalY,1 * scale + originalX,0.5 * scale + originalY);
	line(0 * scale + originalX,0.75 * scale + originalY,1 * scale + originalX,0.75 * scale + originalY);

	line(0.25 * scale + originalX,0 * scale + originalY,0.25 * scale + originalX,1 * scale + originalY);
	line(0.5 * scale + originalX,0 * scale + originalY,0.5 * scale + originalX,1 * scale + originalY);
	line(0.75 * scale + originalX,0 * scale + originalY,0.75 * scale + originalX,1 * scale + originalY);

}

//=================================================================
void drawTriangle(triangleNode *triangleList, int color){
	int triangCell[8];
	setcolor(color);
	//get coordinates of all point ids
	triangleNode *scanNode = triangleList;
	while(scanNode!=NULL){
		point p1 = scanNode->tri->p1;
		point p2 = scanNode->tri->p2;
		point p3 = scanNode->tri->p3;

		triangCell[0] = p1.getX()*scale + originalX;
		triangCell[1] = p1.getY()*scale + originalY;
		triangCell[2] = p2.getX()*scale + originalX;
		triangCell[3] = p2.getY()*scale + originalY;
		triangCell[4] = p3.getX()*scale + originalX;
		triangCell[5] = p3.getY()*scale + originalY;
		triangCell[6] = triangCell[0];
		triangCell[7] = triangCell[1];

		//delay(50);
		drawpoly(4, triangCell);
		scanNode = scanNode->next;
	}
}

////////////////////////////////////////////////////////////////////////////////
void loadTriangleIndices(std::string path, unsigned long long int *&triangleIndicesArr, unsigned long long int &cellNumMax, int &triangleRecordSize){
	//Read information from mydatabin.ver.xfdl
	std::string fileInfoStr = path  + "mydatabin.tri.xfdl";
	std::ifstream cellInfoFile(fileInfoStr.c_str());
	if(!cellInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first read is triangle record size
	cellInfoFile >> strItem;
	triangleRecordSize = atoi(strItem.c_str());
	//second read 
	cellInfoFile >> strItem;
	cellNumMax = atoi(strItem.c_str());
	cellInfoFile.close();


	//Read triangle indices from mydatabin.tri
	std::string fileStr = path + "mydatabin.tri";
	triangleIndicesArr = new unsigned long long int[cellNumMax*triangleRecordSize];
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"There is no filename : "<<path + fileStr;
		return;
	}
	fread(triangleIndicesArr, sizeof(unsigned long long int), cellNumMax*triangleRecordSize, f);
	fclose(f);
/*std::cout<<"cellNumMax: "<<cellNumMax<<std::endl;
for(int i=0; i<cellNumMax*triangleRecordSize; i++){
if(i%3==0) std::cout<<"\n";
std::cout<<triangleIndicesArr[i]<<" ";
}
*/

}

////////////////////////////////////////////////////////////////////////////////
void loadPointData(std::string path, double *&coorPointArr, unsigned long long int &coorPointArrSize, int &vertexRecordSize){
	//Read information from pointPartInfo.xfdl
	std::string fileInfoStr = path + "pointPartitions/" + "pointPartInfo.xfdl";
	std::ifstream vertexPartInfoFile(fileInfoStr.c_str());
	if(!vertexPartInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first read --> copy to xPartNum
	vertexPartInfoFile >> strItem;
	int xPartNum = atoi(strItem.c_str());

	//second read --> copy to yPartNum
	vertexPartInfoFile >> strItem;
	int yPartNum = atoi(strItem.c_str());

	unsigned int *pointPartInfoArr = new unsigned int[xPartNum*yPartNum];
	unsigned long long int pointNumMax = 0;
	//read all partition size (number of points for each partition)
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		vertexPartInfoFile >> strItem;
		pointPartInfoArr[i] = atoi(strItem.c_str());
		pointNumMax = pointNumMax + pointPartInfoArr[i];
	}
//std::cout<<"\npointNumMax: "<<pointNumMax<<std::endl;
	vertexPartInfoFile.close();

	//all points in domain + 4 point ABCD of convexHull
	coorPointArrSize = pointNumMax + 4;
	vertexRecordSize = 2;

	FILE *f;
	std::string dataFileStr;
	unsigned int pointNumbers;

	coorPointArr = new double[coorPointArrSize*vertexRecordSize];
	unsigned long long int index = 0;
	//read all point coordinates from all partition files in folder pointPartitions
	for(unsigned int partId=0; partId<xPartNum*yPartNum; partId++){
		dataFileStr = generateFileName(partId, path + "pointPartitions/" + "pointPart", xPartNum*yPartNum);
		f = fopen(dataFileStr.c_str(), "rb");
		if(!f){
			std::cout<<"not exist "<<dataFileStr<<std::endl;
			continue;
		}
		pointNumbers = pointPartInfoArr[partId];

		fread(&coorPointArr[index*vertexRecordSize], sizeof(double), pointNumbers*vertexRecordSize, f);
		index += pointNumbers;
		fclose(f);
	}
	//add 4 points ABCD in convexHull to the end of coorPointArr
	coorPointArr[pointNumMax*vertexRecordSize] = 0;
	coorPointArr[(pointNumMax)*vertexRecordSize+1] = 0;
	coorPointArr[(pointNumMax+1)*vertexRecordSize] = 0;
	coorPointArr[(pointNumMax+1)*vertexRecordSize+1] = 1;
	coorPointArr[(pointNumMax+2)*vertexRecordSize] = 1;
	coorPointArr[(pointNumMax+2)*vertexRecordSize+1] = 1;
	coorPointArr[(pointNumMax+3)*vertexRecordSize] = 1;
	coorPointArr[(pointNumMax+3)*vertexRecordSize+1] = 0;

/*std::cout<<"coorPointArrSize: "<<coorPointArrSize<<"\n";
for(int i=0; i<coorPointArrSize; i++){
	std::cout<<i<<" "<<coorPointArr[i*2]<<" "<<coorPointArr[i*2+1]<<"\n";
}
*/
	delete [] pointPartInfoArr;
}
////////////////////////////////////////////////////////////////////////////////
void drawTriangleFromFile(std::string path, int color){
	int triangCell[8];
//	setcolor(color);

	int triangleRecordSize;
	unsigned long long int cellNumMax;
	unsigned long long int *triangleIndicesArr;
	loadTriangleIndices(path, triangleIndicesArr, cellNumMax, triangleRecordSize);

	double *coorPointArr;
	unsigned long long int coorPointArrSize;
	int vertexRecordSize;
	loadPointData(path, coorPointArr, coorPointArrSize, vertexRecordSize);

	//get coordinates of all point ids
	unsigned long long int index;
	for(index=0; index<cellNumMax; index++){

		unsigned long long int cellIndex1 = triangleIndicesArr[index*triangleRecordSize];
		unsigned long long int cellIndex2 = triangleIndicesArr[index*triangleRecordSize+1];
		unsigned long long int cellIndex3 = triangleIndicesArr[index*triangleRecordSize+2];

		triangCell[0] = coorPointArr[cellIndex1*vertexRecordSize]*scale + originalX;
		triangCell[1] = coorPointArr[cellIndex1*vertexRecordSize+1]*scale + originalY;

		triangCell[2] = coorPointArr[cellIndex2*vertexRecordSize]*scale + originalX;
		triangCell[3] = coorPointArr[cellIndex2*vertexRecordSize+1]*scale + originalY;

		triangCell[4] = coorPointArr[cellIndex3*vertexRecordSize]*scale + originalX;
		triangCell[5] = coorPointArr[cellIndex3*vertexRecordSize+1]*scale + originalY;
		triangCell[6] = triangCell[0];
		triangCell[7] = triangCell[1];

//std::cout<<cellIndex1<<" "<<coorPointArr[cellIndex1]<<" "<<coorPointArr[cellIndex1+1]<<"==="<<cellIndex2<<" "<<coorPointArr[cellIndex2]<<" "<<coorPointArr[cellIndex2+1]<<"==="<<cellIndex3<<" "<<coorPointArr[cellIndex3]<<" "<<coorPointArr[cellIndex3+1]<<std::endl;

//		drawpoly(4, triangCell);
	}

	delete [] triangleIndicesArr;
	delete [] coorPointArr;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	int driver, mode;
	driver = DETECT; // autotect
	mode = 0;
	initgraph(&driver, &mode, NULL);
	std::cout.precision(16);

	if(argc==1){// no arguments
		std::cout<<"You need to provide two arguments: path and number of partitions\n";
	}
	else{
		std::string path = argv[1];
		unsigned int partNum = atoi(argv[2]);
		delaunay *d = new delaunay(path, partNum);
		double t1 = GetWallClockTime();
//triangle t(point(0,0), point(1,1), point(2,2.00000000001));
//std::cout<<t.colinear();
		d->delaunayProcess();
//	d->printPointArray();
//	d->printTriangleList();
//	d->printPointArray();
	drawGridLines();
	drawTriangle(d->storeTriangleList, RED);

	drawTriangleFromFile(path, RED);
	drawTriangle(d->triangleList, CYAN);
	delete d;
	std::cout<<"done!!!"<<std::endl;
	t1 = GetWallClockTime() - t1;
	std::cout<<"Dealunay time: "<<t1<<"\n";
	}
	
	getch(); // pause to admire 
	restorecrtmode();
}


