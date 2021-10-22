
#include <alloca.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ca.h"
#include "ca.tab.h"
#include "dotgraph.h"
#include "symtable.h"

RootTree *gtree = NULL;

/* the root symbol table for global symbols and layer 0 statement running */
SymTable g_root_symtable;
SymTable *curr_symtable = NULL;

/* mainly for label processing, because label is function scope symbol */
SymTable *curr_fn_symtable = NULL;

/* flag to indicate the background type to guide inference the type of literal
   contained in the right expresssion, it has following regular for type
   inferencing:
   1. if all the operands for a operator are non-fixed literal value, it use
   the variable's type
   2. when one of the operand is the fixed literal then the other non-fixed
   literal value's type will be the fixed literal value's type. when have
   multiple different fixed type in the expression, then report error
   3. when the variable not specify a value, its type will be inferenced by the
   right value's type. The right value's type try to use the first value that
   with the fixed type as the expression's type. When the other part of the
   expression have different type then report error
 */ 
int borning_var_type = 0;
int extern_flag = 0; /* indicate if handling the extern function */
/*int call_flag = 0;  indicate if under a call statement, used for actual parameter checking */
ST_ArgList curr_arglist;

int curr_fn_rettype = 0;

extern int glineno_prev;
extern int gcolno_prev;
extern int glineno;
extern int gcolno;

void yyerror(const char *s, ...);

void check_return_type(int fnrettype) {
  if (fnrettype == VOID) {
    yyerror("line: %d, col: %d: void type function, cannot return a value",
	    glineno, gcolno);
  }
}

// TODO: check if text match the typetok, example: 'a' means char, and it cannot apply any postfix
// true, false means boolean, it cannot apply any postfix
// if postfixtypetok == -1, means only get type from littypetok or both typetok will be considered to check the error messages

// def_lit_type
// U64 stand for positive integer value in lexical
// I64 stand for positive integer value in lexical
// F64 stand for floating point number in lexical
// BOOL stand for boolean point number in lexical
// UCHAR stand for \. transfermation value in lexical
// CHAR stand for any character value in lexical

// literal type depends on the input of
// 1) littypetok: it's the literal type by itself, I64 for negative integer
// value, U64 for positive integer value, F64 for floating point value, BOOL is
// true false value, CHAR is 'x' value, UCHAR is '\x' value.
// 2) postfixtypetok: it's the literal type in the postfix of the literal, e.g.
// 43243u32 4343243.432f32 43243.343f64 -4332i64 3f64 ..., the scope or type of
// postfixtypetok must compitable with the littypetok type. e.g. when literal
// value is 4324324321433u32 then the postfixtypetok is U32, it is a bad value
// and will report an error because U32 is out of the range the literal. and
// when literal value is 43243243.343i32 it also report the error, because
// floating point literal value cannot be i32 type, but 432432f32 is right
// value.
// 3) borning_var_type: if not 0 value, it means a variable is borning
// (creating) and the variable's type is borning_var_type, it will guide the
// final literal type.
//
// The 3 parameters will affect the inference of the literal type by following
// rules:
// 1). if all the operands of an operator are non-fixed literal value
// (`lit->fixed_type == 0` or postfixtypetok is not provided (-1 value)), it
// uses the variable's type (`borning_var_type`)
// 2) when one of the operand is the fixed literal then the other non-fixed
// literal value's type will be the fixed literal value's type. when have
// multiple different fixed type in the expression, then report an error
// 3) when the variable not specify a value, the literal's type will be
// inferenced by the right ( = right-expr) expression's type. It tries to uses
// the first value that with the fixed type as the expression's type. When the
// other part of the expression have different type then report an error
// final literal's type
//
// so the final literal type should better be determined in the walk routines,
// for the first scan is hard to determine the types, because the expression may
// have multiple part and the pre part don't know the later part's type
void create_literal(CALiteral *lit, int textid, int littypetok, int postfixtypetok) {
  int typetok;
  lit->textid = textid;
  lit->littypetok = littypetok;
  lit->borning_var_type = borning_var_type;
  if (postfixtypetok == -1) {
    lit->fixed_type = 0;
    lit->postfixtypetok = 0;
  } else {
    lit->intent_type = 0;
    lit->fixed_type = 1;
    lit->postfixtypetok = postfixtypetok;

    // here can directly determine literal type, it is postfixtypetok
    determine_literal_type(lit, postfixtypetok);
  }
}

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

    if (arg.type == AT_Expr) {
      ST_ArgListActual *aa = actualarglist_current();
      aa->args[aa->argc++] = arg;
      return 0;
    }

    // TODO: remove following code
    if (arg.type == AT_Literal) {
      ST_ArgListActual *aa = actualarglist_current();
      aa->args[aa->argc++] = arg;
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
    ST_ArgListActual *aa = actualarglist_current();
    aa->args[aa->argc++] = arg;
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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    /* copy information */
    p->type = TTE_Empty;

    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_literal(CALiteral *litv) {
    ASTNode *p;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);
    /* copy information */
    p->type = TTE_Literal;
    p->litn.litv = *litv;
    p->litn.bg_type = borning_var_type;
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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);
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

