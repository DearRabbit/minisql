#include <cstdio>
#include <cstring>
#include <iostream>
#include "Database.h"

extern int yyparse(Database *YYDatabase);

int main()
{
	Database a;
	printf("minisql> ");
	yyparse(&a);	
	return 0;
}