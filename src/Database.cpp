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

void Database::processQueryError(Node* root, int errorType)
{
	switch (errorType)
	{
		case MINISQL_ETYPE:

			break;
		case MINISQL_EIO:
			{
				switch (root->operation)
				{
					case OP_CREATE_TABLE:
						fprintf(stderr, "Error: table %s already exists\n", root->strval);
						break;
					case OP_CREATE_INDEX:
						fprintf(stderr, "Error: index %s already exists\n", root->strval);
						break;
					case OP_DROP_TABLE:
						fprintf(stderr, "Error: no such table: %s\n", root->strval);
						break;
					case OP_DROP_INDEX:
						fprintf(stderr, "Error: no such index: %s\n", root->strval);
						break;
					default: break;
				}
			}
			break;
		case MINISQL_ECONSTRAINT:
			break;
	}
}

// API as private
bool db_createTable(Node *root)
{
	int returnVal = m_catMgr.new_table_def(root);
	if (returnVal >= 0)
	{
		// always print 0
		printf("Query OK, %d rows affected\n", returnVal);
		return true;
	}
	else
		processQueryError(root, returnVal);
	return false
}
bool db_createIndex(Node *root)
{
	int returnVal = m_catMgr.new_index_def(root);
	if (returnVal >= 0)
	{
		// ought to success...
		m_idxMgr.new_index(root);
		// always print 0
		printf("Query OK, %d rows affected\n", returnVal);
		return true;
	}
	else
		processQueryError(root, returnVal);
	return false;
}
bool db_dropTable(Node *root)
{
	int returnVal = m_catMgr.delete_table_def(root);
	if (returnVal >= 0)
	{
		// always print 0
		printf("Query OK, %d rows affected\n", returnVal);
		return true;
	}
	else
		processQueryError(root, returnVal);
	return false;
}
bool db_dropIndex(Node *root)
{
	int returnVal = m_catMgr.delete_index_def(root);
	if (returnVal >= 0)
	{
		// always print 0
		printf("Query OK, %d rows affected\n", returnVal);
		return true;
	}
	else
		processQueryError(root, returnVal);
	return false;
}
bool db_insertVal(Node *root)
{
	return false;
}
bool db_selectVal(Node *root)
{
	return false;
}
bool db_deleteVal(Node *root)
{
	return false;
}

bool Database::processSingleAST(Node* root)
{
	switch (root->operation)
	{
		case OP_CREATE_TABLE:
			return db_createTable(root);
		case OP_CREATE_INDEX:
			return db_createIndex(root);
		case OP_DROP_TABLE:
			return db_dropTable(root);
		case OP_DROP_INDEX:
			return db_createIndex(root);
		//!!!!
		case OP_INSERT:
			return db_insertVal(root);
		case OP_SELECT:
			return db_selectVal(root);
		case OP_DELECT:
			return db_deleteVal(root);
		default: return false;
	}
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
