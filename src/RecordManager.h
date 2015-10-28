#ifndef __RECORD__MANAGER__
#define __RECORD__MANAGER__

#include "NodeManager.h"

class RecordManager
{
private:
	static RecordManager* rm_delegate;
public:
	RecordManager();
	~RecordManager();

	// - function:
	// create a .db file(nothing else)
	// --
	// return 1 on success
	int new_table( Node* node );

	// - function:
	// delete a .db file
	int delete_table( Node* node );		
	
	// - function:
	// Insert a record into a table(.db file).
	// - note:
	// If an index on the table exists, call idxmgr::new_entry_idx() as well
	int new_record( Node* node );		// insert record
	
	// - function:
	// Delete a record from a table(.db file).
	// - Note: Should be called with Indexmgr::delete_leaf_idx
	// when there is at least one index on the table
	// -- node: A delete AST. Must also contain a subtree 
	// with table definition.(See also Catalogmgr::getTableDef())
	int delete_record( Node* node);

	// - function:
	// Dumb selection with linear search.
	// -- node: A select AST. Must also contain a subtree
	// with table definition.(See also Catalogmgr::getTableDef())
	// -- curseTable: record block number & offset array, opaque to api.
	// Hasn't been initialized.
	int select_record( Node* node, CurseT** curseTable);
	
	// - function:
	// Further selection that happens on a curseTable.
	// Modify curseTable to get further results.
	// - note:
	// node should be refined somewhere(because this is not the first 
	// search)for easier and faster searching.
	int select_record( Node* node, CurseT* curseTable);

	// - function:
	// Print searching results.
	// -- node: Any AST with table name in it.
	// -- curseTable: record position table.
	int print_record( Node* node, CurseT* curseTable);

	static RecordManager* getInstance();
private:
};


#endif