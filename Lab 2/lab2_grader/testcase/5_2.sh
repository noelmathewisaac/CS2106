#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

rm -f service*.log

UUID="b$(arggen)"
UUID2="$(arggen)"

run_command startlog /bin/echo hello
wait_for_string "[\"/bin/echo\""
wait_for_string_in_file_timeout 1 hello service0.log
run_command startlog /bin/echo "$UUID"
wait_for_string "[\"/bin/echo\", \"b"
wait_for_string_in_file_timeout 1 "$UUID" service1.log
chmod u+rw service1.log &>/dev/null
if [ -f service1.log ]; then echo "$UUID2" >> service1.log; fi
run_command showlog 1
kill_testee

print_logfile_expected_log 1 printf "$UUID\n$UUID2\n"
