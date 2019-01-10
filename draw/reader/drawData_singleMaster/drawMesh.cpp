#include "drawMesh.h"
#include <iostream>
#include <graphics.h>

drawMesh::drawMesh(){

	scale = 400;
	originalX = 100;
	originalY = 50;

//	scale = 1050;
//	originalX = -50;
//	originalY = -403;

	driver = DETECT; // autotect
	mode = 0;
	initgraph(&driver, &mode, NULL);
}

//=============================================================
void drawMesh::drawGridLines(int xPartNum, int yPartNum){
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

	double xPartSize = 1.0/xPartNum;
	double yPartSize = 1.0/yPartNum;

	for(int i=1; i<yPartNum; i++)
		line(originalX, yPartSize * i * scale + originalY, scale + originalX, yPartSize * i * scale + originalY);

	for(int j=1; j<xPartNum; j++)
		line(xPartSize * j * scale + originalX, originalY, xPartSize * j * scale + originalX, scale + originalY);


/*	line(0 * scale + originalX,0.25 * scale + originalY,1 * scale + originalX,0.25 * scale + originalY);
	line(0 * scale + originalX,0.5 * scale + originalY,1 * scale + originalX,0.5 * scale + originalY);
	line(0 * scale + originalX,0.75 * scale + originalY,1 * scale + originalX,0.75 * scale + originalY);

	line(0.25 * scale + originalX,0 * scale + originalY,0.25 * scale + originalX,1 * scale + originalY);
	line(0.5 * scale + originalX,0 * scale + originalY,0.5 * scale + originalX,1 * scale + originalY);
	line(0.75 * scale + originalX,0 * scale + originalY,0.75 * scale + originalX,1 * scale + originalY);
*/
}

//save triangleList to file .tri (all coordinates x1 y1 x2 y2 x3 y3 ....)
void drawMesh::writeTriangleCoorsFromTriangleList(triangleNode *triangleList){
	unsigned int triangleNum = size(triangleList);
	double *triangleCoorArr = new double[triangleNum*6];
	triangleNode *scanNode = triangleList;
	unsigned int index=0;
	while(scanNode!=NULL){
		triangleCoorArr[index*6] = scanNode->tri->p1.getX();
		triangleCoorArr[index*6+1] = scanNode->tri->p1.getY();
		triangleCoorArr[index*6+2] = scanNode->tri->p2.getX();
		triangleCoorArr[index*6+3] = scanNode->tri->p2.getY();
		triangleCoorArr[index*6+4] = scanNode->tri->p3.getX();
		triangleCoorArr[index*6+5] = scanNode->tri->p3.getY();

		index++;
		scanNode = scanNode->next;
	}

	//write to file
	std::string fileStr = "../dataSources/1Kvertices/delaunayResults/triangleCoors.tri";
	FILE *f = fopen(fileStr.c_str(), "w");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);

	delete [] triangleCoorArr;
}
//=============================================================
void drawMesh::drawTriangles(triangleNode *triangleList, int color){
	int triangCell[8];

	writeTriangleCoorsFromTriangleList(triangleList);

	setcolor(color);
	//get coordinates of all point ids
	triangleNode *scanNode = triangleList;
	while(scanNode!=NULL){
		point p1 = scanNode->tri->p1;
		point p2 = scanNode->tri->p2;
		point p3 = scanNode->tri->p3;

		double centerX = scanNode->tri->centerX;
		double centerY = scanNode->tri->centerY;
		double radius = scanNode->tri->radius;

		triangCell[0] = p1.getX()*scale + originalX;
		triangCell[1] = p1.getY()*scale + originalY;
		triangCell[2] = p2.getX()*scale + originalX;
		triangCell[3] = p2.getY()*scale + originalY;
		triangCell[4] = p3.getX()*scale + originalX;
		triangCell[5] = p3.getY()*scale + originalY;
		triangCell[6] = triangCell[0];
		triangCell[7] = triangCell[1];

		drawpoly(4, triangCell);

//		circle(centerX*scale + originalX, centerY*scale + originalY, radius*scale);

		scanNode = scanNode->next;
//delay(1000);
	}
	getch(); // pause to admire 
}


