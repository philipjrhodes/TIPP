/* parallel MPI read with arbitrary number of processes*/
#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int myrank, numprocs, bufsize, *buf, count;
	MPI_File thefile;
	MPI_Status status;
	MPI_Offset filesize, myOffset;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_RDONLY,
	MPI_INFO_NULL, &thefile);

	MPI_File_get_size(thefile, &filesize); /* in bytes */
	filesize = filesize / sizeof(int);

	/* in number of ints */
	bufsize = filesize / numprocs;
	/* local number to read */
	buf = (int *) malloc (bufsize * sizeof(int));
//	buf = (int *) malloc (bufsize);

myOffset = (MPI_Offset) myrank * bufsize * sizeof(int);
//myOffset = (MPI_Offset) myrank * bufsize;


MPI_Barrier(MPI_COMM_WORLD);

	MPI_File_seek(thefile, myOffset, MPI_SEEK_SET);
	MPI_File_read(thefile, buf, bufsize, MPI_INT, &status);

	MPI_Get_count(&status, MPI_INT, &count);

	printf("process %d read %d ints\n", myrank, count);
	/*print the content*/
	if(myrank==1){
		int i;
		for(i=0; i<bufsize; i++) printf("%d ", buf[i]);
	}

	MPI_File_close(&thefile);
	MPI_Finalize();


	return 0;
}
