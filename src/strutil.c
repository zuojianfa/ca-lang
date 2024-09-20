/**
 * Copyright (c) 2023 Rusheng Xia <xrsh_2004@163.com>
 * CA Programming Language and CA Compiler are licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include "strutil.h"

#include <ctype.h>


// return the consumed input size, 0: means no string is consumed, also means no output put
int strutil_parse_ident_str(const char *input, char *output) {
  if (!isalpha(*input))
    return 0; // format error

  int i = 0;
  do {
    output[i++] = *input++;
  } while(isalnum(*input) || *input == '_');

  output[i] = '\0';
  return i;
}


int strutil_parse_alnum_str(const char *input, char *output) {
  int i = 0;
  while(isalnum(*input)) {
    output[i++] = *input++;
  } 

  output[i] = '\0';
  return i;
}


int strutil_parse_number_str(const char *input, char *output) {
  int i = 0;
  while(isdigit(*input)) {
    output[i++] = *input++;
  } 

  output[i] = '\0';
  return i;
}

int strutil_parse_alpha_str(const char *input, char *output) {
  int i = 0;
  while(isalpha(*input)) {
    output[i++] = *input++;
  } 

  output[i] = '\0';
  return i;
}

