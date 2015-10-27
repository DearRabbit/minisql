#ifndef __CATALOG__MANAGER__
#define __CATALOG__MANAGER__


struct DBColumn;
struct DBTable;
struct DBIndex;
struct DBDef;


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
	// nodeAST - A create AST
	// ---
	// returns 1 on success
	// returns 0 on failure	
	int new_table_def(NodeAST* nodeAST);

	// function:
	// Write a new index definition into a .frm file
	// ---
	int new_index_def(NodeAST* nodeAST);

	// function:
	// Delete a table definition from a .frm file
	int delete_table_def(NodeAST* nodeAST);

	// function:
	// Delete an index definition from a .frm file
	int delete_index_def(NodeAST* nodeAST);
	
	// function:
	// Modify nodeAST to attach to it the table 
	// definition. (nodeAST would be modified)
	// ---
	int getTableDef(NodeAST* nodeAST);

	// function:
	// Modify nodeAST to attach to it the index
	// definition
	int getIndexDef(NodeAST* nodeAST);


	static CatalogManager * getInstance();
private:
	//
};

#endif