#include <string>
#include <list>
#include <set>
#include "partition.h"
#include "linkList.h"
#include "gridBound.h"
#include "boundingBox.h"

/* A coarsePartition is a square with low and high coordinates
*/
class coarsePartition{
private:
	unsigned int partIndex(point p);
	unsigned phaseCount;//use phaseCount to control active partitions in the first phase
public:

	//world processes in MPI are divided into multiple groups
	unsigned int groupId;

	//path to data point files
	std::string path;

	//vertexRecordSize = 2 in 2D
	unsigned int vertexRecordSize;

	//The coordinates of current coarse partition
	point lowPoint;
	point highPoint;
	boundingBox geoBound;
	//unit grid size = coarse side size / number of side partitiions
	double xGridCoarsePartSize;
	double yGridCoarsePartSize;

	//partition Id of a partition in coarsePartition
	unsigned int coarsePartId;

	//number of active coarse partitions
	unsigned int activeCoarsePartNum;

	//number of init triangles of current coarse partition
	unsigned long long initTriangleNum;

	//partition size of coarse- and fine-grined partitions
	unsigned int xCoarsePartNum;
	unsigned int yCoarsePartNum;
	unsigned int xFinePartNum;
	unsigned int yFinePartNum;
	//
	unsigned int partNum;



	//when process delaunay, use variable triangleList to store triangles in and out
	triangleNode *triangleList;

	//triangles that will not enclose the new points will be set aside to temporaryTriangleList
	triangleNode *temporaryTriangleList;

	//when delaunay process is done, the triangleList will be transformed into array for parallel process with openMP
	triangle *triangleArr;
	unsigned long long triangleArrSize;

	//trianglePartitionList is an array of linklist, each element (triangle) of array contains a
	//linklist of partitionIds. 
	std::list<unsigned int> *trianglePartitionList;

	//array of partitions
	partition *partArr;

	//conflictPartList is an array of linklist, used to contain all conflictions of each partition in coarsePartition
	std::list<unsigned int> *conflictPartList;

	//currActiveList contains partition Ids that are active
	std::list<unsigned int> currActiveList;
	//contain a set og partition Ids
	std::set<unsigned int> activePartSet;

	//pointIdArr contains start indices for points in each partition
//	unsigned long int *pointIdArr;


	//pointPartInfoArr contains sizes (number of points) for each partitions
	unsigned int *pointPartInfoArr;

	//contains initial points for the initial delaunay
	point *initPointArr;
	unsigned long long initPointArrSize;

	//total number of points in coarsePartition and grid points (not include 4 corner points of square coarsePartition)
	unsigned long long pointNumMax;
	//total number of points in coarsePartition
	unsigned long long pointNum;


	coarsePartition(unsigned int color, unsigned int partId, unsigned int activePartNum, unsigned long long initTriangleSize, unsigned int xPartNum, unsigned int yPartNum, std::string pathStr);
	~coarsePartition();

	point getLowPoint();
	point getHighPoint();

	void setLowPoint(point p);
	void setHighPoint(point p);

	double getcoarsePartitionSizeX();
	double getcoarsePartitionSizeY();


	/** Input: a bounding box of an element
	 output: a bounging grid that intersects with the element bounding box */
	 gridBound boundingGrid(boundingBox bBox);

	/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 	 partition with the higher index. Ex: 2.5/2 = 2 --> column/row 0, 1, 2; 2.0/2 = 1 
	geoBound is is the coarsePartition bound*/
	 gridElement mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY);

	/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
	 partition with the lower index. 2.5/2 = 2 --> column/row 0, 1, 2; 4.0/2 = 2, need to be subtract by 1 --> 1
	 if point p fall on the grid line (partition line) --> take lower element.*/
	 gridElement mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY);

	void readPointPartInfo();
//	void readInitTriangleInfo();
	void loadInitTriangles();
	void loadInitPoints();
	void basicTriangulate(point p);
	void initTriangulate();

	void readTriangleCoors(double *&triangleCoorArr, unsigned long long &triangleNum, std::string fullPath);
	void readTriangleIds(unsigned long long *&triangleIdArr, unsigned long long &triangleNum, std::string fullPath);
	void addFile(std::string fullFileName1, std::string fullFileName2);
	void removeFile(std::string fullPath);

	//transform link list of triangles (triangleList) into array of triangle (triangleArr)
	void triangleTransform();

	//generate all intersections between each triangle and partitions in the coarsePartition.
	void generateIntersection();

	//generate confliction for each partition. 
	void generateConflictPartitions();

	void printConflictPartitions();

	//generate active partitions in the coarsePartition, return number of active partitions
	unsigned int generateActivePartitions();

	//after picking out partitions for processing, we need to update the confliced partitions in conflictPartList
	void updateConflictPartitions();

	//deliver triangles to active partitions
	void deliverTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnBoundaryTriangleIdArr, double *&returnBoundaryTriangleCoorArr, unsigned long long &returnBoundaryTriangleArrSize);

	//number of active partition left over in activePartSet
	unsigned int activePartitionNumber();

	//=========================================================================================
	//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
	//coreNum is the number of cores available in MPI system or PBS
	//However, each time, send only coreNum of active partitions to slave  nodes
	//Assume if total number of active partitions is 20, but number of core available is 6,
	//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
	//Output is the total time to run MPI
	void prepareDataForDelaunayMPI(unsigned int coreNum, unsigned long long *&tempPointIdArr, double *&tempCoorArr, unsigned long long &totalTriangleSize, unsigned int *&activePartIdArr, unsigned int *&activePartSizeArr, unsigned int *&activePartSizeOffsetArr, unsigned int *&pointNumPartArr, unsigned int &currActivePartNum);
	void storeActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum, unsigned long long *&tempPointIdArr, double *&tempCoorArr, unsigned long long totalTriangleSize);
	void storeActivePartitionInfo(unsigned int activePartNum, unsigned int *activePartIdArr, unsigned int *activePartSizeArr, unsigned int *activePartSizeOffsetArr);

	//add return_triangles from MPI to returnAllTriangleIds.tri and returnAllTriangleCoors.tri
	//and merge to current triangleArr in memory
	void addReturnTriangles();


	//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
	//update to the main triangleArr, and ready for the next stages of triangulation
	void updateTriangleArr(unsigned long long *returnTriangleIdArr, double *returnTriangleCoorArr, unsigned long long returnTriangleIdArrSize);

	//return number of partitions that are unfinished
	unsigned int unfinishedPartNum();

	unsigned int unDeliveredTriangleNum();

	void printTriangleArray();

	//When all partitions are processed via MPI, we need to store all trianlges left
	void storeAllTriangles(unsigned long long *&returnStoreTriangleIdArr, unsigned long long &returnStoreTriangleIdArrSize, unsigned long long *&returnBoundaryTriangleIdArr, double *&returnBoundaryTriangleCoorArr, unsigned long long &returnBoundaryTriangleArrSize);
};
