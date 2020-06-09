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
  if (gettimeofday(&TimeInterval, NULL))
  {
    //  Handle error
    throw "Could not get wall clock time.";
  }

  return double(TimeInterval.tv_sec) + double(TimeInterval.tv_usec) * 1.0e-6;
}

//==============================================================================
int coorX_comparison(const void *p1, const void *p2){
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//====================================================================================================
//distribute processes adaptively based on number of points in current coarse active partitions
//based on active partition numbers, number of point in each active partition, number of processes, allocate processes to each group reflectively
//input: 
//	activePartPointSizeArr --> {317, 290, 3, 207, 674, 105, 280, 716, 229, 26, 787, 78}
//	world_size --> 16
//	currActivePartNum --> 12
//output: 
//	rankIdArr (each item is a world_rank such that row_rank=0) --> 0 1 2 3 4 5 6 7 10 11 12 15
//	colorArr (each item belongs to a groupId array) --> 0 1 2 3 4 5 6 7 7 7 8 9 10 10 10 11
//note: currActivePartNum <= world_size (always). Function prepareDataForDelaunayMPI in domain.cpp takes care this condition
//====================================================================================================
void apdaptiveAllocateGroupProcesses(unsigned int *activePartPointSizeArr, unsigned int world_size, unsigned int currActivePartNum, unsigned int *&rankIdArr, unsigned int *&colorArr){
	colorArr = new unsigned int[world_size];
	rankIdArr = new unsigned int[currActivePartNum];
	unsigned int *groupSizeArr;
	bool *allowIncreaseArr;
    double *allowDecreaseArr;


    //generate groupSizeArr: number of processes in each group
	if(currActivePartNum==1){
		for(int i=0; i<world_size; i++) colorArr[i] = 0;
		for(int i=0; i<currActivePartNum; i++) rankIdArr[i] = 0;
	}
	else{
		groupSizeArr = new unsigned int[currActivePartNum];
		allowIncreaseArr = new bool[currActivePartNum];
		allowDecreaseArr = new double[currActivePartNum];

		int totalDistribution = 0;

		unsigned int totalActivePoints = 0;
		for(int i=0; i<currActivePartNum; i++) totalActivePoints += activePartPointSizeArr[i];

		for(int i=0; i<currActivePartNum; i++){
		    double val = (double(activePartPointSizeArr[i])/totalActivePoints)*world_size;
		    if(round(val)>int(val)) allowIncreaseArr[i] = true;
		    else allowIncreaseArr[i]=false;
		    if(int(val)>1) allowDecreaseArr[i] = val - round(val);

		    int processNum = int(val);
		    if(processNum==0) processNum = 1;

			groupSizeArr[i] = processNum;
			totalDistribution += processNum;
		}

//		for(int i=0; i<currActivePartNum; i++) std::cout<<groupSizeArr[i]<<" ";
//		std::cout<<std::endl;

		//if it is still some processes leftover after distribution
	    int leftOver = world_size - totalDistribution;
		if(world_size>totalDistribution){
		    for(int i=0; i<currActivePartNum; i++){
		        if(allowIncreaseArr[i]){
					groupSizeArr[i]++;
		        	leftOver--;
				}
		        if(leftOver==0) break;
		    }
		}

		//if distribution is over the total number of processes
		unsigned int currProcesses = 0;
		unsigned int processOver = 0;
		for(int i=0; i<currActivePartNum; i++) currProcesses += groupSizeArr[i];
		if(currProcesses > world_size) processOver = currProcesses - world_size;
		while(processOver>0){
		    for(int i=0; i<currActivePartNum; i++)
		        if(allowDecreaseArr[i]<0.0){
		            groupSizeArr[i]--;
		            processOver--;
		            if(processOver==0) break;
		        }
	        while(processOver>0.0)
		        for(int i=0; i<currActivePartNum; i++)
		            if(groupSizeArr[i]>1){
		                groupSizeArr[i]--;
		                processOver--;
		                if(processOver==0) break;
		            }
		}

        //in case all partition has the same number of points
        int i=0;
        while((leftOver>0)&&(i<currActivePartNum)){
            groupSizeArr[i]++;
            i++;
            leftOver--;
        }

	for(int i=0; i<currActivePartNum; i++) std::cout<<groupSizeArr[i]<<" ";
	std::cout<<std::endl;

        //generate colorArr based on groupSizeArr
        int processGroup = 0;
        int colorIndex = 0;
        for(int activepartId=0; activepartId<currActivePartNum; activepartId++){
            int processNum = groupSizeArr[activepartId];
            for(int i=0; i<processNum; i++){
                colorArr[colorIndex] = processGroup;
                colorIndex++;
            }
            processGroup++;
        }

        //generate rankIdArr
        rankIdArr[0] = 0;
        for(int activepartId=1; activepartId<currActivePartNum; activepartId++){
            rankIdArr[activepartId] = rankIdArr[activepartId-1] + groupSizeArr[activepartId-1];
        }

        delete [] groupSizeArr;
        delete [] allowIncreaseArr;
        delete [] allowDecreaseArr;
    }
}


//==============================================================================
//distribute processes evenly to coarse active partitions
//based on active partition numbers, number of point in each active partition, number of processes, allocate processes to each group reflectively
//input: 
//	world_size --> 16
//	currActivePartNum --> 12
//output: 
//	rankIdArr (each item is a world_rank such that row_rank=0) --> 0 2 4 6 8 9 10 11 12 13 14 15
//	colorArr (each item belongs to a groupId array) --> 0 0 1 1 2 2 3 3 4 5 6 7 8 9 10 11
//note: currActivePartNum <= world_size (always). Function prepareDataForDelaunayMPI in domain.cpp takes care this condition
//==============================================================================
void equalAllocateGroupProcesses(unsigned int world_size, unsigned int currActivePartNum, unsigned int *&rankIdArr, unsigned int *&colorArr){
	unsigned segmentSize;
	unsigned *groupSizeArr;
	colorArr = new unsigned[world_size];
	rankIdArr = new unsigned[currActivePartNum];

    //generate groupSizeArr: number of processes in each group
	if(currActivePartNum==1){
		for(int i=0; i<world_size; i++) colorArr[i] = 0;
		rankIdArr[0] = 0;
	}
	else{
		groupSizeArr = new unsigned[currActivePartNum];
		if(world_size % currActivePartNum == 0){
			segmentSize = world_size/currActivePartNum;
			for(unsigned i=0; i<currActivePartNum; i++) groupSizeArr[i] = segmentSize;
		}else{//world_size > currActivePartNum
			segmentSize = world_size/currActivePartNum;
			for(unsigned i=0; i<currActivePartNum; i++) groupSizeArr[i] = segmentSize;
			unsigned leftOver = world_size - segmentSize*currActivePartNum;
			unsigned index = 0;
			while(leftOver>0){
				groupSizeArr[index]++;
				index++;
				leftOver--;
			}
		}
			
		//generate colorArr based on groupSizeArr
	    unsigned processGroup = 0, colorIndex = 0;
        for(unsigned activepartId=0; activepartId<currActivePartNum; activepartId++){
            unsigned processNum = groupSizeArr[activepartId];
            for(unsigned i=0; i<processNum; i++){
                colorArr[colorIndex] = processGroup;
                colorIndex++;
            }
            processGroup++;
        }

        //generate rankIdArr
        rankIdArr[0] = 0;
        for(int activepartId=1; activepartId<currActivePartNum; activepartId++)
            rankIdArr[activepartId] = rankIdArr[activepartId-1] + groupSizeArr[activepartId-1];
            
        delete [] groupSizeArr;
	}
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

//==============================================================================
void copyTriangleIdArr(unsigned long long *&triangleIdArr, std::list<unsigned> triangleIdList, triangle *triangleArr){

		unsigned triangleNum = triangleIdList.size();
		allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
		unsigned long long triangleIndex = 0;
		for (std::list<unsigned>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			triangleIdArr[triangleIndex*3] = triangleArr[*it].p1.getId();
			triangleIdArr[triangleIndex*3+1] = triangleArr[*it].p2.getId();
			triangleIdArr[triangleIndex*3+2] = triangleArr[*it].p3.getId();

			triangleIndex++;
		}
}

//==============================================================================
void copyTriangleIdArr(unsigned long long *&triangleIdArr, std::list<unsigned long long> triangleIdList, triangle *triangleArr){

		unsigned triangleNum = triangleIdList.size();
		allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
		unsigned long long triangleIndex = 0;
		for (std::list<unsigned long long>::iterator it=triangleIdList.begin(); it!=triangleIdList.end(); ++it){
			triangleIdArr[triangleIndex*3] = triangleArr[*it].p1.getId();
			triangleIdArr[triangleIndex*3+1] = triangleArr[*it].p2.getId();
			triangleIdArr[triangleIndex*3+2] = triangleArr[*it].p3.getId();

			triangleIndex++;
		}
}

//=========================================================================================
//extract triangleIds from triangleList
void extractTriangleIds(triangleNode *triangleList, unsigned long long *&triangleIdArr, unsigned long long &triangleNum){
	triangleNum = size(triangleList);
	if((triangleNum==0) || (triangleList==NULL)){
		triangleIdArr=NULL;
		return;
	}
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	triangleNode *head = triangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		index++;
		head=head->next;
	}
}

//=========================================================================================
//extract triangleIds from triangleList, each element is triangleNode
void extractTriangleIdsCoors(triangleNode *triangleList, unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long &triangleNum){
	triangleNum = size(triangleList);
	if((triangleNum==0) || (triangleList==NULL)){
		triangleIdArr=NULL;
		triangleCoorArr=NULL;
		return;
	}
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	allocateMemory(triangleCoorArr, double, triangleNum*6);

	triangleNode *head = triangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		triangleCoorArr[index*6] = head->tri->p1.getX();
		triangleCoorArr[index*6+1] = head->tri->p1.getY();
		triangleCoorArr[index*6+2] = head->tri->p2.getX();
		triangleCoorArr[index*6+3] = head->tri->p2.getY();
		triangleCoorArr[index*6+4] = head->tri->p3.getX();
		triangleCoorArr[index*6+5] = head->tri->p3.getY();

		index++;
		head=head->next;
	}
}

