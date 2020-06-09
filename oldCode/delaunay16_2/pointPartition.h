#include <string>

class pointPartition{
public:
	//low left and high right corners of the partition
	point lowPoint;
	point highPoint;

	//partition index
	unsigned int partIndex;

	//when all points in partition are already inserted to domain
	bool finalized;

	//startIndex is offset of global vertex Index
	//ex: 0   100K   1000K ...bilion
	unsigned long long int startIndex;

	//pointCoorArr contains vertex coordinate [x1 y1 x2 y2 ... xn yn]
	//The index for each point is the local index
	double *pointCoorArr;
	//number of point in pointCoorArr, therefore pointCoorArrSize = pointNumber*2
	unsigned int pointNumber;

	//load point data from file
	void loadPointData(std::string);

	//release pointData
	void releasePointData();

	pointPartition();
	~pointPartition();
};
