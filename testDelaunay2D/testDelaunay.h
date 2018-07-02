//read the triangulation results (triangleIds.tri and fullPointPart.tri) using Direct Load,
//then, callculate the areas of all triangles in parallel (MPI)

#include <mpi.h>
#include <iostream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

#define MASTER_RANK 0
#define tag 1000

using namespace std;

class testDelaunay{
private:
	void quit(std::string str);
	void minMax(unsigned long long *intArr, unsigned int intArrSize, unsigned long long *min, unsigned long long *max);
public:
	string path;
	unsigned int segmentSize;
	double domainSize;
	double totalTtriangleArea;

	//number of triangle for each read from triangleIds.tri
	unsigned int readingSize;
	//position of file reading
	unsigned long long readingPos;
	//total number of triangle in file triangleIds.tri
	unsigned long long totalTriangleNum;
	//number of triangle of workers received from master
	unsigned int triangleNum;

	//number of triangle coordinates from each read of master from triangleIds.tri
	double *triangleCoorArr;
	//number of triangles for each task in pool_size
	unsigned int *triangleNumArr;

	//number of triangle coordinates for each worker
	double *coorArr;


	testDelaunay(std::string pathInput, double domainSizeInput, unsigned int segmentSizeInput);
	~testDelaunay(){
		delete [] triangleCoorArr;
		delete [] triangleNumArr;
	}
	double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3);
	void readDirect(string dataFileStr, unsigned int firstPointer, unsigned int loadingSize, int dataRecordSize, double *dataArr);
	void updateBitmap(unsigned long long *pointIdArr, unsigned int pointIdArrSize, bool *bitmap, int bitmapSize, unsigned int memoryThreshold);
	void loadDirect(unsigned long long *pointIdArr, unsigned int pointIdArrSize, double *dataArr, unsigned long long memoryThreshold, std::string dataFileStr);
	void readDataFile(std::string triangleIdFileName, std::string triangleCoorFileName, unsigned long long readingPos, unsigned int *readingSize, unsigned long long totalTriangleNum, double *triangleCoorArr);
	void computeTriangleNumArray(unsigned int *triangleNumArr, unsigned int segmentSize, unsigned int readingSize, int pool_size);
	int readNum(std::string path, unsigned int segmentSize);
};


