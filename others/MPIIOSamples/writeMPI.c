/* example of parallel MPI write into a single file */
#include <stdio.h>
#include <mpi.h>

#define BUFSIZE 10
int main(int argc, char *argv[])
{
	int i, myrank, buf[BUFSIZE];
	MPI_File thefile;
	int rc;

	int option=1;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if(myrank==0) MPI_File_delete("testfile", MPI_INFO_NULL);

	if(myrank<10)
	for (i=0; i<BUFSIZE; i++)
		buf[i] = myrank * BUFSIZE + i;

	if(myrank>=10) option=0;

/*MPI_File_open(MPI_COMM_WORLD, "testfile",MPI_MODE_CREATE | MPI_MODE_WRONLY,MPI_INFO_NULL, &thefile);
MPI_File_set_view(thefile, myrank * BUFSIZE * sizeof(int), MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
MPI_File_write(thefile, buf, BUFSIZE, MPI_INT, MPI_STATUS_IGNORE);
MPI_File_close(&thefile);
*/

MPI_Barrier(MPI_COMM_WORLD);

//	rc = MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &thefile);
	rc = MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &thefile);
	if(rc!=MPI_SUCCESS){
		printf("Not success open testfile\n");
		return -1;
	}

	rc = MPI_File_set_view(thefile, myrank * BUFSIZE * sizeof(int)*option, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
//	rc = MPI_File_seek(thefile, myrank * BUFSIZE * sizeof(int), MPI_SEEK_SET);

MPI_Barrier(MPI_COMM_WORLD);

//	rc = MPI_File_write_all(thefile, buf, BUFSIZE, MPI_INT, MPI_STATUS_IGNORE);
	rc = MPI_File_write(thefile, buf, BUFSIZE, MPI_INT, MPI_STATUS_IGNORE);
	if(rc!=MPI_SUCCESS){
		printf("Can not save testfile\n");
		return -1;
	}

MPI_Barrier(MPI_COMM_WORLD);
	MPI_File_close(&thefile);


/*
char str[10];
sprintf(str, "testfile%d", myrank);
FILE *f = fopen(str, "w");
fwrite(str, sizeof(char), 10, f);
fclose(f);
*/

	MPI_Finalize();
	return 0;
}
