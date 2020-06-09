
#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv){    
	int world_rank, world_size;	
	MPI_Init(&argc, &argv);

	// Get the rank and size in the original communicator
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the group of processes in MPI_COMM_WORLD
	MPI_Group world_group;
	MPI_Comm_group(MPI_COMM_WORLD, &world_group);

	int n = 3;
	const int ranks[3] = {0, 1, 2};

	// Construct a group containing all of the prime ranks in world_group
	MPI_Group prime_group;
	MPI_Group_incl(world_group, 3, ranks, &prime_group);

	// Create a new communicator based on the group
	MPI_Comm prime_comm;
	MPI_Comm_create_group(MPI_COMM_WORLD, prime_group, 0, &prime_comm);

	int prime_rank = -1, prime_size = -1;
	// If this rank isn't in the new communicator, it will be
	// MPI_COMM_NULL. Using MPI_COMM_NULL for MPI_Comm_rank or
	// MPI_Comm_size is erroneous
	if (MPI_COMM_NULL != prime_comm) {
		MPI_Comm_rank(prime_comm, &prime_rank);
		MPI_Comm_size(prime_comm, &prime_size);
	}

	//printf("WORLD RANK/SIZE: %d/%d \t PRIME RANK/SIZE: %d/%d\n", world_rank, world_size, prime_rank, prime_size);
	printf("PRIME RANK/SIZE: %d/%d\n", prime_rank, prime_size);

	MPI_Group_free(&world_group);
	MPI_Group_free(&prime_group);
	MPI_Comm_free(&prime_comm);
}

