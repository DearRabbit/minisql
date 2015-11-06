#include "RecordManager.h"

RecordManager::RecordManager()
{
	m_bufInstance = BufferManager::getInstance();
	m_currentPage = nullptr;
	m_header.tableName = nullptr;
	m_header.columnName = nullptr;
}

RecordManager::~RecordManager()
{
}

RecordManager* RecordManager::getInstance()
{
	return rm_delegate;
}

// private methods
void RecordManager::load_file(string& filename)
{
	// m_bufInstance = BufferManager::getInstance();
	m_currentPage = m_bufInstance->getPager(filename.c_str());
	unsigned char* block0 = m_bufInstance->getblock(m_currentPage, 0, BUFFER_FLAG_NONDIRTY);
	// may cause bug but it's okay
	size_t const_offset = sizeof(unsigned char)*16+sizeof(int)*5;
	memcpy(&m_header, block0, const_offset);

	size_t var_offset = m_header.columnCount*sizeof(int);
	m_header.columnType = new int[m_header.columnCount];
	memcpy(m_header.columnType, block0+const_offset, var_offset);
	m_header.columnLength = new int[m_header.columnCount];
	memcpy(m_header.columnLength, block0+const_offset+var_offset, var_offset);

	m_header.headerLength = const_offset + var_offset*2;
	m_header.valLength = 0;
	for (int i = m_header.columnCount-1; i >=0; --i)
	{
		m_header.valLength += m_header.columnLength[i];
	}
}
void RecordManager::write_back()
{
	unsigned char* block0 = m_bufInstance->getblock(m_currentPage, 0, BUFFER_FLAG_NONDIRTY);
	memcpy(block0, &m_header, sizeof(unsigned char)*16+sizeof(int)*5);
	delete [] m_header.columnType;
	delete [] m_header.columnLength;
}

//public
CursePair RecordManager::new_entry_record(Node* root)
{
	string filename(root->strval);
	filename+=".db";

	load_file(filename);
	unsigned char* blockEmpty;

	if (m_header.nextEmptyNo >= m_header.blockCount)
	{
		blockEmpty = m_bufInstance->newblock(m_currentPage, BUFFER_FLAG_DIRTY);
		m_header.nextEmptyNo = (++m_header.blockCount);
		m_header.nextEmptyOffset = 0;
	}
	else
	{
		blockEmpty = m_bufInstance->getblock(m_currentPage, m_header.nextEmptyNo, BUFFER_FLAG_DIRTY);
	}

	int blockNo = m_header.nextEmptyNo;
	int blockOffset = m_header.nextEmptyOffset;

	unsigned char* ptrBuf = blockEmpty+blockOffset+m_header.valLength;
	Node* ptrData = root->leftSon;
	int blockFlag = *(int*)ptrBuf;

	assert(blockFlag<0);
	if (blockFlag == 0)
	{
		// initially empty

		// need new block
		if (m_header.nextEmptyOffset+blockOffset+m_header.valLength+sizeof(int)\
			> BLOCK_SIZE)
		{
			++m_header.nextEmptyNo;
			m_header.nextEmptyOffset = 0;
		}
		else
		{
			m_header.nextEmptyOffset += m_header.valLength+sizeof(int);
		}
	}
	else //if (blockFlag > 0)
	{
		// deleted
		m_header.nextEmptyOffset = blockFlag&BLOCK_SIZE_MASK;
		m_header.nextEmptyNo = (blockFlag>>BLOCK_NUMBER_OFFSET)&BLOCK_NUMBER_MASK;
	}
	// make flag empty?
	*(unsigned int*)ptrBuf = 0x80000000;
	++m_header.enteyCount;

	for (int i = m_header.columnCount - 1; i >= 0; --i)
	{
		size_t len = m_header.columnLength[i];
		ptrBuf -= len;
		// Warning! maybe some bugs
		if (m_header.columnType[i] == VAL_CHAR)
			strncpy((char*)ptrBuf, ptrData->strval,len);
		else if (m_header.columnType[i] == VAL_FLOAT)
		{
			*(ptrBuf) = (float)ptrData->numval;
		}
		else //(m_header.columnType == VAL_INT)
		{
			*(ptrBuf) = (int)ptrData->numval;
		}
		ptrData = ptrData->leftSon;
	}
		
	write_back();
	return std::make_pair(blockNo, blockOffset);
}

int RecordManager::delete_record(char* tableName, vector<CursePair>& curseTable)
{
	string filename(tableName);
	filename+=".db";

	load_file(filename);

	m_header.enteyCount -= curseTable.size();
	unsigned int blockFlag = (m_header.nextEmptyNo<<BLOCK_NUMBER_OFFSET)\
		|m_header.nextEmptyOffset;
	for (auto it:curseTable)
	{
		unsigned char* block=m_bufInstance->getblock(m_currentPage, it.first, 1);
		*(unsigned int*)(block+it.second+m_header.valLength) = blockFlag;
		blockFlag = (it.first<<BLOCK_NUMBER_OFFSET)|(it.second);
	}
	m_header.nextEmptyOffset = blockFlag&BLOCK_SIZE_MASK;
	m_header.nextEmptyNo = (blockFlag>>BLOCK_NUMBER_OFFSET)&BLOCK_NUMBER_MASK;

	write_back();
	return curseTable.size();
}

int RecordManager::delete_all_record(char* tableName)
{
	// unlike delete select,
	// it just clear all
	string filename(tableName);
	filename+=".db";

	load_file(filename);
	unsigned char* block = nullptr;

	for (int i = m_header.blockCount-1; i > 0; --i)
	{
		block = m_bufInstance->getblock(m_currentPage, i, BUFFER_FLAG_DIRTY);
		memset(block, 0, sizeof(char)*BLOCK_SIZE);
	}
	block = m_bufInstance->getblock(m_currentPage, 0, BUFFER_FLAG_DIRTY);
	memset(block+m_header.headerLength, 0, sizeof(char)*(BLOCK_SIZE - m_header.headerLength));
	
	// TO-DO, rewrite header
	int returnVal = m_header.enteyCount;
	m_header.blockCount = 1;
	m_header.enteyCount = 0;
	m_header.nextEmptyNo = 0;
	m_header.nextEmptyOffset = m_header.headerLength;

	write_back();
	return returnVal;
}

int select_record_raw(char* tableName, Node* node, vector<CursePair>& curseTable)
{

}

int select_record(char* tableName, Node* node, vector<CursePair>& curseTable)
{
	
}





