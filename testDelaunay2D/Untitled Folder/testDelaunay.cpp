#include "testDelaunay.h"

//using namespace std;

//==================================================================================
testDelaunay::testDelaunay(std::string pathInput, double domainSizeInput, unsigned int segmentSizeInput){
	path = pathInput;
	domainSize = domainSizeInput;
	segmentSize = segmentSizeInput;
	readingSize = 0;
	readingPos = 0;
	totalTtriangleArea = 0;
}

//==================================================================================
void testDelaunay::quit(std::string str){
	std::cout<<"There is not enough memory to allocate for "<<str<<std::endl;
	exit(1);
}
//==================================================================================
void testDelaunay::minMax(unsigned long long *intArr, unsigned int intArrSize, unsigned long long *min, unsigned long long *max){
	unsigned long long Min = intArr[0];
	unsigned long long Max = Min;
	for(unsigned int i=0; i<intArrSize; i++){
		if(Min>intArr[i]) Min = intArr[i];
		if(Max<intArr[i]) Max = intArr[i];
	}
	*min = Min;
	*max = Max;
}

//==================================================================================
double testDelaunay::triangleArea(double x1, double y1, double x2, double y2, double x3, double y3){
	return abs((x1-x3)*(y2-y1) - (x1-x2)*(y3-y1))/2;
}
//==================================================================================
void testDelaunay::updateBitmap(unsigned long long *pointIdArr, unsigned int pointIdArrSize, bool *bitmap, int bitmapSize, unsigned int memoryThreshold){
	for(int i=0; i<bitmapSize; i++) bitmap[i] = false;
	for(unsigned int i=0; i<pointIdArrSize; i++){
		int sectionId = pointIdArr[i]/memoryThreshold;
		bitmap[sectionId] = true;
	}
//		cout<<sectionId<<" "<<bitmapSize<<endl;
}

//==================================================================================
//read one time from file to pointCoorArr and attributeArr
//firstPointer, lastPointer is the positions in file
void testDelaunay::readDirect(std::string dataFileStr, unsigned int firstPointer, unsigned int loadingSize, int dataRecordSize, double *dataArr){
	/*reading data for from binary file mydatabin.veratt to page*/
	FILE *fdata=fopen(dataFileStr.c_str(), "rb");
	if(!fdata){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		exit(1);
	}
	fseek(fdata, firstPointer*dataRecordSize*sizeof(double), SEEK_SET);
	fread(dataArr, sizeof(double), loadingSize*dataRecordSize, fdata);
	fclose(fdata);
}

