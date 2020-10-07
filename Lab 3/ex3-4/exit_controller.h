/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * This file contains declarations. You should only modify the exit_controller_t struct,
 * as the method signatures will be needed to compile with the runner.
 */
#ifndef __CS2106_EXIT_CONTROLLER_H_
#define __CS2106_EXIT_CONTROLLER_H_

#include <semaphore.h>

#define MAX_PRIORITIES 5000 // we set a limit on how many possible priorities we have

typedef struct exit_controller {
    // define your variables here
} exit_controller_t;

void exit_controller_init(exit_controller_t *exit_controller, int no_of_priorities);
void exit_controller_wait(exit_controller_t *exit_controller, int priority);
void exit_controller_post(exit_controller_t *exit_controller, int priority);
void exit_controller_destroy(exit_controller_t *exit_controller);


#endif // __CS2106_EXIT_CONTROLLER_H_
