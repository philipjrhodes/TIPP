#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include "common.h"
#include "triangulate.h"


class delaunaySerial{
public:
	std::string path;

	//domain sizes
	unsigned domainSize;
	unsigned int xPartNum, yPartNum;

	//all point coordinates of partitions
	point *pointCoorArr;
	unsigned pointNum;

	//list of triangles that are currently processed delaunay
	triangleNode *initTriangleList;
	//list of triangles that are not currently processed delaunay, will be processed by other partitions
	triangleNode *interiorTriangleList;
	//list of triangle Ids that are not currently processed delaunay, circumcircles are inside partition
	triangleNode *boundaryTriangleList;

	delaunaySerial(unsigned xPartNumInput, unsigned yPartNumInput, unsigned domainSizeInput, std::string srcPath);

	void readPointCoor(unsigned partId);
	void triangulate(unsigned partId);
	boundingBox partBox(unsigned partId);
	void storeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string fileStr);

	void processSerial(unsigned partId, triangleNode *&initTriangleListInput, triangleNode *&returnInteriorTriangleList, triangleNode *&returnBoundaryTriangleList);
};


