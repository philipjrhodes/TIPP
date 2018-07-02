//mpicc mpi1.c -o mpi1
//mpiexec -n 4 -f machinefile ./mpi1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MASTER_RANK 0

int main(int argc, char** argv) {
    int nprocs, namelen, my_id, an_id, root_process, i;
     char processor_name[MPI_MAX_PROCESSOR_NAME];
	char *arg = NULL;
	int argLen;

	int N;
	int *intArr;

    MPI_Init(&argc, &argv);
//    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Get_processor_name(processor_name, &namelen);
	root_process = 0;


/*
printf("myid = %d\n", my_id);

	if(my_id == root_process){
	argLen = strlen(argv);
	arg = (char*) malloc(argLen);
	strcpy(arg, argv[1]);
	}
*/

	if(my_id==root_process){
		intArr = (int*) malloc(nprocs);
		for(i=0; i<nprocs; i++) intArr[i] = i;
	}

	MPI_Scatter(intArr, 1, MPI_INT, &N, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);


	printf("myid = %d & N = %d\n", my_id, N);


printf("myid = %d & arg = %s\n", my_id, argv[1]);

//	MPI_Bcast(&argLen, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	
//	if(my_id != root_process) arg = (char*) malloc(argLen);

//printf("%d %s\n",argLen, arg);

//	MPI_Bcast(arg, 4, MPI_CHAR, MASTER_RANK, MPI_COMM_WORLD);

	if(my_id == root_process){
		//This is the master process 
	    printf("Hello from master process %d on %s of %d\n", my_id, processor_name, nprocs);
	}
	else{
		//This is the other process
	    printf("Hello from other process %d on %s of %d\n", my_id, processor_name, nprocs);
	}
 
    MPI_Finalize();
    return 0;
}
