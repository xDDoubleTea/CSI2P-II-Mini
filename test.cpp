#include <iostream>

int tests(int &x, int &y, int &z, int idx) {
  switch (idx) {
  case 1:
    z = x + 5;
    y = z / 10 - 7 * x;
    // -y - (+z) % (z + 100);
    z = (x++) + (y--);
    x = (--y) * (++z);
    x = z - +-+-+-++y;
    ;
    x = y = z = 3 + 5;
    return 0;
  case 2:
    ((((x)))) = (y) = (((((z)))))++;
    --(x);
    // y=-5---x+++z;
    return -1;
  case 3:
    // --(((((+y)))));
    // ++ +y;
    // -- -y;
    // (1) = -x;
    // 3 ++;
    return -1;
  case 4:
    x = (x +
         (y -
          (z *
           (x /
            (y %
             (z + (x - (y * (z / (x % (y + (z - (x * (y / (z % 5)))))))))))))));
    // std::cout << x << ", " << y << ", " << z << std::endl;
    y = (((((((((((((((x + 5) - y) * z) / x) % y) + z) - x) * y) / z) % x) +
             y) -
            z) *
           x) /
          y) %
         z);
    return 0;
  case 5:
    // y + 5 * x - 2 + z * 3;
    x = 5;
    y = 6;
    x = (3 + 5) - 8 * (10 / 2);
    y = x * x - (12 * 12);
    z = z / z + (+-+-+-+-z - z) + (x * z) % z + (y + z) * 0 - x * y;
    x = (-y * -y - (y * y - 4 * x * z)) / (2 * x * 2 * x);
    return 0;
  case 6:
    // x = ;
    // y = x++ + ;
    // x++
    return -1;
  case 7:
    x = 50 + 10 / 2 - 7 * 3;
    y = (100 % 9) * (2 + 3);
    z = -y + -x;
    x = (5 * 5 * 5) / (2 * 2 * 2);
    y = z / (x + 1);
    return 0;
  case 8:
    x = 10;
    y = 20;
    z = 30;
    // x + y + z;
    // y * 10 / x;
    // z;
    x = x + 1;
    y++;
    --z;
    z = x + y;
    return 0;
  case 9:
    x = 10;
    y = x + 5;
    z = y - x;
    x = x + 1;
    x = z;
    y = x;
    z = y + x + z;
    return 0;
  default:
    return -1;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s [x,y,z,idx]\n", argv[0]);
    return 1;
  }

  int x = argv[1] ? atoi(argv[1]) : 2;
  int y = argv[2] ? atoi(argv[2]) : 3;
  int z = argv[3] ? atoi(argv[3]) : 5;
  int idx = argv[4] ? atoi(argv[4]) : 1;
  // std::cout << "x, y, z  = " << x << ", " << y << ", " << z << std::endl;
  int res = tests(x, y, z, idx);
  std::cout << "x, y, z = " << x << ", " << y << ", " << z << std::endl;
  std::cout << "res = " << res << std::endl;
  return 0;
}
