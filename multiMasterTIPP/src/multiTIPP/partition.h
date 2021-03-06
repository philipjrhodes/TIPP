//#include "linkList.h"
#include <list>

#ifndef PARTITION_H
#define PARTITION_H


class partition{
public:
	//the partition is ready for the parallelism
	bool active;

	//partition done rearrange ot not
	bool finish;

	//contain all triangle Ids belong to this partition
	std::list<unsigned long long> triangleIdList;

	//=====================================================================
	//init function
	partition();
	~partition();
};

#endif
