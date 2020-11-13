#include "zc_io.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>

// The zc_file struct is analogous to the FILE struct that you get from fopen.
struct zc_file
{
  void *ptr;
  size_t size;
  int fd;
  size_t offset;
  // Insert the fields you need here.
};

/**************
 * Exercise 1 *
 **************/

zc_file *zc_open(const char *path)
{
  // To implement
  int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  struct stat buf;
  fstat(fd, &buf);
  off_t size = buf.st_size;

  //No error
  if (fd >= 0)
  {
    void *ptr = mmap(NULL, size,
                     PROT_READ | PROT_WRITE, MAP_SHARED,
                     fd, 0);
    zc_file *file = malloc(sizeof(zc_file));
    file->ptr = ptr;
    file->size = size;
    file->offset = 0;
    file->fd = fd;

    return file;
  }
  return NULL;
}

int zc_close(zc_file *file)
{
  // To implement
  int ret = close(file->fd);
  munmap(file->ptr, file->size);
  free(file);
  return ret;
}

const char *zc_read_start(zc_file *file, size_t *size)
{
  // To implement
  if (file->offset > file->size)
  {
    return NULL;
  }

  else if (file->offset + *size > file->size)

  {
    char *chunk = (char *)file->ptr + file->offset;
    *size = file->size - file->offset;
    file->offset += *size;
    return chunk;
  }
  else
  {
    char *chunk = (char *)file->ptr + file->offset;
    file->offset += *size;
    return chunk;
  }
}

void zc_read_end(zc_file *file)
{
  // To implement
}

/**************
 * Exercise 2 *
 **************/

char *zc_write_start(zc_file *file, size_t size)
{
  // To implement
  return NULL;
}

void zc_write_end(zc_file *file)
{
  // To implement
}

/**************
 * Exercise 3 *
 **************/

off_t zc_lseek(zc_file *file, long offset, int whence)
{
  // To implement
  return -1;
}

/**************
 * Exercise 5 *
 **************/

int zc_copyfile(const char *source, const char *dest)
{
  // To implement
  return -1;
}
