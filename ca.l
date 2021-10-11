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
	/*
	0	        { UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext, U64); return LITERAL; }
	[1-9][0-9]*	{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext, U64); return LITERAL; }
	\-[1-9][0-9]*	{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext, I64); return LITERAL; }
	\-?0\.[0-9]+	{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext, F64); return LITERAL; }
	\-?[1-9][0-9]*\.[0-9]+	{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext, F64); return LITERAL; }
	\'\\.\'		{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext+1, CHAR); return LITERAL; }
	\'.\'		{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, yytext+1, UCHAR); return LITERAL; }
	"true"		{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, "1", BOOL); return LITERAL; }
	"false"		{ UPDATE_LINE_LOCINFO(); create_literal(&yylval.litv, "0", BOOL); return LITERAL; }
	*/

0	        { UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
[1-9][0-9]*	{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
\-[1-9][0-9]*	{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext, yyleng, I64); return LITERAL; }
\-?0\.[0-9]+	{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext, yyleng, F64); return LITERAL; }
\-?[1-9][0-9]*\.[0-9]+	{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext, yyleng, F64); return LITERAL; }
\'\\.\'		{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, UCHAR); return LITERAL; }
\'.\'		{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, CHAR); return LITERAL; }
"true"		{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, "1", 1, BOOL); return LITERAL; }
"false"		{ UPDATE_LINE_LOCINFO(); set_litbuf(&yylval.litb, "0", 1, BOOL); return LITERAL; }

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
		UPDATE_LINE_LOCINFO();
		yylval.symnameid = symname_check_insert(yytext);
		int tok = find_lexical_keyword(yytext);
		if (tok != -1)
			return tok;

		return IDENT;
	}
\n	{ glineno_prev = glineno; gcolno_prev = gcolno; ++glineno; gcolno = 0; }

[ \t]+	{ UPDATE_LINE_LOCINFO(); } /* ignore whitespace */

. 		yyerror("Unknown character: %s", yytext);

%%

int yywrap(void) {
    return 1;
}

