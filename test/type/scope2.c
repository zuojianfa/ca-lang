struct AA {
  int f1;
  char f2;
};

int main() {
    struct AA a = {3231, 1};
    {
	struct AA {
	  double f1;
	};
	struct AA a = {101.11};
	//let b = true;
    }
}

