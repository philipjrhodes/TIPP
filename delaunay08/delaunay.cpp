/*
 * delaunay.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */


//https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm
//https://github.com/Bl4ckb0ne/delaunay-triangulation

#include "delaunay.h"
#include "common.h"

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <fstream>


delaunay::~delaunay(){
	//store triangles that are left over
	storeTriangles(triangleList);
	storeTriangles(temporaryTriangleList);

	removeLinkList(triangleList);
	removeLinkList(storeTriangleList);
	removeLinkList(temporaryTriangleList);
	delete [] pointPartInfoArr;
	delete [] gridCoorPointArr;
}
//==============================================================================
void delaunay::readPointPartFileInfo(){
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
	xPartNum = atoi(strItem.c_str());

	//second read --> copy to yPartNum
	vertexPartInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());

	pointPartInfoArr = new unsigned int[xPartNum*yPartNum];
	pointNumMax = 0;
	//read all partition size (number of points for each partition)
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		vertexPartInfoFile >> strItem;
		pointPartInfoArr[i] = atoi(strItem.c_str());
		std::cout<<pointPartInfoArr[i]<<" ";
		pointNumMax = pointNumMax + pointPartInfoArr[i];
	}

//std::cout<<"pointNumMax: "<<pointNumMax<<std::endl;

	//the size of gridPoints
	vertexPartInfoFile >> strItem;
	gridCoorPointArrSize = atoi(strItem.c_str());

	vertexPartInfoFile.close();
}

//==============================================================================
void delaunay::printPointArray(unsigned int partId){
//	std::cout.precision(16);
std::cout<<partId<<"+++ "<<pointPartInfoArr[partId]<<std::endl;
	for(unsigned int index=0; index<pointPartInfoArr[partId]; index++)
		std::cout<<coorPointArr[index*vertexRecordSize]<<" "<<coorPointArr[index*vertexRecordSize+1]<<std::endl;
		std::cout<<partId<<"+++ "<<pointPartInfoArr[partId]<<std::endl;
}

//==============================================================================
void delaunay::loadPointArr(unsigned int partId){
	std::string dataFileStr = generateFileName(partId, path + "pointPartitions/" + "pointPart", xPartNum*yPartNum);

	FILE *f = fopen(dataFileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		return;
	}

	pointNumbers = pointPartInfoArr[partId];
//	if(coorPointArr!=NULL) delete [] coorPointArr;
	coorPointArr = new double[pointNumbers*vertexRecordSize];
	fread(coorPointArr, sizeof(double), pointNumbers*vertexRecordSize, f);
	fclose(f);
	std::cout<<"========partition "<<partId<<" loaded=========\n";
}

//==============================================================================
delaunay::delaunay(std::string p, unsigned int partitionNum){
	vertexRecordSize = 2;
	partNum = partitionNum;
	//initalize triangleList without element
	triangleList = NULL;
	storeTriangleList = NULL;
	temporaryTriangleList = NULL;
	path = p;
	coorPointArr = NULL;
	gridCoorPointArr = NULL;

	readPointPartFileInfo();
	pointNumbers = 0;

	//globalPointIndex start from 0
	globalPointIndex = 0;

	//determine super-triangle (must be large enough to completely contain all the points)
	//determine super-triangle (must be large enough to completely contain all the points)
	//domain is a square ABCD between (0,0) and (1,1), two initial super triangles are ABC and ACD
	//two coners A(0,0) and B(0,1) in convexHull take global indices pointNumMax and pointNumMax+1
	//two other points in convexHull are C(1,1) and D(1,0) with global indices are pointNumMax+2 and pointNumMax+3
	triangle *t1 = new triangle(point(0.0, 0.0, pointNumMax), point(0.0, 1.0, pointNumMax+1), point(1.0, 1.0, pointNumMax+2));
	//second super triangle is triangle ACD
	triangle *t2 = new triangle(point(0.0, 0.0, pointNumMax), point(1.0, 1.0, pointNumMax+2), point(1.0, 0.0, pointNumMax+3));

	triangleNode *n1 = new triangleNode;
	n1->tri = t1;
	triangleNode *n2 = new triangleNode;
	n2->tri = t2;	
	
	//and add the super-triangles
	insertFront(triangleList, n1);
	insertFront(triangleList, n2);

	triangleNumMax = 0;
}

//==============================================================================
//load grid points for gridCoorPointArr
void delaunay::loadGridPoints(){
	std::string dataFileStr = path + "pointPartitions/gridPoints.ver";
	FILE *f = fopen(dataFileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		return;
	}
	gridCoorPointArr = new double[gridCoorPointArrSize*vertexRecordSize];
	fread(gridCoorPointArr, sizeof(double), gridCoorPointArrSize*vertexRecordSize, f);
	fclose(f);	
}

