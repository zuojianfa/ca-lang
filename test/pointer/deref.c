void func1() {
  int a = 101;
  int *pa = &a;
  int b = *pa;
  int *pb = pa + 1;
  int c = 2;
  pb = pa - c;
}

