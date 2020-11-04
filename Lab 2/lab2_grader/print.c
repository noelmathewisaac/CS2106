#include <stdio.h>

#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("\n\nprint-%s\n\n", argv[1]);
  return 0;
}
