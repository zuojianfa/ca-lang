#include "ca_runtime.h"

struct Slice {};

#ifdef TEST_RUNTIME
int rt_add(int a, int b) { return a + b; }
int rt_sub(int a, int b) { return a - b; }
#endif

