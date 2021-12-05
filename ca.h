#ifndef __CA_H__
#define __CA_H__

#include "ca_types.h"

#define __SUPPORT_BACK_TYPE__

#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

#include "symtable.h"
/*
  The relationship between the enum and data structure:
  TTE_Literal: TLiteralNode
  TTE_Id, TTE_Label, TTE_LabelGoto: TIdNode
  TTE_Expr: TExprNode
 */
typedef enum {
  TTE_Empty,       // for empty expression
  TTE_Literal,     // literal type
  TTE_Id,          // identifier type
  TTE_Label,       // label type
  TTE_LabelGoto,   // label type in goto statement
  TTE_Expr,        // operation type
  TTE_FnDecl,      // function declaration: [extern] fn fnname(fnarg1, fnarg2);
  TTE_FnDef,       // function definition
  TTE_While,
  TTE_If,
  TTE_As,
  TTE_Struct,      // struct definition
  TTE_Print,
  TTE_Ret,
  TTE_Assign,
  TTE_ArgList,
  TTE_StmtList,
  TTE_Num,
} ASTNodeType;

typedef enum {
  TTEId_Empty,
  TTEId_VarDef,    // when TTE_Id
  TTEId_VarAssign, // when TTE_Id
  TTEId_VarUse,    // when TTE_Id
  TTEId_FnName,    // when TTE_Id
  TTEId_Label,     // when TTE_Label
  TTEId_LabelGoto, // when TTE_LabelGoto
  TTEId_Num,
} IdType;

// this type is used for transferring grammar info (type) into later pass such
// as into walk_* routine to transfer information directly when needed, to let
// the walk routine know what it walking for. when needed add new type into this
// enum to tell walk routine it walking for and do special action according to
// the grammar information (type)
typedef enum {
  NGT_None,
  NGT_stmt_expr,
  NGT_Num,
} ASTNodeGrammartype;

/* following used to define the statement tree related structure */

struct ASTNode;

/* literals */
typedef struct {
  CALiteral litv; /* value of literal */
} TLiteralNode;

/* identifiers */
typedef struct {
  IdType idtype;
  int i; /* subscript to sym array */
} TIdNode;

/* operators */
typedef struct {
  int op;       /* operator */
  int noperand; /* number of operands */

  /* the expression's type: see borning_var_type. When the node not suitable for
     value type or there is no type determined for the expression then it have
     `0` value, or it is the intention type value may be used inference in later
     use
   */
  typeid_t expr_type;
  struct ASTNode **operands; /* operands */
} TExprNode;

typedef struct TFnDeclNode {
  int is_extern;   // is extern function
  typeid_t ret;    // specify the return type of the function, typeid_novalue stand for no return value
  //CADataType *ret; 
  typeid_t name;        // function name subscript to sym array
  ST_ArgList args;
} TFnDeclNode;

typedef struct TFnDefNode {
  struct ASTNode *fn_decl;
  struct ASTNode *stmts;
  void *retbb;   // opaque value for storing llvm::BasicBlock
  void *retslot; // opaque value for storing return value slot
} TFnDefNode;

typedef struct TWhileNode {
  struct ASTNode *cond;
  struct ASTNode *body;
} TWhileNode;

/* if else condition, when have no else scope remain is null, when have mutiple
   if else, then conds and bodies stores the nodes, ncond is count of if else
*/
typedef struct TIfNode {
  int ncond;
  int isexpr;
  struct ASTNode **conds;
  struct ASTNode **bodies;
  struct ASTNode *remain;
} TIfNode;

typedef struct TExprAsNode {
  struct ASTNode *expr;
  //CADataType *type;
  typeid_t type;
} TExprAsNode;

typedef struct TPrintNode {
  struct ASTNode *expr;
} TPrintNode;

typedef struct TRet {
  struct ASTNode *expr;
} TRet;

typedef struct TAssign {
  struct ASTNode *id;
  struct ASTNode *expr;
} TAssign;

typedef struct TArgList {
  int argc; /* number of arguments */
  struct ASTNode **exprs; /* operands */
} TArgList;

typedef struct TStmtList {
  int nstmt;
  struct ASTNode **stmts;
} TStmtList;

typedef struct ASTNode {
  ASTNodeType type;      /* type of node */
  ASTNodeGrammartype grammartype; /* grammartype for transfer grammar info into node */
  SymTable *symtable;    /* the scoped symbol table */
  STEntry *entry;
  SLoc begloc;           /* the source code begin location of code unit */
  SLoc endloc;           /* the source code end location of code unit */
  int seq;
  union {
    TLiteralNode litn;   /* literals */
    TIdNode idn;         /* identifiers, include label, goto identifier */
    TExprNode exprn;     /* operators */
    TFnDeclNode fndecln; /* function declaration */
    TFnDefNode fndefn;   /* function definition */
    TWhileNode whilen;   /* while statement */
    TIfNode ifn;         /* if statement */
    TExprAsNode exprasn; /* as statement */
    TPrintNode printn;   /* print statement */
    TRet retn;           /* return statement */
    TAssign assignn;     /* assigment value */
    TArgList arglistn;   /* actual argument list */
    TStmtList stmtlistn; /* statement list */
  };
} ASTNode;

