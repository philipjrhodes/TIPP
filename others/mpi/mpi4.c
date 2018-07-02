/* Split original processes into several groups
*/
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

#define TAG 100

void groupProcess(int *arr, int color, int world_rank, int row_rank, int row_size, MPI_Comm row_comm){
	MPI_Status status;
	int val, buf, i;

//	val = row_rank;
	val = arr[world_rank];
	if(row_rank){/* Have every local worker send its value to its local leader */	
		MPI_Send(&val, 1, MPI_INT, 0, 0, row_comm);
	}
	else{/* Every local leader receives values from its workers */
		for (i = 1; i < row_size; i++) {
			MPI_Recv(&buf, 1, MPI_INT, i, 0, row_comm, &status);
			val += buf;
		}
		printf("%d: leader sum = %d\n", world_rank, val);
	}
}

int main(int argc, char** argv){
	int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	int world_rank, world_size, i, val, buf, size;
	size = 0;
	MPI_Status status;
	int loop = 2;

    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

while(loop>0){
/*	if(world_rank==0){
		int arr1[] = {15, 20, 25, 30};
		MPI_Send(&arr1[1], 1, MPI_INT, 4, TAG, MPI_COMM_WORLD);
		MPI_Send(&arr1[2], 1, MPI_INT, 8, TAG, MPI_COMM_WORLD);
		MPI_Send(&arr1[3], 1, MPI_INT, 12, TAG, MPI_COMM_WORLD);
	}
	else if(world_rank % 4 == 0){
		MPI_Recv(&size, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);
	}
*/
	int color = world_rank / (4*loop); // Determine color based on row
	//int color = world_rank % 4; // Determine color based on row

	// Split the communicator based on the color and use the
	// original rank for ordering
	MPI_Comm row_comm;
	MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);

	int row_rank, row_size;
	MPI_Comm_rank(row_comm, &row_rank);
	MPI_Comm_size(row_comm, &row_size);

	printf("WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE/COLOR: %d/%d/%d size = %d\n", world_rank, world_size, row_rank, row_size, color, size);
//	groupProcess(arr, color, world_rank, row_rank, row_size, row_comm);

	MPI_Comm_free(&row_comm);
	MPI_Barrier(MPI_COMM_WORLD);
	loop--;
}
    MPI_Finalize();
    return 0;
}
