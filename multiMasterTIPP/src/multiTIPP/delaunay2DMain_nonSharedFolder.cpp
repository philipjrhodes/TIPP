//Multiple masters
//dataset stored in server
#include "io.h"
#include "domain_nonSharedFolder.h"
#include "coarsePartition_nonSharedFolder.h"
#include "delaunayMPI_nonSharedFolder.h"

#define SUB_MASTER_RANK 0
#define data_tag1 2003
#define data_tag2 2004
#define data_tag3 2005

//=============================================================================
//transform list of triangleIdArr (triangleIdArrList) into flat array (triangleIdArr)
void transformTriangleIdArrList(std::list<nodeTriangleIdArr> &triangleIdArrList, unsigned long long *&triangleIdArr, unsigned long long &triangleNum){

	if(triangleIdArrList.size()==0) return;
	unsigned long long totalTriangleSize = 0;
	//flattening multiple triangleIdArr in triangleIdArrList into one triangleIdArr (triangleIdArr)
	for(std::list<nodeTriangleIdArr>::iterator it=triangleIdArrList.begin(); it!=triangleIdArrList.end(); it++){
		totalTriangleSize += (*it).triangleNum;
	}

	//triangleNum is total number of triangles
	try {
		triangleIdArr = new unsigned long long[totalTriangleSize*3];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory allocated for triangleIdArr overflow!!!!!!!!!!!\n";
		exit(1);
	}
	triangleNum = totalTriangleSize;

	unsigned long long index = 0;
	//populating triangleIdArr from each item of triangleIdArrList
	for(std::list<nodeTriangleIdArr>::iterator it=triangleIdArrList.begin(); it!=triangleIdArrList.end(); it++){
		unsigned long long *subTriangleArr = (*it).triangleIdArr;
		unsigned long long subTriangleArrSize = (*it).triangleNum;
		for(unsigned long long i=0; i<subTriangleArrSize; i++){
			triangleIdArr[index*3] = subTriangleArr[i*3];
			triangleIdArr[index*3+1] = subTriangleArr[i*3+1];
			triangleIdArr[index*3+2] = subTriangleArr[i*3+2];
//std::cout<<triangleIdArr[index*3]<<" ";
			index++;
		}
		//remove subTriangleArr in triangleIdArrList
		delete [] (*it).triangleIdArr;
	}
	triangleIdArrList.clear();
}

//=============================================================================
//transform list of triangleCoorArr (triangleCoorArrList) into flat array (triangleCoorArr)
void transformTriangleCoorArrList(std::list<nodeTriangleCoorArr> &triangleCoorArrList, double *&triangleCoorArr, unsigned long long &triangleNum){

	if(triangleCoorArrList.size()==0) return;
	unsigned long long totalTriangleSize = 0;
	//flattening multiple triangleCoorArr in triangleCoorArrList into one triangleCoorArr (triangleCoorArr)
	for(std::list<nodeTriangleCoorArr>::iterator it=triangleCoorArrList.begin(); it!=triangleCoorArrList.end(); it++){
		totalTriangleSize += (*it).triangleNum;
	}

	//triangleNum is total number of triangles
	try {
		triangleCoorArr = new double[totalTriangleSize*6];
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory allocated for triangleCoorArr overflow!!!!!!!!!!!\n";
		exit(1);
	}
	triangleNum = totalTriangleSize;

	unsigned long long index = 0;
	//populating triangleCoorArr from each item of triangleCoorArrList
	for(std::list<nodeTriangleCoorArr>::iterator it=triangleCoorArrList.begin(); it!=triangleCoorArrList.end(); it++){
		double *subTriangleArr = (*it).triangleCoorArr;
		unsigned long long subTriangleArrSize = (*it).triangleNum;
		for(unsigned long long i=0; i<subTriangleArrSize; i++){
			triangleCoorArr[index*6] = subTriangleArr[i*6];
			triangleCoorArr[index*6+1] = subTriangleArr[i*6+1];
			triangleCoorArr[index*6+2] = subTriangleArr[i*6+2];
			triangleCoorArr[index*6+3] = subTriangleArr[i*6+3];
			triangleCoorArr[index*6+4] = subTriangleArr[i*6+4];
			triangleCoorArr[index*6+5] = subTriangleArr[i*6+5];
//std::cout<<triangleCoorArr[index*3]<<" ";
			index++;
		}
		//remove subTriangleArr in triangleCoorArrList
		delete [] (*it).triangleCoorArr;
	}
	triangleCoorArrList.clear();
}

//=============================================================================
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectStoreTriangleIdArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned int *rankIdArr, unsigned long long *triangleIdArr, unsigned long long triangleNum, std::string outputPath, double &storeTime){
	double currentTime;
	storeTime = 0;
	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned long long *storeTriangleNumArr = NULL;
	unsigned long long *storeTriangleIdArr = NULL;
	//based on storeTriangleNumArr, compute storeTriangleNumOffsetArr
	unsigned long long *storeTriangleNumOffsetArr = NULL;

	if((world_rank==MASTER_RANK)){//only master node
		storeTriangleNumArr = new unsigned long long[groupNum];
		storeTriangleNumArr[0] = triangleNum;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to master (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, data_tag1, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleNumArr[i], 1, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], data_tag1, MPI_COMM_WORLD, &status);
	}

//	MPI_Gather(&triangleNum, 1, MPI_UNSIGNED, storeTriangleNumArr, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

	//process offset of each triangleIdArr of each subMaster
	if((world_rank==MASTER_RANK)){//only master node
		storeTriangleNumOffsetArr = new unsigned long long[groupNum];
		storeTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<activeCoarsePartNum; i++)
			storeTriangleNumOffsetArr[i] = storeTriangleNumOffsetArr[i-1] + storeTriangleNumArr[i-1];
	}

	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, data_tag2, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		unsigned long long storeTriangleIdArrSize = 0;
		//calculate the size of storeTriangleIdArr that collect from other rank
		for(int i=1; i<activeCoarsePartNum; i++)
			storeTriangleIdArrSize += storeTriangleNumArr[i];
		//add the number of stored triangles from master
		storeTriangleIdArrSize += triangleNum;

		//in case we do not want to collect ans store triangles in master node
		//if(storeTriangleIdArrSize==0) return;

		//allocate memory for storeTriangleIdArr
		try {
			storeTriangleIdArr = new unsigned long long[storeTriangleIdArrSize*3];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!!delaunay2DMain237\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			storeTriangleIdArr[index*3] = triangleIdArr[index*3];
			storeTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			storeTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];
		}

		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&storeTriangleIdArr[storeTriangleNumOffsetArr[i]*3], storeTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], data_tag2, MPI_COMM_WORLD, &status);

		currentTime = MPI_Wtime();
		std::cout<<">>>>>Writing store triangles to file triangleIds.tri<<<<<<\n";
		std::string currPath = outputPath + "/triangleIds.tri";

		storeTriangleIds(storeTriangleIdArr, storeTriangleIdArrSize, currPath, "a");

		delete [] storeTriangleIdArr;
		storeTriangleIdArr = NULL;
		storeTime += MPI_Wtime() - currentTime;
	}

	if(row_rank==SUB_MASTER_RANK) delete [] triangleIdArr;

	if(world_rank==MASTER_RANK){ //only master node
		delete [] storeTriangleNumArr;
		delete [] storeTriangleNumOffsetArr;
		delete [] storeTriangleIdArr;
	}

