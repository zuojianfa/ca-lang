/**
 * Copyright (c) 2023 Rusheng Xia <xrsh_2004@163.com>
 * CA Programming Language and CA Compiler are licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */


%{

#include <stdio.h>
#include <stdlib.h>
#include "ca_parser.h"
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

extern RootTree *gtree;

/* the root symbol table for global symbols and layer 0 statement running */
extern SymTable g_root_symtable;
extern SymTable *curr_symtable;

/* mainly for label processing, because label is function scope symbol */
extern SymTable *curr_fn_symtable;
extern SymTable *g_main_symtable;
extern TypeImplInfo *current_type_impl;
extern int current_trait_id;

extern int extern_flag;
extern ST_ArgList curr_arglist;

extern int glineno;
extern int gcolno;

extern int yychar, yylineno;

%}/* symbol table */

%define parse.error verbose
//			%glr-parser

%union {
  CADataType *datatype;
  CAVariable *var;
  CALiteral litv;   /* literal value */
  CAArrayExpr arrayexpr; /* array expression */
  CAStructExpr structexpr;
  LitBuffer litb;   /* literal buffer */
  int symnameid;    /* symbol table index */
  typeid_t tid;
  ASTNode *astnode; /* node pointer */
  DerefLeft deleft; /* represent dereference left value */
  ArrayItem aitem;  /* array item */
  StructFieldOp structfieldop; /* struct field operation */
  LeftValueId leftvalueid; /* any id that have left value */
  ForStmtId forstmtid; /* the for statement variable type: by value, by pointer, by reference */
  IdGroup symnameids;
  CAPattern *capattern;
  PatternGroup *patterngroup;
  GeneralRange range;
  TypeImplInfo impl_info;
  DomainNames domain_names;
  DomainFn domain_fn;
  DomainAs domain_as;
  FnNameInfo fnname_info;
  void *generic_types;
};

%token	<litb>		LITERAL STR_LITERAL
%token	<symnameid>	VOID I16 I32 I64 U16 U32 U64 F32 F64 BOOL I8 U8 ATOMTYPE_END STRUCT ARRAY POINTER CSTRING
%token	<symnameid>	IDENT // OSELF CSELF
%token			WHILE IF IFE DBGPRINT DBGPRINTTYPE GOTO EXTERN FN RET LET EXTERN_VAR IMPL TRAIT
%token			LOOP FOR IN BREAK CONTINUE MATCH USE MOD
%token			BAND BOR BXOR BNOT
%token			ASSIGN_ADD ASSIGN_SUB ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD ASSIGN_SHIFTL ASSIGN_SHIFTR ASSIGN_BAND ASSIGN_BOR ASSIGN_BXOR
%token			FN_DEF FN_CALL VARG COMMENT EMPTY_BLOCK STMT_EXPR IF_EXPR ARRAYITEM STRUCTITEM TUPLE RANGE SLICE
%token			INFER ADDRESS DEREF TYPE SIZEOF TYPEOF TYPEID ZERO_INITIAL REF DOMAIN
%nonassoc		IFX
%nonassoc		ELSE
%left			IGNORE IRANGE
%left			BOX DROP
%left			LOR
%left			LAND
%left			'|'
%left			'^'
%left			'&'
%left			EQ NE
%left			GE LE '>' '<'
%left			SHIFTL SHIFTR
%left			'+' '-'
%left			'*' '/' '%'
%left			AS
%left			'!' '~'
%nonassoc		UMINUS UADDR
%left			'.' ARROW '[' ']'
%left			UARRAY UDEREF
%type	<litv>		literal
%type	<arrayexpr>	array_def array_def_items
%type	<structexpr>	struct_expr struct_expr_fields named_struct_expr_fields
%type	<astnode>	stmt stmt_list stmt_list_block label_def paragraphs fn_def fn_decl vardef_value type_impl fn_impl_defs fn_impl_defs_all
%type	<astnode>	expr arith_expr cmp_expr logic_expr bit_expr
%type	<astnode>	fn_unit fn_proto fn_args fn_call_or_tuple fn_body fn_args_call_or_tuple gen_tuple_expr gen_tuple_expr_args
%type	<fnname_info>	fn_proto_name
%type	<generic_types>	fn_proto_generic generic_types
%type	<astnode>	fn_args_p fn_args_call_or_tuple_p fn_call fn_method_call fn_domain_call
%type	<astnode>	ifstmt stmt_list_star block_body let_stmt assignment_stmt assign_op_stmt struct_type_def tuple_type_def type_def
%type	<astnode>	trait_def trait_fn_defs trait_fn_def trait_fn_defs_all
%type	<astnode>	ifexpr stmtexpr_list_block stmtexpr_list for_stmt
%type	<forstmtid>	for_stmt_ident
%type	<var>		iddef iddef_typed iddef_typed_for_impl
%type	<symnameid>	label_id attrib_scope ret_type
//			%type	<symnameid>	atomic_type
%type	<tid>		data_type pointer_type array_type ident_type gen_tuple_type
%type	<deleft>	deref_pointer
%type	<aitem>		array_item array_item_r
%type	<structfieldop>	structfield_op
%type	<leftvalueid>	left_value_id
%type	<symnameid>	assign_op
%type	<symnameids>	let_more_var
%type	<capattern>	let_stmt_left let_stmt_one_left let_stmt_left_pattern let_stmt_one_left_typed pattern_tuple_arg let_stmt_more_left_typed let_stmt_more_left pattern_struct_arg
%type	<patterngroup>	pattern_struct_args_all pattern_tuple_args_all pattern_tuple_args pattern_struct_args
%type	<range>		general_range
%type	<impl_info>	type_impl_head
%type	<domain_names>	domain domain_extend
%type	<domain_as>	domain_as
%type	<domain_fn>	domain_fn

