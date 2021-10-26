int main() {
    int first = 1;
    int second = 1;
    int count = 0;
    int tmp = 0;

l1:
    if (count >= 5000000) {
	goto l2;
    }

    tmp = first;
    first = second;
    second = tmp + first;
    count = count + 1;
    goto l1;

l2:

    return second;
}

