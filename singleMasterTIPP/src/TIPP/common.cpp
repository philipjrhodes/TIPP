#include "common.h"

std::string toString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

//==============================================================================
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum, std::string ext){
	std::string partitionMax = toString(partNum);
	int partitionSize = partitionMax.length();
	std::string partEle = toString(partitionId);
   	for(int i=partEle.length(); i<partitionSize; i++)
   	    fileStr = fileStr + '0';
	fileStr = fileStr + toString(partitionId);
	fileStr = fileStr + ext;
	return fileStr;
}

//==============================================================================
//second unit
double GetWallClockTime(void)
{
  struct timeval TimeInterval;
  if (gettimeofday(&TimeInterval, NULL)){
    //  Handle error
    throw "Could not get wall clock time.";
  }
  return double(TimeInterval.tv_sec) + double(TimeInterval.tv_usec) * 1.0e-6;
}



//==============================================================================
void copyTriangleIdsFromTriangleArr(unsigned long long *&triangleIdArr, unsigned long long triangleNum, triangle *triangleArr){
	if(triangleNum==0) return;
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	//copy ids of triangles left over from triangleArr to tempPointIdArr
	for(unsigned long long index=0; index<triangleNum; index++){
		triangleIdArr[index*3] = triangleArr[index].p1.getId();
		triangleIdArr[index*3+1] = triangleArr[index].p2.getId();
		triangleIdArr[index*3+2] = triangleArr[index].p3.getId();
	}
}

//==============================================================================
void copyTriangleCoorsFromTriangleArr(double *&triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr){
	if(triangleNum==0) return;
	allocateMemory(triangleCoorArr, double, triangleNum*6);
	//copy ids of triangles left over from triangleArr to tempPointIdArr
	for(unsigned long long index=0; index<triangleNum; index++){
		triangleCoorArr[index*6] = triangleArr[index].p1.getX();
		triangleCoorArr[index*6+1] = triangleArr[index].p1.getY();
		triangleCoorArr[index*6+2] = triangleArr[index].p2.getX();
		triangleCoorArr[index*6+3] = triangleArr[index].p2.getY();
		triangleCoorArr[index*6+4] = triangleArr[index].p3.getX();
		triangleCoorArr[index*6+5] = triangleArr[index].p3.getY();
	}
}

//==============================================================================
void copyTrianglesFromTriangleArr(unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr){
	if(triangleNum==0) return;
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	allocateMemory(triangleCoorArr, double, triangleNum*6);

	//copy ids of triangles left over from triangleArr to tempPointIdArr
	for(unsigned long long index=0; index<triangleNum; index++){
		triangleIdArr[index*3] = triangleArr[index].p1.getId();
		triangleIdArr[index*3+1] = triangleArr[index].p2.getId();
		triangleIdArr[index*3+2] = triangleArr[index].p3.getId();

		triangleCoorArr[index*6] = triangleArr[index].p1.getX();
		triangleCoorArr[index*6+1] = triangleArr[index].p1.getY();
		triangleCoorArr[index*6+2] = triangleArr[index].p2.getX();
		triangleCoorArr[index*6+3] = triangleArr[index].p2.getY();
		triangleCoorArr[index*6+4] = triangleArr[index].p3.getX();
		triangleCoorArr[index*6+5] = triangleArr[index].p3.getY();
	}
}

//==============================================================================
void copyTriangleIds(unsigned long long *&triangleIdArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr){

		unsigned long long triangleNum = triangleIdList.size();
		allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
		unsigned long long triangleIndex = 0;
		for (std::list<unsigned long long>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			triangleIdArr[triangleIndex*3] = triangleArr[*it].p1.getId();
			triangleIdArr[triangleIndex*3+1] = triangleArr[*it].p2.getId();
			triangleIdArr[triangleIndex*3+2] = triangleArr[*it].p3.getId();

			triangleIndex++;
		}
}

//==============================================================================
void copyTriangles(unsigned long long *&triangleIdArr, double *&triangleCoorArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr){

		unsigned long long triangleNum = triangleIdList.size();
		allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
		allocateMemory(triangleCoorArr, double, triangleNum*6);
		unsigned long long triangleIndex = 0;
		for (std::list<unsigned long long>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			//distribute info of triangle into two arrays tempCoorArr (all point coordinates) and tempPointIdArr (point Ids)
			triangleCoorArr[triangleIndex*6] = triangleArr[*it].p1.getX();
			triangleCoorArr[triangleIndex*6+1] = triangleArr[*it].p1.getY();
			triangleCoorArr[triangleIndex*6+2] = triangleArr[*it].p2.getX();
			triangleCoorArr[triangleIndex*6+3] = triangleArr[*it].p2.getY();
			triangleCoorArr[triangleIndex*6+4] = triangleArr[*it].p3.getX();
			triangleCoorArr[triangleIndex*6+5] = triangleArr[*it].p3.getY();

			triangleIdArr[triangleIndex*3] = triangleArr[*it].p1.getId();
			triangleIdArr[triangleIndex*3+1] = triangleArr[*it].p2.getId();
			triangleIdArr[triangleIndex*3+2] = triangleArr[*it].p3.getId();

			triangleIndex++;
		}
}






