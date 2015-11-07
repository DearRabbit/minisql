#include "RecordManager.h"

RecordManager* RecordManager::rm_delegate=nullptr;

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
	for (int i = m_header.columnCount - 1; i >=0; --i)
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
	delete [] m_header.columnName;
}
void RecordManager::assignColumnName(Node* data)
{
	m_header.columnName = new char*[m_header.columnCount];
	for (int i = m_header.columnCount - 1; i >=0 ;--i)
	{
		m_header.columnName[i] = data->strval;
		data = data->leftSon;
	}
}
void RecordManager::printBorder(int* tableLen, int len)
{
	for (int i = 0; i < len; ++i)
	{
		putchar('+');
		for (int j = 0; j < tableLen[i]; ++j)
		{
			putchar('-');
		}
	}
	putchar('+');
	putchar('\n');
}
bool RecordManager::cmpExpr(Node* expr, unsigned char *data, int columnId)
{
	bool result = false;
	const float floate = 0.000001;
	int strres = strncmp((char*)data, expr->rightSon->strval, m_header.columnLength[columnId]);
	float fres = *(float*)data - expr->rightSon->numval;
	int intres = *(int*)data - (int)expr->rightSon->numval;

	switch (expr->operation)
	{
		case CMP_EQ:
			{
				// char
				if (m_header.columnType[columnId] == VAL_CHAR)
					result = (strres == 0);
				// float
				else if (m_header.columnType[columnId] == VAL_FLOAT)
					result = (fabs(fres)<floate);
				// int
				else
					result = (intres == 0);
			}
			break;
		case CMP_NEQ:
			{
				if (m_header.columnType[columnId] == VAL_CHAR)
					result = (strres != 0);
				else if (m_header.columnType[columnId] == VAL_FLOAT)
					result = (fabs(fres)>floate);
				else
					result = (intres != 0);
			}
			break;
		case CMP_LT:
			{
				if (m_header.columnType[columnId] == VAL_CHAR)
					result = (strres < 0);
				else if (m_header.columnType[columnId] == VAL_FLOAT)
					result = (fres < floate);
				else
					result = (intres < 0);
			}
			break;
		case CMP_GT:
			{
				if (m_header.columnType[columnId] == VAL_CHAR)
					result = (strres > 0);
				else if (m_header.columnType[columnId] == VAL_FLOAT)
					result = (fres > floate);
				else
					result = (intres > 0);
			}
			break;
		case CMP_LE:
			{
				if (m_header.columnType[columnId] == VAL_CHAR)
					result = (strres <= 0);
				else if (m_header.columnType[columnId] == VAL_FLOAT)
					result = !(fres > floate);
				else
					result = (intres <= 0);
			}
			break;
		case CMP_GE:
			{
				if (m_header.columnType[columnId] == VAL_CHAR)
					result = (strres >= 0);
				else if (m_header.columnType[columnId] == VAL_FLOAT)
					result = !(fres < floate);
				else
					result = (intres >= 0);
			}
			break;
		default: assert(1);
	}
	return result;
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

	assert(blockFlag>=0);
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
	++m_header.entryCount;

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

	m_header.entryCount -= curseTable.size();
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
	int returnVal = m_header.entryCount;
	m_header.blockCount = 1;
	m_header.entryCount = 0;
	m_header.nextEmptyNo = 0;
	m_header.nextEmptyOffset = m_header.headerLength;

	write_back();
	return returnVal;
}

