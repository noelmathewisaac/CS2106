/*
	You can modify this file for your testing purpose 
	but note that we will use our own copies of this file for testing during grading so any changes in this file will be overwritten
*/

#include <stdio.h>
#include <unistd.h>

/*C program to get Process Id and Parent Process Id in Linux.*/
int main()
{
    int p_id,p_pid;

    p_id = getpid();  /*process id*/
    p_pid = getpid(); /*parent process id*/

    printf("Process ID: %d\n",p_id);
    printf("Parent Process ID: %d\n",p_pid);

    return 0;
}
