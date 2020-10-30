/**
 * This runner tests ex1 by creating a shared heap,
 * allocating one object in it, and sending it to
 * `num_receiver_processes` other processes via a pipe.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "shmheap.h"

typedef struct {
    int fd[2];
} pipe_pair;

static char shm_name_store[20]="/shmheap";

static const char *find_good_shm_name() {
    for (int i=0; true; ++i){
        sprintf(shm_name_store+8, "%d", i);
        //itoa(i, shm_name_store+8, 10);
        int fd;
        if ((fd = shm_open(shm_name_store, O_RDWR, 0)) == -1) {
            if (errno == ENOENT) return shm_name_store;
            else if (errno == EINVAL || errno == EMFILE || errno == ENAMETOOLONG || errno == ENFILE) {
                printf("Unexpected error\n");
                exit(EXIT_FAILURE);
            }
        }
        else {
            close(fd);
        }
    }
}

static int randint(int min, int max) {
    return rand() % (max - min + 1) + min;
}

#define MEM_SIZE (1 << 16)

static int child_proc(int input_fd, size_t num_bytes, const char *mem_name) {
    // wait until the heap has been created
    {
        char buf[PIPE_BUF];
        int res = read(input_fd, buf, PIPE_BUF);
        assert(res == 1);
    }
    
    // connect to heap
    shmheap_memory_handle mem = shmheap_connect(mem_name);
    
    // retrieve the handle
    char *ptr;
    {
        char buf[PIPE_BUF];
        assert(read(input_fd, buf, PIPE_BUF) == sizeof(shmheap_object_handle));
        const shmheap_object_handle obj = *((const shmheap_object_handle *)buf);
        ptr = shmheap_handle_to_ptr(mem, obj);
    }
    
    // check that the data is correct
    for (size_t i=0; i!=num_bytes; ++i){
        if (ptr[i] != (char)(rand() & 255)) {
            shmheap_disconnect(mem);
            return EXIT_FAILURE;
        }
    }
    
    shmheap_disconnect(mem);
    return EXIT_SUCCESS;
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s num_receiver_processes [seed]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const int num_proc = atoi(argv[1]);

    if (argc > 2) {
        const int seed = atoi(argv[2]);
        srand(seed ? seed : time(NULL));
    }
    
    assert(num_proc > 0);
    
    // find a name for our shm heap
    const char *const mem_name = find_good_shm_name();
    
    // create pipes
    pipe_pair *const pp = malloc(sizeof(pipe_pair) * num_proc);
    
    // amount of bytes to allocate (done before forking so the PRNG will be in sync)
    const size_t num_bytes = randint(1, MEM_SIZE - 16);
    
    // spawn children
    for (int i=0; i!=num_proc; ++i) {
        pipe2(pp[i].fd, O_DIRECT);
        int res = fork();
        assert(res != -1);
        if (res == 0) {
            close(pp[i].fd[1]);
            int input_fd = pp[i].fd[0];
            free(pp);
            return child_proc(input_fd, num_bytes, mem_name);
        }
        close(pp[i].fd[0]);
    }
    
    // init the shm heap
    shmheap_memory_handle mem = shmheap_create(mem_name, MEM_SIZE);
    
    // tell children that we have created the heap
    for (int i=0; i!=num_proc; ++i) {
        int dummy = 0;
        write(pp[i].fd[1], &dummy, 1);
    }
    
    // alloc some memory
    char *const ptr = shmheap_alloc(mem, num_bytes);
    
    // write random stuff to the memory
    for (size_t i=0; i!=num_bytes; ++i){
        ptr[i] = (char)(rand() & 255);
    }
    
    // convert the pointer to handle
    const shmheap_object_handle obj = shmheap_ptr_to_handle(mem, ptr);
    
    // send the object handle to children
    for (int i=0; i!=num_proc; ++i) {
        write(pp[i].fd[1], (const void *)(&obj), sizeof(shmheap_object_handle));
    }

    // free pipes
    free(pp);

    // wait for children
    for (int i=0; i!=num_proc; ++i) {
        int status;
        int pid;
        if ((pid = wait(&status)) == -1) {
            printf("Child mysteriously disappeared\n");
        }
        else if (!WIFEXITED(status)) {
            printf("Child [pid = %d] terminated abruptly!\n", pid);
        }
        else if(WEXITSTATUS(status) != EXIT_SUCCESS) {
            printf("Child [pid = %d] read incorrect data!\n", pid);
        }
        else {
            printf("Child [pid = %d] received data successfully\n", pid);
        }
    }
    
    // destroy shm
    shmheap_destroy(mem_name, mem);
}
