
#include <stdlib.h>
#include <stdarg.h>

#include "ca.h"
#include "ca.tab.h"

RootTree *gtree = NULL;

/* the root symbol table for global symbols and layer 0 statement running */
SymTable g_root_symtable;
SymTable *curr_symtable = NULL;

/* mainly for label processing, because label is function scope symbol */
SymTable *curr_fn_symtable = NULL;

int extern_flag = 0; /* indicate if handling the extern function */
/*int call_flag = 0;  indicate if under a call statement, used for real parameter checking */
ST_ArgList curr_arglist;
ST_ArgListActual curr_arglistactual;

extern int glineno_prev;
extern int gcolno_prev;
extern int glineno;
extern int gcolno;

SymTable *push_new_symtable() {
    SymTable *st = (SymTable *)malloc(sizeof(SymTable));
    sym_init(st, curr_symtable);
    curr_symtable = st;
    return st;
}

SymTable *pop_symtable() {
  if (curr_symtable != &g_root_symtable)
    curr_symtable = curr_symtable->parent;

  return curr_symtable;
}

int add_fn_args(SymTable *st, int name) {
    if (curr_arglist.argc >= MAX_ARGS) {
	yyerror("line: %d, col: %d: too many args '%s', max args support is %d",
		glineno, gcolno, symname_get(name), MAX_ARGS);
	return -1;
    }

    STEntry *entry = sym_getsym(st, name, 0);
    if (entry) {
	yyerror("line: %d, col: %d: parameter '%s' already defined on line %d, col %d.",
		glineno, gcolno, symname_get(name), entry->sloc.row, entry->sloc.col);
	return -1;
    }

    entry = sym_insert(st, name, Sym_Variable);
    curr_arglist.argnames[curr_arglist.argc++] = name;
    return 0;
}

int add_fn_args_actual(SymTable *st, ActualArg arg) {
    if (curr_arglist.argc >= MAX_ARGS) {
	if (arg.type == AT_LITERAL)
	    yyerror("line: %d, col: %d: too many args '%d', max args support is %d",
		    glineno, gcolno, arg.litv, MAX_ARGS);
	else
	    yyerror("line: %d, col: %d: too many args '%s', max args support is %d",
		    glineno, gcolno, symname_get(arg.symnameid), MAX_ARGS);
	    
	return -1;
    }

    if (arg.type == AT_LITERAL) {
	curr_arglistactual.args[curr_arglistactual.argc++] = arg;
	return 0;
    }

    STEntry *entry = sym_getsym(st, arg.symnameid, 0);
    if (!entry) {
	yyerror("line: %d, col: %d: parameter '%s' note defined",
		glineno, gcolno, symname_get(arg.symnameid));
	return -1;
    }

    arg.entry = entry;

    entry = sym_insert(st, arg.symnameid, Sym_Variable);
    curr_arglistactual.args[curr_arglistactual.argc++] = arg;
    return 0;
}

const char *label_name(const char *name) {
    static char label_buf[1024];
    sprintf(label_buf, "l:%s", name);
    return label_buf;
}

void set_address(ASTNode *node, const SLoc *first, const SLoc *last) {
    node->begloc = *first;
    node->endloc = *last;
    node->symtable = curr_symtable;
}

