#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>
#include "common.h"
#include "io.h"
#include "triangulate.h"
//#include "linkList.h"

#define MASTER_RANK 0
#define send_data_tag 2001
#define return_data_tag 2002
#define send_data_tag1 2003


class delaunayMPI{
public:
	std::string inputPath, outputPath;

	unsigned int my_rank;
	unsigned int finePartId;
	unsigned int pool_size;
	MPI_Comm row_comm;
	unsigned int coarsePartId;
	unsigned int xCoarsePartNum;
	unsigned int yCoarsePartNum;

	//fine-grained partition sizes
	unsigned int xFinePartNum, yFinePartNum;

	double xFinePartSize, yFinePartSize;

	//coordinates of current coarse-grained partition
	double coarseLowX, coarseLowY, coarseHighX, coarseHighY;

	//active partitions
	unsigned int *finePartArr;
	//number of active partitions
	unsigned int finePartNum;

	//all point coordinates of partitions
	point *pointCoorArr;

	unsigned long long totalTriangleSize;//number of triangles of all active partitions

	//all partitions in which each partition contains the number of points
	unsigned int *pointNumPartArr;

	//number of points and offset for each active partition
	unsigned int pointNumPartSize;
	unsigned int pointNumPartOffsetSize;


	//list of triangles that are currently processed delaunay (initial)
	triangleNode *triangleList;
	//list of triangles that are not currently processed delaunay, will be processed by other partitions
	triangleNode *boundaryTriangleList;
	//list of triangle Ids that are not currently processed delaunay, circumcircles are inside partition
	triangleNode *interiorTriangleList;

	MPI_Status status;

	//point coordinate file for all partitions
	MPI_File fh;

	delaunayMPI(unsigned int myrank, unsigned int poolsize, MPI_Comm rowcomm, unsigned int coarsePartitionId, unsigned int xCoarsePartionNum, unsigned int yCoarsePartionNum, std::string srcPath, std::string dstPath);

	//read all triangles for all active partitions
	void readTriangleData();

	//based on data received from master, each slave generate all initial triangles for the partitions
	void generateTriangles(unsigned long long triangleNum, double *coorArr, unsigned long long *pointIdArr);

	//read point coordinates from pointPart.ver
	void readPointCoor(unsigned int finePartId);

	void triangulate(unsigned int finePartId);

	void processInteriorTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize);
	void processDirectInteriorTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize);

	void processBoundaryTriangles(unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleArrSize);

	void processMPI(double lowX, double lowY, double highX, double highY, unsigned int xPartNum, unsigned int yPartNum, unsigned long long *inputPointIdArr, double *inputCoorArr, unsigned long long inputTriangleSize, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr, unsigned int *pointNumPartitionArr, unsigned int &currActivePartNum, unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleIdArrSize);

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;
};


