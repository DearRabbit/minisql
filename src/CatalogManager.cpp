#include "CatalogManager.h"

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

}

int
CatalogManager::new_index_def(char* tableName, char* columnName, char* indexName)
{

}

int
CatalogManager::delete_table_def(char* tableName)
{

}

int
CatalogManager::delete_index_def(char* indexName)
{

}

Node*
CatalogManager::get_column_def(char* tableName)
{

}

Node*
CatalogManager::get_column_def(char* tableName, char* columnName)
{

}

bool
CatalogManager::ifexist_table(char* tableName)
{

}

bool
CatalogManager::ifexist_index(char* indexName)
{

}

bool
CatalogManager::if_unique_key(char* tableName, char* columnName)
{

}

bool
CatalogManager::ifexist_index_on_column(char* tableName, char* columnName)
{

}

void
CatalogManager::assertExistTable(char* tableName)
throw(TableExistException)
{

}

void 
CatalogManager::assertExistIndex(char* indexName) 
throw(IndexExistException)
{

}

void
CatalogManager::assertNonExistTable(char* tableName) 
throw(TableNonExistException)
{

}
	
void
CatalogManager::assertNonExistIndex(char* indexName) 
throw(IndexNonExistException)
{

}

void
CatalogManager::assertNonExistColumn(char* tableName, char* columnName) 
throw(ColumnNonExistException)
{

}

void
CatalogManager::assertNotUniqueKey(char* tableName, char* columnName) 
throw(NotUniqueKeyException)
{

}

CatalogManager*
CatalogManager::getInstance()
{
	return cm_delegate;
}