typedef struct NodeChain {
  struct NodeChain *next;
  ASTNode *node;
} NodeChain;

typedef struct RootTree {
  NodeChain *head;
  NodeChain *tail;
  SymTable *root_symtable;
  SLoc begloc_prog;
  SLoc endloc_prog;
  SLoc begloc_main;
  SLoc endloc_main;
  int count;
} RootTree;

int reduce_node_and_type(ASTNode *p, typeid_t *expr_types, int noperands);
int reduce_node_and_type_group(ASTNode **nodes, typeid_t *expr_types, int nodenum);
int parse_lexical_char(const char *text);
int enable_emit_main();
void check_return_type(typeid_t fnrettype);
SymTable *push_new_symtable();
SymTable *push_symtable(SymTable *st);
SymTable *pop_symtable();
int add_fn_args(ST_ArgList *arglist, SymTable *st, CAVariable *var);
int add_fn_args_actual(SymTable *st, ASTNode *arg);
const char *sym_form_label_name(const char *name);
const char *sym_form_type_name(const char *name);
const char *sym_form_function_name(const char *name);
const char *sym_form_struct_signature(const char *name, SymTable *st);
typeid_t sym_form_type_id(int id, int islabel);
typeid_t sym_form_function_id(int fnid);
typeid_t sym_form_type_id_from_token(tokenid_t tok);
tokenid_t sym_primitive_token_from_id(typeid_t id);

int determine_expr_type(ASTNode *node, typeid_t type);
int get_expr_type_from_tree(ASTNode *node, int ispost);
typeid_t inference_expr_type(ASTNode *p);
void create_literal(CALiteral *lit, int textid, tokenid_t littypetok, tokenid_t postfixtypetok);
const char *get_node_name_or_value(ASTNode *node);

ASTNode *build_mock_main_fn_node();

int make_attrib_scope(int attrfn, int attrparam);
int make_program();
void make_paragraphs(ASTNode *paragraph);
ASTNode *make_fn_def(ASTNode *proto, ASTNode *body);
ASTNode *make_fn_body(ASTNode *blockbody);
ASTNode *make_fn_decl(ASTNode *proto);
void add_fn_args_p(ST_ArgList *arglist, int varg);
ASTNode *make_stmt_print(ASTNode *expr);
ASTNode *make_stmt_expr(ASTNode *expr);
ASTNode *make_stmt_ret_expr(ASTNode *expr);
ASTNode *make_stmt_ret();
ASTNode *make_stmtexpr_list_block(ASTNode *exprblockbody);
ASTNode *make_stmtexpr_list(ASTNode *stmts, ASTNode *expr);
//CADataType *make_instance_type_atomic(int atomictype);
CADataType *make_pointer_type(CADataType *datatype);
CADataType *make_array_type(CADataType *type, LitBuffer *size);
CADataType *get_datatype_by_ident(int name);
ASTNode *make_type_def(int name, CADataType *type);
CADataType *make_instance_type_struct(int structtype);
typeid_t make_ret_type_void();
void make_type_postfix(IdToken *idt, int id, int typetok);

ASTNode *make_label_node(int value);
ASTNode *make_goto_node(int i);
ASTNode *make_empty();
ASTNode *make_expr(int op, int noperands, ...);
ASTNode *make_expr_arglists_actual(ST_ArgListActual *al);
//ASTNode *make_fn_decl(int name, ST_ArgList *al, CADataType *rettype, SLoc beg, SLoc end);
//ASTNode *make_fn_define(int name, ST_ArgList *al, CADataType *rettype, SLoc beg, SLoc end);
ASTNode *make_id(int id, IdType idtype);
ASTNode *make_vardef(CAVariable *var, ASTNode *exprn, int global);
ASTNode *make_assign(int id, ASTNode *exprn);
ASTNode *make_goto(int labelid);
ASTNode *make_label_def(int labelid);
ASTNode *make_literal(CALiteral *litv);
ASTNode *make_while(ASTNode *cond, ASTNode *whilebody);
ASTNode *make_if(int isexpr, int argc, ...);
ASTNode *make_fn_proto(int fnid, ST_ArgList *arglist, typeid_t type);
ASTNode *make_fn_call(int fnid, ASTNode *param);
ASTNode *make_ident_expr(int id);
ASTNode *make_as(ASTNode *expr, typeid_t type);
ASTNode *make_stmt_list_zip();
int check_fn_define(typeid_t fnname, ASTNode *param);
// for tree node compress deep into wide, begin for stmt list beginning
void put_astnode_into_list(ASTNode *stmt, int begin);

int add_struct_member(ST_ArgList *arglist, SymTable *st, CAVariable *var);
ASTNode *make_struct_type(int id, ST_ArgList *arglist);

void freeNode(ASTNode *p);
NodeChain *node_chain(RootTree *tree, ASTNode *p);

#ifdef __cplusplus
END_EXTERN_C
#endif

#define CHECK_GET_TYPE_VALUE(p, value, id)			\
  do {								\
    if (!(value)) {						\
      const char *name = symname_get(id);			\
      yyerror("line: %d, col: %d: cannot find data type '%s'",	\
	      (p)->begloc.row, (p)->begloc.col, name);		\
    }								\
  } while(0)

#endif