#if 0
  int exprntypetok = get_expr_type_from_tree(exprn, 1);
  if (var->datatype) {
    // when the variable has a specified type: `let a: i32 = ?`
    if (exprntypetok) {
      // both side have specified type: `let a: i32 = (33 + (33i32 + 4323))`
      if (var->datatype->type != exprntypetok) {
	yyerror("line: %d, column: %d, expected `%s`, found `%s`",
	        exprn->begloc.row, exprn->begloc.col,
		get_type_string(var->datatype->type),
		get_type_string(exprntypetok));
	return NULL;
      }
    } else {
      // right side have no determined a type: `let a: i32 = 4343`
      determine_expr_type(exprn, var->datatype->type);
    }
  } else {
    // when variable type not determined yet, it means:
    // 1) the variable is in definition stage, not just assigment 
    // 2) the variable type is not specified by identifier itself
    // it should inferenced by the right value: right expression
    if (!exprntypetok) {
      // when both side have no a determined type, then determine the right side type
      exprntypetok = inference_expr_type(exprn);
    }

    if (!exprntypetok) {
      yyerror("line: %d, column: %d, inference literal type failed",
	      exprn->begloc.row, exprn->begloc.col);
      return NULL;
    }

    // when right side have a determined type
    const char *namestr = get_type_string(exprntypetok);
    int name = symname_check(namestr);
    if (name == -1) {
      yyerror("line: %d, column: %d, cannot find name for '%s'",
	      exprn->begloc.row, exprn->begloc.col, namestr);
      return NULL;
    }

    var->datatype = catype_get_by_name(name);
  }
#endif

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

static int is_valued_expr(int op) {
  return (op != ARG_LISTS && op != ARG_LISTS_ACTUAL && op != ';' && op != PRINT && op != RET);
}

int get_expr_type_from_tree(ASTNode *node, int ispost) {
  switch (node->type) {
  case TTE_Literal:
    return ispost ? node->litn.litv.postfixtypetok : node->litn.litv.datatype->type;
  case TTE_Id:
    //STEntry *entry = sym_getsym(node->symtable, node->idn.i, 1);
    if (!node->entry || node->entry->sym_type != Sym_Variable) {
      yyerror("line: %d, col: %d: the name '%s' is not a variable",
	      node->begloc.col, node->begloc.row, symname_get(node->idn.i));
      return 0;
    }

    return node->entry->u.var->datatype ? node->entry->u.var->datatype->type : 0;
  case TTE_Expr:
    return node->exprn.expr_type;
  default:
    return 0;
  }
}

const char *get_node_name_or_value(ASTNode *node) {
  switch (node->type) {
  case TTE_Literal:
    return symname_get(node->litn.litv.textid);
  case TTE_Id:
    return symname_get(node->idn.i);
  default:
    // TODO: trival the node and get the string representation of the node when not literal of id
    // walk_node_string or from lexcial analyze
    return "";
  }
}

static int parse_lexical_char(const char *text) {
  if (text[0] != '\\')
    return text[0];

  switch(text[1]) {
  case 'r':
    return '\r';
  case 'n':
    return '\n';
  case 't':
    return '\t';
  default:
    yyerror("line: %d, col: %d: unimplemented special character", glineno, gcolno);
    return -1;
  }
}

