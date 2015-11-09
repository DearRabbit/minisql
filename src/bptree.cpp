#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include "BufferManager.h"
#include "NodeManager.h"
#include "bptree.h"
#include <assert.h>	

using std::vector;


BPT::BPT(const char* filename)
{
	bpt_pager = BufferManager::getInstance()->getPager(filename);
	unsigned char* block = BufferManager::getInstance()->getblock(bpt_pager, 0, BUFFER_FLAG_NONDIRTY);
	// Then the block ptr is freed, so no need to assign dirty bit
	memcpy(&bpt_fileheader, block, IDX_FILEHEADER_SIZE);
}

BPT::~BPT()
{
	unsigned char* block = BufferManager::getInstance()->getblock(bpt_pager, 0, BUFFER_FLAG_DIRTY);
	memcpy(block, &bpt_fileheader, IDX_FILEHEADER_SIZE);
    for(auto tmpNode : bpt_nodes) {
        bptree_writeNode(tmpNode);
        bptree_freeNodePage(tmpNode);
    }
}

int
BPT::insertEntry(void* key, unsigned int block_t, unsigned int block_ofs)
{
	int leaf_page;
	int celln;
	// tree is empty construct the tree first
	if(bpt_fileheader.Root == IDX_FLAG_NONPAGE) {
		BPTNode* rootNode = NULL;
		bptree_getNodeByPage(0,&rootNode);
		rootNode->parent_block = IDX_FLAG_NOROOT;
		rootNode->ncells = 0;
		rootNode->freespc_ofs = IDX_BLOCKHEADER_SIZE + IDX_FILEHEADER_SIZE;
		rootNode->cells_ofs = IDX_BLOCKHEADER_SIZE + IDX_FILEHEADER_SIZE;
		rootNode->right_page = IDX_FLAG_NONPAGE;
		rootNode->is_leaf = IDX_FLAG_LEAF;
		rootNode->page = 0;
		rootNode->cells = new vector<BPTCell*>;
		bpt_fileheader.Root = 0;
	}
	if( bptree_search(key, &leaf_page, &celln) == 0 ) {
		return 0;			// insertion failed, key already there
	}
	BPTNode* leafNode=NULL;
	bptree_getNodeByPage(leaf_page, &leafNode);
	BPTCell* newCell=NULL;
	bptree_newLeafCell(&newCell, key, block_t, block_ofs);

#if _DEBUG
    assert(leafNode!=NULL);
    assert(newCell!=NULL);
#endif

	if( leafNode->ncells < bpt_fileheader.Degree-1 ){    // leafNode not full
		bptree_insertCell(newCell, leafNode, celln);
	}
	else {
		BPTNode* node_quote = NULL;
		vector<BPTCell*> vec = *(leafNode->cells);	// copy
		vec.insert(vec.begin()+celln, newCell);
		leafNode->cells->clear();					// clear cells
		leafNode->ncells = 0;
		leafNode->freespc_ofs = leafNode->cells_ofs;
		bptree_createEmptyNode(leafNode->parent_block, &node_quote, leafNode->is_leaf);
		bptree_split(leafNode, vec, node_quote);
		node_quote->right_page = leafNode->right_page;
		leafNode->right_page = node_quote->page;
		BPTCell* midcell=NULL;
		bptree_newNodeCell(&midcell, bptree_getKey4Cell(node_quote->cells->at(0)), leafNode->page);
		bptree_insertParentCell( leafNode, midcell , node_quote);
	}
	return 1;
}

int
BPT::deleteEntry(CursePair blockPtr)
{
	if(bpt_fileheader.Root == IDX_FLAG_NOROOT)
		return 0;

	BPTNode* node;
	bptree_getNodeByPage(bpt_fileheader.Root, &node);
	while(node->is_leaf != IDX_FLAG_LEAF) {
		bptree_getNodeByPage(node->cells->at(0)->left_ptr.left_page, &node);
	}
	int cellCount;
	while(1) {
		cellCount = 0;
		for(auto tmpCell : *(node->cells)) {
			if(blockPtr.first == tmpCell->left_ptr.leaf_ptr.n_block && 
			    blockPtr.second == tmpCell->left_ptr.leaf_ptr.ofs_block){
				bptree_deleteCell(node, cellCount);
				return 1;
			}
			cellCount++;
		}
		if(node->right_page == IDX_FLAG_NONPAGE)
			break;
		else 
			bptree_getNodeByPage(node->right_page, &node);
	}
	return 0;
}

