#ifndef __dotgraph_h__
#define __dotgraph_h__

#include "ca_parser.h"

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

void dot_init();
void dot_emit(const char *from, const char *to);
void dot_emit_expr(const char *from, const char *to, int op);
void dot_finalize();

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif
