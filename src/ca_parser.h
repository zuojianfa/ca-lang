#ifndef __CA_H__
#define __CA_H__

#include "ca_types.h"

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
  TTE_DbgPrint,
  TTE_DbgPrintType,
  TTE_Ret,
  TTE_Assign,
  TTE_ArgList,
  TTE_StmtList,
  TTE_TypeDef,
  TTE_VarDefZeroValue, // the value of `__zero_init__` to specify initial all with 0, carry nothing
  TTE_ArrayDef,
  TTE_DerefLeft,
  TTE_ArrayItemLeft,
  TTE_ArrayItemRight,
  TTE_StructFieldOpLeft,
  TTE_StructFieldOpRight,
  TTE_StructExpr,
  TTE_LexicalBody,
  TTE_Loop,
  TTE_Break,
  TTE_Continue,
  TTE_For,
  TTE_Box,
  TTE_Drop,
  TTE_LetBind,
  TTE_Range,
  TTE_FnDefImpl,
  TTE_Domain,
  TTE_TraitFn,
  TTE_Num,
} ASTNodeType;

typedef enum {
  TTEId_Empty,
  TTEId_VarDef,    // when TTE_Id
  TTEId_VarAssign, // when TTE_Id
  TTEId_VarUse,    // when TTE_Id
  TTEId_FnName,    // when TTE_Id
  TTEId_Type,      // when TTE_Id, the symname of type name
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

typedef struct {
  CAArrayExpr aexpr;
} TArrayNode;

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
  void *conds;
  void *bodies;
  struct ASTNode *remain;
} TIfNode;

typedef struct TExprAsNode {
  struct ASTNode *expr;
  typeid_t type;
} TExprAsNode;

typedef struct TPrintNode {
  struct ASTNode *expr;
} TPrintNode;

typedef struct TPrintTypeNode {
  typeid_t type;
} TPrintTypeNode;

typedef struct TTypeDef {
  typeid_t newtype;
  typeid_t type;
} TTypeDef;

typedef struct TRet {
  struct ASTNode *expr;
} TRet;

typedef struct TAssign {
  struct ASTNode *id;
  int op; // when op is not -1, then it is the assignment operation
  struct ASTNode *expr;
} TAssign;


typedef struct TLetBind {
  CAPattern *cap;
  struct ASTNode *expr;
} TLetBind;

typedef struct TRange {
  GeneralRange range;
} TRange;

typedef struct TArgList {
  int argc; /* number of arguments */
  struct ASTNode **exprs; /* operands */
} TArgList;

typedef struct TStmtList {
  int nstmt;
  struct ASTNode **stmts;
} TStmtList;

typedef struct TypeImplInfo {
  int class_id;
  int trait_id;
  // for control the common function recursive define count, when count > 0 then it is implement
  // common function, else implement the struct method, and they will use different name convention
  int fn_def_recursive_count;
} TypeImplInfo;

typedef struct TFnDefNodeImpl {
  TypeImplInfo impl_info;
  int count;
  void *data; // vector, each array element occupies one vector item
} TFnDefNodeImpl;

typedef struct DomainNames {
  int relative; // whether it is relative search path
  int count;
  void *parts; // vector, each domain part occupies one vector item
} DomainNames, TDomainNames;

typedef struct DerefLeft {
  int derefcount;
  struct ASTNode *expr;
} DerefLeft, TDerefLeft;

typedef struct ArrayItem {
  //int varname;
  struct ASTNode *arraynode;

  // the indices into the array `arraynode`, it may be multiple like:
  // `let arr: [[[i32;7];8];9] = ???; arr[3][4]`
  void *indices;
} ArrayItem, TArrayItem;

typedef struct StructFieldOp {
  struct ASTNode *expr; // the expr may also be a expression with this struct
  int fieldname;
  int direct; // direct: . op, indirect: -> op
  int tuple;  // 1: when is tuple, fieldname is the parsed numeric field name, 0: when is not tuple
} StructFieldOp, TStructFieldOp;

