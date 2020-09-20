/**
 * CS2106 AY 20/21 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "sm.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1

pid_t pid[1000];
char path[1000][100];
bool running[1000];
int total_process_count;
int status;
int services[32];
int service_index = 0;
int pipefd[2];

// Use this function to any initialisation if you need to.
void sm_init(void)
{
    pid[0] = getpid();
    running[0] = false;
    total_process_count = 0;
}

// Use this function to do any cleanup of resources.
void sm_free(void)
{
}

// Exercise 1a/2: start services
void sm_start(const char *processes[])
{
    char **ptr = processes;
    int pos1 = 0;
    int pos2 = 0;
    int count = 0;
    int num_processes = 0;
    int process_count = 0;
    int fds[31][2];

    //count number of arguments and processes in the service (except last NULL for piped processes)
    while (!(*ptr == NULL && *(ptr - 1) == NULL))
    {
        count++;
        if (*ptr == NULL)
        {
            num_processes++;
        }
        ptr++;
    }

    //Make n-1 pipes for n processes
    for (int i = 0; i < num_processes - 1; i++)
    {
        pipe(fds[i]);
    }

    for (int i = 0; i < count; i++)
    {

        //iterate through array, store the strings for each process to a new array and perform fork, dup, exec
        if (processes[i] == NULL)
        {
            char *ind_process[1000];

            for (int j = 0; j < pos1 - pos2 + 1; j++)
            {
                ind_process[j] = (char *)processes[pos2 + j];
            }

            int id = fork();

            //child process
            if (id == 0)
            {
                //multiprocess service
                if (num_processes > 1)
                {
                    //First process in a service
                    if (process_count == 0)
                    {
                        //close the read end of the first pipe and the STD input of the process
                        close(fds[process_count][READ_END]);
                        close(STDIN_FILENO);

                        //connect the STD output of the process to the write end of the pipe.
                        //The output from exec will be redirected to the pipe
                        dup2(fds[process_count][WRITE_END], STDOUT_FILENO);

                        //close all the unused pipe ends
                        //(all the used ends will be closed automatically after exec)
                        for (int i = 0; i < num_processes - 1; i++)
                        {
                            if (i != process_count - 1)
                            {
                                close(fds[i][READ_END]);
                                close(fds[i][WRITE_END]);
                            }
                        }
                        execv(ind_process[0], ind_process);
                    }

                    //Last process in a service
                    else if (process_count + 1 == num_processes)
                    {
                        //close the write end of the last pipe
                        close(fds[process_count - 1][WRITE_END]);

                        //connect the STD input of the process to the read end of the pipe.
                        //The output from the pipe will be redirected to process
                        dup2(fds[process_count - 1][READ_END], STDIN_FILENO);

                        //close all the unused pipe ends
                        for (int i = 0; i < num_processes - 1; i++)
                        {
                            if (i != process_count - 1)
                            {
                                close(fds[i][READ_END]);
                                close(fds[i][WRITE_END]);
                            }
                        }
                        execv(ind_process[0], ind_process);
                    }

                    //For in between processes (if number of processes > 2). This process is in bewtween 2 pipes.
                    else
                    {
                        //close the write end of the previous pipe
                        //connect the STD input of the process to the read end of the pipe.
                        close(fds[process_count - 1][WRITE_END]);
                        dup2(fds[process_count - 1][READ_END], STDIN_FILENO);

                        //close the read end of the next pipe
                        //connect the STD output of the process to the write end of the pipe.
                        close(fds[process_count][READ_END]);
                        dup2(fds[process_count][WRITE_END], STDOUT_FILENO);

                        //close all ununsed pipe ends
                        for (int i = 0; i < num_processes - 1; i++)
                        {
                            if (!(i == process_count || i == process_count - 1))
                            {
                                close(fds[i][READ_END]);
                                close(fds[i][WRITE_END]);
                            }
                        }
                        execv(ind_process[0], ind_process);
                    }
                }
                else
                {
                    execv(ind_process[0], ind_process);
                }
            }

            // parent process

            //store all the process details into arrays
            pid[total_process_count] = id;
            strcpy(path[total_process_count], ind_process[0]);
            running[total_process_count] = true;

            //update couters
            pos2 = pos1 + 1;
            process_count++;
            total_process_count++;

            //Update service count
            if (i + 1 == count)
            {
                services[service_index] = total_process_count - 1;
                service_index++;
            }
        }
        pos1++;
    }

    for (int i = 0; i < num_processes - 1; i++)
    {
        close(fds[i][READ_END]);
        close(fds[i][WRITE_END]);
    }
}

// Exercise 1b: print service status
size_t sm_status(sm_status_t statuses[])
{
    for (int i = 0; i < total_process_count; i++)
    {
        //check if process finished
        if (waitpid(pid[i], &status, WNOHANG) != 0)
        {
            running[i] = false;
        }
    }

    for (int i = 0; i < service_index; i++)
    {
        statuses[i].path = path[services[i]];
        statuses[i].pid = pid[services[i]];
        statuses[i].running = running[services[i]];
    }

    return service_index;
}

// Exercise 3: stop service, wait on service, and shutdown
void sm_stop(size_t index)
{
    int initial = 0;
    if (index == 0)
    {
        for (int i = 0; i <= services[index]; i++)
        {
            //check if process is running
            if (waitpid(pid[i], &status, WNOHANG) == 0)
            {
                kill(pid[i], SIGTERM);
            }
        }
    }
    else
    {
        initial = services[index - 1];

        for (int i = initial + 1; i <= services[index]; i++)
        {
            if (waitpid(pid[i], &status, WNOHANG) == 0)
            {
                kill(pid[i], SIGTERM);
            }
        }
    }
}

void sm_wait(size_t index)
{
    int initial = 0;
    if (index == 0)
    {
        for (int i = 0; i <= services[index]; i++)

        {
            if (waitpid(pid[i], &status, WNOHANG) == 0)
            {
                waitpid(pid[i], &status, 0);
            }
        }
    }
    else
    {
        initial = services[index - 1];

        for (int i = initial + 1; i <= services[index]; i++)
        {
            if (waitpid(pid[i], &status, WNOHANG) == 0)
            {
                waitpid(pid[i], &status, 0);
            }
        }
    }
}

void sm_shutdown(void)
{
    for (int i = 0; i <= service_index; i++)
    {
        sm_stop(i);
    }
}

// Exercise 4: start with output redirection
void sm_startlog(const char *processes[])
{
    char **ptr = processes;
    int pos1 = 0;
    int pos2 = 0;
    int count = 0;
    int num_processes = 0;
    int process_count = 0;
    int fds[31][2];

    //count number of arguments and processes (except last NULL for piped processes)
    while (!(*ptr == NULL && *(ptr - 1) == NULL))
    {
        count++;
        if (*ptr == NULL)
        {
            num_processes++;
        }
        ptr++;
    }

    for (int i = 0; i < num_processes - 1; i++)
    {
        pipe(fds[i]);
    }

    for (int i = 0; i < count; i++)
    {

        if (processes[i] == NULL)
        {
            char *ind_process[1000];
            for (int j = 0; j < pos1 - pos2 + 1; j++)
            {
                ind_process[j] = (char *)processes[pos2 + j];
            }

            int id = fork();

            //child process
            if (id == 0)
            {
                if (num_processes > 1)
                {
                    if (process_count == 0)
                    {
                        close(fds[process_count][READ_END]);
                        close(STDIN_FILENO);
                        dup2(fds[process_count][WRITE_END], STDOUT_FILENO);
                        for (int i = 0; i < num_processes - 1; i++)
                        {
                            if (i != process_count - 1)
                            {
                                close(fds[i][READ_END]);
                                close(fds[i][WRITE_END]);
                            }
                        }
                        execv(ind_process[0], ind_process);
                    }
                    else if (process_count + 1 == num_processes)
                    {
                        close(fds[process_count - 1][WRITE_END]);
                        dup2(fds[process_count - 1][READ_END], STDIN_FILENO);
                        for (int i = 0; i < num_processes - 1; i++)
                        {
                            if (i != process_count - 1)
                            {
                                close(fds[i][READ_END]);
                                close(fds[i][WRITE_END]);
                            }
                        }
                        char filename[15];
                        snprintf(filename, 15, "service%d.log", service_index);
                        int out = open(filename, O_RDWR | O_CREAT | O_APPEND, 0600);
                        if (-1 == out)
                        {
                            perror("Failed to open %s");
                            exit(1);
                        }
                        int save_out = dup(STDOUT_FILENO);

                        if (-1 == dup2(out, STDOUT_FILENO))
                        {
                            perror("Error redirecting stdout");
                            exit(1);
                        }
                        if (-1 == dup2(out, STDERR_FILENO))
                        {
                            perror("Error redirecting stdout");
                            exit(1);
                        }
                        execv(ind_process[0], ind_process);
                        fflush(stdout);
                        fflush(stderr);
                        close(out);

                        dup2(save_out, STDOUT_FILENO);
                        dup2(save_out, STDERR_FILENO);

                        close(save_out);
                    }
                    else
                    {
                        close(fds[process_count - 1][WRITE_END]);
                        close(fds[process_count][READ_END]);
                        dup2(fds[process_count - 1][READ_END], STDIN_FILENO);
                        dup2(fds[process_count][WRITE_END], STDOUT_FILENO);
                        for (int i = 0; i < num_processes - 1; i++)
                        {
                            if (!(i == process_count || i == process_count - 1))
                            {
                                close(fds[i][READ_END]);
                                close(fds[i][WRITE_END]);
                            }
                        }
                        execv(ind_process[0], ind_process);
                    }
                }
                else
                {
                    char filename[15];
                    snprintf(filename, 15, "service%d.log", service_index);
                    int out = open(filename, O_RDWR | O_CREAT | O_APPEND, 0600);
                    if (-1 == out)
                    {
                        perror("Failed to open file");
                        exit(1);
                    }
                    int save_out = dup(STDOUT_FILENO);

                    if (-1 == dup2(out, STDOUT_FILENO))
                    {
                        perror("Error redirecting stdout");
                        exit(1);
                    }
                    if (-1 == dup2(out, STDERR_FILENO))
                    {
                        perror("Error redirecting stderr");
                        exit(1);
                    }

                    execv(ind_process[0], ind_process);

                    fflush(stdout);
                    fflush(stderr);
                    close(out);

                    dup2(save_out, STDOUT_FILENO);
                    dup2(save_out, STDERR_FILENO);

                    close(save_out);
                }
            }

            // parent process

            pid[total_process_count] = id;
            strcpy(path[total_process_count], ind_process[0]);
            running[total_process_count] = true;

            pos2 = pos1 + 1;
            process_count++;
            total_process_count++;

            if (i + 1 == count)
            {
                services[service_index] = total_process_count - 1;
                service_index++;
            }
        }
        pos1++;
    }

    for (int i = 0; i < num_processes - 1; i++)
    {
        close(fds[i][READ_END]);
        close(fds[i][WRITE_END]);
    }
}

// Exercise 5: show log file
void sm_showlog(size_t index)
{
    FILE *fileptr;
    char filename[15];
    char character;

    snprintf(filename, 15, "service%ld.log", index);

    int pid = fork();

    if (pid == 0)
    {
        //try opening file and print msg if error/dosent exist
        fileptr = fopen(filename, "r");
        if (fileptr == NULL)
        {
            printf("service has no log file");
            exit(0);
        }

        // Read contents from file
        character = fgetc(fileptr);
        while (character != EOF)
        {
            printf("%c", character);
            character = fgetc(fileptr);
        }

        fclose(fileptr);
    }
    wait(NULL);
}
