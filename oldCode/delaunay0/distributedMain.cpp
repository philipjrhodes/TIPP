//g++ -std=gnu++11 common.cpp point.cpp domain.cpp distribute.cpp distributedMain.cpp -o distribute

#include <iostream>
#include "distribute.h"


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	int rowPartitionNum = 0;
	int colPartitionNum = 0;
	int chunkSize = 0;
	int initSize = 0;

	if(argc==1){// no arguments
		std::cout<<"The first two arguments are numbers of partition rows and partition columns\n";
		std::cout<<"The 3th - 4th arguments are source path (../dataSources/10vertices/), point info (mydatabin.ver.xfdl), and point data(mydatabin.ver)\n";

		std::cout<<"The fifth argument is destination path (../dataSources/10vertices/pointPartitions/)\n";
		std::cout<<"The sixth is the chunkSize\n";
		std::cout<<"End of the Program!!!";
	}
	else{

		rowPartitionNum = atoi(argv[1]);
		colPartitionNum = atoi(argv[2]);
		if((rowPartitionNum<=0)||(colPartitionNum<=0)) {std::cout<<"Number of row and column partitions have to be greater than zero\n";}

		std::string sourcePath = argv[3];
		if(sourcePath=="") {std::cout<<"The source path does not exist\n"; exit(1);}

		std::string vertexInfoFile = argv[4];
		if(vertexInfoFile=="") {std::cout<<"The vertexInfoFile does not exist\n"; exit(1);}

		std::string vertexDataFile = argv[5];
		if(vertexInfoFile=="") {std::cout<<"The vertexDataFile does not exist\n"; exit(1);}

		std::string destPath = argv[6];
		if(destPath=="") {std::cout<<"The destination path does not exist\n"; exit(1);}

		chunkSize = atoi(argv[7]);
		if(chunkSize<=0) {std::cout<<"chunk size must greater than zero\n"; exit(1);}

		initSize = atoi(argv[8]);
		if(initSize<=0) {std::cout<<"init size must greater than zero\n"; exit(1);}
	
		distribute dtb(rowPartitionNum, colPartitionNum, sourcePath, vertexInfoFile, vertexDataFile, destPath, chunkSize, initSize);
//		dtb.generateGridPoints();
		dtb.pointsDistribute();
		dtb.sort();
//		dtb.printPointPartitions(i);
//		dtb.checkDuplication();
//		dtb.mergeAllPartitions();
//		dtb.printAllPartitions();
		dtb.mergeAllPartitionsAndInitPoints();
//		dtb.printInitPointsAndPartitions();
		}
	std::cout<<"done!\n";
}

