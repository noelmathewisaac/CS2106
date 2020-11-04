#include <stdlib.h>

#include <unistd.h>
#include <signal.h>

void handle_sigusr(int signum) {
  (void) signum;
  _Exit(0);
}

int main() {
  signal(SIGUSR1, handle_sigusr);
  while (1) {
    sleep(100000);
  }
}
