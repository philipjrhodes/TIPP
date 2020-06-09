#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
 
int main( int argc, char *argv[] )
{
    MPI_Group g1, g2, g4, g5, g45, selfgroup, g6;
    int ranks[16], size, rank, myrank, range[1][3];
    int errs = 0;
    int i, rin[16], rout[16], result;
 
    MPI_Init(0,0);
 
    MPI_Comm_group( MPI_COMM_WORLD, &g1 );
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    if (size < 8) {
        fprintf( stderr, "Test requires 8 processes (16 prefered) only %d provided\n", size ); fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
 
    /* 16 members, this process is rank 0, return in group 1 */
    ranks[0] = myrank; ranks[1] = 2; ranks[2] = 7;
    if (myrank == 2) ranks[1] = 3;
    if (myrank == 7) ranks[2] = 6;
    MPI_Group_incl( g1, 3, ranks, &g2 );
 
    /* Check the resulting group */
    MPI_Group_size( g2, &size );
    MPI_Group_rank( g2, &rank );
 
    if (size != 3) {
        fprintf( stderr, "Size should be %d, is %d\n", 3, size );fflush(stderr);
        errs++;
    }
    if (rank != 0) {
        fprintf( stderr, "Rank should be %d, is %d\n", 0, rank );fflush(stderr);
        errs++;
    }
 
    rin[0] = 0; rin[1] = 1; rin[2] = 2;
    MPI_Group_translate_ranks( g2, 3, rin, g1, rout );
    for (i=0; i<3; i++) {
        if (rout[i] != ranks[i]) {
            fprintf( stderr, "translated rank[%d] %d should be %d\n", i, rout[i], ranks[i] );fflush(stderr);
            errs++;
        }
    }
 
    /* Translate the process of the self group against another group */
    MPI_Comm_group( MPI_COMM_SELF, &selfgroup );
    rin[0] = 0;
    MPI_Group_translate_ranks( selfgroup, 1, rin, g1, rout );
    if (rout[0] != myrank) {
        fprintf( stderr, "translated of self is %d should be %d\n", rout[0], myrank );fflush(stderr);
        errs++;
    }
 
    for (i=0; i<size; i++)
        rin[i] = i;
    MPI_Group_translate_ranks( g1, size, rin, selfgroup, rout );
    for (i=0; i<size; i++) {
        if (i == myrank && rout[i] != 0) {
            fprintf( stderr, "translated world to self of %d is %d\n", i, rout[i] );fflush(stderr);
            errs++;
        }
        else if (i != myrank && rout[i] != MPI_UNDEFINED) {
            fprintf( stderr, "translated world to self of %d should be undefined, is %d\n", i, rout[i] );
            errs++;
        }
    }
    MPI_Group_free( &selfgroup );
 
    /* Exclude everyone in our group */
    {
        int i, *ranks, g1size;
 
        MPI_Group_size( g1, &g1size );
 
        ranks = (int *)malloc( g1size * sizeof(int) );
        for (i=0; i<g1size; i++) ranks[i] = i;
        MPI_Group_excl( g1, g1size, ranks, &g6 );
        if (g6 != MPI_GROUP_EMPTY) {
            fprintf( stderr, "Group formed by excluding all ranks not empty\n" );fflush(stderr);
            errs++;
            MPI_Group_free( &g6 );
        }
        free( ranks );
    }
 
    MPI_Group_free( &g2 );
 
    range[0][0] = 1;
    range[0][1] = size-1;
    range[0][2] = 2;
    MPI_Group_range_excl( g1, 1, range, &g5 );
 
    range[0][0] = 1;
    range[0][1] = size-1;
    range[0][2] = 2;
    MPI_Group_range_incl( g1, 1, range, &g4 );
 
    MPI_Group_union( g4, g5, &g45 );
 
    MPI_Group_compare( MPI_GROUP_EMPTY, g4, &result );
    if (result != MPI_UNEQUAL) {
        errs++;
        fprintf( stderr, "Comparison with empty group gave %d, not 3\n", result );fflush(stderr);
    }
    MPI_Group_free( &g4 );
    MPI_Group_free( &g5 );
    MPI_Group_free( &g45 );
 
    /* Now, duplicate the test, but using negative strides */
    range[0][0] = size-1;
    range[0][1] = 1;
    range[0][2] = -2;
    MPI_Group_range_excl( g1, 1, range, &g5 );
 
    range[0][0] = size-1;
    range[0][1] = 1;
    range[0][2] = -2;
    MPI_Group_range_incl( g1, 1, range, &g4 );
 
    MPI_Group_union( g4, g5, &g45 );
 
    MPI_Group_compare( MPI_GROUP_EMPTY, g4, &result );
    if (result != MPI_UNEQUAL) {
        errs++;
        fprintf( stderr, "Comparison with empty group (formed with negative strides) gave %d, not 3\n", result );fflush(stderr);
    }
    MPI_Group_free( &g4 );
    MPI_Group_free( &g5 );
    MPI_Group_free( &g45 );
    MPI_Group_free( &g1 );
 
    MPI_Finalize();
    return 0;
}
