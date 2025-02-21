#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    int  numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int recv_num;

    // First process starts the circle.
    if (rank == 0) {
        // First process starts the circle.
        // Generate a random number.
        recv_num = rand() % 100;
        printf("Process %d generated number %d\n", rank, recv_num);
        // Send the number to the next process.
        MPI_Send(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        printf("Process %d sent number %d to process %d\n", rank, recv_num, rank + 1);

    } else if (rank == numtasks - 1) {
        // Last process close the circle.
        // Receives the number from the previous process.
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received number %d from process %d\n", rank, recv_num, rank - 1);
        // Increments the number.
        recv_num++;
        // Sends the number to the first process.
        MPI_Send(&recv_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("Process %d sent number %d to process %d\n", rank, recv_num, 0);

    } else {
        // Middle process.
        // Receives the number from the previous process.
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received number %d from process %d\n", rank, recv_num, rank - 1);
        // Increments the number.
        recv_num++;
        // Sends the number to the next process.
        MPI_Send(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        printf("Process %d sent number %d to process %d\n", rank, recv_num, rank + 1);
    }

    MPI_Finalize();

}