%start program

%%

program:	paragraphs { make_program(); }
		/* TODO: this should added in other place that need error recovery and syntax advince */
	|	error { yyerror("%d: error occur, on `%d`", yylineno, yychar); }
		;

paragraphs:	paragraphs stmt { make_paragraphs($2); }
	|       {dot_emit("paragraphs", ""); /*empty */ } /* when not allow empty source file */
		;

fn_unit:	{
			if (current_type_impl)
				current_type_impl->fn_def_recursive_count += 1;
		}
		fn_def
		{	dot_emit("fn_unit", "fn_def"); $$ = $2;
			if (current_type_impl)
				current_type_impl->fn_def_recursive_count -= 1;
		}
	|	fn_decl  { dot_emit("fn_unit", "fn_decl"); }
	|	type_impl{ dot_emit("fn_unit", "type_impl"); }
		;

type_impl:	type_impl_head  { push_type_impl(&$1); }
		'{' fn_impl_defs_all '}' { pop_type_impl(); $$ = $4; }
	;

type_impl_head:	IMPL ident_type { $$ = begin_impl_type($2); }
	|	IMPL ident_type FOR ident_type { $$ = begin_impl_trait_for_type($4, $2); }
	;

fn_impl_defs_all:
		{ $$ = make_fn_def_impl_begin(NULL); }
	| 	fn_impl_defs { $$ = $1; }
		;

/* collect function into a group assoicated with one type */
fn_impl_defs:	fn_impl_defs fn_def { $$ = make_fn_def_impl_next($$, $2); }
	|	fn_def { $$ = make_fn_def_impl_begin($1); }
	;

fn_def:		fn_proto fn_body { $$ = make_fn_def($1, $2); }
		;

fn_body:	block_body { $$ = make_fn_body($1); }
		;

fn_decl: 	EXTERN { extern_flag = 1; } fn_proto ';' { $$ = make_fn_decl($3); }
		;

fn_call:	fn_call_or_tuple { $$ = $1; }
	|	fn_method_call   { $$ = $1; }
	|	fn_domain_call   { $$ = $1; }
	;

fn_call_or_tuple:
		IDENT '(' fn_args_call_or_tuple ')' { $$ = make_fn_call_or_tuple($1, $3); }
		;

