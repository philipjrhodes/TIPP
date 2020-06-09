/*
g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp nonUniformDistribute.cpp nonUniformDistributedMain.cpp -o nonUniformDistribute
./nonUniformDistribute 8 8 1 8 8 ../dataSources/ ../dataSources/10Kvertices/ 25

#The first two arguments are domain sizes, (ex: 8 8). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=8, then then we have 8x8=64 partitons

#The third argument ($3) is scale
#(there ia 31435 points in the original map, if we want the number of points in domain = 500000000 (500 millions), we set scale = 500000000/31435 = 15905)
#The fourth arguments is source path (ex:  ../dataSources/), contained map file. 
#The fifth arguments is map file name
#The sixth arguments is result path, this folder is also contain the result of Delaunay (triangleIds.tri)
#The last arguments is number of initial points in each partitions. (we choose 25)


#scale = #point/#(map points). Map points in nc_inundation_v6c.grd is 31435.
#(there ia 31435 points in the original map, if we want the number of points in domain = 500000000 (500 millions), we set scale = 500000000/31435 = 15905)

*/
#include <iostream>
#include "nonUniformDistribute.h"


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	unsigned rowPartitionNum = 0;
	unsigned colPartitionNum = 0;
	unsigned long long totalPointNum = 0;
	std::string destPath;

	if(argc<6){// no arguments
		std::cout<<"You need to input enough arguments\n";
		std::cout<<"End of the Program!!!";
	}
	else{
		rowPartitionNum = atoi(argv[1]);
		colPartitionNum = atoi(argv[2]);
		if((rowPartitionNum<=0)||(colPartitionNum<=0)) {std::cout<<"Number of row and column partitions have to be greater than zero\n";}

		unsigned long long totalPointNum = atoll(argv[3]);
		if(totalPointNum<=0) {std::cout<<"Number of totalPointNum have to be greater than zero\n";}

		destPath = argv[4];
		if(destPath=="") {std::cout<<"The destination path does not exist\n"; exit(1);}

		unsigned int initialPointNum = atoi(argv[5]);

		distribute *dtb = new distribute(rowPartitionNum, colPartitionNum, totalPointNum, initialPointNum, destPath);
		dtb->readMapData();
		dtb->scaleMapData();
		dtb->pointsDistribute();
		dtb->sortUpdate();
		double stepInterval = 0.5;
//		double stepInterval = 1;
		dtb->addGridPointsToInitPoints(stepInterval);
		dtb->createFullPointCoorData();
//		dtb->printPointPartitions(3);
		dtb->testData();
		dtb->info();
		delete dtb;
		}
	std::cout<<"done!\n";
	return 0;
}

