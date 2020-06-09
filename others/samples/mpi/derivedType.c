#include <stdio.h>
#include <mpi.h>

struct point{
	float x;
	float y;
	int id;
};
     
main(int argc, char **argv){
	int ierr, num_procs, my_id, N, arr[10], arrItem, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	ierr = MPI_Init(&argc, &argv);
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Get_processor_name(processor_name, &namelen);

	


	//MPI_Barrier(MPI_COMM_WORLD);
      printf("Hello world! I'm process %i out of %i processes, N = %d arrItem = %d, processor name = %s\n", 
         my_id, num_procs, N, arrItem, processor_name);

	ierr = MPI_Finalize();
}