fn_method_call:	structfield_op '(' fn_args_call_or_tuple ')' {$$ = make_method_call($1, $3);}
		;

fn_domain_call: domain_fn '(' fn_args_call_or_tuple ')' { $$ = make_domain_call(&$1, $3); }
		;

domain_fn:	domain    { $$ = make_domainfn_domain(&$1); }
	|	domain_as { $$ = make_domainfn_domainas(&$1); }
	;

domain: 	domain DOMAIN IDENT  { domain_append(&$1, $3); $$ = $1; }
	|	IDENT DOMAIN IDENT   { $$ = domain_init(1, $1); domain_append(&$$, $3); }
	|	DOMAIN IDENT         { $$ = domain_init(0, $2); }
	;

domain_as:	'<' domain_extend AS domain_extend '>' DOMAIN IDENT {
		$$ = make_domain_as(&$2, &$4, $7);
		}
	;

domain_extend:	domain { $$ = $1; }
	| 	IDENT  { $$ = domain_init(1, $1); }
	;

fn_proto_name: FN IDENT fn_proto_generic {
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
		    $$.fnname = $2;
		    $$.generic_types = $3;
		}

fn_proto_generic:                     { $$ = NULL; }
	|	'<' generic_types '>' { $$ = $2; }
	;

generic_types: 	generic_types ',' IDENT { vec_append($1, (void *)(long)$3); $$ = $1; }
	|	IDENT { $$ = vec_new(); vec_append($$, (void *)(long)$1); }
		;

fn_proto:	fn_proto_name '(' fn_args ')' ret_type { $$ = make_fn_proto(&$1, &curr_arglist, $5); }
		;

fn_args:	fn_args_p              { add_fn_args_p(&curr_arglist, 0); }
	|	fn_args_p ',' VARG     { add_fn_args_p(&curr_arglist, 1); }
	|                              { }
		;

fn_args_p:	fn_args_p ',' iddef_typed    { add_fn_args(&curr_arglist, curr_symtable, $3); }
	|       // only first parameter can be using self
		iddef_typed_for_impl         { add_fn_args(&curr_arglist, curr_symtable, $1); }
	;

iddef_typed_for_impl:
		iddef_typed
	|	IDENT { $$ = cavar_create_self($1); }
	;

fn_args_call_or_tuple:
		{ actualarglist_new_push(); }
		fn_args_call_or_tuple_p { $$ = make_expr_arglists_actual(actualarglist_current()); }
	|	{ $$ = make_expr_arglists_actual(NULL); }
		;

fn_args_call_or_tuple_p:
		fn_args_call_or_tuple_p ',' expr { add_fn_args_actual(curr_symtable, $3); }
	|	expr { add_fn_args_actual(curr_symtable, $1); }
	;

stmt:		';'			{ $$ = make_empty(); }
	|	expr ';'                { $$ = make_stmt_expr($1); }
	|	DBGPRINT expr ';'          { $$ = make_stmt_print($2); }
	|	DBGPRINTTYPE data_type ';' { $$ = make_stmt_print_datatype($2); }
//	|	DBGPRINTTYPE '(' data_type ')' ';' { $$ = make_stmt_print_datatype($3); } // the brackets conflict with general tuple so disable it
	|	RET expr ';'            { $$ = make_stmt_ret_expr($2); }
	|	RET ';'		        { $$ = make_stmt_ret(); }
	|	let_stmt                { $$ = $1; }
	|	match_stmt              {  }
	|	assignment_stmt         { $$ = $1; }
	|	assign_op_stmt          { $$ = $1; }
	|	BREAK ';'               { $$ = make_break(); }
	|	CONTINUE ';'            { $$ = make_continue(); }
	|	LOOP stmt_list_block    { $$ = make_loop($2); }
	|	for_stmt                { $$ = $1; }
	|	WHILE '(' expr ')' stmt_list_block { $$ = make_while($3, $5); }
	|	ifstmt                  { dot_emit("stmt", "ifstmt"); $$ = $1; }
	|	stmt_list_block         { dot_emit("stmt", "stmt_list_block"); $$ = $1; }
	|	label_def               { dot_emit("stmt", "label_def"); $$ = $1; }
	|	GOTO label_id ';'       { $$ = make_goto($2); }
	|	struct_type_def         { $$ = $1; }
	|	tuple_type_def          { $$ = $1; }
	|	trait_def               { $$ = $1; }
	|	type_def                { $$ = $1; }
	|	DROP IDENT ';'          { $$ = make_drop($2); }
	|	fn_unit           { $$ = $1; }
	;

