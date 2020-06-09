#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>
#include "common.h"
#include "linkList.h"

#define MASTER_RANK 0
#define send_data_tag 2001
#define return_data_tag 2002


class delaunayMPI{
public:
	std::string path;

	//domain sizes
	unsigned int xPartNum, yPartNum;

	//active partitions
	unsigned int *partArr;
	//number of active partitions
	unsigned int partNum;

	double *tempCoorArr;//all coordinates of triangles by sender
	unsigned long long *tempPointIdArr;//all point id of triangles by sender

	//all point coordinates of partitions
	point *pointCoorArr;
//	unsigned long long *pointIdArr;

	unsigned int totalTriangleSize;//number of triangles of all active partitions
	unsigned int *triangleSizeArr;//number of triangle belong to each partition
	unsigned long int *triangleSizeOffsetArr;


	//all partitions in which each partition contains the number of points
	unsigned int *pointNumPartArr;
	//compute offset for parallel reading file
	unsigned long long *pointNumPartOffsetArr;
	//number of points and offset for each active partition
	unsigned int pointNumPartSize;
	unsigned int pointNumPartOffsetSize;


	//list of triangles that are currently processed delaunay
	triangleNode *triangleList;
	//list of triangles that are not currently processed delaunay, will be processed by other partitions
	triangleNode *temporaryTriangleList;
	//list of triangle Ids that are not currently processed delaunay, circumcircles are inside partition
	triangleNode *storeTriangleList;

	MPI_Status status;

	//point coordinate file for all partitions
	MPI_File fh;


	delaunayMPI(double dSize, std::string srcPath);

	//read all triangles for all active partitions
	void readTriangleData(int pool_size);

	//based on data received from master, each slave generate all initial triangles for the partitions
	void generateTriangles(int my_rank, int triangleNum, double *coorArr, unsigned long long *pointIdArr);

	//read point coordinates from pointPart.ver
	void readPointCoor(int my_rank, int partId);

	void triangulate(unsigned int partId, int my_rank);
	boundingBox partBox(int partId);
	void storeTriangleIds(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, std::string fileStr);
	double processStoreTriangles(int my_rank, int pool_size);
	void storeTriangleCoors(double *triangleCoorArr, unsigned int triangleCoorArrSize, std::string fileStr);
	double processTriangleList(int my_rank, int pool_size);
	void printTriangleList(int partId);
	double processMPI(int my_rank, int pool_size);

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=3, then the domain is square between 0,0 - 3,3
	double domainSize;
};


