#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include "BufferManager.h"
#include "NodeManager.h"

#define _DEBUG 0

#include <assert.h>

using std::vector;

typedef struct
{
	void* val;
	union
	{
		int left_page;
		struct
		{
			int n_block;
			int ofs_block;
		} leaf_ptr;
	} left_ptr;
} BPTCell;

typedef struct
{
	// page header
    int 		   parent_block;
	unsigned int   ncells;
	unsigned int   freespc_ofs;
	unsigned int   cells_ofs;		//
	int 		   right_page;		// right page number in a file
	unsigned int   is_leaf;			// leaf or not
	int 		   page;			// page number
	// run-time information
	unsigned char* block;
	vector<BPTCell*>* cells;
} BPTNode;		// combined with a block.	The block get and free must be handled in the tree

class BPT
{
private:
	Pager*              bpt_pager;
    IDXFileHeader       bpt_fileheader;
    vector<BPTNode*>    bpt_nodes;
public:
	BPT(const char* filename);
	~BPT();
	int insertEntry(void* key, unsigned int block_t, unsigned int block_ofs);
	int deleteEntry(void* key);
	int 	 select(Node* exprNode, vector<CursePair>& cursor);
	// bulk loading( not implemented yet)
	int   construct(Node* nodeAST, vector<CursePair>& cursor);
	int 	ifexist(void* key);
#if _DEBUG
    void print();
#endif // _DEBUG

private:
	int bptree_initNodeFirstPage(BPTNode** node, unsigned char* block);
	int          bptree_initNode(BPTNode** bptNode, unsigned char* block);
	int   bptree_createEmptyNode(unsigned int parent, BPTNode** bptNode, unsigned char flag);
	int     bptree_getNodeByPage(unsigned int pagen, BPTNode** bptNode);
	int         bptree_writeNode(BPTNode* bptNode);
	int      bptree_freeNodePage(BPTNode* bptNode);
	int       bptree_newNodeCell(BPTCell** bptCell, void* key, int block_t);
	int       bptree_newLeafCell(BPTCell** bptCell, void* key, int block_t, unsigned int block_ofs);
	int        bptree_deleteCell(BPTNode* bptNode, unsigned int celln);
	int        bptree_insertCell(BPTCell* bptCell, BPTNode* bptNode, unsigned int celln);
	int  bptree_insertParentCell(BPTNode* leftKid, BPTCell* midcell ,BPTNode* rightKid);
	int          bptree_freeCell(BPTCell* cell);

	int             bptree_split(BPTNode* left, vector<BPTCell*> &vec, BPTNode* right );
	void*     bptree_getKey4Cell(BPTCell* cell);
	int            bptree_search(void* key, int* pagen, int* celln);
	int     bptree_addDeletedTag(BPTNode* node);
	int           bptree_cellcmp(void* key, void* val);
};
