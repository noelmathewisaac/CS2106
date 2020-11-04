#!/bin/bash
TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

UUID="$(arggen)"
run_command start "$PERIODICWRITE" "$UUID" \| /bin/cat \| /bin/cat
wait_for_string_timeout 3 "periodic-write-$UUID"
kill_testee