//=========================================================================================
//return a boundingGrid which inclides lower left corner and higher right corner of a boundingBox
gridBound boundingGrid(boundingBox bBox, boundingBox domainBound, unsigned xPartNum, unsigned yPartNum){
	float gridElementSizeX = (domainBound.getHighPoint().getX() - domainBound.getLowPoint().getX())/xPartNum;
    float gridElementSizeY = (domainBound.getHighPoint().getY() - domainBound.getLowPoint().getY())/yPartNum;

    gridElement lowGridElement(mapHigh(bBox, gridElementSizeX, gridElementSizeY, domainBound));
    gridElement highGridElement(mapLow(bBox, gridElementSizeX, gridElementSizeY, domainBound));

	gridBound gb(lowGridElement, highGridElement);
//    return gridBound(lowGridElement, highGridElement);
    return gb;
}

//=========================================================================================
/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the higher index. Ex: 2.5/2 = 2 --> column/row 0, 1, 2; 2.0/2 = 1 */
gridElement mapHigh(boundingBox bBox, double gridElementSizeX, double gridElementSizeY, boundingBox domainBound){
	int gridx, gridy;

	if(bBox.getLowPoint().getX()<0) gridx = 0;
	else gridx = (bBox.getLowPoint().getX() - domainBound.getLowPoint().getX())/gridElementSizeX;
	if(bBox.getLowPoint().getY()<0) gridy = 0;
	else gridy = (bBox.getLowPoint().getY() - domainBound.getLowPoint().getY())/gridElementSizeY;

	gridElement ge(gridx, gridy);

//    return gridElement(gridx, gridy);
	return ge;
}

//=========================================================================================
/** Map a point on the partitioning. If p falls on a partition boundary, we choose the
 partition with the lower index. 2.5/2 = 2 --> column/row 0, 1, 2; 4.0/2 = 2, need to be subtract by 1 --> 1
 if point p fall on the grid line (partition line) --> take lower element.*/
gridElement mapLow(boundingBox bBox, double gridElementSizeX, double gridElementSizeY, boundingBox domainBound){
	int gridx, gridy;

	//-1 because start from zero
	int gridMaxX = (domainBound.getHighPoint().getX() - domainBound.getLowPoint().getX())/gridElementSizeX -1;
	int gridMaxY = (domainBound.getHighPoint().getY() - domainBound.getLowPoint().getY())/gridElementSizeY -1;

	if(bBox.getHighPoint().getX() > domainBound.getHighPoint().getX())
		gridx = gridMaxX;
	else{
		double v = (bBox.getHighPoint().getX() - domainBound.getLowPoint().getX())/gridElementSizeX;
		if((v-(int)v)>0) gridx = (int)v;
		else gridx = (int)v - 1;
	}

	if(bBox.getHighPoint().getY() > domainBound.getHighPoint().getY())
		gridy = gridMaxY;
	else{
		double v = (bBox.getHighPoint().getY() - domainBound.getLowPoint().getY())/gridElementSizeY;
		if((v-(int)v)>0) gridy = (int)v;
		else gridy = (int)v - 1;
	}
	gridElement ge(gridx, gridy);

//	return 	gridElement(gridx, gridy);
	return 	ge;
}

//==============================================================================
//given an index, find a bounding box of a partition in a domain
//input: + partIndex --> partition index of a partition in the domain 
//		 + lowPoint, highPoint --> the domain points
//		 + xPartNum, yPartNum --> granularity of partitions in the domain
//output: the bounding box of cuurent partition
boundingBox findPart(unsigned int partIndex, point lowPoint, point highPoint, unsigned int xPartNum, unsigned int yPartNum){
	double lowPointX = lowPoint.getX();
	double lowPointY = lowPoint.getY();
	double highPointX = highPoint.getX();
	double highPointY = highPoint.getY();

	double xPartSize = (highPointX - lowPointX)/xPartNum;
	double yPartSize = (highPointY - lowPointY)/yPartNum;

	unsigned int gridX = partIndex % xPartNum;
	unsigned int gridY = partIndex / yPartNum;

	point returnLowPoint(lowPointX+gridX*xPartSize, lowPointY+gridY*yPartSize);
	point returnHighPoint(returnLowPoint.getX() + xPartSize, returnLowPoint.getY() + yPartSize);
	return boundingBox(returnLowPoint, returnHighPoint);
}




//=========================================================================================
//generate triangleList based on triangleIdArr and triangleCoorArr
void generateTriangleArr(unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, triangle *triangleArr){
	for(int i=0; i<triangleNum; i++){
		point p1(triangleCoorArr[i*6], triangleCoorArr[i*6+1], triangleIdArr[i*3]);
		point p2(triangleCoorArr[i*6+2], triangleCoorArr[i*6+3], triangleIdArr[i*3+1]);
		point p3(triangleCoorArr[i*6+4], triangleCoorArr[i*6+5], triangleIdArr[i*3+2]);

		//assign new info
		triangleArr[i].delivered = false;
		triangleArr[i].p1.set(p1);
		triangleArr[i].p2.set(p2);
		triangleArr[i].p3.set(p3);

		triangleArr[i].computeCenterRadius();
	}
}


//=========================================================================================
//extract triangleIds from triangleList
void generateOffsetArr(unsigned *intArr, unsigned intArrSize, unsigned *&intOffsetArr){
	allocateMemory(intOffsetArr, unsigned, intArrSize);
	intOffsetArr[0] = 0;
	for(int i=1; i<intArrSize; i++)
		intOffsetArr[i] = intOffsetArr[i-1] + intArr[i-1];
}

