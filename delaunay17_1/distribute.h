#ifdef OPENMP
# include <omp.h>
#endif

#include "domain.h"
#include <string>
#include <cstdlib>
#include <list>

/*
class distribute read a point cloud from file and distribute each point to one of the partitions in the domain
For example: the point A (.2, .7) belong to partition 9 of domain (0,0)-(1,1) which
is divided 4x4 = 16 parititons 
*/
class distribute{
public:
	point *gridPoints;

	unsigned int pointGridArrSize;
	unsigned long long globalPointIndex;

	unsigned int initFineSize;//the number of points that collected from each fine-grained partition
	//the number of points that collected from each coarse-grained partition
	//later, each coarse-grained partition contributes initCoarseSize points to the init points for whole domain (1st level)
	unsigned int initCoarseSize;


	std::list<point> **initPointArr;//x1 y1 x2 y2,...xn,yn, points that are collected from fine-grained partitions
	unsigned int initPointArrSize;//initPointArrSize = intSize x number of partitions
	unsigned int **initPointInfoArr;//the size of each init points that is contributed from each fine-grained partition

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=3, then the domain is square between 0,0 - 3,3
	double domainSize;
	unsigned int nodeNum;//number of nodes in cluster
	point lowDomainPoint;
	point highDomainPoint;

	//coarse-grained and fine-grained partition numbers
	unsigned int xCoarsePartNum;/*number of partition on row axes for the coarse-grained partitions*/
	unsigned int yCoarsePartNum;/*number of partition on column axes for the coarse-grained partitions*/
	unsigned int xFinePartNum;/*number of partition on row axes for the fine-grained partitions*/
	unsigned int yFinePartNum;/*number of partition on column axes for the fine-grained partitions*/

	//source path and destination path
	std::string dataName, path;

	std::string vertexInfofilename;
	std::string vertexfilename;
	
	//=2 because 2D (x,y)
	int vertexRecordSize;

	//number of vertices in domain, it can be divided into k chunkSizes
	unsigned long long vertexRecordCount;

	//the size to read in peacewise fashion
	unsigned long long chunkSize;

	//partList is an 2D array of linklist, each element (fine-grained partition) of array contains a linklist of points.
	//2D array is the structure of coarse-grained and fine-grained partitions of the domain
	std::list<point> **partPointList;

	//pointPartInfo stores number of points for each fine-grained partition
	//2D array is the structure of coarse-grained and fine-grained partitions of the domain
	unsigned long int **pointPartInfoArr;



	//---------------------------------------------------------------------
	//input a point and a patition size, 
	//output partition index where the point belongs to 
	int partIndex(point p, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum);

	//process points partitioning, classify points into slides (1 row, many columns)
	void processDistribution();
	void processInitPoints();
	void createInfoFiles();
	void pointsDistribute(double *pointCoorArr, unsigned long long readingSize);

	//Write a list to a binary file
	void writeListToFile(std::list<point> pointList, unsigned int coarsePartId, unsigned int finePartId);
	void writeToBinaryFile(std::string fileStr, unsigned int coarsePartId, unsigned int finePartId, point *data, unsigned int size);

	//print points in partitions
	void printPointPart(unsigned int coarsePartId, unsigned int finePartId);
	void printAllPartitions();

	//sort points based on coordinate x
	//sort all points in all partition, then add them to one main file: pointPart.ver
	void sortUpdate();

	//append grid points, (not including 4 points of domain square (0,0), (0,1), (1,1), (1,0)) to initPoints.ver 
	void addGridPointsToInitPoints();

	//copy mydatabin.ver from rawPointData to fullPointPart.ver in delaunatResults
	//Append grid points  (include 4 corner points) to the end of fullPointPart.ver
	void createFullPointCoorData();

	//move files in /data0 to other share folders (/data1, /data2, ... /dataN, N is number of nodes in cluater)
	void distributeFiles();

	distribute(double domainsize, unsigned int numberOfNodes, unsigned int xCoarsePartNumber, unsigned int yCoarsePartNumber, unsigned int xFinePartNumber, unsigned int yFinePartNumber, std::string dataNameStr, std::string verInfoFile, std::string verFile, unsigned long long chunksize, unsigned int initcoarsesize, unsigned int initfinesize);
	~distribute();
};
