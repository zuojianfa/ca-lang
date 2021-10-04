%{

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ca.h"
#include "symtable.h"

/* TODO: support redefinition variable in the same scope: { let a: int = 2; let a: double = 4.3; }
TODO: symbol table also stores the semantic information such as symbol type, width, default value, etc. But the syntax tree (ST) stores syntax information. For error recovery processing, you can construct ST without considering the semantic meanings even when existing semantic errors, so in this time the compiler can report semantic error or even modification suggestions, while continuary construct ST. the procedure becomes: Construct syntax tree NOT STOP, do semantic checking, report error.
*/

/* prototypes */
int add_fn_args(SymTable *st, int name);
int add_fn_args_actual(SymTable *st, ActualArg arg);
const char *label_name(const char *name);

ASTNode *make_empty();
ASTNode *make_expr(int op, int noperands, ...);
ASTNode *make_expr_arglists(ST_ArgList *al);
ASTNode *make_expr_arglists_actual(ST_ArgListActual *al);
ASTNode *make_fn_decl(int name, ST_ArgList *al, SLoc beg, SLoc end);
ASTNode *make_fn_define(int name, ST_ArgList *al, SLoc beg, SLoc end);
ASTNode *make_id(int i);
ASTNode *make_lit(int value);
ASTNode *make_label(int value);
ASTNode *make_goto_label(int i);
ASTNode *make_while(ASTNode *cond, ASTNode *whilebody);
ASTNode *make_if(int isexpr, int argc, ...);

void freeNode(ASTNode *p);
NodeChain *node_chain(RootTree *tree, ASTNode *p);
int yylex(void);
void yyerror(const char *s, ...);

int walk(RootTree *tree);

RootTree *gtree = NULL;

/* the root symbol table for global symbols and layer 0 statement running */
SymTable g_root_symtable;
SymTable *curr_symtable = NULL;

/* mainly for label processing, because label is function scope symbol */
SymTable *curr_fn_symtable = NULL;

int extern_flag = 0; /* indicate if handling the extern function */
/*int call_flag = 0;  indicate if under a call statement, used for real parameter checking */
ST_ArgList curr_arglist;
ST_ArgListActual curr_arglistactual;

extern int glineno_prev;
extern int gcolno_prev;
extern int glineno;
extern int gcolno;

extern char *yytext;

%}/* symbol table */

%union {
  int litv; /* integer value */
  int symnameid; /* symbol table index */
  ASTNode *astnode; /* node pointer */
  ActualArg arg;   /* argument */
};

%token	<litv>	I32
%token			I64 U32 U64 F32 F64 BOOL CHAR UCHAR
%token	<symnameid>	IDENT
%token			WHILE IF PRINT GOTO EXTERN FN RET LET EXTERN_VAR
%token			ARG_LISTS ARG_LISTS_ACTUAL FN_DEF FN_CALL VARG COMMENT EMPTY_BLOCK
%nonassoc		IFX
%nonassoc		ELSE
%left			GE LE EQ NE '>' '<'
%left			'+' '-'
%left			'*' '/'
%nonassoc		UMINUS
%type	<astnode>	stmt expr stmt_list stmt_list_block label_def paragraphs fn_def fn_decl
%type	<astnode>	paragraph fn_proto fn_args fn_args_p fn_args_ps fn_call fn_body fn_args_call fn_args_call_p
%type	<astnode>	ifstmt ifexpr stmtexpr_list_block stmt_list_star
%type	<arg>		fn_args_actual
%type	<symnameid>	label_id

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

fn_def:		fn_proto '{' fn_body '}'
		{
		    $1->fndefn.stmts = $3;
		    $$ = $1;
		    curr_symtable = curr_symtable->parent;
		    curr_fn_symtable = &g_root_symtable;
		}
	;

fn_body:	stmt_list { $$ = $1; }
	|
		{
		    /*allow empty statement*/
		    $$ = make_expr(RET, 1, make_lit(0));
		}
	;

