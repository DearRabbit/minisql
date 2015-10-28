#include "Database.h"

extern int yyparse (class NodeManager *YYAST);

//initial uniptr
Database* Database::db_delegate;

Database::Database()
{
	db_delegate = this;
}
Database::~Database()
{
	m_ast.clean();
}
Database* Database::getInstance()
{
	return db_delegate;
}

void Database::run()
{
	printf("minisql> ");
	yyparse(&m_ast);
}
void Database::processAST()
{
	int i = 0;
	vector<Node*>& ast_root = m_ast.getRootTree();
	printf("Do something\n");

	for (auto it : ast_root)
	{
		printf("---------%03d---------:\n", ++i);
		if (it->strval)
			printf("String:%s\n", it->strval);
		printf("num:%lf\n", it->numval);
		printf("op:%d\n", it->operation);
	}
	m_ast.clean();
}
