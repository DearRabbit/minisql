#ifndef __CATALOG__MANAGER__
#define __CATALOG__MANAGER__

#include "NodeManager.h"


class CatalogManager
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
	// - MINISQL_EIO
	int new_index_def(Node* node);

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

#if 0	
	// function:

	// ---
	// return:
	// - MINISQL_OK

	int getTableDef(Node* node);

	// function:
	// Modify node to attach to it the index
	// definition
	int getIndexDef(Node* node);
#endif

	static CatalogManager * getInstance();
private:
	//
};

#endif