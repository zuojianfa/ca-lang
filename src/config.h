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

#ifndef __config_h__
#define __config_h__

#include <stdio.h>

#define MAX_PATH 255
#define MAX_GOTO 1024

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

// pos = code_buffer[goto_pcs[i]];
// addrs[x] = addrof(label x); code_buffer[goto_pcs[i]] = addrs[pos];
typedef struct FPIC_AddrEnv {
  int size;
  int goto_pcs[MAX_GOTO];
} FPIC_AddrEnv;

typedef enum LLVM_Gen_Type {
  LGT_LL,
  LGT_S,
  LGT_C,
  LGT_JIT,
  LGT_NATIVE,
} LLVM_Gen_Type;

typedef enum Optimize_Level {
  OL_NONE,      // no optimization also no debug information
  OL_O1,        // do level 1 optimization
  OL_O2,        // do level 2 optimization
  OL_O3,        // do level 3 optimization
} Optimize_Level;

typedef struct CompileEnv {
  char src_path[MAX_PATH + 1];
  FILE *goutput;
  FILE *ginput;
  int *code_buffer;
  int pc;
  int stackbased;
  FPIC_AddrEnv fpic_addr;

  char outfile[MAX_PATH + 1];   // used in llvm generator
  LLVM_Gen_Type llvm_gen_type;
  Optimize_Level opt_level;
  int emit_debug; // if enable debug information
  int emit_main;  // if emit main function
  int emit_dot;   // if emit dot format file (graphviz) for the grammar
  char dotpath[MAX_PATH + 1];   // dot file path when emit_dot is set
  int dot_sparsed;
  FILE *dotout;
  int dot_step;
} CompileEnv;

extern CompileEnv genv;

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

