#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

rm -f service*.log

PREUUID="$(arggen)"
echo "$PREUUID" > service0.log
UUID="$(arggen)"

run_command startlog /bin/echo "$UUID"
wait_for_string "--- SIGCHLD"
run_command showlog 0
kill_testee

print_logfile_expected_log 0 printf "$PREUUID\n$UUID\n"
