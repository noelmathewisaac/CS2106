/**
 * CS2106 AY 20/21 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sm.h"

struct sm_service {
  sm_status_t status;
  size_t num_processes;
  pid_t *pids;
};

static size_t num_services = 0;
static struct sm_service services[SM_MAX_SERVICES];

static void update_statuses(void);
static void update_service_pids(struct sm_service *service);
static char *make_logfile_name(size_t service_index);
static void sm_start_impl(const char *processes[], bool log);

static int check_posix(int r, const char *msg) {
  if (r == -1) {
    perror(msg);
  }
  return r;
}

// Use this function to any initialisation if you need to.
void sm_init(void) {}

// Use this function to do any cleanup of resources.
void sm_free(void) {
  for (size_t i = 0; i < num_services; ++i) {
    if (services[i].pids) {
      free(services[i].pids);
    }
    if (services[i].status.path) {
      free((void *)services[i].status.path);
    }
  }
}

// Exercise 1a/2: start services
void sm_start(const char *processes[]) { sm_start_impl(processes, false); }

// Exercise 1b: print service status
size_t sm_status(sm_status_t statuses[]) {
  update_statuses();
  for (size_t i = 0; i < num_services; ++i) {
    statuses[i] = services[i].status;
  }
  return num_services;
}

// Exercise 3: stop service, wait on service, and shutdown
void sm_stop(size_t service_index) {
  // not in requirements, but for completeness' sake
  if (service_index >= num_services) {
    printf("Invalid service\n");
    return;
  }

  update_service_pids(services + service_index);
  for (size_t i = 0; i < services[service_index].num_processes; ++i) {
    const pid_t pid = services[service_index].pids[i];
    if (pid < 1) {
      continue;
    }

    check_posix(kill(pid, SIGTERM), "sm_stop: kill");
  }
  sm_wait(service_index);
}

void sm_wait(size_t service_index) {
  if (service_index >= num_services) {
    printf("Invalid service\n");
    return;
  }

  for (size_t i = 0; i < services[service_index].num_processes; ++i) {
    const pid_t pid = services[service_index].pids[i];
    if (pid < 1) {
      continue;
    }

    waitpid(pid, NULL, 0);
    services[service_index].pids[i] = 0;
  }
}

void sm_shutdown(void) {
  for (size_t i = 0; i < num_services; ++i) {
    sm_stop(i);
  }
}

// Exercise 4: start with output redirection
void sm_startlog(const char *processes[]) { sm_start_impl(processes, true); }

// Exercise 5: show log file
void sm_showlog(size_t service_index) {
  if (service_index >= num_services) {
    printf("Invalid service\n");
    return;
  }

  pid_t pid = check_posix(fork(), "sm_showlog: fork");
  if (pid == 0) {
    char *logfile_name = make_logfile_name(service_index);
    if (!logfile_name) {
      _exit(1);
    }

    if (access(logfile_name, F_OK) == -1) {
      printf("service has no log file\n");
      _exit(1);
    }

    execl("/bin/cat", "/bin/cat", logfile_name, (void *)NULL);
    _exit(1);
  }
  check_posix(waitpid(pid, NULL, 0), "sm_showlog: waitpid");
}

static void update_statuses(void) {
  for (size_t i = 0; i < num_services; ++i) {
    sm_status_t *status = &services[i].status;
    if (status->running) {
      pid_t r = waitpid(status->pid, NULL, WNOHANG);
      if (r == status->pid || (r == -1 && errno == ECHILD)) {
        status->running = false;
      }
    }
  }
}

static void update_service_pids(struct sm_service *service) {
  for (size_t i = 0; i < service->num_processes; ++i) {
    if (service->pids[i] < 1) {
      continue;
    }
    const pid_t r = waitpid(service->pids[i], NULL, WNOHANG);
    if (r == service->pids[i] || (r == -1 && errno == ECHILD)) {
      service->pids[i] = 0;
    }
  }
}

static char *make_logfile_name(size_t service_index) {
  char *ret;
  if (asprintf(&ret, "service%zu.log", service_index) < 0) {
    fprintf(stderr, "asprintf: Failed\n");
    return NULL;
  }
  return ret;
}

static void sm_start_impl(const char *processes[], bool log) {
  assert(num_services < SM_MAX_SERVICES);
  const char **cursor = processes;
  struct sm_service *service = services + num_services;
  while (*cursor) {
    while (*(++cursor)) {
      // advance to NULL
    }
    ++service->num_processes;
    ++cursor;
  }

  const size_t num_pipes = service->num_processes - 1;
  int *pipes = calloc(2 * num_pipes, sizeof(int));
  service->pids = calloc(service->num_processes, sizeof(pid_t));
  if (!pipes || !service->pids) {
    perror("sm_start: calloc");
    goto fail;
  }

  for (size_t i = 0; i < num_pipes; ++i) {
    if (check_posix(pipe2(pipes + 2 * i, O_CLOEXEC), "sm_start: pipe2")) {
      goto fail;
    }
  }

  cursor = processes;
  size_t num_execed = 0;
  while (*cursor) {
    assert(num_execed < service->num_processes);
    pid_t fpid = check_posix(fork(), "sm_start: fork");
    if (fpid == 0) {
      if (num_execed == 0) {
        close(STDIN_FILENO);
      }

      if (num_pipes > 0) {
        if (num_execed + 1 < service->num_processes) {
          check_posix(dup2(pipes[num_execed * 2 + 1], STDOUT_FILENO), "sm_start: dup2");
        }

        if (num_execed > 0) {
          check_posix(dup2(pipes[(num_execed - 1) * 2], STDIN_FILENO), "sm_start: dup2");
        }
      }

      if (log && num_execed + 1 == service->num_processes) {
        char *logfile_name = make_logfile_name(num_services);
        if (logfile_name) {
          int logfile = check_posix(open(logfile_name, O_WRONLY | O_CREAT | O_APPEND, 0666),
                                    "sm_start: open");
          if (logfile > -1) {
            check_posix(dup2(logfile, STDOUT_FILENO), "sm_start: dup2");
            close(logfile);
          }
          free(logfile_name);
        }
      }

      // n.b. cast is OK because execv is guaranteed under POSIX to not modify
      // *cursor nor **cursor
      check_posix(execv(cursor[0], (char **)cursor), "sm_start: execv");
      _exit(1);
    }

    service->pids[num_execed] = fpid;
    if (num_execed + 1 == service->num_processes) {
      service->status = (sm_status_t){.path = strdup(*cursor), .pid = fpid, .running = fpid > 0};
    }
    ++num_execed;
    while (*(++cursor)) {
      // advance to NULL
    }
    ++cursor;
  }

  ++num_services;
  goto cleanup;
fail:
  if (service->pids) {
    free(service->pids);
  }
cleanup:
  if (pipes) {
    for (size_t i = 0; i < num_pipes * 2; ++i) {
      if (pipes[i]) {
        close(pipes[i]);
      }
    }
    free(pipes);
  }
}
