#ifndef __IR_generator_h__
#define __IR_generator_h__

#include "ca_parser.h"

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

void init_llvm_env();
int walk(RootTree *tree);

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

