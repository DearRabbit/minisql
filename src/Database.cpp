#include "Database.h"

extern int yyparse (class NodeManager *YYAST);

// initial uniptr
Database* Database::db_delegate;

// Nothing special
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
// Nothing special

bool Database::processSingleAST(Node* root)
{
	Node* ptr;
	int effect_rows = 0;
	switch(root->operation)
	{
		case OP_CREATE_TABLE:
			{
				// v: return 1 if success
				if (m_catMgr.new_table_def(root))
				{
					printf("Query OK, %d rows affected\n", effect_rows);
				}
				else
					fprintf(stderr, "Error: table %s already exists\n", root->strval);
			}
			break;
		case OP_CREATE_INDEX:
			{
				if (m_catMgr.new_index_def(root))
				{
					// ought to success...
					m_idxMgr.new_entry_idx(root);
					printf("Query OK, %d rows affected\n", effect_rows);
				}
				else
					fprintf(stderr, "Error: index %s already exists\n", root->strval);
			}
			break;
		case OP_DROP_TABLE:
			{
				if (m_catMgr.delete_table_def(root))
				{
					printf("Query OK, %d rows affected\n", effect_rows);
				}
				else
					fprintf(stderr, "Error: no such table: %s\n", root->strval);
			}
		case OP_DROP_INDEX:
			{
				if (m_catMgr.delete_index_def(root))
				{
					printf("Query OK, %d rows affected\n", effect_rows);
				}
				else
					fprintf(stderr, "Error: no such index: %s\n", root->strval);
			}
		case OP_INSERT:
			{
				
			}
		default: break;
	}
	return false;
}

//public
void Database::run()
{
	// for the first prompt.
	printf("minisql> ");
	yyparse(&m_ast);
}

// called by yyparse
void Database::processAST()
{
	// TO-DO: use API to rewrite
	const vector<Node*>& ast_root = m_ast.getRootTree();
	printf("Do something\n");

	for (auto it : ast_root)
	{
		processSingleAST(it);
	}
	m_ast.clean();
}
