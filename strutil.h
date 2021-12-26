#ifndef __strutil_h__
#define __strutil_h__

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

int strutil_parse_ident_str(const char *input, char *output);
int strutil_parse_alnum_str(const char *input, char *output);
int strutil_parse_number_str(const char *input, char *output);
int strutil_parse_alpha_str(const char *input, char *output);

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

