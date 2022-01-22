%{

#include <stdio.h>
#include <stdlib.h>
#include "ca.h"
#include "dotgraph.h"
#include "symtable.h"

/* TODO: support redefinition variable in the same scope: { let a: int = 2; let a: double = 4.3; }
TODO: symbol table also stores the semantic information such as symbol type, width, default value, etc. But the syntax tree (ST) stores syntax information. For error recovery processing, you can construct ST without considering the semantic meanings even when existing semantic errors, so in this time the compiler can report semantic error or even modification suggestions, while continuary construct ST. the procedure becomes: Construct syntax tree NOT STOP, do semantic checking, report error.
*/

/*
about main function insertion:
when not use `-main` option: then all should no changed
when use `-main` option: do following things
  1. here will insert main function, and all global statement will be in main function body
  2. the global variable will use `#[scope(global)]` property to specified
  3. initialize a symbol table for main function
  4. create a ASTNode structure for main function
  5. the symbol table layer will be g_root_symtable -> ((user) defined | main) function -> inner scope symbol table
  6. with `-main`, when out of defined function grammar scope will switch to main function scope
  7. without `-main`, when out of defined function grammar scope, will switch to g_root_symtable scope
  8. with `-main`, all defined variable have a function scope, either belongs defined or main function or when
     `#[scope(global)]` specified, the variable will belongs global
  9. without `-main`, function variable belong to function, global function belongs to global
*/

int yylex(void);
void yyerror(const char *s, ...);

extern RootTree *gtree;

/* the root symbol table for global symbols and layer 0 statement running */
extern SymTable g_root_symtable;
extern SymTable *curr_symtable;

/* mainly for label processing, because label is function scope symbol */
extern SymTable *curr_fn_symtable;
extern SymTable *g_main_symtable;

extern int extern_flag;
extern ST_ArgList curr_arglist;

extern int glineno;
extern int gcolno;

extern int yychar, yylineno;

%}/* symbol table */

%define parse.error verbose

%union {
  CADataType *datatype;
  CAVariable *var;
  CALiteral litv;   /* literal value */
  CAArrayLit arraylitv; /* array literal value */
  CAArrayExpr arrayexpr; /* array expression */
  LitBuffer litb;   /* literal buffer */
    //IdToken idtok;    /* return type token */
  int symnameid;    /* symbol table index */
  typeid_t tid;
  ASTNode *astnode; /* node pointer */
};

%token	<litb>		LITERAL STR_LITERAL
%token	<symnameid>	VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR ATOMTYPE_END STRUCT ARRAY POINTER CSTRING
%token	<symnameid>	IDENT
%token			WHILE IF IFE DBGPRINT DBGPRINTTYPE GOTO EXTERN FN RET LET EXTERN_VAR
%token			FN_DEF FN_CALL VARG COMMENT EMPTY_BLOCK STMT_EXPR IF_EXPR
%token			ARROW INFER TYPE SIZEOF TYPEOF TYPEID ZERO_INITIAL
%nonassoc		IFX
%nonassoc		ELSE
%left			GE LE EQ NE '>' '<'
%left			'+' '-'
%left			'*' '/'
%left			AS
%nonassoc		UMINUS
%type	<litv>		literal lit_struct_field lit_struct_field_list lit_struct_def
//			%type	<arraylitv>	lit_array_list lit_array_def
%type	<arrayexpr>	array_def array_items
%type	<astnode>	stmt expr stmt_list stmt_list_block label_def paragraphs fn_def fn_decl vardef_value
%type	<astnode>	paragraph fn_proto fn_args fn_call fn_body fn_args_call
%type			fn_args_p fn_args_call_p
%type	<astnode>	ifstmt stmt_list_star block_body let_stmt struct_type_def type_def
%type	<astnode>	ifexpr stmtexpr_list_block stmtexpr_list
%type	<var>		iddef iddef_typed
%type	<symnameid>	label_id attrib_scope ret_type
//			%type	<symnameid>	atomic_type
//			%type	<idtok>		type_postfix
//			%type	<datatype>	data_type pointer_type array_type ident_type
%type	<tid>		data_type pointer_type array_type ident_type

%start program

%%

