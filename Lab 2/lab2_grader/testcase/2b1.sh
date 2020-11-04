#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVERA" \| "$SLEEPFOREVERB" \| "$SLEEPFOREVERC"
wait_for_string "execve(\"$SLEEPFOREVERA\""
wait_for_string "execve(\"$SLEEPFOREVERB\""
wait_for_string "execve(\"$SLEEPFOREVERC\""

run_command status

send_usr1_wait_sigchld "$(pgrep_sid sfa)"
run_command status

send_usr1_wait_sigchld "$(pgrep_sid sfb)"
run_command status

SFC_PID="$(pgrep_sid sfc)"
send_usr1_wait_sigchld "$SFC_PID"
wait_until_zombie "$SFC_PID"
run_command status

kill_testee
