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

int coorX_comparison(const void *p1, const void *p2)
{
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}


//============================================================================
void storeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string currPath, bool append){

	FILE *f = fopen(currPath.c_str(), append?"a":"w");

	if(!f){
		std::cout<<"not success to open "<<currPath<<std::endl;
		exit(1);
	}
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
}

//============================================================================
void storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string fileStr, bool append){
	if((triangleCoorArr==NULL)||(triangleCoorArrSize==0)) return;
	FILE *f = fopen(fileStr.c_str(), append?"a":"w");
	if(!f){
		std::cout<<"not success to open "<<fileStr<<std::endl;
		exit(1);
	}
	fwrite(triangleCoorArr, triangleCoorArrSize*6, sizeof(double), f);
	fclose(f);
}
