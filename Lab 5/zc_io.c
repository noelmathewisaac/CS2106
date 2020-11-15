#include "zc_io.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

// The zc_file struct is analogous to the FILE struct that you get from fopen.
struct zc_file
{
  void *ptr;
  size_t size;
  int fd;
  int readers;
  size_t offset;
  pthread_mutex_t r_mutex;
  pthread_mutex_t wr_mutex;
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
    void *ptr;
    //If file size is 0, call mmap with size 1 byte
    if (size == 0)
    {
      ptr = mmap(NULL, 1,
                 PROT_READ | PROT_WRITE, MAP_SHARED,
                 fd, 0);
      size = 1;
    }
    else
    {
      ptr = mmap(NULL, size,
                 PROT_READ | PROT_WRITE, MAP_SHARED,
                 fd, 0);
    }
    zc_file *file = malloc(sizeof(zc_file));
    file->ptr = ptr;
    file->size = size;
    file->offset = 0;
    file->fd = fd;
    file->readers = 0;
    pthread_mutex_init(&file->r_mutex, NULL);
    pthread_mutex_init(&file->wr_mutex, NULL);

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
  pthread_mutex_lock(&file->r_mutex);
  file->readers++;
  if (file->readers == 1)
  {
    pthread_mutex_lock(&file->wr_mutex);
  }
  pthread_mutex_unlock(&file->r_mutex);

  if (file->offset + *size > file->size)

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
  pthread_mutex_lock(&file->r_mutex);
  file->readers--;
  if (file->readers == 0)
  {
    pthread_mutex_unlock(&file->wr_mutex);
  }
  pthread_mutex_unlock(&file->r_mutex);
}

/**************
 * Exercise 2 *
 **************/

char *zc_write_start(zc_file *file, size_t size)
{
  // To implement
  pthread_mutex_lock(&file->wr_mutex);
  //If file is not big enough for write, make it bigger using ftruncate and perform mremap
  if (file->offset + size >= file->size)
  {
    ftruncate(file->fd, file->offset + size);
    file->ptr = mremap(file->ptr, file->size, file->offset + size, MREMAP_MAYMOVE);
    char *chunk = (char *)file->ptr + file->offset;
    file->offset += size;
    return chunk;
  }
  else
  {
    char *chunk = (char *)file->ptr + file->offset;
    file->offset += size;
    return chunk;
  }
}

void zc_write_end(zc_file *file)
{
  // To implement
  pthread_mutex_unlock(&file->wr_mutex);
}

/**************
 * Exercise 3 *
 **************/

off_t zc_lseek(zc_file *file, long offset, int whence)
{
  // To implement
  pthread_mutex_lock(&file->wr_mutex);

  //If resulting offset becomes negative at any point, an error (-1) should be returned
  if (whence == SEEK_SET)
  {
    if (offset < 0)
    {
      pthread_mutex_unlock(&file->wr_mutex);
      return -1;
    }
    file->offset = offset;
    pthread_mutex_unlock(&file->wr_mutex);
    return file->offset;
  }
  if (whence == SEEK_CUR)
  {

    if (file->offset + offset < 0)
    {
      pthread_mutex_unlock(&file->wr_mutex);
      return -1;
    }
    file->offset += offset;
    pthread_mutex_unlock(&file->wr_mutex);
    return file->offset;
  }
  if (whence == SEEK_END)
  {
    if (file->size + offset < 0)
    {
      pthread_mutex_unlock(&file->wr_mutex);
      return -1;
    }
    file->offset = file->size + offset;
    pthread_mutex_unlock(&file->wr_mutex);
    return file->offset;
  }
  pthread_mutex_unlock(&file->wr_mutex);
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
