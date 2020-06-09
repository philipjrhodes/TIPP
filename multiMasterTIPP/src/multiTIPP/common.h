#include <sstream>
#include <string>
#include <list>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#include "triangle.h"
#include "linkList.h"
#include "boundingBox.h"
#include "gridBound.h"
#include "gridElement.h"

#ifndef COMMON_H
#define COMMON_H

//============================================================================
#define releaseMemory(varArr)\
	if(varArr != NULL){\
		delete [] varArr; varArr = NULL;\
	}
#define allocateMemory(varArr, dataType, varArrSize)\
	try { varArr = new dataType[varArrSize];}\
	catch (std::bad_alloc&){\
		std::cout<<"Memory overflow!!!!!!!!!!!\n";\
		exit(-1);\
	}


//One node contains N number of triangles, each triangle contains three Ids
struct nodeTriangleIdArr{
	unsigned long long *triangleIdArr;
	unsigned long long triangleNum;
	nodeTriangleIdArr(){
		triangleIdArr = NULL;
		triangleNum = 0;
	}
};

struct nodeTriangleCoorArr{
	double *triangleCoorArr;
	unsigned long long triangleNum;
	nodeTriangleCoorArr(){
		triangleCoorArr = NULL;
		triangleNum = 0;
	}
};


//============================================================================
//template <typename T>
//void generateOffsetArr(unsigned *numArr, T *&numOffsetArr, unsigned numArrSize){
//	numOffsetArr = new T[numArrSize];
//	numOffsetArr[0] = 0;
//	for(unsigned i=1; i<numArrSize; i++)
//		numOffsetArr[i] = numOffsetArr[i-1] + numArr[i-1];
//}

void generateOffsetArr(unsigned *intArr, unsigned *&intOffsetArr, unsigned intArrSize);
void generateOffsetArr(unsigned long long *intArr, unsigned long long *&intOffsetArr, unsigned long long intArrSize);

std::string toString(int value);
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum, std::string ext);
double GetWallClockTime(void);
int coorX_comparison(const void *p1, const void *p2);

void apdaptiveAllocateGroupProcesses(unsigned int *activePartPointSizeArr, unsigned int world_size, unsigned int currActivePartNum, unsigned int *&rankIdArr, unsigned int *&colorArr);
void equalAllocateGroupProcesses(unsigned int world_size, unsigned int currActivePartNum, unsigned int *&rankIdArr, unsigned int *&colorArr);


void copyTriangleIdsFromTriangleArr(unsigned long long *&triangleIdArr, unsigned long long triangleNum, triangle *triangleArr);
void copyTriangleCoorsFromTriangleArr(double *&triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr);
void copyTrianglesFromTriangleArr(unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr);
void copyTriangles(unsigned long long *&triangleIdArr, double *&triangleCoorArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr);
void copyTriangleIdArr(unsigned long long *&triangleIdArr, std::list<unsigned> triangleIdList, triangle *triangleArr);
void copyTriangleIdArr(unsigned long long *&triangleIdArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr);


void extractTriangleIds(triangleNode *triangleList, unsigned long long *&triangleIdArr, unsigned long long &triangleNum);
void extractTriangleIdsCoors(triangleNode *triangleList, unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long &triangleNum);
void extractTriangleIdsFromTriangleList(std::list<triangle> triangleList, unsigned long long *&triangleIdArr, unsigned long long &triangleNum);
void extractTriangleIdsCoorsFromTriangleList(std::list<triangle> triangleList, unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long &triangleNum);
void generateTriangleList(unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, triangleNode *&triangleList);
void generateTriangleArr(unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr);
void generateOffsetArr(unsigned *intArr, unsigned intArrSize, unsigned *&intOffsetArr);


gridBound boundingGrid(boundingBox bBox, boundingBox domainBound, unsigned xPartNum, unsigned yPartNum);
gridElement mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY, boundingBox domainBound);
gridElement mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY, boundingBox domainBound);

boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);


#endif
