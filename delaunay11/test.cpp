#include <iostream>
#include <mpi.h>
#include <fstream>
#include <string>

#define MASTER_RANK 0

	//domain sizes
	int xPartNum, yPartNum;

	//active partitions
	int *partArr;
	//number of active partitions
	int partNum, partId;

	double *tempCoorArr;//all coordinates of triangles by sender
	unsigned long long *tempPointIdArr;//all point id of triangles by sender

	//all point coordinates of partitions
	double *pointCoorArr;
	unsigned long long *pointIdArr;

	int totalTriangleSize;//number of triangles of all active partitions
	int *triangleSizeArr;//number of triangle belong to each partition
	int *triangleSizeOffsetArr;


	//all partitions in which each partition contains the number of points
	int *pointNumPartArr;
	//compute offset for parallel reading file
	int *pointNumPartOffsetArr;
	//number of points and offset for each active partition
	int pointNumPartSize;
	int pointNumPartOffsetSize;

	MPI_Status status;

	//point coordinate file for all partitions
	MPI_File fh;

	std::string path;

/*
int distribute::partIndex(double x, double y, int xPartSize, int yPartSize, int xPartNum, int yPartNum){

        int gridX = x/xPartSize;
        if(gridX>=xPartNum) gridX = gridX-1;
        int gridY = y/yPartSize;
        if(gridY>=xPartNum) gridY = gridY-1;

        return gridY*xPartNum + gridX;
}
*/

//============================================================================
//check whether a point stay inside a partititon
bool insidePartion(double *pointCoorArr, int size, double domainSize, int xPartNum, int yPartNum, int partId){
	double xPartSize = domainSize/xPartNum;
	double yPartSize = domainSize/yPartNum;
	double lowPointX = (partId%yPartNum)*xPartSize;
	double lowPointY = (partId/yPartNum)*yPartSize;
	double highPointX = lowPointX + xPartSize;
	double highPointY = lowPointY + yPartSize;

	for(int i=0; i<size; i++){
		double x = pointCoorArr[i*2];
		double y = pointCoorArr[i*2+1];
		if((x<lowPointX)||(y<lowPointY)||(x>highPointX)||(y>highPointY)){
			std::cout<<x<<" "<<y<<" "<<lowPointX<<" "<<lowPointY<<" "<<highPointX<<" "<<highPointY<<" "<<partId<<"\n";
			return false;
		}
	}
	return true;

}
//============================================================================
//read all triangle data for active partitions, and other partition information
void readTriangleData(int pool_size){

	//read meta data for temprary files
	std::string fileInfoStr = path + "delaunayResults/tempTriangles.xfdl";
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
	partArr = new int[pool_size];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		partArr[i] = atoi(strItem.c_str());
	}

	//third line is the numbers of triangles which belong to active partitions
	triangleSizeArr = new int[pool_size];//number of triangles belong to partitions
	for(int i=0; i<pool_size; i++) triangleSizeArr[i]=0;
	totalTriangleSize = 0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
	}


	//extraTriangleNum is the fake number add up to the totalTriangleSize
	//for threads with my_rank >= partNum
	int extraTriangleNum = 0;
	if(partNum<pool_size) extraTriangleNum = pool_size - partNum;

	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeArr[i] will be 1 (fake)
	if(partNum<pool_size){
		for(int i=partNum; i<pool_size; i++)
			triangleSizeArr[i] = 1;
	}


	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new int[pool_size];
	for(int i=0; i<pool_size; i++) triangleSizeOffsetArr[i]=0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}
	//in case number of active partitions is less than number of MPI threads
	//in this case triangleSizeOffsetArr[i] will be fake
	if(partNum<pool_size){
		for(int i=partNum; i<pool_size; i++)
			triangleSizeOffsetArr[i] = triangleSizeOffsetArr[i-1] + triangleSizeArr[i];
	}


	//fifth line is the numbers of points in each active partition
	pointNumPartArr = new int[pool_size];
	for(int i=0; i<pool_size; i++) pointNumPartArr[i]=0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartArr[i] = atoi(strItem.c_str());
	}
	//sixth line is the offset of previous line (point numbers of each active partition)
	//pointNumPartOffsetArr = new int[partNum];
	pointNumPartOffsetArr = new int[pool_size];
	for(int i=0; i<pool_size; i++) pointNumPartOffsetArr[i]=0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartOffsetArr[i] = atoi(strItem.c_str());
	}
	//seventh line is xPartNum & yPartNum
	triangleInfoFile >> strItem;
	xPartNum = atoi(strItem.c_str());
	triangleInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());
	triangleInfoFile.close();


	//read all coordinates of triangles
	std::string dataFileStr1 = path + "delaunayResults/tempCoor.tri";
	FILE *f1 = fopen(dataFileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<dataFileStr1<<std::endl;
		return;
	}
	tempCoorArr = new double[(totalTriangleSize + extraTriangleNum)*6];
	fread(tempCoorArr, sizeof(double), totalTriangleSize*6, f1);
	fclose(f1);

	//read all point ids of triangles
	std::string dataFileStr2 = path + "delaunayResults/tempPointId.tri";
	FILE *f2 = fopen(dataFileStr2.c_str(), "rb");
	if(!f2){
		std::cout<<"not exist "<<dataFileStr2<<std::endl;
		return;
	}
	tempPointIdArr = new unsigned long long[(totalTriangleSize + extraTriangleNum)*3];
	fread(tempPointIdArr, sizeof(double), totalTriangleSize*3, f2);
	fclose(f2);
}


