#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <unistd.h>
#include <fcntl.h>

//#include <mpi.h>

#define mytag 100

//============================================================================
void writeInt(int *intArr, unsigned intNum, unsigned offset, std::string outputPath){
	if((intNum==0)||(intArr==NULL)) return;
	int filedes = open(outputPath.c_str(), O_RDWR | O_CREAT, 0660);
	if(filedes < 0){
		std::cout<<"Failed to open "<<outputPath<<std::endl;
		exit(1);
	}
	
// 	int original_position = lseek(filedes, 0, SEEK_CUR); //not needed
// 	int original_length = lseek(filedes, 0, SEEK_END);
	
	//pwrite doesn't change the file position
	pwrite(filedes, intArr, intNum*sizeof(int), offset*sizeof(int));
	
	
	// if the file is new, seek to the last byte written. 
// 	if ( original_length == 0 ){
// 	    lseek(filedes, intNum*sizeof(int), SEEK_SET);
// 	}
	
	
	close(filedes);
}

//============================================================================
void readInt(int *&intArr, int &intNum, std::string outputPath){
	FILE *f = fopen(outputPath.c_str(), "rb");
	if(!f){
		std::cout<<"Failed to open "<<outputPath<<std::endl;
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
	int intArr[] = {0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0};
	int intNum = 20;
	int offset=0;
	int numInts2write=19;
	
	writeInt(intArr, intNum, offset, "test.dat");
    std::cout<<"After first write, file offset is: "<<offset<<"\n";
	int *newIntArr;
	readInt(newIntArr, intNum, "test.dat");
	printIntArr(newIntArr, intNum);
	delete [] newIntArr;

	int *subIntArr = new int[numInts2write];
	for(int i=0; i<numInts2write; i++) subIntArr[i] = 9;
	writeInt(subIntArr, numInts2write, 2, "test.dat");
	readInt(newIntArr, intNum, "test.dat");
	printIntArr(newIntArr, intNum);

	delete [] newIntArr;
	delete [] subIntArr;
	
	return 0;
}
