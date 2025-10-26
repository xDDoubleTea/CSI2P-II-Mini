#include <stdio.h>
int main() {
  int x = 2;
  int y = 3;
  int z = 5;
  x = x + (y + (z + (x + y + z)));
  printf("x=%d, y=%d, z=%d\n", x, y, z);
  return 0;
}