deref_pointer: '*' expr { $$ = (DerefLeft) {1, $2}; }

/* deref_left:	'*' expr                { $$ = (DerefLeft) {1, $2}; } */
/* 	|	'*' deref_left          { $$ = (DerefLeft) {1 + $2.derefcount, $2.expr}; } */
/* 	; */

/* array_item:	expr '[' expr ']'      { $$ = arrayitem_begin($1, $3); } */
/* 	|	array_item '[' expr ']' { $$ = arrayitem_append($1, $3); } */
/* 	; */

array_item:	expr array_item_r { $$ = arrayitem_end($2, $1); }
	;

array_item_r: 	'['expr ']' { $$ = arrayitem_begin($2); }
	|	'['expr ']' array_item_r { $$ = arrayitem_append($4, $2); }
	;

general_range:	IGNORE           { general_range_init(&$$, 0, NULL, NULL); } // .., full range
	|	IGNORE expr      { general_range_init(&$$, 0, NULL, $2);   } // .. b, range to
	|	IRANGE expr      { general_range_init(&$$, 1, NULL, $2);   } // ..= b, inclusive range to
	|	expr IGNORE      { general_range_init(&$$, 0, $1, NULL);   } // a .., range from
	|	expr IRANGE      { general_range_init(&$$, 1, $1, NULL);   } // a ..=, inclusive range from
	|	expr IGNORE expr { general_range_init(&$$, 0, $1, $3);     } // a .. b, range
	|	expr IRANGE expr { general_range_init(&$$, 1, $1, $3);     } // a ..= b, inclusive range
	;

attrib_scope:	'#' '[' IDENT '(' IDENT ')' ']' { $$ = make_attrib_scope($3, $5); }
	;

let_stmt:	attrib_scope LET iddef '=' vardef_value ';'  { $$ = make_global_vardef($3, $5, 1); }
//	|	LET iddef '=' vardef_value ';'               { $$ = make_global_vardef($2, $4, 0); }
	|	LET let_stmt_left '=' vardef_value ';' { $$ = make_let_stmt($2, $4); }
	|	LET let_stmt_left ';'                  {
			ASTNode *node = make_vardef_zero_value(VarInit_NoInit);
			$$ = make_let_stmt($2, node);
		}
	;

let_stmt_left:	let_stmt_more_left_typed
	|	let_stmt_more_left
	|	let_stmt_one_left_typed
	|	let_stmt_one_left
	;

let_stmt_more_left_typed:
		let_stmt_more_left ':' data_type { $1->datatype = $3; $$ = $1; }
	;

let_stmt_more_left:
		let_more_var let_stmt_one_left { $2->morebind = $1.groups; $$ = $2; }
	;

let_more_var:	let_more_var IDENT '@' { vec_append($$.groups, (void *)(long)$2); }
	|	IDENT '@' { $$.groups = vec_new(); vec_append($$.groups, (void *)(long)$1); }
	;

let_stmt_one_left:
		let_stmt_left_pattern
	;

let_stmt_one_left_typed:
		let_stmt_one_left ':' data_type { $1->datatype = $3; $$ = $1; }
	;

