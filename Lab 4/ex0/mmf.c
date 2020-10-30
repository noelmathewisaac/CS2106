/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include "mmf.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>

void *mmf_create_or_open(const char *name, size_t sz)
{
    /* TODO */
    struct stat buf;
    void *ptr;

    int fd = open(name, O_RDWR, S_IRUSR | S_IWUSR);

    //if file doesnt exist create a file of size sz
    if (fd < 0)
    {
        fd = open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        ftruncate(fd, sz);
        ptr = mmap(NULL, sz,
                   PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
    }

    //if file exists find the size of the file
    else
    {
        fstat(fd, &buf);
        off_t size = buf.st_size;

        //map to memory
        ptr = mmap(NULL, size,
                   PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
    }
    close(fd);
    return ptr;
}

void mmf_close(void *ptr, size_t sz)
{
    /* TODO */
    munmap(ptr, sz);
}
