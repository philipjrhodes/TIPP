#include <mpi.h>
#include <stdio.h>

/* Several iterations of a parallel MPI read. The idea is to reduce
 * the amount of contention on the server, at the cost of some
 * (theoretical) parallelism.  
 *
 * Usage: readTest procsPerRound 
 *
 * */
int main(int argc, char *argv[])
{
	int myrank, numprocs, bufsize, *buf, count;
	int procsPerRound=atoi(argv[1]);
	MPI_File thefile;
	MPI_Status status;
	MPI_Offset filesize;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	int numRounds = numProcs/procPerRound;

	MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_RDONLY,
	MPI_INFO_NULL, &thefile);

	MPI_File_get_size(thefile, &filesize); /* in bytes */
	filesize = filesize / sizeof(int);

	/* in number of ints */
	bufsize = filesize / numProcs;

	if(myrank>=10) bufsize=0;
	/* local number to read */
	buf = (int *) malloc (bufsize * sizeof(int));

	MPI_File_set_view(thefile, myrank * bufsize * sizeof(int),
	MPI_INT, MPI_INT, "native", MPI_INFO_NULL);

	for(int r=0; r<numRounds; r++){
		MPI_File_read(thefile, buf, bufsize, MPI_INT, &status);
	}
	//MPI_Get_count(&status, MPI_INT, &count);

	//printf("process %d read %d ints\n", myrank, count);

	/*print the content*/
//	if(myrank==10){
//		int i;
//		for(i=0; i<bufsize; i++) printf("%d ", buf[i]);
//		printf("%d\n", count);
//	}

	MPI_File_close(&thefile);
	MPI_Finalize();


	return 0;
}
