/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * Do not modify this file.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "entry_controller.h"
#include "exit_controller.h"

#define ensure_successful_malloc(ptr)                           \
  if (ptr == NULL) {                                            \
    perror("Memory allocation unsuccessful for" #ptr "\n");     \
    exit(1);                                                    \
  }

int n_trains;
int trains_in_loading_bays = 0;
int loading_bays = 5;
int no_of_priorities = 2;
int exit_queues[50];
int trains_in_exit_queues[50][50];
sem_t *loading_bay_sem;
sem_t *exit_queues_sem;

typedef struct thread_task {
  int train_id;
  int loading_time;
  int exit_time;
  int priority;
  // synchronisation
  entry_controller_t *entry_controller;
  exit_controller_t *exit_controller;
} thread_task_t;

void* simulate_train(void* task);

int main ( int argc, char *argv[] ) {
  if (argc < 4) {
    printf("usage: %s no_of_trains no_of_bays no_of_priorities [seed]\n", argv[0]);
    exit(1);
  }

  n_trains = atoi(argv[1]);
  loading_bays = atoi(argv[2]);
  no_of_priorities = atoi(argv[3]);

  if (argc > 4) {
    srand(atoi(argv[4])); // [atoi] defaults to 0 if cannot be parsed.
  }

  // initialise common semaphores
  loading_bay_sem = malloc(sizeof(sem_t));
  ensure_successful_malloc( loading_bay_sem );
  sem_init( loading_bay_sem, 0, 1 );
  exit_queues_sem = malloc(sizeof(sem_t));
  ensure_successful_malloc( exit_queues_sem );
  sem_init( exit_queues_sem, 0, 1 );

  // initialise entry_controller
  entry_controller_t *common_entry_controller = malloc(sizeof(entry_controller_t));
  ensure_successful_malloc( common_entry_controller );
  entry_controller_init( common_entry_controller, loading_bays );

  // initialise exit_controller
  exit_controller_t *common_exit_controller = malloc(sizeof(exit_controller_t));
  ensure_successful_malloc( common_exit_controller );
  exit_controller_init( common_exit_controller, 2 );

  // start each task
  pthread_t threads[n_trains];
  thread_task_t *thread_tasks = malloc(sizeof(thread_task_t) * n_trains);
  ensure_successful_malloc( thread_tasks );

  int i;
  for (i = 0; i < n_trains; i++) {
    thread_tasks[i].train_id = i;
    thread_tasks[i].loading_time = (rand() % 500) * 500;
    thread_tasks[i].exit_time = (rand() % 400) * 500 + 500;
    thread_tasks[i].priority = rand() % no_of_priorities;
    thread_tasks[i].entry_controller = common_entry_controller;
    thread_tasks[i].exit_controller = common_exit_controller;
    pthread_create( &threads[i], NULL, simulate_train, (void*) &thread_tasks[i] );
  }

  for (i = 0; i < n_trains; i++) {
    pthread_join( threads[i], NULL );
  }

  free( thread_tasks );

  exit_controller_destroy( common_exit_controller );
  free( common_exit_controller );

  entry_controller_destroy( common_entry_controller );
  free( common_entry_controller );

  sem_destroy( exit_queues_sem );
  free( exit_queues_sem );

  sem_destroy( loading_bay_sem );
  free( loading_bay_sem );

  return 0;
}

void enter_loading_bays(thread_task_t *task) {
  printf("[Train %3d] joins the loading bay queue\n", task->train_id);
  entry_controller_wait(task->entry_controller);
  sem_wait(loading_bay_sem);
  trains_in_loading_bays++;
  if (trains_in_loading_bays > loading_bays) {
    printf("Incorrect synchronization: too many trains in the loading bays! Expected: %d, Got: %d\n",
           loading_bays,
           trains_in_loading_bays);
    exit(1);
  }
  printf("[Train %3d] enters the loading bays\n", task->train_id);
  sem_post(loading_bay_sem);
}

void exit_loading_bays(thread_task_t *task) {
  int exit_before_current_train[50]; 
  int trains_to_exit_before_current[50][50];
    
  printf("[Train %3d] leaves the loading bay and starts waiting to exit (Priority: %d)\n",
         task->train_id,
         task->priority);

  // update the exit queues
  sem_wait(exit_queues_sem);
  trains_in_exit_queues[task->priority][exit_queues[task->priority]] = task->train_id;
  exit_queues[task->priority]++;
  for (int i = 0; i < task->priority; i++) {
    exit_before_current_train[i] = exit_queues[i];    
    for (int j = 0; j < exit_before_current_train[i]; j++) {
        trains_to_exit_before_current[i][j] = trains_in_exit_queues[i][j];
    }
    printf("[Train %3d] %d trains with priority %d have to exit before current train\n",
             task->train_id,
             exit_before_current_train[i],
             i);
  }

  sem_post(exit_queues_sem);

  exit_controller_wait(task->exit_controller, task->priority);
  
  sem_wait(exit_queues_sem);
  
  //update trains in the queue to exit
  for (int i = 0; i < exit_queues[task->priority]; i++) {
    if (trains_in_exit_queues[task->priority][i] == task->train_id) {
      trains_in_exit_queues[task->priority][i] = trains_in_exit_queues[task->priority][exit_queues[task->priority] - 1];
      trains_in_exit_queues[task->priority][exit_queues[task->priority] - 1] = -1;
      break;
    }
  }
  exit_queues[task->priority]--;
  printf("[Train %3d] Exiting...\n", task->train_id);

  sem_post(exit_queues_sem);

  // update loading bay states
  sem_wait(loading_bay_sem);
  trains_in_loading_bays--;
  sem_post(loading_bay_sem);

  entry_controller_post(task->entry_controller);
  

  sem_wait(exit_queues_sem);
  for (int i = 0; i < task->priority; i++) {
    for (int j = 0; j < exit_before_current_train[i]; j++) {
      for (int k = 0; k < exit_queues[i]; k++) {   
        if (trains_to_exit_before_current[i][j] == trains_in_exit_queues[i][k]) {
          printf("Possible incorrect synchronization: Train %d with priority %d should have exited before current train %d with priority %d!\n",
                 trains_to_exit_before_current[i][j],
                 i,
                 task->train_id,
                 task->priority
                 );
        }
      }
    }
  }
   
  sem_post(exit_queues_sem);

  // perform exiting
  usleep( task->exit_time);
  exit_controller_post(task->exit_controller, task->priority);
  
  printf("[Train %3d] Exiting finished. Another train can exit. \n", task->train_id);

  
}

void* simulate_train( void* task ) {
  thread_task_t* thread_task = (thread_task_t*) task;

  enter_loading_bays( thread_task );
  usleep( thread_task->loading_time );
  exit_loading_bays( thread_task );

  pthread_exit(0);
}