//=========================================================================================
//extract triangleIds from triangleList, each element is triangle
void extractTriangleIdsFromTriangleList(std::list<triangle> triangleList, unsigned long long *&triangleIdArr, unsigned long long &triangleNum){
	triangleNum = triangleList.size();
	if((triangleNum==0) || (triangleList.empty()==true)){
		triangleIdArr=NULL;
		return;
	}
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);

	unsigned long long index = 0;
	//collect trinagleIds and triangleCoors out of boundaryTrangleArr
	for (std::list<triangle>::iterator it=triangleList.begin(); it != triangleList.end(); ++it){
		triangleIdArr[index*3] = (*it).p1.getId();
		triangleIdArr[index*3+1] = (*it).p2.getId();
		triangleIdArr[index*3+2] = (*it).p3.getId();

		index++;
	}
}


//=========================================================================================
//extract triangleIds from triangleList, each element is triangle
void extractTriangleIdsCoorsFromTriangleList(std::list<triangle> triangleList, unsigned long long *&triangleIdArr, double *&triangleCoorArr, unsigned long long &triangleNum){
	triangleNum = triangleList.size();
	if((triangleNum==0) || (triangleList.empty()==true)){
		triangleIdArr=NULL;
		triangleCoorArr=NULL;
		return;
	}
	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	allocateMemory(triangleCoorArr, double, triangleNum*6);

	unsigned long long index = 0;
	//collect trinagleIds and triangleCoors out of boundaryTrangleArr
	for (std::list<triangle>::iterator it=triangleList.begin(); it != triangleList.end(); ++it){
		triangleIdArr[index*3] = (*it).p1.getId();
		triangleIdArr[index*3+1] = (*it).p2.getId();
		triangleIdArr[index*3+2] = (*it).p3.getId();

		triangleCoorArr[index*6] = (*it).p1.getX();
		triangleCoorArr[index*6+1] = (*it).p1.getY();
		triangleCoorArr[index*6+2] = (*it).p2.getX();
		triangleCoorArr[index*6+3] = (*it).p2.getY();
		triangleCoorArr[index*6+4] = (*it).p3.getX();
		triangleCoorArr[index*6+5] = (*it).p3.getY();

		index++;
	}
}