fn_decl: 	EXTERN { extern_flag = 1; } fn_proto ';'
		{
		    curr_symtable = curr_symtable->parent;
		    extern_flag = 0;
		    $$ = $3;
		}
	;

fn_call:	IDENT '(' fn_args_call ')'
		{
		STEntry *entry = sym_getsym(&g_root_symtable, $1, 0);
		if (!entry) {
		    yyerror("line: %d, col: %d: function '%s' not defined", glineno, gcolno, symname_get($1));
		    return -1;
		}

		if (entry->sym_type != Sym_FnDecl && entry->sym_type != Sym_FnDef) {
		    yyerror("line: %d, col: %d: '%s' is not a function", glineno, gcolno, symname_get($1));
		    return -1;
		}

		$$ = make_expr(FN_CALL, 2, make_id($1), $3);
		}
	;

fn_proto:	FN IDENT
		{
		    SymTable *st = (SymTable *)malloc(sizeof(SymTable));
		    sym_init(st, curr_symtable);
		    curr_symtable = st;

		    if (!extern_flag) {
			/* begin processing a new function, so create new symbol table */
			curr_fn_symtable = st;
		    }
		}
		'(' fn_args ')'
		{
		SLoc beg = {glineno, gcolno};
		SLoc end = {glineno, gcolno};

		STEntry *entry = sym_getsym(&g_root_symtable, $2, 0);
		if (extern_flag) {
		    if (entry) {
			/* check if function declaration is the same */
			if (curr_arglist.argc != entry->u.arglists->argc) {
			    yyerror("line: %d, col: %d: function '%s' declaration not identical, see: line %d, col %d.",
				    glineno, gcolno, symname_get($2), entry->sloc.row, entry->sloc.col);
			    return -1;
		        }
		    } else {
			entry = sym_check_insert(&g_root_symtable, $2, Sym_FnDecl);
			entry->u.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
			*entry->u.arglists = curr_arglist;
		    }

		    $$ = make_fn_decl($2, &curr_arglist, beg, end);
		} else {
		    if (entry) {
			if (entry->sym_type == Sym_FnDef) {
			    yyerror("line: %d, col: %d: function '%s' already defined on line %d, col %d.",
				    glineno, gcolno, symname_get($2), entry->sloc.row, entry->sloc.col);
			    return -1;
		        }

		        if (entry->sym_type == Sym_FnDecl) {
			    entry->sym_type = Sym_FnDef;
			    SLoc loc = {glineno, gcolno};
			    entry->sloc = loc;
			}
		    } else {
			entry = sym_check_insert(&g_root_symtable, $2, Sym_FnDef);
			entry->u.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
			*entry->u.arglists = curr_arglist;
		    }

		    $$ = make_fn_define($2, &curr_arglist, beg, end);
		}
		}
	;

fn_args:	{ curr_arglist.argc = 0; }
		fn_args_ps             { $$ = $2; }
	;

fn_args_ps:	fn_args_p              { curr_arglist.contain_varg = 0; $$ = make_expr_arglists(&curr_arglist); }
	|	fn_args_p ',' VARG     { curr_arglist.contain_varg = 1; $$ = make_expr_arglists(&curr_arglist); }
	|                              { $$ = make_expr(ARG_LISTS, 0); }
	;

fn_args_p:	fn_args_p ',' IDENT
		{
		    add_fn_args(curr_symtable, $3);
		    $$ = NULL;
		}
	|	IDENT
		{
		    add_fn_args(curr_symtable, $1);
		    $$ = NULL;
		}
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
		    arg.type = AT_VARIABLE;
		    arg.symnameid = $1;
		    $$ = arg;
		}
	|	I32
		{
		    ActualArg arg;
		    arg.type = AT_LITERAL;
		    arg.litv = $1;
		    $$ = arg;
		}
	;

