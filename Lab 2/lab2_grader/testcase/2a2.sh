#!/bin/bash

TESTCASE="$(basename "${BASH_SOURCE[0]}" .sh)"
SCRIPT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
. "$SCRIPT_DIR/../test_init.sh"

UUID="$(arggen)"
OUTPUT="$(sha256sum <<< "$UUID")"
run_command start /bin/echo $(arggen) \| /bin/cat -u \| /bin/cat -uu \| /bin/cat -uuu \| /bin/cat -uuuu \| /bin/echo "$UUID" \| /bin/cat -uuuuu \| /bin/cat -uuuuuu \| /usr/bin/sha256sum
wait_for_string "execve(\"/usr/bin/sha256sum"
wait_for_string_timeout 2 "$OUTPUT"
kill_testee
