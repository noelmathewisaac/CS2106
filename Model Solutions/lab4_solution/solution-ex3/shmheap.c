/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shmheap.h"

shmheap_memory_handle shmheap_create(const char *name, size_t len) {
    assert((len & 7) == 0);
    int fd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (ftruncate(fd, len)) abort();
    shmheap_block *mem = (shmheap_block *) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    mem->len = len;
    shmheap_node *n = (shmheap_node *) mem->data;
    n->sz = len - sizeof(shmheap_block);
    close(fd);
    // set up the mutex
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mem->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    return mem;
}

void shmheap_destroy(const char *name, shmheap_memory_handle mem) {
    // destroy the mutex
    pthread_mutex_destroy(&mem->mutex);
    
    munmap(mem, mem->len);
    shm_unlink(name);
}

shmheap_memory_handle shmheap_connect(const char *name) {
    int fd = shm_open(name, O_RDWR, 0);
    shmheap_block *mem = (shmheap_block *) mmap(NULL, sizeof(shmheap_block), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    size_t len = mem->len;
    mem = (shmheap_block *) mremap(mem, sizeof(shmheap_block), len, MREMAP_MAYMOVE);
    close(fd);
    // note: do we need to mprotect the new pages?
    return mem;
}

void shmheap_disconnect(shmheap_memory_handle mem) {
    munmap(mem, mem->len);
}

void *shmheap_underlying(shmheap_memory_handle mem) {
    return (void *) mem;
}

static bool node_used(shmheap_node *n) {
    return n->sz & 1;
}

static size_t node_size(shmheap_node *n) {
    return n->sz & ~(size_t)7;
}
    
void *shmheap_alloc(shmheap_memory_handle mem, size_t sz) {
    pthread_mutex_lock(&mem->mutex);
    sz = (sz + 15) & ~((size_t)7);
    shmheap_node *end_node = (shmheap_node *) (((char *) mem) + mem->len);
    shmheap_node *curr = (shmheap_node *) mem->data;
    void* ret = NULL;
    while (curr != end_node) {
        const size_t curr_node_size = node_size(curr);
        if (!node_used(curr) && curr_node_size >= sz) {
            if (curr_node_size >= sz + sizeof(shmheap_node)) {
                shmheap_node *new_node = (shmheap_node *) (curr->data - sizeof(shmheap_node) + sz);
                new_node->sz = curr_node_size - sz;
                curr->sz = sz | (size_t)1;
            }
            else {
                curr->sz |= (size_t)1;
            }
            ret = (void *) curr->data;
            break;
        }
        curr = (shmheap_node *)(curr->data - sizeof(shmheap_node) + node_size(curr));
    }
    pthread_mutex_unlock(&mem->mutex);
    return ret;
}
    
void shmheap_free(shmheap_memory_handle mem, void *ptr) {
    pthread_mutex_lock(&mem->mutex);
    shmheap_node *end_node = (shmheap_node *) (((char *) mem) + mem->len);
    shmheap_node *curr = (shmheap_node *) mem->data;
    shmheap_node *prev = NULL;
    while (curr != end_node) {
        if (curr->data == (char *)ptr) {
            shmheap_node *newbase;
            size_t newsize;
            if (prev && !node_used(prev)) {
                newbase = prev;
                newsize = node_size(prev) + node_size(curr);
            }
            else {
                newbase = curr;
                newsize = node_size(curr);
            }
            shmheap_node *next = (shmheap_node *)(curr->data - sizeof(shmheap_node) + node_size(curr));
            if (next != end_node && !node_used(next)) {
                newsize += node_size(next);
            }
            newbase->sz = newsize;
            pthread_mutex_unlock(&mem->mutex);
            return;
        }
        prev = curr;
        curr = (shmheap_node *)(curr->data - sizeof(shmheap_node) + node_size(curr));
    }
    printf("Attempted to free an invalid pointer\n");
    pthread_mutex_unlock(&mem->mutex);
    abort();
}

shmheap_object_handle shmheap_ptr_to_handle(shmheap_memory_handle mem, void *ptr) {
    return (size_t)((char *)ptr - (char *)mem);
}
void *shmheap_handle_to_ptr(shmheap_memory_handle mem, shmheap_object_handle hdl) {
    return (void *)((char *)mem + hdl);
}
