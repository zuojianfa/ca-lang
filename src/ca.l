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


digit	[0-9]
letter	[a-zA-Z]

%{
#include <stdlib.h>
#include "ca_parser.h"
#include "ca.tab.h"
#include "symtable.h"
extern SymTable *curr_symtable;

int glineno_prev = 1;
int gcolno_prev = 0;
int glineno = 1;
int gcolno = 0;

//#define MAX_STRING_LITERAL_LEN 4096
//char string_buffer[MAX_STRING_LITERAL_LEN];
void *string_buffer = NULL;

// update line location information
#define ULL()                           \
	do {                            \
		glineno_prev = glineno; \
		gcolno_prev = gcolno;   \
		gcolno += yyleng;       \
	} while(0)
%}

%x L_STRING
%x L_RAW_STRING
%x L_COMMENT

%option yylineno

%%

(0x)[a-fA-F0-9]+ { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
(0o)[0-7]+       { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
(0b)[0-1]+       { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
[0-9]+	         { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, U64); return LITERAL; }
0\.[0-9]+	 { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, F64); return LITERAL; }
[0-9]+\.[0-9]+	 { ULL(); set_litbuf(&yylval.litb, yytext, yyleng, F64); return LITERAL; }
\'\\[0-9]{1,3}\' { ULL(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, U8); return LITERAL; }
\'\\.\'		 { ULL(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, U8); return LITERAL; }
\'.\'		 { ULL(); set_litbuf(&yylval.litb, yytext+1, yyleng-1, I8); return LITERAL; }
"true"		 { ULL(); set_litbuf(&yylval.litb, "1", 1, BOOL); return LITERAL; }
"false"		 { ULL(); set_litbuf(&yylval.litb, "0", 1, BOOL); return LITERAL; }

  /*****************************************
   * begin handling string literal *********
   *****************************************/

  /* following are for raw string without escape semantic and '"" character */
  /* e.g. r#"abc "Hello world" "Hello Universe" "def" "# */

r#\"	         	{
				ULL();
				string_buffer = buffer_create();
				buffer_append(string_buffer, yytext+3, yyleng-3);
				BEGIN L_RAW_STRING;
			}
<L_RAW_STRING>\n	{ ULL(); ++glineno; gcolno = 0; buffer_append_char(string_buffer, '\n'); }
<L_RAW_STRING>\r	{ ULL(); ++glineno; gcolno = 0; buffer_append_char(string_buffer, '\r'); }
<L_RAW_STRING>\"#	{
				ULL();
				BEGIN INITIAL;
				int len = 0;
				const char *s = buffer_end(string_buffer, &len);
				set_litbuf(&yylval.litb, s, len, CSTRING);
				return STR_LITERAL;
			}
<L_RAW_STRING>.		{ ULL(); buffer_append_char(string_buffer, yytext[0]); }

  /* following are for raw string without escape semantic */
r\"[^"]*\"	       	{
				ULL();
				set_litbuf(&yylval.litb, yytext+2, yyleng-3, CSTRING);
				return STR_LITERAL;
			}

  /* following are for simple string without '"' and '\' */
\"[^"\\]*\"		{
				ULL();
				yytext[yyleng-1] = 0;
				set_litbuf(&yylval.litb, yytext+1, yyleng-2, CSTRING);
				return STR_LITERAL;
			}
  /* following are for general string with '"' and '\' */
\"[^"\\]*     		{
				ULL();
				string_buffer = buffer_create();
				buffer_append(string_buffer, yytext+1, yyleng-1);
				BEGIN(L_STRING);
			}
