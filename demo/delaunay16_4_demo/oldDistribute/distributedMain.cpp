//g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
//rm ../dataSources/10Kvertices/delaunayResults/*.*
//./distribute 1 2 2 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 2 20


//Input: mydatabin.ver, mydatabin.ver.xfdl in folder rawPointData
//Output: generate files

//  - pointDomainInfo.xfdl --> including partition sizes 
//		+ first line: (coarse-graind partition sizes, ex: 4 x 4), 
//		+ second line: and number of init points for domain (not inluding 4 corners)
//	- initDomainPoints.ver --> init points for large domain (poin1, point2,...)

//  - pointPartInfoXX.xfdl --> partition info for each partition in domain 
//		+ first line: fine-grained partition sizes ex: (4 x 4), and
//		+ second line: 4 double numbers represent lowPoint and highPoint of current partition
//		+ third line: number of init points
//		+ fourth line: number of points of child partitions
//		+ fifth line: path to dataset --> initPartPointXX.ver & pointPartXX_YY.ver 

//	- initPartPointXX.ver --> init points for each partition in the domain (poin1, point2,...)

//  - pointPartXX_YY.ver --> points in child partition in parent partition XX, and child partition YY
//  - 
//The result data include two files: triangleIds.tri (contains all indices of points of fullPointPart.ver) and fullPointPart.ver
//fullPointPart.ver contains all points in the domain and gris points (include 4 corner points)
//g++ -std=gnu++11 common.cpp point.cpp distribute.cpp distributedMain.cpp -o distribute
//rm ../dataSources/10Kvertices/delaunayResults/*.*
//./distribute 1 2 2 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 2 20

#include <iostream>
#include "distribute.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	//if domainSize=1, then the domain is square between 0,0 - 1,1	
	double domainSize;//if domainSize=3, then the domain is square between 0,0 - 3,3

	//interval between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
	double interval;
	
	unsigned int xCoarsePartNum = 0;
	unsigned int yCoarsePartNum = 0;
	unsigned int xFinePartNum = 0;
	unsigned int yFinePartNum = 0;
	unsigned int initCoarseSize = 0;
	unsigned int initFineSize = 0;

	unsigned long long chunkSize = 0;
	int initSize = 0;

	if(argc<12){// no arguments
		std::cout<<"The first four arguments are domain size, interval and numbers of partition rows and partition columns\n";
		std::cout<<"The 5th - 7th arguments are source path (../dataSources/10vertices/), point info (mydatabin.ver.xfdl), and point data(mydatabin.ver)\n";
		std::cout<<"The 8th argument is destination path (../dataSources/10vertices/delaunayResults/)\n";
		std::cout<<"The 9th is the chunkSize\n";
		std::cout<<"The 10th is the number of init points in a patition\n";
		std::cout<<"End of the Program!!!";
	}
	else{
		domainSize = atof(argv[1]);
		xCoarsePartNum = atoi(argv[2]);
		yCoarsePartNum = atoi(argv[3]);
		xFinePartNum = atoi(argv[4]);
		yFinePartNum = atoi(argv[5]);

		if((xCoarsePartNum<=0)||(yCoarsePartNum<=0)) {std::cout<<"Number of row and column partitions have to be greater than zero\n";}
		if((xFinePartNum<=0)||(yFinePartNum<=0)) {std::cout<<"Number of row and column partitions have to be greater than zero\n";}

		std::string sourcePath = argv[6];
		if(sourcePath=="") {std::cout<<"The source path does not exist\n"; exit(1);}

		std::string vertexInfoFile = argv[7];
		if(vertexInfoFile=="") {std::cout<<"The vertexInfoFile does not exist\n"; exit(1);}

		std::string vertexDataFile = argv[8];
		if(vertexInfoFile=="") {std::cout<<"The vertexDataFile does not exist\n"; exit(1);}

		std::string destPath = argv[9];
		if(destPath=="") {std::cout<<"The destination path does not exist\n"; exit(1);}

		chunkSize = atoll(argv[10]);
		if(chunkSize<=0) {std::cout<<"chunk size must greater than zero\n"; exit(1);}

		initCoarseSize = atoi(argv[11]);
		if(initCoarseSize<=0) {std::cout<<"init size must greater than zero\n"; exit(1);}
	
		initFineSize = atoi(argv[12]);
		if(initFineSize<=0) {std::cout<<"init size must greater than zero\n"; exit(1);}

		distribute *dtb = new distribute(domainSize, xCoarsePartNum, yCoarsePartNum, xFinePartNum, yFinePartNum, sourcePath, vertexInfoFile, vertexDataFile, destPath, chunkSize, initCoarseSize, initFineSize);
		dtb->processDistribution();
		dtb->sortUpdate();
		dtb->addGridPointsToInitPoints();
//		dtb->printAllPartitions();
		dtb->createFullPointCoorData();
		delete dtb;
		}
//std::cout<<"unsigned long int: "<<sizeof(unsigned long int)<<", unsigned long long: "<<sizeof(unsigned long long)<<std::endl;
	std::cout<<"done!\n";
	return 0;
}

