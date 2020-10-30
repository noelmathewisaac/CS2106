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
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "shmheap.h"

#define SHMHEAP_CONNECT 0
#define SHMHEAP_DISCONNECT 1
#define SHMHEAP_READ 2
#define SHMHEAP_ALLOC 3
#define SHMHEAP_FREE 4

typedef struct {
    int fd[2];
} pipe_pair;

typedef struct {
    pipe_pair in, out;
} bidir_pipe;

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

static int child_proc(const bidir_pipe *bp, const char *mem_name, int child_idx) {
    shmheap_memory_handle mem;
    void *base;
    int input_fd = bp->in.fd[0];
    int output_fd = bp->out.fd[1];
    int res;
    int type;
    while (read(input_fd, &type, sizeof(type)) == sizeof(type)) {
        switch (type) {
            case SHMHEAP_CONNECT: {
                mem = shmheap_connect(mem_name);
                base = shmheap_underlying(mem);
                flock(STDOUT_FILENO, LOCK_EX);
                printf("#%d: Connected\n", child_idx);
                fflush(stdout);
                flock(STDOUT_FILENO, LOCK_UN);
                char dummy = 0;
                write(output_fd, &dummy, sizeof(dummy));
                break;
            }
            case SHMHEAP_DISCONNECT: {
                shmheap_disconnect(mem);
                flock(STDOUT_FILENO, LOCK_EX);
                printf("#%d: Disconnected\n", child_idx);
                fflush(stdout);
                flock(STDOUT_FILENO, LOCK_UN);
                char dummy = 0;
                write(output_fd, &dummy, sizeof(dummy));
                break;
            }
            case SHMHEAP_READ: {
                shmheap_object_handle hdl;
                size_t count;
                res = read(input_fd, &hdl, sizeof(hdl));
                assert(res == sizeof(hdl));
                res = read(input_fd, &count, sizeof(count));
                assert(res == sizeof(count));
                size_t *data = (size_t*)shmheap_handle_to_ptr(mem, hdl);
                flock(STDOUT_FILENO, LOCK_EX);
                printf("#%d: Read:", child_idx);
                for (size_t i=0; i!=count; ++i) {
                    printf(" %zu", data[i]);
                }
                printf("\n");
                fflush(stdout);
                flock(STDOUT_FILENO, LOCK_UN);
                char dummy = 0;
                write(output_fd, &dummy, sizeof(dummy));
                break;
            }
            case SHMHEAP_ALLOC: {
                size_t first;
                size_t count;
                res = read(input_fd, &first, sizeof(first));
                assert(res == sizeof(first));
                res = read(input_fd, &count, sizeof(count));
                assert(res == sizeof(count));
                size_t *data = (size_t*)shmheap_alloc(mem, sizeof(size_t) * count);
                flock(STDOUT_FILENO, LOCK_EX);
                printf("#%d: Allocated at offset %zu:", child_idx, (char*)data - (char*)base);
                for (size_t i=0; i!=count; ++i) {
                    data[i] = first++;
                    printf(" %zu", data[i]);
                }
                printf("\n");
                fflush(stdout);
                flock(STDOUT_FILENO, LOCK_UN);
                shmheap_object_handle hdl = shmheap_ptr_to_handle(mem, data);
                write(output_fd, &hdl, sizeof(hdl));
                break;
            }
            case SHMHEAP_FREE: {
                shmheap_object_handle hdl;
                res = read(input_fd, &hdl, sizeof(hdl));
                assert(res == sizeof(hdl));
                size_t *data = (size_t*)shmheap_handle_to_ptr(mem, hdl);
                shmheap_free(mem, data);
                flock(STDOUT_FILENO, LOCK_EX);
                printf("#%d: Freed at offset: %zu\n", child_idx, (char*)data - (char*)base);
                fflush(stdout);
                flock(STDOUT_FILENO, LOCK_UN);
                char dummy = 0;
                write(output_fd, &dummy, sizeof(dummy));
                break;
            }
        }
    }
    
    return EXIT_SUCCESS;
}

typedef struct {
    int type;
    int index;
    int id;
    size_t sz;
} command_t;

