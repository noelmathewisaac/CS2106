/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * Your implementation should go in this file.
 */
#include "entry_controller.h"

void entry_controller_init(entry_controller_t *entry_controller, int loading_bays)
{
    sem_init(&entry_controller->mutex1, 0, 1);
    sem_init(&entry_controller->bay_lock, 0, loading_bays);
    entry_controller->head_index = -1;
    entry_controller->tail_index = 0;
    entry_controller->num_bays = loading_bays;
    entry_controller->num_trains = 0;
}

void entry_controller_wait(entry_controller_t *entry_controller)
{

    //Add train to queue

    sem_wait(&entry_controller->mutex1);
    sem_init(&entry_controller->queue[entry_controller->tail_index], 0, 1);
    // sem_wait(&entry_controller->queue[entry_controller->tail_index]);
    entry_controller->tail_index++;
    sem_post(&entry_controller->mutex1);

    //Wait for loading bay slot
    sem_wait(&entry_controller->bay_lock);

    sem_wait(&entry_controller->mutex1);
    sem_wait(&entry_controller->queue[entry_controller->head_index++]);
    sem_post(&entry_controller->mutex1);
}

void entry_controller_post(entry_controller_t *entry_controller)
{
    sem_post(&entry_controller->bay_lock);
}

void entry_controller_destroy(entry_controller_t *entry_controller)
{
    sem_destroy(&entry_controller->bay_lock);
    sem_destroy(&entry_controller->mutex1);
}
