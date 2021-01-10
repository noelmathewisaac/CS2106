#include "exit_controller.h"
#include <stdio.h>
#include <stdbool.h>

/* Invariants in this data structure:
 * 1 If waits called >= posts called:
 *   - then exit_controller->signals_left = 0,
 *   - and sum(exit_controller->in_queue) = (waits - posts)
 * 2 If waits called < posts called:
 *   - then sum(exit_controller->in_queue) = 0,
 *   - and exit_controller->signals_left = (posts - waits)
 * 3 From 1 & 2, we cannot have any exit->in_queue[i] > 0 when signals_left > 0.
 * 4 for all i, exit->in_queue[i] = waits - posts in (exit->priority_sems[i]) */

void exit_controller_post(exit_controller_t *exit_controller, int priority);

void exit_controller_init(exit_controller_t *exit_controller, int no_of_priorities) {
    exit_controller->priorities = no_of_priorities;
    exit_controller->signals_left = 0;
    sem_init(&exit_controller->exit_controller_mutex, 0, 1);
    int i;
    for (i = 0; i < no_of_priorities; i++)
        sem_init(&exit_controller->priority_sems[i], 0, 0);
    exit_controller_post(exit_controller, 0);
}

void exit_controller_wait(exit_controller_t *exit_controller, int priority) {
    sem_wait(&exit_controller->exit_controller_mutex);
    // From invariant 3, we don't need to check for higher prios
    if (exit_controller->signals_left > 0) {
        exit_controller->signals_left--;
        sem_post(&exit_controller->exit_controller_mutex);
        return;
    }

    // Reaching here means that waits >= posts
    exit_controller->in_queue[priority]++;
    sem_post(&exit_controller->exit_controller_mutex);
    sem_wait(&exit_controller->priority_sems[priority]);
}

void exit_controller_post(exit_controller_t *exit_controller, int priority) {
    (void)priority;
    sem_wait(&exit_controller->exit_controller_mutex);
    bool queue_signalled = false;
    int i;
    for (i = 0; i < exit_controller->priorities; i++) {
        if(exit_controller->in_queue[i] > 0) {
            exit_controller->in_queue[i]--;
            sem_post(&exit_controller->priority_sems[i]);
            queue_signalled = true;
            break;
        }
    }

    // Reaching here means that posts > waits
    if (!queue_signalled) {
        exit_controller->signals_left++;
    }
    sem_post(&exit_controller->exit_controller_mutex);
}

void exit_controller_destroy(exit_controller_t *exit_controller){
    sem_destroy(&exit_controller->exit_controller_mutex);
    int i;
    for (i = 0; i < exit_controller->priorities; i++)
        sem_destroy(&exit_controller->priority_sems[i]);
}