//=========================================================================================
//generate triangleList based on triangleIdArr and triangleCoorArr
void generateTriangleList(unsigned long long *triangleIdArr, double *triangleCoorArr, unsigned long long triangleNum, triangleNode *&triangleList){
	//create a list of triangles
	for(unsigned long long index=0; index<triangleNum; index++){
		point p1(triangleCoorArr[index*6], triangleCoorArr[index*6+1], triangleIdArr[index*3]);
		point p2(triangleCoorArr[index*6+2], triangleCoorArr[index*6+3], triangleIdArr[index*3+1]);
		point p3(triangleCoorArr[index*6+4], triangleCoorArr[index*6+5], triangleIdArr[index*3+2]);

		triangle *newTriangle = new triangle(p1, p2, p3);
		newTriangle->computeCenterRadius();
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
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
void generateOffsetArr(unsigned *intArr, unsigned *&intOffsetArr, unsigned intArrSize){
	allocateMemory(intOffsetArr, unsigned, intArrSize);
	intOffsetArr[0] = 0;
	for(int i=1; i<intArrSize; i++)
		intOffsetArr[i] = intOffsetArr[i-1] + intArr[i-1];
}

void generateOffsetArr(unsigned long long *intArr, unsigned long long *&intOffsetArr, unsigned long long intArrSize){
        allocateMemory(intOffsetArr, unsigned long long, intArrSize);
        intOffsetArr[0] = 0;
        for(int i=1; i<intArrSize; i++)
                intOffsetArr[i] = intOffsetArr[i-1] + intArr[i-1];
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
