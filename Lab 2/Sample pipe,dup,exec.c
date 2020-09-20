/**
 * CS2106 AY 20/21 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char const *argv[])
{
    int i = 0;
    while (i < 1)
    {
        int pipefd[2];
        int pipefd2[2];
        pipe(pipefd);
        pipe(pipefd2);

        int pid = fork();
        if (pid == 0)
        {
            {
                close(pipefd[READ_END]);
                dup2(pipefd[WRITE_END], STDOUT_FILENO);
                execlp(argv[1], argv[1], argv[2], NULL);
            }
        }

        pid = fork();
        if (pid == 0)
        {
            {
                close(pipefd[WRITE_END]);
                close(pipefd2[READ_END]);
                dup2(pipefd[READ_END], STDIN_FILENO);
                dup2(pipefd2[WRITE_END], STDOUT_FILENO);
                execlp(argv[3], argv[3], NULL);
            }
        }
        pid = fork();
        if (pid == 0)
        {
            {
                close(pipefd2[WRITE_END]);
                dup2(pipefd2[READ_END], STDIN_FILENO);
                execlp(argv[4], argv[4], NULL);
            }
        }

        close(pipefd[WRITE_END]);
        close(pipefd[READ_END]);
        close(pipefd2[WRITE_END]);
        close(pipefd2[READ_END]);

        i++;
    }
    return 0;
}
