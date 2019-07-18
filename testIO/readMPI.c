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

// readMPI filename
int main(int argc, char *argv[]){

	int myrank, numprocs, *buf, count;
	unsigned long long bufsize, fileSize, chunkSize; 
	MPI_File thefile;
	MPI_Status status;
	char fileName[200];

	if(argc!=2){
		printf("Please input the fileName\n");
		MPI_Finalize();
		return -1;
	}

	strcpy(fileName, argv[1]);

	FILE *f = fopen(fileName, "r");
	fseek(f, 0L, SEEK_END);
	fileSize = ftell(f)/sizeof(unsigned int);
	fclose(f);

	
	
	//printf("%s, %d", fileName, chunkSize);

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	chunkSize = fileSize/numprocs;
//	printf("chunkSize=%lld\n", chunkSize);

	int err=MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &thefile);
	if(err!=0){
		printf("MPI_File_open() failed.\n");
              	MPI_Finalize();
                return 2;
	}
	//MPI_File_get_size(thefile, &fileSize); /* in bytes */
	//fileSize = fileSize / sizeof(unsigned int);

	/* in number of ints */
	//bufsize = fileSize / numprocs;
	bufsize = chunkSize;

	/* local number to read */
	buf = (unsigned int *)malloc(bufsize * sizeof(unsigned int));
	if(buf==NULL){
		printf("The memory is overflow!!!!!!\n");
		return -1;

	}

	MPI_File_set_view(thefile, myrank * bufsize * sizeof(unsigned int), MPI_UNSIGNED, MPI_UNSIGNED, "native", MPI_INFO_NULL);

//	printf("process %d is about to read %lld ints\n", myrank, bufsize);
	double then = MPI_Wtime();
	MPI_File_read(thefile, buf, bufsize, MPI_UNSIGNED, &status);
	double now = MPI_Wtime();

	MPI_Get_count(&status, MPI_INT, &count);

//	printf("process %d read %d ints\n", myrank, count);

	/*print the content*/
	//if(myrank==4) printBuf(buf, bufsize);

	int sum=0;
	for(int i=0; i<bufsize; i++){
		sum += buf[i]/0x01010101;
	}

	int * result;	
	if(myrank==0){ 
		result = malloc(numprocs*sizeof(int));
	}

//	printf("Before the gather\n");
	MPI_Gather(&sum, 1, MPI_INT, result, 1, MPI_INT, 0, MPI_COMM_WORLD);
//	printf("Past the gather\n");

	free(buf);
	MPI_File_close(&thefile);

	if(myrank==0){ 
		unsigned long long tsum=0;

		for(int i=0; i<numprocs; i++){
			tsum += result[i];
		}
		printf("Parallel I/O reading file %s with chunkSize %lld, processNum %d, time: %f.2\n", fileName, chunkSize, numprocs, now-then);
		printf("total sum is %lld\n", tsum);
	}
	MPI_Finalize();
	return 0;
}
