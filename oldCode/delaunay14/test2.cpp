#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "point.h"
#include "common.h"

int main(){
	int size[] = {4,1,6,11};
	int myrank=0;
	std::string fileStr = generateFileName(myrank, "../dataSources/100vertices/delaunayResults/pointPart", 16);
	FILE *f = fopen(fileStr.c_str(), "rb");
	//pointCoorArr is an array of points
	point *pointCoorArr = new point[size[myrank]];
	fread(pointCoorArr, size[myrank], sizeof(point), f);
	fclose(f);
	
	if(myrank==0)
	for(int i=0; i<size[myrank]; i++)
			std::cout<<pointCoorArr[i].getX()<<" "<<pointCoorArr[i].getY()<<" "<<pointCoorArr[i].getId()<<std::endl;
	delete [] pointCoorArr;
    return 0;
}

