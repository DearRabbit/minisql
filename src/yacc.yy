%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Database.h"

extern int yylex(void);
extern char *yytext;
extern FILE *yyin;

/* Error Processing */
#define ErrorTokenLength 100
int ErrorFlag;
char ErrorToken [ErrorTokenLength];

#define MINISQL_PROMPT1() printf("minisql> ")
#define MINISQL_PROMPT2() printf("       > ")
%}

%parse-param { class Database *YYDatabase }

%union{
	char *strval;
	NodeAST *treeNode;
	/* warning: use double? */
	double numval;
}

%token <strval> FILENAME
%token <numval> NUMBER
%token <strval> NAME
%token <strval> STRING

%token AND
%token CHAR
%token CMD_FINISH
%token CREATE
%token DELETE
%token DROP
%token ERRORTOKEN
%token EXECFILE
%token FLOAT
%token FROM
%token INDEX
%token INSERT
%token INTEGER
%token INTO
%token KEY
%token ON
%token OR
%token PRIMARY
%token SELECT
%token TABLE
%token UNIQUE
%token VALUES
%token QUIT
%token WHERE

%token EQ
%token NEQ
%token LT
%token GT
%token LE
%token GE

%type <treeNode> sql 
	create drop select insert delete 
	table_name index_name column_name 
	create_table_element_list create_table_element
	column_def data_type opt_where comparison
	table_constraint_def expr_list expr 
	column_value_list column_value

%type <numval> column_def_opt

%left AND
%left EQ NEQ LT GT LE GE
%left '+' '-'
%left '*' '/'

%%
stmt_list:	
			stmt
		|	stmt_list stmt
		;

stmt:		QUIT CMD_FINISH		
			{ 
				/* !!!cleaning!!! */
				printf("Bye!\n");
				return 0; 
			}
		|	EXECFILE FILENAME CMD_FINISH
			{
				/* clear yyin buffer? */
				yyin = fopen($2, "r");
				if (yyin == NULL)
				{
					fprintf(stderr, "Error: unable to open file \"%s\"\n", $2);
					MINISQL_PROMPT1();
					yyin = stdin;
				}
				free($2);
			}
		|	sql_list CMD_FINISH
			{
				/* do something! */
				YYDatabase->processAST();
				MINISQL_PROMPT1();
			}
		|	sql_list ';' error_statment CMD_FINISH
			{
				YYDatabase->processAST();
				goto processErrorToken;
			}
		|	sql_list error_statment CMD_FINISH
			{
				YYDatabase->delErrRootNode();
				YYDatabase->processAST();
				goto processErrorToken;
			}
		|	error_statment CMD_FINISH
			{
				processErrorToken:
				fprintf(stderr ,"Error: near \"%s\": syntax error\n", ErrorToken);
				YYDatabase->cleanAST();
				ErrorFlag = 0;
				yyclearin;
				yyerrok;
				MINISQL_PROMPT1();
			}
		;

sql_list:
			sql 
			{
				YYDatabase->addRootNode($1);
			}
		|	sql_list ';' sql
			{
				YYDatabase->addRootNode($3);
			}
		;

sql:
			create		
		|	drop
		|	select 
		|	insert
		|	delete 
		;

/* --- create --- */ 
create:
			CREATE TABLE table_name '(' create_table_element_list ')'
			{
				$3->operation = OP_CREATE_TABLE;
				$3->rightSon = $5;
				$$ = $3;
			}
		|	CREATE INDEX index_name ON table_name '(' column_name ')'
			{
				$3->operation = OP_CREATE_INDEX;
				$3->rightSon = $5;
				$5->rightSon = $7;
				$$ = $3;
			}
		;

/* for extension */
table_name:
			NAME
			{
				$$ = YYDatabase->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_NAME;
			}
		;

index_name:
			NAME
			{
				$$ = YYDatabase->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_NAME;
			}
		;

column_name:	
			NAME
			{
				$$ = YYDatabase->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_NAME;
			}
		;

create_table_element_list:
			create_table_element
		|	create_table_element_list ',' create_table_element
			{
				$1->rightSon = $3;
				$$ = $1;
			}
		;

create_table_element:
			column_def
		|	table_constraint_def
		;

