%{

#include <stdio.h>
#include <stdlib.h>
#include "ca.h"
#include "dotgraph.h"
#include "symtable.h"

/* TODO: support redefinition variable in the same scope: { let a: int = 2; let a: double = 4.3; }
TODO: symbol table also stores the semantic information such as symbol type, width, default value, etc. But the syntax tree (ST) stores syntax information. For error recovery processing, you can construct ST without considering the semantic meanings even when existing semantic errors, so in this time the compiler can report semantic error or even modification suggestions, while continuary construct ST. the procedure becomes: Construct syntax tree NOT STOP, do semantic checking, report error.
*/

int yylex(void);
void yyerror(const char *s, ...);

extern RootTree *gtree;

/* the root symbol table for global symbols and layer 0 statement running */
extern SymTable g_root_symtable;
extern SymTable *curr_symtable;

/* mainly for label processing, because label is function scope symbol */
extern SymTable *curr_fn_symtable;

extern int borning_var_type;
extern int extern_flag;
extern ST_ArgList curr_arglist;

extern int glineno;
extern int gcolno;

%}/* symbol table */

%define parse.error detailed

%union {
  CADataType *datatype;
  CAVariable *var;
  CALiteral litv;   /* literal value */
  LitBuffer litb;   /* literal buffer */
  IdToken idtok;    /* return type token */
  int symnameid;    /* symbol table index */
  ASTNode *astnode; /* node pointer */
  ActualArg arg;    /* argument */
};

%token	<litb>		LITERAL
%token	<symnameid>	VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR STRUCT ATOMTYPE_END
%token	<symnameid>	IDENT
%token			WHILE IF IFE PRINT GOTO EXTERN FN RET LET EXTERN_VAR
%token			ARG_LISTS ARG_LISTS_ACTUAL FN_DEF FN_CALL VARG COMMENT EMPTY_BLOCK
%token			ARROW INFER TYPE
%nonassoc		IFX
%nonassoc		ELSE
%left			GE LE EQ NE '>' '<'
%left			'+' '-'
%left			'*' '/'
%left			AS
%nonassoc		UMINUS
%type	<litv>		literal lit_field lit_field_list lit_struct_def
%type	<astnode>	stmt expr stmt_list stmt_list_block label_def paragraphs fn_def fn_decl
%type	<astnode>	paragraph fn_proto fn_args fn_call fn_body fn_args_call
%type			fn_args_p fn_args_call_p
%type	<astnode>	ifstmt stmt_list_star block_body
%type	<astnode>	ifexpr stmtexpr_list_block exprblock_body stmtexpr_list
%type	<var>		iddef iddef_typed
%type	<arg>		fn_args_actual
%type	<symnameid>	label_id
%type	<symnameid>	atomic_type struct_type
%type	<idtok>		type_postfix
%type	<datatype>	datatype instance_type ret_type

%%

program:	paragraphs { return make_program(); }
		;

paragraphs:	paragraphs paragraph { make_paragraphs($2); }
	|       {dot_emit("paragraphs", ""); /*empty */ }
		;

paragraph:     	stmt     { dot_emit("paragraph", "stmt"); }
	|	fn_def   { dot_emit("paragraph", "fn_def"); }
	|	fn_decl  { dot_emit("paragraph", "fn_decl"); }
		;

fn_def:		fn_proto fn_body { $$ = make_fn_def($1, $2); }
		;

fn_body:	block_body { $$ = make_fn_body($1); }
		;

fn_decl: 	EXTERN { extern_flag = 1; } fn_proto ';' { $$ = make_fn_decl($3); }
		;

fn_call:	IDENT '(' fn_args_call ')' { $$ = make_fn_call($1, $3); }
		;

fn_proto:	FN IDENT
		{
		    SymTable *st = push_new_symtable();
		    if (!extern_flag) {
			/* begin processing a new function, so create new symbol table */
			curr_fn_symtable = st;
		    }

		    curr_arglist.argc = 0;
		    curr_arglist.symtable = curr_symtable;
		}
		'(' fn_args ')' ret_type { $$ = make_fn_proto($2, &curr_arglist, $7); }
		;

