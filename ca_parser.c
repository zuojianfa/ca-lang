
#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "ca.h"
#include "ca.tab.h"
#include "ca_types.h"
#include "dotgraph.h"
#include "symtable.h"
#include "config.h"
#include "type_system.h"

/* how to handle the type backward definition, means can use the type before
   defining it:
   1. define an unknown type table
   2. when use the type
     a) when found in symbol table
        just get and use the type
     b) when not found the type in symbol table
       1) when found in unknown type table
          get and use it, but it is undefined
       2) when not found in unknown type table
          create an unknown type, put it into unknown type table and use it
   3. when defining the type
     a) when found in symbol table
        report error
     b) when not found in symbol table
       1) when found in unknown type table
          get the type object and define it using the object memory and remove
	  it from the unknown type table
       2) when not found in unknown type table
          create the object and define it
       3) put the new type object into symbol table
   4. when walk the tree
      check the variable type inference and determine the type
*/

RootTree *gtree = NULL;

/* the root symbol table for global symbols and layer 0 statement running */
SymTable g_root_symtable;

ASTNode *main_fn_node = NULL;

// the generated (when use `-main` option) main function symbol table
SymTable *g_main_symtable = NULL;

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
int extern_flag = 0; /* indicate if handling the extern function */
/*int call_flag = 0;  indicate if under a call statement, used for actual parameter checking */
ST_ArgList curr_arglist;

typeid_t curr_fn_rettype = 0;
int g_node_seqno = 0;

extern int glineno_prev;
extern int gcolno_prev;
extern int glineno;
extern int gcolno;
extern int yychar, yylineno;

int walk(RootTree *tree);

int enable_emit_main() { return genv.emit_main; }

typedef enum OverloadType {
  OLT_Label,
  OLT_Type,
  OLT_Struct,
} OverloadType;

ASTNode *new_ASTNode(ASTNodeType nodetype) {
  ASTNode *p;

  /* allocate node */
  if ((p = malloc(sizeof(ASTNode))) == NULL)
    yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

  p->seq = ++g_node_seqno;
  p->type = nodetype;
  p->grammartype = NGT_None;

  return p;
}

const char *sym_form_label_name(const char *name) {
  // TODO: the buffer need reimplement
  static char label_buf[1024];
  sprintf(label_buf, "l:%s", name);
  return label_buf;
}

const char *sym_form_type_name(const char *name) {
  // TODO: the buffer need reimplement
  static char type_buf[1024];
  sprintf(type_buf, "t:%s", name);
  return type_buf;
}

const char *sym_form_function_name(const char *name) {
  // TODO: the buffer need reimplement
  static char type_buf[1024];
  sprintf(type_buf, "f:%s", name);
  return type_buf;
}

const char *sym_form_pointer_name(const char *name) {
  // TODO: the buffer need reimplement
  static char type_buf[1024];
  sprintf(type_buf, "t:*%s", name);
  return type_buf;
}

const char *sym_form_array_name(const char *name, int dimension) {
  // TODO: the buffer need reimplement
  static char type_buf[1024];
  sprintf(type_buf, "t:[%s;%d]", name, dimension);
  return type_buf;
}

typeid_t sym_form_expr_typeof_id(ASTNode *expr) {
  // TODO: the buffer need reimplement
  char type_buf[32];
  sprintf(type_buf, "+:%p", expr);
  typeid_t id = sym_form_type_id_by_str(type_buf);
  return id;
}

ASTNode *astnode_unwind_from_addr(const char *addr, int *len) {
  ASTNode *expr = NULL;
  //*len = sscanf(addr, "+:%p", &expr);
  char *end = NULL;
  long v = strtol(addr+2, &end, 16);
  *len = end - addr;
  expr = (ASTNode *)v;
  return expr;
}

// id -> (t:)id or (l:)id
typeid_t sym_form_type_id(int id) {
  //const char *name = symname_get(id);
  const char *name = get_inner_type_string(id);
  
  const char *typename = sym_form_type_name(name);
  typeid_t typeid = symname_check_insert(typename);
  return typeid;
}

typeid_t sym_form_type_id_by_str(const char *idname) {
  const char *typename = sym_form_type_name(idname);
  typeid_t typeid = symname_check_insert(typename);
  return typeid;
}

typeid_t sym_form_label_id(int id) {
  const char *name = symname_get(id);
  const char *typename = sym_form_label_name(name);
  typeid_t labelid = symname_check_insert(typename);
  return labelid;
}

typeid_t sym_form_function_id(int fnid) {
  const char *name = symname_get(fnid);
  const char *typename = sym_form_function_name(name);
  typeid_t typeid = symname_check_insert(typename);
  return typeid;
}

typeid_t sym_form_pointer_id(typeid_t type) {
  const char *name = catype_get_type_name(type);
  const char *typename = sym_form_pointer_name(name);
  typeid_t typeid = symname_check_insert(typename);
  return typeid;  
}

typeid_t sym_form_array_id(typeid_t type, int dimension) {
  const char *name = catype_get_type_name(type);
  const char *typename = sym_form_array_name(name, dimension);
  typeid_t typeid = symname_check_insert(typename);
  return typeid;
}

typeid_t sym_form_tuple_id(typeid_t *types, int argc) {
  // format: t:(;), t:(;i32), t:(;i32, bool), t:(;i32, (;i32, i32)), t:(;(;i32, i32,), i32), ...
  void *hs = string_new();
  string_append(hs, "t:(;");
  for (int i = 0; i < argc; ++i) {
    const char *name = catype_get_type_name(types[i]);
    string_append(hs, name);
    string_append(hs, ",");
  }

  if (argc) {
    // remove tailing ','
    string_pop_back(hs);
  }

  string_append(hs, ")");

  const char *typestr = string_c_str(hs);
  typeid_t typeid = symname_check_insert(typestr);
  string_drop(hs);

  return typeid;
}

const char *sym_form_struct_signature(const char *name, SymTable *st) {
  static char name_buf[1024];
  sprintf(name_buf, "%s@%p", name, st);
  return name_buf;
}

typeid_t sym_form_symtable_type_id(SymTable *st, typeid_t name) {
  static char name_buf[1024];
  const char *chname = catype_get_type_name(name);
  sprintf(name_buf, "%s@%p", chname, st);
  return symname_check_insert(name_buf);
}

void set_address(ASTNode *node, const SLoc *first, const SLoc *last) {
    node->begloc = *first;
    node->endloc = *last;
    node->symtable = curr_symtable;
}

int make_program() {
  dot_emit("program", "paragraphs");
  gtree->root_symtable = &g_root_symtable;
  return 0;
}

void make_paragraphs(ASTNode *paragraph) {
  dot_emit("paragraphs", "paragraphs paragraph");
  node_chain(gtree, paragraph);
}

ASTNode *make_fn_def(ASTNode *proto, ASTNode *body) {
  dot_emit("fn_def", "fn_proto fn_body");

  // fix the lexical body with function body for later use (in stage 2 parse)
  assert(body->type == TTE_LexicalBody);
  body->lnoden.fnbuddy = proto;

  proto->fndefn.stmts = body;
  proto->endloc.row = glineno;
  proto->endloc.col = gcolno;
  pop_symtable();

  curr_fn_rettype = 0;
  curr_fn_symtable = NULL;

  if (enable_emit_main()) {
    // push generated main function, current will be the main symbol table
    push_symtable(g_main_symtable);
    curr_fn_symtable = g_main_symtable;
  }

  return proto;
}

ASTNode *make_fn_body(ASTNode *blockbody) {
  dot_emit("fn_body", "block_body");
  return blockbody;
}

ASTNode *make_fn_decl(ASTNode *proto) {
  dot_emit("fn_decl", "EXTERN fn_proto");
  pop_symtable();
  extern_flag = 0;
  curr_fn_rettype = 0;

  if (enable_emit_main()) {
    // push generated main function, current will be the main symbol table
    push_symtable(g_main_symtable);
    curr_fn_symtable = g_main_symtable;
  }

  return proto;
}

static void check_expr_arglists(ST_ArgList *al) {
    int noperands = al->argc;
    ASTNode *p;
    int i;

    // for checking void type function, can only have void parameter
    int void_count = 0;
    for (i = 0; i < noperands; ++i) {
      int name = al->argnames[i];
      STEntry *entry = sym_getsym(curr_symtable, name, 0);
      if (!entry) {
	yyerror("line: %d, col: %d: cannot get entry for %s\n",
		glineno, gcolno, symname_get(name));
	return;
      }

      if (entry->u.var->datatype == sym_form_type_id_from_token(VOID))
	void_count += 1;
    }

    if (noperands > 1 && void_count > 0) {
      yyerror("line: %d, col: %d: void function should only have void", glineno, gcolno);
      return;
    }
}

void add_fn_args_p(ST_ArgList *arglist, int varg) {
  if (varg)
    dot_emit("fn_args", "fn_args_p VARG");
  else
    dot_emit("fn_args", "fn_args_p");

  arglist->contain_varg = varg;
  check_expr_arglists(arglist);
}

ASTNode *make_stmt_print(ASTNode *expr) {
  ASTNode *p = new_ASTNode(TTE_DbgPrint);
  p->printn.expr = expr;
  set_address(p, &expr->begloc, &expr->endloc);
  return p;
}

ASTNode *make_stmt_print_datatype(typeid_t tid) {
  ASTNode *p = new_ASTNode(TTE_DbgPrintType);
  p->printtypen.type = tid;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;  
}

ASTNode *make_stmt_expr(ASTNode *expr) {
  dot_emit("stmt", "expr");
  expr->grammartype = NGT_stmt_expr;
  return expr;
}

