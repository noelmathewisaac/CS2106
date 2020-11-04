#include <stdio.h>

#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }
  setvbuf(stdout, NULL, _IONBF, 0);
  while (1) {
    sleep(1);
    printf("\n\nperiodic-write-%s\n\n", argv[1]);
  }
}
