#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sm.h"

static void process_commands(FILE *file);
static bool handle_command(const size_t num_tokens, char ***tokensp);
static void transform_tokens_for_start(const size_t num_tokens, char ***tokens);
static size_t tokenise(char *const line, char ***tokens);

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  sm_init();
  process_commands(stdin);
  sm_free();
  return 0;
}

static void print_prompt(void) {
  printf("sm> ");
  fflush(stdout);
}

static void process_commands(FILE *file) {
  bool exiting = false;
  char *line = NULL;
  size_t line_size = 0;
  print_prompt();
  while (!exiting) {
    if (getline(&line, &line_size, file) == -1) {
      if (feof(file)) {
        printf("End of commands; shutting down\n");
      } else {
        perror("Error while reading command; shutting down\n");
      }
      sm_shutdown();
      break;
    }
    char **tokens = NULL;
    size_t num_tokens = tokenise(line, &tokens);
    if (!tokens) {
      printf("Failed to tokenise command\n");
      exit(1);
    }

    exiting = handle_command(num_tokens, &tokens);
    free(tokens);

    if (!exiting) {
      print_prompt();
    }
  }

  if (line) {
    free(line);
  }

  if (ferror(file)) {
    perror("Failed to read line");
    exit(1);
  }
}

#define CHECK_ARGC(nargs)                                                                          \
  do {                                                                                             \
    if (num_tokens < nargs) {                                                                      \
      printf("Insufficient arguments for %s\n", cmd);                                              \
      return false;                                                                                \
    }                                                                                              \
  } while (0)

#define SCAN_SERVICE_NUMBER(into)                                                                  \
  do {                                                                                             \
    if (sscanf((*tokensp)[1], "%zu", &into) != 1) {                                                \
      printf("Invalid service number %s\n", (*tokensp)[1]);                                        \
      return false;                                                                                \
    }                                                                                              \
  } while (0)

static bool handle_command(const size_t num_tokens, char ***tokensp) {
  const char *const cmd = (*tokensp)[0];
  if (!cmd) {
    // no-op
  } else if (strcmp(cmd, "start") == 0) {
    CHECK_ARGC(2);
    transform_tokens_for_start(num_tokens, tokensp);
    sm_start((const char **)(*tokensp) + 1);
  } else if (strcmp(cmd, "startlog") == 0) {
    CHECK_ARGC(2);
    transform_tokens_for_start(num_tokens, tokensp);
    sm_startlog((const char **)(*tokensp) + 1);
  } else if (strcmp(cmd, "wait") == 0) {
    CHECK_ARGC(2);
    size_t service_number;
    SCAN_SERVICE_NUMBER(service_number);
    sm_wait(service_number);
  } else if (strcmp(cmd, "stop") == 0) {
    CHECK_ARGC(2);
    size_t service_number;
    SCAN_SERVICE_NUMBER(service_number);
    sm_stop(service_number);
  } else if (strcmp(cmd, "status") == 0) {
    sm_status_t statuses[SM_MAX_SERVICES] = {0};
    size_t num_services = sm_status(statuses);
    for (size_t i = 0; i < num_services; ++i) {
      sm_status_t *status = statuses + i;
      printf("%zu. %s (PID %ld): %s\n", i, status->path, (long)status->pid,
             status->running ? "Running" : "Exited");
    }
  } else if (strcmp(cmd, "showlog") == 0) {
    CHECK_ARGC(2);
    size_t service_number;
    SCAN_SERVICE_NUMBER(service_number);
    sm_showlog(service_number);
  } else if (strcmp(cmd, "shutdown") == 0) {
    sm_shutdown();
    return true;
  } else {
    printf("Unknown command %s\n", cmd);
  }

  return false;
}

static void transform_tokens_for_start(const size_t num_tokens, char ***tokens) {
  char **cursor = (*tokens) + 1;
  while (*cursor) {
    // if this is a pipe, then end this subarray
    if (strcmp(*cursor, "|") == 0) {
      *cursor = NULL;
    }
    ++cursor;
  }

  // resize the tokens array to add an additional NULL at the end
  *tokens = realloc(*tokens, (num_tokens + 2) * sizeof(char *));
  if (!*tokens) {
    perror("Failed to resize tokens array");
    exit(1);
  }
  (*tokens)[num_tokens] = (*tokens)[num_tokens + 1] = NULL;
}

static size_t tokenise(char *const line, char ***tokens) {
  size_t reg_argv_buf_index = 0;
  size_t ret_argv_nmemb = 8;
  size_t ret_argv_index = 0;
  char **ret = calloc(ret_argv_nmemb, sizeof(char *));
  if (!ret) {
    goto fail;
  }

  bool last_was_tok = false;
  while (1) {
    char *const cur = line + reg_argv_buf_index;
    if (*cur == '\0') {
      // if we've hit the end of the line, break
      break;
    } else if (isspace(*cur)) {
      // this is whitespace; if the the last character was part of a token,
      // write a null byte here to terminate the last token
      if (last_was_tok) {
        *cur = '\0';
      }
      last_was_tok = false;
    } else {
      // this is not whitespace (so part of a token)
      // if the previous character was not part of a token (start of line or
      // whitespace), then add this to the result
      if (!last_was_tok) {
        // + 1 for the NULL at the end
        if (ret_argv_index + 1 >= ret_argv_nmemb) {
          // our result array is full, resize it
          ret_argv_nmemb += 8;
          ret = realloc(ret, ret_argv_nmemb * sizeof(char *));
          if (!ret) {
            goto fail;
          }
        }
        ret[ret_argv_index++] = cur;
      }
      last_was_tok = true;
    }
    reg_argv_buf_index++;
  }

  // NULL-terminate the result array
  ret[ret_argv_index] = NULL;
  *tokens = ret;
  return ret_argv_index;

  // N.B. goto is idiomatic for error-handling in C
fail:
  if (ret) {
    free(ret);
  }
  *tokens = NULL;
  return 0;
}
