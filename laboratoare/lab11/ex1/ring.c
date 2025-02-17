#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int numtasks, rank, recv_num;
    MPI_Request request;
    MPI_Status status;
    int *flag;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // Process 0 starts the ring with a random number.
        srand(time(NULL));
        recv_num = rand() % 100;
        printf("Process %d starts with number %d\n", rank, recv_num);

        // Send the number to the next process.
        MPI_Isend(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &request);
        MPI_Test(&request, flag, &status);
        if (*flag == 0) {
            MPI_Wait(&request, &status);
        }

        // Receive the number back from the last process.
        MPI_Irecv(&recv_num, 1, MPI_INT, numtasks - 1, 0, MPI_COMM_WORLD, &request);
        MPI_Test(&request, flag, &status);
        if (*flag == 0) {
            MPI_Wait(&request, &status);
        }
        printf("Process %d received number %d from process %d\n", rank, recv_num, numtasks - 1);

    } else if (rank == numtasks - 1) {
        // Last process closes the circle.
        // Receives the number from the previous process.
        MPI_Irecv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &request);
        MPI_Test(&request, flag, &status);
        if (*flag == 0) {
            MPI_Wait(&request, &status);
        }
        printf("Process %d received number %d from process %d\n", rank, recv_num, rank - 1);

        // Increments the number.
        recv_num++;

        // Sends the number to the first process.
        MPI_Isend(&recv_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        MPI_Test(&request, flag, &status);
        if (*flag == 0) {
            MPI_Wait(&request, &status);
        }
        printf("Process %d sent number %d to process %d\n", rank, recv_num, 0);

    } else {
        // Middle process.
        // Receives the number from the previous process.
        MPI_Irecv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &request);
        MPI_Test(&request, flag, &status);
        if (*flag == 0) {
            MPI_Wait(&request, &status);
        }
        printf("Process %d received number %d from process %d\n", rank, recv_num, rank - 1);

        // Increments the number.
        recv_num++;

        // Sends the number to the next process.
        MPI_Isend(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &request);
        MPI_Test(&request, flag, &status);
        if (*flag == 0) {
            MPI_Wait(&request, &status);
        }
        printf("Process %d sent number %d to process %d\n", rank, recv_num, rank + 1);
    }

    MPI_Finalize();
    return 0;
}
