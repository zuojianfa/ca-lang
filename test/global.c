int global_a_noinit;
int global_a_withinit = 2;
double global_a_f = 3.4;

int fn1() {
	return 2;
}

int fn2() {
	int a = fn1();
	return a;
}

