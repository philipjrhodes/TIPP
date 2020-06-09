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
	std::string dataName;
	unsigned int nodeNum;

	//fine-grained partition sizes
	unsigned int xFinePartNum, yFinePartNum;

	double xFinePartSize, yFinePartSize;

	//coordinates of current coarse-grained partition
	double coarseLowX, coarseLowY, coarseHighX, coarseHighY;

	//active partitions
	unsigned int *finePartArr;
	//number of active partitions
	unsigned int finePartNum;

	double *tempCoorArr;//all coordinates of triangles by sender
	unsigned long long *tempPointIdArr;//all point id of triangles by sender

	//all point coordinates of partitions
	point *pointCoorArr;
//	unsigned long long *pointIdArr;

	unsigned int totalTriangleSize;//number of triangles of all active partitions
	unsigned int *triangleSizeArr;//number of triangle belong to each partition
	unsigned int *triangleSizeOffsetArr;


	//all partitions in which each partition contains the number of points
	unsigned int *pointNumPartArr;
	//compute offset for parallel reading file
//	unsigned long long *pointNumPartOffsetArr;
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


	delaunayMPI(std::string dataNameStr, unsigned int numberOfNodes);

	//read all triangles for all active partitions
	void readTriangleData(unsigned int pool_size, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum);

	//based on data received from master, each slave generate all initial triangles for the partitions
	void generateTriangles(unsigned int my_rank, unsigned int triangleNum, double *coorArr, unsigned long long *pointIdArr);

	//read point coordinates from pointPart.ver
	void readPointCoor(unsigned int my_rank, unsigned int finePartId, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum);

	void triangulate(unsigned int coarsePartId, unsigned int finePartId, unsigned int my_rank);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xFinePartNum, unsigned int yFinePartNum);
	void processStoreTriangles(unsigned int my_rank, unsigned int finePartId, unsigned int pool_size, MPI_Comm row_comm, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum);
	void processTriangleList(unsigned int my_rank, unsigned int pool_size, MPI_Comm row_comm, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum);
	void printTriangleList(unsigned int finePartId);
	void processMPI(unsigned int my_rank, unsigned int pool_size, MPI_Comm row_comm, unsigned int coarsePartId, unsigned int xCoarsePartNum, unsigned int yCoarsePartNum);

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;
};


