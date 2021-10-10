
#include <stdlib.h>
#include <stdarg.h>

#include "ca.h"
#include "ca.tab.h"
#include "symtable.h"

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

void yyerror(const char *s, ...);

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

int add_fn_args(SymTable *st, CAVariable *var) {
    int name = var->name;
    CADataType *datatype = var->datatype;
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
    entry->u.var = cavar_create(name, datatype);
    curr_arglist.argnames[curr_arglist.argc++] = name;
    return 0;
}

int add_fn_args_actual(SymTable *st, ActualArg arg) {
    if (curr_arglist.argc >= MAX_ARGS) {
	if (arg.type == AT_Literal)
	    yyerror("line: %d, col: %d: too many args '%d', max args support is %d",
		    glineno, gcolno, arg.litv, MAX_ARGS);
	else
	    yyerror("line: %d, col: %d: too many args '%s', max args support is %d",
		    glineno, gcolno, symname_get(arg.symnameid), MAX_ARGS);
	    
	return -1;
    }

    if (arg.type == AT_Literal) {
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

    //entry = sym_insert(st, arg.symnameid, Sym_Variable);
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

ASTNode *make_literal(CALiteral *litv) {
    ASTNode *p;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Literal;
    p->litn.litv = *litv;
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

ASTNode *make_vardef(CAVariable *var, ASTNode *exprn) {
  /* TODO: in the future realize multiple let statement in one scope */
  int id = var->name;
  STEntry *entry = sym_getsym(curr_symtable, id, 0);
  if (entry) {
    yyerror("line: %d, col: %d: symbol '%s' already defined in scope on line %d, col %d.",
	    glineno, gcolno, symname_get(id), entry->sloc.row, entry->sloc.col);
    return NULL;
  }

  entry = sym_insert(curr_symtable, id, Sym_Variable);
  entry->u.var = var;

  ASTNode *idn = make_id(id);
  idn->entry = entry;
  return make_expr('=', 2, idn, exprn);
}

ASTNode *make_assign(int id, ASTNode *exprn) {
  STEntry *entry = sym_getsym(curr_symtable, id, 1);
  if (!entry) {
    yyerror("line: %d, col: %d: symbol '%s' not defined", glineno, gcolno, symname_get(id));
    return NULL;
  }

  ASTNode *idn = make_id(id);
  idn->entry = entry;
  return make_expr('=', 2, idn, exprn);
}

ASTNode *make_goto(int labelid) {
  const char *name = symname_get(labelid);
  /* because the label name can using the same name as other names (variable, function, etc)
     so innerly represent the label name as "l:<name>", in order to distinguish them */
  const char *l = label_name(name);
  int lpos = symname_check_insert(l);
  STEntry *entry = sym_getsym(curr_fn_symtable, lpos, 0);
  if (entry) {
    switch(entry->sym_type) {
    case Sym_Label:
    case Sym_Label_Hanging:
      break;
    default:
      yyerror("line: %d, col: %d: label name '%s' appear but not aim to be a label",
	      glineno, gcolno, name);
      return NULL;
    }
  } else {
    entry = sym_insert(curr_fn_symtable, lpos, Sym_Label_Hanging);
    SLoc loc = {glineno, gcolno};
    entry->sloc = loc;
  }

  return make_expr(GOTO, 1, make_goto_node(lpos));
}

ASTNode *make_label_def(int labelid) {
  const char *name = symname_get(labelid);
  /* because the label name can using the same name as other names (variable, function, etc)
     so innerly represent the label name as "l:<name>", in order to distinguish them */
  const char *l = label_name(name);
  int lpos = symname_check_insert(l);
  STEntry *entry = sym_getsym(curr_fn_symtable, lpos, 0);
  if (entry) {
    switch(entry->sym_type) {
    case Sym_Label:
      yyerror("line: %d, col: %d: Label '%s' redefinition", glineno, gcolno, name);
      return NULL;
    case Sym_Label_Hanging:
      entry->sym_type = Sym_Label;
      break;
    default:
      yyerror("line: %d, col: %d: label name '%s' appear but not aim to be a label",
	      glineno, gcolno, name);
      return NULL;
    }
  } else {
    entry = sym_insert(curr_fn_symtable, lpos, Sym_Label);
  }

  SLoc loc = {glineno, gcolno};
  entry->sloc = loc;

  return make_label_node(lpos);
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

    int void_count = 0;
    for (i = 0; i < noperands; ++i) {
      int name = al->argnames[i];
      STEntry *entry = sym_getsym(curr_symtable, name, 0);
      if (!entry) {
	yyerror("cannot get entry for %s\n", symname_get(name));
	return NULL;
      }

      if (entry->u.var->datatype->type == VOID)
	void_count += 1;
    }

    if (noperands > 1 && void_count > 0) {
      yyerror("void function should only have void");
      return NULL;
    }

    if (void_count == 1)
      return make_expr(ARG_LISTS, 0);

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
      if (al->args[i].type == AT_Literal) {
	p->exprn.operands[i] = make_literal(&al->args[i].litv);
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

ASTNode *make_label_node(int i) {
    ASTNode *p = make_id(i);
    if (p) {
	p->type = TTE_Label;
	return p;
    }

    return NULL;
}

ASTNode *make_goto_node(int i) {
    ASTNode *p = make_id(i);
    if (p) {
	p->type = TTE_LabelGoto;
	return p;
    }

    return NULL;
}

ASTNode *make_fn_decl(int name, ST_ArgList *al, CADataType *rettype, SLoc beg, SLoc end) {
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_FnDecl;
    p->fndecln.ret = rettype;
    p->fndecln.name = name;
    p->fndecln.args = *al;
    p->fndecln.is_extern = 0; // TODO: make extern real extern

    set_address(p, &beg, &end);
    return p;
}

ASTNode *make_fn_define(int name, ST_ArgList *al, CADataType *rettype, SLoc beg, SLoc end) {
    ASTNode *decl = make_fn_decl(name, al, rettype, beg, end);
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

ASTNode *make_fn_proto(int id, CADataType *rettype) {
  SLoc beg = {glineno, gcolno};
  SLoc end = {glineno, gcolno};

  STEntry *entry = sym_getsym(&g_root_symtable, id, 0);
  if (extern_flag) {
    if (entry) {
      /* check if function declaration is the same */
      if (curr_arglist.argc != entry->u.arglists->argc) {
	yyerror("line: %d, col: %d: function '%s' declaration not identical, see: line %d, col %d.",
		glineno, gcolno, symname_get(id), entry->sloc.row, entry->sloc.col);
	return NULL;
      }
    } else {
      entry = sym_check_insert(&g_root_symtable, id, Sym_FnDecl);
      entry->u.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
      *entry->u.arglists = curr_arglist;
    }

    return make_fn_decl(id, &curr_arglist, rettype, beg, end);
  } else {
    if (entry) {
      if (entry->sym_type == Sym_FnDef) {
	yyerror("line: %d, col: %d: function '%s' already defined on line %d, col %d.",
		glineno, gcolno, symname_get(id), entry->sloc.row, entry->sloc.col);
	return NULL;
      }

      if (entry->sym_type == Sym_FnDecl) {
	entry->sym_type = Sym_FnDef;
	SLoc loc = {glineno, gcolno};
	entry->sloc = loc;
      }
    } else {
      entry = sym_check_insert(&g_root_symtable, id, Sym_FnDef);
      entry->u.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
      *entry->u.arglists = curr_arglist;
    }

    return make_fn_define(id, &curr_arglist, rettype, beg, end);
  }
}

ASTNode *make_fn_call(int fnname, ASTNode *param) {
  STEntry *entry = sym_getsym(&g_root_symtable, fnname, 0);
  if (!entry) {
    yyerror("line: %d, col: %d: function '%s' not defined", glineno, gcolno, symname_get(fnname));
    return NULL;
  }

  if (entry->sym_type != Sym_FnDecl && entry->sym_type != Sym_FnDef) {
    yyerror("line: %d, col: %d: '%s' is not a function", glineno, gcolno, symname_get(fnname));
    return NULL;
  }

  return make_expr(FN_CALL, 2, make_id(fnname), param);
}

ASTNode *make_ident_expr(int id) {
  STEntry *entry = sym_getsym(curr_symtable, id, 1);
  if (!entry) {
    yyerror("line: %d, col: %d: Variable name '%s' not defined", glineno, gcolno, symname_get(id));
    return NULL;
  }

  ASTNode *node = make_id(id);
  node->entry = entry;
  return node;
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
    catype_init();

    if (sym_init(&g_root_symtable, NULL)) {
	yyerror("init symbol table failed\n");
    }

    curr_symtable = &g_root_symtable;
    curr_fn_symtable = &g_root_symtable;

    
    return 0;
}

