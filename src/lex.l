%{
#include <cstdio>
#include <cstdlib>
#include "yacc.tab.h"

#define MINISQL_PROMPT1() printf("minisql>")
#define MINISQL_PROMPT2() printf("       >")

%}

%%
create		{ return CREATE; }
drop		{ return DROP; }
insert		{ return INSERT; }
delete		{ return DELETE; }
select		{ return SELECT; }
^execfile	{ return EXECFILE; }
^quit		{ return QUIT; }

table		{ return TABLE; }
index		{ return INDEX; }
values		{ return VALUES; }
primary		{ return PRIMARY; }
key			{ return KEY; }
unique		{ return UNIQUE; }
int			{ return INTEGER; }
char		{ return CHAR; }
float		{ return FLOAT; }

on			{ return ON; }
from		{ return FROM; }
into		{ return INTO; }
where		{ return WHERE; }
and	| "&&"	{ return AND; }
or | "||"	{ return OR; }

-?[0-9]+	{
				yylval.intval = atoi(yytext);
				return INTNUM;
			}
			
-?[0-9]+"."[0-9]*	|
-?"."[0-9]+			|
-?[0-9]+[eE][-+]?[0-9]+				|
-?[0-9]+"."[0-9]*[eE][-+]?[0-9]+	|
-?"."[0-9]+[eE][-+]?[0-9]+	{ 
								yylval.fval = atof(yytext);  
								return FLOATNUM; 
							}

'(''|[^'\n])*'			{
							yylval.strval = (unsigned char*)strdup(yytext);
							return STRING;
						}
	
[A-Za-z][A-Za-z0-9_]*	{ 
							yylval.strval = (unsigned char*)strdup(yytext);
							return NAME; 
						}

[A-Za-z][A-Za-z0-9_]*"."[A-Za-z0-9]+	{ 
							/* warning!: Cause Error When table.column is available */
							yylval.strval = (unsigned char*)strdup(yytext);
							return FILENAME; 
						}

"="		{ return EQ; }		
"<>"	{ return NEQ; }
"<"		{ return LT; }
">"		{ return GT; }
"<="	{ return LE;}
">="	{ return GE; }

";"[ \t\r]*"\n"		{ return CMD_FINISH; }

[-+&~|^*/%(),.;!]	{ return yytext[0]; }

"--"[ \t].*	;

[ \t\r]+	;
^\n			{ MINISQL_PROMPT1(); }
\n			{ MINISQL_PROMPT2(); }
.			{ return ERRORTOKEN; }
%%

int yywrap()
{
	if (yyin != stdin)
	{
		fclose(yyin);
		yyin = stdin;
		return 0;
	}
	return 1;
}
