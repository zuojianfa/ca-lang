#include <stdio.h>
#include <string.h>

int main() {
    char *h = "hello";
    char *w = "world";

    puts(h);
    puts(w);

    strcpy(h, w);

    puts(h);
    puts(w);

    return 0;
}

