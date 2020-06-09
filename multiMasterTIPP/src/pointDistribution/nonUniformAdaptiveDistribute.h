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
	double	*mapPointData;
	unsigned int mapPointNum;
	unsigned int initCoarsePointNum;
	unsigned int initFinePointNum;
	unsigned int initPointNum;

	//pointPartInfo stores number of points for each fine-grained partition
	//2D array is the structure of coarse-grained and fine-grained partitions of the domain
	unsigned int **pointPartInfoArr;
	unsigned int **originalPointPartInfoArr;

	unsigned int *pointCoarsePartInfoArr;//number of points in each coarse partition
	unsigned int *rowFinePartNumArr;//number of row/col partitions in each coarse partition

	point domainLowPoint;
	point domainHighPoint;

	std::list<point> initDomainPointList;
	std::list<point> *initPartPointList;


	double *pointCoorArr;//x1 y1 x2 y2,...xn,yn
	point *gridPoints;
	unsigned int gridPointsSize;

	double *pointGridArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
	unsigned int pointGridArrSize;
	unsigned long long globalPointIndex;

	//Each partition contributes some points in order to have initPointArr
	unsigned int *initPointInfoArr;//the size of each init points that is contributed from each partition



	//Number of partitions on x and y axes
	unsigned int xCoarsePartNum;/*number of partition on row axes in domain*/
	unsigned int yCoarsePartNum;/*number of partition on column axesin domain*/

	unsigned int pointNumThreshold;/*number of points in a fine partition*/
	unsigned long long totalInputPointNum;

	double scale;//a number used to scale the number of points in the map

	//source path and destination path
	std::string destPath;

	std::string dataMapFile;
	

	//input a point and a patition size, 
	//output partition index where the point belongs to 
	unsigned int partIndex(point p, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	void writePointCoorArr(double *pointCoorArr, unsigned int pointCoorArrSize, std::string fileStr);
	void writePointPartArr(point *pointPartArr, unsigned int pointPartArrSize, std::string fileStr);
	void writePointList(std::list<point> pointPartList, std::string fileStr);
	void appendPointList(std::list<point> pointPartList, std::string fileStr);

	void readMapData();
	unsigned int closeToQuare(unsigned int pointNum, unsigned int pointNumThreshold);
	void calculatePointCoarsePartNum();
	void calculatePointPartNum();
	void readCoorPart(double *&pointCoorArr, unsigned int &pointNum);
	void generatePoints(boundingBox partBox, double *&pointCoorArr, unsigned int &pointPartNum);
	void distributeCoarsePartition(unsigned int coarsePartId);
	void generatePointsAllCoarsePartitions();
	void generateScalePoints();
	void sortUpdate();
	//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
	void addGridPointsToInitPoints();
	void createFullPointCoorData();

	void printPointPart(unsigned int coarsePartId, unsigned int finePartId);
	void printAllPartitions();
	void readFullPointPartData();
	void testData();

	distribute(unsigned int xCoarsePartNumber, unsigned int yCoarsePartNumber, unsigned int pointNumberThreshold, unsigned long long totalNumberPoints, unsigned int initialCoarsePointNum, unsigned int initialFinePointNum, std::string resultPath);
	~distribute();
};
