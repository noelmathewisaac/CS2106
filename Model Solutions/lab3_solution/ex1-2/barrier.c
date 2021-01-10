/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "barrier.h"

// Initialise barrier here
void barrier_init ( barrier_t *barrier, int count ) {
    barrier->count = count;
    barrier->waiting_counter = 0;
    sem_init(&barrier->waiting_counter_sem, 0, 1);
    sem_init(&barrier->barrier_sem, 0, 0);
}

void barrier_wait ( barrier_t *barrier ) {
    sem_wait(&barrier->waiting_counter_sem);
    if (barrier->waiting_counter < barrier->count - 1) {
        barrier->waiting_counter++;
        sem_post(&barrier->waiting_counter_sem);
        sem_wait(&barrier->barrier_sem);
        sem_wait(&barrier->waiting_counter_sem);
    }
    if (barrier->waiting_counter > 0) {
        barrier->waiting_counter--;
        sem_post(&barrier->barrier_sem);
    }
    sem_post(&barrier->waiting_counter_sem);
}

// Perform cleanup here if you need to
void barrier_destroy ( barrier_t *barrier ) {
    sem_destroy(&barrier->waiting_counter_sem);
    sem_destroy(&barrier->barrier_sem);
}
