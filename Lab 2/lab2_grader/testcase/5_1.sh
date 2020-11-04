#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

rm -f service*.log

UUID="$(arggen)"
UUID2="$(arggen)"

run_command startlog /bin/echo "$UUID"
wait_for_string "--- SIGCHLD"
chmod u+rw service0.log &>/dev/null
if [ -f service0.log ]; then echo "$UUID2" >> service0.log; fi
run_command showlog 0
kill_testee

print_logfile_expected_log 0 printf "$UUID\n$UUID2\n"
