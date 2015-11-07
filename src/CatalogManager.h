#pragma once

#include "NodeManager.h"
#include <map>
// +------------------+--------
// |Hedader_string(16)|
// +------------------+--------
// 
// 
//
// Corresponds to a Node
const int CAT_NAME_MAXSIZE = 32;

using std::map;

typedef map<Node*, int> NodePosMap;

typedef struct 
{
	int lBlock;
	int lOffset;
	int rBlock;
	int rOffset;
} BlockPtr;

typedef struct
{// 4 + 32 + 8 + 32
	int  operation;
	char strval[CAT_NAME_MAXSIZE];
	double numval;
	BlockPtr leftSon;
	BlockPtr rightSon;
} FrmNode;

class CatalogManager
{
private:
	static CatalogManager * cm_delegate;
	NodeManager m_columndef;
	NodeManager cm_catNodeMgr;
	Node*       cm_catRoot;
	vector<Node*> cm_idxList;
	NodePosMap  nodeMap;
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
	// inverted sequence
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
	// !!!!!!*************!!!!! maybe make it a macro
	// -if there's already a index on the column,
	//  return 1(to reassign)
	// -else return 0
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

	// function:
	// Return the ColumnDef of table 'name'
	// point to last column of def 
	// reverse
	// !!! with operaion type
	Node* get_column_def(char* tableName);
	Node* get_column_def(char* tableName, char* columnName);

	int   get_column_id(char* tableName, char* columnName);

	void print_show_tables(char *tableName);
	
	// function groups:
	// test if condis true;
	// call by assertfunc
	bool ifexist_table(char* tableName);
	bool ifexist_index(char* indexName);
	bool if_unique_key(char* tableName, char* columnName);
	bool ifexist_index_on_column(char* tableName, char* columnName);

	// function groups:
	// assert if cond;
	void assertExistTable(char* tableName) throw(TableExistException);
	void assertExistIndex(char* indexName) throw(IndexExistException);
	void assertNonExistTable(char* tableName) throw(TableNonExistException);
	void assertNonExistIndex(char* indexName) throw(IndexNonExistException);
	void assertNonExistColumn(char* tableName, char* columnName) throw(ColumnNonExistException);
	void assertNotUniqueKey(char* tableName, char* columnName) throw(NotUniqueKeyException);

	static CatalogManager * getInstance();
private:
	//
};
