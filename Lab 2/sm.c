/**
 * CS2106 AY 20/21 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "sm.h"
#include <sys/wait.h>
#include <stdlib.h>

pid_t pid[32];
char path[32][32];
bool running[32];
int process_count;
int status;

// Use this function to any initialisation if you need to.
void sm_init(void)
{
    pid[0] = getpid();
    running[0] = false;
    process_count = 0;
}

// Use this function to do any cleanup of resources.
void sm_free(void)
{
}

// Exercise 1a/2: start services
void sm_start(const char *processes[])
{
    int id = fork();
    if (id == 0)
    {
        execv(processes[0], processes);
    }
    else
    {
        pid[process_count] = id;
        strcpy(path[process_count], processes[0]);
        running[process_count] = true;
    }

    process_count++;
}

// Exercise 1b: print service status
size_t sm_status(sm_status_t statuses[])
{
    for (int i = 0; i < process_count; i++)
    {
        if (waitpid(pid[i], &status, WNOHANG) != 0)
        {
            running[i] = false;
        }
        statuses[i].path = path[i];
        statuses[i].pid = pid[i];
        statuses[i].running = running[i];
    }
    return process_count;
}

// Exercise 3: stop service, wait on service, and shutdown
void sm_stop(size_t index)
{
}

void sm_wait(size_t index)
{
}

void sm_shutdown(void)
{
}

// Exercise 4: start with output redirection
void sm_startlog(const char *processes[])
{
}

// Exercise 5: show log file
void sm_showlog(size_t index)
{
}
