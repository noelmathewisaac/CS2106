/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "barrier.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

// Initialise barrier here
void barrier_init(barrier_t *barrier, int count)
{
    barrier->count = count;

    //counter keeps track of number of threads which reached barrier
    barrier->counter = 0;

    //counter_lock is a mutex which allows only 1 thread to increment counter
    sem_init(&barrier->counter_lock, 0, 1);

    //sync_lock used to sync threads at barrier
    sem_init(&barrier->sync_lock, 0, 0);
}

void barrier_wait(barrier_t *barrier)
{
    sem_wait(&barrier->counter_lock);
    barrier->counter++;
    sem_post(&barrier->counter_lock);

    if (barrier->counter == barrier->count)
    {
        //If all threads reach barrier set counter to 0 and signal
        // sync_lock count times
        barrier->counter = 0;
        sem_post(&barrier->sync_lock);
    }
    else
    { //wait till all threads reach barrier
        sem_wait(&barrier->sync_lock);
        sem_post(&barrier->sync_lock);
    }
}

// Perform cleanup here if you need to
void barrier_destroy(barrier_t *barrier)
{
    sem_destroy(barrier->counter_lock);
    sem_destroy(barrier->sync_lock);
}
