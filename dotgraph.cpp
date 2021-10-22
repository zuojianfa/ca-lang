
#include "dotgraph.h"
#include "config.h"

#define CHECK_ENABLE_DOT() do { if (!genv.emit_dot) return; } while(0)

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

void dot_emit(const char *from, const char *to) {
  CHECK_ENABLE_DOT();

  fprintf(genv.dotout, "\t\"%s\" -> \"%s\" [ label=\"%d\" ];\n",
	  from, to, ++genv.dot_step);
}

void dot_finalize() {
  CHECK_ENABLE_DOT();

  fprintf(genv.dotout, "}\n");
  fclose(genv.dotout);
}

