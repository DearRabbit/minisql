#pragma once

#include "NodeManager.h"

class TableExistException {};
class IndexExistException {};
class TableNonExistException {};
class ColumnNonExistException {};
class NotUniqueKeyException {};

class CatalogManager
// throw (TableExistException, TableNonExistException)
{
private:
	static CatalogManager * cm_delegate;
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
	// - MINISQL_OK
	// - MINISQL_EIO		
	int new_table_def(Node* node);

	// function:
	// Write a new index definition into a .frm file
	// Create a new .idx file, insert fileheader
	// - note: There cannot be two indexes with identicle name,
	// even if they are on different tables. (Which is strange
	// and different from sqlite or mysql)
	// ---
	// return:
	// - MINISQL_OK
	int new_index_def(char* tableName, char* columnName, char* indexName);

	// function:
	// Delete a table definition from a .frm file
	// Delete a .db file
	// ---
	// return:
	// - MINISQL_OK
	// - MINISQL_EIO
	int delete_table_def(Node* node);

	// function:
	// Delete an index definition from a .frm file
	// Delete a .idx file
	// ---
	// return:
	// - MINISQL_OK
	// - MINISQL_EIO
	int delete_index_def(Node* node);

	// function:
	// check if there is "tableName" existed;
	// use private func?: ifexist_table(); ****
	// throw TableExistException, catch by create_table;
	void assertExistTable(char* tableName) throw(TableExistException);
	void assertExistIndexName(char* indexName) throw(IndexExistException);

	void assertNonExistTable(char* tableName) throw(TableNonExistException);
	void assertNonExistColumn(char* tableName, char* columnName) throw(ColumnNonExistException);
	void assertNotUniqueKey(char* tableName, char* columnName) throw(NotUniqueKeyException);

	bool ifexist_index(Node* node);

	static CatalogManager * getInstance();
private:
	//
};
