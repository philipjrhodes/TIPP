#include "testPointDuplicate.h"

//g++ -std=c++11 common.cpp testPointDuplicate.cpp -o testPointDuplicate

testPointDuplicate::testPointDuplicate(std::string p){
	vertexRecordSize = 2;

	path = p;
	coorPointArr = NULL;

	readPointPartFileInfo();
	coorPointArr = NULL;
	pointNumbers = 0;
}

testPointDuplicate::~testPointDuplicate(){
	delete [] pointPartInfoArr;
	if(coorPointArr!=NULL) delete [] coorPointArr;
}

void testPointDuplicate::readPointPartFileInfo(){
	//Read information from pointPartInfo.xfdl
	std::string fileInfoStr = path + "pointPartitions/" + "pointPartInfo.xfdl";
	std::ifstream vertexPartInfoFile(fileInfoStr.c_str());
	if(!vertexPartInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first read --> copy to xPartNum
	vertexPartInfoFile >> strItem;
	unsigned int xPartNum = atoi(strItem.c_str());

	//second read --> copy to yPartNum
	vertexPartInfoFile >> strItem;
	unsigned int yPartNum = atoi(strItem.c_str());

	partNum = xPartNum*yPartNum;
	std::cout<<"xPartNum = "<<xPartNum<<"  yPartNum = "<<yPartNum<<std::endl;

	pointPartInfoArr = new unsigned int[partNum];

	//read all partition size (number of points for each partition)
	for(unsigned int i=0; i<xPartNum*yPartNum; i++){
		vertexPartInfoFile >> strItem;
		pointPartInfoArr[i] = atoi(strItem.c_str());
//		std::cout<<pointPartInfoArr[i]<<" ";
	}
	vertexPartInfoFile.close();
}


void testPointDuplicate::loadPointArr(unsigned int partId){
	std::string dataFileStr = generateFileName(partId, path + "pointPartitions/" + "pointPart", partNum);

	FILE *f = fopen(dataFileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		return;
	}
	pointNumbers = pointPartInfoArr[partId];
	if(coorPointArr!=NULL) delete [] coorPointArr;
	coorPointArr = new double[pointNumbers*vertexRecordSize];
	fread(coorPointArr, sizeof(double), pointNumbers*vertexRecordSize, f);
	fclose(f);
	std::cout<<"========partition "<<partId<<" loaded=========\n";
}

void testPointDuplicate::checkPointDuplicate(unsigned int partId){
	std::cout.precision(16);
	if(coorPointArr!=NULL){
		double epsilon = 0.0000000000000001;
		unsigned int size = pointPartInfoArr[partId];
		for(unsigned int index1=0; index1<size; index1++){
			double coorX1 = coorPointArr[index1*vertexRecordSize];
			double coorY1 = coorPointArr[index1*vertexRecordSize+1];
			for(unsigned int index2=0; index2<size; index2++)
				if(index1!=index2){
					double coorX2 = coorPointArr[index2*vertexRecordSize];
					double coorY2 = coorPointArr[index2*vertexRecordSize+1];
					if((fabs(coorX2-coorX1)<epsilon)&&(fabs(coorY2-coorY1)<epsilon)) 
						std::cout<<coorX1<<" "<<coorY1<<"==="<<coorX2<<" "<<coorY2<<std::endl;
				}
		}
	}
}

void testPointDuplicate::printPointArray(unsigned int partId){
//	std::cout.precision(16);
	for(unsigned int index=0; index<pointPartInfoArr[partId]; index++)
		std::cout<<coorPointArr[index*vertexRecordSize]<<" "<<coorPointArr[index*vertexRecordSize+1]<<std::endl;
}

int main(){
	std::string path = "./dataSources/1000000vertices/";
	testPointDuplicate *t = new testPointDuplicate(path);

//	for(int partId = 0; partId<t->partNum; partId++){
int partId = 127;
		t->loadPointArr(partId);
		t->checkPointDuplicate(partId);
//	}
//	t->printPointArray(partId);
	delete t;
	return 0;
}

