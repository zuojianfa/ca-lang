#ifndef __CA_H__
#define __CA_H__

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
  TTE_Num,
} ASTNodeType;

/* following used to define the statement tree related structure */

struct ASTNode;

/* literals */
typedef struct {
  CALiteral litv; /* value of literal */
  int bg_type;    /* for inference the literal's type */
} TLiteralNode;

/* identifiers */
typedef struct {
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
  int expr_type;
  struct ASTNode **operands; /* operands */
} TExprNode;

typedef struct TFnDeclNode {
  int is_extern;   // is extern function
  CADataType *ret; // specify if have return value, can extend into return different type
  int name;        // function name subscript to sym array
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

typedef struct ASTNode {
  ASTNodeType type;      /* type of node */
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

void check_return_type(int fnrettype);
SymTable *push_new_symtable();
SymTable *pop_symtable();
int add_fn_args(SymTable *st, CAVariable *var);
int add_fn_args_actual(SymTable *st, ActualArg arg);
const char *label_name(const char *name);
int determine_expr_type(ASTNode *node, int typetok);
void determine_literal_type(CALiteral *lit, int typetok);
int get_expr_type_from_tree(ASTNode *node, int ispost);
int inference_expr_type(ASTNode *p);
void create_literal(CALiteral *lit, int textid, int littypetok, int postfixtypetok);
const char *get_node_name_or_value(ASTNode *node);


ASTNode *build_mock_main_fn_node();

int make_program();
void make_paragraphs(ASTNode *paragraph);
ASTNode *make_fn_def(ASTNode *proto, ASTNode *body);
ASTNode *make_fn_body(ASTNode *blockbody);
ASTNode *make_fn_decl(ASTNode *proto);
ASTNode *make_fn_args(ASTNode *args);
ASTNode *make_fn_args_ps(int varg);
void make_fn_args_actual(ActualArg *arg, ASTNode *expr);
ASTNode *make_stmt_print(ASTNode *expr);
ASTNode *make_stmt_expr(ASTNode *expr);
ASTNode *make_stmt_ret_expr(ASTNode *expr);
ASTNode *make_stmt_ret();
ASTNode *make_stmtexpr_list_block(ASTNode *exprblockbody);
ASTNode *make_exprblock_body(ASTNode *stmtexprlist);
ASTNode *make_stmtexpr_list(ASTNode *expr);
CADataType *make_instance_type_atomic(int atomictype);
CADataType *make_instance_type_struct(int structtype);
CADataType *make_ret_type_void();
void make_type_postfix(IdToken *idt, int id, int typetok);

ASTNode *make_label_node(int value);
ASTNode *make_goto_node(int i);
ASTNode *make_empty();
ASTNode *make_expr(int op, int noperands, ...);
ASTNode *make_expr_arglists(ST_ArgList *al);
ASTNode *make_expr_arglists_actual(ST_ArgListActual *al);
//ASTNode *make_fn_decl(int name, ST_ArgList *al, CADataType *rettype, SLoc beg, SLoc end);
//ASTNode *make_fn_define(int name, ST_ArgList *al, CADataType *rettype, SLoc beg, SLoc end);
ASTNode *make_id(int id);
ASTNode *make_vardef(CAVariable *var, ASTNode *exprn);
ASTNode *make_assign(int id, ASTNode *exprn);
ASTNode *make_goto(int labelid);
ASTNode *make_label_def(int labelid);
ASTNode *make_literal(CALiteral *litv);
ASTNode *make_while(ASTNode *cond, ASTNode *whilebody);
ASTNode *make_if(int isexpr, int argc, ...);
ASTNode *make_fn_proto(int id, CADataType *rettype);
ASTNode *make_fn_call(int fnname, ASTNode *param);
ASTNode *make_ident_expr(int id);

void freeNode(ASTNode *p);
NodeChain *node_chain(RootTree *tree, ASTNode *p);

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

