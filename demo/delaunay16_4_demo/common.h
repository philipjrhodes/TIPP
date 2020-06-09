#include <sstream>
#include <string>
#include <time.h>
#include "point.h"
#include <sys/time.h>


#ifndef H_COMMON
#define H_COMMON

std::string toString(int value);
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum, std::string ext);
double GetWallClockTime(void);
int coorX_comparison(const void *p1, const void *p2);

void storeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string currPath, bool append);
void storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string fileStr, bool append);

#endif