int RecordManager::select_record_raw(char* tableName, Node* def, Node* expr, vector<CursePair>& curseTable)
{
	string filename(tableName);
	filename+=".db";

	load_file(filename);
	assignColumnName(def);

	// from flag back to column
	int initial_offset = 0;
	int columnId = 0;
	
	for (int i = m_header.columnCount - 1; i >=0 ;--i)
	{
		initial_offset += m_header.columnLength[i];
		if (strcmp(m_header.columnName[i], expr->leftSon->strval) == 0)
		{
			columnId = i;
			// always break!
			break;
		}	
	}

	// all
	int blockNo = 0;
	unsigned char* block = m_bufInstance->getblock(m_currentPage, blockNo, BUFFER_FLAG_NONDIRTY);
	unsigned char* blockPtr = block+m_header.headerLength+m_header.valLength;
	for (int i = m_header.entryCount; i>0;)
	{
		if (*(int*)blockPtr < 0)
		{
			--i;
			if (cmpExpr(expr, blockPtr - initial_offset, columnId))
				curseTable.push_back(std::make_pair(blockNo, blockPtr-block-m_header.valLength));
		}
		blockPtr += (m_header.valLength+sizeof(int));
		if ((blockPtr+sizeof(int)) > (block+BLOCK_SIZE))
		{
			block = m_bufInstance->getblock(m_currentPage, ++blockNo, BUFFER_FLAG_NONDIRTY);
			blockPtr = block+m_header.valLength;
		}
	}

	write_back();
	return 0;
}

int RecordManager::select_record(char* tableName, Node* def, Node* expr, vector<CursePair>& curseTable)
{
	string filename(tableName);
	filename+=".db";

	load_file(filename);
	assignColumnName(def);
	unsigned char* block = nullptr;

	// from start to column
	int initial_offset = m_header.valLength;
	int columnId = 0;
	for (int i = m_header.columnCount - 1; i >=0 ;--i)
	{
		initial_offset -= m_header.columnLength[i];
		if (strcmp(m_header.columnName[i], expr->leftSon->strval) == 0)
		{
			columnId = i;
			// always break!
			break;
		}	
	}
	for (auto it=curseTable.begin(); it!=curseTable.end() ;it++)
	{
		block = m_bufInstance->getblock(m_currentPage, it->first, BUFFER_FLAG_NONDIRTY)\
			+ it->second + initial_offset;
		if (!cmpExpr(expr, block, columnId))
			curseTable.erase(it);
	}
	write_back();
	return 0;
}

Node* RecordManager::get_column_data(char* tableName, int columnId, vector<CursePair>& curseTable)
{
	string filename(tableName);
	filename+=".db";

	load_file(filename);
	if (m_header.entryCount == 0)
	{
		write_back();
		return nullptr;
	}
	Node *root = m_columndata.newEmptyNode();
	root->operation = m_header.columnType[columnId];
	Node *ptr = root;
	Node *ptrNext = nullptr;

	// from flag back to column
	int initial_offset = 0;
	for (int i = m_header.columnCount - 1; i >= columnId; --i)
	{
		initial_offset += m_header.columnLength[i];	
	}

	// all
	int blockNo = 0;
	unsigned char* block = m_bufInstance->getblock(m_currentPage, blockNo, BUFFER_FLAG_NONDIRTY);
	unsigned char* blockPtr = block+m_header.headerLength+m_header.valLength;
	for (int i = m_header.entryCount; i>0;)
	{
		if (*(int*)blockPtr < 0)
		{
			--i;
			ptrNext = m_columndata.newEmptyNode();
			if (root->operation == VAL_CHAR)
			{
				ptrNext->strval = new char[m_header.columnLength[columnId]+1];
				strncpy(ptrNext->strval, (char*)(blockPtr - initial_offset), m_header.columnLength[columnId]);
				(ptrNext->strval)[m_header.columnLength[columnId]]=0;
			}
			else if (root->operation == VAL_FLOAT)
			{
				ptrNext->numval = *(float*)(blockPtr - initial_offset);
			}
			else
			{
				ptrNext->numval = *(int*)(blockPtr - initial_offset);
			}
			ptr->leftSon = ptrNext;
			ptr = ptrNext;

			curseTable.push_back(std::make_pair(blockNo, blockPtr-block-m_header.valLength));
		}
		blockPtr += (m_header.valLength+sizeof(int));
		if ((blockPtr+sizeof(int)) > (block+BLOCK_SIZE))
		{
			block = m_bufInstance->getblock(m_currentPage, ++blockNo, BUFFER_FLAG_NONDIRTY);
			blockPtr = block+m_header.valLength;
		}
	}

	write_back();
	return root;
}

