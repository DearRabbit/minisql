#pragma once

#include "NodeManager.h"

class SQLCatException {};
class TableExistException:SQLCatException {};
class IndexExistException:SQLCatException {};
class TableNonExistException:SQLCatException {};
class IndexNonExistException:SQLCatException {};
class ColumnNonExistException:SQLCatException {};
class NotUniqueKeyException:SQLCatException {};

class CatalogManager throw (SQLCatException)
{
private:
	static CatalogManager * cm_delegate;
	NodeManager m_columndef;
	
public:
	CatalogManager();
	~CatalogManager();

	//int new_database_def(char* db_name);		//for multiple db, useless for now
	//int delete_database_def(char* db_name);

	// function:
	// Write a new table definition into a .frm file.
	// Create a new .db file, insert fileheader.
	// ---
	// node - A create AST
	// ---
	// return : 		
	// nothing now.
	int new_table_def(Node* node);

	// function:
	// Write a new index definition into a .frm file
	// Create a new .idx file, insert fileheader
	// - note: There cannot be two indexes with identicle name,
	// even if they are on different tables. (Which is strange
	// and different from sqlite or mysql)
	// ---
	// return:
	// nothing now.
	int new_index_def(char* tableName, char* columnName, char* indexName);

	// function:
	// Delete a table definition from a .frm file
	// Delete a .db file
	// ---
	// return:
	// nothing now.
	int delete_table_def(char* tableName);

	// function:
	// Delete an index definition from a .frm file
	// Delete a .idx file
	// ---
	// return:
	// nothing now.
	int delete_index_def(char* indexName);

	// function groups:
	// assert if cond;
	void assertExistTable(char* tableName) throw(TableExistException);
	void assertExistIndexName(char* indexName) throw(IndexExistException);
	void assertNonExistTable(char* tableName) throw(TableNonExistException);
	void assertNonExistIndex(char* indexName) throw(IndexNonExistException);
	void assertNonExistColumn(char* tableName, char* columnName) throw(ColumnNonExistException);
	void assertNotUniqueKey(char* tableName, char* columnName) throw(NotUniqueKeyException);
	// function groups:

	static CatalogManager * getInstance();
private:
	//
};
