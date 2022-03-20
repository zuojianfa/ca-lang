struct AA {
  double a;
  char b;
};

int main() {
  struct AA a = {1.23, 'C'};
  {
    struct AA {
      char b;
    };
  
    struct AA a = {'D'};
    int i = 0;
    {
      struct AA {
	int a;
      };

      struct AA a = {12};
      int b = 0;
    }
  }
  return 0;
}
