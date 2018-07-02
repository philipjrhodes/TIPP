#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include "linkList.h"
//#include "boundingBox.h"

//read active partitions (tempCoor.tri, tempPointId.tri, and tempTriangles.xfdl)
//triangulate each partition, store results to returnTriangleIds.tri, returnTriangleCoors.
class delaunayLocal{
public:
	std::string path;
	int vertexRecordSize;

	//domain sizes
	int xPartNum, yPartNum;

	//active partitions
	int *partArr;
	//number of active partitions
	int partNum;

	double *tempCoorArr;//all coordinates of triangles by sender
	unsigned long int *tempPointIdArr;//all point id of triangles by sender

	//store all startId points for each partition
	unsigned long int *startIdPartArr;

	//all new point coordinates of partitions
	double *pointCoorArr;


	int totalTriangleSize;//number of triangles of all active partitions
	int *triangleSizeArr;//number of triangle belong to each partition
	int *triangleSizeOffsetArr;


	//all partitions in which each partition contains the number of points
	int *pointNumPartArr;
	//compute offset for parallel reading file
	int *pointNumPartOffsetArr;

	//number of points and offset for each active partition
	//int pointNumPartSize;
	int pointNumPartOffsetSize;

	//list of triangles that are currently processed delaunay
	triangleNode *triangleList;

	//list of triangles that are not currently delaunay processed, will be processed by other partitions
	triangleNode *temporaryTriangleList;

	//array of storeTriangleLists, each storeTriangleList belongs to a paritition
	triangleNode *storeTriangleList;


	delaunayLocal(std::string pathStr);

	//read all triangles for all active partitions
	void readTriangleData();

	//based on data received from master, each slave generate all initial triangles for the partitions
	void generateTriangles(unsigned int index);

	//read point coordinates from pointPart.ver
	void readPointCoor(unsigned int index);

	void triangulate(unsigned int index);
	boundingBox partBox(unsigned int partId);

	//triangulate all active partitions sequentially
	void partTriangulate();

	void processStoreTriangles();
	void processTriangleList();
	void printTriangleList(unsigned int partId);
	~delaunayLocal();
};
