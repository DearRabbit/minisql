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

// API as private
bool Database::db_createTable(Node *root)
{
	try
	{
		m_catMgr.assertExistTable(root->strval);
	}
	catch (TableExistException)
	{
		fprintf(stderr, "Error: table %s already exists\n", root->strval);
		return false;
	}

	Node* ptr = root;
	// !! ignore multiple primary key
	// !! take the lastdef of primary key
	while (ptr != nullptr)
	{
		// always at the first of list(last def)
		if (ptr->rightSon != nullptr && ptr->rightSon->operation == DEF_PRIMARY)
		{
			// create new index, should be succeed!!!
			// **!!  v:to be modified
			//       v:check again!!
			//       v             v:table       v:column               v:default_name
			m_catMgr.new_index_def(root->strval, ptr->rightSon->strval, "_def");
			break;
		}
		ptr = ptr->leftSon;
	}

	m_catMgr.new_table_def(root);
	// always print 0
	printf("Query OK, 0 rows affected\n");
	return true;
}
bool Database::db_createIndex(Node *root)
{
	try
	{
		//                 v:table name           v:column name
		assertNotUniqueKey(root->leftSon->strval, root->rightSon->strval);
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", root->strval);
		return false;
	}
	catch (ColumnNonExistException)
	{
		fprintf(stderr, "Error: table %s has no column named %s\n",\
			root->leftSon->strval, root->rightSon->strval);
		return false;
	}
	catch (NotUniqueKeyException)
	{
		fprintf(stderr, "Error: column %s is not a unique key\n",\
			root->rightSon->strval);
		return false;
	}

	//// check if there's idx already!
	// if (ifexist_column_idx())
	// {
	// 	Node *data = m_recMgr.get_column_data(table,column);
	// 	m_catMgr.new_index_def(root);
	// 	m_idxMgr.new_index(data);
	// }
	// else
	// {
	// 	m_catMgr.new_index_def(root);
	// }
	
	// always print 0
	printf("Query OK, 0 rows affected\n");
	return true;
}
bool Database::db_dropTable(Node *root)
{
	try
	{
		m_catMgr.assertNonExistTable(root->strval);
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", root->strval);
		return false;
	}
	
	m_catMgr.delete_table_def(root->strval);
	// always print 0
	printf("Query OK, 0 rows affected\n");
	return true;
}
bool Database::db_dropIndex(Node *root)
{
	try
	{
		m_catMgr.assertNonExistIndex(root->strval);
	}
	catch (IndexNonExistException)
	{
		fprintf(stderr, "Error: no such index: %s\n", root->strval);
		return false;
	}

	m_catMgr.delete_index_def(root->strval);
	// always print 0
	printf("Query OK, 0 rows affected\n");
	return true;
}
bool Database::db_insertVal(Node *root)
{
	try
	{
		m_catMgr.assertNonExistTable(root->strval);
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", root->strval);
		return false;
	}

	// **!!!! new func
	// a node mgr should be in catalogMgr
	// parameter can be string
	Node* columnDef = m_catMgr.getTableDef(root);
	Node* valPtr = root->right;
	while (valPtr != nullptr && columnDef != nullptr)
	{
		// insert into index
		if (columnDef->rightSon)
			// :m_idxMgr.new_entry_idx(:node);
		// :m_recMgr.new_record(:node);
	}
	// TO_DO: error processing
	//        && rollback()

	return true;

	/*int returnVal;
	if (m_catMgr.ifexist_index(root))
	{
		 returnVal = m_idxMgr.new_entry_idx(root);
		 if (returnVal < 0)
		 {
		 	processQueryError(root, returnVal);
			return false;
		 }
	}

	returnVal = m_recMgr.new_record(root);
	if (returnVal >= 0)
	{
		// print something
		printf("Query OK, %d rows affected\n", returnVal);
		return true;
	}
	else
		processQueryError(root, returnVal);
	return false;*/
}
bool Database::db_deleteVal(Node *root)
{
	return false;
}
bool Database::db_selectVal(Node *root)
{
	return false;
}


bool Database::processSingleAST(Node* root)
{
	bool result;
	switch (root->operation)
	{
		case OP_CREATE_TABLE:
			result = db_createTable(root);
		case OP_CREATE_INDEX:
			result = db_createIndex(root);
		case OP_DROP_TABLE:
			result = db_dropTable(root);
		case OP_DROP_INDEX:
			result = db_createIndex(root);
		case OP_INSERT:
			result = db_insertVal(root);
		case OP_SELECT:
			result = db_selectVal(root);
		case OP_DELECT:
			result = db_deleteVal(root);
		// !!!!!
		default: result = false;
	}
	return result;
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
	const std::vector<Node*>& ast_root = m_ast.getRootTree();
	printf("Do something\n");

	for (auto it : ast_root)
	{
		processSingleAST(it);
	}
	m_ast.clean();
}
