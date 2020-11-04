#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

rm -f service*.log

# UUID0="$(arggen)"
UUID1="$(arggen)"
UUID2="$(arggen)"

run_command start /bin/echo "$UUID0"
# wait_for_string_in_file_timeout 1 "$UUID0" service0.log

run_command startlog /bin/echo "$UUID1"
wait_for_string_in_file_timeout 1 "$UUID1" service1.log

run_command startlog /bin/echo "$UUID2"
wait_for_string_in_file_timeout 1 "$UUID2" service2.log

kill_testee

# print_logfile_expected_log 0 echo "$UUID0"
# print_logfile_log 0

print_logfile_expected_log 1 echo "$UUID1"
print_logfile_log 1

print_logfile_expected_log 2 echo "$UUID2"
print_logfile_log 2
