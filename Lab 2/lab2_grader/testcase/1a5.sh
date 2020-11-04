#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

UUID="$(arggen)"
run_command start "$PR $UUID"
wait_for_string "print-$UUID"
kill_testee
