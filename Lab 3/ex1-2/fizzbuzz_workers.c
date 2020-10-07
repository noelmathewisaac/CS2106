/**
 * CS2106 AY 20/21 Semester 1 - Lab 3
 *
 * Your implementation should go in this file.
 */
#include "fizzbuzz_workers.h"
#include "barrier.h" // you may use barriers if you think it can help your
                     // implementation

// declare variables to be used here
sem_t lock;
int num;

void fizzbuzz_init(int n)
{
    sem_init(&lock, 0, 1);
    num = 1;
}

void num_thread(int n, void (*print_num)(int))
{
    while (1)
    {
        sem_wait(&lock);

        if (num > n)
        {
            break;
        }
        if (num % 3 != 0 && num % 5 != 0)
        {
            (*print_num)(num);
            num++;
        }
        sem_post(&lock);
    }
    sem_post(&lock);
}

void fizz_thread(int n, void (*print_fizz)(void))
{
    while (1)
    {
        sem_wait(&lock);
        if (num > n)
        {
            break;
        }
        if (num % 3 == 0 && num % 5 != 0)
        {
            (*print_fizz)();
            num++;
        }
        sem_post(&lock);
    }
    sem_post(&lock);
}

void buzz_thread(int n, void (*print_buzz)(void))
{
    while (1)
    {
        sem_wait(&lock);

        if (num > n)
        {
            break;
        }
        if (num % 3 != 0 && num % 5 == 0)
        {
            (*print_buzz)();
            num++;
        }
        sem_post(&lock);
    }
    sem_post(&lock);
}

void fizzbuzz_thread(int n, void (*print_fizzbuzz)(void))
{
    while (1)
    {
        sem_wait(&lock);

        if (num > n)
        {
            break;
        }

        if (num % 3 == 0 && num % 5 == 0)
        {
            (*print_fizzbuzz)();
            num++;
        }
        sem_post(&lock);
    }
    sem_post(&lock);
}

void fizzbuzz_destroy()
{
    sem_destroy(&lock);
}
