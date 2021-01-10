#include <semaphore.h>

#ifndef __CS2106_EXIT_CONTROLLER_H_
#define __CS2106_EXIT_CONTROLLER_H_

#define MAX_PRIORITIES 5000 // we set a limit on how many possible priorities we have

typedef struct exit_controller {
    int   priorities;
    int   signals_left;
    int   in_queue[MAX_PRIORITIES];
    sem_t exit_controller_mutex;
    sem_t priority_sems[MAX_PRIORITIES];
} exit_controller_t;

void exit_controller_init(exit_controller_t *exit_controller, int no_of_priorities);
void exit_controller_wait(exit_controller_t *exit_controller, int priority);
void exit_controller_post(exit_controller_t *exit_controller, int priority);
void exit_controller_destroy(exit_controller_t *exit_controller);


#endif // __CS2106_EXIT_CONTROLLER_H_
