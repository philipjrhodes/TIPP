#include "common.h"
#include "boundingBox.h"
#include "point.h"
#include <string>
#include <stdlib.h>
#include <cstdlib>
#include <list>

/*
class distribute read a point cloud from file and distribute each point to 
one of the partitions in the domain
For example: the point A (.2, .7) belong to partition 9 of domain (0,0)-(1,1) which
is divided 4x4 = 16 parititons 
*/
class distribute{
private:
	double drand(const double min, const double max);
public:

	point domainLowPoint;
	point domainHighPoint;

	//points in 0,0 - 1,1 before collecting init points
	double *basicPointPartCoorArr;//x1 y1 x2 y2,...xn,yn
	//number of points in basic partition before collecting init points
	unsigned int basicPointPartNum;
	//number of points in basic partition after collecting init points
	unsigned int pointPartNum;

	unsigned int initPointNum;

	point *gridPoints;
	unsigned int gridPointsSize;

//	double *pointGridArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
//	unsigned int pointGridArrSize;
	unsigned long long globalPointIndex;

	//Number of partitions on x and y axes
	unsigned int xPartNum;/*number of partition on row axes*/
	unsigned int yPartNum;/*number of partition on column axes*/

	unsigned int domainSize;
	std::string path;
	unsigned long long totalInputPointNum;

	//input a point and a patition size, 
	//output partition index where the point belongs to 
	unsigned int partIndex(point p, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	void writePointCoorArr(double *pointCoorArr, unsigned int pointCoorArrSize, std::string fileStr);
	void writePointPartArr(point *pointPartArr, unsigned int pointPartArrSize, std::string fileStr);
	void writePointList(std::list<point> pointPartList, std::string fileStr);

	void readCoorPart(double *&pointCoorArr, unsigned int &pointNum);
	void generateBasicData(unsigned int &basicPointPartNum);
	void generateAllPartitionPoints();

	//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
	void addGridPointsToInitPoints();
	void createFullPointCoorData();

	void printPointPart(unsigned int partId);
	void printAllPartitions();
	void readFullPointPartData();
	void testData();
	void info();

	distribute(unsigned int xPartNumber, unsigned int yPartNumber, unsigned long long totalNumberPoints, unsigned int initialPointNum, std::string fullPath);
	~distribute();
};
