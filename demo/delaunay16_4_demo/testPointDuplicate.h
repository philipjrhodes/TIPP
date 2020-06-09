#include "common.h"
#include <cmath>
#include <iostream>
#include <fstream>

class testPointDuplicate{
public:
	//number of partitions on regarding x and y axes
	int xPartNum;
	int yPartNum;
	int vertexRecordSize;
	std::string path;
	int partNum;
	unsigned int pointNumbers;

	double *coorPointArr;

	//pointPartInfoArr contains number of points for each partition
	unsigned int *pointPartInfoArr;

	testPointDuplicate(std::string p);
	~testPointDuplicate();
	void readPointPartFileInfo();
	void loadPointArr(unsigned int partId);
	void checkPointDuplicate(unsigned int partId);
	void printPointArray(unsigned int partId);

};