typedef enum LVType {
  LVT_Var,
  LVT_Deref,
  LVT_ArrayItem,
  LVT_StructOp,
} LVType;

typedef struct LeftValueId {
  LVType type;
  union {
    int var;
    DerefLeft deleft; /* represent dereference left value */
    ArrayItem aitem;  /* array item */
    StructFieldOp structfieldop; /* struct field operation */
  };
} LeftValueId;

typedef struct ForStmtId {
  int vartype; // 0: for value, '*': for pointer, '&': for reference
  int var;
} ForStmtId;

typedef struct TLexicalBody {
  struct ASTNode *stmts;
  struct ASTNode *fnbuddy; /* if not null it is also a function body */
} TLexicalBody;

typedef struct TLoop {
  struct ASTNode *body;
} TLoop;

typedef struct TFor {
  ForStmtId var;
  struct ASTNode *listnode;
  struct ASTNode *body;
} TFor;

typedef struct TBox {
  struct ASTNode *expr;
} TBox;

typedef struct TDrop {
  int var; /* subscript to sym array */
} TDrop;

typedef enum VarInitType {
  VarInit_Zero,   // zero filled
  VarInit_NoInit, // no initialize, just use the memory value by what it is
} VarInitType;

typedef struct TVarInit {
  VarInitType type;
} TVarInit;

