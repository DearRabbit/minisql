#include "CatalogManager.h"
#include "BufferManager.h"
#include <unistd.h>
#include <assert.h>

using std::pair;
using std::vector;

typedef pair<Node*, BlockPtr> NodePair;

CatalogManager* CatalogManager::cm_delegate = NULL;

CatalogManager::CatalogManager()
{
	cm_delegate = this;
	BufferManager* bufmgr = BufferManager::getInstance();
	Pager* pager;

	if(access("minisql.frm", R_OK)!=0) {
		bufmgr->createFile("minisql.frm");
		cm_catRoot = cm_catNodeMgr.newEmptyNode();
		cm_catNodeMgr.setRootNode(cm_catRoot);
	}
	else {
		cm_catRoot = cm_catNodeMgr.newEmptyNode();
		cm_catNodeMgr.setRootNode(cm_catRoot);
		pager = bufmgr->getPager("minisql.frm");

		size_t const_offset = sizeof(int)*6 + sizeof(double);
		size_t var_offset=0;
		unsigned char* block0 = bufmgr->getblock(pager, 0, BUFFER_FLAG_NONDIRTY);
		
		memcpy((char*)&(cm_catRoot->operation), block0, sizeof(int) );
		cm_catRoot->strval = new char[CAT_NAME_MAXSIZE];
		memcpy(cm_catRoot->strval, block0+sizeof(int), CAT_NAME_MAXSIZE);
		memcpy((char*)&(cm_catRoot->numval), block0+2*sizeof(int), sizeof(double));
		
		var_offset = sizeof(int)*2+sizeof(double);

		BlockPtr bPtr;
		memcpy(&bPtr, block0+var_offset, sizeof(BlockPtr));

		Node* node = cm_catRoot;
		Node* left = nullptr;

		unsigned char* block;
		vector<NodePair> bPtrStack;
		int count=0;
		while(bPtr.lBlock!=CAT_FLAG_NONBLOCK || !bPtrStack.empty()) {
			while(bPtr.lBlock!=CAT_FLAG_NONBLOCK) {
				block = bufmgr->getblock(pager, bPtr.lBlock, BUFFER_FLAG_NONDIRTY);
				var_offset = bPtr.lOffset;
				left = cm_catNodeMgr.newEmptyNode();
				
				// nodeMap[left] = count++;

				node->leftSon = left;
				node = left;
				left->strval = new char[CAT_NAME_MAXSIZE];
				
				memcpy((char*)&left->operation, block+var_offset, sizeof(int));
				var_offset += sizeof(int);
				memcpy((left->strval), block+var_offset, CAT_NAME_MAXSIZE);
				var_offset += CAT_NAME_MAXSIZE;
				memcpy((char*)&left->numval, block+var_offset, sizeof(double));
				var_offset += sizeof(double);
				memcpy(&bPtr, block+var_offset, sizeof(BlockPtr));
				
				bPtrStack.push_back(NodePair(left, bPtr));
			}
			while(bPtr.rBlock == CAT_FLAG_NONBLOCK && !bPtrStack.empty()){
				// ????
				/*
				bPtr = bPtrStack.pop_back().second;
				node = bPtrStack.pop_back().first;
				*/
				bPtrStack.pop_back();
				bPtr = bPtrStack.back().second;
				node = bPtrStack.back().first;
			}
			if(bPtr.rBlock != CAT_FLAG_NONBLOCK) {
				bPtrStack.pop_back();
				block = bufmgr->getblock(pager, bPtr.rBlock, BUFFER_FLAG_NONDIRTY);
				var_offset = bPtr.rOffset;
				left = cm_catNodeMgr.newEmptyNode();

				// nodeMap[left] = count++;
				
				node->rightSon = left;
				node = left;
				left->strval = new char[CAT_NAME_MAXSIZE];
				
				memcpy((char*)&left->operation, block+var_offset, sizeof(int));
				var_offset += sizeof(int);
				memcpy((left->strval), block+var_offset, CAT_NAME_MAXSIZE);
				var_offset += CAT_NAME_MAXSIZE;
				memcpy((char*)&left->numval, block+var_offset, sizeof(double));
				var_offset += sizeof(double);
				memcpy(&bPtr, block+var_offset, sizeof(BlockPtr));
				
				bPtrStack.push_back(NodePair(left, bPtr));			
			}
		}
	}
}

