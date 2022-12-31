#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "dotgraph.h"
#include "config.h"
#include "IR_generator.h"

#define MAX_OPS 10000

extern CompileEnv genv;
extern FILE *yyin;
extern RootTree *gtree;

extern "C" {
int yyparse(void);
int yyparser_init();
}

static void usage() {
  // [-ll] | [-S] | [-native] | [-c] | [-jit] [-O] | [-g] | [-nomain] | [-dot <dotfile>]
  fprintf(stderr,
	  "Usage: ca [options] <input> [<output>]\n"
	  "Options:\n"
	  "         -ll:      compile into IR assembly file: .ll (llvm)\n"
	  "         -S:       compile into native (as) assembly file: .s\n"
	  "         -native:  compile into native execute file: ELF file on linux, PE file on windows (default value)\n"
	  "         -c:       compile into native object file: .o\n"
	  "         -jit:     interpret using jit (llvm)\n"
	  "         -O[123]:  do optimization of level 1 2 3, default is level 2\n"
	  "         -g:       do not do any optimization (default value)\n"
	  "         -main:    do generate the default main function\n"
	  "         -dot <dotfile>:  generate the do not generate the default main function\n"
	  );
  exit(-1);
}

static int init_config(int argc, char *argv[]) {
  int arg = 0;

  if (++arg >= argc)
    usage();

  genv.llvm_gen_type = LGT_JIT;
  genv.opt_level = OL_NONE;
  genv.emit_debug = 0;
  genv.emit_main = 0;
  genv.emit_dot = 0;
  genv.dot_sparsed = 1;

  int i = 0;
  while(i < 3) {
    if (argv[arg][0] == '-') {
      if (!strcmp(argv[arg], "-ll")) {
	genv.llvm_gen_type = LGT_LL;
      } else if (!strcmp(argv[arg], "-S")) {
	genv.llvm_gen_type = LGT_S;
      } else if (!strcmp(argv[arg], "-native")) {
	genv.llvm_gen_type = LGT_NATIVE;
      } else if (!strcmp(argv[arg], "-c")) {
	genv.llvm_gen_type = LGT_C;
      } else if (!strcmp(argv[arg], "-jit")) {
	genv.llvm_gen_type = LGT_JIT;
      } else if (!strcmp(argv[arg], "-O")) {
	genv.opt_level = OL_O2;
      } else if (!strcmp(argv[arg], "-O1")) {
	genv.opt_level = OL_O1;
      } else if (!strcmp(argv[arg], "-O2")) {
	genv.opt_level = OL_O2;
      } else if (!strcmp(argv[arg], "-O3")) {
	genv.opt_level = OL_O3;
      } else if (!strcmp(argv[arg], "-g")) {
	genv.emit_debug = 1;
      } else if (!strcmp(argv[arg], "-main")) {
	genv.emit_main = 1;
      } else if (!strcmp(argv[arg], "-dot")) {
	genv.emit_dot = 1;
	if (++arg >= argc || argv[arg][0] == '-') {
	  fprintf(stderr, "Should specify a dot file path\n\n");
	  usage();
	}
	strcpy(genv.dotpath, argv[arg]);
      } else {
	usage();
      }

      if (++arg >= argc)
	usage();

      ++i;
      continue;
    }

    break;
  }

  if (arg >= argc)
    usage();

  size_t len = strlen(argv[arg]);
  if (len > MAX_PATH) {
    fprintf(stderr, "too long of source file path: %s\n", argv[arg]);
    return -1;
  }

  strcpy(genv.src_path, argv[arg]);

  genv.ginput = fopen(argv[arg], "r");
  if (!genv.ginput) {
    fprintf(stderr, "Open input file failed: %s, errno=%d\n", argv[1], errno);
    return -1;
  }

  if (++arg >= argc) {
    genv.outfile[0] = '\0';
    genv.goutput = stdout;
  } else {
    strcpy(genv.outfile, argv[arg]);
#if 0
    genv.goutput = fopen(argv[arg], "w+");
    if (!genv.goutput) {
      fprintf(stderr, "Open output file failed: %s, errno=%d\n", argv[arg], errno);
      return -1;
    }
#endif
  }

  genv.code_buffer = (int *)calloc(MAX_OPS, sizeof(int));
  if (!genv.code_buffer) {
    fprintf(stderr, "Allocate buffer failed, errno=%d\n", errno);
    return -1;
  }

  genv.stackbased = 1;

  return 0;
}

int main(int argc, char *argv[]) {
  if (init_config(argc, argv)) {
    fprintf(stderr, "init config failed\n");
    exit(-1);
  }

  if (yyparser_init()) {
    fprintf(stderr, "init parser failed\n");
    exit(-1);
  }

  init_llvm_env();

  yyin = genv.ginput;

  if (genv.llvm_gen_type == LGT_JIT)
    fprintf(stderr, "program `%s` :\n", genv.src_path);

  yyparse();

  walk(gtree);

  dot_finalize();

  return 0;
}

