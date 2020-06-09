#include "io.h"
using namespace std;

//==================================================================================
void quit(std::string str){
	std::cout<<str<<std::endl;
	exit(1);
}

//==================================================================================
void minMax(unsigned long long *intArr, unsigned int intArrSize, unsigned long long &min, unsigned long long &max){
	unsigned long long Min = intArr[0];
	unsigned long long Max = Min;
	for(unsigned int i=0; i<intArrSize; i++){
		if(Min>intArr[i]) Min = intArr[i];
		if(Max<intArr[i]) Max = intArr[i];
	}
	min = Min;
	max = Max;
}

//==================================================================================
void updateBitmap(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, bool *bitmap, int bitmapSize, unsigned long long memoryThreshold){
	for(unsigned int i=0; i<bitmapSize; i++) bitmap[i] = false;
	for(unsigned int i=0; i<triangleIdArrSize; i++){
		int sectionId = triangleIdArr[i]/memoryThreshold;
		bitmap[sectionId] = true;
	}
}

//==================================================================================
//read one time from file to pointCoorArr and attributeArr
//firstPointer, lastPointer is the positions in file
void readDirect(std::string dataFileStr, unsigned int firstPointer, unsigned long int loadingSize, int vertexRecordSize, double *dataArr){
	/*reading data for from binary file mydatabin.veratt to page*/
	FILE *fdata=fopen(dataFileStr.c_str(), "rb");
	if(!fdata){
		std::cout<<"not exist "<<dataFileStr<<std::endl;
		exit(1);
	}
	fseek(fdata, firstPointer*vertexRecordSize*sizeof(double), SEEK_SET);
	fread(dataArr, sizeof(double), loadingSize*vertexRecordSize, fdata);
	fclose(fdata);
}