//	if(world_rank==MASTER_RANK) std::cout<<"&&& Mater time for collectStoreTriangleIdArr: "<<masterTime<<"\n";
}


//=============================================================================
//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
void collectBoundaryTriangleArr(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned int groupNum, unsigned int activeCoarsePartNum, unsigned int *rankIdArr, unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, std::string outputPath, double &storeTime){
	double currentTime;
	storeTime = 0;
	int ierr;
	MPI_Status status;
	//number of store triangles of each process
	unsigned long long *boundaryTriangleNumArr = NULL;
	unsigned long long *boundaryTriangleIdArr = NULL;
	double *boundaryTriangleCoorArr = NULL;
	//based on boundaryTriangleNumArr, compute boundaryTriangleNumOffsetArr
	unsigned long long *boundaryTriangleNumOffsetArr = NULL;

	if((world_rank==MASTER_RANK)){//only master node
		boundaryTriangleNumArr = new unsigned long long[groupNum];
		boundaryTriangleNumArr[0] = triangleNum;
	}

	//collect triangleNum from each subMaster (row_rank==SUB_MASTER_RANK) to superMaster (world_rank==MASTER_RANK)
	if((row_rank==SUB_MASTER_RANK)&&(world_rank!=MASTER_RANK)){
		ierr = MPI_Send(&triangleNum, 1, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, data_tag1, MPI_COMM_WORLD);
	}else if(world_rank==MASTER_RANK){
		for(int i=1; i<groupNum; i++)
			ierr = MPI_Recv(&boundaryTriangleNumArr[i], 1, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], data_tag1, MPI_COMM_WORLD, &status);
	}

	//process offset of each triangleIdArr of each subMaster
	if((world_rank==MASTER_RANK)){//only master node
		boundaryTriangleNumOffsetArr = new unsigned long long[groupNum];
		boundaryTriangleNumOffsetArr[0] = 0;
		for(int i=1; i<activeCoarsePartNum; i++)
			boundaryTriangleNumOffsetArr[i] = boundaryTriangleNumOffsetArr[i-1] + boundaryTriangleNumArr[i-1];
	}

	if((row_rank==SUB_MASTER_RANK)&&(world_rank>MASTER_RANK)){//send triangleIdArr to master
		ierr = MPI_Send(triangleIdArr, triangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, data_tag2, MPI_COMM_WORLD);
		ierr = MPI_Send(triangleCoorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, data_tag2, MPI_COMM_WORLD);
	}
	else if(world_rank==MASTER_RANK){//master rank receives all triangleIdArr from other ranks
		unsigned long long boundaryTriangleArrSize = 0;
		//calculate the size of boundaryTriangleIdArr that collect from other rank
		for(int i=1; i<activeCoarsePartNum; i++)
			boundaryTriangleArrSize += boundaryTriangleNumArr[i];
		//add the number of stored triangles from master
		boundaryTriangleArrSize += triangleNum;

		//allocate memory for boundaryTriangleIdArr
		try {
			boundaryTriangleIdArr = new unsigned long long[boundaryTriangleArrSize*3];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!!delaunay2DMain\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to boundaryTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			boundaryTriangleIdArr[index*3] = triangleIdArr[index*3];
			boundaryTriangleIdArr[index*3+1] = triangleIdArr[index*3+1];
			boundaryTriangleIdArr[index*3+2] = triangleIdArr[index*3+2];
		}

		//allocate memory for boundaryTriangleCoorArr
		try {
			boundaryTriangleCoorArr = new double[boundaryTriangleArrSize*6];
		} catch (std::bad_alloc&) {
		  //Handle error
			std::cout<<"Memory overflow!!!!!!!!!!!!delaunay2DMain\n";
			exit(1);
		}
		//fill triangleIds from triangleIdArr of master to storeTriangleIdArr
		for(unsigned long long index=0; index<triangleNum; index++){
			boundaryTriangleCoorArr[index*6] = triangleCoorArr[index*6];
			boundaryTriangleCoorArr[index*6+1] = triangleCoorArr[index*6+1];
			boundaryTriangleCoorArr[index*6+2] = triangleCoorArr[index*6+2];
			boundaryTriangleCoorArr[index*6+3] = triangleCoorArr[index*6+3];
			boundaryTriangleCoorArr[index*6+4] = triangleCoorArr[index*6+4];
			boundaryTriangleCoorArr[index*6+5] = triangleCoorArr[index*6+5];
		}

		for(int i=1; i<groupNum; i++){
			ierr = MPI_Recv(&boundaryTriangleIdArr[boundaryTriangleNumOffsetArr[i]*3], boundaryTriangleNumArr[i]*3, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], data_tag2, MPI_COMM_WORLD, &status);
			ierr = MPI_Recv(&boundaryTriangleCoorArr[boundaryTriangleNumOffsetArr[i]*6], boundaryTriangleNumArr[i]*6, MPI_DOUBLE, rankIdArr[i], data_tag2, MPI_COMM_WORLD, &status);
		}

		currentTime = MPI_Wtime();
		std::cout<<">>>>>Writing boundary triangles to file boundaryIds.tri and boundaryCoors.tri <<<<<<\n";

		std::string currPath1 = outputPath + "/boundaryIds.tri";
		std::string currPath2 = outputPath + "/boundaryCoors.tri";
std::cout<<"number of boundaryTriangles to store: "<<boundaryTriangleArrSize<<"\n";
		storeTriangleIds(boundaryTriangleIdArr, boundaryTriangleArrSize, currPath1, "a");
		storeTriangleCoors(boundaryTriangleCoorArr, boundaryTriangleArrSize, currPath2, "a");

		delete [] boundaryTriangleIdArr;
		delete [] boundaryTriangleCoorArr;
		boundaryTriangleIdArr = NULL;
		boundaryTriangleCoorArr = NULL;
		storeTime += MPI_Wtime() - currentTime;		
	}

	if(row_rank==SUB_MASTER_RANK){
		delete [] triangleIdArr;
		delete [] triangleCoorArr;
	}

	if(world_rank==MASTER_RANK){ //only master node
		delete [] boundaryTriangleNumArr;
		delete [] boundaryTriangleNumOffsetArr;
	}
// 	if(world_rank==MASTER_RANK) std::cout<<"&&& Mater time for collectBoundaryTriangleArr: "<<masterTime<<"\n";
}



