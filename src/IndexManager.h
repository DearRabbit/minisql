#pragma once

#include "NodeManager.h"

class IndexManager
{
private:
	static IndexManager * im_delegate;
public:
	IndexManager();
	~IndexManager();
	
	// merge two:(revoked)

	// - function:
	// Create a new B+Tree in idx file(create in catMgr)
	// When the table is empty, no B+Tree will be constructed,
	// that is, the index file is empty with only fileheader in it.
	// - note:
	// Must be called together with Catalogmgr::new_index_def(),
	// The function also requires data(sequential) from RecMgr 
	// to build B+Tree
	// - takes:
	// -- node: sequential data from table(rec)
	// - returns:
	// return nothing
	int new_index(Node* data, vector<CursePair>& cursorTable);
		
	// - function:
	// Insert a new entry into an .idx file
	// - note:
	// Should be called during insertions
	// - takes:
	// -- node: An insert ASTNode, which is the same one taken
	// by recordmgr::new_record().
	// - return:
	// --- MINISQL_OK
	// --- MINISQL_ECONSTRAINT: If there exists a record with the same key.
	// --- MINISQL_EIO: If the table does not exist.
	int new_entry_idx(char* tableName, char* columnName, CursePair& cursor);

	// Index deletion happens in Catalogmgr::delete_index_def().

	// - function:
	// Delete an entry from a .idx file
	// - note:
	// Should be called during deletion 
	// if an index is created on the table
	// - takes:
	// -- node: An delete ASTNode, which is the same one taken
	// by recordmgr::delete_record()
	// - return:
	// --- MINISQL_OK
	// --- MINISQL_EIO: If the table does not exist.
	int delete_entry_idx(char* tableName, char* columnName, CursePair& cursor);

	// - function:
	// Raw select from a select AST 
	// - note:
	// Should be called according to select conditions.
	// - takes:
	// -- node: A select from index AST, which should be constructed
	// in the api level.
	// -- curseTable: records position array.
	// - return:
	// return nothing now
	int select_index_raw(Node* node, vector<CursePair>& cursorTable);

	// - function:
	// If curseTable(vector of pos pairs) is empty,
	// 	then add record to curseTable;
	// Else do Further selection on a curseTable,
	// 	Modify curseTable to get further results.
	// - note:
	// -- node: a single node containing a expr.
	// -- curseTable: record block number & offset array, opaque to api.
	// - return:
	// n: Number of lines selected.(size of curseTable)
	int select_index(Node* node, vector<CursePair>& cursorTable);

	// to do 
	void assertMultipleKey(char* tableName, char* columnName, Node* data);

	static IndexManager* getInstance();
private:
};