CatalogManager::~CatalogManager()
{
	size_t const_offset = sizeof(int)*6 + sizeof(double);
	size_t var_offset=0;
	Node* node = cm_catRoot;
	int nodeCount=0;
	NodePosMap nodeMap;
	vector<Node*> tStack;

	while(node!=nullptr||!tStack.empty()) {
		if(node!=nullptr) {
			nodeMap[node] = nodeCount++;
			if(node->rightSon!=nullptr) {
				tStack.push_back(node->rightSon);
			}
			node = node->leftSon;
		}
		else {
			node = tStack.back();
			tStack.pop_back();
		}
	}

	BufferManager* bufmgr = BufferManager::getInstance();
	unsigned char* block;
	Pager* pager = bufmgr.getPager("minisql.frm");
	int blockNumber, blockOffset, nodePerBlock;
	BlockPtr bPtr;

	nodePerBlock = BLOCK_SIZE / const_offset;

	node = cm_catRoot;
	while( node!=nullptr || !tStack.empty() ) {
		if(node!=nullptr) {
			blockNumber = nodeMap[node] / nodePerBlock;
			blockOffset = (nodeMap[node]%nodePerBlock)*const_offset;
			while(blockNumber>=pager->getNPages()) {
				bufmgr.newblock(pager, BUFFER_FLAG_NONDIRTY);
			}
			block = bufmgr.getblock(pager, blockNumber, BUFFER_FLAG_DIRTY);
			if(node->leftSon) {
				bPtr.lBlock = nodeMap[node->leftSon] / nodePerBlock;
				bPtr.lOffset = (nodeMap[node->leftSon]%nodePerBlock)*const_offset;
			}
			else {
				bPtr.lBlock = bPtr.lOffset = -1;
			}
			if(node->rightSon) {
				bPtr.rBlock = nodeMap[node->rightSon] / nodePerBlock;
				bPtr.rOffset = (nodeMap[node->rightSon]%nodePerBlock)*const_offset;
			}
			else {
				bPtr.rBlock = bPtr.rOffset = -1;
			}
			var_offset = blockOffset;
			memcpy(block + var_offset, &node->operation, sizeof(int)); var_offset+=sizeof(int);
			memcpy(block + var_offset, node->strval, CAT_NAME_MAXSIZE); var_offset+=CAT_NAME_MAXSIZE;
			memcpy(block + var_offset, &node->numval, sizeof(double)); var_offset+=sizeof(double);
			memcpy(block + var_offset, &bPtr, sizeof(BlockPtr));

			if(node->rightSon!=nullptr) {
				tStack.push_back(node->rightSon);
			}
			node = node->leftSon;
		}
		else {
			node = tStack.back();
			tStack.pop_back();
		}
	}

	cm_catNodeMgr.clean();
}

int
CatalogManager::new_table_def(Node* node)
{
	Node *tablePtr = cm_catNodeMgr.newCopyDataNode(node);
	tablePtr->leftSon = cm_catRoot->leftSon;
	cm_catRoot->leftSon = tablePtr;
	++cm_catRoot->numval;

	Node *insertPtr = node->leftSon;
	Node *columnPtr = nullptr;
	int columnCount = 0;
	while (insertPtr != nullptr)
	{
		columnPtr = cm_catNodeMgr.newCopyDataNode(insertPtr);
		if (insertPtr->rightSon != nullptr)
		{
			columnPtr->rightSon = cm_catNodeMgr.newCopyDataNode(insertPtr->rightSon);
		}
		columnPtr->leftSon = tablePtr->rightSon;
		tablePtr->rightSon = columnPtr;
		insertPtr = insertPtr->leftSon;
		++columnCount;
	}

	// build file
	string tmpStr(node->strval);
	tmpStr += ".db";
	BufferManager* bufmgr = BufferManager::getInstance();
	bufmgr->createFile(tmpStr.c_str());
	Pager* page = bufmgr->getPager(tmpStr.c_str());
	unsigned char* block = bufmgr->newblock(page, BUFFER_FLAG_DIRTY);

	int* blockData 		= (int*)(block+16*sizeof(char));
	int* blockVarType 	= blockData + 5;
	int* blockVarLen 	= blockVarType + columnCount;

	strncpy((char*)block, "MINISQL v0.01", 16*sizeof(char));
	blockData[0] = 1;
	blockData[1] = 0;
	blockData[2] = 0;
	blockData[3] = sizeof(char)*16+sizeof(int)*(5+2*columnCount);
	blockData[4] = columnCount;

	insertPtr = node->leftSon;
	for (int i = columnCount-1; i >=0 ; --i)
	{
		blockVarType[i] = insertPtr->operation;
		blockVarLen[i]  = (insertPtr->operation == VAL_CHAR)? insertPtr->numval: 4;
		insertPtr = insertPtr->leftSon;
	}

	return 0;
}