let_stmt_left_pattern:
		IDENT                                 { $$ = capattern_new($1, PT_Var, NULL); } // simple pattern, simple variable binding
	|	IDENT '(' pattern_tuple_args_all ')'  { $$ = capattern_new($1, PT_Tuple, $3); } // tuple style pattern match, for tuple/enum/union type
	|	IDENT '{' pattern_struct_args_all '}' { $$ = capattern_new($1, PT_Struct, $3); } // struct style pattern match, for struct/tuple type
	|	'(' pattern_tuple_args_all ')'        { $$ = capattern_new(0, PT_GenTuple, $2); } // general tuple (unnamed) pattern match
	|	'[' pattern_tuple_args_all ']'        { $$ = capattern_new(0, PT_Array, $2); }
	|	'_'                                   { $$ = capattern_new(0, PT_IgnoreOne, NULL); } // ignorance the pattern
//	|	literal
//	|	IDENT ':' ':' IDENT // TODO: domain type or enum type

pattern_tuple_args_all:
		pattern_tuple_args
	|       { $$ = patterngroup_new(); }
	;

pattern_tuple_args: // a,b,c,d a,..,d a,b,.. ..,c,d, _
		pattern_tuple_args ',' pattern_tuple_arg { $$ = patterngroup_push($1, $3); }
	|	pattern_tuple_arg { $$ = patterngroup_push(patterngroup_new(), $1); }
	;

pattern_tuple_arg:
		let_stmt_more_left
	|	let_stmt_one_left
	|	IGNORE {
		/* the ignore .. can appear at any place in a tuple pattern, but only have one ignore */
		$$ = capattern_new(0, PT_IgnoreRange, NULL);
		}
		;

pattern_struct_args_all:
		pattern_struct_args
	|	{ $$ = patterngroup_new(); }

// struct pattern only support field form, not allow no field variable list, so only support the last .., because the field can in any order
pattern_struct_args:
		pattern_struct_args ',' pattern_struct_arg { $$ = patterngroup_push($1, $3); }
	|	pattern_struct_arg { $$ = patterngroup_push(patterngroup_new(), $1); }
	;

pattern_struct_arg:
		IDENT                          {
		/* when the same as field name and as the variable */
		$$ = capattern_new($1, PT_Var, NULL); $$->fieldname = $1;
		}
	|	IDENT ':' let_stmt_more_left   {
		/* first is field name, second is the binded name */
		$3->fieldname = $1; $$ = $3;
		}
	|	IDENT ':' let_stmt_one_left    {
		/* first is field name, second is the binded name */
		$3->fieldname = $1; $$ = $3;
		}
	|	LITERAL ':' let_stmt_more_left {
		/* when tuple treated as struct form, first is */
		$3->fieldname = parse_tuple_fieldname($1.text);
		$$ = $3;
		}
	|	LITERAL ':' let_stmt_one_left  {
		/* when tuple treated as struct form, first is */
		$3->fieldname = parse_tuple_fieldname($1.text);
		$$ = $3;
		}
	|	IGNORE {
		/* the ignore .. only can appear at the end of struct pattern, and only have one */
		$$ = capattern_new(0, PT_IgnoreRange, NULL);
		}
	;

match_stmt: 	MATCH '(' expr ')' '{' match_patterns '}' {}
	;

match_patterns:	match_patterns ',' match_pattern {}
	|	match_patterns match_pattern {}
	|	match_pattern {}
	;

match_pattern:	let_stmt_left INFER stmt_list_block {}
	;

/* match_left:	let_stmt_left */
/* 	| */
/* 	; */

assignment_stmt:left_value_id '=' expr ';'  { $$ = make_assign(&$1, $3); }
	;

left_value_id:	IDENT               { $$.type = LVT_Var; $$.var = $1; }
	|	deref_pointer       { $$.type = LVT_Deref; $$.deleft = $1; }
	|	array_item          { $$.type = LVT_ArrayItem; $$.aitem = $1; }
	|	structfield_op      { $$.type = LVT_StructOp; $$.structfieldop = $1; }
	;

assign_op_stmt:	left_value_id assign_op expr ';' { $$ = make_assign_op(&$1, $2, $3); }
	;

