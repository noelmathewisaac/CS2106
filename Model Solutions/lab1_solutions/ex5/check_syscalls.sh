#!/bin/bash

echo "Printing system call report"

# compile file
gcc -std=c99 pid_checker.c -o ex5

# use strace to get report
strace -c ./ex5
