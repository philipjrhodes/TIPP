#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

//use parallel I/O  --> parallel read each chunk to each process

/*print the content*/
void printBuf(unsigned int *arr, unsigned int size){
	int i;
	for(i=0; i<size; i++) printf("%d ", arr[i]);
	printf("\n");
}


int main(int argc, char *argv[]){

	int myrank, numprocs, bufsize, count;
	MPI_File thefile;
	MPI_Status status;
	unsigned int fileSize;
	char fileName[200];

	unsigned int *localBuf=NULL, *mainBuf=NULL;

	if(argc!=3){
		printf("Please input chunkSize and the fileName\n");
		MPI_Finalize();
		return -1;
	}
	unsigned int chunkSize = atoi(argv[1]);
	strcpy(fileName, argv[2]);
	//printf("%s, %d", fileName, chunkSize);

	MPI_Init(&argc, &argv);

	double currentTime = MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);


	/* buffer for master */
	if(myrank==0){
		mainBuf = (unsigned int *)malloc(chunkSize * numprocs * sizeof(unsigned int));
		if(mainBuf==NULL){
			printf("The memory is overflow!!!!!!\n");
			MPI_Finalize();
			return -1;
		}
		/* read from file to mainBuf */
		FILE *f = fopen(fileName, "r");
		unsigned long int size = chunkSize * numprocs;
		mainBuf = (unsigned int*) malloc(size*sizeof(unsigned int));
		fread(mainBuf, size, sizeof(unsigned int), f);
		fclose(f);
	}

	/* buffer for local */
	localBuf = (unsigned int *)malloc(chunkSize * sizeof(unsigned int));
	if(localBuf==NULL){
		printf("The memory is overflow!!!!!!\n");
		MPI_Finalize();
		return -1;
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Scatter(mainBuf, chunkSize, MPI_UNSIGNED, localBuf, chunkSize, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	/*print the content*/
	//if(myrank==0) printBuf(localBuf, chunkSize);

	if(myrank==0) free(mainBuf);
	free(localBuf);

	if(myrank==0) 
		printf("Master reads file %s and distribute to workers, with chunkSize %d, processNum %d, time: %f.2\n", fileName, chunkSize, numprocs, MPI_Wtime() - currentTime);

	MPI_Finalize();
	return 0;
}
