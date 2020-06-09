#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

int main(int argc, char *argv[]){
	unsigned int *arr;
	char fileName[200];
	int i;

	if(argc!=2){
		printf("Please input the fileName\n");
		return -1;
	}
	strcpy(fileName, argv[1]);

	FILE *f = fopen(fileName, "r");
	fseek(f, 0L, SEEK_END);
	int size = ftell(f)/sizeof(unsigned int);
	arr = (unsigned int*) malloc(size*sizeof(unsigned int));
	fseek(f, 0, SEEK_SET); 
	fread(arr, size, sizeof(unsigned int), f);
	fclose(f);

	for(i=0; i<size; i++) printf("%d ", arr[i]);
	free(arr);
	return 0;
}
