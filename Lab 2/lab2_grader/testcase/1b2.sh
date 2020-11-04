#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start /bin/echo hello
wait_for_string "execve(\"/bin/echo"

run_command start "$SLEEPFOREVER"
wait_for_string "execve(\"$SLEEPFOREVER"
SLEEPFOREVER_PID="$(pgrep_sid sf)"
if [ -z "$SLEEPFOREVER_PID" ]; then
  sleep 0.5
  kill_testee
  exit 1
fi

run_command status
send_usr1_wait_sigchld $SLEEPFOREVER_PID
wait_until_zombie $SLEEPFOREVER_PID
run_command status
kill_testee