int
BPT::deleteEntry(void* key)
{
	int leaf_page;
	int celln;
	if( bptree_search(key, &leaf_page, &celln) ) {
        printf("Delete: key not found.\n");
        printf("Key: %d", *(int*)key);
		exit(1);		// deletion failed, key is not there
	}
	BPTNode* leafNode=NULL;
	bptree_getNodeByPage(leaf_page, &leafNode);
	bptree_deleteCell(leafNode, celln);
	return 1;
}

int
BPT::select(Node* expr, vector<CursePair>& cursor)
{
    if(bpt_fileheader.Root == IDX_FLAG_NOROOT)
        return 0;
    
	void* value = new char[bpt_fileheader.Val_size];
	Node* valPtr = expr->rightSon;
    
	if(bpt_fileheader.Type == IDX_TYPE_INT) {
        int tInt = (int)(valPtr->numval);
        memcpy(value, &tInt, bpt_fileheader.Val_size);
	}
	else if(bpt_fileheader.Type == IDX_TYPE_FLOAT) {
        float tFloat = (float)(valPtr->numval);
        memcpy(value, &tFloat, bpt_fileheader.Val_size);
	}
	else if(bpt_fileheader.Type == IDX_TYPE_STRING) {
        memcpy(value, valPtr->strval, bpt_fileheader.Val_size);
	}

	int pagen, celln, notFound;
	BPTNode* bptNode=NULL;
	BPTNode* minNode=NULL;
	CursePair blockPos;

	notFound = bptree_search(value, &pagen, &celln);
	bptree_getNodeByPage(pagen, &bptNode);

	bptree_getNodeByPage(bpt_fileheader.Root, &minNode);
	while(minNode->is_leaf == IDX_FLAG_NONLEAF) {
		bptree_getNodeByPage(minNode->cells->at(0)->left_ptr.left_page, &minNode);
	}

	if(expr->operation == CMP_EQ) {
		if(!notFound){
			blockPos.first  = bptNode->cells->at(celln)->left_ptr.leaf_ptr.n_block;
			blockPos.second = bptNode->cells->at(celln)->left_ptr.leaf_ptr.ofs_block;
			cursor.push_back(blockPos);	// copy
		}
		// else not found
	}
	else if(expr->operation == CMP_NEQ) {
		BPTNode* node = minNode;
		if(notFound){
			blockPos.first = -1;
			blockPos.second = -1;
		}
		else {
			blockPos.first  = bptNode->cells->at(celln)->left_ptr.leaf_ptr.n_block;
			blockPos.second = bptNode->cells->at(celln)->left_ptr.leaf_ptr.ofs_block;
		}
		while(true) {
			for(auto tmpCell : *(node->cells)) {
				if(tmpCell->left_ptr.leaf_ptr.n_block == blockPos.first &&
			 	   tmpCell->left_ptr.leaf_ptr.ofs_block == blockPos.second) {
					
					continue;
				}
				else {
					cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
												tmpCell->left_ptr.leaf_ptr.ofs_block));
				}
			}
			if(node->right_page != IDX_FLAG_NONPAGE)
				bptree_getNodeByPage(node->right_page, &node);
			else
				break;
		}
	}
	else if(expr->operation == CMP_LT) {
		BPTNode* node = minNode;
		while(node->page != pagen) {
			for(auto tmpCell : *(node->cells) ) {
				cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
											tmpCell->left_ptr.leaf_ptr.ofs_block));
			}
			bptree_getNodeByPage(node->right_page, &node);
		}
		for( int iCell=0; iCell < node->ncells; iCell++) {
			if(iCell == celln) break;
			BPTCell* tmpCell = node->cells->at(iCell);
			cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
										tmpCell->left_ptr.leaf_ptr.ofs_block));
		}
	}
	else if(expr->operation == CMP_GT) {
		BPTNode* node = bptNode;
		int     iCell = celln;
		if(celln == node->ncells) {//shit
			if(node->right_page != IDX_FLAG_NONPAGE){
				bptree_getNodeByPage(node->right_page, &node);
				iCell = 0;
			}
		}
		else {
			iCell = celln + 1;
		}
		while(true) {
			for(; iCell<node->ncells; iCell++) {
				BPTCell* tmpCell = node->cells->at(iCell);
				cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
											tmpCell->left_ptr.leaf_ptr.ofs_block));
			}
			if(node->right_page != IDX_FLAG_NONPAGE) {
				bptree_getNodeByPage(node->right_page, &node);
				iCell = 0;
			}
			else
				break;
		}
	}
	else if(expr->operation == CMP_LE) {
		BPTNode* node = minNode;
		int     fCell = celln;
		if(!notFound) {
			fCell++;
		}
		// fCell is the pos where traverse should end
		while(node->page != pagen) {
			for(auto tmpCell : *(node->cells)) {
				cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
											tmpCell->left_ptr.leaf_ptr.ofs_block));
			}
			bptree_getNodeByPage(node->right_page, &node);
		}
		for(int iCell=0; iCell < fCell; iCell++) {
			BPTCell* tmpCell = node->cells->at(iCell);
			cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
											tmpCell->left_ptr.leaf_ptr.ofs_block));
		}
	}
	else if(expr->operation == CMP_GE) {
		BPTNode* node = bptNode;
		int     iCell = celln;
		if(notFound) {
			if(celln == node->ncells) {
				if(node->right_page != IDX_FLAG_NONPAGE) {
					bptree_getNodeByPage(node->right_page, &node);
					iCell = 0;
				}
			}
			else
				iCell = celln + 1;
		}
		// else iCell = celln
		while(true) {
			for(; iCell<node->ncells; iCell++) {
				BPTCell* tmpCell = node->cells->at(iCell);
				cursor.push_back( CursePair(tmpCell->left_ptr.leaf_ptr.n_block, 
											tmpCell->left_ptr.leaf_ptr.ofs_block));
			}
			if(node->right_page != IDX_FLAG_NONPAGE) {
				bptree_getNodeByPage(node->right_page, &node);
				iCell = 0;
			}
			else
				break;
		}
	}
    
    delete[] (char*)value;
    return 1;
}

