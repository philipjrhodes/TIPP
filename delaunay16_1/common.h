#include <sstream>
#include <string>
#include <time.h>
#include "point.h"
#include <sys/time.h>


std::string toString(int value);
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum, std::string ext);
double GetWallClockTime(void);
int coorX_comparison(const void *p1, const void *p2);