//==============================================================================
//input: an array of points (gridPoint.ver)
//output: init Delaunay
//==============================================================================
void delaunay::initTriangulate(){
	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;

	loadGridPoints();

	//sequentially insert point into delaunay
	for(unsigned int pointIndex=0; pointIndex<gridCoorPointArrSize; pointIndex++){
		p.setX(gridCoorPointArr[pointIndex*vertexRecordSize]);
		p.setY(gridCoorPointArr[pointIndex*vertexRecordSize+1]);
		p.setId(pointIndex+pointNumMax+4);

		triangleNode *preNode = NULL;
		triangleNode *currNode = triangleList;
		bool goNext = true;
		while(currNode != NULL){
			if(currNode->tri->circumCircleContains(p)){
				edge *edge1 = new edge(currNode->tri->getEdge1());
				edge *edge2 = new edge(currNode->tri->getEdge2());
				edge *edge3 = new edge(currNode->tri->getEdge3());

				edgeNode *eNode1 = createNewNode(edge1);
				edgeNode *eNode2 = createNewNode(edge2);
				edgeNode *eNode3 = createNewNode(edge3);

				insertFront(polygon, eNode1);
				insertFront(polygon, eNode2);
				insertFront(polygon, eNode3);

				removeNode(triangleList, preNode, currNode);
				goNext = false;
			}

			//go to the next element in the liskList
			if(goNext){
				preNode = currNode;
				currNode = currNode->next;
			}
			goNext = true;
		}

		//Find all bad edges from polygon, then remove bad edges later
		edgeNode *preEdgeNode1 = NULL;
		edgeNode *currEdgeNode1 = polygon;
		edgeNode *preEdgeNode2 = NULL;
		edgeNode *currEdgeNode2 = polygon;
		while(currEdgeNode1!=NULL){
			while(currEdgeNode2!=NULL){
				//two different edges in polygon but same geological edge
				if((currEdgeNode1!=currEdgeNode2)&&(*(currEdgeNode1->ed) == *(currEdgeNode2->ed))){
					edge *newEdge = new edge(*currEdgeNode1->ed);
					edgeNode *newEdgeNode = createNewNode(newEdge);
					insertFront(badEdges, newEdgeNode);
				}
				preEdgeNode2 = currEdgeNode2;
				currEdgeNode2 = currEdgeNode2->next;
			}
			preEdgeNode1 = currEdgeNode1;
			currEdgeNode1 = currEdgeNode1->next;
			currEdgeNode2 = polygon;
		}

		//remove all bad edges from polygon
		edgeNode *badEdgeNode = badEdges;
		edgeNode *preEdgeNode = NULL;
		edgeNode *currEdgeNode = polygon;
		while(badEdgeNode!=NULL){
			while(currEdgeNode!=NULL){
				if(*(badEdgeNode->ed) == *(currEdgeNode->ed))
					removeNode(polygon, preEdgeNode, currEdgeNode);
				else{
					preEdgeNode = currEdgeNode;
					currEdgeNode = currEdgeNode->next;
				}
			}
			badEdgeNode = badEdgeNode->next;
			
			preEdgeNode = NULL;
			currEdgeNode = polygon;
		}

		//remove all bad edges
		removeLinkList(badEdges);

		//polygon now is a hole. With insert point p, form new triangles and insert into triangleList
		edgeNode *scanEdgeNode = polygon;
		while(scanEdgeNode!=NULL){
			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			triangleNode *newTriangleNode = createNewNode(newTriangle);
			insertFront(triangleList, newTriangleNode);
			scanEdgeNode = scanEdgeNode->next;
		}

		//delete polygon, badTriangle
		removeLinkList(polygon);
	}
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void delaunay::triangulate(unsigned int partId){

	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;

int triangleInHoleNum = 0;
int maxTriangleNum = 0;

	double sweepLine = 0;
	//the current top edge of current partition
	double topLine = (1.0/yPartNum)*(partId/xPartNum+1);
	//std::cout<<"topLine: "<<topLine<<std::endl;
	
	//start to process first partition of a new row, add all triangles in temporaryTriangleList to triangleList
	if(partId % xPartNum == 0){//start a new row
		if(temporaryTriangleList != NULL)
			// add all triangles in temporaryTriangleList to triangleList
			addLinkList(temporaryTriangleList, triangleList);
	}

	//sequentially insert point into delaunay
	for(unsigned int localPointIndex=0; localPointIndex<pointNumbers; localPointIndex++){
	//for(unsigned int localPointIndex=0; localPointIndex<3; localPointIndex++){
		p.setX(coorPointArr[localPointIndex*vertexRecordSize]);
		p.setY(coorPointArr[localPointIndex*vertexRecordSize+1]);
		p.setId(globalPointIndex);
		globalPointIndex++;
//std::cout<<"====================================\n";
//std::cout<<p.getX()<<" "<<p.getY()<<std::endl;
		sweepLine = p.getX();

int badTriangleNum = 0;
int newTriangleNum = 0;

		triangleNode *preNode = NULL;
		triangleNode *currNode = triangleList;
		bool goNext = true;
		while(currNode != NULL){
			//this triangle will never circumcircle the new point p, store it to storeTriangleList
			if(currNode->tri->getFarestCoorX() < sweepLine){
				if(currNode->tri->getHighestCoorY() < topLine){//if this triangle "near" the top edge of current sqare, store it because it does not affect the new inserting point
					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(storeTriangleList, exNode);
				}
				//store this triangle to temporaryTriangleList and use it for the next row
				else{
					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(temporaryTriangleList, exNode);
				}
				goNext = false;
			}
			else
			if(currNode->tri->circumCircleContains(p)){
//std::cout<<currNode->tri->p1.getId()<<" "<<currNode->tri->p2.getId()<<" "<<currNode->tri->p3.getId()<<std::endl;
//std::cout<<"TriangleListSize: "<<size(triangleList)<<std::endl;
				edge *edge1 = new edge(currNode->tri->getEdge1());
				edge *edge2 = new edge(currNode->tri->getEdge2());
				edge *edge3 = new edge(currNode->tri->getEdge3());

				edgeNode *eNode1 = createNewNode(edge1);
				edgeNode *eNode2 = createNewNode(edge2);
				edgeNode *eNode3 = createNewNode(edge3);

				insertFront(polygon, eNode1);
				insertFront(polygon, eNode2);
				insertFront(polygon, eNode3);

				removeNode(triangleList, preNode, currNode);
				goNext = false;
badTriangleNum++;
			}

			//go to the next element in the liskList
			if(goNext){
				preNode = currNode;
				currNode = currNode->next;
			}
			goNext = true;
		}

/*
std::cout<<"Edge list before removing bad edges: \n";	
edgeNode *e = polygon;
while(e!=NULL){
std::cout<<e->ed->p1<<" " <<e->ed->p2<<std::endl;
e=e->next;
}
*/
		//Find all bad edges from polygon, then remove bad edges later
		edgeNode *preEdgeNode1 = NULL;
		edgeNode *currEdgeNode1 = polygon;
		edgeNode *preEdgeNode2 = NULL;
		edgeNode *currEdgeNode2 = polygon;
		while(currEdgeNode1!=NULL){
			while(currEdgeNode2!=NULL){
				//two different edges in polygon but same geological edge
				if((currEdgeNode1!=currEdgeNode2)&&(*(currEdgeNode1->ed) == *(currEdgeNode2->ed))){
					edge *newEdge = new edge(*currEdgeNode1->ed);
					edgeNode *newEdgeNode = createNewNode(newEdge);
					insertFront(badEdges, newEdgeNode);
				}
				preEdgeNode2 = currEdgeNode2;
				currEdgeNode2 = currEdgeNode2->next;
			}
			preEdgeNode1 = currEdgeNode1;
			currEdgeNode1 = currEdgeNode1->next;
			currEdgeNode2 = polygon;
		}

		//remove all bad edges from polygon
		edgeNode *badEdgeNode = badEdges;
		edgeNode *preEdgeNode = NULL;
		edgeNode *currEdgeNode = polygon;
		while(badEdgeNode!=NULL){
			while(currEdgeNode!=NULL){
				if(*(badEdgeNode->ed) == *(currEdgeNode->ed))
					removeNode(polygon, preEdgeNode, currEdgeNode);
				else{
					preEdgeNode = currEdgeNode;
					currEdgeNode = currEdgeNode->next;
				}
			}
			badEdgeNode = badEdgeNode->next;
			
			preEdgeNode = NULL;
			currEdgeNode = polygon;
		}

		//remove all bad edges
		removeLinkList(badEdges);

/*
std::cout<<"Edge list after removing bad edges: \n";

edgeNode *e1 = polygon;
while(e1!=NULL){
std::cout<<e1->ed->p1<<" " <<e1->ed->p2<<std::endl;
e1=e1->next;
}
*/
		//polygon now is a hole. With insert point p, form new triangles and insert into triangleList
		edgeNode *scanEdgeNode = polygon;
		while(scanEdgeNode!=NULL){
			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			triangleInHoleNum++;
//if(newTriangle->colinear()) std::cout<<"this triangle is colinear\n";
			//std::cout<<newTriangle->p1<<"---- "<<newTriangle->p2<<"--- "<<newTriangle->p3<<std::endl;
			triangleNode *newTriangleNode = createNewNode(newTriangle);
			insertFront(triangleList, newTriangleNode);
			scanEdgeNode = scanEdgeNode->next;
newTriangleNum++;
		}

//std::cout<<"badTriangleNum: "<<badTriangleNum<<"  newTriangleNum: "<<newTriangleNum<<std::endl;

		if(maxTriangleNum<triangleInHoleNum) maxTriangleNum = triangleInHoleNum;
		triangleInHoleNum = 0;


		//delete polygon, badTriangle
		removeLinkList(polygon);
	}

std::cout<<"Maximum number of triangles in hole: "<<maxTriangleNum<<std::endl;
//printTriangleList();

	unsigned int triangleStoredSize = size(storeTriangleList);
	unsigned int triangleSize = size(triangleList);
std::cout<<"size of storeTriangleList: "<<triangleStoredSize<<std::endl;
std::cout<<"number of triangles left: "<<triangleSize<<std::endl;
std::cout<<"percent stored: "<<triangleStoredSize*100/(triangleSize+triangleStoredSize)<<"%\n";
}

//==============================================================================
void delaunay::storeTriangles(triangleNode *&storeTriangleList){
	//number of triangle in storeTriangleList
	unsigned int triangleNum = size(storeTriangleList);
	if(triangleNum == 0) return;

	unsigned long long int *triangleData = new unsigned long long int[triangleNum*3];
	unsigned int index = 0;
	
	triangleNode *scanNode = storeTriangleList;
	while(scanNode!=NULL){
		triangleData[index*3] = scanNode->tri->p1.getId();
		triangleData[index*3+1] = scanNode->tri->p2.getId();
		triangleData[index*3+2] = scanNode->tri->p3.getId();
	
		index++;
		scanNode = scanNode->next;
		triangleNumMax++;
	}
	removeLinkList(storeTriangleList);
	storeTriangleList = NULL;

	std::string fileStr = path + "mydatabin.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"There is no filename : "<<path + fileStr;
		return;
	}
	fwrite(triangleData, sizeof(unsigned long long int), triangleNum*3, f);
	fclose(f);

	/*free triangleData*/
	delete [] triangleData;
}

