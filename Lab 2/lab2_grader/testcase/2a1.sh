#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

UUID="$(arggen)"
OUTPUT="$(sha256sum <<< "$UUID")"
run_command start /bin/echo $(arggen) \| /bin/cat \| /usr/bin/sha256sum
wait_for_string "execve(\"/bin/echo"
wait_for_string "execve(\"/bin/cat"
wait_for_string "execve(\"/usr/bin/sha256sum"
wait_for_string "$OUTPUT"
kill_testee
