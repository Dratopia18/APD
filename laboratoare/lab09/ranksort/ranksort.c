#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#define N 1000
#define MASTER 0

void compareVectors(int * a, int * b) {
    int i;
    for(i = 0; i < N; i++) {
        if(a[i]!=b[i]) {
            printf("Sorted incorrectly\n");
            return;
        }
    }
    printf("Sorted correctly\n");
}

void displayVector(int * v) {
    int i;
    int displayWidth = 2 + log10(v[N-1]);
    for(i = 0; i < N; i++) {
        printf("%*i", displayWidth, v[i]);
    }
    printf("\n");
}

int cmp(const void *a, const void *b) {
    int A = *(int*)a;
    int B = *(int*)b;
    return A-B;
}

int main(int argc, char * argv[]) {
    int rank, i, j;
    int nProcesses;
    MPI_Init(&argc, &argv);
    int *pos = (int*)calloc(N, sizeof(int));
    int *v = (int*)malloc(sizeof(int)*N);
    int *vQSort = (int*)malloc(sizeof(int)*N);
    int *result = (int*)malloc(sizeof(int)*N);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    printf("Hello from %i/%i\n", rank, nProcesses);

    if (rank == MASTER) {
        // Generate random vector
        srand(42);
        for(i = 0; i < N; i++) {
            v[i] = rand() % N;
        }

        // Make copy for qsort comparison
        for(i = 0; i < N; i++) {
            vQSort[i] = v[i];
        }
        qsort(vQSort, N, sizeof(int), cmp);
        
        printf("Initial vector:\n");
        displayVector(v);
    }

    // Broadcast the vector to all processes
    MPI_Bcast(v, N, MPI_INT, MASTER, MPI_COMM_WORLD);

    // Calculate chunk size for each process
    int chunk_size = N / nProcesses;
    int start = rank * chunk_size;
    int end = (rank == nProcesses - 1) ? N : start + chunk_size;

    // Calculate positions for assigned chunk
    for(i = start; i < end; i++) {
        for(j = 0; j < N; j++) {
            if(v[i] > v[j] || (v[i] == v[j] && i > j)) {
                pos[i]++;
            }
        }
    }

    // Gather all positions at master process
    int *all_pos = NULL;
    if(rank == MASTER) {
        all_pos = (int*)malloc(N * sizeof(int));
    }
    
    MPI_Gather(pos + start, chunk_size, MPI_INT, 
               all_pos + start, chunk_size, MPI_INT, 
               MASTER, MPI_COMM_WORLD);

    if(rank == MASTER) {
        // Create sorted array using calculated positions
        for(i = 0; i < N; i++) {
            result[all_pos[i]] = v[i];
        }

        printf("Sorted vector:\n");
        displayVector(result);
        compareVectors(result, vQSort);

        // Cleanup
        free(all_pos);
    }

    // Cleanup
    free(v);
    free(vQSort);
    free(pos);
    free(result);
    
    MPI_Finalize();
    return 0;
}
