#ifdef OPENMP
# include <omp.h>
#endif

#include <string>
#include <list>
#include <set>
#include "partition.h"
#include "drawMesh.h"
#include "gridBound.h"
#include "boundingBox.h"

/* A domain is a square with low and high coordinates
*/
class domain{
private:
	int partIndex(point p);
public:
	//The coordinates of domain
	point lowPoint;
	point highPoint;

	boundingBox geoBound;

	//number of partitions on regarding x and y axes
	int xPartNum;
	int yPartNum;
	//
	unsigned int partNum;

	//path to data point files
	std::string path;

	//vertexRecordSize = 2 in 2D
	int vertexRecordSize;

	//when process delaunay, use variable triangleList to store triangles in and out
	triangleNode *triangleList;

	//when delaunay process is done, the triangleList will be transformed into array for parallel process with openMP
	triangle *triangleArr;
	unsigned int triangleArrSize;

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
	std::set<int> activePartSet;

	//pointIdArr contains start indices for points in each partition
	unsigned long int *pointIdArr;


	//pointPartInfoArr contains sizes (number of points) for each partitions
	unsigned int *pointPartInfoArr;

	//contains initial points for the initial delaunay
	double *initPointArr;
	unsigned int initPointArrSize;

	//contains extension points for the initial delaunay
	double *extensionPointArr;
	unsigned int extensionPointArrSize;
	

	//total number of point in domain (wihtout 4 points in the corners of domain square)
	unsigned long int pointNumMax;

	domain(double lowX, double lowY, double highX, double highY, std::string pathStr);
	domain(double lowX, double lowY, double highX, double highY);
//	domain(const point &lPoint, const point &hPoint, std::string pathStr);
	domain(){}
	~domain();

	point getLowPoint();
	point getHighPoint();

	void setLowPoint(point p);
	void setHighPoint(point p);

	double getDomainSizeX();
	double getDomainSizeY();


	/** Input: a bounding box of an element
	 output: a bounging grid that intersects with the element bounding box */
	 gridBound boundingGrid(boundingBox bBox);

	/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 	 partition with the higher index. Ex: 2.5/2 = 2 --> column/row 0, 1, 2; 2.0/2 = 1 
	geoBound is is the domain bound*/
	 gridElement mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY);

	/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
	 partition with the lower index. 2.5/2 = 2 --> column/row 0, 1, 2; 4.0/2 = 2, need to be subtract by 1 --> 1
	 if point p fall on the grid line (partition line) --> take lower element.*/
	 gridElement mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY);


	//void generateBoundingGridPoints();
	void readPointPartFileInfo();
	void loadInitPoints();
	void loadExtensionPoints();
	void initTriangulate();
	void basicTriangulate(point p);
	void drawTriangles();
	void drawActivePartTriangles();
	void drawTriangleArr();
	void drawTriangleArr1();

	//transform link list of triangles (triangleList) into array of triangle (triangleArr)
	void triangleTransform();

	//generate all intersections between each triangle and partitions in the domain.
	void generateIntersection();

	//generate confliction for each partition. 
	void generateConflictPartitions();

	void printConflictPartitions();

	//generate active partitions in the domain
	void generateActivePartitions();

	//after picking out partitions for processing, we need to update the confliced partitions in conflictPartList
	void updateConflictPartitions();

	//deliver triangles to active partitions
	void deliverTriangles();

	//this function stores all triangles belong to active partitions into files for further delaunay on cluster
	void storeActiveParition();

	//Process delaunay for active partitions on cluster. Each partition will be processed (delaunay) on a processor
	void distributeDelaunay();

	//get returned triangles which are processed from MPI, remove those triangles that are delivered, 
	//update to the main triangleArr, and ready for the next stages of triangulation
	void updateTriangleArr();

	//append the content of returnStoreTriangleIds.tri (from MPI processing) to triangleIds.tri
	void collectStoreTriangleIds();

	//return number of partitions that are unfinished
	int unfinishedPartNum();

	int unDeliveredTriangleNum();

	void printTriangleArray();

	//When all partitions are processed via MPI, we need to store all trianlges left
	void storeAllTriangles();
};
