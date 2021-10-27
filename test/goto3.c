int main() {
    int count = 0;
l1:
    if (count >= 5000000) {
      int a = 0;
      a = a + 1;
      if (a == 1) {
	int a = 1;
	a = a + 1;
	if (a == 2) {
	  int a = 2;
	  a = a + 2;
	  int b = 4;
	}
      }
	goto l2;
    }

    count = count + 1;
    goto l1;

l2:
    return count;
}

