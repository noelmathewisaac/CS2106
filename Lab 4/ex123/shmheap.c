/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include "shmheap.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>

shmheap_memory_handle shmheap_create(const char *name, size_t len)
{
    /* TODO */
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, len);
    void *ptr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shmheap_memory_handle handle = malloc(sizeof(shmheap_memory_handle));
    handle->data[0] = ptr;
    handle->sz = len;
    return handle;
}

shmheap_memory_handle shmheap_connect(const char *name)
{
    /* TODO */
    int shm_fd = shm_open(name, O_RDWR, 0666);
    struct stat buf;
    fstat(shm_fd, &buf);
    off_t size = buf.st_size;
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shmheap_memory_handle handle = malloc(sizeof(shmheap_memory_handle));
    handle->data[0] = ptr;
    handle->sz = size;
    return handle;
}

void shmheap_disconnect(shmheap_memory_handle mem)
{
    /* TODO */
    munmap(mem->data, mem->sz);
}

void shmheap_destroy(const char *name, shmheap_memory_handle mem)
{
    /* TODO */
    shmheap_disconnect(mem);
    shm_unlink(name);
}

void *shmheap_underlying(shmheap_memory_handle mem)
{
    /* TODO */
}

void *shmheap_alloc(shmheap_memory_handle mem, size_t sz)
{
    /* TODO */
    printf("lol");
    size_t rounded_size = ((sz + 7) & (-8));
    shmheap_node *node = malloc(sizeof(shmheap_node));
    node->data[0] = mem->data[0] + rounded_size;
    node->sz = rounded_size;
    mem->data[0] = node;
    return node;
}

void shmheap_free(shmheap_memory_handle mem, void *ptr)
{
    /* TODO */
}

shmheap_object_handle shmheap_ptr_to_handle(shmheap_memory_handle mem, void *ptr)
{
    /* TODO */
    return (size_t)((char *)ptr - (char *)mem);
}

void *shmheap_handle_to_ptr(shmheap_memory_handle mem, shmheap_object_handle hdl)
{
    /* TODO */
    return (void *)((char *)mem + hdl);
}