fn_args:	fn_args_p              { add_fn_args_p(&curr_arglist, 0); }
	|	fn_args_p ',' VARG     { add_fn_args_p(&curr_arglist, 1); }
	|                              { }
		;

fn_args_p:	fn_args_p ',' iddef_typed    { add_fn_args(&curr_arglist, curr_symtable, $3); }
	|	iddef_typed                  { add_fn_args(&curr_arglist, curr_symtable, $1); }
	;

fn_args_call:	{ actualarglist_new_push(); }
		fn_args_call_p { $$ = make_expr_arglists_actual(actualarglist_current()); }
	|	{ $$ = make_expr(ARG_LISTS_ACTUAL, 0); }
		;

fn_args_call_p:	fn_args_call_p ',' fn_args_actual { add_fn_args_actual(curr_symtable, $3); }
	|	fn_args_actual { add_fn_args_actual(curr_symtable, $1); }
	;

fn_args_actual: expr { make_fn_args_actual(&$$, $1); }
		;

stmt:		';'			{ $$ = make_expr(';', 2, NULL, NULL); }
	|	expr ';'                { $$ = make_stmt_expr($1); }
	|	PRINT expr ';'          { $$ = make_stmt_print($2); }
	|	RET expr ';'            { $$ = make_stmt_ret_expr($2); }
	|	RET ';'		        { $$ = make_stmt_ret(); }
	|	LET iddef '=' expr ';'  { $$ = make_vardef($2, $4); }
	|	IDENT '=' expr ';'      { $$ = make_assign($1, $3); }
	|	WHILE '(' expr ')' stmt_list_block { $$ = make_while($3, $5); }
	|	IF '(' expr ')' stmt_list_block %prec IFX { $$ = make_if(0, 2, $3, $5); }
	|	ifstmt                  { dot_emit("stmt", "ifstmt"); $$ = $1; }
	|	stmt_list_block         { dot_emit("stmt", "stmt_list_block"); $$ = $1; }
	|	label_def               { dot_emit("stmt", "label_def"); $$ = $1; }
	|	GOTO label_id ';'       { $$ = make_goto($2); }
		;

ifstmt:		IF '(' expr ')' stmt_list_block ELSE stmt_list_block    { $$ = make_if(0, 3, $3, $5, $7); }
		;
//////////////////////////////////////////////////////////////
/* TODO: realize the conflict problem when open the stmt expression using `IF` not `IFE` */
ifexpr:		IFE '(' expr ')' stmtexpr_list_block ELSE stmtexpr_list_block { $$ = make_if(1, 3, $3, $5, $7); }
		;

stmtexpr_list_block: { SymTable *st = push_new_symtable(); }
		exprblock_body { $$ = make_stmtexpr_list_block($2); }
		;
exprblock_body: '{' stmtexpr_list '}'               { $$ = make_exprblock_body($2); }
		;

stmtexpr_list: 	stmt_list expr { $$ = make_stmtexpr_list($2); /* TODO: put stmt_list into list */ }
	|	expr { $$ = make_stmtexpr_list($1); }
		;
/**/
//////////////////////////////////////////////////////////////
stmt_list_block: { SymTable *st = push_new_symtable(); }
		block_body { $$ = make_stmtexpr_list_block($2); }
		;

block_body: 	'{'stmt_list_star '}'               { $$ = make_exprblock_body($2); }
		;

stmt_list_star:	stmt_list                           { dot_emit("stmt_list_star", "stmt_list"); $$ = $1; }
	|	                                    { $$ = make_empty(); /* empty */}
		;

stmt_list:     	stmt                  { dot_emit("stmt_list", "stmt"); $$ = $1; }
	|	stmt_list stmt        { $$ = make_expr(';', 2, $1, $2); }
		;

label_def:	label_id ':'          { $$ = make_label_def($1); }
		;

label_id:	IDENT		      { dot_emit("label_id", "IDENT"); $$ = $1; }
		;

