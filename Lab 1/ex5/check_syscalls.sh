#!/bin/bash

####################
# Lab 1 Exercise 5
# Name: Noel Mathew Isaac
# Student No: A0202072Y
# Lab Group: 3
####################

echo "Printing system call report"

# compile file
gcc -std=c99 pid_checker.c -o ex5
# use strace to get report
strace -c ./ex5
