#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

int main (int argc, char *argv[])
{
    int procs, rank;
    int recv;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int value = rank;
    int received_value;

    for (int i = 2; i <= procs; i *= 2) {
        // TODO
        if (rank % i == 0) {
            MPI_Recv(&recv, 1, MPI_INT, rank + (i / 2), MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            value += recv;
        } else if (rank % (i / 2) == 0) {
            MPI_Send(&value, 1, MPI_INT, rank - (i / 2), 0, MPI_COMM_WORLD);
            break;
        }
    }

    if (rank == MASTER) {
        printf("Result = %d\n", value);
    }

    MPI_Finalize();

}