// inference and set the literal type for the literal, when the literal have no
// a determined type, different from `determine_literal_type`, the later is used by passing a defined type  
int inference_literal_type(CALiteral *lit) {
  if (lit->fixed_type) {
    // no need inference, should report an error
    return lit->datatype->type;
  }

  const char *text = symname_get(lit->textid);
  int littypetok = lit->littypetok;
  int badscope = 0;

  // handle non-fixed type literal value
  switch (littypetok) {
  case I64:
    lit->intent_type = I32;
    lit->u.i64value = atoll(text);
    badscope = check_i64_value_scope(lit->u.i64value, I32);
    break;
  case U64:
    lit->intent_type = I32;
    sscanf(text, "%lu", &lit->u.i64value);
    badscope = check_u64_value_scope((uint64_t)lit->u.i64value, I32);
    break;
  case F64:
    lit->intent_type = F64;
    badscope = check_f64_value_scope(lit->u.f64value, F64);
    lit->u.f64value = atof(text);
    break;
  case BOOL:
    lit->intent_type = BOOL;
    lit->u.i64value = atoll(text) ? 1 : 0;
    break;
  case CHAR:
    lit->intent_type = CHAR;
    lit->u.i64value = (char)parse_lexical_char(text);
    badscope = check_char_value_scope(lit->u.i64value, CHAR);
    break;
  case UCHAR:
    lit->intent_type = UCHAR;
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, UCHAR);
    break;
  default:
    yyerror("line: %d, col: %d: void type have no literal value", glineno, gcolno);
    return 0;
  }

  if (badscope) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno, get_type_string(littypetok), get_type_string(lit->intent_type));
    return 0;
  }

  const char *name = get_type_string(lit->intent_type);
  lit->datatype = catype_get_by_name(symname_check(name));
  lit->fixed_type = 1;

  return lit->intent_type;
}

// inference and set the expr type for the expr, when the expr have no a
// determined type, different from `determine_expr_type`, the later is used by
// passing a defined type
int inference_expr_type(ASTNode *p) {
  // steps, it's a recursive steps
  // 1. firstly inference the expression type, it need check if the type can conflict, and determine a type
  // 2. resolve the node type by using `determine_expr_type(exprn, type)`
  int type1 = 0;
  switch (p->type) {
  case TTE_Literal:
    type1 = inference_literal_type(&p->litn.litv);
    p->litn.litv.fixed_type = 1;
    return type1;
  case TTE_Expr:
    if (p->exprn.expr_type)
      return p->exprn.expr_type;

    for (int i = 0; i < p->exprn.noperand; ++i) {
      int type = inference_expr_type(p->exprn.operands[i]);
      if (!type1) {
	type1 = type;
      } else if (type1 != type) {
	yyerror("line: %d, column: %d, expected `%s`, found `%s`",
		p->begloc.row, p->begloc.col,
		get_type_string(type1), get_type_string(type));
	return 0;
      }
    }

    p->exprn.expr_type = type1;
    return type1;
  case TTE_Id:
    return p->entry->u.var->datatype ? p->entry->u.var->datatype->type : 0;
  default:
    yyerror("line: %d, col: %d: the expression already typed, no need to do inference",
	    glineno, gcolno);
    return 0;
  }
}

// determine and set the literal type for the literal for a specified type,
// different from `inference_literal_type` which have no a defined type
// parameter
void determine_literal_type(CALiteral *lit, int typetok) {
  if (!typetok || typetok == VOID)
    return;

  int littypetok = lit->littypetok;

  // check convertable
  if (!type_convertable(littypetok, typetok)) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno,
	    get_type_string(littypetok), get_type_string(typetok));
    return;
  }

  const char *text = symname_get(lit->textid);

  int badscope = 0;

  // check literal value scope
  switch (littypetok) {
  case I64: // I64 stand for positive integer value in lexical
    lit->u.i64value = atoll(text);
    badscope = check_i64_value_scope(lit->u.i64value, typetok);
    break;
  case U64:
    sscanf(text, "%lu", &lit->u.i64value);
    badscope = check_u64_value_scope((uint64_t)lit->u.i64value, typetok);
    break;
  case F64:
    lit->u.f64value = atof(text);
    badscope = check_f64_value_scope(lit->u.f64value, typetok);
    break;
  case BOOL:
    lit->u.i64value = atoll(text) ? 1 : 0;
    badscope = (typetok != BOOL);
    break;
  case CHAR:
    lit->u.i64value = (char)parse_lexical_char(text);
    badscope = check_char_value_scope(lit->u.i64value, typetok);
    break;
  case UCHAR:
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, typetok);
    break;
  default:
    yyerror("line: %d, col: %d: %s type have no lexical value",
	    glineno, gcolno, get_type_string(littypetok));
    break;
  }

  if (badscope) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno, get_type_string(littypetok), get_type_string(typetok));
    return;
  }

  const char *name = get_type_string(typetok);
  lit->datatype = catype_get_by_name(symname_check(name));
  lit->fixed_type = 1;
}

