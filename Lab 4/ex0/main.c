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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "mmf.h"

typedef struct {
    int in[2];
    int out[2];
} pipe_pair;

static char file_name_store[20] = "tmpfile";

static const char *find_good_file_name() {
    for (int i=0; true; ++i){
        sprintf(file_name_store+7, "%d", i);
        int fd;
        if ((fd = open(file_name_store, O_RDWR, 0)) == -1) {
            if (errno == ENOENT) return file_name_store;
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

#define MMF_CONNECT 0
#define MMF_DISCONNECT 1
#define MMF_WRITE 2
#define MMF_READ 3

static bool send_command(pipe_pair *pp, uint8_t command) {
    const int fd = pp->in[1];
    int res = write(fd, &command, sizeof(uint8_t));
    return res != -1;
}

static void ack_command(pipe_pair *pp) {
    const int fd = pp->out[1];
    char ch = 0;
    write(fd, &ch, 1);
}

static bool wait_for_ack(pipe_pair *pp) {
    char buf[PIPE_BUF];
    int res = read(pp->out[0], buf, PIPE_BUF);
    if (res == 0) return false; // this usually means that the write end got closed, because the child crashed
    assert(res == 1);
    return true;
}

static int child_proc(pipe_pair *pp, size_t file_size, const char *file_name) {
    char *ptr = NULL;
    
    char buf[PIPE_BUF];
    while (true) {
        int res = read(pp->in[0], buf, PIPE_BUF);
        assert(res == sizeof(uint8_t));
        const uint8_t type = *((uint8_t*)buf);
        switch (type) {
            case MMF_CONNECT:
                ptr = mmf_create_or_open(file_name, file_size);
                ack_command(pp);
                break;
            case MMF_DISCONNECT:
                mmf_close(ptr, file_size);
                return 0;
            case MMF_WRITE:
                assert(ptr != NULL);
                for (size_t i=0; i!=file_size; ++i) {
                    ptr[i] = (char)(rand() & 255);
                }
                ack_command(pp);
                break;
            case MMF_READ:
                assert(ptr != NULL);
                for (size_t i=0; i!=file_size; ++i) {
                    if (ptr[i] != (char)(rand() & 255)) {
                        mmf_close(ptr, file_size);
                        return EXIT_FAILURE;
                    }
                }
                ack_command(pp);
                break;
        }
    }
    
    return EXIT_SUCCESS;
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s file_size [seed]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    size_t file_size;
    sscanf(argv[1], "%zu", &file_size);

    if (argc > 2) {
        const int seed = atoi(argv[2]);
        srand(seed ? seed : time(NULL));
    }
    
    const long page_size = sysconf(_SC_PAGESIZE);
    if(!(file_size > 0 && file_size % page_size == 0)) {
        printf("file_size must be a positive multiple of %ld\n", page_size);
        return EXIT_FAILURE;
    }
    
    // ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    
    // find a name for our file
    const char *const file_name = find_good_file_name();
    
    // create pipes
    pipe_pair ppA, ppB;
    
    // spawn process A
    pipe2(ppA.in, O_DIRECT);
    pipe2(ppA.out, O_DIRECT);
    int pidA = fork();
    assert(pidA != -1);
    if (pidA == 0) {
        close(ppA.in[1]);
        close(ppA.out[0]);
        return child_proc(&ppA, file_size, file_name);
    }
    close(ppA.in[0]);
    close(ppA.out[1]);
    
    // spawn process B
    pipe2(ppB.in, O_DIRECT);
    pipe2(ppB.out, O_DIRECT);
    int pidB = fork();
    assert(pidB != -1);
    if (pidB == 0) {
        close(ppB.in[1]);
        close(ppB.out[0]);
        return child_proc(&ppB, file_size, file_name);
    }
    close(ppB.in[0]);
    close(ppB.out[1]);
    
    // perform the operations
    bool has_error = false;
    if (!(true
            && send_command(&ppA, MMF_CONNECT)
            && wait_for_ack(&ppA)
            && send_command(&ppB, MMF_CONNECT)
            && wait_for_ack(&ppB)
            && send_command(&ppA, MMF_WRITE)
            && wait_for_ack(&ppA)
            && send_command(&ppB, MMF_READ)
            && wait_for_ack(&ppB)
            && send_command(&ppB, MMF_WRITE)
            && wait_for_ack(&ppB)
            && send_command(&ppA, MMF_READ)
            && wait_for_ack(&ppA)
            && send_command(&ppA, MMF_WRITE)
            && wait_for_ack(&ppA)
            && send_command(&ppB, MMF_READ)
            && wait_for_ack(&ppB)
            && send_command(&ppB, MMF_WRITE)
            && wait_for_ack(&ppB)
            && send_command(&ppA, MMF_READ)
            && wait_for_ack(&ppA)
            && send_command(&ppA, MMF_DISCONNECT)
            && send_command(&ppB, MMF_DISCONNECT))) {
        send_command(&ppA, MMF_DISCONNECT);
        send_command(&ppB, MMF_DISCONNECT);
        has_error = true;
    }
    
    // wait for children
    int status;
    waitpid(pidA, &status, 0);
    if (!WIFEXITED(status)) {
        printf("Child A terminated abruptly!\n");
    }
    else if(WEXITSTATUS(status) != EXIT_SUCCESS) {
        printf("Child A read incorrect data!\n");
    }
    else if (!has_error) {
        printf("Child A received data successfully\n");
    }
    waitpid(pidB, &status, 0);
    if (!WIFEXITED(status)) {
        printf("Child B terminated abruptly!\n");
    }
    else if(WEXITSTATUS(status) != EXIT_SUCCESS) {
        printf("Child B read incorrect data!\n");
    }
    else if (!has_error) {
        printf("Child B received data successfully\n");
    }
    
    // remove file
    unlink(file_name);
}
