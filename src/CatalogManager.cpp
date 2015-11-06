#include "CatalogManager.h"

cm_catRoot;

CatalogManager* CatalogManager::cm_delegate = NULL;

CatalogManager::CatalogManager()
{
	cm_delegate = this;
}

CatalogManager::~CatalogManager()
{

}

int
CatalogManager::new_table_def(Node* node)
{
	Node *ptr = cm_catNodeMgr.newCopyDataNode(node);
	ptr->leftSon = cm_catRoot->leftSon;
	cm_catRoot->leftSon = ptr;
	++cm_catRoot->numval;
	return 0;
}

int
CatalogManager::new_index_def(char* tableName, char* columnName, char* indexName)
{
	Node *ptr = cm_catRoot->leftSon;
	Node *tmpPtr;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) 
				{
					ptr = ptr->rightSon;
					tmpPtr = cm_catNodeMgr.newEmptyNode();
					STRDUP_NEW(tmpPtr->strval, indexName);
					if (/*ptr!=nullptr && */ptr->rightSon != nullptr)
					{
						tmpPtr->rightSon = ptr->rightSon;
						ptr->rightSon = tmpPtr;
						return 1;
					}
					else
					{
						ptr->rightSon = tmpPtr;
						return 0;
					}
				}	
				ptr = ptr->leftSon;
			}
		}
		ptr = ptr->leftSon;
	}
	assert(1);
}

int
CatalogManager::delete_table_def(char* tableName)
{
	string tmpStr(tableName);
	tmpStr+=".db";
	BufferManager::getInstance()->deleteFile(tmpStr.c_str());

	Node *ptr = cm_catRoot->leftSon;
	Node *tmpPtr = cm_catRoot;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			tmpPtr->leftSon = ptr->leftSon;
			return 0;
		}
		tmpPtr = ptr;
		ptr = ptr->leftSon;
	}
	assert(1);
}

int
CatalogManager::delete_index_def(char* indexName)
{
	string tmpStr(tableName);
	tmpStr+=".idx";
	BufferManager::getInstance()->deleteFile(tmpStr.c_str());

	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	Node *indexPtr = nullptr;
	Node *tmpPtr = nullptr;
	while (tablePtr != nullptr)
	{
		columnPtr = tablePtr->rightSon;
		while (columnPtr != nullptr)
		{
			if (columnPtr->rightSon != nullptr)
			{
				indexPtr = columnPtr->rightSon->rightSon;
				tmpPtr = columnPtr->rightSon;
				while (indexPtr != nullptr)
				{
					if (strcmp(indexName, indexPtr->strval))
					{
						// first node: tp = primary key
						if (tmpPtr->strval == nullptr)
							tmpPtr->rightSon = indexPtr->leftSon;
						// else
						else
							tmpPtr->leftSon = indexPtr->leftSon;
					}
					tmpPtr = indexPtr;
					indexPtr = indexPtr->leftSon;
				}
			}
			columnPtr = columnPtr->leftSon;
		}
		tablePtr = tablePtr->leftSon;
	}
	assert(1);
}

Node*
CatalogManager::get_column_def(char* tableName)
{

}

Node*
CatalogManager::get_column_def(char* tableName, char* columnName)
{

}


// done
bool
CatalogManager::ifexist_table(char* tableName)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
			return true;
		ptr = ptr->leftSon;
	}
	return false;
}

bool
CatalogManager::ifexist_index(char* indexName)
{
	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	Node *indexPtr = nullptr;
	while (tablePtr != nullptr)
	{
		columnPtr = tablePtr->rightSon;
		while (columnPtr != nullptr)
		{
			if (columnPtr->rightSon != nullptr)
			{
				indexPtr = columnPtr->rightSon->rightSon;
				while (indexPtr != nullptr)
				{
					if (strcmp(indexName, indexPtr->strval))
					{
						return true;
					}
					indexPtr = indexPtr->leftSon;
				}
			}
			columnPtr = columnPtr->leftSon;
		}
		tablePtr = tablePtr->leftSon;
	}
	return false;
}

bool
CatalogManager::if_unique_key(char* tableName, char* columnName)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) && ptr->rightSon!=nullptr &&\
					 ptr->rightSon->operation >= DEF_UNIQUE)
					return true;
				ptr = ptr->leftSon;
			}
			return false;
		}
		ptr = ptr->leftSon;
	}
	return false;
}

bool
CatalogManager::ifexist_index_on_column(char* tableName, char* columnName)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) && ptr->rightSon!=nullptr&&\
					 ptr->rightSon->rightSon != nullptr)
					return true;
				ptr = ptr->leftSon;
			}
			return false;
		}
		ptr = ptr->leftSon;
	}
	return false;
}

void
CatalogManager::assertExistTable(char* tableName)
throw(TableExistException)
{
	if (ifexist_table(tableName))
		throw TableExistException());
}

void 
CatalogManager::assertExistIndex(char* indexName) 
throw(IndexExistException)
{
	if (ifexist_index(indexName))
		throw IndexExistException();
}

void
CatalogManager::assertNonExistTable(char* tableName) 
throw(TableNonExistException)
{
	if (!ifexist_table(tableName))
		throw TableExistException());
}
	
void
CatalogManager::assertNonExistIndex(char* indexName) 
throw(IndexNonExistException)
{
	if (!ifexist_index(indexName))
		throw IndexNonExistException();
}

void
CatalogManager::assertNonExistColumn(char* tableName, char* columnName) 
throw(ColumnNonExistException)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval))
					return true;
				ptr = ptr->leftSon;
			}
			throw ColumnNonExistException();
		}
		ptr = ptr->leftSon;
	}
	throw ColumnNonExistException();
}

void
CatalogManager::assertNotUniqueKey(char* tableName, char* columnName) 
throw(NotUniqueKeyException)
{
	if (!if_unique_key(tableName, columnName))
		throw NotUniqueKeyException();
}

CatalogManager*
CatalogManager::getInstance()
{
	return cm_delegate;
}
