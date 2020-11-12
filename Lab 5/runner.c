#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "zc_io.h"

#define eprintf(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#define RUNNER_ERROR(msg, ...) eprintf("RUNNER ERROR: " msg, ##__VA_ARGS__)
#define FAIL(msg, ...) eprintf("FAIL: " msg, ##__VA_ARGS__)

struct thread_data {
  zc_file *file;
  size_t op_size;
};

static pthread_barrier_t barrier;
static volatile _Atomic int thread_counter;

static char path1[32], path2[32], path3[32];

// memory is free nowadays, right ¯\_(ツ)_/¯
static char randdata[5 * 1024 * 1024];
static char scratch[5 * 1024 * 1024];

static void unlink_paths(void) {
  unlink(path1);
  unlink(path2);
  unlink(path3);
}

#define FAIL_IF(cond, msg, ...)                                                                    \
  do {                                                                                             \
    if (cond) {                                                                                    \
      FAIL("thread " msg, ##__VA_ARGS__);                                                          \
      unlink_paths();                                                                              \
      _Exit(1);                                                                                    \
    }                                                                                              \
  } while (0)

static void *threadfn(void *datav) {
  struct thread_data *data = (struct thread_data *)datav;
  zc_file *zcfile = data->file;
  const size_t op_size = data->op_size;
  size_t real_read_size = op_size;
  free(datav);

  pthread_barrier_wait(&barrier);
  const char *read = zc_read_start(zcfile, &real_read_size);
  FAIL_IF(real_read_size != op_size, "zc_read returned wrong size - expected %zu, got %zu\n",
          op_size, real_read_size);
  FAIL_IF(memcmp(read, randdata + op_size * 3, op_size), "zc_read returned wrong contents\n");
  thread_counter = 1;
  zc_read_end(zcfile);
  pthread_barrier_wait(&barrier);

  pthread_barrier_wait(&barrier);
  read = zc_write_start(zcfile, op_size);
  thread_counter = 1;
  zc_write_end(zcfile);
  pthread_barrier_wait(&barrier);

  pthread_barrier_wait(&barrier);
  read = zc_write_start(zcfile, op_size);
  thread_counter = 1;
  zc_write_end(zcfile);
  pthread_barrier_wait(&barrier);

  return NULL;
}

#undef FAIL_IF

static ssize_t fstat_size(int fd) {
  struct stat stat = {0};
  if (fstat(fd, &stat)) {
    return -1;
  }
  return stat.st_size;
}

static void catch_signal(int signum) {
  (void)signum;
  // this is totally not safe but who cares, we're already going to die
  FAIL("caught %s, exiting\n", strsignal(signum));
  _Exit(1);
}

static void fill_buffer(void *const buf, size_t bytes) {
  int32_t *cur = buf, *const end = (int32_t *)buf + bytes / sizeof(int32_t);
  for (; cur < end; ++cur) {
    *cur = (int32_t)lrand48();
  }
}

#define RUNNER_ERROR_IF(cond, msg, ...)                                                            \
  do {                                                                                             \
    if (cond) {                                                                                    \
      RUNNER_ERROR(msg, ##__VA_ARGS__);                                                            \
      goto exit;                                                                                   \
    }                                                                                              \
  } while (0)

#define FAIL_IF(cond, msg, ...)                                                                    \
  do {                                                                                             \
    if (cond) {                                                                                    \
      FAIL(msg, ##__VA_ARGS__);                                                                    \
      goto exit;                                                                                   \
    }                                                                                              \
  } while (0)

#define GEN_SIZE() ((size_t)(1024 + lrand48() % 1048576))

#define TRUNCATE_FILE(file, size)                                                                  \
  do {                                                                                             \
    rewind(file);                                                                                  \
    RUNNER_ERROR_IF(ftruncate(fileno(file), size), "ftruncate failed: %s\n", strerror(errno));     \
  } while (0)

#define FILL_FILE(file, size)                                                                      \
  do {                                                                                             \
    rewind(file);                                                                                  \
    TRUNCATE_FILE(file, size);                                                                     \
    RUNNER_ERROR_IF(fwrite(randdata, 1, size, file) != size, "fwrite failed\n");                   \
    fflush(file);                                                                                  \
    rewind(file);                                                                                  \
  } while (0)

int main(int argc, char *argv[]) {
  int retv = 1;
  long seed;
  if (argc >= 2) {
    seed = atol(argv[1]);
  } else {
    struct timespec t = {0};
    clock_gettime(CLOCK_REALTIME, &t);
    seed = t.tv_nsec ^ t.tv_sec;
  }
  eprintf("using seed %ld\n\n", seed);
  srand48(seed);

  fill_buffer(randdata, sizeof(randdata));

  {
    struct sigaction sigaction = {.sa_handler = catch_signal};
    sigfillset(&sigaction.sa_mask);
    signal(SIGSEGV, catch_signal);
    signal(SIGBUS, catch_signal);
    signal(SIGABRT, catch_signal);
    signal(SIGTERM, catch_signal);
  }

  pthread_t thread = pthread_self();
  FILE *file1 = NULL, *file2 = NULL, *file3 = NULL;

  RUNNER_ERROR_IF(pthread_barrier_init(&barrier, NULL, 2), "failed to init barrier\n");

  // generate new paths, with the PID so it *should* be unique
  // not using mkstemp because for file2 we want to have the library create it
  // and then we might as well handle it ourselves for file1/file3
  RUNNER_ERROR_IF(snprintf(path1, sizeof(path1), "/tmp/zc_io1_%d_%ld", getpid(), lrand48()) < 0,
                  "snprintf failed\n");
  RUNNER_ERROR_IF(snprintf(path2, sizeof(path2), "/tmp/zc_io2_%d_%ld", getpid(), lrand48()) < 0,
                  "snprintf failed\n");
  // file3 in the current directory (we can't use tmpfs if we want to observe caching)
  RUNNER_ERROR_IF(snprintf(path3, sizeof(path3), "zc_io3_%d_%ld", getpid(), lrand48()) < 0,
                  "snprintf failed\n");

  unlink(path1);
  unlink(path2);
  unlink(path3);

  file1 = fopen(path1, "w+");
  RUNNER_ERROR_IF(!file1, "failed to create %s\n", path1);

  setvbuf(file1, NULL, _IONBF, 0);

  eprintf("test 1 - open/read\n");
  {
    const size_t size = GEN_SIZE();
    FILL_FILE(file1, size);
    eprintf("filling file with %zu bytes\n", size);

    zc_file *zcfile = zc_open(path1);
    eprintf("opening %s\n", path1);
    FAIL_IF(!zcfile, "zc_open %s failed\n", path1);

    size_t offset = 0;
    for (int i = 0; i < 5; ++i) {
      const size_t read_size = i == 4 ? size : (size_t)(lrand48() % ((size >> 2) - 16));
      const size_t expected_read_size = i == 4 ? size - offset : read_size;

      eprintf("reading %zu from expected offset %zu\n", read_size, offset);
      size_t real_read_size = read_size;
      const char *read_ptr = zc_read_start(zcfile, &real_read_size);
      FAIL_IF(real_read_size != expected_read_size,
              "zc_read returned wrong size - expected %zu, got %zu\n", expected_read_size,
              real_read_size);
      FAIL_IF(!read_ptr, "zc_read failed - returned NULL\n");
      FAIL_IF(memcmp(randdata + offset, read_ptr, expected_read_size),
              "zc_read returned wrong contents\n");
      zc_read_end(zcfile);
      offset += read_size;
    }

    zc_close(zcfile);
  }
  eprintf("test 1 passed\n\n");

  eprintf("test 2a - write to existing file, overwriting existing contents\n");
  {
    const size_t size = GEN_SIZE();
    FILL_FILE(file1, size);
    eprintf("filling file with %zu bytes\n", size);

    zc_file *zcfile = zc_open(path1);
    eprintf("opening %s\n", path1);
    FAIL_IF(!zcfile, "zc_open %s failed\n", path1);

    size_t offset = 0;
    for (int i = 0; i < 5; ++i) {
      // for 5th iteration, extend the file
      const size_t write_size =
          i == 4 ? (size - offset + 1024) : (size_t)(lrand48() % ((size >> 2) - 16));
      eprintf("writing %zu to expected offset %zu\n", write_size, offset);

      char *write_ptr = zc_write_start(zcfile, write_size);
      FAIL_IF(!write_ptr, "zc_write_start failed - returned NULL\n");
      memcpy(write_ptr, randdata + 1048576 + offset, write_size);
      zc_write_end(zcfile);

      offset += write_size;

      rewind(file1);
      const size_t expected_file_size = offset > size ? offset : size;
      FAIL_IF(fread(scratch, 1, sizeof(scratch), file1) != expected_file_size,
              "fread returned wrong size - did zc_write cause file to have wrong length?\n");
      // file1[0:offset) == randdata[1048576:1048576+offset)
      // file1[offset:expected_file_size) == randdata[offset:expected_file_size)
      FAIL_IF(memcmp(scratch, randdata + 1048576, offset) ||
                  memcmp(scratch + offset, randdata + offset, expected_file_size - offset),
              "zc_write failed - wrong contents seen in file after zc_write_end\n");
    }

    zc_close(zcfile);
  }
  eprintf("test 2a passed\n\n");

  eprintf("test 2b - write to new file\n");
  {
    unlink(path2);
    const size_t size = GEN_SIZE();

    zc_file *zcfile = zc_open(path2);
    eprintf("opening new file %s\n", path2);
    FAIL_IF(!zcfile, "zc_open %s failed\n", path2);

    FILE *file2 = fopen(path2, "r+");
    FAIL_IF(!file2, "could not open %s, did zc_open forget to pass an appropriate mode to open?\n",
            path2);
    setvbuf(file2, NULL, _IONBF, 0);

    size_t offset = 0;
    for (int i = 0; i < 4; ++i) {
      const size_t write_size = (size_t)(lrand48() % ((size >> 2) - 16));
      eprintf("writing %zu to expected offset %zu\n", write_size, offset);

      char *write_ptr = zc_write_start(zcfile, write_size);
      FAIL_IF(!write_ptr, "zc_write_start failed - returned NULL\n");
      memcpy(write_ptr, randdata + offset, write_size);
      zc_write_end(zcfile);

      offset += write_size;

      rewind(file2);
      FAIL_IF(fread(scratch, 1, sizeof(scratch), file2) != offset,
              "fread returned wrong size - did zc_write cause file to have wrong length?\n");
      FAIL_IF(memcmp(scratch, randdata, offset),
              "zc_write failed - wrong contents seen in file after zc_write_end\n");
    }

    zc_close(zcfile);
    fclose(file2);
    file2 = NULL;
    unlink(path2);
  }
  eprintf("test 2b passed\n\n");

#define TEST3_READ(_____sz)                                                                        \
  do {                                                                                             \
    const size_t read_size = _____sz;                                                              \
    size_t real_read_size = read_size;                                                             \
    eprintf("reading %zu from expected offset %zu\n", read_size, offset);                          \
    const char *read_ptr = zc_read_start(zcfile, &real_read_size);                                 \
    FAIL_IF(real_read_size != read_size, "zc_read returned wrong size - expected %zu, got %zu\n",  \
            read_size, real_read_size);                                                            \
    FAIL_IF(!read_ptr, "zc_read failed - returned NULL\n");                                        \
    FAIL_IF(memcmp(file_mirror + offset, read_ptr, read_size),                                     \
            "zc_read returned wrong contents\n");                                                  \
    zc_read_end(zcfile);                                                                           \
    offset += read_size;                                                                           \
  } while (0)

#define TEST3_WRITE(_____sz)                                                                       \
  do {                                                                                             \
    const size_t write_size = _____sz;                                                             \
    eprintf("writing %zu to expected offset %zu\n", write_size, offset);                           \
    char *write_ptr = zc_write_start(zcfile, write_size);                                          \
    FAIL_IF(!write_ptr, "zc_write_start failed - returned NULL\n");                                \
    memcpy(write_ptr, randdata + 1048576 + offset, write_size);                                    \
    zc_write_end(zcfile);                                                                          \
    memcpy(file_mirror + offset, randdata + 1048576 + offset, write_size);                         \
    offset += write_size;                                                                          \
                                                                                                   \
    rewind(file1);                                                                                 \
    FAIL_IF(fread(scratch, 1, sizeof(scratch), file1) != size,                                     \
            "fread returned wrong size - did zc_write cause file to have wrong length?\n");        \
    FAIL_IF(memcmp(scratch, file_mirror, size),                                                    \
            "zc_write failed - wrong contents seen in file after zc_write_end\n");                 \
  } while (0)

#define TEST3_SEEK(f, o, s)                                                                        \
  do {                                                                                             \
    const off_t seek_result = zc_lseek(f, o, s);                                                   \
    FAIL_IF(seek_result == -1, "zc_lseek returned -1\n");                                          \
    FAIL_IF((size_t)seek_result != offset,                                                         \
            "zc_lseek returned wrong offset - expected %zu, got %zu\n", offset,                    \
            (size_t)seek_result);                                                                  \
  } while (0)

  eprintf("test 3 - seek\n");
  {
    const size_t size = GEN_SIZE();
    char *file_mirror = malloc(size);
    RUNNER_ERROR_IF(!file_mirror, "malloc failed\n");
    memcpy(file_mirror, randdata, size);
    FILL_FILE(file1, size);
    eprintf("filling file with %zu bytes\n", size);

    zc_file *zcfile = zc_open(path1);
    eprintf("opening %s\n", path1);
    FAIL_IF(!zcfile, "zc_open %s failed\n", path1);

    size_t offset = lrand48() % (size >> 1);

    eprintf("seeking SEEK_SET to offset %zu\n", offset);
    TEST3_SEEK(zcfile, offset, SEEK_SET);

    TEST3_READ((size_t)(lrand48() % ((size >> 3) - 128)));
    TEST3_WRITE((size_t)(lrand48() % ((size >> 3) - 128)));

    eprintf("seeking SEEK_CUR by 117\n");
    offset += 117;
    TEST3_SEEK(zcfile, 117, SEEK_CUR);

    TEST3_READ(51);
    TEST3_WRITE(95);

    eprintf("seeking SEEK_CUR by -77\n");
    offset -= 77;
    TEST3_SEEK(zcfile, -77, SEEK_CUR);

    TEST3_READ(12);
    TEST3_WRITE(12);

    eprintf("seeking SEEK_END by -515\n");
    offset = fstat_size(fileno(file1)) - 515;
    TEST3_SEEK(zcfile, -515, SEEK_END);

    TEST3_READ(231);
    TEST3_WRITE(61);

    // if we failed above, just forget it..
    free(file_mirror);
    zc_close(zcfile);
  }
  eprintf("test 3 passed\n\n");

#define TEST4_WAITUNTIL(cond, ms)                                                                  \
  do {                                                                                             \
    counter = 0;                                                                                   \
    while (counter < (ms) && !(cond)) {                                                            \
      nanosleep(&(struct timespec){.tv_nsec = 1000000}, NULL);                                     \
      ++counter;                                                                                   \
    }                                                                                              \
  } while (0)

  eprintf("test 4 - synchronisation\n"
          "*** NOTE: if runner hangs for more than a few seconds,\n"
          "*** you probably have a synchronisation problem\n");
  {
    size_t counter = 0;
    const size_t size = GEN_SIZE();
    FILL_FILE(file1, size);
    eprintf("filling file with %zu bytes\n", size);

    zc_file *zcfile = zc_open(path1);
    eprintf("opening %s\n", path1);
    FAIL_IF(!zcfile, "zc_open %s failed\n", path1);

    // acquire simultaneous reads
    const size_t op_size = (size_t)(lrand48() % ((size >> 3) - 16));
    size_t real_read_size = op_size;
    eprintf("acquiring 2 reads of %zu from the same thread\n", op_size);
    const char *read1 = zc_read_start(zcfile, &real_read_size);
    FAIL_IF(real_read_size != op_size, "zc_read returned wrong size - expected %zu, got %zu\n",
            op_size, real_read_size);
    const char *read2 = zc_read_start(zcfile, &real_read_size);
    FAIL_IF(real_read_size != op_size, "zc_read returned wrong size - expected %zu, got %zu\n",
            op_size, real_read_size);
    FAIL_IF(memcmp(read1, randdata, op_size), "zc_read returned wrong contents\n");
    FAIL_IF(memcmp(read2, randdata + op_size, op_size), "zc_read returned wrong contents\n");
    zc_read_end(zcfile);
    zc_read_end(zcfile);

    {
      struct thread_data *thread_data = malloc(sizeof(struct thread_data));
      FAIL_IF(!thread_data, "malloc failed\n");
      *thread_data = (struct thread_data){.file = zcfile, .op_size = op_size};
      pthread_create(&thread, NULL, threadfn,
                     // let the thread free it
                     thread_data);
    }

    eprintf("acquiring 2 reads of %zu from separate threads\n", op_size);
    thread_counter = 0;
    // we acquire first
    read1 = zc_read_start(zcfile, &real_read_size);
    FAIL_IF(real_read_size != op_size, "zc_read returned wrong size - expected %zu, got %zu\n",
            op_size, real_read_size);
    FAIL_IF(memcmp(read1, randdata + op_size * 2, op_size), "zc_read returned wrong contents\n");
    // barrier 1, then they acquire
    pthread_barrier_wait(&barrier);
    // wait ~1s for them to indicate that they have acquired
    TEST4_WAITUNTIL(thread_counter > 0, 1000);
    FAIL_IF(!thread_counter, "thread could not acquire read within 1s\n");
    // release
    zc_read_end(zcfile);
    // barrier 2
    pthread_barrier_wait(&barrier);

    eprintf("acquiring read and write of %zu from separate threads\n", op_size);
    thread_counter = 0;
    // we acquire first
    read1 = zc_read_start(zcfile, &real_read_size);
    FAIL_IF(real_read_size != op_size, "zc_read returned wrong size - expected %zu, got %zu\n",
            op_size, real_read_size);
    FAIL_IF(memcmp(read1, randdata + op_size * 4, op_size), "zc_read returned wrong contents\n");
    // barrier 3, then they acquire
    pthread_barrier_wait(&barrier);
    // wait 500ms, be reasonably sure they are blocked
    TEST4_WAITUNTIL(thread_counter > 0, 500);
    FAIL_IF(thread_counter, "worker thread acquired write while we are reading!\n");
    // release the read
    zc_read_end(zcfile);
    // now wait until they get the write
    TEST4_WAITUNTIL(thread_counter > 0, 1000);
    FAIL_IF(!thread_counter, "thread could not acquire write within 1s\n");
    // barrier 4
    pthread_barrier_wait(&barrier);

    eprintf("acquiring write of %zu from separate threads\n", op_size);
    thread_counter = 0;
    read1 = zc_write_start(zcfile, op_size);
    // barrier 5, then they acquire
    pthread_barrier_wait(&barrier);
    // wait 500ms, be reasonably sure they are blocked
    TEST4_WAITUNTIL(thread_counter > 0, 500);
    FAIL_IF(thread_counter, "worker thread acquired write while we are writing!\n");
    // release the read
    zc_write_end(zcfile);
    // now wait until they get the write
    TEST4_WAITUNTIL(thread_counter > 0, 1000);
    FAIL_IF(!thread_counter, "thread could not acquire write within 1s\n");
    // barrier 6
    pthread_barrier_wait(&barrier);

    pthread_join(thread, NULL);
    thread = pthread_self();

    zc_close(zcfile);
  }
  eprintf("test 4 passed\n\n");

  eprintf("test 5 - copy\n");
  {
    const size_t size = GEN_SIZE();
    FILL_FILE(file1, size);
    eprintf("filling %s with %zu bytes\n", path1, size);

    unlink(path2);
    eprintf("copying %s (file1) to %s (file2)\n", path1, path2);
    FAIL_IF(zc_copyfile(path1, path2), "zc_copyfile returned non-zero\n");
    FILE *file2 = fopen(path2, "r");
    FAIL_IF(!file2,
            "could not open %s, did zc_copyfile forget to pass an appropriate mode to open?\n",
            path2);
    FAIL_IF(fread(scratch, 1, size, file2) != size, "fread returned wrong size for file2\n");
    fclose(file2);
    file2 = NULL;
    FAIL_IF(memcmp(scratch, randdata, size), "wrong contents in file2\n");
  }
  eprintf("test 5 passed\n");

  retv = 0;

exit:
  if (file1) {
    fclose(file1);
  }
  if (file2) {
    fclose(file2);
  }
  if (file3) {
    fclose(file3);
  }
  unlink_paths();

  if (!retv) {
    return 0;
  }
  _Exit(retv);
}
