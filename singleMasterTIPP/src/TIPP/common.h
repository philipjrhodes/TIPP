#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <time.h>
#include <sys/time.h>

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

//============================================================================
template <typename T>
void generateOffsetArr(unsigned *numArr, T *&numOffsetArr, unsigned numArrSize){
	numOffsetArr = new T[numArrSize];
	numOffsetArr[0] = 0;
	for(unsigned int i=1; i<numArrSize; i++)
		numOffsetArr[i] = numOffsetArr[i-1] + numArr[i-1];
}

std::string toString(int value);
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum, std::string ext);
double GetWallClockTime(void);


void copyTriangleIdsFromTriangleArr(unsigned long long *&triangleIdArr, unsigned long long triangleNum, triangle *triangleArr);
void copyTriangleCoorsFromTriangleArr(double *&triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr);
void copyTrianglesFromTriangleArr(unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr);
void copyTriangleIds(unsigned long long *&triangleIdArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr);
void copyTriangles(unsigned long long *&triangleIdArr, double *&triangleCoorArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr);


gridBound boundingGrid(boundingBox bBox, boundingBox domainBound, unsigned xPartNum, unsigned yPartNum);
gridElement mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY, boundingBox domainBound);
gridElement mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY, boundingBox domainBound);

boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);

void generateTriangleArr(unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr);
void generateOffsetArr(unsigned *intArr, unsigned intArrSize, unsigned *&intOffsetArr);
#endif
