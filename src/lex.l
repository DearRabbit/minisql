%{
#include <stdio.h>
#include <string.h>
#include "sqltoken.h"

int lineno = 1;

extern void yyerror(char *str);
%}

%%

create		{ return CREATE; }
drop		{ return DROP; }
select		{ return SELECT; }
insert		{ return INSERT; } 
delete		{ return DELETE; }
quit		{ return QUIT; }
execfile	{ return EXEC; }

table		{ return TABLE; }
unique		{ return UNIQUE; }
index		{ return INDEX; }
values		{ return VALUES; }
primary		{ return PRIMARY; }
key			{ return KEY; }

from		{ return FROM; }
where		{ return WHERE; }
in			{ return IN; }
into		{ return INTO; }
on			{ return ON; }
and			{ return AND; }
or			{ return OR; }

char		{ return OR; }
int			{ return INT; }
float		{ return FLOAT; }

"="		|
"<>"	|
"<"		|
">"		|
"<="	|
">="		{ return COMPARISON; }

";"			{ ++lineno; return yytext[0]; }
[-+*/(),]	
			{ return yytext[0]; }

[A-Za-z][A-Za-z0-9_]*
			{ return NAME; }

[A-Za-z0-9]+"."[A-Za-z]+
			{ return FILENAME; }

[0-9]+	|
[0-9]+"."[0-9]*	|
"."[0-9]*	{ return ACCURAT_NUM; }

[0-9]+[eE][+-]?[0-9]+	|
[0-9]+"."[0-9]*[eE][+-]?[0-9]+	|
"."[0-9]*[eE][+-]?[0-9]+
			{ return APPROX_NUM; }

'[^'\n']*'	{
			int c = input();
			unput(c);
			if (c != '\'')
				return STRING;
			else
				yymore();
			}

'[^'\n']*$	{ yyerror("unterminated string"); }

[ \t\r\n]+	;

'--'.*		;

.			yyerror("syntax error");

%%

int main(/*int argc, const char* argv[]*/)
{
	if (!yyparse())
		printf("Error.");
}
