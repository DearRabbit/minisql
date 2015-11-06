#include "IndexManager.h"

IndexManager* IndexManager::im_delegate = NULL;

IndexManager::IndexManager()
{
	im_delegate = this;

}

IndexManager::~IndexManager()
{

}

static
IndexManager*
IndexManager::getInstance()
{
	return im_delegate();
}

int 
IndexManager::new_index(Node* node, vector<CursePair>& cursorTable)
{

}

int
IndexManager::new_entry_idx(Node* node, CursePair& cursor)
{

}

int
IndexManager::delete_entry_idx(Node* node, CursePair& cursor)
{

}

int 
IndexManager::select_index(Node* node, vector<CursePair>& cursorTable)
{

}

void
IndexManager::assertMultipleKey(char* tableName, char* columnName, Node* data)
{

}