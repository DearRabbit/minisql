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
	// ---
	// node - A create AST
	// ---
	// returns 1 on success
	// returns 0 on failure	
	int new_table_def(Node* node);

	// function:
	// Write a new index definition into a .frm file
	// ---
	int new_index_def(Node* node);

	// function:
	// Delete a table definition from a .frm file
	int delete_table_def(Node* node);

	// function:
	// Delete an index definition from a .frm file
	int delete_index_def(Node* node);
	
	// function:
	// Modify node to attach to it the table 
	// definition. (node would be modified)
	// ---
	int getTableDef(Node* node);

	// function:
	// Modify node to attach to it the index
	// definition
	int getIndexDef(Node* node);


	static CatalogManager * getInstance();
private:
	//
};

#endif