stmt:		';'			{ $$ = make_expr(';', 2, NULL, NULL); }
	|	expr ';'                { $$ = $1; }
	|	PRINT expr ';'          { $$ = make_expr(PRINT, 1, $2); }
	|	RET expr ';'            { $$ = make_expr(RET, 1, $2); }
	|	RET ';'                 { $$ = make_expr(RET, 0); }
	|	LET IDENT '=' expr ';'
		{
		    /* TODO: in the future realize multiple let statement in one scope */
		    STEntry *entry = sym_getsym(curr_symtable, $2, 0);
		    if (entry)
			yyerror("line: %d, col: %d: symbol '%s' already defined in scope on line %d, col %d.",
				glineno, gcolno, symname_get($2), entry->sloc.row, entry->sloc.col);

		    sym_insert(curr_symtable, $2, Sym_Variable);

		    $$ = make_expr('=', 2, make_id($2), $4);
		}		
	|	IDENT '=' expr ';'
		{
		STEntry *entry = sym_getsym(curr_symtable, $1, 1);
		if (!entry) {
		    yyerror("line: %d, col: %d: symbol '%s' not defined", glineno, gcolno, symname_get($1));
		    return -1;
		}

		$$ = make_expr('=', 2, make_id($1), $3);
		}
	|	WHILE '(' expr ')' stmt_list_block { $$ = make_while($3, $5); }
|	IF '(' expr ')' stmt_list_block %prec IFX { $$ = make_if(0, 2, $3, $5); }
	|	ifstmt                  { $$ = $1; }
	|	stmt_list_block         { $$ = $1; }
	|	label_def               { $$ = $1; }
	|	GOTO label_id ';'
		{
    		    const char *name = symname_get($2);
		    /* because the label name can using the same name as other names (variable, function, etc)
		       so innerly represent the label name as "l:<name>", in order to distinguish them */
		    const char *l = label_name(name);
		    int lpos = symname_check_insert(l);
		    STEntry *entry = sym_getsym(curr_fn_symtable, lpos, 0);
		    if (entry) {
			switch(entry->sym_type) {
			case Sym_Label:
			case Sym_Label_Hanging:
			    break;
			default:
			    yyerror("line: %d, col: %d: label name '%s' appear but not aim to be a label",
				    glineno, gcolno, name);
			    return -1;
			}
		    } else {
			entry = sym_insert(curr_fn_symtable, lpos, Sym_Label_Hanging);
			SLoc loc = {glineno, gcolno};
			entry->sloc = loc;
		    }

		    $$ = make_expr(GOTO, 1, make_goto_label(lpos));
		}
		;

ifstmt:		IF '(' expr ')' stmt_list_block ELSE stmt_list_block    { $$ = make_if(0, 3, $3, $5, $7); }
		;

ifexpr:		IF '(' expr ')' stmtexpr_list_block ELSE stmtexpr_list_block    { $$ = make_if(1, 3, $3, $5, $7); }
		;

stmtexpr_list_block:
		'{' expr '}'                        { $$ = $2; }
	|	'{' stmt_list expr '}'              { $$ = $3; }
		;

stmt_list_block: '{' stmt_list_star '}'             { $$ = $2; }
		;

stmt_list_star:	stmt_list                           { $$ = $1; }
	|	                                    { $$ = make_empty(); /* empty */}
		;

stmt_list:     	stmt                  { $$ = $1; }
	|	stmt_list stmt        { $$ = make_expr(';', 2, $1, $2); }
		;

