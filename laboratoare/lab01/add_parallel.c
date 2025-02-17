#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
    schelet pentru exercitiul 5
*/

int *arr;
int array_size = 1000000;
int num_threads = 4;

void *f(void *arg) {
  long id = *(long *)arg;
  int start = id * (double)array_size / num_threads;
  int end = (id + 1) * (double)array_size / num_threads;
  for (int i = start; i < end; i++) {
    arr[i] += 100;
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Specificati dimensiunea array-ului si numarul de thread-uri\n");
    exit(-1);
  }
  pthread_t threads[num_threads];
  long id;
  int ids[num_threads];
  array_size = atoi(argv[1]);
  num_threads = atoi(argv[2]);

  arr = malloc(array_size * sizeof(int));
  for (int i = 0; i < array_size; i++) {
    arr[i] = i;
  }
  struct timespec start, finish;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < array_size; i++) {
    printf("%d", arr[i]);
    if (i != array_size - 1) {
      printf(" ");
    } else {
      printf("\n");
    }
  }

  // TODO: aceasta operatie va fi paralelizata cu num_threads fire de executie
  for (id = 0; id < num_threads; id++) {
    ids[id] = id;
    pthread_create(&threads[id], NULL, f, &ids[id]);
  }

  for (id = 0; id < num_threads; id++) {
    pthread_join(threads[id], NULL);
  }

  for (int i = 0; i < array_size; i++) {
    printf("%d", arr[i]);
    if (i != array_size - 1) {
      printf(" ");
    } else {
      printf("\n");
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &finish);
  double elapsed = (finish.tv_sec - start.tv_sec);
  elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
  printf("Timpul de executie al secventei este %f secunde\n", elapsed);

  return 0;
}
