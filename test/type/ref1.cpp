struct AA {
  char &a;
  double &b;
};

int main() {
  // reference type implemented as pointer which initially must have a value
  // all the operation on it will operate on the object it points to
  char a = '1';
  double b = 2.2;
  AA aa = AA{a, b};

  //return sizeof(AA); // = 16
}