// determine and set the expr type for the expr for a specified type, different
// from `inference_expr_type` which have no a defined type parameter
int determine_expr_type(ASTNode *node, int typetok) {
  switch(node->type) {
  case TTE_Literal:
    if (node->litn.litv.fixed_type)
      return 0;

    // here determine the literal type in this place compare to when create literal node
    determine_literal_type(&node->litn.litv, typetok);
    break;
  case TTE_Expr:
    if (node->exprn.expr_type)
      return node->exprn.expr_type;

    for (int i = 0; i < node->exprn.noperand; ++i) {
      determine_expr_type(node->exprn.operands[i], typetok);
      node->exprn.expr_type = typetok;
    }
    break;
  case TTE_Id:
    if (!node->entry->u.var->datatype) {
      const char *name = get_type_string(typetok);
      node->entry->u.var->datatype = catype_get_by_name(symname_check(name));
    }
    break;
  default:
    // the node need not determine a type, not a literal and an expression node
    break;
  }

  return 0;
}

// reduce the node type when existing one of the determined type in the
// expression, when all part are not determined then not determine the type
// when in walk stage the assignment statement will determine the variable's
// type and the right expression's type when the expression's type not
// determined
static int reduce_node_and_type(ASTNode *p, int *expr_types, int noperands) {
  // check if exist type in the each node and type is conflicting for each node
  // but here cannot create literal value when the value not determined a type
  // because it may be a tree, or can make the type by tranverlling the tree?
  // The answer is yes, here can determine the literal type when the expression
  // exists an fixed type part
  int type1 = 0;
  int typei = 0;
  int notypeid = 0;
  int *nonfixed_node = (int *)alloca(noperands * sizeof(int));
  for (int i = 0; i < noperands; ++i) {
    if (expr_types[i]) {
      nonfixed_node[i] = 0;
      if (!type1) {
	type1 = expr_types[i];
	typei = i;
      } else if (type1 != expr_types[i]) {
	yyerror("line: %d, col: %d: type name conflicting: '%s' of type '%s', '%s' of type '%s'",
		p->begloc.col, p->begloc.row,
		get_node_name_or_value(p->exprn.operands[typei]), get_type_string(type1),
		get_node_name_or_value(p->exprn.operands[i]), get_type_string(expr_types[i]));
	return 0;
      }
    } else {
      nonfixed_node[i] = 1;
    }
  }

  if (!type1 && p->exprn.op == '=') {    
    // when cannot determine a type then handle here
    // when variable type not determined yet, it means:
    // 1) the variable is in definition stage, not just assigment 
    // 2) the variable type is not specified by identifier itself
    // it should inferenced by the right value: right expression
    // when both side have no a determined type, then determine the right side type first
    type1 = inference_expr_type(p->exprn.operands[1]);
    if (!type1) {
      yyerror("line: %d, column: %d, inference expression type failed",
	      p->begloc.row, p->begloc.col);
      return 0;
    }

    nonfixed_node[1] = 0;
  }

  // when the expression have any fixed type node
  for (int i = 0; i < noperands; ++i) {
    if (nonfixed_node[i]) {
      determine_expr_type(p->exprn.operands[i], type1);
    }
  }

  return type1;
}

