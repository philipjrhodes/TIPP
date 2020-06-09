//g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
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
#include "uniformDistribute.h"


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	unsigned long long chunkSize = 0;
	int initSize = 0;

	if(argc<6){// no arguments
		std::cout<<"You need to input enough arguments\n";
		std::cout<<"End of the Program!!!";
	}
	else{
		unsigned int rowPartNum = atoi(argv[1]);
		unsigned int colPartNum = atoi(argv[2]);

		unsigned long long totalPointNum = atoll(argv[3]);

		std::string path = argv[4];
		if(path=="") {std::cout<<"The destination path does not exist\n"; exit(1);}

		unsigned int initPointNum = atoi(argv[5]);

		distribute *dtb = new distribute(rowPartNum, colPartNum, totalPointNum, initPointNum, path);

		dtb->generateAllPartitionPoints();
		dtb->addGridPointsToInitPoints();
		dtb->createFullPointCoorData();
		dtb->testData();
		dtb->info();
//		dtb->printPointPart(1);
		delete dtb;
		}
	std::cout<<"done!\n";
	return 0;
}

