int global_a_noinit;
int global_a_withinit = 2;
double global_a_f = 3.4;

int fn1() {
	int local_a;
	int local_b = 3;
	double local_c = 3.4;
	
	return local_a + local_b;
}

int fn2() {
	int a = fn1();
	return a;
}

