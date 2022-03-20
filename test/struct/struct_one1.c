struct C1 {
  char f1;
};

struct AA {
  int f1;
  double f2;
  char f3;
};

struct A1 {
  double f2;
  int f1;
  char f3;
};

struct A2 {
  struct AA aa;
  struct A1 a1;
};

struct A21 {
  char c1;
  struct AA aa;
  struct A1 a1;
};

struct A22 {
  struct AA aa;
  struct A1 a1;
  char c1;
};

struct A3 {
  struct AA aa;
  struct A1 a1;
  struct C1 c1;
};

int main() {
  struct AA a = {32, 44.3, 'C'};
  struct A1 b = {44.3, 32, 'D'};
  struct A2 c = {{32, 44.3, 'C'}, {44.3, 32, 'D'}};
  struct A3 a3 = {{32, 44.3, 'C'}, {44.3, 32, 'D'}, {'B'}};
  struct C1 c1 = {'A'};

  struct A21 a21;
  struct A22 a22;

  double f1;
  int f2;
  double f21;
  char f3;
  double f4;

  int sa = sizeof(a);
  int sb = sizeof(b);
}

