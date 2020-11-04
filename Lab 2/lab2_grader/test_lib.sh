run_testee() {
  # initially this was a new guid every time, but for easier parsing let's just
  # use one guid..
  DIVIDER="bfa06e76-6ffe-46aa-b71e-426bf7fb37f2"
  LOGFILE="$1.$TESTCASE.trace"
  declare -gi COMMAND_COUNT=0
  rm -f "$LOGFILE"
  coproc setsid -w -- "$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"/strace -f -b execve -e trace='execve,kill,fork,clone' -qqq -s 1024 -- "$1" "$DIVIDER" &>> "$LOGFILE"
  TESTEE_PID=$!
  echo "----------testeepid: $TESTEE_PID" >> "$LOGFILE"
}

# might be worth writing a small utility for this, this is quite unwieldy and creates like 5 processes :/
wait_for_string_in_file_timeout() {
  chmod u+rw "$3" &>/dev/null
  { ( THIS_BASH_PID=$BASHPID; tail -n +0 --pid=$THIS_BASH_PID -f "$3" | { timeout -s 9 "$1" sed "/${2//\//\\/}/ q" && kill -9 $THIS_BASH_PID; } ) } &>/dev/null
}

wait_for_string_timeout() {
  wait_for_string_in_file_timeout "$1" "$2" "$LOGFILE"
}

wait_for_string() {
  wait_for_string_timeout 1 "$1"
}

wait_command_no() {
  wait_for_string "----------${DIVIDER}-${1}-END----------"
}

wait_command() {
  wait_command_no $COMMAND_COUNT
}

run_command() {
  echo "----------command: $@" >> "$LOGFILE"
  echo "$@" >&${COPROC[1]}
  wait_command
  COMMAND_COUNT+=1
}

run_command_nowait() {
  echo "----------command: $@" >> "$LOGFILE"
  echo "$@" >&${COPROC[1]}
  COMMAND_COUNT+=1
}

kill_testee() {
  if [ -n "$COPROC_PID" ] && [ -f "/proc/$TESTEE_PID/stat" ]; then
    kill -9 $(ps -s "$COPROC_PID" -o pid=)
  fi
  wait $TESTEE_PID &>/dev/null
  TESTEE_EXIT=$?
  case "$TESTEE_EXIT" in
  0 | 137) ;;
  *)
    >&2 echo "Non-successful exit: $TESTEE_EXIT $PWD"
    echo "----------exitcode: $TESTEE_EXIT" >> "$LOGFILE"
    ;;
  esac

}

wait_until_zombie() {
  local -i count=0
  until [ $count -ge 10 ] || [ ! -f "/proc/$1/stat" ] || [ "$(cat /proc/$1/stat 2>/dev/null | cut -d' ' -f3)" = "Z" ]; do
    sleep 0.1
    count+=1
  done
}

pgrep_sid() {
  pgrep -s "$TESTEE_PID" "$@"
}

pgrep_sid_log() {
  echo "----------pids $(pgrep_sid "$@" | xargs -n100 echo)" >> "$LOGFILE"
}

print_logfile_expected_log() {
  SERVICE_NO=$1
  shift
  echo "----------logfile${SERVICE_NO}-expected-BEGIN----------" >> "$LOGFILE"
  "$@" >> "$LOGFILE"
  echo "----------logfile${SERVICE_NO}-expected-END----------" >> "$LOGFILE"
}

print_logfile_log() {
  echo "----------logfile$1-BEGIN----------" >> "$LOGFILE"
  chmod u+r "service$1.log" &> /dev/null
  cat "service$1.log" >> "$LOGFILE" 2> /dev/null
  echo "----------logfile$1-END----------" >> "$LOGFILE"
}

arggen() {
  dd if=/dev/urandom bs=6 count=1 2>/dev/null | base64
}

send_usr1_wait_sigchld() {
  kill -USR1 "$1"
  wait_for_string "si_pid=$1"
  sleep 0.25
}
