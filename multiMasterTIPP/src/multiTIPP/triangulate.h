#include "common.h"
#include "linkList.h"

#ifndef H_TRIANGULATE
#define H_TRIANGULATE

void triangulatePartition(boundingBox squareBBox, point *pointArr, unsigned pointNum, triangleNode *&initialTriangleList, triangleNode *&interiorTriangleList, triangleNode *&boundaryTriangleList);

void triangulateDomain(point *pointArr, unsigned pointNum, triangleNode *&triangleList);

void extractInteriorTriangles(boundingBox partBBox, triangleNode *&triangleList, triangleNode *&interiorTriangleList);

#endif
