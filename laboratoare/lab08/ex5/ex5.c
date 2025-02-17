#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define ROOT 0

int main (int argc, char *argv[])
{
    int  numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    // Checks the number of processes allowed.
    if (numtasks != 2) {
        printf("Wrong number of processes. Only 2 allowed!\n");
        MPI_Finalize();
        return 0;
    }

    // How many numbers will be sent.
    int send_numbers = 10;
    int num[send_numbers];
    int tags[send_numbers];

    if (rank == 0) {
        // Generate the random numbers.
        for (int i = 0; i < send_numbers; i++) {
            num[i] = rand() % 100;
            tags[i] = rand() % 100;
        }

        for (int i = 0; i < send_numbers; i++) {
            MPI_Send(&num[i], 1, MPI_INT, 1, tags[i], MPI_COMM_WORLD);
            printf("Process [%d] sent number %d with tag %d to process %d.\n", rank, num[i], tags[i], 1);
        }
        // Generate the random tags.
        // Sends the numbers with the tags to the second process.
    } else {

        // Receives the information from the first process.
        MPI_Status status;
        int recv_num;
        for (int i = 0; i < send_numbers; i++) {
            MPI_Recv(&recv_num, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("Process [%d] received number %d with tag %d from process %d.\n", rank, recv_num, status.MPI_TAG, 0);
        }
        // Prints the numbers with their corresponding tags.

    }

    MPI_Finalize();

}

