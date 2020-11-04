#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVERA" \| "$SLEEPFOREVERB" \| "$SLEEPFOREVERC"
wait_for_string "execve(\"$SLEEPFOREVERA\""
wait_for_string "execve(\"$SLEEPFOREVERB\""
wait_for_string "execve(\"$SLEEPFOREVERC\""
pgrep_sid_log sf

run_command_nowait wait 0
sleep 0.5
kill -USR1 "$(pgrep_sid sfa)"
sleep 0.5
kill -USR1 "$(pgrep_sid sfb)"
sleep 0.5
kill -USR1 "$(pgrep_sid sfc)"
sleep 0.5
wait_command_no 1

kill_testee
