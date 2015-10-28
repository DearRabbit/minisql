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
	// Create a new .idx file(nothing else)
	// - note:
	// Must be called together with Catalogmgr::new_index_def()
	// - takes:
	// -- node: A create index AST
	// - returns:
	// 1 on success
	int new_index(Node* node);		
	
	// - function:
	// Delete a .idx file
	// - note:
	// Must be called together with Catalogmgr::delete_index_def()
	// - takes:
	// -- node: A delete index AST
	int delete_index(Node* node);

	// - function:
	// Insert a new entry into an .idx file
	// - note:
	// Should be called during insertions
	// - takes:
	// -- node: An insert AST. Must be modified 
	// by Catalogmgr::getIndexDef().
	int new_entry_idx(Node* node);

	// - function:
	// Delete an entry from a .idx file
	// - note:
	// Should be called during deletion 
	// if an index is created on the table
	// - takes:
	// -- node: A delete record AST. Must be 
	// modified by Catalogmgr::getIndexDef().
	int delete_entry_idx(Node* node);

	// - function:
	// Raw select from a select AST 
	// - note:
	// Should be called according to select conditions.
	// - takes:
	// -- node: A select record AST. Must
	// be modified by Catalogmgr::getIndexDef()
	// -- curseTable: records position array 
	int select_index(Node* node, CurseT** curseTable);

	// - function:
	// Further selection that happens on a curseTable.
	// Modify curseTable to get further results.
	// - note:
	// rootAST should be refined somewhere(because this is not the first 
	// search) for easier and faster searching.
	int select_index( Node* node, CurseT* curseTable);

	static IndexManager* getInstance();
private:
};

#endif