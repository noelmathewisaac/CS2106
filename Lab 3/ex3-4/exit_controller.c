/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * This file contains declarations. You should only modify the exit_controller_t struct,
 * as the method signatures will be needed to compile with the runner.
 */
#include "exit_controller.h"
#include <stdio.h>
#include <pthread.h>

void exit_controller_init(exit_controller_t *exit_controller, int no_of_priorities)
{
    exit_controller->num_priorities = no_of_priorities;
    sem_init(&exit_controller->mutex, 0, 1);
    sem_init(&exit_controller->exit_line, 0, 1);

    pthread_cond_init(&exit_controller->priority_zero, NULL);
    pthread_mutex_init(&exit_controller->mu, NULL);

    exit_controller->counter = 0;
    exit_controller->p0_count = 0;
    exit_controller->p1_count = 0;
}

void exit_controller_wait(exit_controller_t *exit_controller, int priority)
{

    if (priority == 0)
    {
        //Increment p0 count
        sem_wait(&exit_controller->mutex);
        exit_controller->p0_count++;
        printf("----------\nWaiting\nP0 count -> %d; p1 count -> %d; priority -> %d\n------------\n", exit_controller->p0_count, exit_controller->p1_count, priority);
        sem_post(&exit_controller->mutex);

        //wait to enter exit line
        sem_wait(&exit_controller->exit_line);

        //decrement p0 count and signal if there are no more trains with priority 0 waiting
        sem_wait(&exit_controller->mutex);
        exit_controller->p0_count--;
        pthread_mutex_lock(&exit_controller->mu);
        if (exit_controller->p0_count == 0)
            pthread_cond_signal(&exit_controller->priority_zero);
        pthread_mutex_unlock(&exit_controller->mu);
        printf("----------\nExit line entered\nP0 count -> %d; p1 count -> %d; priority -> %d\n------------\n", exit_controller->p0_count, exit_controller->p1_count, priority);
        sem_post(&exit_controller->mutex);
    }
    else
    {
        //increment p1_count
        sem_wait(&exit_controller->mutex);
        exit_controller->p1_count++;
        printf("----------\nWaiting\nP0 count -> %d; p1 count -> %d; priority -> %d\n------------\n", exit_controller->p0_count, exit_controller->p1_count, priority);
        sem_post(&exit_controller->mutex);

        //block if there are any trains with priority 0 waiting
        pthread_mutex_lock(&exit_controller->mu);
        while (exit_controller->p0_count != 0)
        {
            pthread_cond_wait(&exit_controller->priority_zero, &exit_controller->mu);
        }
        pthread_mutex_unlock(&exit_controller->mu);

        //Decrement p1 and wait to enter exit line
        sem_wait(&exit_controller->mutex);
        sem_wait(&exit_controller->exit_line);
        exit_controller->p1_count--;
        printf("----------\nExit line entered\nP0 count -> %d; p1 count -> %d; priority -> %d\n------------\n", exit_controller->p0_count, exit_controller->p1_count, priority);
        sem_post(&exit_controller->mutex);
    }
}

void exit_controller_post(exit_controller_t *exit_controller, int priority)
{
    sem_post(&exit_controller->exit_line);
}

void exit_controller_destroy(exit_controller_t *exit_controller)
{
    sem_destroy(&exit_controller->exit_line);
    sem_destroy(&exit_controller->mutex);
}
