#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){

	if(argc!=4){
		printf("Please input the size of binary file, and number of binary files, and path\n");
		return -1;
	}
	unsigned int  fileSize = atoi(argv[1]);
	unsigned int fileNum = atoi(argv[2]);
	char path[200];

	unsigned int *arr = NULL;
	arr =  (unsigned int *)malloc(fileSize*sizeof(unsigned int));
	if(arr==NULL){
		printf("The memory is overflow!!!!!!\n");
		return -1;
	}

	int i;
	for(i=0; i<fileSize; i++) arr[i]=i;

	char fileName[15];
	char currPath[100];
	for(i=0; i<fileNum; i++){
		sprintf(fileName, "testFile%d.dat", i);
		strcpy(path, argv[3]);
		strcat(path, fileName);
		printf("%s\n", path);
		FILE *f = fopen(path, "w");
		fwrite(arr, fileSize, sizeof(unsigned int), f);
		fclose(f);
	}
	free(arr);

	//for(i=0; i<Size; i++) printf("%d ", arr[i]);
	return 0;
}
