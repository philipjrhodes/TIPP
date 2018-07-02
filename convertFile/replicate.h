
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <math.h>

//copy all point coordinates in a domain between 0,1 to a bigger domain.

class replicate{
public:
	unsigned long segmentSize;
	unsigned long 	pointNum;
	unsigned long long totalPointNum;
	int verRecordSize;
	std::string pathStr;
	int replicateNum;
	std::string sourcePath;

	replicate(std::string path, int replicateNumber, unsigned long size);
	void printCoorData(double *coorData, unsigned int pointNum);
	void printMainFile();
	void addFile(double shiftX, double shiftY);
	void generateReplication();

	~replicate();
};

