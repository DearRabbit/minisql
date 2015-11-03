#pragma once

#define __LRU__CLOCK__

#include <map>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include "macros.h"

#define _DEBUG 0

const int BLOCK_SIZE   = 4096;
const int BLOCK_NUMBER = 512;
//const int MAX_FILE_NUMBER = 4;

class Pager;

class blockInfo
{	// never copied, only in a pool
private:
	unsigned char b_buf[BLOCK_SIZE];
	int b_dirty;
	int b_pin;
	int b_block;
	const Pager* b_pager;
public:
	blockInfo() {
		memset(b_buf, 0, BLOCK_SIZE);
		b_dirty = 0;
		b_pager = NULL;
		b_pin = 0;
		b_block = 0;
	}
	void setDirty(int sdirty=1) {
		b_dirty = sdirty;
	}
	int isDirty() const {
		return b_dirty;
	}
	void setblock(const Pager* pager, const int sblock_num ) {
		b_block = sblock_num;
		b_dirty = 0;
		b_pin   = 0;
		b_pager = pager;
		memset(b_buf, 0, BLOCK_SIZE);
	}
	unsigned char* getblock() {
		return b_buf;
	}
	int isPinned() const {
		return b_pin;
	}
	int getBlockNum() const {
		return b_block;
	}
	int isSet() const {
		return b_pager != NULL;
	}
	const Pager* getPager() const {
		return b_pager;
	}
	void setPin(int tpin=1) {
		b_pin = tpin;
	}
	void clear() {
		memset(b_buf, 0, BLOCK_SIZE);
		b_dirty = 0;
		b_pin = 0;
		b_pager = NULL;
	}
};

class Pager
{
	char* filename;
	FILE* fileptr;
	unsigned int n_pages;
public:
	Pager(const char* fname) {
		filename = new char[strlen(fname) + 1];     // holy shit!
		strcpy(filename, fname);
		fileptr = fopen(filename, "rb+");
		struct stat st;
		stat(filename, &st);
		n_pages = st.st_size / BLOCK_SIZE;
	}
	~Pager() {
		delete[] filename;
		fclose(fileptr);
	}
	FILE* getHandler() const {
		return fileptr;
	}
	void newblock() {
		n_pages++;
	}
	char* getFileName() const {
        return filename;
	}
	int getNPages() const {
		return n_pages;
	}
};

class BufferManager
{
private:
	// only one instance. The delegate
	static BufferManager* bm_delegate;

	blockInfo* bm_blockPool;

	// (maybe a little weird)
	//std::map<std::string, std::map<int, int> > bm_filePool;
	std::map< const Pager*, std::map<int, int> > bm_filePool;


	// for LRU
	int LRU_clock[BLOCK_NUMBER];
	int LRU_ptr;
public:
	BufferManager();
	~BufferManager();

	Pager* getPager(const char* filename);
    //	int    closeFile(Pager* pager);

	// return errno(defined and handled elsewhere)
	// create a file and allocate the first block for it.
	// allocation means to configure blockpool and filepool.
	int createFile(const char* filename) ;

	int deleteFile(const char* filename) ;
	// return the address of the block in main mem
	// if the file is not loaded, load it and read block
	// else if the block is not loaded, read it
	// else return the block
	//unsigned char* getblock(Pager* pager,const int block_num, const int dirty) ;
	unsigned char* getblock(Pager* pager, const int block_num, const int dirty);
	unsigned char* newblock(Pager* pager, const int dirty);
	int 		   pinblock(Pager* pager, const int block);
	int 		 unpinblock(Pager* pager, const int block);
	// return static instance ptr
	static BufferManager* getInstance() ;

private:
    // returns the block_id that is freshly read into the pool
	// read the block from disk
	// (only happens when getblock cannot allocate a block, that is, the block is not in blockpool)
	// allocation always happens here
	int bm_readblock(Pager* pager, const int block_num);

	// write the block back into disk
	// (does only the write job)
	int bm_writeblock(const int block_id);

	// flush the blocks
	int bm_flush();

	// return the least recently used block_id
	int bm_LRU();

	// return the size of the file described by fstream &
	// int bm_getFileSize(std::fstream & ifs) const;

	void bm_setRecentlyUsed(const int block_id);
};