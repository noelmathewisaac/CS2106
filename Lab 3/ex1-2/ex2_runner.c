/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * Do not modify this file.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "fizzbuzz_workers.h"

#define ensure_successful_malloc(ptr)                           \
  if (ptr == NULL) {                                            \
    perror("Memory allocation unsuccessful for" #ptr "\n");     \
    exit(1);                                                    \
  }

typedef struct thread_task {
  int             thread_id;
  int             n_loops;
  int             thread_return;
} thread_task_t;

void* run_thread(void* task);
void print_num(int n);
void print_fizz(void);
void print_buzz(void);
void print_fizzbuzz(void);

int main ( int argc, char *argv[] ) {
  if (argc < 2) {
    printf("usage: %s n_loops [seed]\n", argv[0]);
    exit(1);
  }

  if (argc > 2) {
    srand(atoi(argv[2])); // [atoi] defaults to 0 if cannot be parsed.
  }

  const int total_threads = 4;
  const int n_loops = atoi(argv[1]);

  // initialise the reusable_barrier
  fizzbuzz_init ( n_loops );

  // start each task
  pthread_t threads[total_threads];
  thread_task_t *thread_tasks = malloc(sizeof(thread_task_t) * total_threads);
  ensure_successful_malloc( thread_tasks );

  int i;
  for (i = 0; i < total_threads; i++) {
    thread_tasks[i].thread_id = i;
    thread_tasks[i].n_loops = n_loops;
    pthread_create( &threads[i], NULL, run_thread, (void*) &thread_tasks[i] );
  }

  // wait and collect the tasks
  bool error_found = false;
  for (i = 0; i < total_threads; i++) {
    pthread_join( threads[i], NULL );
  }

  free(thread_tasks);

  fizzbuzz_destroy();

  if (error_found) {
    printf ( "Incorrect executions found\n" );
    exit(1);
  }
  return 0;
}

void* run_thread( void* task ) {
  thread_task_t* thread_task = (thread_task_t*) task;
  const int thread_id = thread_task->thread_id;
  const int n_loops = thread_task->n_loops;

  switch (thread_id) {
    case 0:
      num_thread( n_loops, print_num );
      break;
    case 1:
      fizz_thread( n_loops, print_fizz );
      break;
    case 2:
      buzz_thread( n_loops, print_buzz );
      break;
    case 3:
      fizzbuzz_thread( n_loops, print_fizzbuzz );
      break;
    default:
      printf ("Unexpected thread id: %d\n", thread_id);
      exit(1);
  }

  thread_task->thread_return = 0;
  pthread_exit(0);
}

void print_num( int n ) {
  usleep( (rand() % 500) * 200 );
  printf( "%d\n", n );
}

void print_fizz( void ) {
  usleep( (rand() % 500) * 200 );
  printf( "Fizz\n" );
}

void print_buzz( void ) {
  usleep( (rand() % 500) * 200 );
  printf( "Buzz\n" );
}

void print_fizzbuzz( void ) {
  usleep( (rand() % 500) * 200 );
  printf( "FizzBuzz\n" );
}