label_def:	label_id ':'
		{
		    const char *name = symname_get($1);
		    /* because the label name can using the same name as other names (variable, function, etc)
		       so innerly represent the label name as "l:<name>", in order to distinguish them */
		    const char *l = label_name(name);
		    int lpos = symname_check_insert(l);
		    STEntry *entry = sym_getsym(curr_fn_symtable, lpos, 0);
		    if (entry) {
		      switch(entry->sym_type) {
		      case Sym_Label:
			yyerror("line: %d, col: %d: Label '%s' redefinition", glineno, gcolno, name);
			return -1;
		      case Sym_Label_Hanging:
			entry->sym_type = Sym_Label;
			break;
		      default:
			yyerror("line: %d, col: %d: label name '%s' appear but not aim to be a label",
				glineno, gcolno, name);
			return -1;
		      }
		    } else {
		      entry = sym_insert(curr_fn_symtable, lpos, Sym_Label);
		    }

		    SLoc loc = {glineno, gcolno};
		    entry->sloc = loc;

		    $$ = make_label(lpos);
		}
		;

label_id:	IDENT		      { $$ = $1; }
		;

expr:     	I32               { $$ = make_lit($1); }
	|	IDENT
		{
		    STEntry *entry = sym_getsym(curr_symtable, $1, 1);
		    if (!entry)
			yyerror("line: %d, col: %d: Variable name '%s' not defined", glineno, gcolno, symname_get($1));

		    $$ = make_id($1);
		}
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
		;

%%

