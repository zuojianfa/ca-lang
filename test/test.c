#include <stdio.h>

int main() {
	int i = 0, j = 0;
	int l1 = 0;
l1 :
	++i; l3:  ++j;
	if (i == 0) {
		++i;
		goto l1;
	}

	if (i == 1) {
		++i;
		goto l3;
	}


 l2 :
 printf("i: %d, j: %d\n", i, j);

return 0;
}
