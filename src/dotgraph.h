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

/**
 * @brief Generate dot graph files representing the program's grammar tree.
 */

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

