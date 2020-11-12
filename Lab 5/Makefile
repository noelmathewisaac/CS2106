CC=gcc
CFLAGS=-g -pthread -std=c99 -fPIC -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

.PHONY: clean

all: runner

runner.o: CFLAGS+=-O2 -std=c11

libzc_io.so: zc_io.o zc_io.h
	$(CC) -shared -pthread -o $@ zc_io.o

runner: runner.o libzc_io.so zc_io.h
	$(CC) -pthread -o $@ runner.o -L. -lzc_io

clean:
	rm *.o *.so runner
