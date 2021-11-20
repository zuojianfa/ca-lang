#ifndef __type_system_h__
#define __type_system_h__

#include "ca.h"
#include <stdint.h>

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
CADataType *catype_make_type_symname(int formalname, int type, int size);
CADataType *catype_make_pointer_type(CADataType *datatype);
CADataType *catype_make_array_type(CADataType *type, uint64_t len);
CADataType *catype_make_struct_type(int symname, ST_ArgList *arglist);
CADataType *catype_make_unknown_type(int formalname, int size);

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