assign_op:	ASSIGN_ADD          { $$ = ASSIGN_ADD;    }
	|	ASSIGN_SUB	    { $$ = ASSIGN_SUB;    }
	|	ASSIGN_MUL	    { $$ = ASSIGN_MUL;    }
	|	ASSIGN_DIV	    { $$ = ASSIGN_DIV;    }
	|	ASSIGN_MOD	    { $$ = ASSIGN_MOD;    }
	|	ASSIGN_SHIFTL	    { $$ = ASSIGN_SHIFTL; }
	|	ASSIGN_SHIFTR	    { $$ = ASSIGN_SHIFTR; }
	|	ASSIGN_BAND	    { $$ = ASSIGN_BAND;   }
	|	ASSIGN_BOR	    { $$ = ASSIGN_BOR;    }
	|	ASSIGN_BXOR	    { $$ = ASSIGN_BXOR;   }
	;

vardef_value:	expr                { $$ = $1; }
	|	ZERO_INITIAL        { $$ = make_vardef_zero_value(VarInit_Zero); }
	;

for_stmt:	FOR                   { SymTable *st = push_new_symtable(); /* the inner variable and / or listnode need a symbol table in for stmt */ }
		'(' for_stmt_ident IN expr ')' { make_for_var_entry($4.var); }
		stmt_list_block       { $$ = make_for_stmt($4, $6, $9); }
	;

for_stmt_ident:	IDENT               { $$ = (ForStmtId){0, $1}; }
	|	'*' IDENT           { $$ = (ForStmtId){'*', $2}; }
	|	REF IDENT           { $$ = (ForStmtId){'&', $2}; }
	;

ifstmt:		{ ifstmt_new_push(); } ifstmt1 { $$ = ifstmt_current(); ifstmt_pop(0); }
	;

ifstmt1:   	ifpart elsepart
		;

ifpart:  	IF '(' expr ')' stmt_list_block { make_ifpart(ifstmt_current(), $3, $5); }
//	|	IF expr stmt_list_block { make_ifpart(ifstmt_current(), $2, $3); }
		;

elsepart:	ELSE stmt_list_block { make_elsepart(ifstmt_current(), $2); }
	|	ELSE ifstmt1 // just for syntax structure, no need action, all done in stack
	|                    // just for syntax structure, no need action, all done in stack
		;

//////////////////////////////////////////////////////////////
/* 		TODO: realize the conflict problem when open the stmt expression using `IF` not `IFE` */
//ifexpr:		IFE '(' expr ')' stmtexpr_list_block ELSE stmtexpr_list_block { $$ = make_if(1, 3, $3, $5, $7); }
//		;

ifexpr:		{ ifstmt_new_push(); }
		IFE '(' expr ')' stmtexpr_list_block ELSE stmtexpr_list_block
		{
		ASTNode *p = ifstmt_current();
		make_ifpart(p, $4, $6);
		make_elsepart(p, $8);
		$$ = p;
		ifstmt_pop(1);
		}
		;

stmtexpr_list_block:                   { SymTable *st = push_new_symtable(); }
		'{' stmtexpr_list '}'  { ASTNode *node = make_lexical_body($3); $$ = make_stmtexpr_list_block(node); }
		;

stmtexpr_list:  stmt_list expr { $$ = make_stmtexpr_list(make_stmt_list_zip(), $2); }
	|	expr { $$ = $1; }
		;
/**/
//////////////////////////////////////////////////////////////
stmt_list_block: { SymTable *st = push_new_symtable(); }
		block_body { $$ = make_stmtexpr_list_block($2); }
		;

block_body: 	'{'                // { push_lexical_body(); }
		stmt_list_star '}'    { $$ = make_lexical_body($2); }
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
	|	general_range         { $$ = make_general_range(&$1); }
	|	array_def             { $$ = make_array_def($1); }
	|	array_item            { $$ = make_arrayitem_right($1); }
	|	struct_expr           { $$ = make_struct_expr($1); }
	|	gen_tuple_expr        { $$ = $1; }
