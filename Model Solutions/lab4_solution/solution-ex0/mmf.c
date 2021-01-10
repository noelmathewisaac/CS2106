/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "mmf.h"

void *mmf_create_or_open(const char *name, size_t sz) {
    int fd = open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ftruncate(fd, sz);
    void *ptr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return ptr;
}

void mmf_close(void *ptr, size_t sz) {
    munmap(ptr, sz);
}