expr:     	literal               { $$ = make_literal(&$1); }
	|	IDENT                 { $$ = make_ident_expr($1); }
	|	'-'expr %prec UMINUS  { $$ = make_expr(UMINUS, 1, $2); }
	|	expr '+' expr         { $$ = make_expr('+', 2, $1, $3); }
	|	expr '-' expr         { $$ = make_expr('-', 2, $1, $3); }
	|	expr '*' expr         { $$ = make_expr('*', 2, $1, $3); }
	|	expr '/' expr         { $$ = make_expr('/', 2, $1, $3); }
	|	expr '<' expr         { $$ = make_expr('<', 2, $1, $3); }
	|	expr '>' expr         { $$ = make_expr('>', 2, $1, $3); }
	|	expr GE expr          { $$ = make_expr(GE, 2, $1, $3); }
	|	expr LE expr          { $$ = make_expr(LE, 2, $1, $3); }
	|	expr NE expr          { $$ = make_expr(NE, 2, $1, $3); }
	|	expr EQ expr          { $$ = make_expr(EQ, 2, $1, $3); }
	|	'('expr ')'           { dot_emit("expr", "'(' expr ')'"); $$ = $2; }
	|	fn_call               { dot_emit("expr", "fn_call"); $$ = $1; }
	|	ifexpr                { dot_emit("expr", "ifexpr"); $$ = $1; }
	|	expr AS datatype      { dot_emit("expr", "expr AS datatype"); $$ = $1; /* TODO: handle AS expression */ }
		;

datatype:	instance_type         { dot_emit("datatype", "instance_type"); $$ = $1; }
	|	pointer_type          { dot_emit("datatype", "pointer_type"); }
	;

instance_type:	atomic_type { $$ = make_instance_type_atomic($1); }
	|	struct_type { $$ = make_instance_type_struct($1); }
		;

atomic_type:	VOID | I32 | I64 | U32 | U64 | F32 | F64 | BOOL | CHAR | UCHAR
		    // { /* TODO: handle user defined type */ }
		;

struct_type:	IDENT { dot_emit("struct_type", "IDENT"); $$ = $1; }
	;

pointer_type:	'*' instance_type { dot_emit("", ""); }
	|	'*' pointer_type  { dot_emit("", ""); }
	;

iddef:		iddef_typed  { dot_emit("iddef", "iddef_typed"); $$ = $1; borning_var_type = $1->datatype->type; }
	|	IDENT        { dot_emit("iddef", "IDENT"); $$ = cavar_create($1, NULL); }
	;

iddef_typed:	IDENT ':' datatype { dot_emit("iddef_typed", "IDENT ':' datatype"); $$ = cavar_create($1, $3); }
	;

ret_type:	ARROW datatype   { dot_emit("ret_type", "ARROW datatype"); $$ = $2; }
	|	{ $$ = make_ret_type_void(); }
		;

literal:	LITERAL { dot_emit("literal", "LITERAL"); create_literal(&$$, $1.text, $1.typetok, -1); }
	|	LITERAL type_postfix { dot_emit("literal", "LITERAL type_postfix"); create_literal(&$$, $1.text, $1.typetok, $2.typetok); }
	|	lit_struct_def   { dot_emit("literal", "lit_struct_def"); $$ = $1; }
	;

type_postfix:	I32 { make_type_postfix(&$$, $1, I32); }
	| 	I64 { make_type_postfix(&$$, $1, I64); }
	| 	U32 { make_type_postfix(&$$, $1, U32); }
	| 	U64 { make_type_postfix(&$$, $1, U64); }
	| 	F32 { make_type_postfix(&$$, $1, F32); }
	| 	F64 { make_type_postfix(&$$, $1, F64); }
	;

lit_struct_def:	IDENT '{' lit_field_list  '}'
		{
		    dot_emit("", "");
		    // TODO: define structure
		    
		}
	;

lit_field_list:	lit_field_list ',' lit_field
		{
		    dot_emit("", "");
		    // TODO: define structure 
		}
	|	lit_field        { dot_emit("", ""); $$ = $1; }
	;

lit_field:	literal          { dot_emit("", ""); $$ = $1; }
	;

%%

