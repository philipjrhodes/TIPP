#include "pointPartition.h"


pointPartition::pointPartition(point lPoint, point hPoint, unsigned int partId, unsigned long long int startId){
	finalized = false;
	lowPoint = lPoint;
	highPoint = hPoint;
	partIndex = partId;
	startIndex = startId;
	
	pointCoorArr = NULL;
	pointCoorArrSize = 0;
}

pointPartition::~pointPartition(){
	if(pointCoorArr!=NULL) delete [] pointArr;
}

void pointPartition::loadPointData(std::string pointPartFile, unsigned int fileSize){
	FILE *fp = fopen(pointPartFile.c_str(), "r");
	if(!fp) {std::cout<<"not exist file "<<pointPartFile<<std::endl; exit(1);}
	pointArr = new double[fileSize];
	fread(pointCoorArr, sizeof(double), fileSize, fp);
	fclose(fp);	

	pointNumber = fileSize/2;
}

void pointPartition::releasePointData(){
	delete [] pointCoorArr;
	pointNumber = 0;
}

