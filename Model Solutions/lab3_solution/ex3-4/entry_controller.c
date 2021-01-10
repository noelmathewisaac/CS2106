#include "entry_controller.h"
#include <stdio.h>
#include <stdlib.h>

void entry_controller_post( entry_controller_t *entry_controller );

void entry_controller_init( entry_controller_t *entry_controller, int loading_bays ) {
    entry_controller->queue_head = 0;
    entry_controller->queue_tail = 0;
    sem_init(&entry_controller->struct_sem, 0, 1);
    int i;
    for (i = 0; i < ENTRY_CONTROLLER_MAX_USES; i++)
        sem_init(&entry_controller->waiting_sem[i], 0, 0);
    for (i = 0; i < loading_bays; i++)
        entry_controller_post( entry_controller );
}

void entry_controller_wait( entry_controller_t *entry_controller ) {
    sem_wait(&entry_controller->struct_sem);
    if (entry_controller->queue_head >= ENTRY_CONTROLLER_MAX_USES) {
        printf("more fifo sem uses than expected (max: %d)",
               ENTRY_CONTROLLER_MAX_USES);
        exit(1);
    }

    const int prev_queue_tail = entry_controller->queue_tail;
    entry_controller->queue_tail++;
    sem_post(&entry_controller->struct_sem);
    sem_wait(&entry_controller->waiting_sem[prev_queue_tail]);
}

void entry_controller_post( entry_controller_t *entry_controller ) {
    sem_wait(&entry_controller->struct_sem);
    if (entry_controller->queue_head >= ENTRY_CONTROLLER_MAX_USES) {
        printf("more fifo sem uses than expected (max: %d)",
               ENTRY_CONTROLLER_MAX_USES);
        exit(1);
    }
    sem_post(&entry_controller->waiting_sem[entry_controller->queue_head]);
    entry_controller->queue_head++;
    sem_post(&entry_controller->struct_sem);
}

void entry_controller_destroy( entry_controller_t *entry_controller ) {
    sem_destroy(&entry_controller->struct_sem);
    int i;
    for (i = 0; i < ENTRY_CONTROLLER_MAX_USES; i++)
        sem_destroy(&entry_controller->waiting_sem[i]);
}

