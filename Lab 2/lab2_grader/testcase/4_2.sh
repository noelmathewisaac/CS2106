#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

rm -f service*.log

UUID="$(arggen)"
EXPECTED="$(sha256sum <<< "$UUID")"

run_command startlog /bin/echo "$UUID" \| /bin/cat \| /usr/bin/sha256sum
wait_for_string "--- SIGCHLD"
wait_for_string_in_file_timeout 1 "$EXPECTED" service0.log
kill_testee

print_logfile_expected_log 0 echo "$EXPECTED"
print_logfile_log 0
