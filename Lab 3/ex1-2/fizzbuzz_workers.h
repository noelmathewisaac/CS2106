/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * Do not modify this file.
 */
#ifndef __CS2106_FIZZBUZZ_WORKERS_H_
#define __CS2106_FIZZBUZZ_WORKERS_H_

void fizzbuzz_init( int n );

void num_thread( int n, void (*num_printer)(int) );

void fizz_thread( int n, void (*fizz_printer)(void) );

void buzz_thread( int n, void (*buzz_printer)(void) );

void fizzbuzz_thread( int n, void (*fizzbuzz_printer)(void) );

void fizzbuzz_destroy();

#endif // __CS2106_FIZZBUZZ_WORKERS_H_
