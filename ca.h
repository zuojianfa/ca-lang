#ifndef __CA_H__
#define __CA_H__

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
  int value; /* value of literal */
} TLiteralNode;

/* identifiers */
typedef struct {
  int i; /* subscript to sym array */
} TIdNode;

/* operators */
typedef struct {
  int op;       /* operator */
  int noperand; /* number of operands */
  struct ASTNode **operands; /* operands */
} TExprNode;

typedef struct TFnDeclNode {
  int is_extern;  // is extern function
  int ret;        // specify if have return value, can extend into return different type
  int name;       // function name subscript to sym array
  ST_ArgList args;
} TFnDeclNode;

typedef struct TFnDefNode {
  struct ASTNode *fn_decl;
  struct ASTNode *stmts;
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

#endif

