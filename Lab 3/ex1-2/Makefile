CC=gcc
CFLAGS=-g -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
LDLIBS=-lpthread

.PHONY: clean

ex2_runner: fizzbuzz_workers.o barrier.o ex2_runner.o
ex1_runner: barrier.o ex1_runner.o
clean:
	rm barrier.o fizzbuzz_workers.o ex1_runner.o ex1_runner ex2_runner.o ex2_runner

.PHONY: all
all: ex1_runner ex2_runner
