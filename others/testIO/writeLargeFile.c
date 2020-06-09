#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){

	if(argc!=3){
		printf("Please input the chunksize, and number of chunks, and path\n");
		return -1;
	}
	unsigned int  chunkSize = atoi(argv[1]);
	unsigned int chunkNum = atoi(argv[2]);
	char path[200];

	unsigned long int fileSize = chunkSize*chunkNum;
	unsigned int *arr = NULL;
	arr =  (unsigned int *)malloc(fileSize*sizeof(unsigned int));
	if(arr==NULL){
		printf("The memory is overflow!!!!!!\n");
		return -1;
	}

	unsigned long int i;
	for(i=0; i<fileSize; i++) arr[i]=i;

	strcpy(path, "largeTestFile.dat");
	printf("%s\n", path);
	FILE *f = fopen(path, "w");
	fwrite(arr, fileSize, sizeof(unsigned int), f);
	fclose(f);

	free(arr);
	printf("Writed to largeTestFile.dat successfully with size: %d\n", chunkSize*chunkNum);
	return 0;
}
