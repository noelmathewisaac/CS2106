#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start /bin/echo a
wait_for_string "execve(\"/bin/echo"
pgrep_sid_log echo

run_command wait 0

run_command start "$SLEEPFOREVER"
wait_for_string "execve(\"$SLEEPFOREVER"
pgrep_sid_log sf

run_command shutdown
pgrep_sid_log ""
kill_testee
