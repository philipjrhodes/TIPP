//g++ -std=gnu++11 replicate.cpp  -o replicate
//./replicate ../dataSources/1Kvertices/rawPointData/ 9 100

//copy all point coordinates in a domain between 0,1 to a bigger domain.
#include "replicate.h"

replicate::replicate(std::string path, int replicateNumber, unsigned long size){
	verRecordSize = 2;
	segmentSize = size;
	replicateNum = replicateNumber;
	sourcePath = path;

	//read number of points in mydatabin.ver.xfdl
	std::string vertexInfofilename = sourcePath + "mydatabin.ver.xfdl";
	std::ifstream vertexInfofile(vertexInfofilename.c_str());
	if(!vertexInfofile){
		std::cout<<"non exist "<<vertexInfofilename.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	vertexInfofile >> strItem;
	vertexInfofile >> strItem;
	pointNum = atoi(strItem.c_str());
	vertexInfofile.close();

	totalPointNum = pointNum*replicateNumber;

}
//=============================================================================
//read coordinate  from original file, update x,y, then add to main file
void replicate::addFile(double shiftX, double shiftY){
	std::string mainFileStr = sourcePath + "largeMydatabin.ver";
	std::string originalFileStr = sourcePath + "mydatabin.ver";

	//open main file
	FILE *f = fopen(mainFileStr.c_str(), "a");//appending 
	if(!f) {std::cout<<"not exist "<<mainFileStr<<std::endl; exit(1);}

	//open original file
	FILE *f1 = fopen(originalFileStr.c_str(), "r");//appending 
	if(!f) {std::cout<<"not exist "<<originalFileStr<<std::endl; exit(1);}

	double *coorData = new double[segmentSize*verRecordSize];

	unsigned long long readingPos = 0;
	unsigned int loadingSize = segmentSize;
	while(readingPos<pointNum-1){
        if((readingPos+loadingSize)>pointNum-1)
			loadingSize = pointNum-readingPos;

		//read loadingSize of Cells
		fread(coorData, sizeof(double), verRecordSize*loadingSize, f1);

		//shift all points in coorData
		for(unsigned int index=0; index<loadingSize; index++){
			coorData[index*verRecordSize] += shiftX;
			coorData[index*verRecordSize+1] += shiftY;
		}

		//append to main file
		fwrite(coorData, sizeof(double), loadingSize*verRecordSize, f);

	readingPos += loadingSize;
	}

	fclose(f);
	fclose(f1);
	delete [] coorData;
}
//=============================================================================
void replicate::generateReplication(){
	int domainSizeX = sqrt(replicateNum);
	int shiftX, shiftY;

	//copy mydatabin.ver into largeMydatabin.ver
	std::string cpCommand = "cp " + sourcePath + "mydatabin.ver " + sourcePath + "largeMydatabin.ver";
	system(cpCommand.c_str());

	//start from 1 because we already had originalfile that is copied into largeMydatabin.ver at first
	for(int index = 1; index<replicateNum; index++){
		shiftX = index % domainSizeX;
		shiftY = (int) index / domainSizeX;
		std::cout<<"================ replicate with square="<<index<<", shiftX="<<shiftX<<", shiftX="<<shiftY<<" ================="<<std::endl;
		addFile(shiftX, shiftY);
	}

	//Create a metadata file largeMydatabin.ver.xfdl for largeMydatabin.ver
	std::ofstream xfdlFile;
	std::string metaDataFile = sourcePath + "largeMydatabin.ver.xfdl";
	xfdlFile.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
	xfdlFile<<verRecordSize<<"\n"<<totalPointNum;
	xfdlFile.close();
	std::cout<<"done!!\n";
}

//=============================================================================
void replicate::printMainFile(){
	std::string mainFileStr = sourcePath + "largeMydatabin.ver";

	//open main file
	FILE *f = fopen(mainFileStr.c_str(), "r");//appending 
	if(!f) {std::cout<<"not exist "<<mainFileStr<<std::endl; exit(1);}

	double *coorData = new double[pointNum*replicateNum*verRecordSize];

/*	for(int squareIndex=0; squareIndex<2; squareIndex++){
		//read loadingSize of Cells
		fread(coorData, sizeof(double), verRecordSize*pointNum, f);
		printCoorData(coorData, pointNum);
		std::cout<<"\n===============================================================================\n";
	}
*/
	fread(coorData, sizeof(double), verRecordSize*replicateNum*pointNum, f);
	printCoorData(coorData, pointNum*replicateNum);
	std::cout<<"\n";
	delete [] coorData;
}

//=============================================================================
void replicate::printCoorData(double *coorData, unsigned int pointNum){
	for(int pointIndex=0; pointIndex<pointNum; pointIndex++){
		std::cout<<"["<<coorData[pointIndex*verRecordSize]<<","<<coorData[pointIndex*verRecordSize+1]<<"]\n";
	}
}
//=============================================================================
replicate::~replicate(){
	
}

//=============================================================================
int main(int argc, char **argv){
	if(argc<4){
		std::cout<<"The first argument is the source path to the dataset for ex(../dataSources/10vertices/)\n";
		std::cout<<"The second argument is the number of replication, should be 4, 9 16, 25, ...\n";
		std::cout<<"The third argument the segment size which is the number og points to read for big dataset\n";
		std::cout<<"for example: if a large file mydatabin.ver has 10000 bytes, and is divided by 10 times, therefore segment size may be 1000";
	}
	else{
		std::string sourcePath = argv[1];
		int replicateNum = atoi(argv[2]);
		unsigned long segmentSize = atoi(argv[3]);

		std::cout<<"sourcePath= "<<sourcePath<<", replicateNum= "<<replicateNum<<", segmentSize="<<segmentSize<<std::endl;
		replicate *r = new replicate(sourcePath, replicateNum, segmentSize);
		r->generateReplication();
		r->printMainFile();
		delete r;
	}
	return 0;
}
