   #include <stdio.h>
   #include <mpi.h>
     
   main(int argc, char **argv)
   {
      int ierr, num_procs, my_id, N, arr[10], arrItem, namelen;
      char processor_name[100];

      ierr = MPI_Init(&argc, &argv);

      /* find out MY process ID, and how many processes were started. */

      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
      ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
      MPI_Get_processor_name(processor_name, &namelen);

      printf("Hello world! I'm process %i out of %i processes, N = %d arrItem = %d, processor name = %s\n", 
         my_id, num_procs, N, arrItem, processor_name);

      ierr = MPI_Finalize();
   }
