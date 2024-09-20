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

#include "dotgraph.h"
#include "config.h"
#include "ca.tab.h"

#include <string>
#include <unordered_map>

#define CHECK_ENABLE_DOT()			\
  do {						\
    if (!genv.emit_dot)				\
      return;					\
  } while (0)

/**
 * @brief Description of the DOT file format for the grammar tree.
 *
 * The DOT file format includes left and right nodes representing
 * the connections in the grammar tree. Sequence numbers are used
 * to denote the order of procedures within the grammar tree.
 *
 * The name sequence number works as follows:
 * - Each left (from) name increments its sequence number, becoming
 *   the new left dot name.
 * - Each right (to) name retrieves its current sequence number,
 *   serving as the new right dot name.
 *
 * @note The right (to) name must already be registered in the map,
 * as Bison is an LR bottom-up parser, ensuring that all references
 * are valid.
 */
std::unordered_map<std::string, int> s_name_seq;

void dot_init() {
  CHECK_ENABLE_DOT();
  genv.dotout = fopen(genv.dotpath, "w");
  if (!genv.dotout) {
    yyerror("open dot file failed: '%s'", genv.dotpath);
    return;
  }

  fprintf(genv.dotout, "digraph {\n");
  fprintf(genv.dotout, "rankdir=LR;\n"); // Left to Right, instead of Top to Botto
  fprintf(genv.dotout, "\t\"%s\" [color=cyan, style=filled] ;\n", "program");
  genv.dot_step = 0;
}

static const char *create_seqname(const char *name, int seq, int from) {
  static char fromnamebuf[1024];
  static char tonamebuf[1024];
  
  sprintf(from ? fromnamebuf : tonamebuf, "%s-%d", name, seq);
  return from ? fromnamebuf : tonamebuf;
}

void dot_emit(const char *from, const char *to) {
  CHECK_ENABLE_DOT();

  if (!genv.dot_sparsed) {
    fprintf(genv.dotout, "\t\"%s\" -> \"%s\" [ label=\"%d\" ];\n",
	    from, to, ++genv.dot_step);
    return;
  }

  /*
   * If the first "from" name does not exist in the map, the returned
   * number will be 0, and the sequence (seq) will be set to 1.
   */
  int seqfrom = ++s_name_seq[from];
  const char *seqfromname = create_seqname(from, seqfrom, 1);

  /*
   * When seqto is 0, it indicates that the right name is either being
   * generated or parsed for the first time.
   */
  int seqto = s_name_seq[to];
  const char *seqtoname = create_seqname(to, seqto, 0);

  // TODO: Set the original name as the label for the dot node.
  fprintf(genv.dotout, "\t\"%s\" -> \"%s\" [ label=\"%d\" ];\n",
	  seqfromname, seqtoname, ++genv.dot_step);
}

void dot_emit_expr(const char *from, const char *to, int op) {
  switch(op) {
  case DBGPRINT:
    dot_emit("stmt", "DBGPRINT expr");
    break;
  case RET:
    // if return an expression
    dot_emit("stmt", "RET expr");
    // or if return nothing
    dot_emit("stmt", "RET");
    break;
  case UMINUS:
    dot_emit("expr", "-expr");
    break;
  case '+':
    dot_emit("expr", "expr '+' expr");
    break;
  case '-':
    dot_emit("expr", "expr '-' expr");
    break;
  case '*':
    dot_emit("expr", "expr '*' expr");
    break;
  case '/':
    dot_emit("expr", "expr '/' expr");
    break;
  case '%':
    dot_emit("expr", "expr '%' expr");
    break;
  case '<':
    dot_emit("expr", "expr '<' expr");
    break;
  case '>':
    dot_emit("expr", "expr '>' expr");
    break;
  case GE:
    dot_emit("expr", "expr GE expr");
    break;
  case LE:
    dot_emit("expr", "expr LE expr");
    break;
  case NE:
    dot_emit("expr", "expr NE expr");
    break;
  case EQ:
    dot_emit("expr", "expr EQ expr");
    break;
  default:
    break;
  }
}

void dot_finalize() {
  CHECK_ENABLE_DOT();

  fprintf(genv.dotout, "}\n");
  fclose(genv.dotout);
}