int
CatalogManager::new_index_def(char* tableName, char* columnName, char* indexName)
{
	Node *ptr = cm_catRoot->leftSon;
	Node *tmpPtr = nullptr;

	string tmpStr(tableName);
	tmpStr += "_";
	tmpStr += columnName;
	tmpStr += ".idx";
	BufferManager* bufmgr = BufferManager::getInstance();
	bufmgr->createFile(tmpStr.c_str());
	Pager* page = bufmgr->getPager(tmpStr.c_str());
	unsigned char* block = bufmgr->newblock(page, BUFFER_FLAG_DIRTY);
	// TO-DO

	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval) == 0)
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) == 0)
				{
					ptr = ptr->rightSon;
					tmpPtr = cm_catNodeMgr.newEmptyNode();
					STRDUP_NEW(tmpPtr->strval, indexName);
					if (/*ptr!=nullptr && */ptr->rightSon != nullptr)
					{
						tmpPtr->rightSon = ptr->rightSon;
						ptr->rightSon = tmpPtr;
						return 1;
					}
					else
					{
						ptr->rightSon = tmpPtr;
						return 0;
					}
				}	
				ptr = ptr->leftSon;
			}
		}
		ptr = ptr->leftSon;
	}
	exit(-1);
}

int
CatalogManager::delete_table_def(char* tableName)
{
	string tmpStr(tableName);
	tmpStr+=".db";
	BufferManager::getInstance()->deleteFile(tmpStr.c_str());

	Node *ptr = cm_catRoot->leftSon;
	Node *tmpPtr = cm_catRoot;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval) == 0)
		{
			tmpPtr->leftSon = ptr->leftSon;
			return 0;
		}
		tmpPtr = ptr;
		ptr = ptr->leftSon;
	}
	exit(-1);
}

int
CatalogManager::delete_index_def(char* indexName)
{
	string tmpStr(indexName);
	tmpStr+=".idx";
	BufferManager::getInstance()->deleteFile(tmpStr.c_str());

	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	Node *indexPtr = nullptr;
	Node *tmpPtr = nullptr;
	while (tablePtr != nullptr)
	{
		columnPtr = tablePtr->rightSon;
		while (columnPtr != nullptr)
		{
			if (columnPtr->rightSon != nullptr)
			{
				indexPtr = columnPtr->rightSon->rightSon;
				tmpPtr = columnPtr->rightSon;
				while (indexPtr != nullptr)
				{
					if (strcmp(indexName, indexPtr->strval) == 0)
					{
						// first node: tp = primary key
						if (tmpPtr->strval == nullptr)
							tmpPtr->rightSon = indexPtr->leftSon;
						// else
						else
							tmpPtr->leftSon = indexPtr->leftSon;
					}
					tmpPtr = indexPtr;
					indexPtr = indexPtr->leftSon;
				}
			}
			columnPtr = columnPtr->leftSon;
		}
		tablePtr = tablePtr->leftSon;
	}
	exit(-1);
}

Node*
CatalogManager::get_column_def(char* tableName)
{
	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	Node *returnRoot = nullptr;
	Node *returnPtr = nullptr;

	while (tablePtr != nullptr)
	{
		if (strcmp(tableName, tablePtr->strval) == 0)
		{
			columnPtr = tablePtr->rightSon;
			while (columnPtr != nullptr)
			{
				// Ignore Primary key...
				returnPtr = m_columndef.newCopyDataNode(columnPtr);
				returnPtr->leftSon = returnRoot;
				returnRoot = returnPtr;

				columnPtr = columnPtr->leftSon;
			}
			return returnRoot;
		}
		tablePtr = tablePtr->leftSon;
	}
	exit(-1);
}

