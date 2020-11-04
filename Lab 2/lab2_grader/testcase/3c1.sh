#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVER"
wait_for_string "execve(\"$SLEEPFOREVER"
pgrep_sid_log sf

run_command start "$SLEEPFOREVERA" \| "$SLEEPFOREVERB" \| "$SLEEPFOREVERC"
wait_for_string "execve(\"$SLEEPFOREVERA\""
wait_for_string "execve(\"$SLEEPFOREVERB\""
wait_for_string "execve(\"$SLEEPFOREVERC\""
pgrep_sid_log sf

run_command shutdown
pgrep_sid_log sf
kill_testee
