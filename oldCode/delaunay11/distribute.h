#ifdef OPENMP
# include <omp.h>
#endif

#include "domain.h"
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
public:
	double *pointCoorArr;//x1 y1 x2 y2,...xn,yn
	point *gridPoints;

	double *pointGridArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
	int pointGridArrSize;
	unsigned int globalPointIndex;

	//Each partition contributes some points in order to have initPointArr
	point *initPointArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
	int initSize;//the number of points that each partition contributes
	int initPointArrSize;//initPointArrSize = intSize x number of partitions
	int *initPointInfoArr;//the size of each init points that is contributed from each partition

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=3, then the domain is square between 0,0 - 3,3
	double domainSize;

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	double interval;

	//Number of partitions on x and y axes
	int xPartNum;/*number of partition on row axes*/
	int yPartNum;/*number of partition on column axes*/

	//partition Sizes
	double xPartSize;
	double yPartSize;

	//source path and destination path
	std::string srcPath;
	std::string destPath;

	std::string vertexInfofilename;
	std::string vertexfilename;
	
	//=2 because 2D (x,y)
	int vertexRecordSize;

	//number of vertices in domain, it can be divided into k chunkSizes
	unsigned long long vertexRecordCount;

	//the size to read in peacewise fashion
	unsigned long long chunkSize;

	//partList is an array of linklist, each element (partition) of array contains 
	//linklist of coordinates (x1,y1, x2,y2,..). 
	std::list<point> *partList;

	//pointPartInfo stores number of points for each partition
	unsigned int *pointPartInfoArr;

	

	//input a point and a patition size, 
	//output partition index where the point belongs to 
	//int partIndex1(point p);
	int partIndex(point p);

	//process points partitioning, classify points into domain partitions (ex: 4x4)
//	void pointsDistribute1();

	//process points partitioning, classify points into slides (1 row, many columns)
	void pointsDistribute();

	//Write a list to a binary file
	void writeListToFile(std::list<point> pointList, int partitionId);
	void writeToBinaryFile(std::string fileStr, int partitionId, point *data, unsigned int size);

	//print points in partitions
	void printPointPartitions(int partitionId);
	void printPointPartitions1(int partitionId);

	//print points for all partitions
	void printAllPartitions();
	void printInitPointsAndPartitions();

	//sort points based on coordinate x
	//sort all points in all partition, then add them to one main file: pointPart.ver
	void sortAdd();
	void appendTwoFiles(point * pointArr, unsigned int pointNum);

	//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
	void addGridPointsToInitPoints();

	//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	void createFullPointCoorData();

	distribute(double domainsize, double distance, int xPartNumber, int yPartNumber, std::string sourcePath, std::string verInfoFile, std::string verFile, std::string resultPath, unsigned long long chunksize, int initsize);
	~distribute();
};
