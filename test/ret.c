int main() {
	int a = 1;
	int b = 2;
	if (a == b) {
		int a1 = 1;
		int b1 = 2;
		if (a1 == b1) {
			return 1;
		} else {
			int a3 = 2;
			int b3 = 3;
			if (a3 != b3)
				return 4;
			else
				return 5;
		}
	}
	int c = 3;
	int d = 4;
	if (c + 1 == d)
		return 6;
	else
		return 10;

	return 11;
}
