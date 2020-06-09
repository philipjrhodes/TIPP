#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

#include "point.h"
#include "common.h"

//read files
#include <sys/types.h>
#include <dirent.h>


#ifndef IO_H
#define IO_H

void quit(std::string str);
void minMax(unsigned long long *intArr, unsigned int intArrSize, unsigned long long &min, unsigned long long &max);
void updateBitmap(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, bool *bitmap, int bitmapSize, unsigned long long memoryThreshold);
void readDirect(std::string dataFileStr, unsigned int firstPointer, unsigned long int loadingSize, int vertexRecordSize, double *dataArr);
void loadDirect(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, double *&triangleCoorArr, unsigned long long memoryThreshold, std::string dataFileStr);
void readDataFile(std::string triangleIdFileName, std::string triangleCoorFileName, unsigned long long readingPos, unsigned long int &readingSize, unsigned long long totalTriangleNum, unsigned long long *&triangleIdArr, double *&triangleCoorArr);
unsigned readNum(std::string path, std::string fileName, unsigned long int &segmentSize, unsigned long long &totalTriangleNum);
unsigned long long numberTriangles(std::string path, std::string fileName);
void appendTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string outputPath);
void writeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string outputPath);
void writePointCoordinates(double *pointCoorArr, unsigned pointNum, std::string outputPath);
void writePointAttributes(double *pointAttArr, unsigned pointNum, std::string outputPath);
void appendFile(std::string fileNameStr1, std::string fileNameStr2);
void appendFile1(std::string fileNameStr1, std::string fileNameStr2);
int getdir (std::string dir, std::list<std::string> &files);
void combineFiles(std::string dstPath);
void combineSubFiles(std::string dstPath, std::string fileNameStr);
void deleteFiles(std::string dstPath);
void exeCommand(std::string commandStr);
void readMeshInfo(std::string path, unsigned long long &totalTriangleNum, unsigned &domainSize, unsigned &threshold);

void storeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string fileStr, std::string optionStr);
void storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string fileStr, std::string optionStr);
void storePointCoorArr(double *pointCoorArr, unsigned int pointCoorArrSize, std::string fileStr, std::string optionStr);
void storePointPartArr(point *pointPartArr, unsigned int pointPartArrSize, std::string fileStr, std::string optionStr);
void readTriangleCoors(double *&triangleCoorArr, unsigned long long &triangleNum, std::string fileStr);
void readTriangleIds(unsigned long long *&triangleIdArr, unsigned long long &triangleNum, std::string fileStr);
void readPoints(point *&pointArr, unsigned pointNum, std::string fileStr);
void readPoints_NoAllocation(point *pointArr, unsigned pointNum, std::string fileStr);

void command(std::string commandStr);
void addFile(std::string outputPath, std::string fileName1, std::string fileName2);

//void storeActivePartitionInfo(unsigned int activePartNum, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr, unsigned xPartNum, unsigned yPartNum, std::string fileInfoStr);
#endif