//==================================================================================
//This is a unique LRU load based on triangleIdArr, item is a coordinate and attribute
//triangleIdArr need to be sort and unique first, then load directly one time based on unique list
//Input: triangleIdArr, triangleIdArrSize, triangleIdArr is an array
//Output: triangleCoorArr
//read from files into 2 Arrays: coordinateArr, and attributeArr
void loadDirect(unsigned long long *triangleIdArr, unsigned int triangleIdArrSize, double *&triangleCoorArr, unsigned long long memoryThreshold, std::string dataFileStr){
    unsigned int pageNumber;
    int vertexRecordSize = 2;//a triangle has 3 points, each point has two coordinates.
    unsigned long long firstPointer, lastPointer;
    minMax(triangleIdArr, triangleIdArrSize, firstPointer, lastPointer);
    unsigned long long loadingSize = lastPointer-firstPointer+1; 

//std::cout.precision(16);
//cout<<"firstPointer: "<<firstPointer<<", lastPointer: "<<lastPointer<<", loadingSize: "<<loadingSize<<", triangleIdArrSize: "<<triangleIdArrSize<<" "<<endl;
    
    if(loadingSize<memoryThreshold){
        double *tempArr=new double[loadingSize*vertexRecordSize];
        //read directly to pointCoorArrTemp and attributeArrTemp
        readDirect(dataFileStr, firstPointer, loadingSize, vertexRecordSize, tempArr);
        
        //copy data from triangleCoorArr to pointCoorArr and attributeArr
        for(unsigned int cellPointId=0; cellPointId<triangleIdArrSize; cellPointId++){
            triangleCoorArr[cellPointId*vertexRecordSize] = tempArr[(triangleIdArr[cellPointId]-firstPointer)*vertexRecordSize];
            triangleCoorArr[cellPointId*vertexRecordSize+1] = tempArr[(triangleIdArr[cellPointId]-firstPointer)*vertexRecordSize+1];
//if((triangleCoorArr[cellPointId*vertexRecordSize]>16)||(triangleCoorArr[cellPointId*vertexRecordSize]<0))
//std::cout<<triangleIdArr[cellPointId]<<" "<<triangleCoorArr[cellPointId*vertexRecordSize]<<" "<<triangleCoorArr[cellPointId*vertexRecordSize+1]<<"\n";
        }
        delete [] tempArr;
    }
    else{//not enough memory to load a whole chunk to memory
		//read sequentially each section from coorData file from top to bottom to buffer (triangleCoorArr), 
		//then fill the data in buffer to pointCoorArr and attributeArr until all data has been filled

		//firstPointer, lastPointer constitute a range to read from coorData file 
		unsigned long long firstPointer = 0;
		unsigned long long lastPointer = memoryThreshold-1;
		double *tempArr;

		/*Load to triangleCellList from binary file *bin.ver */
		FILE *fdata=fopen(dataFileStr.c_str(), "rb");
		if(!fdata) quit("not exist " + dataFileStr);

		fseek(fdata, 0L, SEEK_END);//move to the end of file

		//maxFilePointerSize is maximum number of data nodes (each data node includes coordinate and attributes)
		unsigned long long maxFilePointerSize = ftell(fdata)/(vertexRecordSize*sizeof(double));
std::cout<<"Nunber points in fullPointPart.ver: "<<maxFilePointerSize<<"\n";

		fseek(fdata, 0L, SEEK_SET);//move back to the begin of file

		//Use bitmap array to set those sections in vertex file need to read
		//Each bitmap item is correspoding to a section in vertex file, it is true or false, 
		//true means the section in file need to read.
		unsigned int bitmapSize = maxFilePointerSize/memoryThreshold;
		if(bitmapSize < (double)maxFilePointerSize/memoryThreshold)
			bitmapSize= maxFilePointerSize/memoryThreshold + 1;
		bool *bitmap = new bool[bitmapSize];

		//update bitmap based on triangleIdArr
		updateBitmap(triangleIdArr, triangleIdArrSize, bitmap, bitmapSize, memoryThreshold);

		for(unsigned int bitmapId=0; bitmapId<bitmapSize; bitmapId++){
			if(lastPointer>=maxFilePointerSize)
				lastPointer = maxFilePointerSize;

			if(bitmap[bitmapId]){//need to read this section on vertex file
				unsigned long int readingSize = lastPointer - firstPointer + 1;
				tempArr = new double[readingSize*vertexRecordSize];

				fseek(fdata, firstPointer*vertexRecordSize*sizeof(double), SEEK_SET);
				//read a section from file to buffer (triangleCoorArr)
				fread(tempArr, sizeof(double), readingSize*vertexRecordSize, fdata);

				//fill data from buffer to pointCoorArr and attributeArr
				for(unsigned long int index = 0; index<triangleIdArrSize; index++){
					unsigned long int currentPointer = triangleIdArr[index];
					if((currentPointer >= firstPointer)&&(currentPointer <= lastPointer)){//can fill
						triangleCoorArr[index*vertexRecordSize] = tempArr[(currentPointer-firstPointer)*vertexRecordSize];
						triangleCoorArr[index*vertexRecordSize+1] = tempArr[(currentPointer-firstPointer)*vertexRecordSize+1];
//cout<<currentPointer<<" "<<firstPointer<<" "<<lastPointer<<" "<<readingSize<<"   "<<triangleCoorArr[index*vertexRecordSize]<<" "<<triangleCoorArr[(currentPointer-firstPointer)*vertexRecordSize]<<endl;
					}
				}
				delete [] tempArr;
			}
			//update firstPointer, lastPointer
			firstPointer = lastPointer + 1;
			lastPointer += memoryThreshold - 1;
		}
		delete [] bitmap;
	    fclose(fdata);
	}
}

