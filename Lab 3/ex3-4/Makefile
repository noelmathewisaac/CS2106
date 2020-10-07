CC=gcc
CFLAGS=-g -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
LDLIBS=-lpthread

.PHONY: clean

train_runner: entry_controller.o exit_controller.o train_runner.o
clean:
	rm entry_controller.o exit_controller.o train_runner.o train_runner

.PHONY: all
all: train_runner
