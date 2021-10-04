#include <stdio.h>

int main() {
  int a = 1;
  printf("%d\n", a);
  int b = 1;
  printf("%d\n", b);
  int c = 0;
  while (c < 10000000) {
    int t = a;
    a = b;
    b = t + a;
    c = c + 1;
  }

  printf("%d\n", b);

  return 0;
}