//	|	tuple_expr            { $$ = make_tuple_expr($1); }
	|	IDENT                 { $$ = make_ident_expr($1); }
	|	arith_expr            { $$ = $1; }
	|	cmp_expr              { $$ = $1; }
	|	logic_expr	      { $$ = $1; }
	|	bit_expr              { $$ = $1; }
	|	'('expr ')'           { dot_emit("expr", "'(' expr ')'"); $$ = $2; }
	|	fn_call               { dot_emit("expr", "fn_call"); $$ = $1; }
	|	ifexpr                { dot_emit("expr", "ifexpr"); $$ = $1; }
	|	expr AS data_type     { $$ = make_as($1, $3); }
	|	SIZEOF '(' data_type ')'{ $$ = make_sizeof($3); }
	|	deref_pointer         { $$ = make_deref($1.expr); }
	|	'&' expr %prec UADDR  { $$ = make_address($2); }
	|	structfield_op        { $$ = make_structfield_right($1); }
	|	BOX expr              { $$ = make_boxed_expr($2); }
//	|	BOX type
	;

arith_expr:	'-' expr %prec UMINUS { $$ = make_uminus_expr($2); }
	|	expr '+' expr         { $$ = make_expr('+', 2, $1, $3); }
	|	expr '-' expr         { $$ = make_expr('-', 2, $1, $3); }
	|	expr '*' expr         { $$ = make_expr('*', 2, $1, $3); }
	|	expr '/' expr         { $$ = make_expr('/', 2, $1, $3); }
	|	expr '%' expr         { $$ = make_expr('%', 2, $1, $3); }
	;

cmp_expr:	expr '<' expr         { $$ = make_expr('<', 2, $1, $3); }
	|	expr '>' expr         { $$ = make_expr('>', 2, $1, $3); }
	|	expr GE expr          { $$ = make_expr(GE, 2, $1, $3); }
	|	expr LE expr          { $$ = make_expr(LE, 2, $1, $3); }
	|	expr NE expr          { $$ = make_expr(NE, 2, $1, $3); }
	|	expr EQ expr          { $$ = make_expr(EQ, 2, $1, $3); }
	;

logic_expr:	expr LAND expr        { $$ = make_expr(LAND, 2, $1, $3); }
	|	expr LOR expr         { $$ = make_expr(LOR, 2, $1, $3); }
	;

bit_expr:	expr '&' expr         { $$ = make_expr(BAND, 2, $1, $3); }
	|	expr '|' expr         { $$ = make_expr(BOR, 2, $1, $3); }
	|	expr '^' expr         { $$ = make_expr(BXOR, 2, $1, $3); }
	|	'!' expr              { $$ = make_expr(BNOT, 1, $2); }
	|	expr SHIFTL expr      { $$ = make_expr(SHIFTL, 2, $1, $3); }
	|	expr SHIFTR expr      { $$ = make_expr(SHIFTR, 2, $1, $3); }
	;

structfield_op:	expr '.' IDENT	      { $$ = make_element_field($1, $3, 1, 0); }
	|	expr ARROW IDENT      { $$ = make_element_field($1, $3, 0, 0); }
	|	expr '.' LITERAL      { $$ = make_element_field($1, $3.text, 1, 1); }
	|	expr ARROW LITERAL    { $$ = make_element_field($1, $3.text, 0, 1); }
	;

gen_tuple_expr:	'(' gen_tuple_expr_args ')' { $$ = make_gen_tuple_expr($2); }
	;

gen_tuple_expr_args:
		gen_tuple_expr_args_p { $$ = make_expr_arglists_actual(actualarglist_current()); }
	|	{ $$ = make_expr_arglists_actual(NULL); }
		;

gen_tuple_expr_args_p:
		gen_tuple_expr_args_p ',' expr { add_fn_args_actual(curr_symtable, $3); }
	|	expr ',' expr { actualarglist_new_push(); add_fn_args_actual(curr_symtable, $1); add_fn_args_actual(curr_symtable, $3); }
	;

data_type:	ident_type            { $$ = $1; }
	|	pointer_type          { $$ = $1; }
	|	array_type            { $$ = $1; }
	|	TYPEOF '(' expr ')'   { $$ = make_typeof($3); }
	|	gen_tuple_type        { $$ = $1; }
	;

