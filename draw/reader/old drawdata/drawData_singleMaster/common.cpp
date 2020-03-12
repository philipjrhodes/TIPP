#include "common.h"

std::string toString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

//==============================================================================
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum){
	std::string partitionMax = toString(partNum);
	int partitionSize = partitionMax.length();
	std::string partEle = toString(partitionId);
   	for(int i=partEle.length(); i<partitionSize; i++)
   	    fileStr = fileStr + '0';
	fileStr = fileStr + toString(partitionId);
	fileStr = fileStr + ".ver";
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
