#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) 
{
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int i;
    double centroid[3];/*ignore this array*/

    if (world_rank == 0) 
    {
        int destination;
        for (i=0; i<3; i++)
        {
            /*Ignore centroid buffer been sent for now*/

            destination = i+1;/*destination rank or process*/
            MPI_Send(&centroid, 3, MPI_DOUBLE, destination, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("\nEND: This need to print after all MPI_Send/MPI_Recv has been completed\n\n");
    } 
    else
    {   
        MPI_Recv(&centroid, 3, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
        sleep(1); /*This represent many calculations that will happen here later, instead of sleep*/
        printf("Printing at Rank/Process number: %d\n", world_rank);
        MPI_Barrier(MPI_COMM_WORLD);
    }


    MPI_Finalize();
    return 0;
}
