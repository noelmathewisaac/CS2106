#include "fizzbuzz_workers.h"
#include "barrier.h"

barrier_t barrier0;
barrier_t barrier1;

void fizzbuzz_init ( int n ) {
    (void)n;
    barrier_init( &barrier0, 4 );
    barrier_init( &barrier1, 4 );
}

void num_thread( int n, void (*print_num)(int) ) {
    int i;
    for (i = 0; i < n; i ++) {
        if (i % 5 != 0 && i % 3 != 0)
            print_num(i);
        barrier_wait(&barrier0);
        barrier_wait(&barrier1);
    }
}

void fizz_thread( int n, void (*print_fizz)(void) ) {
    int i;
    for (i = 0; i < n; i ++) {
        if (i % 5 != 0 && i % 3 == 0)
            print_fizz();
        barrier_wait(&barrier0);
        barrier_wait(&barrier1);
    }
}

void buzz_thread( int n, void (*print_buzz)(void) ) {
    int i;
    for (i = 0; i < n; i ++) {
        if (i % 5 == 0 && i % 3 != 0)
            print_buzz();
        barrier_wait(&barrier0);
        barrier_wait(&barrier1);
    }
}

void fizzbuzz_thread( int n, void (*print_fizzbuzz)(void) ) {
    int i;
    for (i = 0; i < n; i ++) {
        if (i % 5 == 0 && i % 3 == 0)
            print_fizzbuzz();
        barrier_wait(&barrier0);
        barrier_wait(&barrier1);
    }
}

void fizzbuzz_destroy() {
    barrier_destroy( &barrier0 );
    barrier_destroy( &barrier1 );
}
