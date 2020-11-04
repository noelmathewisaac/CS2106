#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVERA" \| "$SLEEPFOREVERB" \| "$SLEEPFOREVERC"
wait_for_string "execve(\"$SLEEPFOREVERA\""
wait_for_string "execve(\"$SLEEPFOREVERB\""
wait_for_string "execve(\"$SLEEPFOREVERC\""
pgrep_sid_log sf

run_command stop 0
pgrep_sid_log sf

run_command stop 0

kill_testee
