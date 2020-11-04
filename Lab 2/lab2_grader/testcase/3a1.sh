#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVER"
wait_for_string "execve(\"$SLEEPFOREVER"
pgrep_sid_log sf

run_command stop 0
pgrep_sid_log sf

run_command stop 0

kill_testee