//from triangle Ids (triangleIdFileName) read all the coordinates from coordinate file (triangleCoorFileName) into triangleCoorArr
//at position readingPos (offset) with a size = readingSize. However, readingSize can be changed if readingPos is too close to the end of triangleIds file. Parameter totalTriangleNum is the size of triangleIds file.
//=============================================================================
void readDataFile(std::string triangleIdFileName, std::string triangleCoorFileName, unsigned long long readingPos, unsigned long int &readingSize, unsigned long long totalTriangleNum, unsigned long long *&triangleIdArr, double *&triangleCoorArr){
	int triangleRecordSize = 3;
	//read file
	std::string fileStr = triangleIdFileName;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f) quit("not exist" + triangleIdFileName);

	if(readingSize > (totalTriangleNum - readingPos)) 
		readingSize = totalTriangleNum - readingPos;

	cout<<"reading size in readDataFile: " + to_string(readingSize) + "\n";

	triangleIdArr = new unsigned long long[readingSize*triangleRecordSize];
	triangleCoorArr = new double[readingSize*triangleRecordSize*2];
	
	fseek(f, readingPos*triangleRecordSize*sizeof(unsigned long long), SEEK_SET);
	fread(triangleIdArr, sizeof(unsigned long long), readingSize*triangleRecordSize, f);
	fclose(f);
/*
//	for(int i=0; i<readingSize; i++)
	for(int i=0; i<10; i++)
		std::cout<<triangleIdArr[i*3]<<" "<<triangleIdArr[i*3+1]<<" "<<triangleIdArr[i*3+2]<<"\n";
*/
	unsigned long long memoryThreshold = 1000000000;
	loadDirect(triangleIdArr, readingSize*3, triangleCoorArr, memoryThreshold, triangleCoorFileName);

//	for(int i=0; i<10; i++)
//		std::cout<<triangleCoorArr[i*6]<<","<<triangleCoorArr[i*6+1]<<",  "<<triangleCoorArr[i*6+2]<<","<<triangleCoorArr[i*6+3]<<",  "<<triangleCoorArr[i*6+4]<<","<<triangleCoorArr[i*6+5]<<"\n";

	//check triangle Ids not pass the maximum number of points
	//read triangleCoorFileName, find the max points
	f = fopen(triangleCoorFileName.c_str(), "rb");
	if(!f) quit("not exist " + triangleIdFileName);

	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long totalPointNum = ftell(f)/(2*sizeof(double));
	fclose(f);

	for(unsigned int i=0; i<readingSize*triangleRecordSize; i++)
		if(triangleIdArr[i] >= totalPointNum) quit("The dataset is wrong!!!!");
}