int
BPT::ifexist(void* key)
{
	int rst;
	int pagen, celln;
	rst = bptree_search(key, &pagen, &celln);
	return !rst;
}
// - function: create an empty node and init its parent field and its isleaf field
int
BPT::bptree_createEmptyNode(unsigned int parentn, BPTNode** bptNode, unsigned char flag)
{
	///Freelist Management
	if(bpt_fileheader.N_freepages >= 1) {
		bptree_getNodeByPage(bpt_fileheader.Free_list, bptNode);
		bpt_fileheader.Free_list = (*bptNode)->right_page;
		bpt_fileheader.N_freepages--;
		(*bptNode)->parent_block = parentn;
		(*bptNode)->is_leaf = flag;
		(*bptNode)->ncells = 0;
		(*bptNode)->cells->clear();
		(*bptNode)->freespc_ofs = (*bptNode)->cells_ofs;
		(*bptNode)->right_page = IDX_FLAG_NONPAGE;
	}
	else { //new block
		unsigned char* block = BufferManager::getInstance()->newblock(bpt_pager, BUFFER_FLAG_DIRTY);
		BufferManager::getInstance()->pinblock(bpt_pager, bpt_pager->getNPages()-1);
		bptree_initNode(bptNode, block);
		(*bptNode)->parent_block = parentn;
		(*bptNode)->is_leaf = flag;
		(*bptNode)->freespc_ofs = IDX_BLOCKHEADER_SIZE;
		(*bptNode)->cells_ofs = IDX_BLOCKHEADER_SIZE;
		(*bptNode)->right_page = IDX_FLAG_NONPAGE;
		(*bptNode)->page = bpt_pager->getNPages()-1;
		(*bptNode)->ncells = 0;
		(*bptNode)->cells->clear();
	    // For management
	    bpt_nodes.push_back(*bptNode);
	}
	return 1;
}