//==================================================================================
//This is a unique LRU load based on pointIdArr, item is a coordinate and attribute
//pointIdArr need to be sort and unique first, then load directly one time based on unique list
//Input: pointIdArr, pointIdArrSize, pointIdArr is an array
//Output: dataArr
//read from files into 2 Arrays: coordinateArr, and attributeArr
void testDelaunay::loadDirect(unsigned long long *pointIdArr, unsigned int pointIdArrSize, double *dataArr, unsigned long long memoryThreshold, std::string dataFileStr){
    unsigned int pageNumber;
    int dataRecordSize = 3*2;//a triangle has 3 points, each point has two coordinates.
    unsigned long long firstPointer, lastPointer;
    minMax(pointIdArr, pointIdArrSize, &firstPointer, &lastPointer);
    unsigned int loadingSize = lastPointer-firstPointer+1; 

//cout<<firstPointer<<" "<<lastPointer<<" "<<pointIdArrSize<<" "<<memoryThreshold<<endl;
    
    //for ex: memoryThreshold=100000000
    if(loadingSize<memoryThreshold){
        double *tempArr=new double[loadingSize*dataRecordSize];
        //read directly to pointCoorArrTemp and attributeArrTemp
        readDirect(dataFileStr, firstPointer, loadingSize, dataRecordSize, tempArr);
        
        //copy data from dataArr to pointCoorArr and attributeArr
        for(int cellPointId=0; cellPointId<pointIdArrSize; cellPointId++){
            dataArr[cellPointId*dataRecordSize] = tempArr[(pointIdArr[cellPointId]-firstPointer)*dataRecordSize];
            dataArr[cellPointId*dataRecordSize+1] = tempArr[(pointIdArr[cellPointId]-firstPointer)*dataRecordSize+1];
        }
        delete [] tempArr;
    }
    else{//not enough memory to load a whole chunk to memory
		//read sequentially each section from coorData file from top to bottom to buffer (dataArr), 
		//then fill the data in buffer to pointCoorArr and attributeArr until all data has been filled

		//firstPointer, lastPointer constitute a range to read from coorData file 
		unsigned int firstPointer = 0;
		unsigned int lastPointer = memoryThreshold-1;
		double *tempArr;

		/*Load to triangleCellList from binary file *bin.ver */
		FILE *fdata=fopen(dataFileStr.c_str(), "rb");
		if(!fdata) {std::cout<<"not exist "<<dataFileStr<<std::endl; exit(1);}

		fseek(fdata, 0L, SEEK_END);//move to the end of file
		unsigned int fileSize = ftell(fdata);

		fseek(fdata, 0L, SEEK_SET);//move back to the begin of file

		//maxFilePointerSize is maximum number of data nodes (each data node includes coordinate and attributes)
		unsigned int maxFilePointerSize = fileSize/(dataRecordSize*sizeof(double));

		//Use bitmap array to set those sections in vertex file need to read
		//Each bitmap item is correspoding to a section in vertex file, it is true or false, 
		//true means the section in file need to read.
		int bitmapSize = maxFilePointerSize/memoryThreshold;
		if(bitmapSize < (double)maxFilePointerSize/memoryThreshold)
			bitmapSize= maxFilePointerSize/memoryThreshold + 1;
		bool *bitmap = new bool[bitmapSize];

		//update bitmap based on pointIdArr
		updateBitmap(pointIdArr, pointIdArrSize, bitmap, bitmapSize, memoryThreshold);

		for(int bitmapId=0; bitmapId<bitmapSize; bitmapId++){

			if(lastPointer>=maxFilePointerSize)
				lastPointer = maxFilePointerSize;

			if(bitmap[bitmapId]){//need to read this section on vertex file

				unsigned int readingSize = lastPointer - firstPointer + 1;
				tempArr = new double[readingSize*dataRecordSize];

				//read a section from file to buffer (dataArr)
				fread(tempArr, sizeof(double), readingSize*dataRecordSize, fdata);

				//fill data from buffer to pointCoorArr and attributeArr
				for(unsigned int index = 0; index<pointIdArrSize; index++){
					unsigned int currentPointer = pointIdArr[index];
					if((currentPointer >= firstPointer)&&(currentPointer <= lastPointer)){//can fill
						dataArr[index*dataRecordSize] = tempArr[(currentPointer-firstPointer)*dataRecordSize];
						dataArr[index*dataRecordSize+1] = tempArr[(currentPointer-firstPointer)*dataRecordSize+1];
//cout<<currentPointer<<" "<<firstPointer<<" "<<lastPointer<<" "<<readingSize<<"   "<<dataArr[index*dataRecordSize]<<" "<<dataArr[(currentPointer-firstPointer)*dataRecordSize]<<endl;
					}
				}
				delete [] tempArr;
			}
			//update firstPointer, lastPointer
			firstPointer = lastPointer + 1;
			lastPointer += memoryThreshold - 1;
		}

		delete [] bitmap;
	    fclose(fdata);
	}
}

/*
//==================================================================================
void loadTriangles(){

    unsigned int chunkSize = loadingSize;
	unsigned int *cellVertexIdList;
	double t0;
	FILE *fCell=fopen(triangleCellIdfilename.c_str(), "rb");
	if(!fCell) {cout<<"not exist "<<triangleCellIdfilename<<endl; exit(1);}

	while(readingPos<triangleListRecordCount-1){

		t0 = timestamp();
        if((readingPos+loadingSize)>triangleListRecordCount-1)

			loadingSize = triangleListRecordCount-readingPos;

        cellVertexIdList = new unsigned int[triangleCellRecordSize*loadingSize];
		if(!cellVertexIdList) quit("cellVertexIdList");

		//read loadingSize of Cells
		fread(cellVertexIdList, sizeof(unsigned int), triangleCellRecordSize*loadingSize, fCell);

		double *pointCoorArr = new double[triangleCellRecordSize*loadingSize*vertexRecordSize];

		loadDirect(cellVertexIdList, triangleCellRecordSize*loadingSize, pointCoorArr, memoryThreshold);

		readingPos = readingPos + chunkSize;
cout<<"============================================================\n";
	}
    fclose(fCell);
	delete [] cellVertexIdList;
}
*/

//=============================================================================
//Each item in TriangleNumArray is a number of triangles for workers
//input: segmentSize(default reading size for each worker),
//		 pool_size (number of tasks in MPI), 
//		 readingSize the number of triangles of current reading
//output: triangleNumArr (number of )
void testDelaunay::computeTriangleNumArray(unsigned int *triangleNumArr, unsigned int segmentSize, unsigned int readingSize, int pool_size){
	if(segmentSize == readingSize){
		int taskSize = segmentSize/pool_size;
		for(int i=0; i<pool_size; i++) triangleNumArr[i] = taskSize;
	}
	else if(segmentSize > readingSize){
		int taskSize = readingSize/pool_size;
		for(int i=0; i<pool_size; i++) triangleNumArr[i] = taskSize;
		int leftOver = readingSize % pool_size;
		int index = 0;
		//add leftover to each element in array
		while(leftOver>0){
			triangleNumArr[index]++;
			index++;
		}
	}
}

