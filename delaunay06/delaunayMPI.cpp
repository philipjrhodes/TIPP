#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>
#include "linkList.h"

//#include "triangle.h"

#define MASTER_RANK 0
#define send_data_tag 2001
#define return_data_tag 2002

std::string path;

//domain sizes
int xPartNum, yPartNum;

//active partitions
int *partArr=NULL;
//number of active partitions
int partNum;

double *tempCoorArr=NULL;//all coordinates of triangles by sender
unsigned long int *tempPointIdArr=NULL;//all point id of triangles by sender

//store all startId points for each partition
unsigned long int *startIdPartArr=NULL;
unsigned long int startId;

//all point coordinates of partitions
double *pointCoorArr=NULL;


int totalTriangleSize;//number of triangles of all active partitions
int *triangleSizeArr=NULL;//number of triangle belong to each partition
int *triangleSizeOffsetArr=NULL;


//all partitions in which each partition contains the number of points
int *pointNumPartArr=NULL;
//compute offset for parallel reading file
int *pointNumPartOffsetArr=NULL;
//number of points and offset for each active partition
int pointNumPartSize;
int pointNumPartOffsetSize;


//list of triangles that are currently processed delaunay
triangleNode *triangleList = NULL;
//list of triangles that are not currently processed delaunay, will be processed by other partitions
triangleNode *temporaryTriangleList = NULL;
//list of triangle Ids that are not currently processed delaunay, circumcircles are inside partition
triangleNode *storeTriangleList = NULL;


MPI_Status status;

//point coordinate file for all partitions
MPI_File fh;

//read all triangles for all active partitions
void readTriangleData();

//based on data received from master, each slave generate all initial triangles for the partitions
void generateTriangles(int my_rank, int triangleNum, double *coorArr, unsigned long int *pointIdArr);

//read point coordinates from pointPart.ver
void readPointCoor(int my_rank);

void triangulate(unsigned int partId);
boundingBox partBox(int partId);
void processStoreTriangles(int my_rank);
void processTriangleList(int my_rank);
void releaseMemory();
void printTriangleList(int partId);

