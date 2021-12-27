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

// update line location information
#define ULL()                           \
	do {                            \
		glineno_prev = glineno; \
		gcolno_prev = gcolno;   \
		gcolno += yyleng;       \
	} while(0)
%}

%x L_STRING

%option yylineno

%%

0	        { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
[1-9][0-9]*	{ ULL(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
\-[1-9][0-9]*	{ ULL(); set_litbuf(&yylval.litb, yytext, yyleng, I64); return LITERAL; }
\-?0\.[0-9]+	{ ULL(); set_litbuf(&yylval.litb, yytext, yyleng, F64); return LITERAL; }
\-?[1-9][0-9]*\.[0-9]+	{ ULL(); set_litbuf(&yylval.litb, yytext, yyleng, F64); return LITERAL; }
\'\\.\'		{ ULL(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, UCHAR); return LITERAL; }
\'.\'		{ ULL(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, CHAR); return LITERAL; }
"true"		{ ULL(); set_litbuf(&yylval.litb, "1", 1, BOOL); return LITERAL; }
"false"		{ ULL(); set_litbuf(&yylval.litb, "0", 1, BOOL); return LITERAL; }

\"              { ULL(); BEGIN L_STRING;  }


\/\/.*$         { ULL(); /* return COMMENT; */ }
[-()<>=+*/;{}.:,]	{ ULL(); return *yytext; }
\#|\[|\]        { ULL(); return *yytext; }
">="		{ ULL(); return GE; }
"<="		{ ULL(); return LE; }
"=="		{ ULL(); return EQ; }
"!="		{ ULL(); return NE; }
"..."		{ ULL(); return VARG; }
"->"		{ ULL(); return ARROW; }
"=>"		{ ULL(); return INFER; }

({letter}|_)+({letter}|{digit}|_)*	{
		ULL();
		yylval.symnameid = symname_check_insert(yytext);
		int tok = find_lexical_keyword(yytext);
		if (tok != -1)
			return tok;

		return IDENT;
	}
\n	{ glineno_prev = glineno; gcolno_prev = gcolno; ++glineno; gcolno = 0; }

[ \t]+	{ ULL(); } /* ignore whitespace */

. 		yyerror("Unknown character: %s", yytext);

%%

int yywrap(void) {
    return 1;
}

