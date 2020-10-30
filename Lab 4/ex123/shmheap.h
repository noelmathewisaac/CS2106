/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include <stddef.h>

/*
You should modify these structs to suit your implementation,
but remember that all the functions declared here must have
a signature that is callable using the APIs specified in the
lab document.

You may define other helper structs or convert the existing
structs to typedefs, as long as the functions satisfy the
requirements in the lab document.  If you declare additional names (helper structs or helper functions), they should be prefixed with "shmheap_" to avoid potential name clashes.
*/

typedef struct {} shmheap_memory_handle;
typedef struct {} shmheap_object_handle;


/*
These functions form the public API of your shmheap library.
*/

shmheap_memory_handle shmheap_create(const char *name, size_t len);
shmheap_memory_handle shmheap_connect(const char *name);
void shmheap_disconnect(shmheap_memory_handle mem);
void shmheap_destroy(const char *name, shmheap_memory_handle mem);
void *shmheap_underlying(shmheap_memory_handle mem);
void *shmheap_alloc(shmheap_memory_handle mem, size_t sz);
void shmheap_free(shmheap_memory_handle mem, void *ptr);
shmheap_object_handle shmheap_ptr_to_handle(shmheap_memory_handle mem, void *ptr);
void *shmheap_handle_to_ptr(shmheap_memory_handle mem, shmheap_object_handle hdl);