int
BPT::bptree_initNodeFirstPage(BPTNode** node, unsigned char* block)
{
	*node = new BPTNode;
	memcpy((unsigned char*)*node, block+IDX_FILEHEADER_SIZE, IDX_BLOCKHEADER_SIZE);
	(*node)->block = block;
	(*node)->cells = new vector<BPTCell*>;
	unsigned char* block_pos = (*node)->block+IDX_BLOCKHEADER_SIZE +
							(	(*node)->page==0 ? IDX_FILEHEADER_SIZE:0   );
	for(unsigned int i=0;i<(*node)->ncells; i++) {
		BPTCell* cell = new BPTCell;
		memcpy( (char*)&(cell->left_ptr), block_pos, IDX_LEFTPTR_SIZE);
		block_pos+=IDX_LEFTPTR_SIZE;
		cell->val = new char[bpt_fileheader.Val_size];
		memcpy( cell->val, block_pos, bpt_fileheader.Val_size);
		(*node)->cells->insert( (*node)->cells->begin()+i, cell);
		block_pos+=bpt_fileheader.Val_size;
	}
	return 1;
}
// - function: init a node from its corresponding block
int
BPT::bptree_initNode(BPTNode** bptNode, unsigned char* block)
{
	*bptNode = new BPTNode;
	memcpy((unsigned char*)*bptNode, block, IDX_BLOCKHEADER_SIZE);
	(*bptNode)->block = block;
	(*bptNode)->cells = new vector<BPTCell*>;
	unsigned char* block_pos = (*bptNode)->block+IDX_BLOCKHEADER_SIZE +
							(	(*bptNode)->page==0 ? IDX_FILEHEADER_SIZE:0   );
	for(unsigned int i=0;i<(*bptNode)->ncells; i++) {
		BPTCell* cell = new BPTCell;
		memcpy( (char*)&(cell->left_ptr), block_pos, IDX_LEFTPTR_SIZE);
		block_pos+=IDX_LEFTPTR_SIZE;
		cell->val = new char[bpt_fileheader.Val_size];
		memcpy( cell->val, block_pos, bpt_fileheader.Val_size);
		(*bptNode)->cells->insert( (*bptNode)->cells->begin()+i, cell);
		block_pos+=bpt_fileheader.Val_size;
	}
	return 1;
}
// - Flush the Node to the block in the buffer
int
BPT::bptree_writeNode(BPTNode* node)
{
	if(node->page == 0) {
		memcpy(node->block+IDX_FILEHEADER_SIZE, (unsigned char*)node, IDX_BLOCKHEADER_SIZE);
	}
	else{
		memcpy(node->block, (unsigned char*)node, IDX_BLOCKHEADER_SIZE);
	}
	unsigned char* block_pos = node->block+IDX_BLOCKHEADER_SIZE +
								(	node->page==0 ? IDX_FILEHEADER_SIZE:0   );
	for(unsigned int i=0; i<node->ncells; i++) {
		memcpy(block_pos, &(node->cells->at(i)->left_ptr),IDX_LEFTPTR_SIZE);
		block_pos+=IDX_LEFTPTR_SIZE;
		memcpy(block_pos, node->cells->at(i)->val, bpt_fileheader.Val_size);
		block_pos+=bpt_fileheader.Val_size;
	}
	return 1;
}

int
BPT::bptree_getNodeByPage(unsigned int pagen, BPTNode** bptNode)
{
#if _DEBUG
if(pagen>= bpt_pager->getNPages()){
    int i=0;
}
assert(pagen < bpt_pager->getNPages());
//assert(*bptNode==NULL);
#endif // _DEBUG

    for(auto tmpNode: bpt_nodes) {
        if(tmpNode->page == pagen) {
            *bptNode = tmpNode;
            return 1;
        }
    }
	// If get node in the first page
	unsigned char* block = BufferManager::getInstance()->getblock(bpt_pager, pagen, BUFFER_FLAG_DIRTY);
#if _DEBUG
    assert(block!=NULL);
#endif

	BufferManager::getInstance()->pinblock(bpt_pager, pagen);
	if( pagen==0 ) {
		bptree_initNodeFirstPage(bptNode, block);
	}
	else{
		bptree_initNode( bptNode,  block);
	}
	// for management
    bpt_nodes.push_back(*bptNode);
	return 1;
}
// - function: destroy the BPTNode
// - note: Not necessarily release the block
int
BPT::bptree_freeNodePage(BPTNode* bptNode)
{
	BufferManager::getInstance()->unpinblock(bpt_pager, bptNode->page);
	for(unsigned int i=0; i<bptNode->ncells; i++) {
		bptree_freeCell(bptNode->cells->at(i));
	}
	delete   bptNode->cells;
	delete   bptNode;
	return 1;
}
// - The creation of a cell. Destruction can happen at many places,
// like in the destruction of a node and when splitting a parent node.
int
BPT::bptree_newNodeCell(BPTCell** bptCell, void* key, int left_page)
{
	*bptCell = new BPTCell;
	(*bptCell)->val = new char[bpt_fileheader.Val_size];

	memcpy( (*bptCell)->val, key, bpt_fileheader.Val_size );
	(*bptCell)->left_ptr.left_page = left_page;
	return 1;
}

int
BPT::bptree_newLeafCell(BPTCell** bptCell, void* key, int page, unsigned int offset)
{
	*bptCell = new BPTCell;
	(*bptCell)->val = new char[bpt_fileheader.Val_size];

	memcpy( (*bptCell)->val, key, bpt_fileheader.Val_size );

	(*bptCell)->left_ptr.leaf_ptr.n_block = page;
	(*bptCell)->left_ptr.leaf_ptr.ofs_block = offset;
	return 1;
}