<L_STRING>[^"\\]+       { ULL(); buffer_append(string_buffer, yytext, yyleng); }

  /* Handle escape sequences in string */
<L_STRING>\\x[0-9]{1,3} {
				ULL();
				int code = atoi(yytext + 2);
				if (code > 255)
				   yyerror("string escape decimal larger than 255");
				buffer_append_char(string_buffer, code);
			}

<L_STRING>\\n           { ULL(); buffer_append_char(string_buffer, '\n'); }
<L_STRING>\\r           { ULL(); buffer_append_char(string_buffer, '\r'); }
<L_STRING>\\t		{ ULL(); buffer_append_char(string_buffer, '\t'); }
<L_STRING>\\v		{ ULL(); buffer_append_char(string_buffer, '\v'); }
<L_STRING>\\a		{ ULL(); buffer_append_char(string_buffer, '\a'); }
<L_STRING>\\b		{ ULL(); buffer_append_char(string_buffer, '\b'); }
<L_STRING>\\f		{ ULL(); buffer_append_char(string_buffer, '\f'); }
<L_STRING>\\\\		{ ULL(); buffer_append_char(string_buffer, '\\'); }
<L_STRING>\\\"		{ ULL(); buffer_append_char(string_buffer, '\"'); }

<L_STRING>\\.           { ULL(); buffer_append_char(string_buffer, yytext[1]); }
<L_STRING>\\\n          { ULL(); ++glineno; gcolno = 0; /* A backslash at the end of the line. Do nothing */ }
<L_STRING>\"            {
				ULL();
				BEGIN INITIAL;
				int len = 0;
				int s = buffer_binary_end(string_buffer, &len);
				set_litbuf_symname(&yylval.litb, s, len, CSTRING);
				return STR_LITERAL;
			}

<L_STRING><<EOF>>	{ ULL(); yyerror("end line when processing string\n"); }
<L_STRING>\\  		{ ULL(); yyerror("end line when processing string\n"); }

  /*****************************************
   * end handling string literal *********
   *****************************************/

 /*
\/\*                { ULL(); BEGIN L_COMMENT; }
<L_COMMENT>\n       { ULL(); ++glineno; gcolno = 0; }
<L_COMMENT>\r       { ULL(); ++glineno; gcolno = 0; }
<L_COMMENT>\"       { ULL(); }
<L_COMMENT>\/       { ULL(); }
<L_COMMENT>\*       { ULL(); }
<L_COMMENT>\*\/     { BEGIN INITIAL; }
 */

 /* \.		{ ULL(); return DOT; } */
\/\/.*$         { ULL(); /* return COMMENT; */}
[-()<>=+*/;{}.:,%!|^@]	{ ULL(); return *yytext; }
\#|\[|\]        { ULL(); return *yytext; }
"::"            { ULL(); return DOMAIN; }
"&&"            { ULL(); return LAND; }
"||"            { ULL(); return LOR; }

">="		{ ULL(); return GE; }
"<="		{ ULL(); return LE; }
"=="		{ ULL(); return EQ; }
"!="		{ ULL(); return NE; }
"..."		{ ULL(); return VARG; }
"..="		{ ULL(); return IRANGE; }
".."		{ ULL(); return IGNORE; }
"->"		{ ULL(); return ARROW; }
"=>"		{ ULL(); return INFER; }
\&		{ ULL(); return '&'; }
 /*'*'		{ ULL(); return DEREF; } */

"+="            { ULL(); return ASSIGN_ADD; }
"-="		{ ULL(); return ASSIGN_SUB; }
"*="		{ ULL(); return ASSIGN_MUL; }
"/="		{ ULL(); return ASSIGN_DIV; }
"%="		{ ULL(); return ASSIGN_MOD; }
"<<="		{ ULL(); return ASSIGN_SHIFTL; }
">>="		{ ULL(); return ASSIGN_SHIFTR; }
"&="		{ ULL(); return ASSIGN_BAND; }
"\|="		{ ULL(); return ASSIGN_BOR; }
"^="		{ ULL(); return ASSIGN_BXOR; }

"<<"		{ ULL(); return SHIFTL; }
">>"            { ULL(); return SHIFTR; }
_		{ ULL(); return '_'; }


({letter}|_)+({letter}|{digit}|_)*	{
		ULL();
		yylval.symnameid = symname_check_insert(yytext);
		int tok = find_lexical_keyword(yytext);
		if (tok != -1)
			return tok;

		return IDENT;
	}
\n	{ ULL(); ++glineno; gcolno = 0; }

[ \t]+	{ ULL(); } /* ignore whitespace */

. 		yyerror("Unknown character: %s", yytext);

%%

int yywrap(void) {
    return 1;
}

