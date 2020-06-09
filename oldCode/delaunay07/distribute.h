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
	float *pointCoorArr;//x1 y1 x2 y2,...xn,yn

	float *pointGridArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
	int pointGridArrSize;

	//Each partition contributes some points in order to have initPointArr
	point *initPointArr;//x1 y1 x2 y2,...xn,yn, these points are grid points inside domain
	int initSize;//the number of points that each partition contributes
	int initPointArrSize;//initPointArrSize = intSize x number of partitions
	int *initPointInfoArr;//the size of each init points that is contributed from each partition

	bool *pointDuplicatedArr;//store infor about the grid points are duplicated with original points

	//Number of partitions on x and y axes
	int xPartNum;/*number of partition on row axes*/
	int yPartNum;/*number of partition on column axes*/

	//partition Sizes
	float xPartSize;
	float yPartSize;

	//source path and destination path
	std::string srcPath;
	std::string destPath;

	std::string vertexInfofilename;
	std::string vertexfilename;
	
	//=2 because 2D (x,y)
	int vertexRecordSize;

	//number of vertices in domain, it can be divided into k chunkSizes
	unsigned int vertexRecordCount;

	//the size to read in peacewise fashion
	unsigned int chunkSize;

	//partList is an array of linklist, each element (partition) of array contains 
	//linklist of coordinates (x1,y1, x2,y2,..). 
	std::list<float> *partList;

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
	void writeListToFile(std::list<float> pointList, int partitionId);
	void writeToBinaryFile(std::string fileStr, int partitionId, float *data, unsigned int size);

	//print points in partitions
	void printPointPartitions(int partitionId);

	//print points for all partitions
	void printAllPartitions();
	void printInitPointsAndPartitions();

	//sort points based on coordinate x
	//sort all points in all partition, then add them to one main file: pointPart.ver
	void sort();

	//These points are not belong to original data, they are generated follow the grid of xPartNum x 		yPartNum, and locaed inside doamin, not on square border
	void generateGridPoints();

	//check gridPoints to make sure not duplicate with original data.
	void checkDuplication();

	//merge all partition files into one files
	void mergeAllPartitions();

	//merge all partition files and init points and 4 point of domain square (0,0), (0,1), (1,1), (1,0) into one files
	void mergeAllPartitionsAndInitPoints();

	distribute(int xPartNumber, int yPartNumber, std::string sourcePath, std::string verInfoFile, std::string verFile, std::string resultPath, unsigned int chunksize, int initsize);
	~distribute();
};
