#ifndef __ca_runtime_h__
#define __ca_runtime_h__

#include "ca_types.h"

//#define TEST_RUNTIME // for test runtime functions in when print

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

#ifdef TEST_RUNTIME
int rt_add(int a, int b);
int rt_sub(int a, int b);
#endif

#ifdef __cplusplus
END_EXTERN_C
#endif
#endif

