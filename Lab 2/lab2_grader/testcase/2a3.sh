#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVER" \| $PR hello
wait_for_string "execve(\"$SLEEPFOREVER"
wait_for_string "execve(\"$PR"
wait_for_string "print-hello"
kill_testee