typedef struct ASTNode {
  ASTNodeType type;      /* type of node */
  ASTNodeGrammartype grammartype; /* grammartype for transfer grammar info into node */
  SymTable *symtable;    /* the scoped symbol table */
  STEntry *entry;        /* when type is TTE_Id, it can speed up lookup, may can be removed later */
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
    TPrintTypeNode printtypen;   /* print type statement */
    TTypeDef typedefn;   /* handle typedef, just for type checking */
    TRet retn;           /* return statement */
    TAssign assignn;     /* assigment value */
    TArgList arglistn;   /* actual argument list */
    TStmtList stmtlistn; /* statement list */
    TArrayNode anoden;   /* array expresssion node */
    TDerefLeft deleftn;  /* dereference left node */
    TArrayItem aitemn;   /* array item operation: left or right */
    TStructFieldOp sfopn;/* struct field operation */
    CAStructExpr snoden; /* struct expression definition */
    TLexicalBody lnoden; /* lexical scope body */
    TLoop loopn;         /* loop node */
    TFor forn;           /* for node */
    TBox boxn;           /* box node */
    TDrop dropn;         /* drop node */
    TLetBind letbindn;   /* the binding operation for let */
    TRange rangen;       /* range node */
    TVarInit varinitn;
    TFnDefNodeImpl fndefn_impl; /* for function definition in type impl */
    TDomainNames domainn; /* for domain method call */
    TTraitFnList traitfnlistn; /* trait function list */
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

typedef struct CallParamAux {
  int checked;
  ASTNode *param;
} CallParamAux;

typeid_t reduce_node_and_type_group(ASTNode **nodes, typeid_t *expr_types, int nodenum, int assignop);
int parse_lexical_char(const char *text);
int parse_tuple_fieldname(int fieldname);
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
typeid_t sym_form_symtable_type_id(SymTable *st, typeid_t name);
typeid_t sym_form_expr_typeof_id(ASTNode *expr);
typeid_t sym_form_type_id(int id);
typeid_t sym_form_type_id_by_str(const char *idname);
typeid_t sym_form_label_id(int id);
typeid_t sym_form_function_id(int fnid);
typeid_t sym_form_method_id(int fnid, TypeImplInfo *type_impl);
typeid_t sym_form_pointer_id(typeid_t type);
typeid_t sym_form_array_id(typeid_t type, int dimension);
typeid_t sym_form_tuple_id(typeid_t *types, int argc);
typeid_t sym_form_type_id_from_token(tokenid_t tok);
tokenid_t sym_primitive_token_from_id(typeid_t id);
ASTNode *astnode_unwind_from_addr(const char *addr, int *len);

int determine_expr_type(ASTNode *node, typeid_t type);
int get_expr_type_from_tree(ASTNode *node);
typeid_t inference_expr_type(ASTNode *p);
void create_literal(CALiteral *lit, int textid, tokenid_t littypetok, tokenid_t postfixtypetok);
void create_string_literal(CALiteral *lit, const LitBuffer *litb);
const char *get_node_name_or_value(ASTNode *node);

ASTNode *build_mock_main_fn_node();

int make_attrib_scope(int attrfn, int attrparam);
int make_program();
void make_paragraphs(ASTNode *paragraph);
ASTNode *make_fn_def(ASTNode *proto, ASTNode *body);
ASTNode *make_fn_body(ASTNode *blockbody);
ASTNode *make_fn_def_impl_begin(ASTNode *fndef);
ASTNode *make_fn_def_impl_next(ASTNode *impl, ASTNode *fndef);
ASTNode *trait_fn_begin(ASTNode *fndef_proto);
ASTNode *trait_fn_next(ASTNode *fnlist, ASTNode *fndef_proto);
ASTNode *make_trait_defs(int trait_id, ASTNode *defs);
ASTNode *make_fn_decl(ASTNode *proto);
void add_fn_args_p(ST_ArgList *arglist, int varg);
ASTNode *make_stmt_print(ASTNode *expr);
ASTNode *make_stmt_print_datatype(typeid_t tid);
ASTNode *make_stmt_expr(ASTNode *expr);
ASTNode *make_stmt_ret_expr(ASTNode *expr);
ASTNode *make_stmt_ret();
ASTNode *make_stmtexpr_list_block(ASTNode *exprblockbody);
ASTNode *make_stmtexpr_list(ASTNode *stmts, ASTNode *expr);
typeid_t make_pointer_type(typeid_t datatype);
typeid_t make_array_type(typeid_t type, LitBuffer *size);
typeid_t make_tuple_type(ST_ArgList *arglist);
ASTNode *make_type_def(int name, typeid_t type);
typeid_t make_ret_type_void();
void make_type_postfix(IdToken *idt, int id, int typetok);

ASTNode *make_label_node(int value);
ASTNode *make_goto_node(int i);
ASTNode *make_empty();
ASTNode *make_expr(int op, int noperands, ...);
ASTNode *make_expr_arglists_actual(ST_ArgListActual *al);
ASTNode *make_id(int id, IdType idtype);
ASTNode *make_global_vardef(CAVariable *var, ASTNode *exprn, int global);
ASTNode *make_let_stmt(CAPattern *cap, ASTNode *exprn);
ASTNode *make_vardef_zero_value(VarInitType init_type);
ASTNode *make_assign(LeftValueId *lvid, ASTNode *exprn);
ASTNode *make_assign_op(LeftValueId *lvid, int op, ASTNode *exprn);
ASTNode *make_goto(int labelid);
ASTNode *make_label_def(int labelid);
ASTNode *make_literal(CALiteral *litv);
ASTNode *make_general_range(GeneralRange *range);
ASTNode *make_array_def(CAArrayExpr expr);
CAArrayExpr make_array_def_fill(ASTNode *expr, CALiteral *literal);
ASTNode *make_struct_expr(CAStructExpr expr);
ASTNode *make_tuple_expr(CAStructExpr expr);
ASTNode *make_arrayitem_right(ArrayItem ai);
ASTNode *make_structfield_right(StructFieldOp sfop);
ASTNode *make_boxed_expr(ASTNode *expr);
ASTNode *make_drop(int id);
ASTNode *make_break();
ASTNode *make_continue();
ASTNode *make_loop(ASTNode *loopbody);
void make_for_var_entry(int id);
ASTNode *make_for(ForStmtId id, ASTNode *listnode, ASTNode *stmts);
ASTNode *make_for_stmt(ForStmtId id, ASTNode *listnode, ASTNode *stmts);
ASTNode *make_while(ASTNode *cond, ASTNode *whilebody);
ASTNode *new_ifstmt_node();
ASTNode *make_ifpart(ASTNode *p, ASTNode *cond, ASTNode *body);
ASTNode *make_elsepart(ASTNode *p, ASTNode *body);
ASTNode *make_if(int isexpr, int argc, ...);
ASTNode *make_fn_proto(int fnid, ST_ArgList *arglist, typeid_t type);
ASTNode *make_fn_call_or_tuple(int fnid, ASTNode *param);
ASTNode *make_method_call(StructFieldOp fieldop, ASTNode *param);
DomainNames domain_init(int relative, int name);
void domain_append(DomainNames *names, int name);
ASTNode *make_domain_call(DomainNames *names, ASTNode *param);
ASTNode *make_gen_tuple_expr(ASTNode *param);
ASTNode *make_ident_expr(int id);
ASTNode *make_uminus_expr(ASTNode *expr);
ASTNode *make_as(ASTNode *expr, typeid_t type);
ASTNode *make_sizeof(typeid_t type);
typeid_t make_typeof(ASTNode *node);
ASTNode *make_deref(ASTNode *expr);
ASTNode *make_address(ASTNode *expr);
StructFieldOp make_element_field(ASTNode *node, int fieldname, int direct, int tuple);
ASTNode *make_stmt_list_zip();
ArrayItem arrayitem_begin(ASTNode *expr);
ArrayItem arrayitem_append(ArrayItem ai, ASTNode *expr);
ArrayItem arrayitem_end(ArrayItem ai, ASTNode *arraynode);
CAStructExpr structexpr_new();
CAStructExpr structexpr_append(CAStructExpr sexpr, ASTNode *expr);
CAStructExpr structexpr_append_named(CAStructExpr sexpr, ASTNode *expr, int name);
CAStructExpr structexpr_end(CAStructExpr sexpr, int name, int named);
TypeImplInfo begin_impl_type(int class_id);
TypeImplInfo begin_impl_trait_for_type(int class_id, int trait_id);
void push_type_impl(TypeImplInfo *new_info);
void pop_type_impl();

int check_fn_define(typeid_t fnname, ASTNode *param, int tuple, STEntry *entry, int is_method);
// for tree node compress deep into wide, begin for stmt list beginning
void put_astnode_into_list(ASTNode *stmt, int begin);

int add_struct_member(ST_ArgList *arglist, SymTable *st, CAVariable *var);
int add_tuple_member(ST_ArgList *arglist, typeid_t tid);
void reset_arglist_with_new_symtable();
ASTNode *make_struct_type(int id, ST_ArgList *arglist, int tuple);

//void push_lexical_body();
//void pop_lexical_body();
ASTNode *make_lexical_body(ASTNode *stmts);

void freeNode(ASTNode *p);
NodeChain *node_chain(RootTree *tree, ASTNode *p);

void yyerror(const char *s, ...);
void caerror(const SLoc *beg, const SLoc *end, const char *s, ...);

typedef CADataType *(*query_type_fn_t)(TStructFieldOp *sfopn, void **self_value);

#ifdef __cplusplus
END_EXTERN_C
#endif

#define CHECK_GET_TYPE_VALUE(p, value, id)				\
  do {									\
    if (!(value)) {							\
      const char *name_ = symname_get(id);				\
      caerror(&((p)->begloc), &((p)->endloc), "cannot find data type '%s'", name_); \
    }									\
  } while(0)

#define CHECK_GET_TYPE_VALUE_LOC(loc, value, id)			\
  do {									\
    if (!(value)) {							\
      const char *name_ = symname_get(id);				\
      caerror(&(loc), NULL, "cannot find data type '%s'", name_);	\
    }									\
  } while(0)

const static char *OSELF = "self";
const static char *CSELF = "Self";

#endif

