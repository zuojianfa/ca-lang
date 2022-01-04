int main() {
	unsigned int *p1 = 0;
	unsigned int *p2 = (unsigned int *)0x12345678;
	int **pp1 = (int **)p1;
	int **pp2 = (int **)p2;
	int ***ppp1 = (int ***)pp1;
	int ***ppp2 = (int ***)pp2;
	unsigned int **pp1   = (int **)p1;
	unsigned int **pp2   = (int **)p2;
	unsigned int ***ppp1 = (int ***)pp1;
	unsigned int ***ppp2 = (int ***)pp2;
}