//===================================================================
//read info about the current coarse partition: 
void readPointPartInfo(unsigned coarsePartId, unsigned xCoarsePartNum, unsigned yCoarsePartNum, double *&infoArr, unsigned *&pointNumArr, std::string inputPath){
	//Read information from pointPartInfoXX.xfdl
	std::string fileInfoStr = generateFileName(coarsePartId, inputPath + "/pointPartInfo", xCoarsePartNum*yCoarsePartNum, ".xfdl");
	std::ifstream partInfoFile(fileInfoStr.c_str());
	if(!partInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	//xFinePartNum, yFinePartNum, low point and high point, initPointNum (7 variables)
	infoArr = new double[7];
	unsigned xFinePartNum, yFinePartNum;
	std::string strItem;
	//first line --> read xFinePartNum, yFinePartNum
	partInfoFile >> strItem;
	infoArr[0] = atoi(strItem.c_str());
	partInfoFile >> strItem;
	infoArr[1] = atoi(strItem.c_str());
	xFinePartNum = infoArr[0];
	yFinePartNum = infoArr[1];

	//skip second line --> 4 coordinates of low point and high point of current partition
	for(int i=0; i<4; i++){
		partInfoFile >> strItem;
		infoArr[i+2] = atoi(strItem.c_str());
	}

	//skip third line --> number of init points
	partInfoFile >> strItem;
	infoArr[6] = atoi(strItem.c_str());

	//fourth line --> number of points in fine-grained partitions of current coarse partition
	pointNumArr = new unsigned int[xFinePartNum*yFinePartNum];
	for(unsigned int i=0; i<xFinePartNum*yFinePartNum; i++){
		partInfoFile >> strItem;
		pointNumArr[i] = atoi(strItem.c_str());
	}
	partInfoFile.close();
}

//=============================================================================
//read coordinate points from all files pointPartXX_YY.ver in a coarse active partition
//Input: coarsePartId, xCoarsePartNum, yCoarsePartNum
//Output:
//	- pointCoorArr --> all coordinate point in a coarse active partition, each point has two coordinate value (x, y)
//	- pointNumArr --> number of point in each fine partition in a coarse active partition
//	- chunkNum --> number of file in a coarse active partition
void readPointInCoarsePartition(unsigned coarsePartId, unsigned xCoarsePartNum, unsigned yCoarsePartNum, double *&infoArr, double *&initPointCoorArr, unsigned long long *&initPointIdArr, unsigned &initPointNum, double *&pointCoorArr, unsigned long long *&pointIdArr, unsigned *&pointNumArr, unsigned &chunkNum, std::string inputPath, double &readTime){
	readTime = 0;
	double currentTime;

	currentTime = MPI_Wtime();
	std::string fileStr;
	point *initPointArr = NULL;
	point *pointArr = NULL;
	unsigned pointNum = 0;
//	double infoArr[7];
	unsigned xFinePartNum, yFinePartNum;

	//read coarse partition info
	readPointPartInfo(coarsePartId, xCoarsePartNum, yCoarsePartNum, infoArr, pointNumArr, inputPath);
	xFinePartNum = infoArr[0];
	yFinePartNum = infoArr[1];
	chunkNum = xFinePartNum*yFinePartNum;
	initPointNum = infoArr[6];

	//calculate number of points in a coarse partition
	for(int i=0; i<chunkNum; i++){
		pointNum += pointNumArr[i];
	}
	pointArr = new point[pointNum];
	unsigned int offset = 0;

	currentTime = MPI_Wtime();
	//read init points for each coarse partition
	fileStr = generateFileName(coarsePartId, inputPath + "/initPointPart", xCoarsePartNum*yCoarsePartNum, ".ver");
	//initPointArr = new point[initPointNum];
	readPoints(initPointArr, initPointNum, fileStr);

	//read all points in all fine partitions in a coarse partition
	for(int finePartId=0; finePartId<chunkNum; finePartId++){
		if(pointNumArr[finePartId]!=0){
			fileStr = generateFileName(coarsePartId, inputPath + "/pointPart", xCoarsePartNum*yCoarsePartNum, "");
			fileStr = generateFileName(finePartId, fileStr + "_", xFinePartNum*yFinePartNum,".ver");
			point *tempPointArr = pointArr + offset;
			readPoint_withoutAllocation(tempPointArr, pointNumArr[finePartId], fileStr);
			offset += pointNumArr[finePartId];
		}
	}
	readTime += MPI_Wtime() - currentTime;

	initPointCoorArr = new double[initPointNum*2];
	initPointIdArr = new unsigned long long[initPointNum];
	//transform point into  (double, double)
	for(unsigned int i=0; i<initPointNum; i++){
		initPointCoorArr[i*2] = initPointArr[i].getX();
		initPointCoorArr[i*2+1] = initPointArr[i].getY();
		initPointIdArr[i] = initPointArr[i].getId();
	}
	delete [] initPointArr;

	pointCoorArr = new double[pointNum*2];
	pointIdArr = new unsigned long long[pointNum];
	//transform point into  (double, double)
	for(unsigned int i=0; i<pointNum; i++){
		pointCoorArr[i*2] = pointArr[i].getX();
		pointCoorArr[i*2+1] = pointArr[i].getY();
		pointIdArr[i] = pointArr[i].getId();
	}
	delete [] pointArr;
}

//=============================================================================
//Load init points and point data from files (initPointPartXX.ver, pointPartXX_YY.ver) by master and send to sub-master
//input: activePartNum, coarsePartId, activePartIdArr, pointNumPartArr fofr each sub-master, groupNum is number of sub-masters
void loadAllPointForSubMasters(unsigned int world_rank, unsigned int world_size, unsigned int row_rank, unsigned int row_size, unsigned xCoarsePartNum, unsigned yCoarsePartNum, unsigned int activeCoarsePartNum, unsigned int *rankIdArr, unsigned int *coarsePartIdArr, double *&infoArr, double *&initPointCoorArr, unsigned long long *&initPointIdArr, unsigned &initPointNum, double *&pointCoorArr, unsigned long long *&pointIdArr, unsigned *&pointNumArr, unsigned &chunkNum, std::string inputPath, double &readTime){

	readTime = 0;
	double currentTime;

	double amountTime;
	int ierr;
	MPI_Status status;

	double *readInfoArr=NULL;
	double *readInitPointCoorArr=NULL;//master read all init point coordinates (initPointPartXX.ver)
	unsigned long long *readInitPointIdArr=NULL;//master read all init point Ids (initPointPartXX.ver)
	unsigned readInitPointNum;//number of init points

	double *readPointCoorArr=NULL;//master read all point coordinates from coarse active partitions
	unsigned long long *readPointIdArr=NULL;//master read all point Ids from coarse active partitions
	unsigned *readPointNumArr=NULL;//master read number of points in fine partitions of a coarse active partition
	unsigned readChunkNum;//master read number of files (chunks) in a coarse pactive partition

	//If group has only one process (sub-master, no workers)
	if(activeCoarsePartNum<=1) return;

	if(world_rank==MASTER_RANK){	//read for master only
		readPointInCoarsePartition(coarsePartIdArr[0], xCoarsePartNum, yCoarsePartNum, infoArr, initPointCoorArr, initPointIdArr, initPointNum, pointCoorArr, pointIdArr, pointNumArr, chunkNum, inputPath, amountTime);
		readTime += amountTime;
	}

	//read all fine partitions in a coarse active partition, then send to a sub-master
	//content to send inclusing all coordinate points in a coarse active partition and the array of sizes of each fine partition
	if(world_rank==MASTER_RANK){
		for(int i=1; i<activeCoarsePartNum; i++){
			readPointInCoarsePartition(coarsePartIdArr[i], xCoarsePartNum, yCoarsePartNum, readInfoArr, readInitPointCoorArr, readInitPointIdArr, readInitPointNum, readPointCoorArr, readPointIdArr, readPointNumArr, readChunkNum, inputPath, amountTime);
			readTime += amountTime;

			MPI_Send(readInfoArr, 7, MPI_DOUBLE, rankIdArr[i], data_tag1, MPI_COMM_WORLD);
			MPI_Send(&readInitPointNum, 1, MPI_UNSIGNED, rankIdArr[i], data_tag1, MPI_COMM_WORLD);
			MPI_Send(readInitPointCoorArr, readInitPointNum*2, MPI_DOUBLE, rankIdArr[i], data_tag2, MPI_COMM_WORLD);
			MPI_Send(readInitPointIdArr, readInitPointNum, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], data_tag3, MPI_COMM_WORLD);

			MPI_Send(&readChunkNum, 1, MPI_UNSIGNED, rankIdArr[i], data_tag1, MPI_COMM_WORLD);
			MPI_Send(readPointNumArr, readChunkNum, MPI_UNSIGNED, rankIdArr[i], data_tag2, MPI_COMM_WORLD);
			unsigned pointNum = 0;
			for(unsigned i=0; i<readChunkNum; i++) pointNum += readPointNumArr[i];
			MPI_Send(readPointCoorArr, pointNum*2, MPI_DOUBLE, rankIdArr[i], data_tag3, MPI_COMM_WORLD);
			MPI_Send(readPointIdArr, pointNum, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], data_tag3, MPI_COMM_WORLD);

			delete [] readInfoArr;
			delete [] readInitPointCoorArr;
			delete [] readInitPointIdArr;
			delete [] readPointCoorArr;
			delete [] readPointIdArr;
			delete [] readPointNumArr;
		}
	}else if(row_rank==SUB_MASTER_RANK){
		infoArr = new double[7];
		MPI_Recv(infoArr, 7, MPI_DOUBLE, MASTER_RANK, data_tag1, MPI_COMM_WORLD, &status);
		MPI_Recv(&initPointNum, 1, MPI_UNSIGNED, MASTER_RANK, data_tag1, MPI_COMM_WORLD, &status);
		initPointCoorArr = new double[initPointNum*2];
		MPI_Recv(initPointCoorArr, initPointNum*2, MPI_DOUBLE, MASTER_RANK, data_tag2, MPI_COMM_WORLD, &status);
		initPointIdArr = new unsigned long long[initPointNum];
		MPI_Recv(initPointIdArr, initPointNum, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, data_tag3, MPI_COMM_WORLD, &status);

		MPI_Recv(&chunkNum, 1, MPI_UNSIGNED, MASTER_RANK, data_tag1, MPI_COMM_WORLD, &status);
		pointNumArr = new unsigned int[chunkNum];
		MPI_Recv(pointNumArr, chunkNum, MPI_UNSIGNED, MASTER_RANK, data_tag2, MPI_COMM_WORLD, &status);
		unsigned pointNum = 0;
		for(unsigned i=0; i<chunkNum; i++) pointNum += pointNumArr[i];
		pointCoorArr = new double[pointNum*2];
		pointIdArr = new unsigned long long[pointNum];
		MPI_Recv(pointCoorArr, pointNum*2, MPI_DOUBLE, MASTER_RANK, data_tag3, MPI_COMM_WORLD, &status);
		MPI_Recv(pointIdArr, pointNum, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, data_tag3, MPI_COMM_WORLD, &status);
	}

	//collect the time receiving points of sub-master from masters
	//MPI_Reduce(&pointReceiveTime, &allPointReceiveTime, 1, MPI_DOUBLE, MPI_MAX, MASTER_RANK, MPI_COMM_WORLD);
}