int
BPT::bptree_insertParentCell(BPTNode* leftKid, BPTCell* midcell ,BPTNode* rightKid)
{
	if(leftKid->page == bpt_fileheader.Root) {
		BPTNode* newRoot;

		bptree_createEmptyNode(IDX_FLAG_NOROOT, &newRoot, IDX_FLAG_NONLEAF);
		bptree_insertCell(midcell, newRoot, 0);
		newRoot->right_page = rightKid->page;
		bpt_fileheader.Root = newRoot->page;
        rightKid->parent_block = leftKid->parent_block = newRoot->page;
		return 1;
	}
	else {
		BPTNode* parent=NULL;
		bptree_getNodeByPage(leftKid->parent_block, &parent);

		unsigned int cell_number=0;     // find the cell in the parent block that points to the leftKid
		for(auto cell : *(parent->cells) ) {
			if(cell->left_ptr.left_page == leftKid->page){
				cell->left_ptr.left_page = rightKid->page;
				break;
			}
			cell_number++;
		}
		if(cell_number == parent->ncells) parent->right_page = rightKid->page;
		if(parent->ncells < bpt_fileheader.Degree-1) {      // parent not full. Insert.
			bptree_insertCell(midcell, parent, cell_number);
		}
		else {              // Split the parent node
			BPTNode* node_quote = NULL;
			vector<BPTCell*> vec = *(parent->cells);	// copy
			vec.insert(vec.begin()+cell_number, midcell);// add midcell
			parent->cells->clear();					// clear cells
			parent->ncells = 0;
			parent->freespc_ofs = parent->cells_ofs;
			int parent_rightpage = parent->right_page;
			bptree_createEmptyNode(parent->parent_block, &node_quote, parent->is_leaf);
            // redistribute cells
			unsigned int i=0;
			for(;i<bpt_fileheader.Degree/2;i++) {
                // Insert cells into parent(left)
				bptree_insertCell(vec.at(i), parent, i);
			}
			// Three ptrs on the left
			parent->right_page = vec.at(i)->left_ptr.left_page;
			// New midcell, for parent's parent
			midcell=NULL;
			bptree_newNodeCell(&midcell, bptree_getKey4Cell(vec.at(i)), parent->page);
			// always throw a cell away
			bptree_freeCell(vec.at(i));
			i++;	// throw a key away
            // Insert cells into parent(right)
			for(unsigned int j=0; i<bpt_fileheader.Degree; i++, j++) {
				bptree_insertCell(vec.at(i), node_quote, j);
			}
			node_quote->right_page = parent_rightpage;
            // Update kids' parents
            BPTNode* tmpNode = NULL;
            for( auto tmpCell : *(parent->cells) ) {
                bptree_getNodeByPage(tmpCell->left_ptr.left_page, &tmpNode);
                tmpNode->parent_block = parent->page;
            }
            bptree_getNodeByPage(parent->right_page, &tmpNode);
            tmpNode->parent_block = parent->page;
            for( auto tmpCell : *(node_quote->cells) ) {
                bptree_getNodeByPage(tmpCell->left_ptr.left_page, &tmpNode);
                tmpNode->parent_block = node_quote->page;
            }
          		 bptree_getNodeByPage(node_quote->right_page, &tmpNode);
            tmpNode->parent_block = node_quote->page;
			bptree_insertParentCell( parent, midcell , node_quote);
			}
	}
    return 1;
}

int
BPT::bptree_freeCell(BPTCell* cell)
{
	delete[] (char*)cell->val;
	delete          cell;

	return 1;
}
// - function: insert new cell in the block, ncells++, update freespc_ofs
// - note:
int
BPT::bptree_insertCell(BPTCell* bptCell, BPTNode* bptNode, unsigned int celln)
{
#if _DEBUG
	assert(celln <= bptNode->ncells);
    assert(celln < bpt_fileheader.Degree);
	assert(bptNode->ncells < bpt_fileheader.Degree);
	#endif // _DEBUG

    bptNode->cells->insert(bptNode->cells->begin()+celln, bptCell);
	    bptNode->freespc_ofs+=bpt_fileheader.Val_size + IDX_LEFTPTR_SIZE;
    	bptNode->ncells ++;
	return 1;
}