// UMINUS + - * / < > GE LE NE EQ
// the left type and right type seperately calculation and interface, if the
// right side have the fixed type, or the right side will use the left side's
// type, and when the left side not have a type yet, then it will use the right
// side's type and when the right side have no fixed type then, the right side
// literal will use the literal itself's default type or intent type
ASTNode *make_expr(int op, int noperands, ...) {
    va_list ap;
    ASTNode *p;
    int i;

    int check_type = is_valued_expr(op);
    int *expr_types = check_type ? (int *)alloca(noperands * sizeof(int)) : NULL;

    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;

    // try to inference the expression type here
    p->exprn.expr_type = 0;
    va_start(ap, noperands);
    for (i = 0; i < noperands; i++) {
      p->exprn.operands[i] = va_arg(ap, ASTNode*);
    }

    int expr_len = noperands;
    int rettype = 0;
    if (check_type) {
      switch(op) {
      case FN_CALL: {
	ASTNode *idn = p->exprn.operands[0];
	STEntry *entry = sym_getsym(&g_root_symtable, idn->idn.i, 0);
	expr_types[0] = entry->u.f.rettype->type;
	expr_len = 1;
	break;
      }
	/*
      case '=': {
	expr_types[0] = p->exprn.operands[1]->entry->u.var->datatype ?
	  p->exprn.operands[1]->entry->u.var->datatype->type : 0;
	expr_types[1] = get_expr_type_from_tree(p->exprn.operands[1], 1);
	break;
      }
	*/
      default:
	for (i = 0; i < noperands; i++) {
	  expr_types[i] = get_expr_type_from_tree(p->exprn.operands[i], 1);
	}
        break;
      }

      rettype = reduce_node_and_type(p, expr_types, expr_len);
    }

    p->exprn.expr_type = rettype;

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
	yyerror("line: %d, col: %d: cannot get entry for %s\n",
		glineno, gcolno, symname_get(name));
	return NULL;
      }

      if (entry->u.var->datatype->type == VOID)
	void_count += 1;
    }

    if (noperands > 1 && void_count > 0) {
      yyerror("line: %d, col: %d: void function should only have void", glineno, gcolno);
      return NULL;
    }

    if (void_count == 1)
      return make_expr(ARG_LISTS, 0);

    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    p->exprn.expr_type = 0;
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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    p->exprn.expr_type = 0;
    for (i = 0; i < noperands; i++) {
      // TODO: remove AT_Literal AT_Variable statement
      if (al->args[i].type == AT_Expr) {
	p->exprn.operands[i] = al->args[i].exprn;
      } else if (al->args[i].type == AT_Literal) {
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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

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
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    p->ifn.ncond = ncond;
    p->ifn.isexpr = isexpr;
    if ((p->ifn.conds = malloc(ncond * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    if ((p->ifn.bodies = malloc(ncond * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

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
      if (curr_arglist.argc != entry->u.f.arglists->argc) {
	yyerror("line: %d, col: %d: function '%s' declaration not identical, see: line %d, col %d.",
		glineno, gcolno, symname_get(id), entry->sloc.row, entry->sloc.col);
	return NULL;
      }
    } else {
      entry = sym_check_insert(&g_root_symtable, id, Sym_FnDecl);
      entry->u.f.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
      *entry->u.f.arglists = curr_arglist;
    }
    entry->u.f.rettype = rettype;

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
      entry->u.f.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
      *entry->u.f.arglists = curr_arglist;
    }
    entry->u.f.rettype = rettype;

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

  // check formal parameter and actual parameter
  ST_ArgList *formalparam = entry->u.f.arglists;

  // check parameter number
  if(formalparam->contain_varg && formalparam->argc > param->exprn.noperand
     ||
     !formalparam->contain_varg && formalparam->argc != param->exprn.noperand) {
    yyerror("line: %d, col: %d: actual parameter number `%d` not match formal parameter number `%d`",
	    glineno, gcolno, param->exprn.noperand, formalparam->argc);
    return NULL;
  }

  // check and determine parameter type
  for (int i = 0; i < param->exprn.noperand; ++i) {
    STEntry *paramentry = sym_getsym(formalparam->symtable, formalparam->argnames[i], 0);
    int formaltype = paramentry->u.var->datatype->type;
    int realtype = formaltype;
    ASTNode *expr = param->exprn.operands[i]; // get one parameter

    // TODO: remove TTE_Literal and TTE_Id case for not used when using TTE_Expr
    switch(expr->type) {
    case TTE_Literal:
      determine_literal_type(&expr->litn.litv, formaltype);
      break;
    case TTE_Id: {
      // get the actual parameter type
      STEntry *identry = sym_getsym(param->symtable, expr->idn.i, 1);
      realtype = identry->u.var->datatype->type;
      break;
    }
    case TTE_Expr:
      // NEXT TODO: get parameter type here, resolve parameter type & literal value
      determine_expr_type(expr, formaltype);
      realtype = expr->exprn.expr_type;
      break;
    default:
      break;
    }

    // check the formal parameter and actual parameter type
    if (realtype != formaltype) {
      yyerror("line: %d, col: %d: the %d parameter type '%s' not match the parameter declared type '%s'",
	      param->begloc.row, param->begloc.col, i, get_type_string(realtype), get_type_string(formaltype));
      return NULL;
    }
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
    dot_init();

    if (sym_init(&g_root_symtable, NULL)) {
	yyerror("init symbol table failed\n");
    }

    curr_symtable = &g_root_symtable;
    curr_fn_symtable = &g_root_symtable;

    
    return 0;
}

