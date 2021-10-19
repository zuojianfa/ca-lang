%{

#include <stdio.h>
#include <stdlib.h>
#include "ca.h"
#include "symtable.h"

/* TODO: support redefinition variable in the same scope: { let a: int = 2; let a: double = 4.3; }
TODO: symbol table also stores the semantic information such as symbol type, width, default value, etc. But the syntax tree (ST) stores syntax information. For error recovery processing, you can construct ST without considering the semantic meanings even when existing semantic errors, so in this time the compiler can report semantic error or even modification suggestions, while continuary construct ST. the procedure becomes: Construct syntax tree NOT STOP, do semantic checking, report error.
*/

int yylex(void);
void yyerror(const char *s, ...);

int walk(RootTree *tree);

extern RootTree *gtree;

/* the root symbol table for global symbols and layer 0 statement running */
extern SymTable g_root_symtable;
extern SymTable *curr_symtable;

/* mainly for label processing, because label is function scope symbol */
extern SymTable *curr_fn_symtable;

extern int borning_var_type;
extern int extern_flag;
extern ST_ArgList curr_arglist;
extern ST_ArgListActual curr_arglistactual;
extern int curr_fn_rettype;

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
%type	<astnode>	paragraph fn_proto fn_args fn_args_p fn_args_ps fn_call fn_body fn_args_call fn_args_call_p
%type	<astnode>	ifstmt stmt_list_star block_body
%type	<astnode>	ifexpr stmtexpr_list_block exprblock_body stmtexpr_list
%type	<var>		iddef iddef_typed
%type	<arg>		fn_args_actual
%type	<symnameid>	label_id
%type	<symnameid>	atomic_type struct_type
%type	<idtok>		type_postfix
%type	<datatype>	datatype instance_type ret_type

%%

program:	paragraphs
		{
		    gtree->root_symtable = &g_root_symtable;
		    walk(gtree);
		    return 0;
		}
		;

paragraphs:	paragraphs paragraph          { node_chain(gtree, $2); }
	|       { /*empty */ }
	;

paragraph:     	stmt
	|	fn_def
	|	fn_decl
		;

fn_def:		fn_proto fn_body
		{
		    $1->fndefn.stmts = $2;
		    $1->endloc.row = glineno; $$->endloc.col = gcolno;
		    $$ = $1;
		    pop_symtable();
		    curr_fn_rettype = 0;
		    curr_fn_symtable = &g_root_symtable;
		}
	;

fn_body:	block_body { $$ = $1; }
	;

fn_decl: 	EXTERN { extern_flag = 1; } fn_proto ';'
		{
		    pop_symtable();
		    extern_flag = 0;
		    curr_fn_rettype = 0;
		    $$ = $3;
		}
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
		}
		'(' fn_args ')' ret_type
		{
		    curr_fn_rettype = $7->type;
		    $$ = make_fn_proto($2, $7);
		}
	;

fn_args:	{ curr_arglist.argc = 0; }
		fn_args_ps             { $$ = $2; }
	;

fn_args_ps:	fn_args_p              { curr_arglist.contain_varg = 0; $$ = make_expr_arglists(&curr_arglist); }
	|	fn_args_p ',' VARG     { curr_arglist.contain_varg = 1; $$ = make_expr_arglists(&curr_arglist); }
	|                              { $$ = make_expr(ARG_LISTS, 0); }
	;

fn_args_p:	fn_args_p ',' iddef_typed    { add_fn_args(curr_symtable, $3); $$ = NULL; }
	|	iddef_typed                  { add_fn_args(curr_symtable, $1); $$ = NULL; }
	;

fn_args_call:	{ curr_arglistactual.argc = 0; }
		fn_args_call_p
		{
		    $$ = make_expr_arglists_actual(&curr_arglistactual);
		}
	|	{ $$ = make_expr(ARG_LISTS_ACTUAL, 0); }
	;

fn_args_call_p:	fn_args_call_p ',' fn_args_actual
		{
		    add_fn_args_actual(curr_symtable, $3);
		    $$ = NULL;
		}
	|	fn_args_actual
		{
		    add_fn_args_actual(curr_symtable, $1);
		    $$ = NULL;
		}
	;

fn_args_actual:	IDENT
		{
		    ActualArg arg;
		    arg.type = AT_Variable;
		    arg.symnameid = $1;
		    $$ = arg;
		}
	|	literal
		{
		    ActualArg arg;
		    arg.type = AT_Literal;
		    arg.litv = $1;
		    $$ = arg;
		}
	;

