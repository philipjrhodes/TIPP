//g++ -std=gnu++11 -O3 common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
//This program generate 4 files initPoints.ver, pointCoorPart.ver, pointIdPart.ver, and pointPartInfo.xfdl
//initPoints.ver contains all init points (which is collected from all partitions) and grid points (NOT include 4 corners)
//pointCoorPart.ver contains all points in all partitions (after extracting out init points, the init point which are extrated out are equal for all partitions)
//pointCoorPart.ver --> {partition 0: (x1, y1) (x2, y2),..} {partition 1: (x3, y4) (x5, y6),..} .... {partition 15: (xn, yn) (xn+1, yn+1),..}
//pointIdPart.ver --> {partition 0: (id1) (id2),..} {partition 1: (id3) (id4),..} .... {partition 15: (idn) (idn+1),..}
//pointPartInfo.xfdl:
//- first line: size of the domain (how many partitions) ex: (4 x 4)
//- second line: number of points in each partition not including init points
//- third line: offsets of number of points in second line
//- fouth line: total number of init points, each partition has the same number of init point as the others
//- fifth line: number of grid points (not include 4 corner points) of domain
/*for ex, with 1Kvertices
4 4
46 48 41 49 27 51 41 45 34 51 52 40 36 39 38 42 
0 46 94 135 184 211 262 303 348 382 433 485 525 561 600 638 
320
12
*/

#include <iostream>
#include "distribute.h"


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	//if domainSize=1, then the domain is square between 0,0 - 1,1	
	double domainSize;//if domainSize=3, then the domain is square between 0,0 - 3,3

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	double interval;
	
	int rowPartitionNum = 0;
	int colPartitionNum = 0;
	unsigned long long chunkSize = 0;
	int initSize = 0;

	if(argc<10){// no arguments
		std::cout<<"The first four arguments are domain size, interval and numbers of partition rows and partition columns\n";
		std::cout<<"The 5th - 7th arguments are source path (../dataSources/10vertices/), point info (mydatabin.ver.xfdl), and point data(mydatabin.ver)\n";
		std::cout<<"The 8th argument is destination path (../dataSources/10vertices/delaunayResults/)\n";
		std::cout<<"The 9th is the chunkSize\n";
		std::cout<<"The 10th is the number of init points in a patition\n";
		std::cout<<"End of the Program!!!";
	}
	else{
		domainSize = atof(argv[1]);
		interval = atof(argv[2]);
		rowPartitionNum = atoi(argv[3]);
		colPartitionNum = atoi(argv[4]);
		if((rowPartitionNum<=0)||(colPartitionNum<=0)) {std::cout<<"Number of row and column partitions have to be greater than zero\n";}

		std::string sourcePath = argv[5];
		if(sourcePath=="") {std::cout<<"The source path does not exist\n"; exit(1);}

		std::string vertexInfoFile = argv[6];
		if(vertexInfoFile=="") {std::cout<<"The vertexInfoFile does not exist\n"; exit(1);}

		std::string vertexDataFile = argv[7];
		if(vertexInfoFile=="") {std::cout<<"The vertexDataFile does not exist\n"; exit(1);}

		std::string destPath = argv[8];
		if(destPath=="") {std::cout<<"The destination path does not exist\n"; exit(1);}

		chunkSize = atoll(argv[9]);
		if(chunkSize<=0) {std::cout<<"chunk size must greater than zero\n"; exit(1);}

		initSize = atoi(argv[10]);
		if(initSize<=0) {std::cout<<"init size must greater than zero\n"; exit(1);}
	
		distribute *dtb = new distribute(domainSize, interval, rowPartitionNum, colPartitionNum, sourcePath, vertexInfoFile, vertexDataFile, destPath, chunkSize, initSize);
		dtb->pointsDistribute();
		dtb->sortUpdate();
//		for(int i=0; i<rowPartitionNum*colPartitionNum; i++)
//			dtb->printPointPartitions1(i);
		dtb->addGridPointsToInitPoints();
//		dtb->printAllPartitions();
		dtb->createFullPointCoorData();
		delete dtb;
		}
//std::cout<<"unsigned long int: "<<sizeof(unsigned long int)<<", unsigned long long: "<<sizeof(unsigned long long)<<std::endl;
	std::cout<<"done!\n";
	return 0;
}

