#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <mpi.h>

#define mytag 100

//============================================================================
void writeInt(int *intArr, unsigned intNum, unsigned offset, std::string outputPath){
	if((intNum==0)||(intArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	fseek(f, offset*sizeof(int), SEEK_SET); // seek to offset
	fwrite(intArr, intNum, sizeof(int), f);
	fclose(f);
}

//============================================================================
void readInt(int *&intArr, int &intNum, std::string outputPath){
	FILE *f = fopen(outputPath.c_str(), "rb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	intNum = ftell(f)/sizeof(int);
	intArr = new int[intNum];
	fseek(f, 0, SEEK_SET); // seek to offset
	fread(intArr, intNum, sizeof(int), f);
	fclose(f);
}

//============================================================================
void printIntArr(int *intArr, int intNum){
	std::cout<<"number of items: "<<intNum<<"\n";
	for(int i=0; i<intNum; i++) std::cout<<intArr[i]<<" ";
	std::cout<<"\n";
}

//============================================================================
int main(int argc, char **argv){
	int intArr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int intNum = 10;
	int offset=0;
	writeInt(intArr, intNum, offset, "test.dat");

	int *newIntArr;
	readInt(newIntArr, intNum, "test.dat");
	printIntArr(newIntArr, intNum);
	delete [] newIntArr;

	int *subIntArr = new int[3];
	for(int i=0; i<3; i++) subIntArr[i] = i+1;
	writeInt(subIntArr, 3, 5, "test.dat");
	readInt(newIntArr, intNum, "test.dat");
	printIntArr(newIntArr, intNum);

	delete [] newIntArr;
	delete [] subIntArr;
	
	return 0;
}