Node*
CatalogManager::get_column_def(char* tableName, char* columnName)
{
	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	while (tablePtr != nullptr)
	{
		if (strcmp(tableName, tablePtr->strval) == 0)
		{
			columnPtr = tablePtr->rightSon;
			while (columnPtr != nullptr)
			{
				if (strcmp(columnName, columnPtr->strval) == 0)
				{
					return m_columndef.newCopyDataNode(columnPtr);
				}
				columnPtr = columnPtr->leftSon;
			}
		}
		tablePtr = tablePtr->leftSon;
	}
	exit(-1);
}
int   
CatalogManager::get_column_id(char* tableName, char* columnName)
{
	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	int count = 0;
	while (tablePtr != nullptr)
	{
		if (strcmp(tableName, tablePtr->strval) == 0)
		{
			columnPtr = tablePtr->rightSon;
			while (columnPtr != nullptr)
			{
				if (strcmp(columnName, columnPtr->strval) == 0)
				{
					return count;
				}
				++count;
				columnPtr = columnPtr->leftSon;
			}
		}
		tablePtr = tablePtr->leftSon;
	}
	exit(-1);
}


// done
bool
CatalogManager::ifexist_table(char* tableName)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval) == 0)
			return true;
		ptr = ptr->leftSon;
	}
	return false;
}

bool
CatalogManager::ifexist_index(char* indexName)
{
	Node *tablePtr = cm_catRoot->leftSon;
	Node *columnPtr = nullptr;
	Node *indexPtr = nullptr;
	while (tablePtr != nullptr)
	{
		columnPtr = tablePtr->rightSon;
		while (columnPtr != nullptr)
		{
			if (columnPtr->rightSon != nullptr)
			{
				indexPtr = columnPtr->rightSon->rightSon;
				while (indexPtr != nullptr)
				{
					if (strcmp(indexName, indexPtr->strval) == 0)
					{
						return true;
					}
					indexPtr = indexPtr->leftSon;
				}
			}
			columnPtr = columnPtr->leftSon;
		}
		tablePtr = tablePtr->leftSon;
	}
	return false;
}

bool
CatalogManager::if_unique_key(char* tableName, char* columnName)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) == 0 && ptr->rightSon!=nullptr &&\
					 ptr->rightSon->operation >= DEF_UNIQUE)
					return true;
				ptr = ptr->leftSon;
			}
			return false;
		}
		ptr = ptr->leftSon;
	}
	return false;
}

bool
CatalogManager::ifexist_index_on_column(char* tableName, char* columnName)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval))
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) == 0 && ptr->rightSon!=nullptr&&\
					 ptr->rightSon->rightSon != nullptr)
					return true;
				ptr = ptr->leftSon;
			}
			return false;
		}
		ptr = ptr->leftSon;
	}
	return false;
}

void
CatalogManager::assertExistTable(char* tableName)
throw(TableExistException)
{
	if (ifexist_table(tableName))
		throw TableExistException();
}

void 
CatalogManager::assertExistIndex(char* indexName) 
throw(IndexExistException)
{
	if (ifexist_index(indexName))
		throw IndexExistException();
}

void
CatalogManager::assertNonExistTable(char* tableName) 
throw(TableNonExistException)
{
	if (!ifexist_table(tableName))
		throw TableExistException();
}
	
void
CatalogManager::assertNonExistIndex(char* indexName) 
throw(IndexNonExistException)
{
	if (!ifexist_index(indexName))
		throw IndexNonExistException();
}

void
CatalogManager::assertNonExistColumn(char* tableName, char* columnName) 
throw(ColumnNonExistException)
{
	Node *ptr = cm_catRoot->leftSon;
	while (ptr != nullptr)
	{
		if (strcmp(tableName, ptr->strval) == 0)
		{
			ptr = ptr->rightSon;
			while (ptr != nullptr)
			{
				if (strcmp(columnName, ptr->strval) == 0)
					return;
				ptr = ptr->leftSon;
			}
			throw ColumnNonExistException();
		}
		ptr = ptr->leftSon;
	}
	throw ColumnNonExistException();
}

void
CatalogManager::assertNotUniqueKey(char* tableName, char* columnName) 
throw(NotUniqueKeyException)
{
	if (!if_unique_key(tableName, columnName))
		throw NotUniqueKeyException();
}

CatalogManager*
CatalogManager::getInstance()
{
	return cm_delegate;
}
