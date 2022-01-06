int main() {
	unsigned int *p1 = 0;
	unsigned int *p2 = (unsigned int *)12345678;
	int *pp1 = (int *)p1;
	int ****pp2 = (int ****)p2;
	//int ***ppp1 = (int ***)pp1;
	//int ***ppp2 = (int ***)pp2;
	// unsigned int **ppu1   = (unsigned int **)p1;
	// unsigned int **ppu2   = (unsigned int **)p2;
	// unsigned int ***pppu1 = (unsigned int ***)pp1;
	// unsigned int ***pppu2 = (unsigned int ***)pp2;
	return 0;
}

