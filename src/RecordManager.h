#pragma once

#include "NodeManager.h"
#include "BufferManager.h"
#include <cassert>
#include <cstring>
#include <cmath>

// related with BLOCK_SIZE & BLOCK_NUMBER
const int BLOCK_SIZE_OFFSET = 0;
const int BLOCK_SIZE_MASK = BLOCK_SIZE;
const int BLOCK_NUMBER_OFFSET = 12; //2^12 = 4096
const int BLOCK_NUMBER_MASK = BLOCK_NUMBER; //


// TO-DO: rearrange

// Head data at
// Block 0:
//
// |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
// |Header_string(16)|totalblock(4),totalentry(4),block_no(4),offset(4)|
// |_________________|_________________________________________________|
//
// |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|¯¯¯¯¯¯¯|
// |val_list_num(4)|val_type(4*n),val_length(4*n)|  ...  |
// |_______________|_____________________________|_______|

// for record,
// |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
// | val_list(...) | MSB=0 if deleted|block_no, offset|
// |_______________|__________________________________|


typedef struct {
	char 	Header_string[16];
	int 	blockCount;
	int 	entryCount;
	int 	nextEmptyNo;
	int 	nextEmptyOffset;
	int 	columnCount;

	int* 	columnType;
	int*	columnLength;

	size_t 	headerLength; //also the start
	size_t 	valLength;
	char* 	tableName;
	char** 	columnName; //optional
} RecFileInfo;

class RecordManager
{
private:
	static RecordManager* rm_delegate;
	RecFileInfo m_header;

	// BufMgr
	BufferManager* m_bufInstance;
	Pager* m_currentPage;

	void load_file(string& filename);
	void write_back();
	void assignColumnName(Node* data);
	void printBorder(int* tableLen, int len);

	bool cmpExpr(Node* expr, unsigned char *data, int columnid);

public:
	RecordManager();
	~RecordManager();
	static RecordManager* getInstance();

	// Table creation and deletion all happen in Catalogmgr
	
	// - function:
	// Insert a record into a table(.db file).
	// - return:
	// a pair of address
	CursePair new_entry_record(Node* root);		// insert record
	
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
	int select_record_raw(char* tableName, Node* node, Node* def, vector<CursePair>& curseTable);
	
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
	int select_record(char* tableName, Node* node, Node* def, vector<CursePair>& curseTable);

	// - function:
	// Get data when constructing
	Node* get_column_data(char* tableName, int columnId, vector<CursePair>& curseTable);

	// - function:
	// Print searching results.
	// -- curseTable: record position table.
	// return size of table;
	int print_select_record(char* tableName, Node* def, vector<CursePair>& curseTable);

	int print_all_record(char* tableName, Node* def);

	// to do 
	void assertMultipleKey(char* tableName, char* columnName, Node* data);


private:
};
