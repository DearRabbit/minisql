#include "CatalogManager.h"
#include "BufferManager.h"
#include <unistd.h>

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
		bufmgr.createFile("minisql.frm");
		cm_catRoot = cm_catNodeMgr.newEmptyNode();
		cm_catNodeMgr.setRootNode(cm_catRoot);
	}
	else {
		cm_catRoot = cm_catNodeMgr.newEmptyNode();
		cm_catNodeMgr.setRootNode(cm_catRoot);
		pager = bufmgr.getPager("minisql.frm");

		size_t const_offset = sizeof(int)*6 + sizeof(double);
		size_t var_offset=0;
		unsigned char* block0 = bufmgr.getblock(pager, 0, BUFFER_FLAG_NONDIRTY);
		
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

		while(bPtr.lBlock!=CAT_FLAG_NONBLOCK || !bPtrStack.empty()) {
			while(bPtr.lBlock!=CAT_FLAG_NONBLOCK) {
				block = bufmgr.getblock(pager, bPtr.lBlock, BUFFER_FLAG_NONDIRTY);
				var_offset = bPtr.lOffset;
				left = cm_catNodeMgr.newEmptyNode();
				node->leftSon = left;
				node = left;
				left->strval = new char[CAT_NAME_MAXSIZE];
				memcpy((char*)&left->operation, block+var_offset, sizeof(int));
				memcpy((char*)&left->operation, block+var_offset, sizeof(int));
				var_offset += sizeof(int);
				memcpy((left->strval), block+var_offset, CAT_NAME_MAXSIZE);
				var_offset += CAT_NAME_MAXSIZE;
				memcpy((char*)&left->numval, block+var_offset, sizeof(double));
				var_offset += sizeof(double);
				memcpy(&bPtr, block+var_offset, sizeof(BlockPtr));
				bPtrStack.push_back(NodePair(left, bPtr);
			}
			while(bPtr.rBlock==CAT_FLAG_NONBLOCK&&!bPtrStack.empty()){
				bPtr = bPtrStack.pop_back().second;
				node = bPtrStack.pop_back().first;
			}
			block = bufmgr.getblock(pager, bPtr.rBlock, BUFFER_FLAG_NONDIRTY);
			var_offset = bPtr.rOffset;
			left = cm_catNodeMgr.newEmptyNode();
			node->rightSon = left;
			node = left;
			left->strval = new char[CAT_NAME_MAXSIZE];
			memcpy((char*)&left->operation, block+var_offset, sizeof(int));
			memcpy((char*)&left->operation, block+var_offset, sizeof(int));
			var_offset += sizeof(int);
			memcpy((left->strval), block+var_offset, CAT_NAME_MAXSIZE);
			var_offset += CAT_NAME_MAXSIZE;
			memcpy((char*)&left->numval, block+var_offset, sizeof(double));
			var_offset += sizeof(double);
			memcpy(&bPtr, block+var_offset, sizeof(BlockPtr));
			bPtrStack.push_back(NodePair(left, bPtr);			
		}
	}
}

CatalogManager::~CatalogManager()
{
	Node* node = cm_catRoot;
	vector<Node*> tStack;

	while(node!=nullptr || !tStack.empty()) {
		
	}
}

int
CatalogManager::new_table_def(Node* node)
{

}

int
CatalogManager::new_index_def(char* tableName, char* columnName, char* indexName)
{

}

int
CatalogManager::delete_table_def(char* tableName)
{

}

int
CatalogManager::delete_index_def(char* indexName)
{

}

Node*
CatalogManager::get_column_def(char* tableName)
{

}

Node*
CatalogManager::get_column_def(char* tableName, char* columnName)
{

}

bool
CatalogManager::ifexist_table(char* tableName)
{

}

bool
CatalogManager::ifexist_index(char* indexName)
{

}

bool
CatalogManager::if_unique_key(char* tableName, char* columnName)
{

}

bool
CatalogManager::ifexist_index_on_column(char* tableName, char* columnName)
{

}

void
CatalogManager::assertExistTable(char* tableName)
throw(TableExistException)
{

}

void 
CatalogManager::assertExistIndex(char* indexName) 
throw(IndexExistException)
{

}

void
CatalogManager::assertNonExistTable(char* tableName) 
throw(TableNonExistException)
{

}
	
void
CatalogManager::assertNonExistIndex(char* indexName) 
throw(IndexNonExistException)
{

}

void
CatalogManager::assertNonExistColumn(char* tableName, char* columnName) 
throw(ColumnNonExistException)
{

}

void
CatalogManager::assertNotUniqueKey(char* tableName, char* columnName) 
throw(NotUniqueKeyException)
{

}

CatalogManager*
CatalogManager::getInstance()
{
	return cm_delegate;
}
