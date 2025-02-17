#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROOT 3

int main (int argc, char *argv[])
{
    int  numtasks, rank, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Get_processor_name(hostname, &len);

    int value;

    if (rank == ROOT) {

        // The ROOT process receives an element from any source.
        MPI_Status status;
        MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // Prints the element and the source. HINT: MPI_Status.
        printf("Process [%d] received %d from process [%d].\n", rank, value, status.MPI_SOURCE);

    } else {

        // Generate a random number.
        srand(time(NULL));
        value = rand() % (rank * 50 + 1);

        printf("Process [%d] send %d.\n", rank, value);

        // Sends the value to the ROOT process.
        MPI_Send(&value, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);;
        printf("Process [%d] sent %d to process [%d].\n", rank, value, ROOT);

    }

    MPI_Finalize();

}

