#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 2

void *f(void *arg) {
  long id = *(long *)arg;
  for (int i = 0; i < 10; i++) {
    printf("Hello World nr. %d pentru threadul %ld\n", i, id);
    sleep(1);
  }
  pthread_exit(NULL);
}

void *g(void *arg) {
  long id = *(long *)arg;
  for (int i = 0; i < 10; i++) {
    printf("Salut lume nr. %d pentru threadul %ld\n", i, id);
    sleep(1);
  }
}

int main(int argc, char *argv[]) {
  long cores = sysconf(_SC_NPROCESSORS_ONLN);
  printf("Numarul de nuclee este: %ld\n", cores);
  pthread_t threads[NUM_THREADS];
  int r, s;
  long id;
  void *status;
  long ids[NUM_THREADS];

  for (id = 0; id < NUM_THREADS; id++) {
    ids[id] = id;
    r = pthread_create(&threads[id], NULL, f, &ids[id]);
    s = pthread_create(&threads[id], NULL, g, &ids[id]);

    if (r) {
      printf("Eroare la crearea thread-ului %ld\n", id);
      exit(-1);
    }

    if (s) {
      printf("Eroare la crearea thread-ului %ld\n", id);
      exit(-1);
    }
  }

  for (id = 0; id < NUM_THREADS; id++) {
    r = pthread_join(threads[id], &status);
    s = pthread_join(threads[id], &status);

    if (r) {
      printf("Eroare la asteptarea thread-ului %ld\n", id);
      exit(-1);
    }

    if (s) {
      printf("Eroare la asteptarea thread-ului %ld\n", id);
      exit(-1);
    }
  }

  return 0;
}
