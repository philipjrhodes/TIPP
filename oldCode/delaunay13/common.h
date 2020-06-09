#include <sstream>
#include <string>
#include <time.h>
#include <sys/time.h>


std::string toString(int value);
std::string generateFileName(unsigned int partitionId, std::string fileStr, int partNum);
double GetWallClockTime(void);

