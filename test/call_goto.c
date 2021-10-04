#include <stdio.h>

extern int main_fn();

int main() {
	int ret = main_fn();
	printf("ret = %d\n", ret);
	return 0;
}

