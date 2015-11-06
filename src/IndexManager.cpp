#include "IndexManager.h"
#include "bptree.h"

IndexManager* IndexManager::im_delegate = NULL;

IndexManager::IndexManager()
{
	im_delegate = this;
}

IndexManager::~IndexManager()
{

}

IndexManager*
IndexManager::getInstance()
{
	return im_delegate;
}

int 
IndexManager::new_index(char* tableName, char* columnName, Node* data, vector<CursePair>& cursor)
{
	char* fileName;
	Node* node=data;
	vector<CursePair>::iterator it = cursor.begin();

	fileName = catIdxName(tableName, columnName);
	// File must exist
	// Temporarily this, maybe use construct() later.
	if(data->operation == VAL_INT) {
        int tInt;
		BPT bpt(fileName);
		while(node != nullptr) {
            tInt = (int)(node->numval);
			// TODO:How is an int stored in numval?
			bpt.insertEntry(&tInt, it->first, it->second);
			node = node->rightSon;
			it++;
		}
	}
	else if(data->operation == VAL_FLOAT) {
        float tFloat;
		BPT bpt(fileName);
		while(node != nullptr) {
            tFloat = (float)(node->numval);
			bpt.insertEntry(&tFloat, it->first, it->second);
			node = node->rightSon;
			it++;
		}
	}
	else if(data->operation == VAL_CHAR){ 
		BPT bpt(fileName);
		while(node != nullptr) {
			bpt.insertEntry(node->strval, it->first, it->second);
			node = node->rightSon;
			it++;
		}
	}
	delete[] fileName;
	return (int)cursor.size();
}

int
IndexManager::new_entry_idx(char* tableName, char* columnName, Node* data, vector<CursePair>& cursor)
{
	char* fileName;
	Node* node = data;
	vector<CursePair>::iterator it = cursor.begin();

	fileName = catIdxName(tableName, columnName);
	if(data->operation == VAL_INT) {
        int tInt;
		BPT bpt(fileName);
		while(node != nullptr) {
            tInt = node->numval;
			// TODO:How is an int stored in numval?
			bpt.insertEntry(&tInt, it->first, it->second);
			node = node->rightSon;
			it++;
		}
	}
	else if(data->operation == VAL_FLOAT) {
        float tFloat;
		BPT bpt(fileName);
		while(node != nullptr) {
            tFloat = node->numval;
			bpt.insertEntry(&tFloat, it->first, it->second);
			node = node->rightSon;
			it++;
		}
	}
	else if(data->operation == VAL_CHAR){ 
		BPT bpt(fileName);
		while(node != nullptr) {
			bpt.insertEntry(node->strval, it->first, it->second);
			node = node->rightSon;
			it++;
		}
	}
	delete[] fileName;
	return (int)cursor.size();
}

int
IndexManager::delete_entry_idx(char* tableName, char* columnName, Node* data)
{
	char* fileName;
	Node* node = data;

	fileName = catIdxName(tableName, columnName);
	if(data->operation == VAL_INT) {
        int tInt;
		BPT bpt(fileName);
		while(node != nullptr) {
			// TODO:How is an int stored in numval?
            tInt = node->numval;
			bpt.deleteEntry(&tInt);
			node = node->rightSon;
		}
	}
	else if(data->operation == VAL_FLOAT) {
        float tFloat;
		BPT bpt(fileName);
		while(node != nullptr) {
            tFloat = node->numval;
			bpt.deleteEntry(&tFloat);
			node = node->rightSon;
		}
	}
	else if(data->operation == VAL_CHAR){ 
		BPT bpt(fileName);
		while(node != nullptr) {
			bpt.deleteEntry(node->strval);
			node = node->rightSon;
		}
	}
	delete[] fileName;

    return 1;
}

int 
IndexManager::select_record_raw(char* tableName, char* columnName, Node* expr, vector<CursePair>& cursor)
{
	char* fileName;
	IDXFileHeader* fileHeader;

	fileName = catIdxName(tableName, columnName);
	fileHeader = newIdxHeader(fileName);
	if(fileHeader->Type == IDX_TYPE_INT) {
		BPT bpt(fileName);
		bpt.select(expr, cursor);
	}
	else if(fileHeader->Type == IDX_TYPE_FLOAT) {
		BPT bpt(fileName);
		bpt.select(expr, cursor);
	}
	else if(fileHeader->Type == IDX_TYPE_STRING) {
		BPT bpt(fileName);
		bpt.select(expr, cursor);
	}

	delete[] fileName;
	delete fileHeader;
	return (int)cursor.size();
}

int 
IndexManager::select_record(char* tableName, char* columnName, Node* expr, vector<CursePair>& cursor)
{
	char* fileName;
	IDXFileHeader* fileHeader;
	vector<CursePair> localCusor;
	auto opair = cursor.begin();
	auto ipair = localCusor.begin();

	fileName = catIdxName(tableName, columnName);
	fileHeader = newIdxHeader(fileName);
	if(fileHeader->Type == IDX_TYPE_INT) {
		BPT bpt(fileName);
		bpt.select(expr, localCusor);
	}
	else if(fileHeader->Type == IDX_TYPE_FLOAT) {
		BPT bpt(fileName);
		bpt.select(expr, localCusor);
	}
	else if(fileHeader->Type == IDX_TYPE_STRING) {
		BPT bpt(fileName);
		bpt.select(expr, localCusor);
	}
	for(opair = cursor.begin(); opair!=cursor.end(); opair++) {
		for(ipair = localCusor.begin(); ipair!=localCusor.end(); ipair++){
			if(ipair->first == opair->first && ipair->second == opair->second) {
				continue;
			}
			else {
				cursor.erase(opair);
				localCusor.erase(ipair);
			}
		}
	}

	delete[] fileName;
	delete fileHeader;
	return (int)cursor.size();
}

void
IndexManager::assertMultipleKey(char* tableName, char* columnName, Node* data)
throw (MultipleKeyException)
{
	char* fileName;
	int rst;

	fileName = catIdxName(tableName, columnName);
	if(data->operation == VAL_INT) {
        int tInt = data->numval;
		BPT bpt(fileName);
		rst = bpt.ifexist(&tInt);
	}
	else if(data->operation == VAL_FLOAT) {
        float tFloat = data->numval;
		BPT bpt(fileName);
		rst = bpt.ifexist(&tFloat);
	}
	else if(data->operation == VAL_CHAR) {
		BPT bpt(fileName);
		rst = bpt.ifexist(data->strval);
	}

	delete[] fileName;
	if(rst==1)
		throw MultipleKeyException(tableName, columnName);
}

inline
char*
IndexManager::catIdxName(char* tableName, char* columnName)
{
	char* idxName = new char[strlen(tableName)+strlen(columnName)+2];
	strcpy(idxName, tableName);
	strcat(idxName, "_");
	strcat(idxName, columnName);
	strcat(idxName, ".idx");
	return idxName;
}

IDXFileHeader*
IndexManager::newIdxHeader(char* fileName)
{
	Pager* pager = BufferManager::getInstance()->getPager(fileName);
	unsigned char* block = BufferManager::getInstance()->getblock(pager, 0, BUFFER_FLAG_NONDIRTY);
	IDXFileHeader* idxHeader = new IDXFileHeader;
	memcpy(idxHeader, block, IDX_FILEHEADER_SIZE);
	return idxHeader;
}