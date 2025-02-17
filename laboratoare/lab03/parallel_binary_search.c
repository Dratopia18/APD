#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define INSIDE -1 // daca numarul cautat este in interiorul intervalului
#define OUTSIDE -2 // daca numarul cautat este in afara intervalului

struct my_arg {
	int id;
	int N;
	int P;
	int number;
	int *left;
	int *right;
	int *keep_running;
	int *v;
	int *found;
	pthread_barrier_t *barrier;
};

/*
void binary_search() {
	while (keep_running) {
		int mid = left + (right - left) / 2;
		if (left > right) {
			printf("Number not found\n");
			break;
		}

		if (v[mid] == number) {
			keep_running = 0;
			printf("Number found at position %d\n", mid);
		} else if (v[mid] > number) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
}
*/

void *f(void *arg)
{
    struct my_arg* data = (struct my_arg*) arg;
    
    while (*data->keep_running) {
        int size = *data->right - *data->left;
        if (size <= 0) {
            // Search space is empty
            data->found[data->id] = OUTSIDE;
            break;
        }
        
        // Calculate the section size for each thread
        int section_size = size / data->P;
        
        // Calculate bounds for this thread
        int thread_left = *data->left + data->id * section_size;
        int thread_right = (data->id == data->P - 1) ? 
                          *data->right : // Last thread takes remainder
                          thread_left + section_size;
        
        // Perform binary search in thread's section
        int left = thread_left;
        int right = thread_right - 1;
        
        while (left <= right && *data->keep_running) {
            int mid = left + (right - left) / 2;
            
            if (data->v[mid] == data->number) {
                // Number found
                data->found[data->id] = mid;
                *data->keep_running = 0;
                break;
            } else if (data->v[mid] > data->number) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }
        
        // Mark result for this section
        if (*data->keep_running) {
            if (thread_left <= data->number && data->number <= thread_right) {
                data->found[data->id] = INSIDE;
            } else {
                data->found[data->id] = OUTSIDE;
            }
        }

        // Wait for all threads to complete their section
        pthread_barrier_wait(data->barrier);

        // Only continue if number wasn't found and it might be in our section
        if (*data->keep_running && data->found[data->id] == INSIDE) {
            *data->left = thread_left;
            *data->right = thread_right;
        } else {
            break;
        }

        // Wait for all threads to update bounds before next iteration
        pthread_barrier_wait(data->barrier);
    }
    
    return NULL;
}

void display_vector(int *v, int size) {
	int i;

	for (i = 0; i < size; i++) {
		printf("%d ", v[i]);
	}

	printf("\n");
}


int main(int argc, char *argv[])
{
    int r, N, P, number, keep_running, left, right;
    int *v;
    int *found;
    void *status;
    pthread_t *threads;
    struct my_arg *arguments;
    pthread_barrier_t barrier;  // Added barrier

    if (argc < 4) {
        printf("Usage:\n\t./ex N P number\n");
        return 1;
    }

    N = atoi(argv[1]);
    P = atoi(argv[2]);
    number = atoi(argv[3]);
    keep_running = 1;
    left = 0;
    right = N;

    // Initialize barrier
    pthread_barrier_init(&barrier, NULL, P);

    v = (int*) malloc(N * sizeof(int));
    threads = (pthread_t*) malloc(P * sizeof(pthread_t));
    arguments = (struct my_arg*) malloc(P * sizeof(struct my_arg));
    found = (int*) malloc(P * sizeof(int));

    for (int i = 0; i < N; i++) {
        v[i] = i * 2;
    }
    display_vector(v, N);

    for (int i = 0; i < P; i++) {
        arguments[i].id = i;
        arguments[i].N = N;
        arguments[i].P = P;
        arguments[i].number = number;
        arguments[i].left = &left;
        arguments[i].right = &right;
        arguments[i].keep_running = &keep_running;
        arguments[i].v = v;
        arguments[i].found = found;
        arguments[i].barrier = &barrier;  // Pass barrier to threads
        
        r = pthread_create(&threads[i], NULL, f, &arguments[i]);
        if (r) {
            printf("Eroare la crearea thread-ului %d\n", i);
            exit(-1);
        }
    }

    for (int i = 0; i < P; i++) {
        r = pthread_join(threads[i], &status);
        if (r) {
            printf("Eroare la asteptarea thread-ului %d\n", i);
            exit(-1);
        }
    }

    // Check results
    int final_position = -1;
    for (int i = 0; i < P; i++) {
        if (found[i] >= 0) {
            final_position = found[i];
            break;
        }
    }

    if (final_position >= 0) {
        printf("Number %d found at position %d\n", number, final_position);
    } else {
        printf("Number %d not found\n", number);
    }

    // Cleanup
    pthread_barrier_destroy(&barrier);
    free(v);
    free(threads);
    free(arguments);
    free(found);

    return 0;
}
