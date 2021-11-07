#ifndef __type_system_h__
#define __type_system_h__

#include "ca.h"

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

const char *get_printf_format(int type);
int is_unsigned_type(int type);
const char *get_printf_format(int type);
int inference_literal_type(CALiteral *lit);
void determine_literal_type(CALiteral *lit, int typetok);
const char *get_type_string(int tok);
int inference_literal_type(CALiteral *lit);

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