//save triangleList to file .tri (all coordinates x1 y1 x2 y2 x3 y3 ....)
void drawMesh::writeTriangleCoorsFromTriangleArr(triangle *triangleArr, unsigned int triangleNum){

	double *triangleCoorArr = new double[triangleNum*6];
	unsigned int index=0;
	for(unsigned int i=0; i<triangleNum; i++){
		triangleCoorArr[index*6] = triangleArr[i].p1.getX();
		triangleCoorArr[index*6+1] = triangleArr[i].p1.getY();
		triangleCoorArr[index*6+2] = triangleArr[i].p2.getX();
		triangleCoorArr[index*6+3] = triangleArr[i].p2.getY();
		triangleCoorArr[index*6+4] = triangleArr[i].p3.getX();
		triangleCoorArr[index*6+5] = triangleArr[i].p3.getY();
		index++;
	}

	//write to file
	std::string fileStr = "../dataSources/1Kvertices/delaunayResults/triangleCoors.tri";
	FILE *f = fopen(fileStr.c_str(), "w");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fwrite(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);

	delete [] triangleCoorArr;
}

//=============================================================
void drawMesh::drawTriangleArr(triangle *triangleArr, int triangArrSize,  int color){
	int triangCell[8];

	writeTriangleCoorsFromTriangleArr(triangleArr, triangArrSize);
	boundingBox activeBox(point(0.25,0.5), point(0.5,0.75));

/*	setcolor(RED);
	line(0.25 * scale + originalX,0.5 * scale + originalY,0.5 * scale + originalX,0.5 * scale + originalY);
	line(0.25 * scale + originalX,0.75 * scale + originalY,0.5 * scale + originalX,0.75 * scale + originalY);
	line(0.25 * scale + originalX,0.5 * scale + originalY,0.25 * scale + originalX,0.75 * scale + originalY);
	line(0.5 * scale + originalX,0.5 * scale + originalY,0.5 * scale + originalX,0.75 * scale + originalY);
*/
	setcolor(color);
	for(int i=0; i<triangArrSize; i++){
//	if(triangleArr[i].intersect(activeBox)){
		point p1 = triangleArr[i].p1;
		point p2 = triangleArr[i].p2;
		point p3 = triangleArr[i].p3;

		double centerX = triangleArr[i].centerX;
		double centerY = triangleArr[i].centerY;
		double radius = triangleArr[i].radius;

		triangCell[0] = p1.getX()*scale + originalX;
		triangCell[1] = p1.getY()*scale + originalY;
		triangCell[2] = p2.getX()*scale + originalX;
		triangCell[3] = p2.getY()*scale + originalY;
		triangCell[4] = p3.getX()*scale + originalX;
		triangCell[5] = p3.getY()*scale + originalY;
		triangCell[6] = triangCell[0];
		triangCell[7] = triangCell[1];

		drawpoly(4, triangCell);
//	}

//		circle(centerX*scale + originalX, centerY*scale + originalY, radius*scale);

//delay(1000);
	}
//	getch(); // pause to admire 
}

//=============================================================
void drawMesh::drawTriangleCoorArr(double *triangleArr, int triangArrSize,  int color){
	int triangCell[8];

	boundingBox activeBox(point(0.25,0.5), point(0.5,0.75));
	setcolor(color);

	for(int i=0; i<triangArrSize; i++){
		point p1(triangleArr[i*6], triangleArr[i*6+1]);
		point p2(triangleArr[i*6+2], triangleArr[i*6+3]);
		point p3(triangleArr[i*6+4], triangleArr[i*6+5]);

		triangCell[0] = p1.getX()*scale + originalX;
		triangCell[1] = p1.getY()*scale + originalY;
		triangCell[2] = p2.getX()*scale + originalX;
		triangCell[3] = p2.getY()*scale + originalY;
		triangCell[4] = p3.getX()*scale + originalX;
		triangCell[5] = p3.getY()*scale + originalY;
		triangCell[6] = triangCell[0];
		triangCell[7] = triangCell[1];

		drawpoly(4, triangCell);
	}
}


//=============================================================
drawMesh::~drawMesh(){
	getch();
	restorecrtmode();
	closegraph();
}
