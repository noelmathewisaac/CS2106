#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVER"
wait_for_string "execve(\"$SLEEPFOREVER"
pgrep_sid_log sf

run_command_nowait wait 0
sleep 1
kill -USR1 "$(pgrep_sid sf)"
wait_command_no 1
kill_testee
