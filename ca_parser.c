
#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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

void yyerror(const char *s, ...);
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
  static char label_buf[1024];
  sprintf(label_buf, "l:%s", name);
  return label_buf;
}

const char *sym_form_type_name(const char *name) {
  static char type_buf[1024];
  sprintf(type_buf, "t:%s", name);
  return type_buf;
}

const char *sym_form_function_name(const char *name) {
  static char type_buf[1024];
  sprintf(type_buf, "f:%s", name);
  return type_buf;
}

const char *sym_form_pointer_name(const char *name) {
  static char type_buf[1024];
  sprintf(type_buf, "t:*%s", name);
  return type_buf;
}

const char *sym_form_array_name(const char *name, int dimension) {
  static char type_buf[1024];
  sprintf(type_buf, "t:[%s;%d]", name, dimension);
  return type_buf;
}

typeid_t sym_form_expr_typeof_id(ASTNode *expr) {
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
  p->begloc = expr->begloc;
  p->endloc = expr->endloc;
  p->symtable = curr_symtable;
  return p;
}

ASTNode *make_stmt_print_datatype(typeid_t tid) {
  ASTNode *p = new_ASTNode(TTE_DbgPrintType);
  p->printtypen.type = tid;
  p->begloc = (SLoc){glineno, gcolno};
  p->endloc = (SLoc){glineno, gcolno};
  p->symtable = curr_symtable;
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
  return p;
}

ASTNode *make_stmt_ret() {
  /* if (curr_fn_rettype != sym_form_type_id_from_token(VOID)) */
  /*   yyerror("line: %d, col: %d: function have no return type", glineno, gcolno); */

  ASTNode *p = new_ASTNode(TTE_Ret);
  p->retn.expr = NULL;
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
void create_literal(CALiteral *lit, int textid, tokenid_t littypetok, tokenid_t postfixtypetok) {
  lit->textid = textid;
  lit->littypetok = littypetok;
  if (postfixtypetok == -1) {
    lit->fixed_type = 0;
    lit->datatype = typeid_novalue;
  } else {
    lit->fixed_type = 1;

    // here can directly determine literal type, it is postfixtypetok
    CADataType *catype = catype_get_primitive_by_token(postfixtypetok);
    determine_primitive_literal_type(lit, catype);
  }
}

void create_string_literal(CALiteral *lit, const LitBuffer *litb) {
  lit->fixed_type = 1;
  lit->littypetok = litb->typetok; // litb->typetok should be CSTRING;
  lit->textid = litb->text;
  lit->datatype = sym_form_type_id_from_token(litb->typetok);
  lit->u.strvalue.text = litb->text;
  lit->u.strvalue.len = litb->len;
}

void create_array_literal(CALiteral *lit, CAArrayLit arraylit) {
  lit->fixed_type = 0;
  lit->littypetok = ARRAY;
  lit->textid = -1;
  lit->datatype = typeid_novalue;
  lit->catype = NULL;
  lit->u.arrayvalue = arraylit;
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
  ASTNode *node = make_expr(ARRAY, 1, p);
  return node;
}

ASTNode *make_arrayitem_right(ASTNode *aitem) {
  // TODO:
  return NULL;
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

ASTNode *make_vardef(CAVariable *var, ASTNode *exprn, int global) {
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
    }
    // all else generate local variable against main or defined function
  }

  int id = var->name;
  STEntry *entry = sym_getsym(symtable, id, 0);
  if (entry) {
    yyerror("line: %d, col: %d: symbol '%s' already defined in scope on line %d, col %d.",
	    glineno, gcolno, symname_get(id), entry->sloc.row, entry->sloc.col);
    return NULL;
  }

  entry = sym_insert(symtable, id, Sym_Variable);
  entry->u.var = var;

  ASTNode *idn = make_id(id, TTEId_VarDef);
  idn->entry = entry;
  idn->symtable = symtable;

  ASTNode *p = new_ASTNode(TTE_Assign);
  p->assignn.id = idn;
  p->assignn.expr = exprn;
  p->begloc = idn->begloc;
  p->endloc = exprn->endloc;
  p->symtable = symtable;
  return p;
}

ASTNode *make_vardef_zero_value() {
  ASTNode *p = new_ASTNode(TTE_VarDefZeroValue);
  p->begloc = (SLoc){glineno, gcolno};
  p->endloc = (SLoc){glineno, gcolno};
  p->symtable = curr_symtable;
  return p;
}

ASTNode *make_assign(int id, ASTNode *exprn) {
  dot_emit("stmt", "varassign");

  STEntry *entry = sym_getsym(curr_symtable, id, 1);
  if (!entry) {
    yyerror("line: %d, col: %d: symbol '%s' not defined", glineno, gcolno, symname_get(id));
    return NULL;
  }

  ASTNode *idn = make_id(id, TTEId_VarAssign);
  idn->entry = entry;

  ASTNode *p = new_ASTNode(TTE_Assign);
  p->assignn.id = idn;
  p->assignn.expr = exprn;
  p->begloc = idn->begloc;
  p->endloc = exprn->endloc;
  p->symtable = curr_symtable;
  return p;
}