//return number of reads for the big file (triangleIds.tri)
// #read = totalTriangleNum/segmentSize +1
//=============================================================================
unsigned readNum(std::string path, std::string fileName, unsigned long int &segmentSize, unsigned long long &totalTriangleNum){
	std::string triangleIdFileName = path + fileName;
	FILE *f = fopen(triangleIdFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		return 0;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	totalTriangleNum = ftell(f)/(3*sizeof(unsigned long long));
	fclose(f);

	std::cout<<"totalTriangleNum = "<<totalTriangleNum<<std::endl;
	if(totalTriangleNum < segmentSize){
		segmentSize = totalTriangleNum;
		return 1;
	}

	if(totalTriangleNum % segmentSize == 0) 
		return totalTriangleNum/segmentSize;
	else
		return totalTriangleNum/segmentSize + 1;
}


//========================================================================
unsigned long long numberTriangles(std::string path, std::string fileName){
	std::string triangleIdFileName = path + fileName;
	FILE *f = fopen(triangleIdFileName.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<triangleIdFileName<<std::endl;
		return 0;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long totalTriangleNum = ftell(f)/(3*sizeof(unsigned long long));
	fclose(f);

	return totalTriangleNum;
}

//============================================================================
void appendTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string outputPath){
	if((triangleIdArrSize==0)||(triangleIdArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "a");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
}

//============================================================================
void writeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string outputPath){
	if((triangleIdArrSize==0)||(triangleIdArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
	std::cout<<"write to file "<<outputPath<<", with "<<triangleIdArrSize<<" triangles\n";
}

//============================================================================
void writePointCoordinates(double *pointCoorArr, unsigned pointNum, std::string outputPath){
	if((pointNum==0)||(pointCoorArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(pointCoorArr, pointNum*2, sizeof(double), f);
	fclose(f);
	std::cout<<"write to file "<<outputPath<<", with "<<pointNum<<" points\n";
}

//============================================================================
void writePointAttributes(double *pointAttArr, unsigned pointNum, std::string outputPath){
	if((pointNum==0)||(pointAttArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(pointAttArr, pointNum*2, sizeof(double), f);
	fclose(f);
	std::cout<<"write to file "<<outputPath<<", with "<<pointNum<<" attibutess\n";
}

//============================================================================
void appendFile(std::string fileNameStr1, std::string fileNameStr2){
	FILE *f = fopen(fileNameStr1.c_str(), "rb");
	if(!f) quit("does not exist " + fileNameStr1);
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long triangleNum = ftell(f)/(3*sizeof(unsigned long long));
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	unsigned long long *triangleIdArr = new unsigned long long[triangleNum*3];
	fread(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);

	f = fopen(fileNameStr2.c_str(), "ab");
	if(!f) quit("does not exist " + fileNameStr2);
	fwrite(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
	delete [] triangleIdArr;
}

//============================================================================
void appendFile1(std::string fileNameStr1, std::string fileNameStr2){
	FILE *f = fopen(fileNameStr1.c_str(), "rb");
	if(!f) return;
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned long long triangleNum = ftell(f)/(3*sizeof(unsigned long long));
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	unsigned long long *triangleIdArr = new unsigned long long[triangleNum*3];
	fread(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);

	f = fopen(fileNameStr2.c_str(), "ab");
	if(!f) quit("does not exist " + fileNameStr2);
	fwrite(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
	delete [] triangleIdArr;
}

//===================================================================
//std::list<std::string> list_dir(std::string path){
//input: a directory
//output: a list og strings (filenames in the directory)
int getdir (std::string dir, std::list<std::string> &files){
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

//============================================================================
//ex: (001_0.tri, 001_2.tri, 001_3.tri 002_0.tri 002_1.tri ,...) --> triangleIds_ZOrder.tri
void combineFiles(std::string dstPath){
	std::list<std::string> files;
	std::list<std::string> processFiles;
	getdir(dstPath, files);
	std::string s;
	unsigned count = 0;
	//count number of 0*.tri files
	for(std::list<std::string>::iterator it=files.begin(); it!=files.end(); it++){
		s = (*it);
		if(s[0]=='0'){
			count++;
			processFiles.push_back(s);
		}
	}
	processFiles.sort();
	std::string dstfile = dstPath + "triangleIds_ZOrder.tri";
	std::cout<<"start to combine all *.tri to file "<<dstfile<<"\n";
	for(std::list<std::string>::iterator it=processFiles.begin(); it!=processFiles.end(); it++){
		appendFile(dstPath + (*it), dstfile);
		//std::cout<<(*it)<<" ";
	}
	std::cout<<"done combining all *.tri to file "<<dstfile<<"\n";
}

//============================================================================
//ex: (001_0.tri, 001_2.tri, 001_3.tri) --> 001.tri
void combineSubFiles(std::string dstPath, std::string fileNameStr){
	std::list<std::string> files;
	std::list<std::string> processFiles;
	getdir(dstPath, files);
	std::string s;
	unsigned count = 0;
	unsigned fileNameStrLen = fileNameStr.length();

	for(std::list<std::string>::iterator it=files.begin(); it!=files.end(); it++){
		s = (*it);
		if((s.substr(0, fileNameStrLen)==fileNameStr) && (s.substr(fileNameStrLen, 1) == "_")){
			count++;
			processFiles.push_back(s);
		}
	}
	processFiles.sort();
	std::string dstfile = dstPath + fileNameStr + ".tri";
	for(std::list<std::string>::iterator it=processFiles.begin(); it!=processFiles.end(); it++){
		appendFile(dstPath + (*it), dstfile);
		//std::cout<<(*it)<<" ";
	}
}

//============================================================================
void deleteFiles(std::string dstPath){
	std::string rmCommand = "rm " + dstPath + "triangleIds_ZOrder.tri";
	system(rmCommand.c_str());
	rmCommand = "rm " + dstPath + "0*.tri";
	system(rmCommand.c_str());
}

//============================================================================
void exeCommand(std::string commandStr){
	system(commandStr.c_str());
}

//============================================================================
void readMeshInfo(std::string path, unsigned long long &totalTriangleNum, unsigned &domainSize, unsigned &threshold){
	std::string fullPart = path + "meshDataInfo.txt";
	std::ifstream readMeshInfoFile(fullPart.c_str());
	if(!readMeshInfoFile){
		std::cout<<"There is no filename : "<<fullPart<<"\n";
		exit(1);
	}
	std::string strItem;
	readMeshInfoFile >> strItem;
	totalTriangleNum = atoll(strItem.c_str());
	readMeshInfoFile >> strItem;
	domainSize = atoi(strItem.c_str());
	readMeshInfoFile >> strItem;
	threshold = atoi(strItem.c_str());
	readMeshInfoFile.close();
}

//============================================================================
void storeTriangleIds(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string fileStr, std::string optionStr){
	FILE *f = fopen(fileStr.c_str(), optionStr.c_str());
	if(!f) quit("not success to open " + fileStr);
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
}

//============================================================================
void storeTriangleCoors(double *triangleCoorArr, unsigned long long triangleCoorArrSize, std::string fileStr, std::string optionStr){
	FILE *f = fopen(fileStr.c_str(), optionStr.c_str());
	if(!f) quit("not success to open " + fileStr);
	fwrite(triangleCoorArr, triangleCoorArrSize*6, sizeof(double), f);
	fclose(f);
}

//==============================================================================
void storePointCoorArr(double *pointCoorArr, unsigned int pointCoorArrSize, std::string fileStr, std::string optionStr){
	if((pointCoorArr==NULL)||(pointCoorArrSize==0)) return;
	FILE *f = fopen(fileStr.c_str(), optionStr.c_str());
	if(!f) quit("not success to open " + fileStr);
	fwrite(pointCoorArr, pointCoorArrSize*2, sizeof(double), f);
	fclose(f);
}

//==============================================================================
void storePointPartArr(point *pointPartArr, unsigned int pointPartArrSize, std::string fileStr, std::string optionStr){
	if((pointPartArr==NULL)||(pointPartArrSize==0)) return;
	FILE *f = fopen(fileStr.c_str(), optionStr.c_str());
	if(!f) quit("not success to open " + fileStr);
	fwrite(pointPartArr, pointPartArrSize, sizeof(point), f);
	fclose(f);
}


//============================================================================
void readTriangleCoors(double *&triangleCoorArr, unsigned long long &triangleNum, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f) quit("not success to open " + fileStr);
	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(6*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	allocateMemory(triangleCoorArr, double, triangleNum*6);
	fread(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);
}

//============================================================================
void readTriangleIds(unsigned long long *&triangleIdArr, unsigned long long &triangleNum, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f) quit("not success to open " + fileStr);
	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(3*sizeof(unsigned long long)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	allocateMemory(triangleIdArr, unsigned long long, triangleNum*3);
	fread(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
}
//============================================================================
void readPoints(point *&pointArr, unsigned pointNum, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f) quit("not exist " + fileStr);
	allocateMemory(pointArr, point, pointNum);
	fread(pointArr, sizeof(point), pointNum, f);
	fclose(f);
}

//============================================================================
void readPoints_NoAllocation(point *pointArr, unsigned pointNum, std::string fileStr){
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f) quit("not exist " + fileStr);
	fread(pointArr, sizeof(point), pointNum, f);
	fclose(f);
}

//============================================================================
void command(std::string commandStr){
	system(commandStr.c_str());
}

//=========================================================================================
void addFile(std::string outputPath, std::string fileName1, std::string fileName2){
	exeCommand("cat " + outputPath + "/" + fileName1 + " >> " + outputPath + "/" + fileName2);
	exeCommand("rm " + outputPath + "/" + fileName1);
}