//=============================================================================
void testDelaunay::readDataFile(std::string triangleIdFileName, std::string triangleCoorFileName, unsigned long long readingPos, unsigned int *readingSize, unsigned long long totalTriangleNum, double *triangleCoorArr){
	//read file
	FILE *f = fopen(triangleIdFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		exit(1);
		//MPI_Abort(MPI_COMM_WORLD, error);
	}
/*	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long totalTriangleNum = ftell(f)/(3*sizeof(unsigned long long));
	//move pinter to the beginning of the file
	fseek(f, 0, SEEK_SET);
*/
	if(*readingSize > (totalTriangleNum - readingPos)) 
		*readingSize = totalTriangleNum - readingPos;

	unsigned long long *pointIdArr = new unsigned long long[*readingSize];
	fread(pointIdArr, sizeof(unsigned long long), *readingSize*3, f);
	fclose(f);

	unsigned long long memoryThreshold = 100000000;
	loadDirect(pointIdArr, *readingSize*3, triangleCoorArr, memoryThreshold, triangleCoorFileName);

	delete [] pointIdArr;
}

//=============================================================================
int testDelaunay::readNum(std::string path, unsigned int segmentSize){
	std::string triangleIdFileName = path + "triangleIds.tri";
	FILE *f = fopen(triangleIdFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		return 0;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long totalTriangleNum = ftell(f)/(3*sizeof(unsigned long long));
	fclose(f);
	
	if(totalTriangleNum % segmentSize == 0) 
		return totalTriangleNum/segmentSize;
	else
		return totalTriangleNum/segmentSize + 1;
std::cout<<"totalTriangleNum = "<<totalTriangleNum<<std::endl;

}
//=============================================================================
int main (int argc, char** argv){
//=============================================================================

	if(argc<=2){// no arguments
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first argument is the path to the dataset\n";
		std::cout<<"The second argument is the domain size\n";
		std::cout<<"The third argument is the segmentation to read big dataset (each chunk at a time)\n";
		std::cout<<"Notice that the segmentSize should be divisible by number of tasks!!\n";
		return 0;
	}
    int my_rank, pool_size, ierr;
	int coreNum;
	MPI_Status status;

	std::string path = argv[1];
	double domainSize = atof(argv[2]);//the size of 1 side of the domain
	//segmentSize should be divisible by number of tasks
	//segmentSize is the redaingSize of each read, except the last read if there are some triangles leftover
	unsigned long int segmentSize = atoi(argv[3]);

	testDelaunay *t = new testDelaunay(path, domainSize, segmentSize);
	//number of times to read trianglesIds.tri
	int readTime = 0;

	//ACTION - start do to parallel
	//Initialize MPI.
    MPI_Init(&argc, &argv);

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	//pool_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
	MPI_Comm_size(MPI_COMM_WORLD, &pool_size);

	//master node find number of triangles in triangleIds.tri
	if(my_rank == MASTER_RANK){
		readTime = t->readNum(path, segmentSize);
	//	triangleNumArr = new unsigned int[pool_size];
	}

	MPI_Barrier(MPI_COMM_WORLD);
	//send readTime to each workers from master
	MPI_Bcast(&readTime, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	std::cout<<my_rank<<"---"<<readTime<<" "<<path<<std::endl;
//	std::cout<<my_rank<<"---"<<readTime<<" "<<std::endl;


/*	readingPos = 0;
	for(int i=0; i<readTime; i++){
		//read triangle coordinates from files
		if(my_rank==MASTER_RANK){
			readDataFile(triangleIdFileName, triangleCoorFileName, readingPos, &readingSize, totalTriangleNum, triangleCoorArr);
			computeTriangleNumArray(triangleNumArr, segmentSize, readingSize, pool_size);
		}
		MPI_Barrier(MPI_COMM_WORLD);
		//send number of triangles all slave nodes and one master
		MPI_Scatter(triangleNumArr, 1, MPI_INT, &triangleNum, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

		if(my_rank==MASTER_RANK)
			for(int process_id=1; process_id<pool_size; process_id++){
				//send triangles coordinates to workers
				ierr = MPI_Send(&triangleCoorArr[process_id], triangleNumArr[process_id]*6, MPI_DOUBLE, process_id, tag, MPI_COMM_WORLD);
			 coorArr = triangleCoorArr;
			
			}
		else{//workers triangles of corrdinates from master
			coorArr = new double[triangleNum];
			ierr = MPI_Recv(coorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, tag, MPI_COMM_WORLD, &status);
		}
		readingPos += readingSize;
		MPI_Barrier(MPI_COMM_WORLD);
	}
*/
	if(my_rank == MASTER_RANK){
		delete t;
	}else
		delete [] coorArr;

    MPI_Finalize();
}