//atomic_type:	VOID | I16 | I32 | I64 | U16 | U32 | U64 | F32 | F64 | BOOL | I8 | U8
ident_type: 	IDENT { $$ = sym_form_type_id($1); }
		;

struct_type_def: STRUCT IDENT { reset_arglist_with_new_symtable(); }
		'{' struct_members_dot '}'       { $$ = make_struct_type($2, &curr_arglist, 0); }
	;

struct_members_dot: struct_members | struct_members ','

struct_members: struct_members ',' iddef_typed   { add_struct_member(&curr_arglist, curr_symtable, $3); }
	|	iddef_typed                      { add_struct_member(&curr_arglist, curr_symtable, $1); }
	|	
	;

tuple_type_def:	STRUCT IDENT { tuplelist_new_push(); }
		'(' tuple_members_dot ')' ';'    { $$ = make_struct_type($2, tuplelist_current(), 1); tuplelist_pop(); }
	;

gen_tuple_type:	 { tuplelist_new_push(); }
		'(' tuple_members_dot ')'
		{ $$ = make_tuple_type(tuplelist_current()); tuplelist_pop(); }
	;

tuple_members_dot: tuple_members | tuple_members ','
	;

tuple_members:	tuple_members ',' data_type      { add_tuple_member(tuplelist_current(), $3); }
	|	data_type                        { add_tuple_member(tuplelist_current(), $1); }
	|
	;

trait_def:	TRAIT IDENT { current_trait_id = sym_form_type_id($2); }
		'{' trait_fn_defs_all '}' { $$ = make_trait_defs($2, $5); current_trait_id = 0; }
	;

trait_fn_defs_all:            { $$ = trait_fn_begin(NULL); }
	|	trait_fn_defs { $$ = $1; }
	;

trait_fn_defs:	trait_fn_defs trait_fn_def { $$ = trait_fn_next($1, $2); }
	|	trait_fn_def { $$ = trait_fn_begin($1); }
	;

trait_fn_def:	fn_proto ';' { $$ = $1; pop_symtable(); }
	|	fn_def   { $$ = $1; /* for default function implementation in trait */ }
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

literal:	LITERAL { dot_emit("literal", "LITERAL"); create_literal(&$$, $1.text, $1.typetok, tokenid_novalue); }
	|	LITERAL IDENT    { create_literal(&$$, $1.text, $1.typetok, sym_primitive_token_from_id($2)); }
	|	STR_LITERAL      { create_string_literal(&$$, &$1); }
//	|	lit_array_def    { create_array_literal(&$$, $1); }
//	|	lit_struct_def   { dot_emit("literal", "lit_struct_def"); $$ = $1; }
	;

array_def:	'[' array_def_items ']' { $$ = $2; }
	|	'[' ']' { $$ = arrayexpr_new(); }
	|	'['expr ';' literal ']' { $$ = make_array_def_fill($2, &$4); }
	;

array_def_items:array_def_items ',' expr { $$ = arrayexpr_append($1, $3); }
	|	expr { $$ = arrayexpr_append(arrayexpr_new(), $1); }
	;

struct_expr:	IDENT '{' struct_expr_fields  '}' { $$ = structexpr_end($3, $1, 0); }
	|	IDENT '{' named_struct_expr_fields '}' { $$ = structexpr_end($3, $1, 1); }
	|	IDENT '{' '}' { $$ = structexpr_end(structexpr_new(), $1, 0); }
	;

struct_expr_fields: struct_expr_fields ',' expr { $$ = structexpr_append($1, $3); }
	|	expr { $$ = structexpr_append(structexpr_new(), $1); }
	;

named_struct_expr_fields:
		named_struct_expr_fields ',' IDENT ':' expr { $$ = structexpr_append_named($1, $5, $3); }
	|	IDENT ':' expr { $$ = structexpr_append_named(structexpr_new(), $3, $1); }
	;

//tuple_expr:	IDENT '(' tuple_expr_fields ')'
//	;

//tuple_expr_fields:
//		tuple_expr_fields ',' expr
//	|	expr
//	;

%%