int
BPT::bptree_split(BPTNode* left, vector<BPTCell*>&  vec, BPTNode* right)
{
	unsigned int i;
	for(i=0; i<(bpt_fileheader.Degree+1)/2; i++) {
		bptree_insertCell(vec.at(i), left, i);
	}
	for(unsigned int j=0;i<bpt_fileheader.Degree;j++,i++) {
		bptree_insertCell(vec.at(i), right, j);
	}
	return 1;
}

void*
BPT::bptree_getKey4Cell(BPTCell* cell)
{
    return cell->val;
}
// - This function is just crazy
int
BPT::bptree_deleteCell(BPTNode* bptNode, unsigned int celln)
{
    // Destroy the cell
	bptree_freeCell( bptNode->cells->at(celln) );	// another cell destroy
	bptNode->cells->erase(bptNode->cells->begin()+celln);
	bptNode->freespc_ofs-=bpt_fileheader.Val_size + IDX_LEFTPTR_SIZE;
	bptNode->ncells--;
    // The root is empty,
	if( bptNode->page == bpt_fileheader.Root )
    {
        // If root is not a leaf and has less than 2 children
		if(bptNode->ncells == 0 && bptNode->is_leaf == IDX_FLAG_NONLEAF) {
            bpt_fileheader.Root = bptNode->right_page;
            bptree_addDeletedTag(bptNode);
		}
        // If root is a leaf and has less than 1 cell
        else if( bptNode->ncells == 0 && bptNode->is_leaf == IDX_FLAG_LEAF ) {
            bpt_fileheader.Root = IDX_FLAG_NONPAGE;
            bptree_addDeletedTag(bptNode);
        }
	} // Too small a node, redistribute or merge
	else if(   ( ( bptNode->is_leaf == IDX_FLAG_LEAF )
						&& ( bptNode->ncells < ( bpt_fileheader.Degree/2 ) )  )
		     ||( ( bptNode->is_leaf == IDX_FLAG_NONLEAF )
		     			&& ( bptNode->ncells < ( bpt_fileheader.Degree-1 )/2 ) ) )
    {
		int leftBob;
		int rightBob = bptNode->page;   // mostly right, sometimes left
		BPTNode* leftNode = NULL;
		BPTNode* rightNode = bptNode;
		BPTNode* opennedNode;
		BPTNode* parentNode = NULL;

		bptree_getNodeByPage(bptNode->parent_block, &parentNode);
        // Find cell in parent that points to the node that is deleted a cell from.
		unsigned int i=0; unsigned int parent_cell;
		for(; i < parentNode->ncells; i++) {
			if( parentNode->cells->at(i)->left_ptr.left_page == bptNode->page ) {
				break;
			}
		}
		// If the cell is unfortunately the first in parentNode.
		if(i==0) {
			parent_cell = 0;
			// Switch left & right
			leftBob  = rightBob;
			// Set the right bob.
			rightBob = ( (parentNode->ncells == 1) ? parentNode->right_page :
									parentNode->cells->at(i+1)->left_ptr.left_page );
			leftNode = rightNode; rightNode = NULL;
			bptree_getNodeByPage(rightBob, &rightNode);
			opennedNode = rightNode;
		}
		else {
			parent_cell = i-1;
			leftBob  = parentNode->cells->at(i-1)->left_ptr.left_page;
			bptree_getNodeByPage(leftBob, &leftNode);
			opennedNode = leftNode;
		}
		//redistribute
		if( (  (leftNode->ncells+rightNode->ncells >= bpt_fileheader.Degree) &&
						(leftNode->is_leaf == IDX_FLAG_LEAF)  ) ||
			(  (leftNode->ncells+rightNode->ncells >= bpt_fileheader.Degree-1) &&
			    		(leftNode->is_leaf == IDX_FLAG_NONLEAF) )   )
{	// Redistribute
	if(leftNode->is_leaf == IDX_FLAG_LEAF) {
		if(leftNode->ncells > rightNode->ncells) {
			BPTCell* tmpcell = leftNode->cells->at(leftNode->ncells-1);

			leftNode->cells->erase(leftNode->cells->begin()+leftNode->ncells-1);
			leftNode->ncells--;
			leftNode->freespc_ofs-=IDX_LEFTPTR_SIZE + bpt_fileheader.Val_size;
			bptree_insertCell(tmpcell, rightNode,0);
			memcpy(parentNode->cells->at(parent_cell)->val, tmpcell->val, bpt_fileheader.Val_size);
		}
		else {
			BPTCell* tmpcell = rightNode->cells->at(0);

			rightNode->cells->erase(rightNode->cells->begin());
			rightNode->ncells--;
			rightNode->freespc_ofs-=IDX_LEFTPTR_SIZE + bpt_fileheader.Val_size;
			bptree_insertCell(tmpcell, leftNode, leftNode->ncells);
			memcpy(parentNode->cells->at(parent_cell)->val, rightNode->cells->at(0)->val, bpt_fileheader.Val_size);
		}
	}
	else {	// IDX_FLAG_NONLEAF
        //
		if(leftNode->ncells > rightNode->ncells) {
			void*    tmpval = new char[bpt_fileheader.Val_size];
			BPTCell* tmpcell = leftNode->cells->at(leftNode->ncells-1);
			int      tPage = leftNode->right_page;

			leftNode->right_page = tmpcell->left_ptr.left_page;
			leftNode->cells->erase(leftNode->cells->begin()+leftNode->ncells-1);
			leftNode->ncells--;
			leftNode->freespc_ofs-=IDX_LEFTPTR_SIZE + bpt_fileheader.Val_size;
            tmpcell->left_ptr.left_page = tPage;
			memcpy(tmpval, tmpcell->val, bpt_fileheader.Val_size);
			memcpy(tmpcell->val, parentNode->cells->at(parent_cell)->val, bpt_fileheader.Val_size);
			bptree_insertCell(tmpcell, rightNode, 0);
			memcpy(parentNode->cells->at(parent_cell)->val, tmpval , bpt_fileheader.Val_size);
			delete[] (char*)tmpval;
			// update kids' parent
			BPTNode* tmpNode;
            bptree_getNodeByPage(tPage, &tmpNode);
            tmpNode->parent_block = rightNode->page;
		}
		else {
			void*    tmpval  = new char[bpt_fileheader.Val_size];
			BPTCell* tmpcell = rightNode->cells->at(0);
			int      tPage   = tmpcell->left_ptr.left_page;

			tmpcell->left_ptr.left_page = leftNode->right_page;
			rightNode->cells->erase( rightNode->cells->begin() );
			rightNode->ncells--;
			rightNode->freespc_ofs-=IDX_LEFTPTR_SIZE + bpt_fileheader.Val_size;
			leftNode->right_page = tPage;
			memcpy(tmpval, tmpcell->val, bpt_fileheader.Val_size);
			memcpy(tmpcell->val, parentNode->cells->at(parent_cell)->val, bpt_fileheader.Val_size);
			bptree_insertCell(tmpcell, leftNode, leftNode->ncells);
			memcpy(parentNode->cells->at(parent_cell)->val, tmpval, bpt_fileheader.Val_size);
			delete[] (char*)tmpval;

			BPTNode* tmpNode;
            bptree_getNodeByPage(tPage, &tmpNode);
            tmpNode->parent_block = leftNode->page;
		}
	}
}
			else
{	// merge
	if(leftNode->is_leaf == IDX_FLAG_LEAF) {
		for( auto i : *(rightNode->cells) ) {	// special. This is ptr reassignment.
			bptree_insertCell( i, leftNode, leftNode->ncells);
		}
		leftNode->right_page = rightNode->right_page;
		rightNode->ncells = 0;
		rightNode->cells->clear();
		bptree_addDeletedTag(rightNode);
		if(parent_cell == parentNode->ncells-1) {
			parentNode->right_page = leftNode->page;
		} else {
			parentNode->cells->at(parent_cell+1)->left_ptr.left_page = leftNode->page;
		}
		bptree_deleteCell(parentNode, parent_cell);
	}
	else {      // A nonleaf node merge
		BPTCell* newcell = new BPTCell;
		newcell->val = new char[bpt_fileheader.Val_size];
		memcpy(newcell->val, parentNode->cells->at(parent_cell)->val, bpt_fileheader.Val_size);
		newcell->left_ptr.left_page = leftNode->right_page;
		bptree_insertCell(newcell, leftNode, leftNode->ncells);
		for(auto i: *(rightNode->cells)) {
			bptree_insertCell( i, leftNode, leftNode->ncells);
		}
		leftNode->right_page = rightNode->right_page;
		rightNode->ncells = 0;
		rightNode->cells->clear();
		bptree_addDeletedTag(rightNode);
		if(parent_cell == parentNode->ncells-1) {
			parentNode->right_page = leftNode->page;
		}
		else {
			parentNode->cells->at(parent_cell+1)->left_ptr.left_page = leftNode->page;
		}
        //Update kids' parent
        BPTNode *tmpNode;
        for(auto tmpCell: *(leftNode->cells)) {
            bptree_getNodeByPage(tmpCell->left_ptr.left_page, &tmpNode);
            tmpNode->parent_block = leftNode->page;
        }
        bptree_getNodeByPage(leftNode->right_page, &tmpNode);
        tmpNode->parent_block = leftNode->page;
		bptree_deleteCell(parentNode, parent_cell);
	}
}

	}//End of Reditribute or Merge.
	return 1;
}

