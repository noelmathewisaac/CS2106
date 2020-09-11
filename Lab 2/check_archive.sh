#!/bin/bash

STATUS=0

fail() {
  STATUS=1
  >&2 echo "$@"
}

if [ "$#" -ne 1 ]; then
  >&2 echo "You should provide your zip file as a parameter and nothing else!"
  exit 1
fi

if [ ! -f "$1" ]; then
  >&1 echo "$1 does not exist or is not a file."
  exit 1
fi

if ! [[ $1 =~ ^E[0-9]{7}.zip$ ]]; then
  >&2 echo "zip file is wrongly named: it should be <NUSNET ID>.zip"
  exit 1
fi

echo "Checking zip file...."

TMPDIR="$(mktemp -d)"
ZIPPATH="$(realpath "$1")"

pushd "$TMPDIR" &> /dev/null
unzip "$ZIPPATH" sm.c 'ex6/*' &> /dev/null

if [ ! -f sm.c ]; then
  fail "Ex1-5: Failed - sm.c missing"
else
  cat > sm.h <<'EOF'
#ifndef CS2106_SM_H
#define CS2106_SM_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define SM_MAX_SERVICES 32

typedef struct sm_status {
  pid_t pid;
  const char *path;
  bool running;
} sm_status_t;

void sm_init(void);
void sm_free(void);
void sm_start(const char *processes[]);
void sm_startlog(const char *processes[]);
size_t sm_status(sm_status_t statuses[]);
void sm_stop(size_t index);
void sm_wait(size_t index);
void sm_shutdown(void);
void sm_showlog(size_t index);

#endif
EOF

  if ! gcc -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -c sm.c &> /dev/null; then
    fail "Ex1-5: Failed - sm.c failed to compile"
  fi
fi

if [ -d ex6 ]; then
  pushd ex6 &> /dev/null
  for f in main.c sm.h sm.c smc.c README; do
    if [ ! -f $f ]; then
      fail "Ex6: Failed - $f missing"
    fi
  done

  if ! gcc -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -o sm sm.c main.c &> /dev/null; then
    fail "Ex6: Failed - sm (sm.c and main.c) failed to compile"
  fi

  if ! gcc -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -o sm smc.c &> /dev/null; then
    fail "Ex6: Failed - smc (smc.c) failed to compile"
  fi
  popd &> /dev/null
fi

if [ $STATUS -eq 0 ]; then
  >&2 echo "All OK"
else
  >&2 echo "There were one or more errors. Please check your zip."
fi

popd &> /dev/null
rm -rf "$TMPDIR"

exit $STATUS
