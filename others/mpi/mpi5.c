
#include <mpi.h>
#include <stdio.h>


int main(int argc, char **argv)

{

  int rank[2], size[2], namelen, xranks[] = { 0 };
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Group mpi_group_world, group_slaves;
  MPI_Comm comm_slaves;
  int send_val, recv_val, send_val2, recv_val2;


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank[0]);
  MPI_Comm_size(MPI_COMM_WORLD, &size[0]);
  MPI_Get_processor_name(processor_name, &namelen);

  MPI_Comm_group(MPI_COMM_WORLD, &mpi_group_world);
  MPI_Group_excl(mpi_group_world, 1, xranks, &group_slaves);
  MPI_Comm_create(MPI_COMM_WORLD, group_slaves, &comm_slaves);

  printf("Hello world! I’m rank %d of %d on %s\n", rank[0], size[0], processor_name);

  if (rank[0]) {

    MPI_Comm_rank(comm_slaves, &rank[1]);

    MPI_Comm_size(comm_slaves, &size[1]);

    printf("In the slave universe I’m rank %d of %d on %s\n", rank[1], size[1], processor_name);

    send_val = size[1];

    MPI_Reduce(&send_val, &recv_val, 1, MPI_INT, MPI_SUM, 0, comm_slaves);

    if (!rank[1]) printf("Slave leader received reduced value %d\n", recv_val);

  }

  send_val2 = size[0];

  MPI_Reduce(&send_val2, &recv_val2, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (!rank[0]) printf("Master received reduced value %d\n", recv_val2);

  if (comm_slaves != MPI_COMM_NULL) MPI_Comm_free(&comm_slaves);

  MPI_Group_free(&group_slaves);

  MPI_Group_free(&mpi_group_world);

  MPI_Finalize();

}