column_def:	
			NAME data_type column_def_opt
			{
				$$ = $2;
				$$->strval = $1;
				if ($3 != 0)
				{
					$$->leftSon = YYDatabase->newEmptyNode();
					$$->leftSon->operation = $3;
				}
			}
		;

data_type:
			CHAR
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = VAL_CHAR;
				$$->numval = 1;
			}
		|	CHAR '(' NUMBER ')'
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = VAL_CHAR;
				$$->numval = $3;
			}
		|	INTEGER
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = VAL_INT;
			}
		|	FLOAT
			INTEGER
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = VAL_FLOAT;
			}
		;

column_def_opt:
			/* empty */
			{
				$$ = 0;
			}
		|	PRIMARY KEY
			{
				$$ = DEF_PRIMARY;
			}
		|	UNIQUE
			{
				$$ = DEF_UNIQUE;
			}
		;

table_constraint_def:
			PRIMARY KEY '(' NAME ')'
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = DEF_PRIMARY;
				$$->strval = $4;
			}
		;

/* --- end create --- */


/* --- drop --- */
drop:	
			DROP TABLE table_name
			{
				$3->operation = OP_DROP_TABLE;
				$$ = $3;
			}
		|	DROP INDEX index_name
			{
				$3->operation = OP_DROP_INDEX;
				$$ = $3;
			}
		;
/* --- end drop --- */


/* --- select --- */

select:
			SELECT '*' FROM table_name opt_where
			{
				$4->operation = OP_SELECT;
				$4->rightSon = $5;
				$$ = $4;
			}
		;

opt_where:
			/* empty */
			{ $$ = nullptr; }
		|	WHERE expr_list
			{
				$$ = $2;
			}
		;

expr_list:
			expr AND expr
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = OP_AND;
				$$->leftSon = $1;
				$$->rightSon = $3;
			}
		/*|	expr OR expr*/
		;

expr:
			column_value comparison column_name
			{	
				/* keys are always at left */
				$$ = $2;
				$2->leftSon = $3;
				$2->rightSon = $1;
			}
		|	column_name comparison column_value
			{	
				$$ = $2;
				$2->leftSon = $1;
				$2->rightSon = $3;
			}
		;

comparison:
			EQ
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = CMP_EQ;
			}
		|	NEQ
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = CMP_NEQ;
			}
		|	LT
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = CMP_LT;
			}
		|	GT
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = CMP_GT;
			}
		|	LE
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = CMP_LE;
			}
		|	GE
			{
				$$ = YYDatabase->newEmptyNode();
				$$->operation = CMP_GE;
			}
		;

/* --- end select --- */


/* --- insert --- */
insert:	
			INSERT INTO table_name VALUES '(' column_value_list ')'
			{
				$3->operation = OP_INSERT;
				$3->rightSon = $6;
				$$ = $3;
			}
		;

column_value_list:
			column_value
		|	column_value_list ',' column_value
			{
				$1->rightSon = $3;
				$$ = $1;
			}
		;

column_value:
			STRING
			{
				$$ = YYDatabase->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_CHAR;
			}
		|	NUMBER
			{
				$$ = YYDatabase->newEmptyNode();
				$$->numval = $1;
				$$->operation = VAL_NUMBER;
			}
		;
/* --- end insert --- */


/* --- delete --- */
delete:
			DELETE FROM table_name opt_where
			{
				$3->operation = OP_DELECT;
				$3->rightSon = $4;
				$$ = $3;
			}
		;

/* --- end delete --- */


/* --- other --- */
error_statment:
			error	
			{
				if (ErrorFlag == 0)
				{
					char *tokenTmpPtr;
					strncpy(ErrorToken, yytext, ErrorTokenLength);
					ErrorFlag = 1;
					tokenTmpPtr = strchr(ErrorToken, '\n');
					if (tokenTmpPtr)
						*tokenTmpPtr = 0;
				}
			}
		|	ERRORTOKEN
			{
				if (ErrorFlag == 0)
				{
					strncpy(ErrorToken, yytext, ErrorTokenLength);
					ErrorFlag = 1;
				}
			}
		;
/* --- end other --- */
%%

int yyerror(Database *YYDatabase, const char* str)
{
	/* dummy */
	return 0;
}
