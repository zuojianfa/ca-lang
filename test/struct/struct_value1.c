struct AA {
  int f1;
  double f2;
};

int main() {
  int a = 0x12345678;
  double b = 3.1415926;
  struct AA aa1 = {a, b};
  //let aa2 = AA {f1: 0x123456789i64, f2: 3.1415926, f3: true};
  //let aa3: AA = AA {0x123456789i64, 3.1415926, true};
  //let aa4: AA = AA {f1: 0x123456789i64, f2: 3.1415926, f3: true};
  return 0;
}

