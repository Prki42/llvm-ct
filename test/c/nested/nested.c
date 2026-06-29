#include <stdio.h>

int f(int x, int y) {
  int r = 0;
  if (x & 1) {
    for (int i = 0; i < y; i++) {
      if (x & (1 << (i + 1)))
        r += i * 3;
      else
        r += i;
    }
  } else {
    r = y * y;
  }
  return r;
}

int main() {
  for (int x = 0; x < 16; x++)
    printf("f(%d, 4) = %d\n", x, f(x, 4));
  return 0;
}
