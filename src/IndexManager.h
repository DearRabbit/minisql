#ifndef __INDEX__MANAGER__
#define __INDEX__MANAGER__

#include "NodeManager.h"

class IndexManager
{
private:
	static IndexManager * im_delegate;
public:
	IndexManager();
	~IndexManager();
	

	// - function:
	// Create a new .idx file( And construct a B+Tree if necessary).
	// When the table if empty, no B+Tree will be constructed, that is,
	// the index file is empty with only fileheader in it.
	// - note:
	// Must be called together with Catalogmgr::new_index_def().
	// Index deletion happens in Catalogmgr::delete_index_def().
	// - takes:
	// -- node: A create index AST
	// - returns:
	// MINISQL_OK
	// MINISQL_EIO: If the index already exists.
	int new_index(Node* node);
		
	// drop index file happens in Catalogmgr.

	// - function:
	// Insert a new entry into an .idx file
	// - note:
	// Should be called during insertions
	// - takes:
	// -- node: An insert ASTNode, which is the same one taken
	// by recordmgr::new_record().
	// - return:
	// --- MINISQL_OK
	// --- MINISQL_ERR: If there exists a record with the same key.
	// --- MINISQL_EIO: If the table does not exist.
	int new_entry_idx(Node* node);

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
	int delete_entry_idx(Node* node);

	// - function:
	// Raw select from a select AST 
	// - note:
	// Should be called according to select conditions.
	// - takes:
	// -- node: A select record AST. 
	// -- curseTable: records position array.
	// - return:
	// --- MINISQL_OK
	// --- MINISQL_EIO: If the table does not exist.
	int select_index(Node* node, CurseT** curseTable);

	// - function:
	// Further selection that happens on a curseTable.
	// Modify curseTable to get further results.
	// - note:
	// rootAST should be refined somewhere(because this is not the first 
	// search) for easier and faster searching.
	// - return:
	// --- MINISQL_OK
	// --- MINISQL_EIO: If the table does not exist.
	int select_index( Node* node, CurseT* curseTable);

	static IndexManager* getInstance();
private:
};

#endif