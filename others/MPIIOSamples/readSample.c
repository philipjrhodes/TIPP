#include <stdio.h>
#include <stdlib.h>

#define BUFFER 10

int main(int argc, char **argv)
{
	FILE *f = fopen("testfile", "r");
	if(!f) {printf("testfile does not exist!\n"); return 0;}

	fseek(f, 0, SEEK_END); //seek to end of file
	int elementNum = ftell(f)/sizeof(int); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	int arr[elementNum];

	fread(arr, elementNum, sizeof(int), f);
	fclose(f);

	int i;
	for(i=0; i<elementNum; i++) printf("%d ", arr[i]);
	printf("\n");

/*
	int i;
	if(argc==2){
		int chunkNumber =  atoi(argv[1]);
		int size = BUFFER*chunkNumber;
		int arr[size];

		FILE *f = fopen("testfile", "r");
		fread(arr, size, sizeof(int), f);
		fclose(f);

		for(i=0; i<size; i++) printf("%d ", arr[i]);
	}
*/

   return 0;
}