void readPointCoor(int my_rank, int partId){
	//if(partId = 8739) return;
	//if(my_rank==30) return;
	int count1 = 0;
	int count2 = 0;
	std::string currPath1 = path + "delaunayResults/pointCoorPart.ver";
	MPI_File_open(MPI_COMM_WORLD, currPath1.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	pointCoorArr = new double[pointNumPartSize*2];

	//2 mean coordinates x, y
	//PI_File_seek(fh, pointNumPartOffsetSize*2*sizeof(double), MPI_SEEK_SET);
	//MPI_File_set_view(fh, pointNumPartOffsetSize*2*sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
	MPI_File_seek( fh, pointNumPartOffsetSize*2*sizeof(double), MPI_SEEK_SET);
	MPI_File_read(fh, pointCoorArr, pointNumPartSize*2, MPI_DOUBLE, &status);
	MPI_Get_count(&status, MPI_DOUBLE, &count1); 
	MPI_File_close(&fh);


	std::string currPath2 = path + "delaunayResults/pointIdPart.ver";
	MPI_File_open(MPI_COMM_WORLD, currPath2.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	pointIdArr = new unsigned long long[pointNumPartSize];

	//2 mean coordinates x, y
	MPI_File_set_view(fh, pointNumPartOffsetSize*sizeof(unsigned long long), MPI_UNSIGNED_LONG_LONG, MPI_UNSIGNED_LONG_LONG, "native", MPI_INFO_NULL);
	MPI_File_read(fh, pointIdArr, pointNumPartSize, MPI_UNSIGNED_LONG_LONG, &status);
	MPI_Get_count(&status, MPI_UNSIGNED_LONG_LONG, &count2); 
	MPI_File_close(&fh);

	std::cout<<my_rank<<" "<<count1<<" "<<count2<<"\n";

}



//=============================================================================
int main (int argc, char** argv){
//=============================================================================

    int my_rank, pool_size;
	double domainSize=2;
	path = "../dataSources/2Bvertices/";

	partArr=NULL;
	tempCoorArr=NULL;
	tempPointIdArr=NULL;
	pointCoorArr=NULL;
	pointIdArr=NULL;
	pointNumPartArr=NULL;
	pointNumPartOffsetArr=NULL;

	//Initialize MPI.
    MPI_Init(&argc, &argv);

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	//pool_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
	MPI_Comm_size(MPI_COMM_WORLD, &pool_size);

	if(my_rank==0) readTriangleData(pool_size);

	MPI_Barrier(MPI_COMM_WORLD);


	//broadcast xPartNum & yPartNum to all slave nodes
	MPI_Bcast(&xPartNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&yPartNum, 1, MPI_INT, 0, MPI_COMM_WORLD);

	double xPartSize = domainSize/xPartNum;
	double yPartSize = domainSize/yPartNum;

	//broadcast partNum to all slave nodes
	MPI_Bcast(&partNum, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//send active partId to all nodes (master , slaves)
	MPI_Scatter(partArr, 1, MPI_INT, &partId, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//send number of points and offsets in each active partition to all nodes (master , slaves)
	//Scatter pointNumPartArr & pointNumPartOffsetArr to pointNumPartSize & pointNumPartOffsetSize from master to all nodes
	//this command is used for readPointCoor()
	MPI_Scatter(pointNumPartArr, 1, MPI_INT, &pointNumPartSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Scatter(pointNumPartOffsetArr, 1, MPI_INT, &pointNumPartOffsetSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
//	if(my_rank==0)
		std::cout<<"partId: "<<partId<<", pointNumPartSize: "<<pointNumPartSize<<", pointNumPartOffsetSize: "<<pointNumPartOffsetSize<<"\n";
//		for(int i=0; i<pool_size; i++) std::cout<<pointNumPartArr[i]<<" "<<pointNumPartOffsetArr[i]<<"\n";
//std::cout<<"rank: "<<my_rank<<", partId: "<<partId<<", pointNumPartSize: "<<pointNumPartSize<<", pointNumPartOffsetSize: "<<pointNumPartOffsetSize<<"\n";
//std::cout<<my_rank<<" "<<partId<<"\n";
//if(partId == 8927) std::cout<<my_rank<<" "<<pointNumPartOffsetSize<<"\n";

	readPointCoor(my_rank, partId);
//std::cout<<"Done reading from process: "<<my_rank<<"\n";

//	if(insidePartion(pointCoorArr, pointNumPartSize, domainSize, xPartNum, yPartNum, partId))
//		std::cout<<"partition: "<<partId<<" is good!!\n";

	delete [] pointCoorArr;
	delete [] pointIdArr;
	MPI_Finalize();
}

