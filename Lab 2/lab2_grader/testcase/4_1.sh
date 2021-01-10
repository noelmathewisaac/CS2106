#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

rm -f service*.log

UUID="$(arggen)"

run_command startlog /bin/echo "$UUID"
wait_for_string "--- SIGCHLD"
kill_testee

print_logfile_expected_log 0 echo "$UUID"
print_logfile_log 0