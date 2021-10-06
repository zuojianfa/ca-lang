digit	[0-9]
letter	[a-zA-Z]

%{
#include <stdlib.h>
#include "ca.h"
#include "ca.tab.h"
#include "symtable.h"
void yyerror(char *, ...);
extern SymTable *curr_symtable;

int glineno_prev = 1;
int gcolno_prev = 0;
int glineno = 1;
int gcolno = 0;

#define UPDATE_LINE_LOCINFO()           \
	do {                            \
		glineno_prev = glineno; \
		gcolno_prev = gcolno;   \
		gcolno += yyleng;       \
	} while(0)
%}

%%

0	        { UPDATE_LINE_LOCINFO(); yylval.litv = atoi(yytext); return I32; }
[1-9][0-9]*	{ UPDATE_LINE_LOCINFO(); yylval.litv = atoi(yytext); return I32; }
\/\/.*$         { UPDATE_LINE_LOCINFO(); /* return COMMENT; */ }
[-()<>=+*/;{}.:,]	{ UPDATE_LINE_LOCINFO(); return *yytext; }
">="		{ UPDATE_LINE_LOCINFO(); return GE; }
"<="		{ UPDATE_LINE_LOCINFO(); return LE; }
"=="		{ UPDATE_LINE_LOCINFO(); return EQ; }
"!="		{ UPDATE_LINE_LOCINFO(); return NE; }
"..."		{ UPDATE_LINE_LOCINFO(); return VARG; }
"->"		{ UPDATE_LINE_LOCINFO(); return ARROW; }
"=>"		{ UPDATE_LINE_LOCINFO(); return INFER; }

({letter}|_)+({letter}|{digit}|_)*	{
		int tok = find_lexical_keyword(yytext);
		if (tok != -1)
			return tok;

		yylval.symnameid = symname_check_insert(yytext);
		// yylval.symnameid = sym_check_insert(curr_symtable, yytext, Sym_Variable);
		UPDATE_LINE_LOCINFO();
		return IDENT;
	}
\n	{ glineno_prev = glineno; gcolno_prev = gcolno; ++glineno; gcolno = 0; }

[ \t]+	{ UPDATE_LINE_LOCINFO(); } /* ignore whitespace */

. 		yyerror("Unknown character: %s", yytext);

%%

int yywrap(void) {
    return 1;
}