//==============================================================================
void delaunay::printTriangleList(){
	std::cout<<"Print TriangleList and storeTriangleList: \n";
	triangleNode *scanNode1 = triangleList;
	while(scanNode1!=NULL){
		std::cout<<scanNode1->tri->p1.getId()<<" "<<scanNode1->tri->p2.getId()<<" "<<scanNode1->tri->p3.getId()<<std::endl;
		scanNode1 = scanNode1->next;
	}
	
	std::cout<<"=======\n";
	
	triangleNode *scanNode2 = storeTriangleList;
	while(scanNode2!=NULL){
		std::cout<<scanNode2->tri->p1.getId()<<" "<<scanNode2->tri->p2.getId()<<" "<<scanNode2->tri->p3.getId()<<std::endl;
		scanNode2 = scanNode2->next;
	}
}

//==============================================================================
void delaunay::delaunayProcess(){
	//delaunay for grid Points as initialization
	initTriangulate();

//	int partIDArr[] = {0, 16, 32};//for 8x8
	int partIDArr[] = {0, 8};
//	for(unsigned int partId=0; partId<partNum; partId++){
	for(unsigned int partId=0; partId<2; partId++){
		loadPointArr(partIDArr[partId]);
		triangulate(partIDArr[partId]);
		delete [] coorPointArr;


		//store all triangles in storeTriangleList to file
		if(storeTriangleList!=NULL){
//			storeTriangles(storeTriangleList);
		}

	}

	//Create a metadata file for triangles: .xfdl
	std::ofstream xfdlFile;
	std::string xfdlFileStr =  path + "mydatabin.tri.xfdl";
	xfdlFile.open (xfdlFileStr.c_str(), std::ofstream::out | std::ofstream::trunc);
	//add 4 points of convexHull (square ABCD)
	xfdlFile<<3<<"\n"<<triangleNumMax;
	xfdlFile.close();

}

