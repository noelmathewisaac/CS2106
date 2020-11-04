# set -ux
TEST_INIT_DIR="$(dirname "$(realpath -s "${BASH_SOURCE[0]}")")"
if [ -z "$TEST_BIN_ALREADY_SET" ]; then
  SLEEPFOREVER=/tmp/sf # "$(realpath "$TEST_INIT_DIR/sleepforever")"
  SLEEPFOREVERA=/tmp/sfa # "$(realpath "$TEST_INIT_DIR/sleepforever")"
  SLEEPFOREVERB=/tmp/sfb # "$(realpath "$TEST_INIT_DIR/sleepforever")"
  SLEEPFOREVERC=/tmp/sfc # "$(realpath "$TEST_INIT_DIR/sleepforever")"
  PERIODICWRITE=/tmp/pw # "$(realpath "$TEST_INIT_DIR/periodicwrite")"
  PR=/tmp/pr
fi

. "$TEST_INIT_DIR/test_lib.sh"

if [ -z "$1" ]; then
  >&2 echo "Usage: $0 <testee>"
  exit 1
fi

cd "$(dirname "$1")"
run_testee "./$(basename "$1")"