// function: find a position for key
// takes:
// - leaf_page: page number of the leaf node
// - celln : cell number of the cell
// returns:
// - 1: no key found
// - 0: the key is found
// - pagen: the result leaf block
// - celln: the result cell number
int
BPT::bptree_search(void* key, int* pagen, int* celln)
{
    if(bpt_fileheader.Root == IDX_FLAG_NOROOT) return 1;
 
    BPTNode* root = NULL;
	bptree_getNodeByPage(  bpt_fileheader.Root , &root);

#if _DEBUG
assert(root!=NULL);
#endif // _DEBUG

	BPTNode* node = root;
	int      next_page=IDX_FLAG_NONPAGE;

	while( node->is_leaf == IDX_FLAG_NONLEAF ) {
		next_page = IDX_FLAG_NONPAGE;
		for(auto i : *(node->cells) ) {
			if( bptree_cellcmp(key , i->val) < 0 ) {
				next_page = i->left_ptr.left_page;
				break;
			}
		}
		if(next_page == IDX_FLAG_NONPAGE) {
			next_page = node->right_page;
		}
		node = NULL;
		bptree_getNodeByPage(next_page, &node);
	}
    *pagen = node->page;

    for(unsigned int i=0; i<node->ncells; i++ ) {
        BPTCell* tmp = node->cells->at(i);
        if( bptree_cellcmp(key, tmp->val) == 0 ) {
            *celln = i;
            return 0;
        }
        else if( bptree_cellcmp(key, tmp->val) < 0 ) {
            *celln = i;
            return 1;
        }
    }
    *celln = node->ncells;      // at the end of the node
	return 1;
}