ASTNode *make_stmt_ret_expr(ASTNode *expr) {
  if (!curr_fn_rettype && genv.emit_main) {
    // when not in a function and emit main provided, then use int as the rettype
    curr_fn_rettype = sym_form_type_id_from_token(I32);
  }

  //check_return_type(curr_fn_rettype);

  ASTNode *p = new_ASTNode(TTE_Ret);
  p->retn.expr = expr;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

ASTNode *make_stmt_ret() {
  /* if (curr_fn_rettype != sym_form_type_id_from_token(VOID)) */
  /*   yyerror("line: %d, col: %d: function have no return type", glineno, gcolno); */

  ASTNode *p = new_ASTNode(TTE_Ret);
  p->retn.expr = NULL;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

ASTNode *make_stmtexpr_list_block(ASTNode *exprblockbody) {
  dot_emit("stmtexpr_list_block", "exprblock_body");
  // or
  dot_emit("stmt_list_block", "block_body");

  SymTable *st = pop_symtable();
  return exprblockbody;
}

ASTNode *make_stmtexpr_list(ASTNode *stmts, ASTNode *expr) {
  dot_emit("stmtexpr_list", "stmt_list expr");
  // or
  dot_emit("stmtexpr_list", "expr");
  
  ASTNode *node = make_expr(STMT_EXPR, 2, stmts, expr);
  return node;
}

ASTNode *make_lexical_body(ASTNode *stmts) {
  ASTNode *node = new_ASTNode(TTE_LexicalBody);
  node->lnoden.stmts = stmts;
  node->lnoden.fnbuddy = NULL;
  set_address(node, &stmts->begloc, &stmts->endloc);
  return node;
}

typeid_t make_pointer_type(typeid_t type) {
  return sym_form_pointer_id(type);
  /* return catype_make_pointer_type(type); */
}

typeid_t make_array_type(typeid_t type, LitBuffer *size) {
  if (size->typetok != U64) {
    yyerror("line: %d, col: %d: array size not usize (u64) type, but `%s` type",
	    glineno, gcolno, get_type_string(size->typetok));
    return typeid_novalue;
  }

  const char *text = symname_get(size->text);
  if (!text) {
    yyerror("line: %d, col: %d: get literal size failed", glineno, gcolno);
    return typeid_novalue;
  }

  uint64_t len;
  sscanf(text, "%lu", &len);
  return sym_form_array_id(type, (int)len);
  /*return catype_make_array_type(type, len);*/
}

typeid_t make_tuple_type(ST_ArgList *arglist) {
  // t:(;), t:(;i32), t:(;i32, bool), t:(;i32, (;i32, i32)), t:(;(;i32, i32,), i32), ...
  typeid_t id = typeid_novalue;
  if (arglist->argc == 1)
    id = arglist->types[0];
  else
    id = sym_form_tuple_id(arglist->types, arglist->argc);
  
  return id;
}

ASTNode *make_type_def(int id, typeid_t type) {
  // make it can have the same name for the type name and variable name
  // implemented just like the label type: add a prefix before the type name
  typeid_t newtype = sym_form_type_id(id);
  CADataType *primtype = catype_get_primitive_by_name(newtype);
  if (primtype) {
    yyerror("line: %d, col: %d: type alias id `%s` cannot be primitive type",
	    glineno, gcolno, symname_get(id));
    return NULL;
  }

  STEntry *entry = sym_getsym(curr_symtable, newtype, 0);
  if (entry) {
    yyerror("line: %d, col: %d: type `%s` defined multiple times",
	    glineno, gcolno, symname_get(id));
    return NULL;
  }

  entry = sym_insert(curr_symtable, newtype, Sym_DataType);
  entry->u.datatype.id = type;
  entry->u.datatype.idtable = curr_symtable;
  entry->u.datatype.members = NULL;
  SLoc loc = {glineno, gcolno};
  entry->sloc = loc;

  ASTNode *p = new_ASTNode(TTE_TypeDef);
  p->typedefn.newtype = newtype;
  p->typedefn.type = type;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

typeid_t make_ret_type_void() {
  dot_emit("ret_type", "");
  typeid_t typesym = sym_form_type_id_from_token(VOID);
  return typesym;
}

void make_type_postfix(IdToken *idt, int id, int typetok) {
  dot_emit("type_postfix", get_type_string(typetok));
  idt->symnameid = id;
  idt->typetok = typetok;
}

void check_return_type(typeid_t fnrettype) {
  if (fnrettype == sym_form_type_id_from_token(VOID)) {
    yyerror("line: %d, col: %d: void type function, cannot return a value",
	    glineno, gcolno);
  }
}

// TODO: check if text match the typetok, example: 'a' means char, and it cannot apply any postfix
// true, false means boolean, it cannot apply any postfix
// if postfixtypetok == -1, means only get type from littypetok or both typetok will be considered to check the error messages

// U64 stand for positive integer value in lexical
// I64 stand for positive integer value in lexical
// F64 stand for floating point number in lexical
// BOOL stand for boolean point number in lexical
// U8 stand for \. transfermation value in lexical
// I8 stand for any character value in lexical

// literal type depends on the input of
// 1) littypetok: it's the literal type by itself, I64 for negative integer
// value, U64 for positive integer value, F64 for floating point value, BOOL is
// true false value, I8 is 'x' value, U8 is '\x' value.
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
void create_literal(CALiteral *lit, int textid, tokenid_t littypetok, tokenid_t postfixtypetok) {
  lit->textid = textid;
  lit->littypetok = littypetok;
  lit->postfixtypetok = postfixtypetok;
  lit->fixed_type = 0;
  lit->datatype = typeid_novalue;
}

void create_string_literal(CALiteral *lit, const LitBuffer *litb) {
  lit->fixed_type = 1;
  lit->littypetok = litb->typetok; // litb->typetok should be CSTRING;
  lit->textid = litb->text;
  lit->datatype = sym_form_type_id_from_token(litb->typetok);
  lit->u.strvalue.text = litb->text;
  lit->u.strvalue.len = litb->len;
}

SymTable *push_new_symtable() {
    SymTable *st = (SymTable *)malloc(sizeof(SymTable));
    sym_init(st, curr_symtable);
    curr_symtable = st;
    return st;
}

SymTable *push_symtable(SymTable *st) {
    st->parent = curr_symtable;
    curr_symtable = st;
    return st;
}

SymTable *pop_symtable() {
  if (curr_symtable != &g_root_symtable)
    curr_symtable = curr_symtable->parent;

  return curr_symtable;
}

int add_fn_args(ST_ArgList *arglist, SymTable *st, CAVariable *var) {
  dot_emit("fn_args_p", "fn_args_p ',' iddef_typed");
  // or
  dot_emit("fn_args_p", "iddef_typed"); 
  
    int name = var->name;
    if (arglist->argc >= MAX_ARGS) {
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
    entry->u.var = cavar_create(name, var->datatype);
    arglist->argnames[arglist->argc++] = name;
    return 0;
}

int add_fn_args_actual(SymTable *st, ASTNode *arg) {
  dot_emit("fn_args_call_p", "fn_args_call_p ',' fn_args_actual");
  // or
  dot_emit("fn_args_call_p", "fn_args_actual");
  
  ST_ArgListActual *aa = actualarglist_current();
  if (aa->argc >= MAX_ARGS) {
    // TODO: how to output the expression's value or name (I know the literal and
    // single variable can get the name) in order to support this, it should use
    // the full text functionality, such as according to line column number to get
    // the text from the source file
    yyerror("line: %d, col: %d: too many args '%s', max args support is %d",
	    glineno, gcolno, "todo:get the args text :)", MAX_ARGS);
	    
    return -1;
  }

  // arg.type must be == AT_Expr
  aa->args[aa->argc++] = arg;
  return 0;
}

ASTNode *make_empty() {
    dot_emit("stmt_list_star", "");

    ASTNode *p = new_ASTNode(TTE_Empty);
    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_literal(CALiteral *litv) {
    dot_emit("expr", "literal");

    ASTNode *p = new_ASTNode(TTE_Literal);
    p->litn.litv = *litv;
    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});

    return p;
}

ASTNode *make_array_def(CAArrayExpr expr) {
  ASTNode *p = new_ASTNode(TTE_ArrayDef);
  p->anoden.aexpr = expr;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  ASTNode *node = make_expr(ARRAY, 1, p);
  return node;
}

ASTNode *make_struct_expr(CAStructExpr expr) {
  ASTNode *p = new_ASTNode(TTE_StructExpr);
  p->snoden = expr;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  ASTNode *node = make_expr(STRUCT, 1, p);
  return node;
}

ASTNode *make_tuple_expr(CAStructExpr expr) {
  // not used currently, the tuple expression now use the form of function call
  // because the form of named tuple definition is the same as function call
  return NULL;
}

ASTNode *make_arrayitem_right(ArrayItem ai) {
  ASTNode *p = new_ASTNode(TTE_ArrayItemRight);
  p->aitemn = ai;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  ASTNode *node = make_expr(ARRAYITEM, 1, p);
  return node;
}

ASTNode *make_structfield_right(StructFieldOp sfop) {
  ASTNode *p = new_ASTNode(TTE_StructFieldOpRight);
  p->sfopn = sfop;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  ASTNode *node = make_expr(STRUCTITEM, 1, p);
  return node;
}

ASTNode *make_boxed_expr(ASTNode *expr) {
  ASTNode *p = new_ASTNode(TTE_Box);
  p->boxn.expr = expr;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  ASTNode *node = make_expr(BOX, 1, p);
  return node;
}

ASTNode *make_drop(int id) {
  ASTNode *p = new_ASTNode(TTE_Drop);
  p->dropn.var = id;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

ASTNode *make_id(int i, IdType idtype) {
    ASTNode *p = new_ASTNode(TTE_Id);
    p->idn.i = i;
    p->idn.idtype = idtype;

    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_deref_left(DerefLeft deleft) {
  ASTNode *p = new_ASTNode(TTE_DerefLeft);
  p->deleftn = deleft;

  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p; 
}

int make_attrib_scope(int attrfn, int attrparam) {
  const char *scope = symname_get(attrfn);
  const char *global = symname_get(attrparam);
  if (strcmp(scope, "scope")) {
    yyerror("line: %d, col: %d: attribute function here only support `scope`",
	    glineno, gcolno);
    return -1;
  }

  if (strcmp(global, "global") && strcmp(global, "local")) {
    yyerror("line: %d, col: %d: attribute function only support `scope`",
	    glineno, gcolno);
    return -1;
  }

  return attrparam;
}

void register_variable(CAVariable *cavar, SymTable *symtable) {
  STEntry *entry = sym_getsym(symtable, cavar->name, 0);
  if (entry) {
    yyerror("line: %d, col: %d: symbol '%s' already defined in scope on line %d, col %d.",
	    glineno, gcolno, symname_get(cavar->name), entry->sloc.row, entry->sloc.col);
    return;
  }

  entry = sym_insert(symtable, cavar->name, Sym_Variable);
  entry->u.var = cavar;
}

ASTNode *make_global_vardef(CAVariable *var, ASTNode *exprn, int global) {
  dot_emit("stmt", "vardef");

  /* TODO: in the future realize multiple let statement in one scope */

  SymTable *symtable = curr_symtable;
  var->global = 0;

  // curr_symtable == g_main_symtable` already include the judgement
  if (enable_emit_main()) {
    // only take effect when `-main` used to generate main function
    var->global = global;

    // it is in generated main function scope, and `#[scope(global)]` is provided
    if (curr_symtable == g_main_symtable && var->global) {
      // generate a global variable, use global symbol table here
      symtable = &g_root_symtable;

      // TODO: FIXME, there are a bugs when declare the global variable with
      // attribute, because when right expression is complex and/or is not in
      // global scope (because when using generated main function the declared
      // variable (in global scope) will be put into main function scope and the
      // declared variable becoming non-global variable) then there be some bugs
      // here just reassign the symtable using global table to fix it, but still
      // have some bugs      
      exprn->symtable = symtable;
    }
    // all else generate local variable against main or defined function
  }

  register_variable(var, symtable);

  CAPattern *cap = capattern_new(var->name, PT_Var, NULL);
  cap->datatype = var->datatype;

  ASTNode *p = new_ASTNode(TTE_LetBind);
  p->letbindn.cap = cap;
  p->letbindn.expr = exprn;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &exprn->endloc);
  p->symtable = symtable;
  return p;
}

void capattern_register_variable(int name, typeid_t datatype, SLoc *loc) {
  CAVariable *cavar = cavar_create_with_loc(name, datatype, loc);
  register_variable(cavar, curr_symtable);
}

void register_capattern_symtable(CAPattern *cap, SLoc *loc);
void register_structpattern_symtable(CAPattern *cap, int withname, int withsub, SLoc *loc) {
  typeid_t type = cap->datatype;
  if (withname) {
    type = sym_form_type_id(cap->name);
    if (cap->datatype != typeid_novalue && cap->datatype != type) {
      yyerror("line: %d, col: %d: left `%s` type not match right `%s`",
	      glineno, gcolno, symname_get(cap->name), catype_get_type_name(cap->datatype));
      return;
    }
  }

  if (!withname && !withsub) {
    capattern_register_variable(cap->name, type, loc);
  }

  size_t size = vec_size(cap->morebind);
  for (size_t i = 0; i < size; ++i) {
    int name = (int)(long)vec_at(cap->morebind, i);
    capattern_register_variable(name, type, loc);
  }

  // walk for structure items
  if (withsub) {
    PatternGroup *pg = cap->items;
    for (int i = 0; i < pg->size; ++i)
      register_capattern_symtable(pg->patterns[i], loc);
  }
}

void register_capattern_symtable(CAPattern *cap, SLoc *loc) {
  switch (cap->type) {
  case PT_Var:
    register_structpattern_symtable(cap, 0, 0, loc);
    break;
  case PT_GenTuple:
    register_structpattern_symtable(cap, 0, 1, loc);
    break;
  case PT_Tuple:
  case PT_Struct:
    register_structpattern_symtable(cap, 1, 1, loc);
    break;
  case PT_IgnoreOne:
  case PT_IgnoreRange:
    return;
  }
}

static int capattern_check_ignore(CAPattern *cap) {
  int count = 0;
  switch (cap->type) {
  case PT_Var:
    return 0;
  case PT_GenTuple:
  case PT_Tuple:
  case PT_Struct:
    for (int i = 0; i < cap->items->size; ++i) {
      if (cap->items->patterns[i]->type == PT_IgnoreRange)
	count += 1;
      else
	if (capattern_check_ignore(cap->items->patterns[i]))
	  return -1;
    }

    if (!count)
      return 0;

    // struct ignore range must be at the end
    if (count > 1 || cap->type == PT_Struct && cap->items->patterns[cap->items->size-1]->type != PT_IgnoreRange)
      return -1;

    return 0;
  case PT_IgnoreOne:
    return 0;
  case PT_IgnoreRange:
    // the ignore range must be in tuple or struct
    return -1;
  }
}

ASTNode *make_let_stmt(CAPattern *cap, ASTNode *exprn) {
  if (capattern_check_ignore(cap)) {
    yyerror("line: %d, col: %d: capattern can only have one ignore range field", glineno, gcolno);
    return NULL;
  }

  // parse variables (may with different datatype) in CAPattern and record them in the symbol table for later use
  if (curr_symtable == &g_root_symtable && cap->type != PT_Var) {
    yyerror("line: %d, col: %d: left `%s` cannot do pattern match for `%s` globally",
	    glineno, gcolno, symname_get(cap->name));
    return NULL;
  }

  register_capattern_symtable(cap, &exprn->endloc);

  ASTNode *p = new_ASTNode(TTE_LetBind);
  p->letbindn.cap = cap;
  p->letbindn.expr = exprn;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &exprn->endloc);
  p->symtable = exprn->symtable;
  return p;
}

ASTNode *make_vardef_zero_value() {
  ASTNode *p = new_ASTNode(TTE_VarDefZeroValue);
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

static ASTNode *make_assign_common(ASTNode *left, ASTNode *right) {
  ASTNode *p = new_ASTNode(TTE_Assign);
  p->assignn.id = left;
  p->assignn.op = -1;
  p->assignn.expr = right;
  set_address(p, &left->begloc, &right->endloc);
  return p;
}

static ASTNode *make_assign_var(int id, ASTNode *exprn) {
  dot_emit("stmt", "varassign");

  STEntry *entry = sym_getsym(curr_symtable, id, 1);
  if (!entry) {
    yyerror("line: %d, col: %d: symbol '%s' not defined", glineno, gcolno, symname_get(id));
    return NULL;
  }

  ASTNode *idn = make_id(id, TTEId_VarAssign);
  idn->entry = entry;

  ASTNode *p = make_assign_common(idn, exprn);
  return p;
}

static ASTNode *make_deref_left_assign(DerefLeft deleft, ASTNode *exprn) {
  ASTNode *derefln = make_deref_left(deleft);
  ASTNode *p = make_assign_common(derefln, exprn);
  return p;
}

static ASTNode *make_arrayitem_left_assign(ArrayItem ai, ASTNode *exprn) {
  ASTNode *aitemn = new_ASTNode(TTE_ArrayItemLeft);
  aitemn->aitemn = ai;
  set_address(aitemn, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});

  ASTNode *p = make_assign_common(aitemn, exprn);
  return p;
}

static ASTNode *make_structfield_left_assign(StructFieldOp sfop, ASTNode *exprn) {
  ASTNode *sfopn = new_ASTNode(TTE_StructFieldOpLeft);
  sfopn->sfopn = sfop;
  set_address(sfopn, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});

  ASTNode *p = make_assign_common(sfopn, exprn);
  return p;
}

ASTNode *make_assign(LeftValueId *lvid, ASTNode *exprn) {
  switch (lvid->type) {
  case LVT_Var:
    return make_assign_var(lvid->var, exprn);
  case LVT_Deref:
    return make_deref_left_assign(lvid->deleft, exprn);
  case LVT_ArrayItem:
    return make_arrayitem_left_assign(lvid->aitem, exprn);
  case LVT_StructOp:
    return make_structfield_left_assign(lvid->structfieldop, exprn);
  default:
    yyerror("line: %d, col: %d: unknown assignment type", glineno, gcolno);
    return NULL;
  }
}

ASTNode *make_assign_op(LeftValueId *lvid, int op, ASTNode *exprn) {
  ASTNode *node = make_assign(lvid, exprn);
  node->assignn.op = op;
  return node;
}

ASTNode *make_goto(int labelid) {
  dot_emit("stmt", "GOTO label_id");
  /* because the label name can using the same name as other names (variable, function, etc)
     so innerly represent the label name as "l:<name>", in order to distinguish them */
  int lpos = sym_form_label_id(labelid);
  STEntry *entry = sym_getsym(curr_fn_symtable, lpos, 0);
  if (entry) {
    switch(entry->sym_type) {
    case Sym_Label:
    case Sym_Label_Hanging:
      break;
    default:
      yyerror("line: %d, col: %d: label name '%s' appear but not aim to be a label",
	      glineno, gcolno, symname_get(labelid));
      return NULL;
    }
  } else {
    entry = sym_insert(curr_fn_symtable, lpos, Sym_Label_Hanging);
    SLoc loc = {glineno, gcolno};
    entry->sloc = loc;
  }

  return make_goto_node(lpos);
}

ASTNode *make_label_def(int labelid) {
  dot_emit("label_def", "label_id");

  const char *name = symname_get(labelid);

  /* because the label name can using the same name as other names (variable, function, etc)
     so innerly represent the label name as "l:<name>", in order to distinguish them */
  int lpos = sym_form_label_id(labelid);
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

static typeid_t get_structfield_expr_type_from_tree(ASTNode *node) {
  inference_expr_type(node->sfopn.expr);
  typeid_t objtype = get_expr_type_from_tree(node->sfopn.expr);
  if (objtype == typeid_novalue)
    return typeid_novalue;

  CADataType *catype = catype_get_by_name(node->symtable, objtype);
  CHECK_GET_TYPE_VALUE(node, catype, objtype);
  // it's a pointer type
  if (!node->sfopn.direct) {
    if (catype->type != POINTER) {
      yyerror("line: %d, col: %d: member reference `%s` is not a pointer type; try '.'?",
	      node->begloc.row, node->begloc.col, catype_get_type_name(catype->signature));
      return typeid_novalue;
    }

    assert(catype->pointer_layout->dimension == 1);
    catype = catype->pointer_layout->type;
  }

  if (catype->type != STRUCT) {
    yyerror("line: %d, col: %d: member reference `%s` is not a struct type",
	    node->begloc.row, node->begloc.col, catype_get_type_name(catype->signature));
    return typeid_novalue;
  }

  if (catype->struct_layout->tuple) {
    if (node->sfopn.fieldname < catype->struct_layout->fieldnum)
      return catype->struct_layout->fields[node->sfopn.fieldname].type->signature;
  } else {
    for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
      if (catype->struct_layout->fields[i].name == node->sfopn.fieldname)
	return catype->struct_layout->fields[i].type->signature;
    }
  }

  yyerror("line: %d, col: %d: cannot find field name `%s` in struct `%s`",
	  node->begloc.row, node->begloc.col, symname_get(node->sfopn.fieldname),
	  symname_get(catype->struct_layout->name));

  return typeid_novalue;
}

typeid_t get_expr_type_from_tree(ASTNode *node) {
  switch (node->type) {
  case TTE_Literal:
    return node->litn.litv.datatype;
  case TTE_Id:
    if (!node->entry || node->entry->sym_type != Sym_Variable) {
      yyerror("line: %d, col: %d: the name '%s' is not a variable",
	      node->begloc.col, node->begloc.row, symname_get(node->idn.i));
      return typeid_novalue;
    }

    return node->entry->u.var->datatype;
  case TTE_DerefLeft: {
    ASTNode *expr = node->deleftn.expr;
    typeid_t innerid = get_expr_type_from_tree(expr);
    if (innerid == typeid_novalue)
      return typeid_novalue;

    CADataType *catype = catype_get_by_name(expr->symtable, innerid);
    for (int i = 0; i < node->deleftn.derefcount; ++i) {
      if (catype->type != POINTER) {
	yyerror("line: %d, col: %d: non pointer type `%s` cannot do dereference, index: `%d`",
		expr->begloc.row, expr->begloc.col, catype_get_type_name(catype->signature), i);
	return typeid_novalue;
      }
      assert(catype->pointer_layout->dimension == 1);
      catype = catype->pointer_layout->type;
    }

    return catype->signature;
  }
  case TTE_ArrayItemLeft: {
    //STEntry *entry = sym_getsym(node->symtable, node->aitemn.varname, 1);
    //CADataType *catype = catype_get_by_name(node->symtable, entry->u.var->datatype);
    inference_expr_type(node->aitemn.arraynode);
    typeid_t arraytype = get_expr_type_from_tree(node->aitemn.arraynode);
    CADataType *catype = catype_get_by_name(node->symtable, arraytype);
    void *indices = node->aitemn.indices;
    size_t size = vec_size(indices);
    for (int i = 0; i < size; ++i) {
      if (catype->type != ARRAY) {
	yyerror("line: %d, col: %d: type `%d` not an array on index `%d`",
		node->begloc.row, node->begloc.col, catype->type, i);
	return typeid_novalue;
      }

      catype = catype->array_layout->type;
    }

    return catype->signature;
  }
  case TTE_StructFieldOpLeft:
    return get_structfield_expr_type_from_tree(node);
  case TTE_As:
    return node->exprasn.type;
  case TTE_Expr:
    return node->exprn.expr_type;
  default:
    return typeid_novalue;
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

int parse_lexical_char(const char *text) {
  if (text[0] != '\\')
    return text[0];

  int n = 0;

  switch(text[1]) {
  case 'r':
    return '\r';
  case 'n':
    return '\n';
  case 't':
    return '\t';
  default:
    n = atoi(text+1);
    return n;

    yyerror("line: %d, col: %d: unimplemented special character", glineno, gcolno);
    return -1;
  }
}

int is_logic_op(int op) { 
  switch (op) {
  case '<':
  case '>':
  case GE:
  case LE:
  case NE:
  case EQ:
    return 1;
  default:
    return 0;
  }
}

typeid_t get_fncall_form_datatype(ASTNode *node, int id) {
  STEntry *entry = sym_getsym(&g_root_symtable, id, 0);
  if (entry)
    return entry->u.f.rettype;

  const char *fnname = catype_get_type_name(id);
  typeid_t tupleid = sym_form_type_id_by_str(fnname);
  entry = sym_getsym(node->symtable, tupleid, 1);
  if (!entry) {
    yyerror("line: %d, col: %d: can find entry for function call or tuple call: `%s`",
	    node->begloc.row, node->begloc.col, fnname);
    return typeid_novalue;
  }

  if (entry->sym_type != Sym_DataType) {
    yyerror("line: %d, col: %d: the entry is not datatype entry: `%s`",
	    node->begloc.row, node->begloc.col, fnname);
    return typeid_novalue;
  }

  if (!entry->u.datatype.tuple) {
    yyerror("line: %d, col: %d: only support tuple call here: `%s`",
	    node->begloc.row, node->begloc.col, fnname);
    return typeid_novalue;
  }

  return entry->u.datatype.id;
}

typeid_t inference_expr_type(ASTNode *node);
typeid_t inference_expr_expr_type(ASTNode *node) {
  CADataType *catype = NULL;
  typeid_t type1 = typeid_novalue;
  int possible_pointer_op = 0;
  int pointer_op = 0;

  if (node->exprn.expr_type != typeid_novalue)
    return node->exprn.expr_type;

  switch (node->exprn.op) {
  case ARRAY: {
    ASTNode *anode = node->exprn.operands[0];
    size_t size = arrayexpr_size(anode->anoden.aexpr);
    typeid_t prevtypeid = typeid_novalue;
    for (int i = 0; i < size; ++i) {
      ASTNode *node = arrayexpr_get(anode->anoden.aexpr, i);
      typeid_t typeid = inference_expr_type(node);
      if (i == 0) prevtypeid = typeid;
      if (prevtypeid != typeid) {
	yyerror("line: %d, col: %d: array element `%d` have different type `%s` != `%s`",
		glineno, gcolno, i, catype_get_type_name(prevtypeid), catype_get_type_name(typeid));
	return typeid_novalue;
      }

      prevtypeid = typeid;
    }

    CADataType *subcatype = catype_get_primitive_by_name(prevtypeid);
    CADataType *catype = catype_make_array_type(subcatype, size, 0);
    type1 = catype->signature;
    break;
  }
  case ARRAYITEM: {
    assert(node->exprn.noperand == 1);
    ASTNode *right = node->exprn.operands[0];
    assert(right->type == TTE_ArrayItemRight);
    //STEntry *entry = sym_getsym(right->symtable, right->aitemn.varname, 1);
    //CADataType *catype = catype_get_by_name(right->symtable, entry->u.var->datatype);
    inference_expr_type(right->aitemn.arraynode);
    typeid_t arraytype = get_expr_type_from_tree(right->aitemn.arraynode);
    CADataType *catype = catype_get_by_name(right->symtable, arraytype);
    void *indices = right->aitemn.indices;
    size_t size = vec_size(indices);
    for (int i = 0; i < size; ++i) {
      if (catype->type != ARRAY) {
	yyerror("line: %d, col: %d: type `%d` not an array on index `%d`",
		node->begloc.row, node->begloc.col, catype->type, i);
	return typeid_novalue;
      }

      catype = catype->array_layout->type;
    }

    type1 = catype->signature;
    break;
  }
  case STRUCT: {
    ASTNode *anode = node->exprn.operands[0];
    CADataType *catype = catype_get_by_name(anode->symtable, anode->snoden.name);
    CHECK_GET_TYPE_VALUE(anode, catype, anode->snoden.name);
    type1 = catype->signature;
    break;
  }
  case TUPLE: {
    // the general tuple expresssion
    ASTNode *anode = node->exprn.operands[0];
    assert(anode->type == TTE_ArgList);
    int argc = anode->arglistn.argc;
    typeid_t *args = (typeid_t *)malloc(argc * sizeof(typeid_t));
    for (int i = 0; i < argc; ++i)
      args[i] = inference_expr_type(anode->arglistn.exprs[i]);

    type1 = sym_form_tuple_id(args, argc);
    free(args);

    CADataType *catype = catype_get_by_name(anode->symtable, type1);
    CHECK_GET_TYPE_VALUE(anode, catype, type1);
    type1 = catype->signature;
    break;
  }
  case STRUCTITEM: {
    assert(node->exprn.noperand == 1);
    ASTNode *p = node->exprn.operands[0];
    assert(p->type == TTE_StructFieldOpRight);
    inference_expr_type(p->sfopn.expr);
    type1 = get_structfield_expr_type_from_tree(p);
    break;
  }
  case FN_CALL: {
    // get function return type
    ASTNode *idn = node->exprn.operands[0];
    type1 = get_fncall_form_datatype(node, idn->idn.i);
    break;
  }
  case STMT_EXPR:
    type1 = inference_expr_type(node->exprn.operands[1]);
    break;
  case SIZEOF:
    type1 = sym_form_type_id_from_token(U64);
    break;
  case DEREF:
    type1 = inference_expr_type(node->exprn.operands[0]);
    catype = catype_get_by_name(node->symtable, type1);
    if (catype->type != POINTER) {
      yyerror("line: %d, col: %d: only an address (pointer) type can do dereference, `%s` type cannot",
	      node->begloc.row, node->begloc.col, catype_get_type_name(catype->signature));
      return typeid_novalue;
    }

    // TODO; check if the pointer signature is already formalized
    type1 = catype->pointer_layout->type->signature;
    break;
  case ADDRESS:
    type1 = inference_expr_type(node->exprn.operands[0]);
    catype = catype_get_by_name(node->symtable, type1);
    catype = catype_make_pointer_type(catype);
    type1 = catype->signature;
    break;
  case '+':
  case '-':
    assert(node->exprn.noperand == 2);
    possible_pointer_op = 1;
  case AS:
  case UMINUS:
  case IF_EXPR:
  default:
    for (int i = 0; i < node->exprn.noperand; ++i) {
      typeid_t type = inference_expr_type(node->exprn.operands[i]);
      if (type1 == typeid_novalue) {
	type1 = type;
      } else if (!catype_check_identical_in_symtable(node->symtable, type1, node->symtable, type)) {
	if (node->exprn.op == SHIFTL || node->exprn.op == SHIFTR) {
	  CADataType *catype1 = catype_get_by_name(node->symtable, type1);
	  CADataType *catype2 = catype_get_by_name(node->symtable, type);
	  if (!catype_is_integer(catype1->type) || !catype_is_integer(catype2->type)) {
	    yyerror("line: %d, column: %d, expected `integer`, but found `%s` `%s` for shift operation",
		    node->begloc.row, node->begloc.col, symname_get(type1), symname_get(type));
	    return typeid_novalue;
	  }
	} else {
	  if (possible_pointer_op) {
	    CADataType *catype1 = catype_get_by_name(node->symtable, type1);
	    CADataType *catype2 = catype_get_by_name(node->symtable, type);
	    pointer_op = (catype1->type == POINTER && catype_is_integer(catype2->type));
	  }

	  if (!pointer_op) {
	    yyerror("line: %d, column: %d, expected `%s`, found `%s`",
		    node->begloc.row, node->begloc.col, symname_get(type1), symname_get(type));
	    return typeid_novalue;
	  }
	}
      }
    }
    break;
  }

  if (is_logic_op(node->exprn.op))
    type1 = sym_form_type_id_from_token(BOOL);

  node->exprn.expr_type = type1;
  return type1;
}

// inference and set the expr type for the expr, when the expr have no a
// determined type, different from `determine_expr_type`, the later is used by
// passing a defined type
typeid_t inference_expr_type(ASTNode *p) {
  // steps, it's a recursive steps
  // 1. firstly inference the expression type, it need check if the type can conflict, and determine a type
  // 2. resolve the node type by using `determine_expr_type(exprn, type)`
  typeid_t type1 = typeid_novalue;
  CADataType *typedt, *exprdt;
  switch (p->type) {
  case TTE_Literal: {
    CALiteral *litv = &p->litn.litv;
    if (litv->postfixtypetok != tokenid_novalue && !litv->fixed_type) {
      // when literal carry a postfix like i32 u64 etc, determine them directly
      CADataType *catype = catype_get_primitive_by_token(litv->postfixtypetok);
      determine_primitive_literal_type(litv, catype);
      type1 = catype->signature;
    } else {
      type1 = inference_literal_type(litv);
    }

    litv->fixed_type = 1;
    return type1;
  }
  case TTE_Id:
    if (p->idn.idtype == TTEId_FnName)
      return typeid_novalue;

    return p->entry->u.var->datatype;
  case TTE_As:
    type1 = inference_expr_type(p->exprasn.expr);
    // TODO: handle when complex type
    typedt = catype_get_by_name(p->symtable, p->exprasn.type);
    CHECK_GET_TYPE_VALUE(p, typedt, p->exprasn.type);

    exprdt = catype_get_by_name(p->symtable, type1);
    CHECK_GET_TYPE_VALUE(p, exprdt, type1);

    if (!as_type_convertable(exprdt->type, typedt->type)) {
      yyerror("line: %d, column: %d, type `%s` cannot convert (as) to type `%s`",
		p->begloc.row, p->begloc.col,
		get_type_string(exprdt->type), get_type_string(typedt->type));
      return -1;
    }
    
    //return p->exprasn.type;
    return typedt->signature;
  case TTE_Expr:
    return inference_expr_expr_type(p);
  case TTE_If:
    if (!p->ifn.isexpr) {
      yyerror("line: %d, col: %d: if statement is not if expression",
	      glineno, gcolno);
      return typeid_novalue;
    }

    // determine if expression type
    // TODO: realize multiple if else statement
    type1 = inference_expr_type((ASTNode *)(vec_at(p->ifn.bodies, 0)));
    if (p->ifn.remain) {
      typeid_t type2 = inference_expr_type(p->ifn.remain);
      if (!catype_check_identical_in_symtable(p->symtable, type1, p->symtable, type2)) {
	yyerror("line: %d, col: %d: if expression type not same `%s` != `%s`",
		glineno, gcolno, symname_get(type1), symname_get(type2));
	return typeid_novalue;
      }
    }
    return type1;
  case TTE_LexicalBody:
    return inference_expr_type(p->lnoden.stmts);
  case TTE_Box:
    type1 = inference_expr_type(p->boxn.expr);
    typedt = catype_get_by_name(p->boxn.expr->symtable, type1);
    exprdt = catype_make_pointer_type(typedt);
    return exprdt->signature;
  default:
    yyerror("line: %d, col: %d: the expression already typed, no need to do inference",
	    glineno, gcolno);
    return typeid_novalue;
  }
}

int determine_expr_type(ASTNode *node, typeid_t type);
static int determine_expr_expr_type(ASTNode *node, typeid_t type) {
  if (node->exprn.expr_type != typeid_novalue)
    return 0;

  CADataType *datatype = NULL;

  switch (node->exprn.op) {
  case ARRAY: {
    // most important is check if the inference type and the determined is compatible
    CADataType *determinedcatype = catype_get_by_name(node->symtable, type);
    if (determinedcatype->type != ARRAY) {
      yyerror("line: %d, col: %d: expression type is array type, cannot determined into `%s` type",
	      node->begloc.row, node->begloc.col, catype_get_type_name(type));
      return -1;
    }

    assert(determinedcatype->array_layout->dimension == 1);
    CAArrayExpr aexpr = node->exprn.operands[0]->anoden.aexpr;
    size_t size = arrayexpr_size(aexpr);
    int len = determinedcatype->array_layout->dimarray[0];
    if (len != size) {
      yyerror("line: %d, col: %d: determined array size `%d` not match the expression type `%d`",
	      node->begloc.row, node->begloc.col, len, size);
      return -1;
    }

    for (size_t i = 0; i < size; ++i) {
      ASTNode *subnode = arrayexpr_get(aexpr, i);
      CADataType *subcatype = determinedcatype->array_layout->type;
      determine_expr_type(subnode, subcatype->signature);
    }

    break;
  }
  case ARRAYITEM: {
    // arrayitem operation should not determine the type, the array type should already be determined
    // following just do some checks
    assert(node->exprn.noperand == 1);
    ASTNode *right = node->exprn.operands[0];
    assert(right->type == TTE_ArrayItemRight);
    //STEntry *entry = sym_getsym(right->symtable, right->aitemn.varname, 1);
    //CADataType *catype = catype_get_by_name(right->symtable, entry->u.var->datatype);
    inference_expr_type(right->aitemn.arraynode);
    typeid_t arraytype = get_expr_type_from_tree(right->aitemn.arraynode);
    CADataType *catype = catype_get_by_name(right->symtable, arraytype);
    void *indices = right->aitemn.indices;
    size_t size = vec_size(indices);
    for (int i = 0; i < size; ++i) {
      if (catype->type != ARRAY) {
	yyerror("line: %d, col: %d: type `%d` not an array on index `%d`",
		node->begloc.row, node->begloc.col, catype->type, i);
	return typeid_novalue;
      }

      catype = catype->array_layout->type;
    }

    CADataType *determinedcatype = catype_get_by_name(node->symtable, type);
    if (determinedcatype->signature != catype->signature) {
      yyerror("line: %d, col: %d: determined type on arrayitem operation not equal the array's nature type `%s` != `%s`",
	      node->begloc.row, node->begloc.col, catype_get_type_name(determinedcatype->signature), catype_get_type_name(catype->signature));
      return -1;
    }

    break;
  }
  case STRUCTITEM: {
    // structitem operation should not determine the type, the struct type should already be determined
    // following just do some checks
    assert(node->exprn.noperand == 1);
    ASTNode *p = node->exprn.operands[0];
    assert(p->type == TTE_StructFieldOpRight);
    inference_expr_type(p->sfopn.expr);
    typeid_t origtype = get_structfield_expr_type_from_tree(p);
    if (origtype != type) {
      yyerror("line: %d, col: %d: determined type `%s` not compatible with original one `%s`",
	      node->begloc.row, node->begloc.col, catype_get_type_name(type), catype_get_type_name(origtype));

      return -1;
    }
    break;
  }
  case FN_CALL: {
    // get function return type
    ASTNode *idn = node->exprn.operands[0];
    typeid_t type1 = get_fncall_form_datatype(node, idn->idn.i);
    catype_check_identical_in_symtable_witherror(node->symtable, type, node->symtable, type1, 1, &node->begloc);
    break;
  }
  case STMT_EXPR:
    determine_expr_type(node->exprn.operands[1], type);
    break;
  case SIZEOF:
    if (type != sym_form_type_id_from_token(U64)) {
      yyerror("line: %d, col: %d: conflict when determining type: `%s` != `u64`",
	      node->begloc.row, node->begloc.col, catype_get_type_name(type));

      return -1;
    }
    break;
  case DEREF:
    datatype = catype_get_by_name(node->symtable, type);
    datatype = catype_make_pointer_type(datatype);
    determine_expr_type(node->exprn.operands[0], datatype->signature);
    break;
  case ADDRESS: {
    // check if right side is a variable and it's address type the same as determined one
    // only the variable can have an address
    // TODO: handle function address
    ASTNode *idnode = node->exprn.operands[0];
    if (idnode->type != TTE_Id) {
      yyerror("line: %d, col: %d: only a variable can have an address, but find type `%d`",
	      node->begloc.row, node->begloc.col, idnode->type);
      return -1;
    }

    datatype = catype_get_by_name(node->symtable, type);
    if (datatype->type != POINTER) {
      yyerror("line: %d, col: %d: a pointer type cannot determined into `%s` type",
	      node->begloc.row, node->begloc.col, catype_get_type_name(datatype->signature));
      return -1;
    }

    CADataType *idcatype = catype_get_by_name(idnode->symtable, idnode->entry->u.var->datatype);

    if (idcatype->signature != datatype->pointer_layout->type->signature) {
      yyerror("line: %d, col: %d: variable address type `%s` cannot be type of `%s`",
	      node->begloc.row, node->begloc.col,
	      catype_get_type_name(idcatype->signature),
	      catype_get_type_name(datatype->pointer_layout->type->signature));
      return -1;
    }

    break;
  }
  case '>':
  case '<':
  case GE:
  case LE:
  case NE:
  case EQ: {
    // determine logical expresssion type, must be bool
    datatype = catype_get_by_name(node->symtable, type);
    if (datatype->type != BOOL) {
      yyerror("line: %d, col: %d: `bool` required for determining logical operation, but `%s` type found",
	      node->begloc.row, node->begloc.col, catype_get_type_name(datatype->signature));
      return -1;
    }

    inference_expr_type(node);
    break;
  }
  case '+':
  case '-':
    assert(node->exprn.noperand == 2);
    datatype = catype_get_by_name(node->symtable, type);
    if (datatype->type == POINTER) {
      if (node->exprn.operands[0]->type == TTE_Expr)
	determine_expr_type(node->exprn.operands[0], type);

      typeid_t firstid = get_expr_type_from_tree(node->exprn.operands[0]);
      if (firstid == typeid_novalue) {
	yyerror("line: %d, col: %d: when determining pointer type, right value should already determined, but here cannot find a determined type",
		node->begloc.row, node->begloc.col);
	return -1;
      }

      CADataType *firstca = catype_get_by_name(node->symtable, firstid);
      if (firstca->type != POINTER) {
	yyerror("line: %d, col: %d: should only can determined into pointer type, but find `%s` type",
		node->begloc.row, node->begloc.col, catype_get_type_name(firstid));
	return -1;
      }

      if (datatype->signature != firstca->signature) {
	yyerror("line: %d, col: %d: determined type `%s` not equal to determining type `%s`",
		node->begloc.row, node->begloc.col, catype_get_type_name(datatype->signature),
		catype_get_type_name(firstca->signature));
	return -1;
      }

      typeid_t secondid = inference_expr_type(node->exprn.operands[1]);
      CADataType *secondca = catype_get_by_name(node->symtable, secondid);
      if (!catype_is_integer(secondca->type)) {
	yyerror("line: %d, col: %d: the 2nd pointer operand not support non-integer type, but find `%s`",
		node->begloc.row, node->begloc.col, catype_get_type_name(secondca->signature));
	return -1;
      }

      node->exprn.operands[1]->exprn.expr_type = secondca->signature;
      break;
    }
  case LAND:
  case LOR:
  case AS:
  case UMINUS:
  case IF_EXPR:
  default:
    for (int i = 0; i < node->exprn.noperand; ++i) {
      determine_expr_type(node->exprn.operands[i], type);
    }
    break;
  }

  node->exprn.expr_type = type;
  return 0;
}

// determine and set the expr type for the expr for a specified type, different
// from `inference_expr_type` which have no a defined type parameter
int determine_expr_type(ASTNode *node, typeid_t type) {
  // TODO: handle when complex type
  CADataType *catype = catype_get_by_name(node->symtable, type);
  CHECK_GET_TYPE_VALUE(node, catype, type);
  typeid_t id;
  CADataType *exprcatype = NULL;
  switch(node->type) {
  case TTE_Literal: {
    CALiteral *litv = &node->litn.litv;
    if (litv->postfixtypetok != tokenid_novalue && !litv->fixed_type) {
      // when literal carry a postfix like i32 u64 etc, determine them directly
      CADataType *postcatype = catype_get_primitive_by_token(litv->postfixtypetok);
      determine_primitive_literal_type(litv, postcatype);
      litv->fixed_type = 1;
      if (postcatype->signature != catype->signature) {
	yyerror("line: %d, col: %d: `%s` type required, but found `%s`\n",
		node->begloc.row, node->begloc.col, catype_get_type_name(catype->signature),
		catype_get_type_name(postcatype->signature));
	return -1;	
      }
    }

    if (litv->fixed_type)
      return 0;

    // here determine the literal type in this place compare to when create literal node
    determine_literal_type(litv, catype);
    break;
  }
  case TTE_Id:
    if (!node->entry) {
      STEntry *entry = sym_getsym(node->symtable, node->idn.i, 1);
      if (!entry) {
	yyerror("line: %d, col: %d: cannot find symbol for id: `%d` in symbol table\n",
		node->begloc.row, node->begloc.col, node->idn.i);
	return -1;
      }

      if (entry->sym_type == Sym_Variable) {
	yyerror("line: %d, col: %d: variable not filled the entry yet: `%s`\n",
		node->begloc.row, node->begloc.col, symname_get(entry->sym_name));
	return -1;
      } else {
	// no need to determine type
	return 0;
      }
    }

    if (node->entry->u.var->datatype == typeid_novalue)
      node->entry->u.var->datatype = catype->signature;
    else if (!catype_check_identical_in_symtable(node->symtable,
				     node->entry->u.var->datatype,
				     node->symtable, type)) {
      // fprintf(stderr, 
      yyerror("line: %d, col: %d: determine different type `%s` != `%s`\n",
	     node->begloc.row, node->begloc.col, symname_get(type),
	     symname_get(node->entry->u.var->datatype));
      return 0;
    }

    break;
  case TTE_DerefLeft: {
    ASTNode *expr = node->deleftn.expr;
    typeid_t exprid = inference_expr_type(expr);
    typeid_t innerid = get_expr_type_from_tree(expr);
    assert(exprid == innerid);
    if (innerid == typeid_novalue) {
      yyerror("line: %d, col: %d: dereference left operation must be fixed type to: `%s`, but find non-fixed",
	      expr->begloc.row, expr->begloc.col, catype_get_type_name(type));
      return typeid_novalue;
    }

    CADataType *catype = catype_get_by_name(expr->symtable, innerid);
    for (int i = 0; i < node->deleftn.derefcount; ++i) {
      if (catype->type != POINTER) {
	yyerror("line: %d, col: %d: non array type `%s` cannot do dereference, index: `%d`",
		expr->begloc.row, expr->begloc.col, catype_get_type_name(catype->signature), i);
	return -1;
      }
      assert(catype->pointer_layout->dimension == 1);
      catype = catype->pointer_layout->type;
    }

    if (catype->signature != type) {
      yyerror("line: %d, col: %d: determined type `%s` not compatible with `%s`",
	      expr->begloc.row, expr->begloc.col, catype_get_type_name(type),
	      catype_get_type_name(catype->signature));
      return -1;
    }

    break;
  }
  case TTE_As:
    exprcatype = catype_get_by_name(node->symtable, node->exprasn.type);
    CHECK_GET_TYPE_VALUE(node, exprcatype, node->exprasn.type);

    if (!catype_check_identical(catype, exprcatype)) {
      yyerror("line: %d, column: %d, type `%s` cannot determined into `%s`",
	      node->begloc.row, node->begloc.col,
	      catype_get_type_name(catype->signature),
	      catype_get_type_name(exprcatype->signature));
      return -1;
    }

    // here should keep the original type as it is 
    id = inference_expr_type(node->exprasn.expr);
    //determine_expr_type(node->exprasn.expr, type);
    break;
  case TTE_Expr:
    return determine_expr_expr_type(node, type);
  case TTE_If:
    if (!node->ifn.isexpr) {
      yyerror("line: %d, col: %d: if statement is not if expression", glineno, gcolno);
      return -1;
    }

    // determine if expression type
    // TODO: realize multiple if else statement
    determine_expr_expr_type((ASTNode *)(vec_at(node->ifn.bodies, 0)), type);
    if (node->ifn.remain) {
      determine_expr_expr_type(node->ifn.remain, type);
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
// determined: int reduce_node_and_type(ASTNode *p, typeid_t *expr_types, int noperands)
int reduce_node_and_type_group(ASTNode **nodes, typeid_t *expr_types, int nodenum, int assignop) {
  // check if exist type in the each node and type is conflicting for each node
  // but here cannot create literal value when the value not determined a type
  // because it may be a tree, or can make the type by tranverlling the tree?
  // The answer is yes, here can determine the literal type when the expression
  // exists an fixed type part
  typeid_t type1 = typeid_novalue;
  int typei = 0;
  int notypeid = 0;
  int *nonfixed_node = (int *)alloca(nodenum * sizeof(int));
  for (int i = 0; i < nodenum; ++i) {
    if (expr_types[i] != typeid_novalue) {
      nonfixed_node[i] = 0;
      if (type1 == typeid_novalue) {
	type1 = expr_types[i];
	typei = i;
      } else if (assignop == -1 && !catype_check_identical_in_symtable(nodes[i]->symtable, type1, nodes[i]->symtable, expr_types[i])) {
	// when assignop not -1 it will not check the type
	CADataType *dt1 = catype_get_by_name(nodes[i]->symtable, type1);
	CADataType *dt2 = catype_get_by_name(nodes[i]->symtable, expr_types[i]);

	yyerror("line: %d, col: %d: type name conflicting: type `%s`(`%s`) with type `%s`(`%s`)",
		nodes[i]->begloc.row, nodes[i]->begloc.col,
		catype_get_type_name(type1), catype_get_type_name(dt1->signature),
		catype_get_type_name(expr_types[i]), catype_get_type_name(dt2->signature));
	return 0;
      }
    } else {
      nonfixed_node[i] = 1;
    }
  }

  // when the expression have any fixed type node
  for (int i = 0; i < nodenum; ++i) {
    if (nonfixed_node[i]) {
      determine_expr_type(nodes[i], type1);
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
    dot_emit_expr("from", "to", op);

    va_list ap;
    int i;

    ASTNode *p = new_ASTNode(TTE_Expr);
    p->exprn.op = op;
    p->exprn.noperand = noperands;

    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    // try to inference the expression type here
    va_start(ap, noperands);
    for (i = 0; i < noperands; i++) {
      p->exprn.operands[i] = va_arg(ap, ASTNode*);
    }
    va_end(ap);

    p->exprn.expr_type = typeid_novalue;

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

    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_expr_arglists_actual(ST_ArgListActual *al) {
  dot_emit("fn_args_call", "fn_args_call_p");

  int argc = al ? al->argc : 0;
    
  ASTNode *p = new_ASTNode(TTE_ArgList);
  p->arglistn.argc = argc;

  if (al && (p->arglistn.exprs = (ASTNode **)malloc(argc * sizeof(ASTNode))) == NULL)
    yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

  for (int i = 0; i < argc; i++)
    p->arglistn.exprs[i] = al->args[i];

  if (argc == 1) {
    p->begloc = p->arglistn.exprs[0]->begloc;
    p->endloc = p->arglistn.exprs[0]->endloc;
  } else if (argc > 1) {
    p->begloc = p->arglistn.exprs[0]->begloc;
    p->endloc = p->arglistn.exprs[argc - 1]->endloc;
  } else {
    p->begloc = (SLoc){glineno, gcolno};
    p->endloc = (SLoc){glineno, gcolno};
  }

  p->symtable = curr_symtable;
  if (al)
    actualarglist_pop();

  return p;
}

ASTNode *make_label_node(int i) {
  ASTNode *p = make_id(i, TTEId_Label);
  if (p) {
    p->type = TTE_Label;
    return p;
  }

  return NULL;
}

ASTNode *make_goto_node(int i) {
  ASTNode *p = make_id(i, TTEId_LabelGoto);
  if (p) {
    p->type = TTE_LabelGoto;
    return p;
  }

  return NULL;
}

ASTNode *build_mock_main_fn_node() {
  ASTNode *decl = new_ASTNode(TTE_FnDecl);

  typeid_t typesym = sym_form_type_id_from_token(I32);
  decl->fndecln.ret = typesym;

  int fnid = symname_check("main");
  decl->fndecln.name = sym_form_function_id(fnid);
  
  decl->fndecln.is_extern = 0;

  set_address(decl, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  ASTNode *p = new_ASTNode(TTE_FnDef);
  p->fndefn.fn_decl = decl;
  p->fndefn.stmts = NULL;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});

  return p;
}

static ASTNode *build_fn_decl(typeid_t name, ST_ArgList *al, typeid_t rettype, SLoc beg, SLoc end) {
    ASTNode *p = new_ASTNode(TTE_FnDecl);
    p->fndecln.ret = rettype;
    p->fndecln.name = name;
    p->fndecln.args = *al;
    p->fndecln.is_extern = 0; // TODO: make extern real extern

    set_address(p, &beg, &end);
    return p;
}

static ASTNode *build_fn_define(typeid_t name, ST_ArgList *al, typeid_t rettype, SLoc beg, SLoc end) {
    ASTNode *decl = build_fn_decl(name, al, rettype, beg, end);
    ASTNode *p = new_ASTNode(TTE_FnDef);
    p->fndefn.fn_decl = decl;
    p->fndefn.stmts = NULL;

    set_address(p, &beg, &end);
    return p;
}

ASTNode *make_break() {
  ASTNode *p = new_ASTNode(TTE_Break);
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

ASTNode *make_continue() {
  ASTNode *p = new_ASTNode(TTE_Continue);
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
  return p;
}

ASTNode *make_loop(ASTNode *loopbody) {
  ASTNode *p = new_ASTNode(TTE_Loop);
  p->loopn.body = loopbody;

  set_address(p, &loopbody->begloc, &loopbody->endloc);
  return p;
}

void make_for_var_entry(int id) {
  STEntry *entry = sym_getsym(curr_symtable, id, 0);
  if (entry) {
    yyerror("line: %d, col: %d: strange variable '%s' already defined in scope on line %d, col %d.",
	    glineno, gcolno, symname_get(id), entry->sloc.row, entry->sloc.col);
    return;
  }

  entry = sym_insert(curr_symtable, id, Sym_Variable);
  CAVariable *cavar = cavar_create(id, typeid_novalue);
  entry->u.var = cavar;
}

ASTNode *make_for(ForStmtId id, ASTNode *listnode, ASTNode *stmts) {
  ASTNode *p = new_ASTNode(TTE_For);
  p->forn.var = id;
  p->forn.listnode = listnode;
  p->forn.body = stmts;

  set_address(p, &listnode->begloc, &stmts->endloc);
  return p;
}

ASTNode *make_for_stmt(ForStmtId id, ASTNode *listnode, ASTNode *stmts) {
  ASTNode *forn = make_for(id, listnode, stmts);

  // the inner variable and / or listnode also need a lexical body in for statement
  ASTNode *node = make_lexical_body(forn);
  SymTable *st = pop_symtable();
  return node;
}

ASTNode *make_while(ASTNode *cond, ASTNode *whilebody) {
    dot_emit("stmt", "whileloop");

    ASTNode *p = new_ASTNode(TTE_While);
    p->whilen.cond = cond;
    p->whilen.body = whilebody;

    set_address(p, &cond->begloc, &whilebody->endloc);
    return p;
}

ASTNode *new_ifstmt_node() {
  ASTNode *p = new_ASTNode(TTE_If);
  p->ifn.ncond = 0;
  p->ifn.isexpr = 0;
  p->ifn.conds = NULL;
  p->ifn.bodies = NULL;
  p->ifn.remain = NULL;
  return p;
}

ASTNode *make_ifpart(ASTNode *p, ASTNode *cond, ASTNode *body) {
  vec_append(p->ifn.conds, cond);
  vec_append(p->ifn.bodies, body);
  return p;
}

ASTNode *make_elsepart(ASTNode *p, ASTNode *body) {
  p->ifn.remain = body;
  return p;
}

#if 0
ASTNode *make_if(int isexpr, int argc, ...) {
    dot_emit("stmt", "if");
    // or
    dot_emit("ifstmt", "ifelse");
    // or
    dot_emit("ifexpr", "ife");
    
    // TODO: implement multiple if else nodes
    va_list ap;

    int ncond = argc / 2;
    int remainder = argc % 2;

    ASTNode *p = new_ASTNode(TTE_If);
    p->ifn.ncond = ncond;
    p->ifn.isexpr = isexpr;
    if ((p->ifn.conds = malloc(ncond * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    if ((p->ifn.bodies = malloc(ncond * sizeof(ASTNode))) == NULL)
      yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

    va_start(ap, argc);
    for (int i = 0; i < ncond; i++) {
	p->ifn.conds[i] = va_arg(ap, ASTNode*);
	p->ifn.bodies[i] = va_arg(ap, ASTNode*);
    }

    p->ifn.remain = remainder ? va_arg(ap, ASTNode*) : NULL;
    va_end(ap);

    set_address(p, &p->ifn.conds[0]->begloc,
		remainder ? &p->ifn.remain->endloc : &p->ifn.bodies[ncond-1]->endloc);

    if (isexpr)
      return make_expr(IF_EXPR, 1, p);
    else
      return p;
}
#endif

// compare if the previous defined function proto is the same as the current defining
static int pre_check_fn_proto(STEntry *prev, typeid_t fnname, ST_ArgList *currargs, typeid_t rettype) {
  ST_ArgList *prevargs = prev->u.f.arglists;

  /* check if function declaration is the same */
  if (currargs->argc != prevargs->argc) {
    yyerror("line: %d, col: %d: function '%s' parameter number not identical with previous, see: line %d, col %d.",
	    glineno, gcolno, catype_get_function_name(fnname), prev->sloc.row, prev->sloc.col);
    return -1;
  }

  // check parameter types
  if (prevargs->contain_varg != currargs->contain_varg) {
    yyerror("line: %d, col: %d: function '%s' variable parameter not identical, see: line %d, col %d.",
	    glineno, gcolno, catype_get_function_name(fnname), prev->sloc.row, prev->sloc.col);
    return -1;
  }

  return 0;
}

static STEntry *check_tuple_name(int id) {
  // check tuple type
  typeid_t type = sym_form_type_id(id);
  STEntry *entry = sym_getsym(&g_root_symtable, type, 0);

  if (entry && entry->u.datatype.tuple)
    return entry;

  return NULL;
}

ASTNode *make_fn_proto(int fnid, ST_ArgList *arglist, typeid_t rettype) {
  dot_emit("fn_proto", "FN IDENT ...");

  typeid_t fnname = sym_form_function_id(fnid);

  curr_fn_rettype = rettype;

  SLoc beg = {glineno, gcolno};
  SLoc end = {glineno, gcolno};

  if (check_tuple_name(fnid)) {
    yyerror("line: %d, col: %d: function '%s' already defined as tuple in previous",
	    glineno, gcolno, symname_get(fnid));
    return NULL;
  }

  //void *carrier = get_post_function(fnname);
  // for handle post function declaration or define
  CallParamAux *paramaux = get_post_function(fnname);
  STEntry *entry = sym_getsym(&g_root_symtable, fnname, 0);
  if (extern_flag) {
    if (entry) {
      pre_check_fn_proto(entry, fnname, arglist, rettype);
    } else {
      entry = sym_check_insert(&g_root_symtable, fnname, Sym_FnDecl);
      entry->u.f.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
      *entry->u.f.arglists = *arglist;
    }
    entry->u.f.rettype = rettype;

    ASTNode *decln = build_fn_decl(fnname, arglist, rettype, beg, end);
    if (paramaux && !paramaux->checked) {
      paramaux->param = decln;
      paramaux->checked = 1;
      put_post_function(fnname, paramaux);
    }

    return decln;
  } else {
    if (entry) {
      if (entry->sym_type == Sym_FnDef) {
	yyerror("line: %d, col: %d: function '%s' already defined on line %d, col %d.",
		glineno, gcolno, symname_get(fnname), entry->sloc.row, entry->sloc.col);
	return NULL;
      }

      if (entry->sym_type == Sym_FnDecl) {
	entry->sym_type = Sym_FnDef;
	SLoc loc = {glineno, gcolno};
	entry->sloc = loc;
      } else {
	yyerror("line: %d, col: %d: name '%s' is not a function defined on line %d, col %d.",
		glineno, gcolno, symname_get(fnname), entry->sloc.row, entry->sloc.col);
	return NULL;
      }

      pre_check_fn_proto(entry, fnname, arglist, rettype);
    } else {
      entry = sym_check_insert(&g_root_symtable, fnname, Sym_FnDef); 
      entry->u.f.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
      *entry->u.f.arglists = *arglist;
    }
    entry->u.f.rettype = rettype;

    ASTNode *defn = build_fn_define(fnname, arglist, rettype, beg, end);
    if (paramaux && !paramaux->checked) {
      paramaux->param = defn;
      paramaux->checked = 1;
      put_post_function(fnname, paramaux);
    }

    return defn;
  }
}

int check_fn_define(typeid_t fnname, ASTNode *param, int tuple, STEntry *entry) {
  // check formal parameter and actual parameter
  ST_ArgList *formalparam = NULL;
  if (tuple)
    formalparam = entry->u.datatype.members;
  else
    formalparam = entry->u.f.arglists;

  if (!formalparam) {
    yyerror("line: %d, col: %d: cannot find arglist, seems `%s` is not a function or named tuple",
	    glineno, gcolno, catype_get_type_name(fnname));
    return -1;
  }

  // check parameter number
  if(formalparam->contain_varg && formalparam->argc > param->arglistn.argc
     ||
     !formalparam->contain_varg && formalparam->argc != param->arglistn.argc) {
    yyerror("line: %d, col: %d: actual parameter number `%d` not match formal parameter number `%d`",
	    glineno, gcolno, param->arglistn.argc, formalparam->argc);
    return -1;
  }

  return 0;
}

CallParamAux *new_CallParamAux(ASTNode *param, int checked) {
  CallParamAux *paramaux = (CallParamAux *)malloc(sizeof(CallParamAux));
  paramaux->param = param;
  paramaux->checked = checked;
  return paramaux;
}

void delete_CallParamAux(CallParamAux *paramaux) {
  free(paramaux);
}

// id: can be function name or tuple name, tuple is special
ASTNode *make_fn_call_or_tuple(int id, ASTNode *param) {
  dot_emit("fn_call", symname_get(id));

  typeid_t fnname = sym_form_function_id(id);

  // tuple type cannot have the same name with function in the same symbol table

  STEntry *entry = NULL;
  int tuple = extract_function_or_tuple(param->symtable, fnname, &entry, NULL, NULL);
  if (tuple != -1) {
    check_fn_define(fnname, param, tuple, entry);
  } else {
    // when no any name find in the symbol table then make a function call
    // request with specified name, the request may also be tuple call
    CallParamAux *paramaux = new_CallParamAux(param, 0);
    put_post_function(fnname, paramaux);
  }

  return make_expr(FN_CALL, 2, make_id(fnname, TTEId_FnName), param);
}

ASTNode *make_gen_tuple_expr(ASTNode *param) {
  return make_expr(TUPLE, 1, param);
}

ASTNode *make_ident_expr(int id) {
  dot_emit("expr", "IDENT"); 

  STEntry *entry = sym_getsym(curr_symtable, id, 1);
  if (!entry) {
    yyerror("line: %d, col: %d: Variable name '%s' not defined", glineno, gcolno, symname_get(id));
    return NULL;
  }

  ASTNode *node = make_id(id, TTEId_VarUse);
  node->entry = entry;
  return node;
}

ASTNode *make_uminus_expr(ASTNode *expr) {
  // only U64 literal type can combinate '-' here, when littypetok is I64,
  // it means the literal already combined with '-' so here no need combined
  // again, when littypetok is other type like BOOL I8 U8 etc, they are
  // not support combine with '-' so just walk with a UMINUS operator
  if (expr->type != TTE_Literal || expr->litn.litv.littypetok != U64)
    return make_expr(UMINUS, 1, expr);

  // handle uminus literal combining, to patch the lexier cannot coping with
  // uminus value
  CALiteral *lit = &expr->litn.litv;
  const char *littext = symname_get(lit->textid);
  char buffer[1024] = "-";
  strcpy(buffer+1, littext);

  lit->textid = symname_check_insert(buffer);
  lit->littypetok = I64;

  return expr;
}

int add_struct_member(ST_ArgList *arglist, SymTable *st, CAVariable *var) {
  // just similar as add_fn_args,
  // TODO: combine with add_fn_args into one function
  int name = var->name;
  if (arglist->argc >= MAX_ARGS) {
    yyerror("line: %d, col: %d: too many struct members '%s', max member supports are %d",
	    glineno, gcolno, symname_get(name), MAX_ARGS);
    return -1;
  }

  STEntry *entry = sym_getsym(st, name, 0);
  if (entry) {
    yyerror("line: %d, col: %d: member '%s' already defined on line %d, col %d.",
	    var->loc.row, var->loc.col, symname_get(name), entry->sloc.row, entry->sloc.col);
    return -1;
  }

  entry = sym_insert(st, name, Sym_Member);
  entry->u.var = cavar_create(name, var->datatype);
  arglist->argnames[arglist->argc++] = name;
  return 0;
}

int add_tuple_member(ST_ArgList *arglist, typeid_t tid) {
  if (arglist->argc >= MAX_ARGS) {
    yyerror("line: %d, col: %d: too many struct members '%d', max member supports are `%d`",
	    glineno, gcolno, arglist->argc, MAX_ARGS);
    return -1;
  }

  arglist->types[arglist->argc++] = tid;
  return 0;
}

void reset_arglist_with_new_symtable() {
  SymTable *st = push_new_symtable();
  curr_arglist.argc = 0;
  curr_arglist.contain_varg = 0;
  curr_arglist.symtable = curr_symtable;
}

static STEntry *check_function_name(int id) {
  // check tuple type
  typeid_t fnname = sym_form_function_id(id);
  STEntry *entry = sym_getsym(&g_root_symtable, fnname, 0);

  if (entry)
    return entry;

  return NULL; 
}

ASTNode *make_struct_type(int id, ST_ArgList *arglist, int tuple) {
  dot_emit("struct_type_def", "IDENT");

  // see make_fn_proto
  arglist->symtable = curr_symtable;

  // popup the structure member symbol table
  // after that will define type name in it
  if (!tuple)
    curr_symtable = pop_symtable();

  // 0. check if current scope already exists such type and report error when already exists
  const char *structname = symname_get(id);
  if (check_function_name(id)) {
    yyerror("line: %d, col: %d: tuple '%s' already defined as function in previous",
	    glineno, gcolno, structname);
    return NULL;
  }

  typeid_t structtype = sym_form_type_id(id);
  STEntry *entry = sym_getsym(curr_symtable, structtype, 0);
  if (entry) {
    yyerror("line: %d, col: %d: type '%s' already defined",
	    glineno, gcolno, symname_get(id));
    return NULL;
  }

  CADataType *primtype = catype_get_primitive_by_name(structtype);
  if (primtype) {
    yyerror("line: %d, col: %d: struct type id `%s` cannot be primitive type",
	    glineno, gcolno, symname_get(id));
    return NULL;
  }

  ASTNode *p = new_ASTNode(TTE_Struct);
  entry = sym_insert(curr_symtable, structtype, Sym_DataType);
  entry->u.datatype.tuple = tuple;
  entry->u.datatype.id = structtype;
  entry->u.datatype.idtable = curr_symtable;
  entry->u.datatype.members = (ST_ArgList *)malloc(sizeof(ST_ArgList));

  // just remember the argument list and for later use
  *entry->u.datatype.members = *arglist;
  entry->sloc = (SLoc){glineno, gcolno};
  p->entry = entry;

  typeid_t fnid = sym_form_function_id(id);
  CallParamAux *paramaux = get_post_function(fnid);
  if (paramaux && !paramaux->checked && sym_is_sub_symtable(paramaux->param->symtable, curr_symtable)) {
    // check if current symtable is the parent of the request symtable
    delete_CallParamAux(paramaux);
    remove_post_function(fnid);
  }

  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &entry->sloc);
  return p;
}

ASTNode *make_as(ASTNode *expr, typeid_t type) {
  dot_emit("expr", "expr AS datatype");

  ASTNode *p = new_ASTNode(TTE_As);
  p->exprasn.type = type;
  p->exprasn.expr = expr;
  set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});

  ASTNode *node = make_expr(AS, 1, p);
  return node;
}

ASTNode *make_sizeof(typeid_t type) {
  ASTNode *p = make_id(type, TTEId_Type);
  ASTNode *node = make_expr(SIZEOF, 1, p);
  node->exprn.expr_type = sym_form_type_id_from_token(U64);
  return node;
}

typeid_t make_typeof(ASTNode *expr) { return sym_form_expr_typeof_id(expr); }

ASTNode *make_deref(ASTNode *expr) {
  ASTNode *node = make_expr(DEREF, 1, expr);
  return node;
}

ASTNode *make_address(ASTNode *expr) {
  ASTNode *node = make_expr(ADDRESS, 1, expr);
  return node;
}

int parse_tuple_fieldname(int fieldname) {
  const char *name = symname_get(fieldname);
  int len = strlen(name);
  int i = 0;
  for (; i < len; ++i) {
    if (name[i] < '0' || name[i] > '9')
      break;
  }

  if (i != len || (len > 1 && name[0] == '0'))
    yyerror("line: %d, col: %d: unknown field name `%s`", glineno, gcolno, name);

  fieldname = atoi(name);
  return fieldname;
}

StructFieldOp make_element_field(ASTNode *expr, int fieldname, int direct, int tuple) {
  if (tuple)
    fieldname = parse_tuple_fieldname(fieldname);

  StructFieldOp sfop = {expr, fieldname, direct, tuple};
  return sfop;
}

typedef struct ASTNodeList {
  ASTNode **stmtlist;
  int capacity;
  int len;
  struct ASTNodeList *next;
} ASTNodeList;

ASTNodeList *nodelisthead = NULL;
void put_astnode_into_list(ASTNode *stmt, int begin) {
  dot_emit("stmt_list", "stmt");

  if (begin) {
    // push a new list handle here when first encounter the list
    ASTNodeList *newlist = (ASTNodeList *)malloc(sizeof(ASTNodeList));
    newlist->len = 0;
    newlist->capacity = 10;
    newlist->stmtlist = (ASTNode **)malloc(sizeof(ASTNode *) * newlist->capacity);
    newlist->next = nodelisthead;
    nodelisthead = newlist;
  }

  // enhance capacity when needed
  if (nodelisthead->len == nodelisthead->capacity) {
    nodelisthead->capacity *= 2;
    nodelisthead->stmtlist = (ASTNode **)realloc(nodelisthead->stmtlist, sizeof(ASTNode *) * nodelisthead->capacity);
  }

  nodelisthead->stmtlist[nodelisthead->len++] = stmt;
}

ASTNode *make_stmt_list_zip() {
  dot_emit("stmt_list_star", "stmt_list_zip");

  // pop the previous list handle when reducing a handler
  ASTNodeList *oldlist = nodelisthead;
  nodelisthead = nodelisthead->next;

  if (oldlist->len == 1) {
    ASTNode *p = oldlist->stmtlist[0];
    free(oldlist->stmtlist);
    free(oldlist);
    return p;
  }

  int len = oldlist->len;
  ASTNode *p = new_ASTNode(TTE_StmtList);
  p->stmtlistn.nstmt = len;
  if ((p->stmtlistn.stmts = (ASTNode **)malloc(len * sizeof(ASTNode))) == NULL)
    yyerror("line: %d, col: %d: out of memory", glineno, gcolno);

  for (int i = 0; i < len; ++i)
    p->stmtlistn.stmts[i] = oldlist->stmtlist[i];

  free(oldlist->stmtlist);
  free(oldlist);

  const SLoc *beg = &(SLoc){glineno, gcolno};
  const SLoc *end = &(SLoc){glineno, gcolno};

  if (len == 1) {
    if (p->stmtlistn.stmts[0]) {
      beg = &p->stmtlistn.stmts[0]->begloc;
      end = &p->stmtlistn.stmts[0]->endloc;
    }
  } else if (len > 1){
    if (p->stmtlistn.stmts[0]) {
      beg = &p->stmtlistn.stmts[0]->begloc;
    }

    if (p->stmtlistn.stmts[len-1]) {
      end = &p->stmtlistn.stmts[len-1]->endloc;
    }
  }

  p->begloc = *beg;
  p->endloc = *end;

  p->symtable = curr_symtable;

  return p;
}

ArrayItem arrayitem_begin(ASTNode *expr) {
   void *handle = vec_new();
   vec_append(handle, expr);
   return (ArrayItem) {NULL, handle};
}

ArrayItem arrayitem_append(ArrayItem ai, ASTNode *expr) {
  vec_append(ai.indices, expr);
  return ai;
}

ArrayItem arrayitem_end(ArrayItem ai, ASTNode *arraynode) {
  ai.arraynode = arraynode;
  vec_reverse(ai.indices);
  return ai;
}

CAStructExpr structexpr_new() {
  CAStructExpr o = {typeid_novalue, 0, vec_new()};
  return o;
}

CAStructExpr structexpr_append(CAStructExpr sexpr, ASTNode *expr) {
  vec_append(sexpr.data, expr);
  return sexpr;
}

CAStructExpr structexpr_append_named(CAStructExpr sexpr, ASTNode *expr, int name) {
  CAStructNamed *s = (CAStructNamed *)malloc(sizeof(CAStructNamed));
  s->expr = expr;
  s->name = name;
  vec_append(sexpr.data, s);
  return sexpr; 
}

CAStructExpr structexpr_end(CAStructExpr sexpr, int name, int named) {
  typeid_t structtype = sym_form_type_id(name);
  sexpr.name = structtype;
  sexpr.named = named;
  return sexpr;
}

//void push_lexical_body() {}
//void pop_lexical_body() {}

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
  case TTE_As:
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
  fprintf(stderr, "[grammar line: %d, token: %d] ", yylineno, yychar);

  va_list ap;
  va_start(ap, s);
  int n = vfprintf(stderr, s, ap);
  va_end(ap);

  fprintf(stderr, "\n");
  exit(-1);
}

void caerror(SLoc *beg, SLoc *end, const char *s, ...) {
  fprintf(stderr, "[grammar line: %d, token: %d] ", yylineno, yychar);
  if (beg)
    fprintf(stderr, "line: %d, col: %d: ", beg->row, beg->col);

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
    curr_fn_symtable = NULL;
    if (enable_emit_main()) {
      main_fn_node = build_mock_main_fn_node();
      g_main_symtable = push_new_symtable();
      curr_fn_symtable = g_main_symtable;
    }

    return 0;
}

