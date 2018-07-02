#include "qHullConvert2DFile1.h"

string convertFile::getFileName(string fullName){
	int lastIndex = fullName.find_last_of(".");
	if(lastIndex!=0)
		return fullName.substr(0, lastIndex);
	else
		return fullName;
}

void convertFile::writeToBinaryFile(string fileNameStr, unsigned int *intArr, unsigned int intArrSize){
	FILE *f = fopen(fileNameStr.c_str(), "w");
	fwrite(intArr, sizeof(unsigned int), intArrSize, f);
	fclose(f);
}
void convertFile::writeToBinaryFile(string fileNameStr, float *floatArr, unsigned int floatArrSize){
	FILE *f = fopen(fileNameStr.c_str(), "w");
	fwrite(floatArr, sizeof(float), floatArrSize, f);
	fclose(f);
}
void convertFile::writeToBinaryFile(string fileNameStr, double *doubleArr, unsigned int doubleArrSize){
/*cout<<"================================\n";
for(int i=0; i<doubleArrSize; i++){
	if (i%2==0) cout<<endl;
	cout<<doubleArr[i]<<" ";
}
*/
	FILE *f = fopen(fileNameStr.c_str(), "a");
	fwrite(doubleArr, sizeof(double), doubleArrSize, f);
	fclose(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertFile::readBinaryFile(string binaryFileName, int bufferSize){

/*	ifstream binaryFile (binaryFileName.c_str(), ios::out | ios::binary);
	float *data = new float[bufferSize];
	binaryFile.read(reinterpret_cast<char*>(data), bufferSize*sizeof(float));
	binaryFile.close();*/

	FILE *fp = fopen(binaryFileName.c_str(), "r");
	//cout<<binaryFileName.c_str();
	float *data = new float[bufferSize];
	fread(data, sizeof(float), bufferSize, fp);
	fclose(fp);

//	unsigned int *data = new unsigned int[bufferSize];
//	fread(data, sizeof(unsigned int), bufferSize, fp);
//	fclose(fp);
	for(int i=0; i<bufferSize; i++){
//	cout<<i<" "<<;
	cout<<data[i]<<" ";
		//if((i+1)%4==0) cout<<"\n";
		if((i+1)%2==0) cout<<"\n";
		//if((i+1)%3==0) cout<<"\n";
	}
	delete [] data;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read the content of a file, recordSize is number of element in a row, recordCount is the number of rows
void convertFile::printDoubleFile(string dataFileNameStr, string infoFileNameStr){

	std::string strItem;
	std::ifstream xfdlFile(infoFileNameStr.c_str());
	if(!xfdlFile) {
		std::cout<<infoFileNameStr<<" does not exist!!!\n";
		exit(1);
	}
	xfdlFile >> strItem;
	int recordSize = atoi(strItem.c_str());
	xfdlFile >> strItem;
	int recordCount = atoi(strItem.c_str());
	xfdlFile.close();

	unsigned int size = recordSize * recordCount;
	FILE *f = fopen(dataFileNameStr.c_str(), "r");
	if(!f) {
		std::cout<<dataFileNameStr<<" is not exist!!!\n";
		exit(1);
	}
	cout<<"The content of "<<dataFileNameStr<<", "<<recordSize<<" "<<recordCount<<endl;
	double *dataArr = new double[size];
	fread(dataArr, sizeof(double), size, f);
	fclose(f);

	cout.precision(16);

	for(unsigned int i=0; i<size; i++){
		if(i%recordSize==0) cout<<"\n";
		cout<<dataArr[i]<<" ";
	}
	cout<<endl;
	delete [] dataArr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read the content of a file, recordSize is number of element in a row, recordCount is the number of rows
void convertFile::printIntFile(string fileNameStr, int recordSize, unsigned int recordCount){
	unsigned int size = recordSize * recordCount;
	FILE *f = fopen(fileNameStr.c_str(), "r");
	//if(!f) {fclose(f); return;}
	cout<<"The content of "<<fileNameStr<<", "<<recordSize<<" "<<recordCount<<endl;
	unsigned int* dataArr = new unsigned int[size];
	fread(dataArr, sizeof(unsigned int), size, f);
	for(int i=0; i<size; i++){
		if(i%recordSize==0) cout<<"\n";
		cout<<dataArr[i]<<" ";
	}
	cout<<endl;
	fclose(f);
	delete [] dataArr;
}

/*Compute the record size of datatextFile (number of attributes of a vertex)*/
int convertFile::attributeRecordSize(string dataTextFileName){
	// Open data text file (attributes)
	ifstream instreamData(dataTextFileName.c_str());
	if((dataTextFileName=="")||(!instreamData)) return 0;
	else{
		/* Identify the number of attributes in dataTextFileName */
		string line;
		int AttrNum=0;
		getline(instreamData, line);/*read the first line of data file*/
		if(line.c_str() == "") AttrNum = 0;
		else for(int i=0; i<line.length(); i++)
				if(line[i]==' ') AttrNum++;
		//int blankCount = count(line, ' ');/*count the number of blank space*/
		return AttrNum+1;
	}
	instreamData.close();
}

//convert vertext file (coordinates) from text to binary, also create a make up attribute file
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void convertFile::convertVertexFile(string vertexTextFileName, int attributeRecordSize, int *recordSize, unsigned long long int *recordCount){
	ifstream instreamVertex(vertexTextFileName.c_str());
	std::string strItem, strItem1, strItem2;

	if(!instreamVertex){
		cout<<"There is no filename : "<<vertexTextFileName;
		return;
	}

    // First line of input contains dimensionality
	instreamVertex >> strItem;
	*recordSize = atoi(strItem.c_str());

	// Second line contains number of records
	instreamVertex >> strItem;
	*recordCount = atoi(strItem.c_str());

	//if the file is loo large to read, then we read in peacewise fashion
	unsigned int thresholdSize;
	unsigned long long int fileSize = (*recordSize) * (*recordCount);

	if(fileSize<100000000) thresholdSize = fileSize;
	else thresholdSize = 100000000;

	double *data = new double[thresholdSize];
	string newBinaryFileNameStr = getFileName(vertexTextFileName) + "bin.ver";
//cout.precision(16);

	//first delete all content if exist this file, then append
	FILE *f = fopen(newBinaryFileNameStr.c_str(), "wa");

	double valX, valY;
	double epsilon = 0.0000001;
	unsigned long long int newFileSize=0;
	/*load content from text file to array data*/
	unsigned long long int fileIndex = 0;
	unsigned int bufferIndex = 0;

	while(!instreamVertex.eof()&&(fileIndex<fileSize)){
		instreamVertex >> strItem1;
		instreamVertex >> strItem2;
		if(bufferIndex>=thresholdSize){
			//store data to file then reset the bufferIndex
			fwrite(data, sizeof(double), thresholdSize, f);
			bufferIndex = 0;
		}
		//read coordinates x and y of a point
		valX = stod(strItem1.c_str());
		valY = stod(strItem2.c_str());
		if((1.0-valX>epsilon)&&(1.0-valY>epsilon)&&(valX>epsilon)&&(valY>epsilon)){
			data[bufferIndex] = valX;
			bufferIndex++;
			data[bufferIndex] = valY;
			bufferIndex++;
			newFileSize++;
//std::cout<<valX<<" "<<valY<<"\n";
		}
		else { 
			std::cout<<"Closed pointX = "<<valX<<", closed pointY = "<<valY<<std::endl;
		}
		fileIndex +=2;
	}
	instreamVertex.close();
	fwrite(data, sizeof(double), bufferIndex, f);
	fclose(f);
cout<<fileIndex<<" "<<bufferIndex<<" "<<fileSize<<endl;

	/*free array data*/
	delete [] data;


	//Create a metadata file for vertex: .xfdl
	ofstream xfdlFile;
	string metaDataFile = newBinaryFileNameStr + ".xfdl";
	xfdlFile.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
	xfdlFile<<(*recordSize)<<"\n"<<newFileSize;
	xfdlFile.close();

/*
	//create file "bin.att"
	unsigned int attSize = attributeRecordSize*(*recordCount);
	data = new float[attSize];
	//make up attribute data 
	for(unsigned int i=0; i<attSize; i++){
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		data[i] = r;
	}
	newBinaryFileNameStr = getFileName(vertexTextFileName) + "bin.att";
	writeToBinaryFile(newBinaryFileNameStr.c_str(), data, attSize);
	delete [] data;

	//Create a metadata file for attribute: .xfdl
	ofstream xfdlFile1;
	metaDataFile = newBinaryFileNameStr + ".xfdl";
	xfdlFile1.open (metaDataFile.c_str(), std::ofstream::out | std::ofstream::trunc);
	xfdlFile1<<attributeRecordSize<<"\n"<<(*recordCount);
	xfdlFile1.close();
*/

}
/////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	convertFile cf;
	int recordSize;
	unsigned long long int recordCount;
	string path = "./data/2D/10vertices/";

	if(argc==1){
		cout<<"You need to input the path to store datasource\n";
		cout<<"Convert nothing!!!\n";		
	}
	else if(argc==2){
		cout<<"The path is: "<<argv[1]<<endl;
		path = argv[1];

//	cout<<"-----------------------------------------------------\n";
	//convert coordinate file
	cf.convertVertexFile(path + "/mydata.ver", 2, &recordSize, &recordCount);
//	cf.printDoubleFile(path + "/mydatabin.ver", path + "/mydatabin.ver.xfdl");

//	cout<<"-----------------------------------------------------\n";

	cout<<"\ndone!!!!\n";
	}
 return 0;
}

