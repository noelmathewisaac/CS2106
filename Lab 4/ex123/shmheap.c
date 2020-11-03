/*************************************
* Lab 4
* Name: Noel Mathew Isaac
* Student No: A0202072Y
* Lab Group: 3
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
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0777);
    ftruncate(shm_fd, len);
    void *ptr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shmheap_memory_handle handle = malloc(sizeof(shmheap_memory_handle));
    handle->ptr = ptr;
    handle->sz = len;
    shmheap_node *node = malloc(sizeof(shmheap_node));
    node->sz = len - sizeof(handle);
    node->ptr = handle->ptr + sizeof(handle);
    handle->next = node;
    return handle;
}

shmheap_memory_handle shmheap_connect(const char *name)
{
    /* TODO */
    int shm_fd = shm_open(name, O_RDWR, 0777);
    struct stat buf;
    fstat(shm_fd, &buf);
    off_t size = buf.st_size;
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shmheap_memory_handle handle = malloc(sizeof(shmheap_memory_handle));
    handle->ptr = ptr;
    handle->sz = size;
    shmheap_node *node = malloc(sizeof(shmheap_node));
    node->sz = size - sizeof(handle);
    node->ptr = handle->ptr + sizeof(handle);
    handle->next = node;
    return handle;
}

void shmheap_disconnect(shmheap_memory_handle mem)
{
    /* TODO */
    munmap(mem->ptr, mem->sz);
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
    return (void *)mem->ptr;
}

void *shmheap_alloc(shmheap_memory_handle mem, size_t sz)
{
    /* TODO */

    size_t rounded_size = ((sz + 7) & (-8)); //rounding to next higher mutiple of 8
    shmheap_node *curr = mem->next;
    while (curr != NULL)
    {
        if (curr->sz >= rounded_size && (curr->sz & 1) == 0)
        {
            shmheap_node *next = malloc(sizeof(shmheap_node));
            next->sz = (curr->sz & -2) - rounded_size;
            next->ptr = curr->ptr + rounded_size + sizeof(curr);
            curr->sz = rounded_size | 1; //Last bit ORed with 1 to indicate that its allocated.
            curr->next = next;
            return curr->ptr;
        }
        curr = curr->next;
    }
}

void shmheap_free(shmheap_memory_handle mem, void *ptr)
{
    /* TODO */
    shmheap_node *curr = mem->next;
    shmheap_node *prev;

    while (curr != NULL)
    {
        if (curr->ptr == (char *)ptr)
        {
            curr->sz = (curr->sz & -2); // Set the last bit to 0 to indicate free node
            if ((curr->next->sz & 1) == 0)
            {
                curr->sz = curr->sz + curr->next->sz;
                curr->next = curr->next->next;
            }
            if (prev != NULL)
            {
                if ((prev->sz & 1) == 0)
                {
                    {
                        prev->sz = prev->sz + curr->sz;
                        prev->next = curr->next;
                    }
                }
            }
            break;
        }
        prev = curr;
        curr = curr->next;
    }
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
