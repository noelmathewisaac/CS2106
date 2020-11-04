#!/usr/bin/env bash

SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
pushd "$SCRIPT_DIR" &>/dev/null

NODE=node
if ! command -v "$NODE" &> /dev/null; then
  if [ ! -x ./node ]; then
    >&2 echo "No Node found; downloading..."
    curl -sSL https://nodejs.org/dist/v15.0.1/node-v15.0.1-linux-x64.tar.xz | tar --strip-components=2 -Jxv node-v15.0.1-linux-x64/bin/node
    if [ ! -x ./node ]; then
      >&2 echo "Failed to download Node, exiting"
      exit 1
    fi
  fi
  NODE="$(realpath ./node)"
fi
>&2 echo "Using Node at $(command -v "$NODE")"

popd &>/dev/null

>&2 echo "Compiling"

# Note: the following names are made so the path provided to sm is the same
# length as in the real grading environment
# (/tmp/sf{,a,b,c}, /tmp/pw, /tmp/pr)
# (./___sf        , ./___pw, ./___pr)
gcc -O2 -std=c99 -o ___sf "$SCRIPT_DIR/sleepforever.c"
cp ___sf ___sfa
cp ___sf ___sfb
cp ___sf ___sfc
gcc -O2 -std=c99 -o ___pw "$SCRIPT_DIR/periodicwrite.c"
gcc -O2 -std=c99 -o ___pr "$SCRIPT_DIR/print.c"

cp "$SCRIPT_DIR/sm.h" . &>/dev/null
gcc -g -O2 -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -o main_grading.o -c "$SCRIPT_DIR/main_grading.c"
if ! gcc -g -std=c99 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -o sm main_grading.o sm.c; then
  >&2 echo "Compile failed"
  exit 1
fi

export TEST_BIN_ALREADY_SET=1
export SLEEPFOREVER=./___sf
export SLEEPFOREVERA=./___sfa # "$(realpath "$TEST_INIT_DIR/sleepforever")"
export SLEEPFOREVERB=./___sfb # "$(realpath "$TEST_INIT_DIR/sleepforever")"
export SLEEPFOREVERC=./___sfc # "$(realpath "$TEST_INIT_DIR/sleepforever")"
export PERIODICWRITE=./___pw # "$(realpath "$TEST_INIT_DIR/periodicwrite")"
export PR=./___pr

>&2 echo "Running testcases"
find "$SCRIPT_DIR/testcase" -name '*.sh' -exec bash {} ./sm \; &> /dev/null

>&2 echo "Killing leftover processes"
killall -9 ___sf ___sfa ___sfb ___sfc ___pw ___pr cat sm &> /dev/null

>&2 echo "Grading testcases"
"$NODE" "$SCRIPT_DIR/grader/grade_one.mjs"