int main () {    
    int num_proc, num_objects;
    size_t mem_size;
    scanf("%d%zu%d", &num_proc, &mem_size, &num_objects);
    // eat the newline;
    {
        char linebuf[1<<16];
        char *ret = fgets(linebuf, 1<<16, stdin);
        assert (ret != NULL);
    }
    
    const long page_size = sysconf(_SC_PAGESIZE);
    if(!(mem_size > 0 && mem_size % page_size == 0)) {
        printf("file_size must be a positive multiple of %ld\n", page_size);
        return EXIT_FAILURE;
    }

    assert(num_proc > 0);
    assert(num_objects > 0);
    
    size_t start_index = 0;
    
    // find a name for our shm heap
    const char *const mem_name = find_good_shm_name();
    
    // create pipes
    bidir_pipe *const pp = malloc(sizeof(bidir_pipe) * num_proc);
    
    // spawn children
    for (int i=0; i!=num_proc; ++i) {
        pipe2(pp[i].in.fd, O_DIRECT);
        pipe2(pp[i].out.fd, O_DIRECT);
        int res = fork();
        assert(res != -1);
        if (res == 0) {
            close(pp[i].in.fd[1]);
            close(pp[i].out.fd[0]);
            const bidir_pipe curr_pp = pp[i];
            free(pp);
            return child_proc(&curr_pp, mem_name, i);
        }
        close(pp[i].in.fd[0]);
        close(pp[i].out.fd[1]);
    }
    
    // init the shm heap
    shmheap_memory_handle mem = shmheap_create(mem_name, mem_size);
    
    // store objects and sizes
    shmheap_object_handle *objects_arr = malloc(sizeof(shmheap_object_handle) * num_objects);
    size_t *sizes_arr = malloc(sizeof(size_t) * num_objects);
    
    // read the input
    char linebuf[1<<16];
    command_t commands[1<<16];
    command_t *commands_end;
    while (true) {
        char *const ret = fgets(linebuf, 1<<16, stdin);
        if (ret == NULL && feof(stdin)) break; // EOF
        assert(ret != NULL);
        commands_end = commands;
        const char *lineptr = linebuf;
        int loc;
        command_t cmd;
        while(true) {
            int ret1 = sscanf(lineptr, "%d%d%n", &cmd.type, &cmd.index, &loc);
            assert(0 <= cmd.type && cmd.type < 5);
            assert(0 <= cmd.index && cmd.index < num_proc);
            if (ret1 == EOF) break;
            assert(ret1 == 2);
            lineptr += loc;
            switch (cmd.type) {
                case SHMHEAP_CONNECT: break;
                case SHMHEAP_DISCONNECT: break;
                case SHMHEAP_READ:
                case SHMHEAP_FREE: {
                    int ret3 = sscanf(lineptr, "%d%n", &cmd.id, &loc);
                    assert(0 <= cmd.id && cmd.id < num_objects);
                    assert(ret3 == 1);
                    lineptr += loc;
                    break;
                }
                case SHMHEAP_ALLOC: {
                    int ret3 = sscanf(lineptr, "%d%zu%n", &cmd.id, &cmd.sz, &loc);
                    assert(0 <= cmd.id && cmd.id < num_objects);
                    assert(ret3 == 2);
                    lineptr += loc;
                    break;
                }
            }
            *commands_end++ = cmd;
            char tmp;
            int ret2 = sscanf(lineptr, " %1[|]%n", &tmp, &loc);
            if (ret2 == EOF) break;
            assert(ret2 == 1);
            lineptr += loc;
        }
        if (commands == commands_end) break; // probably EOF
        for (command_t *it = commands; it != commands_end; ++it) {
            switch (it->type) {
                case SHMHEAP_CONNECT:
                case SHMHEAP_DISCONNECT: {
                    write(pp[it->index].in.fd[1], &it->type, sizeof(it->type));
                    break;
                }
                case SHMHEAP_READ: {
                    write(pp[it->index].in.fd[1], &it->type, sizeof(it->type));
                    write(pp[it->index].in.fd[1], &objects_arr[it->id], sizeof(objects_arr[it->id]));
                    write(pp[it->index].in.fd[1], &sizes_arr[it->id], sizeof(sizes_arr[it->id]));
                    break;
                }
                case SHMHEAP_ALLOC: {
                    write(pp[it->index].in.fd[1], &it->type, sizeof(it->type));
                    write(pp[it->index].in.fd[1], &start_index, sizeof(start_index));
                    write(pp[it->index].in.fd[1], &it->sz, sizeof(it->sz));
                    start_index += it->sz;
                    break;
                }
                case SHMHEAP_FREE: {
                    write(pp[it->index].in.fd[1], &it->type, sizeof(it->type));
                    write(pp[it->index].in.fd[1], &objects_arr[it->id], sizeof(objects_arr[it->id]));
                    break;
                }
            }
        }
        for (command_t *it = commands; it != commands_end; ++it) {
            switch (it->type) {
                case SHMHEAP_CONNECT:
                case SHMHEAP_DISCONNECT:
                case SHMHEAP_READ:
                case SHMHEAP_FREE: {
                    char dummy;
                    read(pp[it->index].out.fd[0], &dummy, sizeof(dummy));
                    break;
                }
                case SHMHEAP_ALLOC: {
                    shmheap_object_handle hdl;
                    read(pp[it->index].out.fd[0], &hdl, sizeof(hdl));
                    objects_arr[it->id] = hdl;
                    sizes_arr[it->id] = it->sz;
                    break;
                }
            }
        }
    }
    
    free(sizes_arr);
    free(objects_arr);

    // close the input pipes, so that the children will terminate
    for (int i=0; i!=num_proc; ++i) {
        close(pp[i].in.fd[1]);
    }
    
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
            // success... keep quiet
        }
    }
    
    // destroy shm
    shmheap_destroy(mem_name, mem);
}