ASTNode *make_deref_left_assign(DerefLeft deleft, ASTNode *exprn) {
  ASTNode *derefln = make_deref_left(deleft);
  ASTNode *p = new_ASTNode(TTE_Assign);
  p->assignn.id = derefln;
  p->assignn.expr = exprn;
  p->begloc = derefln->begloc;
  p->endloc = exprn->endloc;
  p->symtable = curr_symtable;
  return p;
}

ASTNode *make_arrayitem_left_assign(ASTNode *left, ASTNode *expr) {
  // TODO:
  return NULL;
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

// TODO: check if these should return typeid_t or tokenid_t
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
  case FN_CALL: {
    // get function return type
    ASTNode *idn = node->exprn.operands[0];
    STEntry *entry = sym_getsym(&g_root_symtable, idn->idn.i, 0);
    type1 = entry->u.f.rettype;
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
    // only the variable can have an address
    // TODO: handle function address
    if (node->exprn.operands[0]->type != TTE_Id) {
      yyerror("line: %d, col: %d: only a variable can have an address, but find type `%d`",
	      node->begloc.row, node->begloc.col, node->exprn.operands[0]->type);
      return typeid_novalue;
    }

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
	if (possible_pointer_op) {
	  CADataType *catype1 = catype_get_by_name(node->symtable, type1);
	  CADataType *catype2 = catype_get_by_name(node->symtable, type);
	  pointer_op = (catype1->type == POINTER && is_integer_type(catype2->type));
	}

	if (!pointer_op) {
	  yyerror("line: %d, column: %d, expected `%s`, found `%s`",
		  node->begloc.row, node->begloc.col, symname_get(type1), symname_get(type));
	  return typeid_novalue;
	}
      }
    }
    break;
  }
  
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
  case TTE_Literal:
    type1 = inference_literal_type(&p->litn.litv);
    p->litn.litv.fixed_type = 1;
    return type1;
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
    type1 = inference_expr_type(p->ifn.bodies[0]);
    if (p->ifn.remain) {
      typeid_t type2 = inference_expr_type(p->ifn.remain);
      if (!catype_check_identical_in_symtable(p->symtable, type1, p->symtable, type2)) {
	yyerror("line: %d, col: %d: if expression type not same `%s` != `%s`",
		glineno, gcolno, symname_get(type1), symname_get(type2));
	return typeid_novalue;
      }
    }
    return type1;
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
  case FN_CALL: {
    // get function return type
    ASTNode *idn = node->exprn.operands[0];
    STEntry *entry = sym_getsym(&g_root_symtable, idn->idn.i, 0);
    catype_check_identical_in_symtable_witherror(node->symtable, type, node->symtable, entry->u.f.rettype, 1, &node->begloc);
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

    if (idnode->entry->u.var->datatype != datatype->pointer_layout->type->signature) {
      yyerror("line: %d, col: %d: variable address type `%s` cannot be type of `%s`",
	      node->begloc.row, node->begloc.col,
	      catype_get_type_name(idnode->entry->u.var->datatype),
	      catype_get_type_name(datatype->pointer_layout->type->signature));
      return -1;
    }

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
      if (!is_integer_type(secondca->type)) {
	yyerror("line: %d, col: %d: the 2nd pointer operand not support non-integer type, but find `%s`",
		node->begloc.row, node->begloc.col, catype_get_type_name(secondca->signature));
	return -1;
      }

      node->exprn.operands[1]->exprn.expr_type = secondca->signature;
      break;
    }
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
  //CHECK_GET_TYPE_VALUE(node, catype, type);
  tokenid_t typetok = catype ? catype->type : tokenid_novalue;
  typeid_t id;
  CADataType *exprcatype = NULL;
  switch(node->type) {
  case TTE_Literal:
    if (node->litn.litv.fixed_type)
      return 0;

    // here determine the literal type in this place compare to when create literal node
    determine_literal_type(&node->litn.litv, catype);
    break;
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
    determine_expr_expr_type(node->ifn.bodies[0], type);
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
// determined
int reduce_node_and_type(ASTNode *p, typeid_t *expr_types, int noperands) {
  // check if exist type in the each node and type is conflicting for each node
  // but here cannot create literal value when the value not determined a type
  // because it may be a tree, or can make the type by tranverlling the tree?
  // The answer is yes, here can determine the literal type when the expression
  // exists an fixed type part
  typeid_t type1 = typeid_novalue;
  int typei = 0;
  int notypeid = 0;
  int *nonfixed_node = (int *)alloca(noperands * sizeof(int));
  for (int i = 0; i < noperands; ++i) {
    if (expr_types[i] != typeid_novalue) {
      nonfixed_node[i] = 0;
      if (type1 == typeid_novalue) {
	type1 = expr_types[i];
	typei = i;
      } else if (!catype_check_identical_in_symtable(p->symtable, type1, p->symtable, expr_types[i])) {
	yyerror("line: %d, col: %d: type name conflicting: '%s' of type '%s', '%s' of type '%s'",
		p->begloc.col, p->begloc.row,
		get_node_name_or_value(p->exprn.operands[typei]), symname_get(type1),
		get_node_name_or_value(p->exprn.operands[i]), symname_get(expr_types[i]));
	return 0;
      }
    } else {
      nonfixed_node[i] = 1;
    }
  }

  // when the expression have any fixed type node
  for (int i = 0; i < noperands; ++i) {
    if (nonfixed_node[i]) {
      determine_expr_type(p->exprn.operands[i], type1);
    }
  }

  return type1;
}

int reduce_node_and_type_group(ASTNode **nodes, typeid_t *expr_types, int nodenum) {
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
      } else if (!catype_check_identical_in_symtable(nodes[i]->symtable, type1, nodes[i]->symtable, expr_types[i])) {
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

  ASTNode *p = new_ASTNode(TTE_FnDef);
  p->fndefn.fn_decl = decl;
  p->fndefn.stmts = NULL;

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

ASTNode *make_while(ASTNode *cond, ASTNode *whilebody) {
    dot_emit("stmt", "whileloop");

    ASTNode *p = new_ASTNode(TTE_While);
    p->whilen.cond = cond;
    p->whilen.body = whilebody;

    set_address(p, &cond->begloc, &whilebody->endloc);
    return p;
}

ASTNode *make_if(int isexpr, int argc, ...) {
  // NEXT TODO:
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

ASTNode *make_fn_proto(int fnid, ST_ArgList *arglist, typeid_t rettype) {
  dot_emit("fn_proto", "FN IDENT ...");

  typeid_t fnname = sym_form_function_id(fnid);

  curr_fn_rettype = rettype;

  SLoc beg = {glineno, gcolno};
  SLoc end = {glineno, gcolno};

  //void *carrier = get_post_function(fnname);
  // for handle post function declaration or define
  int existpost = exists_post_function(fnname);
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
    if (existpost)
      put_post_function(fnname, decln);

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
    if (existpost)
      put_post_function(fnname, defn);

    return defn;
  }
}

int check_fn_define(typeid_t fnname, ASTNode *param) {
  STEntry *entry = sym_getsym(&g_root_symtable, fnname, 0);
  if (!entry) {
    yyerror("line: %d, col: %d: function '%s' not defined", glineno, gcolno, symname_get(fnname));
    return -1;
  }

  if (entry->sym_type != Sym_FnDecl && entry->sym_type != Sym_FnDef) {
    yyerror("line: %d, col: %d: '%s' is not a function", glineno, gcolno, symname_get(fnname));
    return -1;
  }

  // check formal parameter and actual parameter
  ST_ArgList *formalparam = entry->u.f.arglists;

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

ASTNode *make_fn_call(int fnid, ASTNode *param) {
  dot_emit("fn_call", symname_get(fnid));

  typeid_t fnname = sym_form_function_id(fnid);

#ifdef __SUPPORT_BACK_TYPE__
  STEntry *entry = sym_getsym(&g_root_symtable, fnname, 0);
  if (entry) {
    check_fn_define(fnname, param);
  } else {
    put_post_function(fnname, NULL);
  }
#else
  check_fn_define(fnname, param);
#endif

  return make_expr(FN_CALL, 2, make_id(fnname, TTEId_FnName), param);
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

ASTNode *make_struct_type(int id, ST_ArgList *arglist) {
  dot_emit("struct_type_def", "IDENT");

  // see make_fn_proto
  arglist->symtable = curr_symtable;

  // popup the structure member symbol table
  // after that will define type name in it
  curr_symtable = pop_symtable();

  // 0. check if current scope already exists such type and report error when already exists
  const char *structname = symname_get(id);
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

#if 1
  ASTNode *p = new_ASTNode(TTE_Struct);
  entry = sym_insert(curr_symtable, structtype, Sym_DataType);
  entry->u.datatype.id = structtype;
  entry->u.datatype.idtable = curr_symtable;
  entry->u.datatype.members = (ST_ArgList *)malloc(sizeof(ST_ArgList));

  // just remember the argument list and for later use
  *entry->u.datatype.members = *arglist;
  entry->sloc = (SLoc){glineno, gcolno};
  p->entry = entry;
  
#else
  // 1. get the struct signature and check if it already exists in catype caches
  // when already exists then just use the type and skip step 2.
  const char *signature = sym_form_struct_signature(structname, curr_symtable);
  int symname = symname_check_insert(signature);

  // TODO: handle struct definition, should not need get type here directly
  CADataType *dt = catype_get_primitive_by_name(symname);
  if (!dt) {
    // 2. form a struct CADataType object when have no such cache
    // 2.a. put the CADataType into catype cache
    dt = catype_make_struct_type(symname, arglist);
  }

  // 3. put the struct CADataType into current scope Symbol table for later use
  entry = sym_insert(curr_symtable, structtype, Sym_DataType);
  entry->sloc = (SLoc){glineno, gcolno};
  entry->u.datatype = dt;
  
  // 4. create ASTNode object and set the object type as the datatype type
  // and set the CADataType object
  ASTNode *p = new_ASTNode(TTE_Struct);
  p->entry = entry;
#endif

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

ASTNode *make_element_field(ASTNode *node, int name) {
  // TODO:
  return NULL;
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

