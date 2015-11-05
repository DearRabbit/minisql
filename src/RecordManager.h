#pragma once

#include "NodeManager.h"

class RecordManager
{
private:
	static RecordManager* rm_delegate;
public:
	RecordManager();
	~RecordManager();

	// Table creation and deletion all happen in Catalogmgr
	
	// - function:
	// Insert a record into a table(.db file).
	// - return:
	// a pair of address
	CursePair new_entry_record(Node* node);		// insert record
	
	// - function:
	// Delete records from a table(.db file).
	// - Note: CurseT contains blocks' num & offset
	// - return:
	// n : return size of table;
	int delete_record(char* tableName, vector<CursePair>& curseTable);

	int delete_all_record(char* tableName);
	
	// - function:
	// Dumb selection with linear search.
	// -- node: A select AST. Must also contain a subtree
	// with table definition.(See also Catalogmgr::getTableDef())
	// -- curseTable: record block number & offset array, opaque to api.
	// Hasn't been initialized.
	// - return:
	// n: number of lines affected.
	int select_record_raw(Node* node, vector<CursePair>& curseTable);
	
	// - function:
	// Selection with linear search.
	// If curseTable(vector of pos pairs) is empty,
	// 	then search the whole table(rarely);
	// Else do Further selection on a curseTable,
	// 	Modify curseTable to get further results.
	// - note:
	// -- node: a single node containing a expr.
	// -- curseTable: record block number & offset array, opaque to api.
	// - return:
	// n: Number of lines selected.(size of curseTable)
	int select_record(Node* node, vector<CursePair>& curseTable);

	// - function:
	// Get data when constructing
	Node* get_column_data(char* tableName, char* columnName);

	// - function:
	// Print searching results.
	// -- curseTable: record position table.
	// return size of table;
	int print_select_record(char* tableName, vector<CursePair>& curseTable);

	int print_all_record(char* tableName);

	// to do 
	void assertMultipleKey(char* tableName, char* columnName, Node* data);

	static RecordManager* getInstance();
private:
};
