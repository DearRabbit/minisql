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
			m_catMgr.new_index_def(root->strval, ptr->rightSon->strval, (char*)"_def");
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
	char* tableName = root->leftSon->strval;
	char* columnName = root->rightSon->strval;
	char* indexName = root->strval;
	try
	{
		// maybe put inside
		m_catMgr.assertNotUniqueKey(tableName, columnName);
		m_catMgr.assertExistIndex(indexName);
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", tableName);
		return false;
	}
	catch (ColumnNonExistException)
	{
		fprintf(stderr, "Error: table %s has no column named %s\n", tableName, columnName);
		return false;
	}
	catch (NotUniqueKeyException)
	{
		fprintf(stderr, "Error: column %s is not a unique key\n", columnName);
		return false;
	}
	catch (IndexExistException)
	{
		fprintf(stderr, "Error: index %s already exists\n", indexName);
		return false;
	}
	
	// check if there's idx on the column already!
	// if not, new index in idxmgr
	// something...!!!!
	if (m_catMgr.new_index_def(tableName, columnName, indexName))
	{
		vector<CursePair> cursor;
		Node *data = m_recMgr.get_column_data(tableName, columnName, cursor);
		m_idxMgr.new_index(data, cursor);
	}
	
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
	Node *columnDef = nullptr;
	Node *ptrDef = nullptr;
	Node *ptrData = root->leftSon;

	try
	{
		m_catMgr.assertNonExistTable(root->strval);
		ptrDef = columnDef = m_catMgr.get_column_def(root->strval);

		while (ptrData != nullptr)
		{
			if (ptrDef == nullptr)
			{
				throw TypeMismatchException("too many values");
			}
			if (CHECK_TYPE(ptrDef->operation, ptrData->strval))
			{
				throw TypeMismatchException(string("at column ")+string(ptrDef->strval));
			}
			if (m_catMgr.if_unique_key(root->strval, ptrDef->strval))
			{
				//                                   v:tableName   v:columnName
				if (m_catMgr.ifexist_index_on_column(root->strval, ptrDef->strval))
				{
					m_idxMgr.assertMultipleKey(root->strval, ptrDef->strval, ptrData);
				}
				else
				{
					m_recMgr.assertMultipleKey(root->strval, ptrDef->strval, ptrData);
				}
			}
			ptrData = ptrData->leftSon;
			ptrDef = ptrDef->leftSon;
		}
		if (ptrDef != nullptr)
		{
			throw TypeMismatchException("too many values");
		}
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", root->strval);
		return false;
	}
	catch (MultipleKeyException e)
	{
		fprintf(stderr, "Error: UNIQUE constraint failed: %s\n", e.name.c_str());
		return false;
	}
	catch (TypeMismatchException e)
	{
		fprintf(stderr, "Error: type mismatch: %s\n", e.columnName.c_str());		
		return false;
	}

	CursePair cursor = m_recMgr.new_entry_record(root);
	ptrDef = columnDef;
	ptrData = root->leftSon;
	while(ptrDef != nullptr)
	{
		if (m_catMgr.ifexist_index_on_column(root->strval, ptrDef->strval))
		{
			m_idxMgr.new_entry_idx(root->strval, ptrDef->strval, cursor);
		}
		ptrDef = ptrDef->leftSon;
		ptrData = ptrData->leftSon;
	}
 	
	// currently print 1
	printf("Query OK, 1 rows affected\n");
	return true;
}
bool Database::db_selectVal(Node *root)
{
	// |¯¯¯¯¯¯¯¯|
	// |  root  |
	// |________|
	//        v(R)
	// 		 |¯¯¯¯¯¯¯¯|
	// 		 |AND(...)|
	// 		 |________|
	//        v     v
	// |¯¯¯¯¯¯¯¯|  |¯¯¯¯¯¯¯¯|
	// | cmp(op)|  | cmp(op)|
	// |________|  |________|
	//  v      v
	// column  val
	Node* ptrConj = root->rightSon;
	Node* ptrExpr = nullptr;
	Node* columnDef = nullptr;
	vector<Node*> columnWithIndex;
	vector<Node*> columnWithoutIndex;
	try
	{
		m_catMgr.assertNonExistTable(root->strval);

		while (ptrConj != nullptr)
		{
			// now support 'and' only.
			// ptrExpr->operation <= OP_OR equals to
			// ptrExpr->operation == OP_AND || OP_OR
			if (ptrConj->operation == OP_AND)
			{
				// column name put in the left side
				ptrExpr = ptrConj->leftSon;
			}
			else
			{
				ptrExpr = ptrConj;
				// end here
				ptrConj = ptrConj->rightSon;
			}
			m_catMgr.assertNonExistColumn(root->strval, ptrExpr->leftSon->strval);
			columnDef = m_catMgr.get_column_def(root->strval, ptrExpr->leftSon->strval);

			if (m_catMgr.ifexist_index_on_column(root->strval, ptrExpr->leftSon->strval))
			{
				columnWithIndex.push_back(ptrExpr);
			}
			else
			{
				columnWithoutIndex.push_back(ptrExpr);
			}

			// check type
			if (CHECK_TYPE(columnDef->operation, ptrExpr->rightSon->strval))
				throw TypeMismatchException(string("at column ")+string(ptrExpr->leftSon->strval));

			ptrConj = ptrConj->rightSon;
		}
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", root->strval);
		return false;
	}
	catch (ColumnNonExistException)
	{
		fprintf(stderr, "Error: table %s has no column named %s\n",\
		 	root->strval, ptrExpr->leftSon->strval);
		return false;
	}
	catch (TypeMismatchException e)
	{
		fprintf(stderr, "Error: type mismatch: %s\n", e.columnName.c_str());		
		return false;
	}

	if (root->rightSon == nullptr)
		// currently support select *
		m_recMgr.print_all_record(root->strval);
	else
	{
		vector<CursePair> cursor;
		size_t tmpSize = columnWithIndex.size();

		for (auto it : columnWithIndex)
		{
			if (it == columnWithIndex[0])
				m_idxMgr.select_record_raw(it, cursor);
			else
				m_idxMgr.select_record(it, cursor);
		}
		for (auto it : columnWithoutIndex)
		{
			if (tmpSize == 0 && it == columnWithoutIndex[0])
				m_recMgr.select_record_raw(it, cursor);
			else
				m_recMgr.select_record(it, cursor);
		}

		m_recMgr.print_select_record(root->strval, cursor);
	}
	// always print 0
	printf("Query OK, 0 rows affected\n");
	return true;
}
bool Database::db_deleteVal(Node *root)
{
	Node* ptrConj = root->rightSon;
	Node* ptrExpr = nullptr;
	Node* columnDef = nullptr;
	vector<Node*> columnWithIndex;
	vector<Node*> columnWithoutIndex;
	try
	{
		m_catMgr.assertNonExistTable(root->strval);

		while (ptrConj != nullptr)
		{
			// now support 'and' only.
			// ptrExpr->operation <= OP_OR equals to
			// ptrExpr->operation == OP_AND || OP_OR
			if (ptrConj->operation == OP_AND)
			{
				// column name put in the left side
				ptrExpr = ptrConj->leftSon;
			}
			else
			{
				ptrExpr = ptrConj;
				// end here
				ptrConj = ptrConj->rightSon;
			}
			m_catMgr.assertNonExistColumn(root->strval, ptrExpr->leftSon->strval);
			columnDef = m_catMgr.get_column_def(root->strval, ptrExpr->leftSon->strval);

			if (m_catMgr.ifexist_index_on_column(root->strval, ptrExpr->leftSon->strval))
			{
				columnWithIndex.push_back(ptrExpr);
			}
			else
			{
				columnWithoutIndex.push_back(ptrExpr);
			}

			// check type
			if (CHECK_TYPE(columnDef->operation, ptrExpr->rightSon->strval))
				throw TypeMismatchException(string("at column ")+string(ptrExpr->leftSon->strval));

			ptrConj = ptrConj->rightSon;
		}
	}
	catch (TableNonExistException)
	{
		fprintf(stderr, "Error: no such table: %s\n", root->strval);
		return false;
	}
	catch (ColumnNonExistException)
	{
		fprintf(stderr, "Error: table %s has no column named %s\n",\
		 	root->strval, ptrExpr->leftSon->strval);
		return false;
	}
	catch (TypeMismatchException e)
	{
		fprintf(stderr, "Error: type mismatch: %s\n", e.columnName.c_str());		
		return false;
	}

	int returnVal;
	if (root->rightSon == nullptr)
	{
		returnVal = m_recMgr.delete_all_record(root->strval);
		// TO-DO
		// delete all thing on the idx of table 'name'
	}	
	else
	{
		vector<CursePair> cursor;
		size_t tmpSize = columnWithIndex.size();

		for (auto it : columnWithIndex)
		{
			if (it == columnWithIndex[0])
				m_idxMgr.select_record_raw(it, cursor);
			else
				m_idxMgr.select_record(it, cursor);
		}
		for (auto it : columnWithoutIndex)
		{
			if (tmpSize == 0 && it == columnWithoutIndex[0])
				m_recMgr.select_record_raw(it, cursor);
			else
				m_recMgr.select_record(it, cursor);
		}
		returnVal = m_recMgr.delete_record(root->strval, cursor);
	}
	// delete index
	// for (auto it : something from cat?)
	// {
		//// TO-DO
		// m_idxMgr.delete_all_on_column(table, column, cursor);
	// }

	printf("Query OK, %d rows affected\n", returnVal);
	return true;
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
	const vector<Node*>& ast_root = m_ast.getRootTree();
	printf("Do something\n");

	for (auto it : ast_root)
	{
		processSingleAST(it);
	}
	m_ast.clean();
}
