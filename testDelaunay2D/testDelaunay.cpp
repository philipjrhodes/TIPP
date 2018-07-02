//mpic++ -std=gnu++11 testDelaunay.cpp -o testDelaunay

#include <mpi.h>
#include <iostream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

#define MASTER_RANK 0
#define tag 1000

using namespace std;

//==================================================================================
void quit(std::string str){
	std::cout<<"There is not enough memory to allocate for "<<str<<std::endl;
	exit(1);
}
//==================================================================================
void minMax(unsigned long long *intArr, unsigned int intArrSize, unsigned long long *min, unsigned long long *max){
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
double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3){
	return fabs((x1-x3)*(y2-y1) - (x1-x2)*(y3-y1))/2;
}
//==================================================================================
void updateBitmap(unsigned long long *pointIdArr, unsigned int pointIdArrSize, bool *bitmap, int bitmapSize, unsigned long long memoryThreshold){
	for(unsigned int i=0; i<bitmapSize; i++) bitmap[i] = false;
	for(unsigned int i=0; i<pointIdArrSize; i++){
		int sectionId = pointIdArr[i]/memoryThreshold;
		bitmap[sectionId] = true;
	}
//		cout<<sectionId<<" "<<bitmapSize<<endl;
}

//==================================================================================
//read one time from file to pointCoorArr and attributeArr
//firstPointer, lastPointer is the positions in file
void readDirect(std::string dataFileStr, unsigned int firstPointer, unsigned long int loadingSize, int vertexRecordSize, double *dataArr){
	/*reading data for from binary file mydatabin.veratt to page*/
	FILE *fdata=fopen(dataFileStr.c_str(), "rb");
	if(!fdata){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		exit(1);
	}
	fseek(fdata, firstPointer*vertexRecordSize*sizeof(double), SEEK_SET);
	fread(dataArr, sizeof(double), loadingSize*vertexRecordSize, fdata);
	fclose(fdata);
}

