%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Database.h"

extern int yylex(void);
extern char *yytext;
extern FILE *yyin;
extern int yyBatchFlag;

#ifdef SHOW_TIME
	extern clock_t clk_start;
#endif

/* Error Processing */
#define ErrorTokenLength 100
int ErrorFlag;
char ErrorToken [ErrorTokenLength];

int yyerror(NodeManager *YYAST, const char* str)
{
	/* dummy */
	return 0;
}

%}

%parse-param { class NodeManager *YYAST }

%union{
	char *strval;
	Node *treeNode;
	/* warning: use double? */
	double numval;
}

%token <strval> FILENAME
%token <numval> NUMBER
%token <strval> NAME
%token <strval> STRING

%token SYS_AND
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
%token SHOW
%token SELECT
%token TABLE
%token TABLES_IN_SHOW
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
	table_constraint_def expr expr_list
	column_value_list column_value insert_val_list

%type <numval> column_def_opt

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
				else 
				{
					#ifdef SHOW_TIME
					clk_start = clock();
					#endif
					yyBatchFlag = 1;
				}
				delete ($2);
			}
		|	SHOW TABLES_IN_SHOW CMD_FINISH
			{
				Database::getInstance()->show_tables();
				MINISQL_PROMPT1();
			}
		|	sql_list CMD_FINISH
			{
				/* do something! */
				Database::getInstance()->processAST();
				if (yyBatchFlag)
					putchar('\n');
				else
					MINISQL_PROMPT1();
			}
		|	sql_list ';' error_statment CMD_FINISH
			{
				Database::getInstance()->processAST();
				goto processErrorToken;
			}
		|	sql_list error_statment CMD_FINISH
			{
				YYAST->delLastRootNode();
				Database::getInstance()->processAST();
				goto processErrorToken;
			}
		|	error_statment CMD_FINISH
			{
				processErrorToken:
				fprintf(stderr ,"Error: near \"%s\": syntax error\n", ErrorToken);
				YYAST->clean();
				ErrorFlag = 0;
				yyclearin;
				yyerrok;

				if (yyBatchFlag)
					putchar('\n');
				else
					MINISQL_PROMPT1();
			}
		;

sql_list:
			sql 
			{
				YYAST->setRootNode($1);
			}
		|	sql_list ';' sql
			{
				YYAST->setRootNode($3);
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
				$3->leftSon = $5;
				$$ = $3;
			}
		|	CREATE INDEX index_name ON table_name '(' column_name ')'
			{
				$3->operation = OP_CREATE_INDEX;
				$3->leftSon = $5;
				$3->rightSon = $7;
				$$ = $3;
			}
		;

/* for extension */
table_name:
			NAME
			{
				$$ = YYAST->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_NAME;
			}
		;

index_name:
			NAME
			{
				$$ = YYAST->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_NAME;
			}
		;

column_name:	
			NAME
			{
				$$ = YYAST->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_NAME;
			}
		;

create_table_element_list:
			create_table_element
		|	create_table_element_list ',' create_table_element
			{
				$3->leftSon = $1;
				$$ = $3;
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
					$$->rightSon = YYAST->newEmptyNode();
					$$->rightSon->operation = $3;
				}
			}
		;

data_type:
			CHAR
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = VAL_CHAR;
				$$->numval = 2;
			}
		|	CHAR '(' NUMBER ')'
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = VAL_CHAR;
				$$->numval = $3+1;
			}
		|	INTEGER
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = VAL_INT;
			}
		|	FLOAT
			{
				$$ = YYAST->newEmptyNode();
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
				$$ = YYAST->newEmptyNode();
				$$->operation = DEF_SINGLE_PRIMARY;
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
			expr
		|	expr_list SYS_AND expr
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = OP_AND;
				$$->leftSon = $1;
				$$->rightSon = $3;
			}
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
				$$ = YYAST->newEmptyNode();
				$$->operation = CMP_EQ;
			}
		|	NEQ
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = CMP_NEQ;
			}
		|	LT
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = CMP_LT;
			}
		|	GT
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = CMP_GT;
			}
		|	LE
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = CMP_LE;
			}
		|	GE
			{
				$$ = YYAST->newEmptyNode();
				$$->operation = CMP_GE;
			}
		;

/* --- end select --- */


/* --- insert --- */
insert:	
			INSERT INTO table_name VALUES insert_val_list
			{
				$3->operation = OP_INSERT;
				$3->rightSon = $5;
				$3->leftSon = $5;
				$$ = $3;
			}
		;

insert_val_list:
			'(' column_value_list ')'
			{
				$$ = $2;
			}
		|	insert_val_list ',' '(' column_value_list ')'
			{
				$4->rightSon = $1;
				$$ = $4;
			}
		;



column_value_list:
			column_value
		|	column_value_list ',' column_value
			{
				$3->leftSon = $1;
				$$ = $3;
			}
		;

column_value:
			STRING
			{
				$$ = YYAST->newEmptyNode();
				$$->strval = $1;
				$$->operation = VAL_CHAR;
			}
		|	NUMBER
			{
				$$ = YYAST->newEmptyNode();
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

