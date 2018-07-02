#include "drawMesh.h"
#include <iostream>
#include <graphics.h>

drawMesh::drawMesh(){


//	scale = 450;
//	originalX = 20;
//	originalY = 20;

	scale = 1300;
	originalX = -580;
	originalY = -580;


	driver = DETECT; // autotect
	mode = 0;
	initgraph(&driver, &mode, NULL);
}

//==============================================================================
void drawMesh::drawBox(point lowPoint, point highPoint, int color){
	setcolor(color);
	int square[10];

	square[0] = originalX + lowPoint.getX()*scale;
	square[1] = originalY + lowPoint.getY()*scale;
	square[2] = originalX + highPoint.getX()*scale;
	square[3] = originalY + lowPoint.getY()*scale;
	square[4] = originalX + highPoint.getX()*scale;
	square[5] = originalY + highPoint.getY()*scale;
	square[6] = originalX + lowPoint.getX()*scale;
	square[7] = originalY + highPoint.getY()*scale;
	square[8] = square[0];
	square[9] = square[1];

	drawpoly(5, square);
}

//==============================================================================
//given an index, find a bounding box of a partition in a domain
//input: + partIndex --> partition index of a partition in the domain 
//		 + lowPoint, highPoint --> the domain points
//		 + xPartNum, yPartNum --> granularity of partitions in the domain
//output: the bounding box of cuurent partition
boundingBox drawMesh::findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum){
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

//=============================================================
void drawMesh::oldDrawGridLines(int xPartNum, int yPartNum){
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
}

//=============================================================
void drawMesh::drawGridLines(point lowPoint, point highPoint, int xPartNum, int yPartNum, int color){
	for(int i=0; i<xPartNum*yPartNum; i++){
		boundingBox b = findPart(i, lowPoint, highPoint, xPartNum, yPartNum);
//std::cout<<i<<" "<<b.getLowPoint() <<" "<< b.getHighPoint()<<"\n";
		drawBox(b.getLowPoint(), b.getHighPoint(), color);
	}
}

//=============================================================
void drawMesh::drawTriangles(triangleNode *triangleList, int color){
	int triangCell[8];

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

//=============================================================
void drawMesh::drawTriangleArr(triangle *triangleArr, int triangArrSize,  int color){
	int triangCell[8];

	setcolor(color);
	//get coordinates of all point ids

	for(int i=0; i<triangArrSize; i++){
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

//		circle(centerX*scale + originalX, centerY*scale + originalY, radius*scale);

//delay(1000);
	}
//	getch(); // pause to admire 
}

//=============================================================
//draw triangles with input data is the array of coordinate of many 6 doubles
void drawMesh::drawCoordinateTriangles(double *triangleCoorArr, int triangleCoorArrSize,  int color){
	int triangCell[8];

	setcolor(color);
	//get coordinates of all point ids

	for(unsigned int i=0; i<triangleCoorArrSize; i++){

		triangCell[0] = triangleCoorArr[i*6]*scale + originalX;
		triangCell[1] = triangleCoorArr[i*6+1]*scale + originalY;
		triangCell[2] = triangleCoorArr[i*6+2]*scale + originalX;
		triangCell[3] = triangleCoorArr[i*6+3]*scale + originalY;
		triangCell[4] = triangleCoorArr[i*6+4]*scale + originalX;
		triangCell[5] = triangleCoorArr[i*6+5]*scale + originalY;
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
