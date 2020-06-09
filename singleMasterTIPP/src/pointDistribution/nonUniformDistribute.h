//#include "common.h"
#include "io.h"
#include "boundingBox.h"
//#include "point.h"
#include <string>
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
	double	*mapPointData;
	unsigned int *scalePartInfoArr;
	unsigned int mapPointNum;
	unsigned int initPointNum;

	unsigned long long totalInputPointNum;

	point domainLowPoint;
	point domainHighPoint;

	double *pointCoorArr;//x1 y1 x2 y2,...xn,yn
	point *gridPoints;
	unsigned int gridPointsSize;

	double *pointGridArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
	unsigned int pointGridArrSize;
	unsigned long long globalPointIndex;

	//Number of partitions on x and y axes
	unsigned int xPartNum;/*number of partition on row axes*/
	unsigned int yPartNum;/*number of partition on column axes*/
	double scale;//a number used to scale the number of points in the map
//	unsigned int scalePartX;/*number of columns for scaling*/
//	unsigned int scalePartY;/*number of rows for scaling*/
	//source path and destination path
	std::string destPath;

	std::string dataMapFile;
	
	//pointPartInfo stores number of points for each partition
	unsigned int *pointPartInfoArr;

	//linklist of coordinates (x1,y1, x2,y2,..). 
	std::list<point> *partList;
	
	//input a point and a patition size, 
	//output partition index where the point belongs to 
	unsigned int partIndex(point p, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	void writePointCoorArr(double *pointCoorArr, unsigned int pointCoorArrSize, std::string fileStr);
	void writePointPartArr(point *pointPartArr, unsigned int pointPartArrSize, std::string fileStr);
	void writeToBinaryFile(std::string fileStr, point *data, unsigned int size);
	void writeListToFile(std::list<point> pointList, std::string fileStr);
	void readPointArr(point *pointArr, unsigned int pointArrSize, std::string fileStr);

	void readMapData();
	void scaleMapData();
	void readCoorPart(double *&pointCoorArr, unsigned int &pointNum);
	void generatePoints(boundingBox partBox, double *&pointCoorArr, unsigned int &pointPartNum);
	void pointsDistribute();
	void sortUpdate();
	//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
	void addGridPointsToInitPoints(double stepInterval);
	void createFullPointCoorData();
	void testData();

	void printPointPartitions(unsigned int partitionId);
	void info();

	distribute(unsigned int xPartNumber, unsigned int yPartNumber, unsigned long long totalNumberPoints, unsigned int initialPointNum, std::string resultPath);
	~distribute();
};
