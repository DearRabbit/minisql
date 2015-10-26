#ifndef __CATALOG__MANAGER__
#define __CATALOG__MANAGER__

#include <string>
#include <list>

using std::list;
using std::string;

/*
typedef struct
{
	string col_name;
	int    unique;
	///!!! datatype enums
} DBColumn;

typedef struct
{
	string table_name;
	unsigned int ncol;
	DBColumn* cols[32];

	// table name, attributes names and types, unique or not.
} DBTable;

typedef struct
{
	string   idx_name;
	DBTable* idx_table;
	DBColumn* idx_col;

	// index name, on which table, on which col.
} DBIndex;

typedef struct
{
	string  db_name;
	list < DBTable* >  db_tables;
	list < DBIndex* >  db_indexes;

	// table name, table col nums, primary keys, indexes.
} DBDef;
*/

struct DBColumn;  // should be opaque to the api level
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

	int new_database_def(string& db_name);
	int delete_database_def(string& db_name);
	
	int new_table_def(string& table_name, string* col_names, int * col_types, unsigned int ncol);
	int new_index_def(string& idx_name,  string& table_name, string& col_name);
	int delete_table_def(string& table_name);
	int delete_index_def(string& table_name);	
	
	static CatalogManager * getInstance();

//  Called across mgrs only 
	DBTable* getDBTable();
	
private:
	int cm_writeDBDef(DBDef* db_def);
//
};

#endif
