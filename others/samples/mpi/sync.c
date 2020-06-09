   #include <stdio.h>
   #include <mpi.h>
     
   main(int argc, char **argv)
   {
      int ierr, num_procs, my_id, N, arr[10], arrItem, namelen;
      char processor_name[MPI_MAX_PROCESSOR_NAME];

      ierr = MPI_Init(&argc, &argv);

      /* find out MY process ID, and how many processes were started. */

      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
      ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
      MPI_Get_processor_name(processor_name, &namelen);


	if(my_id==0){
		int i=0;
		unsigned int s=0;
		for(i=0; i<100000000; i++) {s=s+i;s=s-i+i-i;}
		printf("Hello world! I'm process %i out of %i processes, N = %d arrItem = %d, processor name = %s\n", my_id, num_procs, N, arrItem, processor_name);
		printf("THIS IS MASTER\n");
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(my_id!=0)
      printf("Hello world! I'm process %i out of %i processes, N = %d arrItem = %d, processor name = %s\n", 
         my_id, num_procs, N, arrItem, processor_name);

      ierr = MPI_Finalize();
   }
