CC=gcc
CFLAGS=-g -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE
LDLIBS=-lpthread -lrt

.PHONY: clean

main: mmf.o main.o
clean:
	rm main.o mmf.o main

.PHONY: all
all: main