//****************************************************************************
int main (int argc, char** argv){
//****************************************************************************

	int triangleSize;//number of triangles of current active partitions
	int triangleSizeOffset;//the offset of number of triangles of current active partitions

	//process temporary data of active partitions
	double *coorArr=NULL;//all coordinates of triangles for receivers
	unsigned long int *pointIdArr=NULL;//all point id of triangles for receivers

	//active partition Id
	int partId;

    int ierr, my_rank, pool_size, namelen, root_process;

	bool i_am_the_master = false;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

	if(argc==1)// no arguments
		std::cout<<"You need to provide argument: path to the dataset\n";
	else path = argv[1];

	//ACTION - start do to parallel

	//Initialize MPI.
    MPI_Init(&argc, &argv);

	root_process = 0;

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	//pool_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
    MPI_Comm_size(MPI_COMM_WORLD, &pool_size);
//std::cout<<"my Id: "<<my_rank<<"\n";

    MPI_Get_processor_name(processor_name, &namelen);

	if(my_rank == MASTER_RANK) i_am_the_master = true;

	//read triangle data from file
	if(i_am_the_master)	readTriangleData();

	MPI_Barrier(MPI_COMM_WORLD);

	//broadcast xPartNum & yPartNum to all slave nodes
	MPI_Bcast(&xPartNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&yPartNum, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//send number of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeArr, 1, MPI_INT, &triangleSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send number of offset of temporary triangles of each active partition to all nodes (master , slaves)
	MPI_Scatter(triangleSizeOffsetArr, 1, MPI_INT, &triangleSizeOffset, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send active partId to all nodes (master , slaves)
	MPI_Scatter(partArr, 1, MPI_INT, &partId, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send number of points and offsets in each active partition to all nodes (master , slaves)
	//Scatter pointNumPartArr & pointNumPartOffsetArr to pointNumPartSize & pointNumPartOffsetSize from master to all nodes
	//this command is used for readPointCoor()
	MPI_Scatter(pointNumPartArr, 1, MPI_INT, &pointNumPartSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Scatter(pointNumPartOffsetArr, 1, MPI_INT, &pointNumPartOffsetSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send startId for each machine (master & slaves)
	MPI_Scatter(startIdPartArr, 1, MPI_UNSIGNED_LONG_LONG, &startId, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, MPI_COMM_WORLD);

//if(my_rank==3) std::cout<<"startId: "<<startId<<"\n";

	MPI_Barrier(MPI_COMM_WORLD);

	if(i_am_the_master){

		//This is the master (root) process
//	    printf("Hello from master process %d on %s of %d\n", my_rank, processor_name, pool_size);

        //distribute a portion of the array (triangleArr) to each child process
		for(int process_id = 1; process_id < pool_size; process_id++) {//pool_size is number of processes
			ierr = MPI_Send(&tempCoorArr[triangleSizeOffsetArr[process_id]*6], triangleSizeArr[process_id]*6, MPI_DOUBLE, process_id, send_data_tag, MPI_COMM_WORLD);
			ierr = MPI_Send(&tempPointIdArr[triangleSizeOffsetArr[process_id]*3], triangleSizeArr[process_id]*3, MPI_UNSIGNED_LONG_LONG, process_id, send_data_tag, MPI_COMM_WORLD);
		}	
		//pointNum & startId for master
		coorArr = &tempCoorArr[triangleSizeOffset*6];
		pointIdArr = &tempPointIdArr[triangleSizeOffset*3];
		//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
		generateTriangles(my_rank, triangleSize, coorArr, pointIdArr);
		delete [] tempCoorArr;
		delete [] tempPointIdArr;
		delete [] triangleSizeArr;
		delete [] triangleSizeOffsetArr;
	}
	else{//////////////////////This is the other process/////////////////////////////////
//	    printf("Hello from other process %d on %s of %d\n", my_rank, processor_name, pool_size);

		//allocate coorArr & pointIdArr for slave machines
		coorArr = new double[triangleSize*6];
		pointIdArr = new unsigned long int[triangleSize*3];

//		for(int i=0; i<triangleSize; i++)
//			std::cout<<pointIdArr[3*i]<<"--"<<pointIdArr[3*i+1]<<"--"<<pointIdArr[3*i+2]<<"\n";

		//receive data from master (process 0)
		ierr = MPI_Recv(coorArr, triangleSize*6, MPI_DOUBLE, root_process, send_data_tag, MPI_COMM_WORLD, &status);
		ierr = MPI_Recv(pointIdArr, triangleSize*3, MPI_UNSIGNED_LONG_LONG, root_process, send_data_tag, MPI_COMM_WORLD, &status);

		//from two arrays coorArr and pointIdArr, generate a linklist to delaunay.
		generateTriangles(my_rank, triangleSize, coorArr, pointIdArr);
		delete [] coorArr;
		delete [] pointIdArr;
	}

	MPI_Barrier(MPI_COMM_WORLD);

	//parallel read from file pointPart.ver
	readPointCoor(my_rank);

	MPI_Barrier(MPI_COMM_WORLD);

	//after we have triangle information triangleList from generateTriangles and pointCoorArr from readPointCoordinates we will do delaunay/triangulate
	triangulate(partId);

//if(partId==2) printTriangleList(partId);

	MPI_Barrier(MPI_COMM_WORLD);

	//srore ids of storeTriangleList from triangulate function
	processStoreTriangles(my_rank);

	MPI_Barrier(MPI_COMM_WORLD);

	//store triangleList & temporaryTriangleList from triangulate function
	processTriangleList(my_rank);

	//release allocated memory
	releaseMemory();

    MPI_Finalize();
    return 0;
}

//============================================================================
//process storeTriangleList (tranform into array triangle Ids and send back to master node)
void processStoreTriangles(int my_rank){
	//number of store triangles of each process
	int *storeTriangleNumArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	int *storeTriangleNumOffsetArr = NULL;
	int storeTriangleNumOffset;

	if(my_rank==0) //only master node
		storeTriangleNumArr = new int[partNum];
	unsigned int triangleNum = size(storeTriangleList);

	MPI_Gather(&triangleNum, 1, MPI_INT, storeTriangleNumArr, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	if(my_rank==0){//only master node
		storeTriangleNumOffsetArr = new int[partNum];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<partNum; i++)
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
	}
	MPI_Scatter(storeTriangleNumOffsetArr, 1, MPI_INT, &storeTriangleNumOffset, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

//if(my_rank==0)//only master node
//	for(int i=0; i<partNum; i++) std::cout<<storeTriangleNumArr[i]<<"\n";
//	std::cout<<triangleNum<<"\n";

//if(my_rank==3)
//std::cout<<"********triangleNum = "<<triangleNum<<", myrank = "<<my_rank<<std::endl;


	if(triangleNum!=0){//need to store
		//transform storeTriangleList to array of ids
		unsigned long int *triangleIdArr = new unsigned long int[triangleNum*3];
		triangleNode *head = storeTriangleList;
		unsigned long int index = 0;
		while(head!=NULL){
			triangleIdArr[index*3] = head->tri->p1.getId();
			triangleIdArr[index*3+1] = head->tri->p2.getId();
			triangleIdArr[index*3+2] = head->tri->p3.getId();
/*
if(my_rank==1){
	std::cout<<triangleIdArr[index*3]<<" "<<triangleIdArr[index*3+1]<<" "<<triangleIdArr[index*3+2]<<"\n";
	std::cout<<head->tri->p1.getX()<<" "<<head->tri->p1.getY()<<" "<<head->tri->p2.getX()<<" "<<head->tri->p2.getY()<<" "<<head->tri->p3.getX()<<" "<<head->tri->p3.getY()<<"\n";
}
*/

			index++;
			head=head->next;
		}
		removeLinkList(storeTriangleList);

		std::string currPath = path + "returnStoreTriangleIds.tri";
		//store array storeTriangleIds of each processes to a single files
//		MPI_File_open(MPI_COMM_WORLD, currPath.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
		MPI_File_open(MPI_COMM_WORLD, currPath.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
		MPI_File_set_view(fh, storeTriangleNumOffset * 3 * sizeof(unsigned long int), MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, "native", MPI_INFO_NULL);
		MPI_File_write(fh, triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG, MPI_STATUS_IGNORE);
		MPI_File_close(&fh);
	
		delete [] triangleIdArr;
	}
	if(my_rank==0){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
	}

//if(my_rank==1) std::cout<<"triangleNum: "<<triangleNum<<"\n";
}

//============================================================================
//process triangleList & temporaryTriangleList (tranform into array of triangles and send back to master node)
void processTriangleList(int my_rank){
	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	//number of store triangles of each process
	int *triangleNumArr = NULL;
	//based on triangleNumArr, compute triangleNumOffsetArr
	int *triangleNumOffsetArr = NULL;
	int triangleNumOffset;

	//only master node
	if(my_rank==0) triangleNumArr = new int[partNum];
	unsigned int triangleNum = size(triangleList);

//if(my_rank==0)
//std::cout<<"triangleNum::::: "<<triangleNum<<"\n";

	MPI_Gather(&triangleNum, 1, MPI_INT, triangleNumArr, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	if(my_rank==0){//only master node
		triangleNumOffsetArr = new int[partNum];
		triangleNumOffsetArr[0] = 0;
		for(int i=1; i<partNum; i++){
			triangleNumOffsetArr[i] = triangleNumOffsetArr[i-1] + triangleNumArr[i-1];
//			std::cout<<triangleNumOffsetArr[i]<<"\n"; 
		}
	}
	MPI_Scatter(triangleNumOffsetArr, 1, MPI_INT, &triangleNumOffset, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);


	//flat point coordinates and Ids separately
	if(triangleNum!=0){
		unsigned long int *triangleIdArr = new unsigned long int[triangleNum*3];
		//this is local pointCoorArr (different from the global one)
		double *pointCoorArr = new double[triangleNum*6];

		triangleNode *head = triangleList;
		unsigned long int index = 0;
		while(head!=NULL){
			triangleIdArr[index*3] = head->tri->p1.getId();
			triangleIdArr[index*3+1] = head->tri->p2.getId();
			triangleIdArr[index*3+2] = head->tri->p3.getId();

			pointCoorArr[index*6] = head->tri->p1.getX();
			pointCoorArr[index*6+1] = head->tri->p1.getY();
			pointCoorArr[index*6+2] = head->tri->p2.getX();
			pointCoorArr[index*6+3] = head->tri->p2.getY();
			pointCoorArr[index*6+4] = head->tri->p3.getX();
			pointCoorArr[index*6+5] = head->tri->p3.getY();
/*
if(my_rank==1){
std::cout<<pointCoorArr[index*6]<<" "<<pointCoorArr[index*6+1]<<" "<<triangleIdArr[index*3]<<"\n";
std::cout<<pointCoorArr[index*6+2]<<" "<<pointCoorArr[index*6+3]<<" "<<triangleIdArr[index*3+1]<<"\n";
std::cout<<pointCoorArr[index*6+4]<<" "<<pointCoorArr[index*6+5]<<" "<<triangleIdArr[index*3+2]<<"\n\n";
}
*/

			index++;
			head=head->next;
		}
		removeLinkList(triangleList);

//if(my_rank==3) std::cout<<"triangleNum: "<<triangleNum<<" "<<"triangleNumOffset: "<<triangleNumOffset<<"\n";

		//save to single file
		std::string currPath = path+ "returnTriangleIds.tri";
		//store array storeTriangleIds of each processes to a single files
		MPI_File_open(MPI_COMM_WORLD, currPath.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
		MPI_File_set_view(fh, triangleNumOffset * 3 * sizeof(unsigned long int), MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, "native", MPI_INFO_NULL);
		MPI_File_write(fh, triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG, MPI_STATUS_IGNORE);
		MPI_File_close(&fh);

		std::string currPath1 = path + "returnTriangleCoors.tri";
		//store array storeTriangleIds of each processes to a single files
		MPI_File_open(MPI_COMM_WORLD, currPath1.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
		MPI_File_set_view(fh, triangleNumOffset * 6 * sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_write(fh, pointCoorArr, triangleNum*6, MPI_DOUBLE, MPI_STATUS_IGNORE);
		MPI_File_close(&fh);

		//release memory
		delete [] triangleIdArr;
		delete [] pointCoorArr;
	}
	if(my_rank==0){
		delete [] triangleNumArr;
		delete [] triangleNumOffsetArr;
	}
}

//============================================================================
//read point coordinates from pointPart.ver 
//Depending on partId, read only coordinates belong tos that partition
void readPointCoor(int my_rank){

	if(pointNumPartSize>0){
		std::string currPath = path + "pointPart.ver";
		MPI_File_open(MPI_COMM_WORLD, currPath.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

		pointCoorArr = new double[pointNumPartSize*2];
		//2 mean coordinates x, y
		//PI_File_seek(fh, pointNumPartOffsetSize*2*sizeof(double), MPI_SEEK_SET);
		MPI_File_set_view(fh, pointNumPartOffsetSize*2*sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
		MPI_File_read(fh, pointCoorArr, pointNumPartSize*2, MPI_DOUBLE, &status);
		MPI_File_close(&fh);

/*	if(my_rank==2)
		for(int i=0; i<pointNumPartSize; i++) 
		std::cout<<pointCoorArr[i*2]<<" "<<pointCoorArr[i*2+1]<<"\n";
*/	}
}

//============================================================================
//based on data received from master, each slave generate all initial triangles for the partitions
void generateTriangles(int my_rank, int triangleNum, double *coorArr, unsigned long int *pointIdArr){

	for(int index=0; index<triangleNum; index++){
		point p1(coorArr[index*6], coorArr[index*6+1], pointIdArr[index*3]);
		point p2(coorArr[index*6+2], coorArr[index*6+3], pointIdArr[index*3+1]);
		point p3(coorArr[index*6+4], coorArr[index*6+5], pointIdArr[index*3+2]);
//std::cout<<pointIdArr[index*3]<<" "<<pointIdArr[index*3+1]<<" "<<pointIdArr[index*3+2]<<"\n";
		triangle *newTriangle = new triangle(p1, p2, p3);
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
/*
if(my_rank==1){
triangleNode *head = triangleList;
while(head!=NULL){
	std::cout<<head->tri->p1.getId()<<" "<<head->tri->p2.getId()<<" "<<head->tri->p3.getId()<<"========"<<head->tri->p1.getX()<<" "<<head->tri->p1.getY()<<" "<<head->tri->p2.getX()<<" "<<head->tri->p2.getY()<<" "<<head->tri->p3.getX()<<" "<<head->tri->p3.getY()<<"\n";
	head = head->next;
}
}
*/
}
//============================================================================
//read all triangle data for active partitions, and other partition information
void readTriangleData(){

	//read meta data for temprary files
	std::string fileInfoStr = path + "tempTriangles.xfdl";
	std::ifstream triangleInfoFile(fileInfoStr.c_str());
	if(!triangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	//read first line (1 number only) --> number of active partitions
	std::string strItem;
	triangleInfoFile >> strItem;
	partNum = atoi(strItem.c_str());

	//second line is active partition Ids
	partArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		partArr[i] = atoi(strItem.c_str());
	}

	//third line is the numbers of triangles which belong to active partitions
	triangleSizeArr = new int[partNum];//number of triangles belong to partitions
	totalTriangleSize = 0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
//std::cout<<triangleSizeArr[i]<<" ";
	}
std::cout<<"\n";

	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}
	//fifth line is the numbers of points in each active partition
	pointNumPartArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartArr[i] = atoi(strItem.c_str());
	}
	//sixth line is the offset of previous line (point numbers of each active partition)
	pointNumPartOffsetArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartOffsetArr[i] = atoi(strItem.c_str());
	}
	//seventh line is xPartNum & yPartNum
	triangleInfoFile >> strItem;
	xPartNum = atoi(strItem.c_str());
	triangleInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());

	//eighth line is startIds for points in each active partition
	startIdPartArr = new unsigned long int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		startIdPartArr[i] = atoi(strItem.c_str());
//std::cout<<startIdPartArr[i]<<" ";
	}

	triangleInfoFile.close();


	//read all coordinates of triangles
	std::string dataFileStr1 = path + "tempCoor.tri";
	FILE *f1 = fopen(dataFileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<dataFileStr1<<std::endl;
		return;
	}
	tempCoorArr = new double[totalTriangleSize*6];
	fread(tempCoorArr, sizeof(double), totalTriangleSize*6, f1);
	fclose(f1);

	//read all point ids of triangles
	std::string dataFileStr2 = path + "tempPointId.tri";
	FILE *f2 = fopen(dataFileStr2.c_str(), "rb");
	if(!f2){
		std::cout<<"not exist "<<dataFileStr2<<std::endl;
		return;
	}
	tempPointIdArr = new unsigned long int[totalTriangleSize*3];
	fread(tempPointIdArr, sizeof(double), totalTriangleSize*3, f2);
	fclose(f2);

/*
for(int i=0; i<totalTriangleSize; i++)
	std::cout<<tempPointIdArr[i*3]<<" "<<tempPointIdArr[i*3+1]<<" "<<tempPointIdArr[i*3+2]<<"========"<<tempCoorArr[i*6]<<" "<<tempCoorArr[i*6+1]<<" "<<tempCoorArr[i*6+2]<<" "<<tempCoorArr[i*6+3]<<" "<<tempCoorArr[i*6+4]<<" "<<tempCoorArr[i*6+5]<<"\n";
*/
}

//==============================================================================
void releaseMemory(){
	delete [] partArr;
	delete [] startIdPartArr;
	delete [] pointNumPartArr;
	delete [] pointNumPartOffsetArr;
}

//==============================================================================
//determine boundingbox of current partition based on partId
boundingBox partBox(int partId){
	double xPartSize = 1.0/xPartNum;
	double yPartSize = 1.0/yPartNum;
	int gridPartX = partId % xPartNum;
	int gridPartY = partId / yPartNum;
	point lowPoint(gridPartX*xPartSize, gridPartY*yPartSize);
	point highPoint((gridPartX+1)*xPartSize, (gridPartY+1)*yPartSize);
//if(partId==13){
//std::cout<<xPartSize<<" "<<xPartSize<<"\n";
//std::cout<<"partID: "<<partId<<" "<<gridPartX*xPartSize<<" "<<gridPartY*yPartSize<<" "<<(gridPartX+1)*xPartSize<<" "<<(gridPartY+1)*yPartSize<<"\n";
//}
	return boundingBox(lowPoint, highPoint);
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void triangulate(unsigned int partId){

	if(pointCoorArr==NULL) return;
std::cout<<"start triangulation from partId = "<<partId<<std::endl;

	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;
	double sweepLine = 0;

	unsigned long int partPointId = startId;

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);



	//sequentially insert point into delaunay
	for(unsigned int localPointIndex=0; localPointIndex<pointNumPartSize; localPointIndex++){
		p.setX(pointCoorArr[localPointIndex*2]);
		p.setY(pointCoorArr[localPointIndex*2+1]);
		p.setId(partPointId);
		partPointId++;
/*
if(partId==13)
std::cout<<p.getId()<<" "<<p.getX()<<" "<<p.getY()<<std::endl;
*/
		sweepLine = p.getX();

		triangleNode *preNode = NULL;
		triangleNode *currNode = triangleList;
		bool goNext = true;
		while(currNode != NULL){
			//this triangle will never circumcircle the new point p
			if(currNode->tri->getFarestCoorX() < sweepLine){
				//if this triangle is within partition area, store it to storeTriangleList
				if(currNode->tri->inside(currPartBBox)){
 					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(storeTriangleList, exNode);
				}
				//store this triangle to temporaryTriangleList and use it for the next row
				else{
					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(temporaryTriangleList, exNode);
				}
				goNext = false;
			}
			else//the circumcircle of this triangle can contain the new point
			if(currNode->tri->circumCircleContains(p)){
				edge *edge1 = new edge(currNode->tri->getEdge1());
				edge *edge2 = new edge(currNode->tri->getEdge2());
				edge *edge3 = new edge(currNode->tri->getEdge3());

				edgeNode *eNode1 = createNewNode(edge1);
				edgeNode *eNode2 = createNewNode(edge2);
				edgeNode *eNode3 = createNewNode(edge3);

				insertFront(polygon, eNode1);
				insertFront(polygon, eNode2);
				insertFront(polygon, eNode3);

				removeNode(triangleList, preNode, currNode);
				goNext = false;
			}

			//go to the next element in the liskList
			if(goNext){
				preNode = currNode;
				currNode = currNode->next;
			}
			goNext = true;
		}

		//Find all bad edges from polygon, then remove bad edges later
//		edgeNode *preEdgeNode1 = NULL;
		edgeNode *currEdgeNode1 = polygon;
//		edgeNode *preEdgeNode2 = NULL;
		edgeNode *currEdgeNode2 = polygon;
		while(currEdgeNode1!=NULL){
			while(currEdgeNode2!=NULL){
				//two different edges in polygon but same geological edge
				if((currEdgeNode1!=currEdgeNode2)&&(*(currEdgeNode1->ed) == *(currEdgeNode2->ed))){
					edge *newEdge = new edge(*currEdgeNode1->ed);
					edgeNode *newEdgeNode = createNewNode(newEdge);
					insertFront(badEdges, newEdgeNode);
				}
//				preEdgeNode2 = currEdgeNode2;
				currEdgeNode2 = currEdgeNode2->next;
			}
//			preEdgeNode1 = currEdgeNode1;
			currEdgeNode1 = currEdgeNode1->next;
			currEdgeNode2 = polygon;
		}

		//remove all bad edges from polygon
		edgeNode *badEdgeNode = badEdges;
		edgeNode *preEdgeNode = NULL;
		edgeNode *currEdgeNode = polygon;
		while(badEdgeNode!=NULL){
			while(currEdgeNode!=NULL){
				if(*(badEdgeNode->ed) == *(currEdgeNode->ed)){
					removeNode(polygon, preEdgeNode, currEdgeNode);
					break;
				}
				else{
					preEdgeNode = currEdgeNode;
					currEdgeNode = currEdgeNode->next;
				}
			}
			badEdgeNode = badEdgeNode->next;
			
			preEdgeNode = NULL;
			currEdgeNode = polygon;
		}

		//remove all bad edges
		removeLinkList(badEdges);


//std::cout<<"Edge list after removing bad edges: \n";

		//polygon now is a hole. With insert point p, form new triangles and insert into triangleList
		edgeNode *scanEdgeNode = polygon;
		while(scanEdgeNode!=NULL){
/*			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			triangleNode *newTriangleNode = createNewNode(newTriangle);
			insertFront(triangleList, newTriangleNode);
			scanEdgeNode = scanEdgeNode->next;
*/
			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			newTriangle->computeCenterRadius();
			if(newTriangle->colinear()||newTriangle->isBad())
				delete newTriangle;
			else{
				triangleNode *newTriangleNode = createNewNode(newTriangle);
				insertFront(triangleList, newTriangleNode);
			}
		scanEdgeNode = scanEdgeNode->next;
		}

		//delete polygon, badTriangle
		removeLinkList(polygon);
	}
	delete [] pointCoorArr;
	std::cout<<"done triangulation from partId = "<<partId<<std::endl;
}

//===========================================================================
void printTriangleList(int partId){
	std::cout<<"triangleList size:"<<size(triangleList)<<"\n";
	std::cout<<"triangle Id List:"<<"\n";
	triangleNode *head = triangleList;
	while(head!=NULL){
		std::cout<<head->tri->p1.getX()<<" "<<head->tri->p1.getY()<<" " <<head->tri->p1.getId()<<"\n";
		std::cout<<head->tri->p2.getX()<<" "<<head->tri->p2.getY()<<" " <<head->tri->p2.getId()<<"\n";
		std::cout<<head->tri->p3.getX()<<" "<<head->tri->p3.getY()<<" " <<head->tri->p3.getId()<<"\n";
		head = head->next;
	}
}
