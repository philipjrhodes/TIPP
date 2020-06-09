#include <string>
#include <list>
#include <set>

#include "io.h"
#include "common.h"
#include "partition.h"
#include "linkList.h"
#include "gridBound.h"
#include "boundingBox.h"
#include "triangulate.h"

struct nodeArr{
	triangle *triangleArr;
	unsigned long long triangleNum;
};

//A domain is a square with low and high coordinates
class domain{
public:
	//The coordinates of domain
	point lowPoint;
	point highPoint;

	//if domainSize=1, then the domain is square between 0,0 - 1,1
	//if domainSize=3, then the domain is square between 0,0 - 3,3
	unsigned int domainSize;

	boundingBox geoBound;

	//number of partitions on regarding x and y axes
	unsigned int xPartNum;
	unsigned int yPartNum;
	//
	unsigned int partNum;
	//current active partitions
	unsigned int currActivePartNum;

	//path to data point files
	std::string inputPath;
	std::string outputPath;

	//vertexRecordSize = 2 in 2D
	unsigned int vertexRecordSize;

	//number of point in each coarse partition
	unsigned int *pointCoarsePartNum;

	//when process delaunay, use variable triangleList to store triangles in and out
	triangleNode *triangleList;

	//when delaunay process is done, the triangleList will be transformed into array for parallel process with openMP
	triangle *triangleArr;
	unsigned long long triangleArrSize;

	//trianglePartitionList is an array of linklist, each element (triangle) of array contains a
	//linklist of partitionIds. 
	std::list<unsigned int> *trianglePartitionList;

	//array of partitions
	partition *partArr;

	//conflictPartList is an array of linklist, used to contain all conflictions of each partition in domain
	std::list<unsigned int> *conflictPartList;

	//currActiveList contains partition Ids that are active
	std::list<unsigned int> currActiveList;
	//contain a set og partition Ids
	std::set<unsigned int> activePartSet;
	//store all active partition Ids for a stage
	//used to collect all triangles in file boundaryTrianglesXX.tri and storedTriangleIdsXX.tri in updateTriangle()
	unsigned int *activePartArr;
	unsigned int activePartArrSize;

	//contains initial points for the initial delaunay
	point *initPointArr;
	unsigned long long initPointArrSize;

	//total number of points in domain and grid points (not include 4 corner points of square domain)
	unsigned long long pointNumMax;
	//total number of points in domain
	unsigned long long pointNum;

	domain(double lowX, double lowY, double highX, double highY, std::string srcPath, std::string dstPath);
	domain(double lowX, double lowY, double highX, double highY);
	domain(){}
	~domain();

	double getDomainSizeX();
	double getDomainSizeY();

	//void generateBoundingGridPoints();
	void readPointPartFileInfo();
	void loadInitPoints();
	void initTriangulate();
	void initTriangulateAdvance();

	//transform link list of triangles (triangleList) into array of triangle (triangleArr)
	void triangleTransform();

	void updateEmptyPartitions();

	//generate all intersections between each triangle and partitions in the domain.
	void generateIntersection();

	//generate confliction for each partition. 
	void generateConflictPartitions();

	void printConflictPartitions();

	//generate active partitions in the domain, return number of active partitions
	unsigned int generateActivePartitions();

	//after picking out partitions for processing, we need to update the confliced partitions in conflictPartList
	void updateConflictPartitions();

	//deliver triangles to active partitions
	void deliverTriangles();

	//number of active partition left over in activePartSet
	unsigned int activePartitionNumber();

	//=========================================================================================
	//Extract trangles in all active partitions and send to slave nodes for further Dalaunay Triangulation
	//coreNum is the number of cores available in MPI system or PBS
	//However, each time, send only coreNum of active partitions to slave  nodes
	//Assume if total number of active partitions is 20, but number of core available is 6,
	//then each time send job to MPI, we only sent 6 tasks, list of sending : 6, 6, 6, 2
	//Output is the total time to run MPI
	unsigned int prepareDataForDelaunayMPI(unsigned int processNum, unsigned int *&activePartPointSizeArr);
	void storeActivePartitions(unsigned int *activePartIdArr, unsigned int activePartNum);
	void storeActivePartitionInfo(unsigned int activePartNum, unsigned int *activePartIdArr, unsigned int *activePartSizeArr);

	//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
	//update to the main triangleArr, and ready for the next stages of triangulation
	void updateTriangleArr();

	//return number of partitions that are unfinished
	unsigned int unfinishedPartNum();

	unsigned int unDeliveredTriangleNum();

	void printTriangleArray();

	//When all partitions are processed via MPI, we need to store all trianlges left
	void storeAllTriangles();

	//find xFinePartNum and yFinePartNum
	unsigned int countFinePartitions();

};