ASTNode *make_empty() {
    ASTNode *p;
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_Empty;

    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_lit(int value) {
    ASTNode *p;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Literal;
    p->litn.value = value;
    p->begloc.row = glineno_prev;
    p->begloc.col = gcolno_prev;
    p->endloc.row = glineno;
    p->endloc.col = gcolno;

    return p;
}

ASTNode *make_id(int i) {
    ASTNode *p;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Id;
    p->idn.i = i;

    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_expr(int op, int noperands, ...) {
    va_list ap;
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    va_start(ap, noperands);
    for (i = 0; i < noperands; i++)
	p->exprn.operands[i] = va_arg(ap, ASTNode*);

    const SLoc *beg = &(SLoc){glineno, gcolno};
    const SLoc *end = &(SLoc){glineno, gcolno};

    if (noperands == 1) {
	if (p->exprn.operands[0]) {
	    beg = &p->exprn.operands[0]->begloc;
	    end = &p->exprn.operands[0]->endloc;
	}
    } else if (noperands > 1){
	if (p->exprn.operands[0]) {
	    beg = &p->exprn.operands[0]->begloc;
	}

	if (p->exprn.operands[noperands-1]) {
	    end = &p->exprn.operands[noperands-1]->endloc;
	}
    }

    p->begloc = *beg;
    p->endloc = *end;

    va_end(ap);
    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_expr_arglists(ST_ArgList *al) {
    int noperands = al->argc;
    int op = ARG_LISTS;
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    for (i = 0; i < noperands; i++)
	p->exprn.operands[i] = make_id(al->argnames[i]);

    if (noperands == 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[0]->endloc;
    } else if (noperands > 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[noperands-1]->endloc;
    } else {
	p->begloc = (SLoc){glineno, gcolno};
	p->endloc = (SLoc){glineno, gcolno};
    }

    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_expr_arglists_actual(ST_ArgListActual *al) {
    int noperands = al->argc;
    int op = ARG_LISTS_ACTUAL;
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    for (i = 0; i < noperands; i++) {
      if (al->args[i].type == AT_LITERAL) {
	p->exprn.operands[i] = make_lit(al->args[i].litv);
	p->exprn.operands[i]->entry = NULL;
      } else {
	p->exprn.operands[i] = make_id(al->args[i].symnameid);
	p->exprn.operands[i]->entry = al->args[i].entry;
      }
    }

    if (noperands == 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[0]->endloc;
    } else if (noperands > 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[noperands-1]->endloc;
    } else {
	p->begloc = (SLoc){glineno, gcolno};
	p->endloc = (SLoc){glineno, gcolno};
    }

    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_label(int i) {
    ASTNode *p = make_id(i);
    if (p) {
	p->type = TTE_Label;
	return p;
    }

    return NULL;
}

ASTNode *make_goto_label(int i) {
    ASTNode *p = make_id(i);
    if (p) {
	p->type = TTE_LabelGoto;
	return p;
    }

    return NULL;
}

ASTNode *make_fn_decl(int name, ST_ArgList *al, SLoc beg, SLoc end) {
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_FnDecl;
    p->fndecln.ret = 0;
    p->fndecln.name = name;
    p->fndecln.args = *al;
    p->fndecln.is_extern = 0; // TODO: make extern real extern

    set_address(p, &beg, &end);
    return p;
}

ASTNode *make_fn_define(int name, ST_ArgList *al, SLoc beg, SLoc end) {
    ASTNode *decl = make_fn_decl(name, al, beg, end);
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_FnDef;
    p->fndefn.fn_decl = decl;
    p->fndefn.stmts = NULL;

    set_address(p, &beg, &end);
    return p;
}

ASTNode *make_while(ASTNode *cond, ASTNode *whilebody) {
    ASTNode *p;

    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_While;
    p->whilen.cond = cond;
    p->whilen.body = whilebody;

    set_address(p, &cond->begloc, &whilebody->endloc);
    return p;
}

ASTNode *make_if(int isexpr, int argc, ...) {
    // TODO: implement multiple if else nodes
    va_list ap;
    ASTNode *p;

    int ncond = argc / 2;
    int remainder = argc % 2;

    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    p->ifn.ncond = ncond;
    p->ifn.isexpr = isexpr;
    if ((p->ifn.conds = malloc(ncond * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    if ((p->ifn.bodies = malloc(ncond * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_If;

    va_start(ap, argc);
    for (int i = 0; i < ncond; i++) {
	p->ifn.conds[i] = va_arg(ap, ASTNode*);
	p->ifn.bodies[i] = va_arg(ap, ASTNode*);
    }

    p->ifn.remain = remainder ? va_arg(ap, ASTNode*) : NULL;
    va_end(ap);

    set_address(p, &p->ifn.conds[0]->begloc,
		remainder ? &p->ifn.remain->endloc : &p->ifn.bodies[ncond-1]->endloc);
    return p;
}

void freeNode(ASTNode *p) {
    int i;
    if (!p) return;
    if (p->type == TTE_Expr) {
	for (i = 0; i < p->exprn.noperand; i++)
	    freeNode(p->exprn.operands[i]);
	free(p->exprn.operands);
    }
    free (p);
}

NodeChain *node_chain(RootTree *tree, ASTNode *p) {
  static int is_main_start_set = 0;
  switch (p->type) {
  case TTE_Literal:
  case TTE_LabelGoto:
  case TTE_Expr:
      if (!is_main_start_set) {
	  gtree->begloc_main = p->begloc;
	  is_main_start_set = 1;
      }
      gtree->endloc_main = p->endloc;
  default:
      gtree->endloc_prog = p->endloc;
      break;
  }

  NodeChain *node = (NodeChain *)calloc(1, sizeof(NodeChain));
  node->node = p;

  if (!tree->head) {
    tree->head = tree->tail = node;
    tree->count = 1;
    return node;
  }

  tree->tail->next = node;
  tree->tail = node;
  tree->count ++;
  return node;
}

void yyerror(const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  int n = vfprintf(stderr, s, ap);
  va_end(ap);

  fprintf(stderr, "\n");
  exit(-1);
}

int yyparser_init() {
    gtree = (RootTree *)calloc(1, sizeof(RootTree));
    if (!gtree) {
	yyerror("init root tree failed\n");
    }

    symname_init();
    lexical_init();

    if (sym_init(&g_root_symtable, NULL)) {
	yyerror("init symbol table failed\n");
    }

    curr_symtable = &g_root_symtable;
    curr_fn_symtable = &g_root_symtable;

    
    return 0;
}

