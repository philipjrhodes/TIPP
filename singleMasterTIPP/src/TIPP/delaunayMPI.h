#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>

//#include "linkList.h"
#include "triangulate.h"

#define MASTER_RANK 0
#define send_data_tag 2001
#define return_data_tag 2002


class delaunayMPI{
public:
	std::string inputPath;
	std::string outputPath;

	//domain sizes
	unsigned int xPartNum, yPartNum;

	bool shareFolderOption;

	//active partitions
	unsigned int *partArr;
	//number of active partitions
	unsigned int partNum;

	double *tempCoorArr;//all coordinates of triangles by sender
	unsigned long long *tempPointIdArr;//all point id of triangles by sender

	//all point coordinates of partitions
	point *pointArr;

	unsigned long long totalTriangleSize;//number of triangles of all active partitions
	unsigned int *triangleSizeArr;//number of triangle belong to each partition
	unsigned int *triangleSizeOffsetArr;


	//all partitions in which each partition contains the number of points
	unsigned int *pointNumPartArr;

	//number of points and offset for each active partition
	unsigned int pointNumPartSize;
	unsigned int pointNumPartOffsetSize;

	MPI_Status status;

	//point coordinate file for all partitions
	MPI_File fh;

	//list of triangles that are currently processed delaunay
	triangleNode *initialTriangleList;
	//list of triangle Ids that are not currently processed delaunay, circumcircles are inside partition
	triangleNode *interiorTriangleList;
	triangleNode *boundaryTriangleList;

	delaunayMPI(double dSize, bool shareFolderOptionInput, std::string srcPath, std::string dstPath);

	//read all triangles for all active partitions
	void readTriangleData(int pool_size);

	//based on data received from master, each slave generate all initial triangles for the partitions
	void generateInitTriangles(int my_rank, unsigned long long triangleNum, double *coorArr, unsigned long long *pointIdArr);

	//read point coordinates from pointPart.ver
	void readPointCoor(int my_rank, unsigned int partId);
	void readCoorPointInActivePartitions(double *&pointCoorArr, unsigned long long *&pointIdArr);
	void scatterPointData(int my_rank, int pool_size, unsigned int partId);

	void triangulate(unsigned int partId, int my_rank);
	boundingBox partBox(unsigned int partId);
	void processInteriorTriangles(int my_rank, unsigned partId, unsigned int pool_size);
	void processDirectInteriorTriangles(int my_rank, unsigned partId, unsigned int pool_size);
	void processBoundaryTriangles(int my_rank, unsigned int pool_size);
	void processMPI(int my_rank, unsigned int pool_size, double &masterTime, double &readTime, double &storeTime);

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=3, then the domain is square between 0,0 - 3,3
	double domainSize;
};


