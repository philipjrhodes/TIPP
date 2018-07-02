#include <stdio.h>
#include <mpi.h>

int main( int argc, char ** argv )
{
	int nt, it;

	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &nt );
	MPI_Comm_rank( MPI_COMM_WORLD, &it );
	printf(" Hello from: %d of total %d\n",it,nt);
	MPI_Finalize();
}
