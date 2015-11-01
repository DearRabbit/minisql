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
	// n : number of line affected, currently 1;
	int new_record(Node* node);		// insert record
	
	// - function:
	// Delete records from a table(.db file).
	// - Note: CurseT contains blocks' num & offset
	// - return:
	// n : number of lines afftected.()
	int delete_record(char* tableName, CurseT& curseTable);


	// TO-DO: merge 2 function v:v
	/*
	// - function:
	// Dumb selection with linear search.
	// -- node: A select AST. Must also contain a subtree
	// with table definition.(See also Catalogmgr::getTableDef())
	// -- curseTable: record block number & offset array, opaque to api.
	// Hasn't been initialized.
	// - return:
	// n: number of lines affected.
	int select_record( Node* node, CurseT** curseTable);*/
	
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
	// n: Number of lines selected.
	int select_record(Node* node, CurseT& curseTable);

	// - function:
	// Print searching results.
	// -- curseTable: record position table.
	// Undefined print style;
	int print_record(char* tableName, CurseT& curseTable);

	static RecordManager* getInstance();
private:
};
