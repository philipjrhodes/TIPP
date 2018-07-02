#include "mpi.h"
#define FILESIZE (1024 * 1024)
int main(int argc, char **argv)
{
int *buf, rank, nprocs, nints, bufsize;
MPI_File fh;
MPI_Status status;
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
bufsize = FILESIZE/nprocs;
buf = (int *) malloc(bufsize);
nints = bufsize/sizeof(int);
MPI_File_open(MPI_COMM_WORLD, "/pfs/datafile",
MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
MPI_File_seek(fh, rank*bufsize, MPI_SEEK_SET);
MPI_File_read(fh, buf, nints, MPI_INT, &status);
MPI_File_close(&fh);
free(buf);
MPI_Finalize();
return 0;
