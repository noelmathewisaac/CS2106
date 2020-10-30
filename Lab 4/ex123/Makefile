CC=gcc
CFLAGS=-g -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
LDLIBS=-lpthread -lrt

.PHONY: all
all: runner_ex1 runner_ex2 runner_ex3

.PHONY: clean

runner_ex1: shmheap.o runner_ex1.o
runner_ex2: shmheap.o runner_ex2.o
runner_ex3: shmheap.o runner_ex3.o
clean:
	rm runner_ex1.o runner_ex2.o runner_ex3.o shmheap.o runner_ex1 runner_ex2 runner_ex3
