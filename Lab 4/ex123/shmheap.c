/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include "shmheap.h"

shmheap_memory_handle shmheap_create(const char *name, size_t len) {
    /* TODO */
}

shmheap_memory_handle shmheap_connect(const char *name) {
    /* TODO */
}

void shmheap_disconnect(shmheap_memory_handle mem) {
    /* TODO */
}

void shmheap_destroy(const char *name, shmheap_memory_handle mem) {
    /* TODO */
}

void *shmheap_underlying(shmheap_memory_handle mem) {
    /* TODO */
}

void *shmheap_alloc(shmheap_memory_handle mem, size_t sz) {
    /* TODO */
}

void shmheap_free(shmheap_memory_handle mem, void *ptr) {
    /* TODO */
}

shmheap_object_handle shmheap_ptr_to_handle(shmheap_memory_handle mem, void *ptr) {
    /* TODO */
}

void *shmheap_handle_to_ptr(shmheap_memory_handle mem, shmheap_object_handle hdl) {
    /* TODO */
}
