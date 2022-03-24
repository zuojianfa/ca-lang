int func1() {
    return 1;
}

int main() {
    int a = 1;
    int b = 2;
    int c = a < b;
    int d = a > b;

    int e = c && func1();
    int f = c || func1();
    int e2 = d && func1();
    int f2 = d || func1();

    return 0;
}

