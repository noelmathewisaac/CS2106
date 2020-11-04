#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

run_command start "$SLEEPFOREVERA $(uuidgen) $(uuidgen) $(uuidgen)"
wait_for_string "execve(\"$SLEEPFOREVERA"
sleep 0.2
kill_testee