stmt:		';'			{ $$ = make_expr(';', 2, NULL, NULL); }
	|	expr ';'                { inference_expr_type($1); $$ = $1; }
	|	PRINT expr ';'          { inference_expr_type($2); $$ = make_expr(PRINT, 1, $2); }
	|	RET expr ';'            { determine_expr_type($2, curr_fn_rettype); $$ = make_expr(RET, 1, $2); }
	|	RET ';'                 { $$ = make_expr(RET, 0); }
	|	LET iddef '=' expr ';'  { inference_expr_type($4); $$ = make_vardef($2, $4); borning_var_type = 0; }
	|	IDENT '=' expr ';'      { $$ = make_assign($1, $3); }
	|	WHILE '(' expr ')' stmt_list_block { inference_expr_type($3); $$ = make_while($3, $5); }
	|	IF '(' expr ')' stmt_list_block %prec IFX { inference_expr_type($3); $$ = make_if(0, 2, $3, $5); }
	|	ifstmt                  { $$ = $1; }
	|	stmt_list_block         { $$ = $1; }
	|	label_def               { $$ = $1; }
	|	GOTO label_id ';'       { $$ = make_goto($2); }
		;

ifstmt:		IF '(' expr ')' stmt_list_block ELSE stmt_list_block    { inference_expr_type($3); $$ = make_if(0, 3, $3, $5, $7); }
		;
//////////////////////////////////////////////////////////////
/* TODO: realize the conflict problem when open the stmt expression using `IF` not `IFE` */
ifexpr:		IFE '(' expr ')' stmtexpr_list_block ELSE stmtexpr_list_block { inference_expr_type($3); $$ = make_if(1, 3, $3, $5, $7); }
		;

stmtexpr_list_block: { SymTable *st = push_new_symtable(); }
		exprblock_body { $$ = $2; SymTable *st = pop_symtable(); }
		;
exprblock_body: '{' stmtexpr_list '}'               { $$ = $2; }
		;

stmtexpr_list: 	stmt_list expr { inference_expr_type($2); $$ = $2; /* TODO: put stmt_list into list */ }
	|	expr { inference_expr_type($1); $$ = $1; }
		;
/**/
//////////////////////////////////////////////////////////////
stmt_list_block: { SymTable *st = push_new_symtable(); }
		block_body { $$ = $2; SymTable *st = pop_symtable(); }
		;

block_body: 	'{'stmt_list_star '}'              { $$ = $2; }
		;

stmt_list_star:	stmt_list                           { $$ = $1; }
	|	                                    { $$ = make_empty(); /* empty */}
		;

stmt_list:     	stmt                  { $$ = $1; }
	|	stmt_list stmt        { $$ = make_expr(';', 2, $1, $2); }
		;

label_def:	label_id ':'          { $$ = make_label_def($1); }
		;

label_id:	IDENT		      { $$ = $1; }
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
	|	'('expr ')'           { $$ = $2; }
	|	fn_call               { $$ = $1; }
	|	ifexpr                { $$ = $1; }
	|	expr AS datatype      { $$ = $1; /* TODO: handle AS expression */ }
		;

datatype:	instance_type         { $$ = $1; }
	|	pointer_type
	;

instance_type:	atomic_type
		{
		    CADataType *dt = catype_get_by_name($1);
		    if (!dt)
			yyerror("line: %d, col: %d: cannot find type: %s",
				glineno, gcolno, symname_get($1));
		    $$ = dt;
		}
	|	struct_type
		{
		    yyerror("line: %d, col: %d: cannot find type: %s",
			    glineno, gcolno, symname_get($1));
		    $$ = NULL;
		}
	;

atomic_type:	VOID | I32 | I64 | U32 | U64 | F32 | F64 | BOOL | CHAR | UCHAR
		    // { /* TODO: handle user defined type */ }
		;

struct_type:	IDENT { $$ = $1; }
	;

pointer_type:	'*' instance_type
	|	'*' pointer_type
	;

iddef:		iddef_typed  { $$ = $1; borning_var_type = $1->datatype->type; }
	|	IDENT        { $$ = cavar_create($1, NULL); }
	;

iddef_typed:	IDENT ':' datatype { $$ = cavar_create($1, $3); }
	;

ret_type:	ARROW datatype   { $$ = $2; }
	|	{
		    int name = symname_check_insert("void");
		    CADataType *dt = catype_get_by_name(name);
		    if (!dt)
			yyerror("line: %d, col: cannot get void datatype", glineno, gcolno);

		    $$ = dt;
		}
	;

literal:	LITERAL { create_literal(&$$, $1.text, $1.typetok, -1); }
	|	LITERAL type_postfix { create_literal(&$$, $1.text, $1.typetok, $2.typetok); }
	|	lit_struct_def   { $$ = $1; }
	;

type_postfix:	I32 { $$.symnameid = $1; $$.typetok = I32; }
	| 	I64 { $$.symnameid = $1; $$.typetok = I64; }
	| 	U32 { $$.symnameid = $1; $$.typetok = U32; }
	| 	U64 { $$.symnameid = $1; $$.typetok = U64; }
	| 	F32 { $$.symnameid = $1; $$.typetok = F32; }
	| 	F64 { $$.symnameid = $1; $$.typetok = F64; }
	;

lit_struct_def:	IDENT '{' lit_field_list  '}'
		{
		    // TODO: define structure
		    
		}
	;

lit_field_list:	lit_field_list ',' lit_field
		{
		    // TODO: define structure 
		}
	|	lit_field        { $$ = $1; }
	;

lit_field:	literal          { $$ = $1; }
	;

%%

