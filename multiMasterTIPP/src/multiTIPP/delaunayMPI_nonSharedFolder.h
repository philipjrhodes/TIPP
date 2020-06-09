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
#define send_data_tag1 2003


class delaunayMPI{
public:
	std::string path;

	unsigned int my_rank;
	unsigned int finePartId;
	unsigned int pool_size;
	MPI_Comm row_comm;
	unsigned int coarsePartId;
	unsigned int xCoarsePartNum;
	unsigned int yCoarsePartNum;

	double *allPointCoorArr;//all points coordinate in current coarse partition
	unsigned long long *allPointIdArr;//all point Ids in current coarse partition
	unsigned *pointNumArr;//number of points in each fine partition in curretn coarse partition
	unsigned allFineParNum;//number of fine partition in curretn coarse partition

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

	unsigned long long totalTriangleSize;//number of triangles of all active partitions
	unsigned int *triangleSizeArr;//number of triangle belong to each partition
	unsigned int *triangleSizeOffsetArr;


	//all partitions in which each partition contains the number of points
	unsigned int *pointNumPartArr;

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


	delaunayMPI(unsigned int myrank, unsigned int poolsize, MPI_Comm rowcomm, unsigned int coarsePartitionId, unsigned int xCoarsePartionNum, unsigned int yCoarsePartionNum, double *pointCoorArr_submaster, unsigned long long *pointIdArr_submaster, unsigned *pointNumberArr, unsigned chunkNum);

	//based on data received from master, each slave generate all initial triangles for the partitions
	void generateTriangles(unsigned long long triangleNum, double *coorArr, unsigned long long *pointIdArr);

	//workers processes collect point coordinates from sub-masters 
	void collectPointCoor(unsigned int finePartId);

	//Scatter data point from each sub_master (pointCoorArr_subMaster)
	void scatterPointCoor(double *pointCoorArr_subMaster, unsigned int *pointNumPartArr);

	void triangulate(unsigned int finePartId);
	boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xFinePartNum, unsigned int yFinePartNum);
	void storeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string currPath);

	void processStoreTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize);

	void storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string fileStr);
	void processTriangleList(unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleArrSize);
	void printTriangleList(unsigned int finePartId);

	void processMPI(double lowX, double lowY, double highX, double highY, unsigned int xPartNum, unsigned int yPartNum, unsigned long long *inputPointIdArr, double *inputCoorArr, unsigned long long inputTriangleSize, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr, unsigned int *pointNumPartitionArr, unsigned int &currActivePartNum, unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnTriangleIdArr, double *&returnTriangleCoorArr, unsigned long long &returnTriangleIdArrSize);

	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;
	
	void testData(unsigned coarsePartId, unsigned finePartId, point *pointCoorArr, unsigned int pointNumPartSize);
	~delaunayMPI();
};