int RecordManager::print_select_record(char* tableName, Node* def, vector<CursePair>& curseTable)
{
	string filename(tableName);
	filename+=".db";

	load_file(filename);
	assignColumnName(def);
	// print title
	int* tableLen = new int[m_header.columnCount];
	for (int i = 0; i < m_header.columnCount; ++i)
	{
		int maxlen = strlen(m_header.columnName[i]);
		if (m_header.columnType[i] == VAL_INT)
			maxlen = (maxlen > 10) ? maxlen : 10;
		else if (m_header.columnType[i] == VAL_FLOAT)
			maxlen = (maxlen > 8) ? maxlen : 8;
		else
			maxlen = (maxlen > m_header.columnLength[i]) ? maxlen : m_header.columnLength[i];

		tableLen[i] = maxlen; 
	}

	// TO-DO
	// print title
	printBorder(tableLen, m_header.columnCount);
	// print column name
	printBorder(tableLen, m_header.columnCount);

	// print data
	printBorder(tableLen, m_header.columnCount);

	delete [] tableLen;
	write_back();
	return 0;
}

int RecordManager::print_all_record(char* tableName, Node* def)
{
	string filename(tableName);
	filename+=".db";

	load_file(filename);
	assignColumnName(def);

	int* tableLen = new int[m_header.columnCount];
	for (int i = m_header.columnCount-1; i >=0 ; --i)
	{
		int maxlen = strlen(m_header.columnName[i]);
		if (m_header.columnType[i] == VAL_INT)
			maxlen = (maxlen > 10) ? maxlen : 10;
		else if (m_header.columnType[i] == VAL_FLOAT)
			maxlen = (maxlen > 8) ? maxlen : 8;
		else
			maxlen = (maxlen > m_header.columnLength[i]) ? maxlen : m_header.columnLength[i];

		tableLen[i] = maxlen; 
	}

	// print title
	printBorder(tableLen, m_header.columnCount);
	// print column name
	for (int i = 0; i < m_header.columnCount; ++i)
	{
		putchar('|');
		cout.width(tableLen[i]);
		cout <<m_header.columnName[i];
	}
	putchar('|');
	putchar('\n');
	printBorder(tableLen, m_header.columnCount);

	// print data
	// TO-DO test
	int blockNo = 0;
	unsigned char* blockHead = m_bufInstance->getblock(m_currentPage, 0, BUFFER_FLAG_NONDIRTY);
	unsigned char* blockPtr = blockHead + m_header.headerLength;
	unsigned char* blockFlagPtr = blockPtr + m_header.valLength;
	for (int j = m_header.entryCount; j > 0;)
	{
		if (*(int*)blockFlagPtr < 0)
		{
			for (int i = 0; i < m_header.columnCount; ++i)
			{
				putchar('|');
				cout.width(tableLen[i]);
				if (m_header.columnType[i] == VAL_CHAR)
				{
					cout <<blockPtr;
				}
				else if (m_header.columnType[i] == VAL_FLOAT)
				{
					cout <<*(float*)blockPtr;
				}
				else
				{
					cout <<*(int*)blockPtr;
				}
				blockPtr += m_header.columnLength[i];
			}
			putchar('|');
			putchar('\n');
			--j;
		}
		blockPtr += (m_header.valLength+sizeof(int));
		if ((blockPtr) > (blockHead+BLOCK_SIZE))
		{
			blockHead = m_bufInstance->getblock(m_currentPage, ++blockNo, BUFFER_FLAG_NONDIRTY);
			blockPtr = blockHead;
			blockFlagPtr = blockHead + m_header.valLength;
		}
	}
	printBorder(tableLen, m_header.columnCount);

	delete [] tableLen;
	write_back();
	return 0;
}

void RecordManager::assertMultipleKey(char* tableName, char* columnName, Node* data)
{
	// empty
}
void RecordManager::clean()
{
	m_columndata.clean();
}





