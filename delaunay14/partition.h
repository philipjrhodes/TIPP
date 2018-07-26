#include "linkList.h"
#include <list>

class partition{
public:
	//the partition is ready for the parallelism
	bool active;

	//partition done rearrange ot not
	bool finish;

	//contain all triangle Ids belong to this partition
	std::list<unsigned int> triangleIdList;

	//=====================================================================
	//init function
	partition();
	~partition();
};
