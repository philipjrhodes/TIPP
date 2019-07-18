#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

/* Several iterations of a parallel MPI read. The idea is to reduce
 * the amount of contention on the server, at the cost of some
 * (theoretical) parallelism.  
 *
 * Usage: readTest procsPerRound 
 *
 * */

/*print the content*/
void printBuf(unsigned int *arr, unsigned int size){
	int i;
	for(i=0; i<size; i++) printf("%d ", arr[i]);
	printf("\n");
}

int main(int argc, char *argv[])
{
	int myrank, numProcs, bufsize, *buf, count;
	char fileName[200];

	if(argc!=4){
		printf("Please input the chunksize, chunkNum, and file name\n");
		MPI_Finalize();
		return -1;
	}

	unsigned int chunkSize=atoi(argv[1]);
	unsigned int chunkNum=atoi(argv[2]);//chunkNum is multiple of numprocs
	strcpy(fileName, argv[3]);

	MPI_File thefile;
	MPI_Status status;
	MPI_Offset filesize;

	MPI_Init(&argc, &argv);
	double currentTime = MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

	int numRounds = chunkNum;

	MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &thefile);

	MPI_File_get_size(thefile, &filesize); /* in bytes */
	filesize = filesize / sizeof(unsigned int);

	/* in number of ints */
	//bufsize = chunkSize / numProcs;

	/* local number to read */
	buf = (unsigned int *) malloc (chunkSize * chunkNum * sizeof(unsigned int));
	if(buf==NULL){
		printf("The memory is overflow!!!!!!\n");
		return -1;
	}

	int r;
	for(r=0; r<numRounds; r++){
		MPI_File_set_view(thefile, myrank * chunkSize * sizeof(unsigned int) + r * chunkSize * numProcs * sizeof(unsigned int), MPI_UNSIGNED, MPI_UNSIGNED, "native", MPI_INFO_NULL);
		MPI_File_read(thefile, &buf[r*chunkSize], chunkSize, MPI_INT, &status);

		MPI_Barrier(MPI_COMM_WORLD);

		//if(myrank==0) printBuf(&buf[r*chunkSize], chunkSize);
	}


	//MPI_Get_count(&status, MPI_INT, &count);

	//printf("process %d read %d ints\n", myrank, count);


	free(buf);
	MPI_File_close(&thefile);

	if(myrank==0) printf("Parallel I/O with multi-chunks, chunkSize: %d, chunkNum: %d, numProcs: %d, time: %f.2\n", chunkSize, chunkNum, numProcs, MPI_Wtime() - currentTime);
	MPI_Finalize();


	return 0;
}