program:	paragraphs { make_program(); }
		/* TODO: this should added in other place that need error recovery and syntax advince */
	|	error { yyerror("%d: error occur, on `%d`", yylineno, yychar); }
		;

paragraphs:	paragraphs paragraph { make_paragraphs($2); }
	|       {dot_emit("paragraphs", ""); /*empty */ } /* when not allow empty source file */
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
		    if (enable_emit_main()) {
			// popup generated main function, current will be global symbol table
		        pop_symtable();
		    }

		    SymTable *st = push_new_symtable();
		    if (!extern_flag) {
			/* begin processing a new function, so create new symbol table */
			curr_fn_symtable = st;
		    } else {
			curr_fn_symtable = NULL;
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
	|	{ $$ = make_expr_arglists_actual(NULL); }
		;

fn_args_call_p:	fn_args_call_p ',' expr { add_fn_args_actual(curr_symtable, $3); }
	|	expr { add_fn_args_actual(curr_symtable, $1); }
	;

stmt:		';'			{ $$ = make_empty(); }
	|	expr ';'                { $$ = make_stmt_expr($1); }
	|	DBGPRINT expr ';'          { $$ = make_stmt_print($2); }
	|	DBGPRINTTYPE data_type ';' { $$ = make_stmt_print_datatype($2); }
	|	DBGPRINTTYPE '(' data_type ')' ';' { $$ = make_stmt_print_datatype($3); }
	|	RET expr ';'            { $$ = make_stmt_ret_expr($2); }
	|	RET ';'		        { $$ = make_stmt_ret(); }
	|	let_stmt                { $$ = $1; }
	|	IDENT '=' expr ';'      { $$ = make_assign($1, $3); }
	|	WHILE '(' expr ')' stmt_list_block { $$ = make_while($3, $5); }
	|	IF '(' expr ')' stmt_list_block %prec IFX { $$ = make_if(0, 2, $3, $5); }
	|	ifstmt                  { dot_emit("stmt", "ifstmt"); $$ = $1; }
	|	stmt_list_block         { dot_emit("stmt", "stmt_list_block"); $$ = $1; }
	|	label_def               { dot_emit("stmt", "label_def"); $$ = $1; }
	|	GOTO label_id ';'       { $$ = make_goto($2); }
	|	struct_type_def         { $$ = $1; }
	|	type_def                { $$ = $1; }
		;

attrib_scope:	'#' '[' IDENT '(' IDENT ')' ']' { $$ = make_attrib_scope($3, $5); }
	;

let_stmt:	attrib_scope LET iddef '=' vardef_value ';'  { $$ = make_vardef($3, $5, 1); }
	|	LET iddef '=' vardef_value ';'               { $$ = make_vardef($2, $4, 0); }
	;

vardef_value:	expr                { $$ = $1; }
	|	ZERO_INITIAL        { $$ = make_vardef_zero_value(); }
	;

ifstmt:		IF '(' expr ')' stmt_list_block ELSE stmt_list_block    { $$ = make_if(0, 3, $3, $5, $7); }
		;
//////////////////////////////////////////////////////////////
/* TODO: realize the conflict problem when open the stmt expression using `IF` not `IFE` */
ifexpr:		IFE '(' expr ')' stmtexpr_list_block ELSE stmtexpr_list_block { $$ = make_if(1, 3, $3, $5, $7); }
		;

stmtexpr_list_block: { SymTable *st = push_new_symtable(); }
		'{' stmtexpr_list '}' { $$ = make_stmtexpr_list_block($3); }
		;

stmtexpr_list:  stmt_list expr { $$ = make_stmtexpr_list(make_stmt_list_zip(), $2); }
	|	expr { $$ = $1; }
		;
/**/
//////////////////////////////////////////////////////////////
stmt_list_block: { SymTable *st = push_new_symtable(); }
		block_body { $$ = make_stmtexpr_list_block($2); }
		;

block_body: 	'{'stmt_list_star '}' { $$ = $2; }
		;

stmt_list_star:	stmt_list             { $$ = make_stmt_list_zip(); }
	|                             { $$ = make_empty(); /* empty */}
		;
// TODO: the stmt_list and stmt may have recursive tree, so here cannot directly put following into a list simply
stmt_list:     	stmt                  { put_astnode_into_list($1, 1); }
	|	stmt_list stmt        { put_astnode_into_list($2, 0); }
		;

label_def:	label_id ':'          { $$ = make_label_def($1); }
		;

label_id:	IDENT		      { dot_emit("label_id", "IDENT"); $$ = $1; }
		;

expr:     	literal               { $$ = make_literal(&$1); }
	|	array_def             { $$ = make_array_def($1); }
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
	|	expr AS data_type     { $$ = make_as($1, $3); }
	|	SIZEOF '(' data_type ')'{ $$ = make_sizeof($3); }
		;

data_type:	ident_type            { $$ = $1; }
	|	pointer_type          { $$ = $1; }
	|	array_type            { $$ = $1; }
	|	TYPEOF '(' expr ')'   { $$ = make_typeof($3); }
	;

//atomic_type:	VOID | I32 | I64 | U32 | U64 | F32 | F64 | BOOL | CHAR | UCHAR
ident_type: 	IDENT { $$ = sym_form_type_id($1); }
		;

struct_type_def: STRUCT IDENT
		{
		SymTable *st = push_new_symtable();
		curr_arglist.argc = 0;
		curr_arglist.symtable = curr_symtable;
		}
		'{' struct_members_dot '}'       { $$ = make_struct_type($2, &curr_arglist); }
	;

struct_members_dot: struct_members | struct_members ','

// "struct_members: iddef_typed ',' struct_members" support last ',' expression
// but "struct_members: struct_members ',' iddef_typed" not support last ','
struct_members: struct_members ',' iddef_typed   { add_struct_member(&curr_arglist, curr_symtable, $3); }
	|	iddef_typed                      { add_struct_member(&curr_arglist, curr_symtable, $1); }
	|	
	;

type_def:	TYPE IDENT '=' data_type ';' { $$ = make_type_def($2, $4); }
	;

pointer_type:	'*' data_type   { $$ = make_pointer_type($2); }
	;

array_type:	'[' data_type ';' LITERAL ']'
		{
		// the LITERAL must be u64 or usize type that which is 8bytes length
		$$ = make_array_type($2, &$4);
		}
	;

iddef:		iddef_typed  { dot_emit("iddef", "iddef_typed"); $$ = $1; }
	|	IDENT        { dot_emit("iddef", "IDENT"); $$ = cavar_create($1, typeid_novalue); }
	;

iddef_typed:	IDENT ':' data_type { dot_emit("iddef_typed", "IDENT ':' data_type"); $$ = cavar_create($1, $3); }
	;

ret_type:	ARROW data_type   { dot_emit("ret_type", "ARROW data_type"); $$ = $2; }
	|	{ $$ = make_ret_type_void(); }
		;

literal:	LITERAL { dot_emit("literal", "LITERAL"); create_literal(&$$, $1.text, $1.typetok, -1); }
	|	LITERAL IDENT    { create_literal(&$$, $1.text, $1.typetok, sym_primitive_token_from_id($2)); }
	|	STR_LITERAL      { create_string_literal(&$$, &$1); }
//	|	lit_array_def    { create_array_literal(&$$, $1); }
	|	lit_struct_def   { dot_emit("literal", "lit_struct_def"); $$ = $1; }
	;

//////////////////////
array_def:	'[' array_items ']' { $$ = $2; }
	;

array_items:	array_items ',' expr { $$ = arrayexpr_append($1, $3); }
	|	expr { $$ = arrayexpr_append(arrayexpr_new(), $1); }
	;
//////////////////////

//lit_array_def:	'[' lit_array_list ']' { $$ = $2; }
//	;

//lit_array_list:	lit_array_list ',' literal { $$ = arraylit_append($1, &$3); }
//	|	literal { $$ = arraylit_append(arraylit_new(), &$1);}
//	;
lit_struct_def:	IDENT '{' lit_struct_field_list  '}'
		{
		    dot_emit("", "");
		    // TODO: define structure	    
		}
	;

lit_struct_field_list:
		lit_struct_field_list ',' lit_struct_field
		{
		    dot_emit("", "");
		    // TODO: define structure 
		}
	|	lit_struct_field        { dot_emit("", ""); $$ = $1; }
	;

lit_struct_field:
		literal          { dot_emit("", ""); $$ = $1; }
	;

%%

