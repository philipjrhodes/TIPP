/*
 * delaunay.h
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */

#ifndef H_DELAUNAY
#define H_DELAUNAY

#include "linkList.h"
#include <list>

class delaunay{
public:
	double *coorPointArr;//point coordinate array [x1 y1 x2 y2 ... xn yn]
	double *gridCoorPointArr;//grid points
	int gridCoorPointArrSize;

	//number of partitions on regarding x and y axes
	int xPartNum;
	int yPartNum;
	unsigned int partNum;

	//path to data point files
	std::string path;

	//vertexRecordSize = 2 in 2D
	int vertexRecordSize;

	//pointPartInfoArr contains number of points for each partition
	unsigned int *pointPartInfoArr;

	//total number of point in domain
	unsigned long long int pointNumMax;

	//number of triangles
	unsigned long long int triangleNumMax;	

	//current point id
	unsigned long long int globalPointIndex;

	//number of current active point in coorPointArr
	unsigned int pointNumbers;

//	unsigned int pointNumbers;//number of points
	triangleNode *triangleList;
	triangleNode *storeTriangleList;
	triangleNode *temporaryTriangleList;

	void loadGridPoints();
	void initTriangulate();
	delaunay(std::string p, unsigned int partitionNum);

	//read fileInfo from mydatabin.ver.xfdl
	void readPointPartFileInfo();

	//Read each partition from file to coorPointArr
	//then insert those points onto domain for traigulating.
	void delaunayProcess();

	//load point array from files
	void loadPointArr(unsigned int partId);
	void printPointArray(unsigned int partId);
	void printTriangleList();
	void storeTriangles(triangleNode *&storeTriangleList);

	//implement delaunay triangulation
	void triangulate(unsigned int partId);

	~delaunay();
};

#endif