int
BPT::bptree_addDeletedTag(BPTNode* node)
{
	node->right_page = bpt_fileheader.Free_list;
	bpt_fileheader.Free_list = node->page;
	bpt_fileheader.N_freepages++;
	return 1;
}

int
BPT::bptree_cellcmp(void* key, void* val)
{
	if(bpt_fileheader.Type == IDX_TYPE_INT) {
		return *(int*)key - *(int*)val;
	}
	else if(bpt_fileheader.Type == IDX_TYPE_FLOAT) {
		return *(float*)key - *(float*)val;
	}
	else if(bpt_fileheader.Type == IDX_TYPE_STRING) {
		return strcmp((char*)key, (char*)val);
	}
    return 0;
}

#if _DEBUG
#include <list>
using std::list;

void
BPT::print()
{
    if(bpt_fileheader.Root==IDX_FLAG_NONPAGE) return;
    list<int> nextPageQueue;

    int nextLevelNode=0;
    int level=0;
    int lCount=0;
    int lNode=1;
    int qHead;
    BPTNode* node = NULL;

    nextPageQueue.push_back(bpt_fileheader.Root);
    while(!nextPageQueue.empty()) {
        qHead = nextPageQueue.front(); nextPageQueue.pop_front();
        bptree_getNodeByPage(qHead, &node);
        printf(" (%d) ", node->page);
        for(auto cellPtr: *(node->cells) ) {
        	if(bpt_fileheader.Type == IDX_TYPE_STRING)
	            printf("%s ", (char*)cellPtr->val );
	        else if(bpt_fileheader.Type == IDX_TYPE_FLOAT)
	        	printf("%f ", *(float*)cellPtr->val );
	        else if(bpt_fileheader.Type == IDX_TYPE_INT)
	        	printf("%d ", *(int*)cellPtr->val );
            if(node->is_leaf == IDX_FLAG_NONLEAF){
                nextPageQueue.push_back(cellPtr->left_ptr.left_page);
                nextLevelNode++;
            }
        }
        printf(" [%d] ", node->parent_block);
        printf("| ");
        if(node->is_leaf == IDX_FLAG_NONLEAF){
            nextPageQueue.push_back(node->right_page);
            nextLevelNode++;
        }
        lCount++;
        if(lCount == lNode) {
            lCount = 0;
            lNode = nextLevelNode;
            nextLevelNode = 0;
            level++;
            printf("\n");
        }
        node = NULL;
    }
}
#endif

int
BPT::construct(Node* root, vector<CursePair>& cTable)
{
    return 0;
}


