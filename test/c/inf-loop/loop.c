#include <stdio.h>
#include <stdlib.h>

// Runs infinitely without branch condition lowering

int main(int argc, char **argv) {
  int a = argc > 1 ? atoi(argv[1]) : 5;
  if (a > 0) {
    while(1) if (a > 0) break;
  } else {
    while (1) if (a < 0) break;
  }
  printf("OK");
}
