#include <stdio.h>
#include <stdlib.h>

int gf_mul(int a, int b) {
  int result = 0;
  if (b & 1)
    result = result ^ a;
  a = a << 1;
  if (a & 0x100)
    a = a ^ 0x11b;
  if (b & 2)
    result = result ^ a;
  a = a << 1;
  if (a & 0x100)
    a = a ^ 0x11b;
  if (b & 4)
    result = result ^ a;
  return result;
}

int main(int argc, char **argv) {
  int a = argc > 1 ? atoi(argv[1]) : 0x53;
  for (int b = 0; b < 8; b++)
    printf("gf_mul(0x%x, 0x%x) = 0x%x\n", a, b, gf_mul(a, b));
  return 0;
}
