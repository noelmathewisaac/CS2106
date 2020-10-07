#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "barrier.h"

#define ensure_successful_malloc(ptr)                       \
  if (ptr == NULL)                                          \
  {                                                         \
    perror("Memory allocation unsuccessful for" #ptr "\n"); \
    exit(1);                                                \
  }

sem_t *common_sem;
int waiting_at_barrier = 0;

typedef struct thread_task
{
  int thread_id;
  barrier_t *barrier;
  int thread_return;
  int sleep_time;
} thread_task_t;

void *run_thread(void *task);

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("usage: %s threads [seed]\n", argv[0]);
    exit(1);
  }

  if (argc > 2)
  {
    srand(atoi(argv[2])); // [atoi] defaults to 0 if cannot be parsed.
  }

  const int total_threads = argc > 1 ? atoi(argv[1]) : 50;
  // initialise common semaphore
  common_sem = malloc(sizeof(sem_t));
  ensure_successful_malloc(common_sem);
  sem_init(common_sem, 0, 1);

  // initialise the barrier
  barrier_t *barrier = malloc(sizeof(barrier_t));
  ensure_successful_malloc(barrier);
  barrier_init(barrier, total_threads);

  // start each task
  pthread_t threads[total_threads];
  thread_task_t *thread_tasks = malloc(sizeof(thread_task_t) * total_threads);
  ensure_successful_malloc(thread_tasks);

  int i;
  for (i = 0; i < total_threads; i++)
  {
    thread_tasks[i].thread_id = i;
    thread_tasks[i].barrier = barrier;
    thread_tasks[i].sleep_time = (rand() % 500) * 1000;
    pthread_create(&threads[i], NULL, run_thread, (void *)&thread_tasks[i]);
  }

  // wait and collect the tasks
  bool error_found = false;
  for (i = 0; i < total_threads; i++)
  {
    pthread_join(threads[i], NULL);
    if (thread_tasks[i].thread_return < total_threads)
    {
      error_found = true;
      printf("[Thread %2d] exited with %d other tasks (expected: %d)\n",
             i,
             thread_tasks[i].thread_return,
             total_threads);
    }
  }

  free(thread_tasks);

  barrier_destroy(barrier);
  free(barrier);

  sem_destroy(common_sem);
  free(common_sem);

  if (error_found)
  {
    printf("Incorrect executions found\n");
    exit(1);
  }
  return 0;
}

/* Each thread would just sleep for a random amount of time, then wait on the
   barrier. */
void *run_thread(void *task)
{
  thread_task_t *thread_task = (thread_task_t *)task;
  const int thread_id = thread_task->thread_id;
  barrier_t *barrier = thread_task->barrier;

  usleep(thread_task->sleep_time);

  sem_wait(common_sem);
  waiting_at_barrier++;
  sem_post(common_sem);

  printf("[Thread %2d] waiting on barrier\n", thread_id);
  barrier_wait(barrier);
  printf("[Thread %2d] exiting barrier\n", thread_id);
  thread_task->thread_return = waiting_at_barrier;

  return 0;
}