//=============================================================================
//Each active partition in domain will be Delaunay processed as a master in a group
//color is the group Id
//row_rank is the rank in each group
//row_size is the number of processes in each group
//row_comm is communication for that group
//=============================================================================
void delaunayPartition(unsigned int world_rank, unsigned int world_size, unsigned int groupNum, unsigned int *rankIdArr, unsigned int color, unsigned int row_rank, unsigned int row_size, MPI_Comm row_comm, unsigned xCoarsePartNum, unsigned yCoarsePartNum, unsigned *activeCoarsePartIdArr, unsigned *activeCoarsePartSizeArr, unsigned activeCoarsePartNum, double *activeTriangleCoorArr, unsigned long long *activeTriangleIdArr, std::string inputPath, std::string outputPath, double &masterTime, double &masterCollectTime, double &storeTime, double &readTime){
	double currentTime, masterAmountTime, masterCollectAmountTime, storeAmountTime, readAmountTime;
	masterTime = 0;
	masterCollectTime = 0;
	storeTime = 0;
	readTime = 0;

	std::list<nodeTriangleIdArr> storeTriangleIdArrList;
	unsigned long long *storeTriangleIdArr = NULL;
	unsigned long long storeTriangleNum = 0;
	nodeTriangleIdArr storeTriangleIdArrNode;
	storeTriangleIdArrNode.triangleIdArr = NULL;
	storeTriangleIdArrNode.triangleNum = 0;

	std::list<nodeTriangleIdArr> returnTriangleIdArrList;
	unsigned long long *returnTriangleIdArr = NULL;
	unsigned long long returnTriangleNum = 0;
	nodeTriangleIdArr returnTriangleIdArrNode;
	returnTriangleIdArrNode.triangleIdArr = NULL;
	returnTriangleIdArrNode.triangleNum = 0;

	std::list<nodeTriangleCoorArr> returnTriangleCoorArrList;
	double *returnTriangleCoorArr = NULL;
	nodeTriangleCoorArr returnTriangleCoorArrNode;
	returnTriangleCoorArrNode.triangleCoorArr = NULL;
	returnTriangleCoorArrNode.triangleNum = 0;

	std::list<nodeTriangleIdArr> boundaryTriangleIdArrList;
	unsigned long long *boundaryTriangleIdArr = NULL;
	unsigned long long boundaryTriangleNum = 0;
	nodeTriangleIdArr boundaryTriangleIdArrNode;
	boundaryTriangleIdArrNode.triangleIdArr = NULL;
	boundaryTriangleIdArrNode.triangleNum = 0;

	std::list<nodeTriangleCoorArr> boundaryTriangleCoorArrList;
	double *boundaryTriangleCoorArr = NULL;
	nodeTriangleCoorArr boundaryTriangleCoorArrNode;
	boundaryTriangleCoorArrNode.triangleCoorArr = NULL;
	boundaryTriangleCoorArrNode.triangleNum = 0;


	MPI_Barrier(MPI_COMM_WORLD);
	currentTime = MPI_Wtime();

	unsigned int groupId = color;//process Id, multiple processes are divided into many groups
	unsigned int coarsePartId;// partition Id of an active partition
	unsigned int initTriangleNum;//number of init triangles of an active partition
	MPI_Status status;

	//master rank reads info from tempTrianglesCoarseParts.xfdl
	if(world_rank == MASTER_RANK){
		//activeCoarsePartNum = activePartNum;
		//current partId & number of init triangles of master coarse partition
		coarsePartId = activeCoarsePartIdArr[groupId];
		initTriangleNum = activeCoarsePartSizeArr[groupId];
	}

	//then send to all other sub masters (row_rank = 0)
	//copy activeCoarsePartNum, xCoarsePartNum, yCoarsePartNum from world_rank to their ranks
	MPI_Bcast(&activeCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&xCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&yCoarsePartNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	MPI_Bcast(&groupNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//copy second. third values ... in array activePartIdArr, activePartSizeArr to coarsePartId, initTriangleNum of each rank with row_rank==0
	if(world_rank == MASTER_RANK){
		for(int i=1; i<activeCoarsePartNum; i++){
			MPI_Send(&activeCoarsePartIdArr[i], 1, MPI_INT, rankIdArr[i], send_data_tag, MPI_COMM_WORLD);
			MPI_Send(&activeCoarsePartSizeArr[i], 1, MPI_INT, rankIdArr[i], send_data_tag, MPI_COMM_WORLD);
		}
	}
	else if(row_rank==0){
		MPI_Recv(&coarsePartId, 1, MPI_INT, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
		MPI_Recv(&initTriangleNum, 1, MPI_INT, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
	}


	//Send activeTriangleCoorArr and activeTriangleIdArr to sub_masters
	double *activeTriangleCoorArr_submaster;
	unsigned long long *activeTriangleIdArr_submaster;

	unsigned *activePartSizeOffsetCoorArr;
	unsigned *activePartSizeOffsetIdArr;
	if(world_rank == MASTER_RANK){
		//for the master node
		activeTriangleCoorArr_submaster = activeTriangleCoorArr;
		activeTriangleIdArr_submaster = activeTriangleIdArr;

		activePartSizeOffsetCoorArr = new unsigned[activeCoarsePartNum];
		activePartSizeOffsetIdArr = new unsigned[activeCoarsePartNum];
		activePartSizeOffsetCoorArr[0] = 0;
		activePartSizeOffsetIdArr[0] = 0;
		for(int i=1; i<activeCoarsePartNum; i++){
			activePartSizeOffsetCoorArr[i] = activePartSizeOffsetCoorArr[i-1] + activeCoarsePartSizeArr[i-1]*6;
			activePartSizeOffsetIdArr[i] = activePartSizeOffsetIdArr[i-1] + activeCoarsePartSizeArr[i-1]*3;
		}

		for(int i=1; i<activeCoarsePartNum; i++){
			MPI_Send(&activeTriangleCoorArr[activePartSizeOffsetCoorArr[i]], activeCoarsePartSizeArr[i]*6, MPI_DOUBLE, rankIdArr[i], send_data_tag, MPI_COMM_WORLD);
			MPI_Send(&activeTriangleIdArr[activePartSizeOffsetIdArr[i]], activeCoarsePartSizeArr[i]*3, MPI_UNSIGNED_LONG_LONG, rankIdArr[i], send_data_tag, MPI_COMM_WORLD);
		}
	}
	else if(row_rank==0){
		activeTriangleCoorArr_submaster = new double[initTriangleNum*6];
		activeTriangleIdArr_submaster = new unsigned long long[initTriangleNum*3];
		MPI_Recv(activeTriangleCoorArr_submaster, initTriangleNum*6, MPI_DOUBLE, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
		MPI_Recv(activeTriangleIdArr_submaster, initTriangleNum*3, MPI_UNSIGNED_LONG_LONG, MASTER_RANK, send_data_tag, MPI_COMM_WORLD, &status);
	}

	if(world_rank == MASTER_RANK){
		delete [] activePartSizeOffsetCoorArr;
		delete [] activePartSizeOffsetIdArr;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	masterTime += MPI_Wtime() - currentTime;
	currentTime = MPI_Wtime();

	if(world_rank == MASTER_RANK) std::cout<<"<<<<<Reading points from pointPartXX_YY.ver<<<<<<\n";

	double *infoArr_submaster=NULL;//information about a coarse partition (xFinePartNum, yFinePartNum, low point and high point, initPointNum (7 variables))
	double *initPointCoorArr_submaster=NULL;// all init points coordinates of a coarse active partition (sub-master)
	unsigned long long *initPointIdArr_submaster=NULL;// all init point Ids of a coarse active partition (sub-master)	
	unsigned initPointNum;

	//These data are for sub-master, and pointCoorArr_submaster will deliver to workers for Delaunay triangulation (via pMPI)
	double *pointCoorArr_submaster=NULL;// all points coordinates of a coarse active partition (sub-master)
	unsigned long long *pointIdArr_submaster=NULL;// all point Ids of a coarse active partition (sub-master)
	unsigned *pointNumArr_submaster=NULL;//number of points in each fine partition in a coarse active partition
	unsigned chunkNum;// number of fine partitions in a coarse active partition

	loadAllPointForSubMasters(world_rank, world_size, row_rank, row_size, xCoarsePartNum, yCoarsePartNum, activeCoarsePartNum, rankIdArr, activeCoarsePartIdArr, infoArr_submaster, initPointCoorArr_submaster, initPointIdArr_submaster, initPointNum, pointCoorArr_submaster, pointIdArr_submaster, pointNumArr_submaster, chunkNum, inputPath, readAmountTime);
	readTime += readAmountTime;


	MPI_Barrier(MPI_COMM_WORLD);
	masterCollectTime = MPI_Wtime() - currentTime;
	masterCollectTime -= readTime;

	delaunayMPI *pMPI = new delaunayMPI(row_rank, row_size, row_comm, coarsePartId, xCoarsePartNum, yCoarsePartNum, pointCoorArr_submaster, pointIdArr_submaster, pointNumArr_submaster, chunkNum);
	coarsePartition *c;

	if(row_rank == SUB_MASTER_RANK){
		c = new coarsePartition(groupId, coarsePartId, activeCoarsePartNum, initTriangleNum, xCoarsePartNum, yCoarsePartNum, infoArr_submaster, pointNumArr_submaster, initPointCoorArr_submaster, initPointIdArr_submaster, initPointNum, activeTriangleCoorArr_submaster, activeTriangleIdArr_submaster, initTriangleNum);
		c->initTriangulate();
		c->triangleTransform();
	}


	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage
	//Number of current active partitions
	unsigned int activePartNum;

	while(!delaunayStop){
		if(row_rank==SUB_MASTER_RANK){
			c->generateIntersection();
			c->generateConflictPartitions();
			activePartNum = c->generateActivePartitions();

			c->updateConflictPartitions();
			c->deliverTriangles(storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, boundaryTriangleIdArrNode.triangleIdArr, boundaryTriangleCoorArrNode.triangleCoorArr, boundaryTriangleIdArrNode.triangleNum);
			if(storeTriangleIdArrNode.triangleIdArr!=NULL){
				storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
				storeTriangleIdArrNode.triangleIdArr = NULL;
				storeTriangleIdArrNode.triangleNum = 0;
			}
			boundaryTriangleCoorArrNode.triangleNum = boundaryTriangleIdArrNode.triangleNum;
			if(boundaryTriangleIdArrNode.triangleIdArr!=NULL){
				boundaryTriangleIdArrList.push_back(boundaryTriangleIdArrNode);
				boundaryTriangleIdArrNode.triangleIdArr = NULL;
				boundaryTriangleIdArrNode.triangleNum = 0;
			}
			if(boundaryTriangleCoorArrNode.triangleCoorArr!=NULL){
				boundaryTriangleCoorArrList.push_back(boundaryTriangleCoorArrNode);
				boundaryTriangleCoorArrNode.triangleCoorArr = NULL;
				boundaryTriangleCoorArrNode.triangleNum = 0;
			}

			if(c->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
		}
		//sychronize all processes in a group
		MPI_Barrier(row_comm);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);


		unsigned int currActivePartNumLeftOver=0;
		unsigned long long *tempPointIdArr = NULL;
		double *tempCoorArr = NULL;
		unsigned int *activePartIdArr = NULL;
		unsigned int *activePartSizeArr = NULL;
		unsigned int *activePartSizeOffsetArr = NULL;
		unsigned int *pointNumPartArr = NULL;
		unsigned int currActivePartNum;
		unsigned long long totalTriangleSize=0;
		double coarseLowX = 0;
		double coarseLowY = 0;
		double coarseHighX = 0;
		double coarseHighY = 0;
		unsigned int xFinePartNum = 0;
		unsigned int yFinePartNum = 0;

		while(!activePartStop){
			if(row_rank==SUB_MASTER_RANK){
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				c->prepareDataForDelaunayMPI(row_size, tempPointIdArr, tempCoorArr, totalTriangleSize, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr, pointNumPartArr, currActivePartNum);
				coarseLowX = c->lowPoint.getX();
				coarseLowY = c->lowPoint.getY();
				coarseHighX = c->highPoint.getX();
				coarseHighY = c->highPoint.getY();
				xFinePartNum = c->xFinePartNum;
				yFinePartNum = c->yFinePartNum;

				//currActivePartNum is the number of active partition leftover for the next loop.
				//it means that if currActivePartNum==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				currActivePartNumLeftOver = c->activePartitionNumber();
				if(currActivePartNumLeftOver<=0) activePartStop = true;

				//std::cout<<"$$$world_rank: "+toString(world_rank)+", number of active partitions leftover: "+ toString(currActivePartNumLeftOver)+"\n";
				//std::cout<<"Number of active partitions leftover: "+ toString(currActivePartNumLeftOver)+"\n";
			}
			//sychronize all processes
			//MPI_Barrier(row_comm);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, SUB_MASTER_RANK, row_comm);

			//process MPI
			pMPI->processMPI(coarseLowX, coarseLowY, coarseHighX, coarseHighY, xFinePartNum, yFinePartNum, tempPointIdArr, tempCoorArr, totalTriangleSize, activePartIdArr, activePartSizeArr, activePartSizeOffsetArr, pointNumPartArr, currActivePartNum, storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, returnTriangleIdArrNode.triangleIdArr, returnTriangleCoorArrNode.triangleCoorArr, returnTriangleIdArrNode.triangleNum);

			if(row_rank==SUB_MASTER_RANK){
				delete [] tempPointIdArr;
				delete [] tempCoorArr;
				delete [] activePartIdArr;
				delete [] activePartSizeArr;
				delete [] activePartSizeOffsetArr;
				delete [] pointNumPartArr;
				currActivePartNum = 0;
				totalTriangleSize = 0;
			}


			//sychronize all processes
			MPI_Barrier(row_comm);

			if(row_rank==SUB_MASTER_RANK){
				if(storeTriangleIdArrNode.triangleIdArr!=NULL){
					storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
					storeTriangleIdArrNode.triangleIdArr = NULL;
					storeTriangleIdArrNode.triangleNum = 0;
				}
				if((returnTriangleCoorArrNode.triangleCoorArr!=NULL)&&(returnTriangleIdArrNode.triangleIdArr!=NULL)){
					returnTriangleCoorArrNode.triangleNum = returnTriangleIdArrNode.triangleNum;
					returnTriangleIdArrList.push_back(returnTriangleIdArrNode);
					returnTriangleCoorArrList.push_back(returnTriangleCoorArrNode);
					returnTriangleIdArrNode.triangleIdArr = NULL;
					returnTriangleIdArrNode.triangleNum = 0;
					returnTriangleCoorArrNode.triangleCoorArr = NULL;
					returnTriangleCoorArrNode.triangleNum = 0;
				}

			}

			//sychronize all processes
			MPI_Barrier(row_comm);
		}

		if(row_rank==SUB_MASTER_RANK){
			transformTriangleIdArrList(returnTriangleIdArrList, returnTriangleIdArr, returnTriangleNum);
			transformTriangleCoorArrList(returnTriangleCoorArrList, returnTriangleCoorArr, returnTriangleNum);
			c->updateTriangleArr(returnTriangleIdArr, returnTriangleCoorArr, returnTriangleNum);
		}
		//sychronize all sub processes
		MPI_Barrier(row_comm);
	}
	delete pMPI;

	if(row_rank==SUB_MASTER_RANK){
		delete [] infoArr_submaster;
		delete [] initPointCoorArr_submaster;// all init points coordinates of a coarse active partition (sub-master)
		delete [] initPointIdArr_submaster;// all init points Ids of a coarse active partition (sub-master)

		delete [] pointCoorArr_submaster;// all points coordinates of a coarse active partition (sub-master)
		delete [] pointIdArr_submaster;// all points Ids of a coarse active partition (sub-master)
		delete [] pointNumArr_submaster;//number of points in each fine partition in a coarse active partition
	}
	else if(row_rank==SUB_MASTER_RANK){
		delete [] activeTriangleCoorArr_submaster;
		delete [] activeTriangleIdArr_submaster;
	}

	if(row_rank==SUB_MASTER_RANK){
		c->storeAllTriangles(storeTriangleIdArrNode.triangleIdArr, storeTriangleIdArrNode.triangleNum, boundaryTriangleIdArrNode.triangleIdArr, boundaryTriangleCoorArrNode.triangleCoorArr, boundaryTriangleIdArrNode.triangleNum);
		if(storeTriangleIdArrNode.triangleIdArr!=NULL){
			storeTriangleIdArrList.push_back(storeTriangleIdArrNode);
			storeTriangleIdArrNode.triangleIdArr = NULL;
			storeTriangleIdArrNode.triangleNum = 0;	
		}
		boundaryTriangleCoorArrNode.triangleNum = boundaryTriangleIdArrNode.triangleNum;
		if(boundaryTriangleIdArrNode.triangleIdArr!=NULL){
			boundaryTriangleIdArrList.push_back(boundaryTriangleIdArrNode);
			boundaryTriangleIdArrNode.triangleIdArr = NULL;
			boundaryTriangleIdArrNode.triangleNum = 0;
		}
		if(boundaryTriangleCoorArrNode.triangleCoorArr!=NULL){
			boundaryTriangleCoorArrList.push_back(boundaryTriangleCoorArrNode);
			boundaryTriangleCoorArrNode.triangleCoorArr = NULL;
			boundaryTriangleCoorArrNode.triangleNum = 0;
		}
	}


	MPI_Barrier(MPI_COMM_WORLD);
	currentTime = MPI_Wtime();

	//transform list of triangleIdArr (storeTriangleCoorArr) into flat array (triangleIdArr)
	//storeTriangleCoorArr will be ersased after inside function
	//triangleIdArr contains all triangle in a coarsePartition after a stage
	transformTriangleIdArrList(storeTriangleIdArrList, storeTriangleIdArr, storeTriangleNum);
//for(int i=0; i<triangleNum; i++)
//std::cout<<triangleIdArr[i*3]<<" "<<triangleIdArr[i*3+1]<<" "<<triangleIdArr[i*3+2]<<"  ";

	transformTriangleIdArrList(boundaryTriangleIdArrList, boundaryTriangleIdArr, boundaryTriangleNum);
	transformTriangleCoorArrList(boundaryTriangleCoorArrList, boundaryTriangleCoorArr, boundaryTriangleNum);

	//collect store triangles (triangleIdArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
	collectStoreTriangleIdArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, rankIdArr, storeTriangleIdArr, storeTriangleNum, outputPath, storeAmountTime);

	if(world_rank==MASTER_RANK){
		//masterCollectTime += masterAmountTime;
		storeTime += storeAmountTime;		
	}
	//collect boundary triangles (boundaryTriangleIdArr, boundaryTriangleCoorArr) from all row_rank = SUB_MASTER_RANK to master_rank = 0
	collectBoundaryTriangleArr(world_rank, world_size, row_rank, row_size, groupNum, activeCoarsePartNum, rankIdArr, boundaryTriangleIdArr, boundaryTriangleCoorArr, boundaryTriangleNum, outputPath, storeAmountTime);

	if(world_rank==MASTER_RANK){
		delete c;
		//masterCollectTime += masterAmountTime;
		storeTime += storeAmountTime;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	masterCollectTime += MPI_Wtime() - currentTime;
	masterCollectTime -= storeTime;
}


//=============================================================================
int main (int argc, char** argv){
//=============================================================================

    int world_rank, world_size;//world_rank --> world_rank
	double domainSize;
	std::string inputPath;
	std::string outputPath;

	if(argc<=4){
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first and second argument are source and result paths to the dataset\n";
		std::cout<<"The third argument is the domain size (both sides)\n";
		std::cout<<"The fourth argument is the adaptive process distribution or equal process distribution\n";
		return 0;
	}

	inputPath = argv[1];
	outputPath = argv[2];
	domainSize = atof(argv[3]);

	//process distribution styles (adaptive or equal)
	//adaptive: processes distributed for each group based on number of point in that coase partition
	//equal process distribution: processes distributed are equal to all groups
	bool processDistribution = atoi(argv[4]);

	domain *d;

	double loadInitPointTime = 0, initialTime = 0, masterTime = 0, masterCollectTime = 0; //master time for triangle collection only
	double updateTime = 0, storeTime = 0, readTime = 0, amountTime = 0, currentTime, overAllTime;
	unsigned int xFinePartNum, yFinePartNum;

	unsigned int *colorArr;// each item is a groupId array
	unsigned int *rankIdArr;//each item isa world_rank such that row_rank==0
	unsigned int color;

	//active patitions data
	unsigned *activePartIdArr=NULL;
	unsigned int *activePartSizeArr=NULL;
	unsigned activePartNum;
	double *activeTriangleCoorArr=NULL;
	unsigned long long *activeTriangleIdArr=NULL;
	unsigned xCoarsePartNum, yCoarsePartNum;


	//ACTION - start do to parallel
	//Initialize MPI.
    MPI_Init(&argc, &argv);

	overAllTime = MPI_Wtime();

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	//world_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if(world_rank==MASTER_RANK){
		currentTime = MPI_Wtime();
		d = new domain(0.0,0.0, domainSize, domainSize, inputPath, outputPath);
		d->loadInitPoints();
		loadInitPointTime += MPI_Wtime()-currentTime;

		d->readFinePartitionSize(xFinePartNum, yFinePartNum);
		std::cout<<"=======================================================\n";
		std::cout<<"Triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<std::endl;
		std::cout<<"=======================================================\n";

		std::cout<<"Time for loadInitPoints() is "<<loadInitPointTime<<std::endl;

		currentTime = MPI_Wtime();
		d->initTriangulate();

		d->triangleTransform();
		initialTime += MPI_Wtime()-currentTime;
		std::cout<<"Time for initialTime is "<<initialTime<<std::endl;
	}

	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage


	int stage = 1;
	//Number of current active partitions
	while(!delaunayStop){//when still have coarse active partitions are waiting to process
		if(world_rank==MASTER_RANK){
			double amountTime1, amountTime2;
			currentTime = MPI_Wtime();
			d->generateIntersection();
			d->generateConflictPartitions();
			//Number of real active partitions
			activePartNum = d->generateActivePartitions();
			d->updateConflictPartitions();
			masterTime += MPI_Wtime()-currentTime;

			d->deliverTriangles(amountTime1, amountTime2);
			masterTime += amountTime1;
			storeTime += amountTime2;

			if(d->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
			std::cout<<"*****************************[[[ STAGE: "<<stage<<" ]]]******************, # active partitions: "<<activePartNum<<std::endl;

		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

		int currActivePartNum, activePartNumLeftOver;
		//groupSize: number of processes in a group, groupNum: number of groups
		int groupSize, groupNum;
		while(!activePartStop){//if number of coarse partitions is greater than number of processes --> multiple shifts
			if(world_rank==MASTER_RANK){
				double amountTime1, amountTime2;
				//collect triangles for the group of active partitions
				unsigned int *activePartPointSizeArr;//point numbers of active partitions
				currActivePartNum = d->prepareDataForDelaunayMPI(world_size, activePartPointSizeArr, xCoarsePartNum, yCoarsePartNum, activePartIdArr, activePartSizeArr, activePartNum, activeTriangleCoorArr, activeTriangleIdArr, amountTime1);
				masterTime += amountTime1;

				//number of processes in a group
				groupSize = world_size/currActivePartNum;

				groupNum = currActivePartNum;
				if(processDistribution==1)
					apdaptiveAllocateGroupProcesses(activePartPointSizeArr, world_size, currActivePartNum, rankIdArr, colorArr);
				else equalAllocateGroupProcesses(world_size, currActivePartNum, rankIdArr, colorArr);

				std::cout<<"groupSize: " + toString(groupSize) + ", groupNum: " + toString(groupNum)<<"\n";
				delete [] activePartPointSizeArr;

				//activePartNumLeftOver is then umber of active partition leftover for the next loop.
				//it means that if activePartNumLeftOver==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				activePartNumLeftOver = d->activePartitionNumber();
				if(activePartNumLeftOver<=0) activePartStop = true;
			}
			//sychronize all processes
//			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
			MPI_Bcast(&groupSize, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
			MPI_Bcast(&groupNum, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

			//send groupId to all processes
			MPI_Scatter(colorArr, 1, MPI_UNSIGNED, &color, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

			//Split the world communicator based on the currActivePartNum into multiple communicatiors
			MPI_Comm row_comm;
			int row_rank, row_size;

			MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);
			MPI_Comm_rank(row_comm, &row_rank);
			MPI_Comm_size(row_comm, &row_size);

			//Processes that are out of scope --> do nothing
			//for ex: 17 processes --> 5 groups: 0, 1, 2, 3, 4 (group 4 work but will stop very soon, with world_rank=16)
			//process multiple masters, each master process a coarsePartition
			double amountTime1=0, amountTime2=0, amountTime3, amountTime4;
			delaunayPartition(world_rank, world_size, groupNum, rankIdArr, color, row_rank, row_size, row_comm, xCoarsePartNum, yCoarsePartNum, activePartIdArr, activePartSizeArr, activePartNum, activeTriangleCoorArr, activeTriangleIdArr, inputPath, outputPath, amountTime1, amountTime2, amountTime3, amountTime4);

			if(world_rank==MASTER_RANK){
				masterTime += amountTime1;
				masterCollectTime += amountTime2;
				storeTime += amountTime3;
				readTime += amountTime4;
			}

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Comm_free(&row_comm);
		}//end while(!activePartStop)

		if(world_rank==MASTER_RANK){
			delete [] colorArr;
			delete [] rankIdArr;
			delete [] activePartIdArr;
			delete [] activePartSizeArr;
			delete [] activeTriangleCoorArr;
			delete [] activeTriangleIdArr;

			currentTime = MPI_Wtime();
			d->updateTriangleArr();
			amountTime = MPI_Wtime()-currentTime;
			updateTime += amountTime;

			stage++;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);

	}//end while(!delaunayStop)

	if(world_rank==MASTER_RANK){
		currentTime = MPI_Wtime();
		d->storeAllTriangles();//store triangles leftover in domain

		amountTime = MPI_Wtime()-currentTime;
		storeTime += amountTime;

		std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		overAllTime = MPI_Wtime()-overAllTime;

		std::cout<<"====================================================================\n";
		std::cout<<"done!!!"<<std::endl;
		std::cout<<"Multi-Master, uniform data version\n";
		std::cout<<"datasources: "<<inputPath<<std::endl;
		std::cout<<"Delaunay output: "<<outputPath<<std::endl;
		std::cout<<"Number of processes: "<<world_size<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<std::endl;
		unsigned int  pointPartNum = d->pointNumMax/(d->xPartNum*d->yPartNum*xFinePartNum*yFinePartNum);
		std::cout<<"#Points/sub-partition: "<<pointPartNum<<"\n";
		std::cout<<"Load initial points time: "<<loadInitPointTime<<"\n";
		std::cout<<"Initial time: "<<initialTime<<"\n";
		std::cout<<"Active Partitions time: "<<masterTime<<"\n";
		std::cout<<"Update time: "<<updateTime<<"\n";
		std::cout<<"Triangles collected time: "<<masterCollectTime<<"\n";
		std::cout<<"Store time: "<<storeTime<<"\n";
		std::cout<<"Read time: "<<readTime<<"\n";
		double realMasterTime = initialTime + masterTime + updateTime;
		std::cout<<"Load points + Triangles collection + Read time + Store time: "<<loadInitPointTime + masterCollectTime + readTime + storeTime<<"\n";
		std::cout<<"Master time = Initial + active Partitions + Update time: "<<realMasterTime<<"\n";
		double workerTime = overAllTime - (loadInitPointTime + realMasterTime + masterCollectTime + storeTime + readTime);
		std::cout<<"Workers time: "<<workerTime<<"\n";
		std::cout<<"Master + Workers time: "<<realMasterTime + workerTime <<"\n";
		std::cout<<"I/O time: "<<readTime + storeTime<<"\n";
		std::cout<<"Overall time: "<<overAllTime<<"\n";
		std::cout<<"====================================================================\n";

		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
		resultFile<<"\n\Multi-Master"<<", "<<inputPath<<", "<<d->xPartNum<<" x "<<d->yPartNum<<" x "<<xFinePartNum<<" x "<<yFinePartNum<<", #Points/sub-partition: "<<pointPartNum<<",  #processes: "<< world_size<<",\nloadInitPointTime: "<<loadInitPointTime<<", initialTime: "<<initialTime<<", activePartitionTime: "<<masterTime<<", updateTime: "<<updateTime<<", masterCollectTime: "<<masterCollectTime<<", readTime: "<<readTime<<", storeTime: "<<storeTime<<",\nloadInitPointTime + masterCollectTime + readTime + storeTime: "<<loadInitPointTime+masterCollectTime+readTime+storeTime<<", master time: "<<realMasterTime<<", workerTime: "<<workerTime<<", masterTime & workerTime: "<<realMasterTime + workerTime<<", I/O time: "<<storeTime+readTime<<", overAllTime: "<<overAllTime<<"\n";

		resultFile.close();
	}
	if(world_rank==MASTER_RANK) delete d;
    MPI_Finalize();
}