//==================================================================================
//This is a unique LRU load based on pointIdArr, item is a coordinate and attribute
//pointIdArr need to be sort and unique first, then load directly one time based on unique list
//Input: pointIdArr, pointIdArrSize, pointIdArr is an array
//Output: dataArr
//read from files into 2 Arrays: coordinateArr, and attributeArr
void loadDirect(unsigned long long *pointIdArr, unsigned int pointIdArrSize, double *dataArr, unsigned long long memoryThreshold, std::string dataFileStr){
    unsigned int pageNumber;
    int vertexRecordSize = 2;//a triangle has 3 points, each point has two coordinates.
    unsigned long long firstPointer, lastPointer;
    minMax(pointIdArr, pointIdArrSize, &firstPointer, &lastPointer);
    unsigned long long loadingSize = lastPointer-firstPointer+1; 

std::cout.precision(16);
//cout<<"firstPointer: "<<firstPointer<<", lastPointer: "<<lastPointer<<", pointIdArrSize: "<<pointIdArrSize<<" "<<endl;
    
    //for ex: memoryThreshold=100000000
    if(loadingSize<memoryThreshold){
        double *tempArr=new double[loadingSize*vertexRecordSize];
        //read directly to pointCoorArrTemp and attributeArrTemp
        readDirect(dataFileStr, firstPointer, loadingSize, vertexRecordSize, tempArr);
        
        //copy data from dataArr to pointCoorArr and attributeArr
        for(unsigned int cellPointId=0; cellPointId<pointIdArrSize; cellPointId++){
            dataArr[cellPointId*vertexRecordSize] = tempArr[(pointIdArr[cellPointId]-firstPointer)*vertexRecordSize];
            dataArr[cellPointId*vertexRecordSize+1] = tempArr[(pointIdArr[cellPointId]-firstPointer)*vertexRecordSize+1];
//std::cout<<pointIdArr[cellPointId]<<" "<<dataArr[cellPointId*vertexRecordSize]<<" "<<dataArr[cellPointId*vertexRecordSize+1]<<"\n";
        }
        delete [] tempArr;
    }
    else{//not enough memory to load a whole chunk to memory
		//read sequentially each section from coorData file from top to bottom to buffer (dataArr), 
		//then fill the data in buffer to pointCoorArr and attributeArr until all data has been filled

		//firstPointer, lastPointer constitute a range to read from coorData file 
		unsigned long long firstPointer = 0;
		unsigned long long lastPointer = memoryThreshold-1;
		double *tempArr;

		/*Load to triangleCellList from binary file *bin.ver */
		FILE *fdata=fopen(dataFileStr.c_str(), "rb");
		if(!fdata) {std::cout<<"not exist "<<dataFileStr<<std::endl; exit(1);}

		fseek(fdata, 0L, SEEK_END);//move to the end of file

		//maxFilePointerSize is maximum number of data nodes (each data node includes coordinate and attributes)
		unsigned long long maxFilePointerSize = ftell(fdata)/(vertexRecordSize*sizeof(double));
std::cout<<"Nunber points in fullPointPart.ver: "<<maxFilePointerSize<<"\n";

		fseek(fdata, 0L, SEEK_SET);//move back to the begin of file

		//Use bitmap array to set those sections in vertex file need to read
		//Each bitmap item is correspoding to a section in vertex file, it is true or false, 
		//true means the section in file need to read.
		unsigned int bitmapSize = maxFilePointerSize/memoryThreshold;
		if(bitmapSize < (double)maxFilePointerSize/memoryThreshold)
			bitmapSize= maxFilePointerSize/memoryThreshold + 1;
		bool *bitmap = new bool[bitmapSize];

		//update bitmap based on pointIdArr
		updateBitmap(pointIdArr, pointIdArrSize, bitmap, bitmapSize, memoryThreshold);

		for(unsigned int bitmapId=0; bitmapId<bitmapSize; bitmapId++){

			if(lastPointer>=maxFilePointerSize)
				lastPointer = maxFilePointerSize;

			if(bitmap[bitmapId]){//need to read this section on vertex file

				unsigned long int readingSize = lastPointer - firstPointer + 1;
				tempArr = new double[readingSize*vertexRecordSize];

				fseek(fdata, firstPointer*vertexRecordSize*sizeof(double), SEEK_SET);
				//read a section from file to buffer (dataArr)
				fread(tempArr, sizeof(double), readingSize*vertexRecordSize, fdata);

				//fill data from buffer to pointCoorArr and attributeArr
				for(unsigned long int index = 0; index<pointIdArrSize; index++){
					unsigned long int currentPointer = pointIdArr[index];
					if((currentPointer >= firstPointer)&&(currentPointer <= lastPointer)){//can fill
						dataArr[index*vertexRecordSize] = tempArr[(currentPointer-firstPointer)*vertexRecordSize];
						dataArr[index*vertexRecordSize+1] = tempArr[(currentPointer-firstPointer)*vertexRecordSize+1];
//cout<<currentPointer<<" "<<firstPointer<<" "<<lastPointer<<" "<<readingSize<<"   "<<dataArr[index*vertexRecordSize]<<" "<<dataArr[(currentPointer-firstPointer)*vertexRecordSize]<<endl;
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

//=============================================================================
//Each item in TriangleNumArray is a number of triangles for workers
//input: segmentSize(default reading size for each worker),
//		 pool_size (number of tasks in MPI), 
//		 readingSize the number of triangles of current reading
//output: triangleNumArr (number of )
void computeTriangleNumArray(unsigned int *triangleNumArr, unsigned int segmentSize, unsigned long int readingSize, int pool_size){
	if(segmentSize == readingSize){
		unsigned int taskSize = segmentSize/pool_size;
		for(unsigned int i=0; i<pool_size; i++) triangleNumArr[i] = taskSize;
	}
	else if(segmentSize > readingSize){
		unsigned int taskSize = readingSize/pool_size;
		for(unsigned int i=0; i<pool_size; i++) triangleNumArr[i] = taskSize;
		int leftOver = readingSize - taskSize*pool_size;
//std::cout<<"leftOver: "<<leftOver<<", taskSize: "<<taskSize<<"\n";
		unsigned int index = 0;
		//add leftover to each element in array
		while(leftOver>0){
			triangleNumArr[index]++;
			index++;
			leftOver--;
		}
	}
}

//=============================================================================
void readDataFile(std::string triangleIdFileName, std::string triangleCoorFileName, unsigned long long readingPos, unsigned long int *readingSize, unsigned long long totalTriangleNum, double *triangleCoorArr){
	int triangleRecordSize = 3;
	//read file
	FILE *f = fopen(triangleIdFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		exit(1);
		//MPI_Abort(MPI_COMM_WORLD, error);
	}

	if(*readingSize > (totalTriangleNum - readingPos)) 
		*readingSize = totalTriangleNum - readingPos;

//	fseek(f, 0, SEEK_END); // seek to end of file
//	unsigned long long totalTriangleNum = ftell(f)/(3*sizeof(unsigned long long));
	//move pinter to the beginning of the file
//	fseek(f, 0, SEEK_SET);

	unsigned long long *pointIdArr = new unsigned long long[*readingSize*triangleRecordSize];
	fseek(f, readingPos*triangleRecordSize*sizeof(unsigned long long), SEEK_SET);
	fread(pointIdArr, sizeof(unsigned long long), *readingSize*triangleRecordSize, f);
	fclose(f);

//	for(int i=0; i<*readingSize; i++)
//		std::cout<<pointIdArr[i*3]<<" "<<pointIdArr[i*3+1]<<" "<<pointIdArr[i*3+2]<<"\n";

	unsigned long long memoryThreshold = 1000000000;
	loadDirect(pointIdArr, *readingSize*3, triangleCoorArr, memoryThreshold, triangleCoorFileName);

	//check triangle Ids not pass the maximum number of points
	//read triangleCoorFileName, find the max points
	f = fopen(triangleCoorFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		exit(1);
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long totalPointNum = ftell(f)/(2*sizeof(double));
	fclose(f);

	for(unsigned int i=0; i<*readingSize*triangleRecordSize; i++)
		if(pointIdArr[i] >= totalPointNum){
			std::cout<<"The dataset is wrong!!!!";
			exit(1);
		}


	delete [] pointIdArr;
}

//=============================================================================
int readNum(std::string path, unsigned int segmentSize, unsigned long long *totalTriangleNum){
	std::string triangleIdFileName = path + "triangleIds.tri";
	FILE *f = fopen(triangleIdFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		return 0;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	*totalTriangleNum = ftell(f)/(3*sizeof(unsigned long long));
	fclose(f);

std::cout<<"totalTriangleNum = "<<*totalTriangleNum<<std::endl;	
	if(*totalTriangleNum % segmentSize == 0) 
		return *totalTriangleNum/segmentSize;
	else
		return *totalTriangleNum/segmentSize + 1;


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
	std::string triangleIdFileName = path + "triangleIds.tri";
	std::string triangleCoorFileName = path + "fullPointPart.ver";

	double domainSize = atof(argv[2]);//the size of 1 side of the domain
	//segmentSize should be divisible by number of tasks
	//segmentSize is the redaingSize of each read, except the last read if there are some triangles leftover
	unsigned long int segmentSize = atoi(argv[3]);

	//number of triangle for each read from triangleIds.tri
	unsigned long int readingSize = segmentSize;
	//position of file reading
	unsigned long long readingPos = 0;
	//total number of triangle in file triangleIds.tri
	unsigned long long totalTriangleNum;
	//number of triangle of workers received from master
	unsigned int triangleNum;

	//number of triangle coordinates from each read of master from triangleIds.tri
	double *triangleCoorArr;
	//number of triangles for each task in pool_size
	unsigned int *triangleNumArr;
	unsigned int *triangleNumOffsetArr;
	//number of triangle coordinates for each worker
	double *coorArr;

	//number of times to read trianglesIds.tri
	int readTime = 0;

	//the area of triangles calculated from a process
	double triangleAreas = 0;
	double totalTriangleAreas = 0;
	double *triangleAreaArr;

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
		readTime = readNum(path, segmentSize, &totalTriangleNum);
		triangleNumArr = new unsigned int[pool_size];
		triangleCoorArr = new double[segmentSize*6];
		triangleNumOffsetArr = new unsigned int[pool_size];
		triangleAreaArr = new double[pool_size];
		for(unsigned int i=0; i<pool_size; i++) triangleAreaArr[i]=0;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	//send readTime to each workers from master
	MPI_Bcast(&readTime, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

//	std::cout<<my_rank<<"---"<<readTime<<" "<<path<<std::endl;

	readingPos = 0;
	for(int i=0; i<readTime; i++){
//	for(int i=0; i<1; i++){
		//read triangle coordinates from files
		if(my_rank==MASTER_RANK){
			readDataFile(triangleIdFileName, triangleCoorFileName, readingPos, &readingSize, totalTriangleNum, triangleCoorArr);
			computeTriangleNumArray(triangleNumArr, segmentSize, readingSize, pool_size);
			triangleNumOffsetArr[0] = 0;
			for(unsigned int i=1; i<pool_size; i++) triangleNumOffsetArr[i] = triangleNumOffsetArr[i-1] + triangleNumArr[i-1];
			for(unsigned int i=0; i<pool_size; i++) std::cout<<triangleNumArr[i]<<" ";
			std::cout<<std::endl;
			for(unsigned int i=0; i<pool_size; i++) std::cout<<triangleNumOffsetArr[i]<<" ";
			std::cout<<std::endl;
		}
		MPI_Barrier(MPI_COMM_WORLD);
		//send number of triangles all slave nodes and one master
		MPI_Scatter(triangleNumArr, 1, MPI_UNSIGNED, &triangleNum, 1, MPI_UNSIGNED, MASTER_RANK, MPI_COMM_WORLD);

		if(my_rank==MASTER_RANK){
			for(unsigned int process_id=1; process_id<pool_size; process_id++)
				//send triangles coordinates to workers
				ierr = MPI_Send(&triangleCoorArr[triangleNumOffsetArr[process_id]*6], triangleNumArr[process_id]*6, MPI_DOUBLE, process_id, tag, MPI_COMM_WORLD);
			coorArr = triangleCoorArr;
			//calculate the area of triangles
			for(unsigned int i=0; i<triangleNum; i++) 
				triangleAreas += triangleArea(coorArr[i*6], coorArr[i*6+1], coorArr[i*6+2], coorArr[i*6+3], coorArr[i*6+4], coorArr[i*6+5]);
		}
		else{//workers triangles of corrdinates from master
			coorArr = new double[triangleNum*6];
			ierr = MPI_Recv(coorArr, triangleNum*6, MPI_DOUBLE, MASTER_RANK, tag, MPI_COMM_WORLD, &status);
			std::cout.precision(16);

			for(unsigned int i=0; i<triangleNum; i++) 
				triangleAreas += triangleArea(coorArr[i*6], coorArr[i*6+1], coorArr[i*6+2], coorArr[i*6+3], coorArr[i*6+4], coorArr[i*6+5]);

/*
			if(my_rank==3)
				if(i<4)	
					for(int i=0; i<triangleNum; i++) std::cout<<coorArr[i*6]<<" "<<coorArr[i*6+1]<<" "<<coorArr[i*6+2]<<" "<<coorArr[i*6+3]<<" "<<coorArr[i*6+4]<<" "<<coorArr[i*6+5]<<"\n";
				else
					for(int i=0; i<triangleNum; i++) std::cout<<coorArr[i*6]<<" "<<coorArr[i*6+1]<<" "<<coorArr[i*6+2]<<" "<<coorArr[i*6+3]<<" "<<coorArr[i*6+4]<<" "<<coorArr[i*6+5]<<"\n";
*/
		}

		MPI_Gather(&triangleAreas, 1, MPI_DOUBLE, triangleAreaArr, 1, MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);
		//compute the total areas for all triangles in domain
		if(my_rank == MASTER_RANK){
			for(unsigned int i=0; i<pool_size; i++) 
				totalTriangleAreas += triangleAreaArr[i];
			std::cout<<"totalTriangleAreas of loop "<<i<<" is: "<<totalTriangleAreas<<"\n";
			totalTriangleAreas = 0;
		}

		readingPos += readingSize;
		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Gather(&triangleAreas, 1, MPI_DOUBLE, triangleAreaArr, 1, MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);

	//compute the total areas for all triangles in domain
	if(my_rank == MASTER_RANK){
		for(unsigned int i=0; i<pool_size; i++) 
			totalTriangleAreas += triangleAreaArr[i];
		std::cout<<"totalTriangleAreas: "<<totalTriangleAreas<<"\n";
	}

	if(my_rank == MASTER_RANK){
		delete [] triangleCoorArr;
		delete [] triangleNumArr;
		delete [] triangleNumOffsetArr;
	}else
		delete [] coorArr;

    MPI_Finalize();
}
