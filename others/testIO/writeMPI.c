/* example of parallel MPI write into a single file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
	int myrank, numProcs;
	MPI_File thefile;
	int rc;
	int option=1;
	char fileName[200];

	if(argc!=3){
		printf("Please input the chunksize, and path\n");
		MPI_Finalize();
		return -1;
	}
	unsigned int  chunkSize = atoi(argv[1]);
//	unsigned int chunkNum = atoi(argv[2]);
	strcpy(fileName, argv[2]);

	MPI_Init(&argc, &argv);
	double currentTime = MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

	//unsigned long int fileSize = chunkSize*chunkNum;
	//unsigned long int fileSize = chunkSize;
	unsigned long int bufSize = chunkSize;
	unsigned int *arr = NULL;
	arr =  (unsigned int *)malloc(bufSize*sizeof(unsigned int));
	if(arr==NULL){
		printf("The memory is overflow!!!!!!\n");
		MPI_Finalize();
		return -1;
	}
	unsigned long int i;
	for(i=0; i<bufSize; i++) arr[i]= i + bufSize*myrank;


	rc = MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &thefile);
	if(rc!=MPI_SUCCESS){
		printf("Not success open testfile\n");
		return -1;
	}

	MPI_File_set_view(thefile, myrank * bufSize * sizeof(unsigned int), MPI_UNSIGNED, MPI_UNSIGNED, "native", MPI_INFO_NULL);
	rc = MPI_File_write(thefile, arr, bufSize, MPI_UNSIGNED, MPI_STATUS_IGNORE);
	if(rc!=MPI_SUCCESS){
		printf("Can not save testfile\n");
		MPI_Finalize();
		return -1;
	}

	MPI_File_close(&thefile);
	MPI_Barrier(MPI_COMM_WORLD);

	//if(myrank==0) printf("File %s with size = %d has been written successfully in %f.2 seconds\n", fileName, chunkSize*chunkNum*numProcs, MPI_Wtime() - currentTime);
	if(myrank==0) printf("File %s with chunkSize = %d, and numProcs = %d has been written successfully in %f.2 seconds\n", fileName, chunkSize, numProcs, MPI_Wtime() - currentTime);
	MPI_Finalize();
	return 0;
}
