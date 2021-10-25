
#include "dotgraph.h"
#include "config.h"
#include "ca.tab.h"

#include <unordered_map>

#define CHECK_ENABLE_DOT()			\
  do {						\
    if (!genv.emit_dot)				\
      return;					\
  } while (0)

// the name sequence number, each left (from) name will increase it's sequence
// number and as the new left dot name, each right (to) name will find it's
// current sequence number which as the new right dot name, the right (to) name
// should already registered in the map, because the bison is a LR bottom-up
// parser.
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

  // if first from not exist the map return number will be 0 and seq will be 1
  int seqfrom = ++s_name_seq[from];
  const char *seqfromname = create_seqname(from, seqfrom, 1);

  // when seqto is 0, it means the right name is just generating or parsing
  int seqto = s_name_seq[to];
  const char *seqtoname = create_seqname(to, seqto, 0);

  // TODO: set original name as the label to the dot node
  fprintf(genv.dotout, "\t\"%s\" -> \"%s\" [ label=\"%d\" ];\n",
	  seqfromname, seqtoname, ++genv.dot_step);
}

void dot_emit_expr(const char *from, const char *to, int op) {
  switch(op) {
  case ARG_LISTS:
    dot_emit("fn_args_ps", "");
    break;
  case ARG_LISTS_ACTUAL:
    dot_emit("fn_args_call", "");
    break;
  case ';':
    // when left = right = NULL
    dot_emit("stmt", ";");
    // or when left, right != NULL
    dot_emit("stmt_list", "stmt_list stmt");
    break;
  case PRINT:
    dot_emit("stmt", "PRINT expr");
    break;
  case RET:
    // when return expression
    dot_emit("stmt", "RET expr");
    // or when return nothing
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