int add_fn_args(SymTable *st, int name) {
    if (curr_arglist.argc >= MAX_ARGS) {
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
    curr_arglist.argnames[curr_arglist.argc++] = name;
    return 0;
}

int add_fn_args_actual(SymTable *st, ActualArg arg) {
    if (curr_arglist.argc >= MAX_ARGS) {
	if (arg.type == AT_LITERAL)
	    yyerror("line: %d, col: %d: too many args '%d', max args support is %d",
		    glineno, gcolno, arg.litv, MAX_ARGS);
	else
	    yyerror("line: %d, col: %d: too many args '%s', max args support is %d",
		    glineno, gcolno, symname_get(arg.symnameid), MAX_ARGS);
	    
	return -1;
    }

    if (arg.type == AT_LITERAL) {
	curr_arglistactual.args[curr_arglistactual.argc++] = arg;
	return 0;
    }

    STEntry *entry = sym_getsym(st, arg.symnameid, 0);
    if (!entry) {
	yyerror("line: %d, col: %d: parameter '%s' note defined",
		glineno, gcolno, symname_get(arg.symnameid));
	return -1;
    }

    entry = sym_insert(st, arg.symnameid, Sym_Variable);
    curr_arglistactual.args[curr_arglistactual.argc++] = arg;
    return 0;
}

const char *label_name(const char *name) {
    static char label_buf[1024];
    sprintf(label_buf, "l:%s", name);
    return label_buf;
}

void set_address(ASTNode *node, const SLoc *first, const SLoc *last) {
    node->begloc = *first;
    node->endloc = *last;
    node->symtable = curr_symtable;
}

ASTNode *make_empty() {
    ASTNode *p;
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_Empty;

    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_lit(int value) {
    ASTNode *p;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Literal;
    p->litn.value = value;
    p->begloc.row = glineno_prev;
    p->begloc.col = gcolno_prev;
    p->endloc.row = glineno;
    p->endloc.col = gcolno;

    return p;
}

ASTNode *make_id(int i) {
    ASTNode *p;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Id;
    p->idn.i = i;

    set_address(p, &(SLoc){glineno_prev, gcolno_prev}, &(SLoc){glineno, gcolno});
    return p;
}

ASTNode *make_expr(int op, int noperands, ...) {
    va_list ap;
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    va_start(ap, noperands);
    for (i = 0; i < noperands; i++)
	p->exprn.operands[i] = va_arg(ap, ASTNode*);

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

    va_end(ap);
    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_expr_arglists(ST_ArgList *al) {
    int noperands = al->argc;
    int op = ARG_LISTS;
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    for (i = 0; i < noperands; i++)
	p->exprn.operands[i] = make_id(al->argnames[i]);

    if (noperands == 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[0]->endloc;
    } else if (noperands > 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[noperands-1]->endloc;
    } else {
	p->begloc = (SLoc){glineno, gcolno};
	p->endloc = (SLoc){glineno, gcolno};
    }

    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_expr_arglists_actual(ST_ArgListActual *al) {
    int noperands = al->argc;
    int op = ARG_LISTS_ACTUAL;
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    if ((p->exprn.operands = malloc(noperands * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");
    /* copy information */
    p->type = TTE_Expr;
    p->exprn.op = op;
    p->exprn.noperand = noperands;
    for (i = 0; i < noperands; i++) {
	if (al->args[i].type == AT_LITERAL)
	    p->exprn.operands[i] = make_lit(al->args[i].litv);
	else
	    p->exprn.operands[i] = make_id(al->args[i].symnameid);
    }

    if (noperands == 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[0]->endloc;
    } else if (noperands > 1) {
	p->begloc = p->exprn.operands[0]->begloc;
	p->endloc = p->exprn.operands[noperands-1]->endloc;
    } else {
	p->begloc = (SLoc){glineno, gcolno};
	p->endloc = (SLoc){glineno, gcolno};
    }

    p->symtable = curr_symtable;
    return p;
}

ASTNode *make_label(int i) {
    ASTNode *p = make_id(i);
    if (p) {
	p->type = TTE_Label;
	return p;
    }

    return NULL;
}

ASTNode *make_goto_label(int i) {
    ASTNode *p = make_id(i);
    if (p) {
	p->type = TTE_LabelGoto;
	return p;
    }

    return NULL;
}

ASTNode *make_fn_decl(int name, ST_ArgList *al, SLoc beg, SLoc end) {
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_FnDecl;
    p->fndecln.ret = 0;
    p->fndecln.name = name;
    p->fndecln.args = *al;
    p->fndecln.is_extern = 0; // TODO: make extern real extern

    set_address(p, &beg, &end);
    return p;
}

ASTNode *make_fn_define(int name, ST_ArgList *al, SLoc beg, SLoc end) {
    ASTNode *decl = make_fn_decl(name, al, beg, end);
    ASTNode *p;
    int i;
    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_FnDef;
    p->fndefn.fn_decl = decl;
    p->fndefn.stmts = NULL;

    set_address(p, &beg, &end);
    return p;
}

ASTNode *make_while(ASTNode *cond, ASTNode *whilebody) {
    ASTNode *p;

    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_While;
    p->whilen.cond = cond;
    p->whilen.body = whilebody;

    set_address(p, &cond->begloc, &whilebody->endloc);
    return p;
}

ASTNode *make_if(int isexpr, int argc, ...) {
    // TODO: implement multiple if else nodes
    va_list ap;
    ASTNode *p;

    int ncond = argc / 2;
    int remainder = argc % 2;

    /* allocate node */
    if ((p = malloc(sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    p->ifn.ncond = ncond;
    p->ifn.isexpr = isexpr;
    if ((p->ifn.conds = malloc(ncond * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    if ((p->ifn.bodies = malloc(ncond * sizeof(ASTNode))) == NULL)
	yyerror("out of memory");

    /* copy information */
    p->type = TTE_If;

    va_start(ap, argc);
    for (int i = 0; i < ncond; i++) {
	p->ifn.conds[i] = va_arg(ap, ASTNode*);
	p->ifn.bodies[i] = va_arg(ap, ASTNode*);
    }

    p->ifn.remain = remainder ? va_arg(ap, ASTNode*) : NULL;
    va_end(ap);

    set_address(p, &p->ifn.conds[0]->begloc,
		remainder ? &p->ifn.remain->endloc : &p->ifn.bodies[ncond-1]->endloc);
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

    if (sym_init(&g_root_symtable, NULL)) {
	yyerror("init symbol table failed\n");
    }

    curr_symtable = &g_root_symtable;
    curr_fn_symtable = &g_root_symtable;

    
    return 0;
}

