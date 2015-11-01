#include "BufferManager.h"
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility>

#if _DEBUG
#include <iostream>
#endif // _DEBUG


BufferManager* BufferManager::bm_delegate = NULL;


BufferManager::BufferManager()
{
	bm_blockPool = new blockInfo[BLOCK_NUMBER];
	bm_delegate = this;
	memset(LRU_clock, 0, BLOCK_NUMBER*sizeof(int));
	LRU_ptr = 0;
}

BufferManager::~BufferManager()
{
    bm_flush();
    delete bm_blockPool;
    for( auto pagerPair : bm_filePool) {
        delete pagerPair.first;
    }
	// destroy the map
}

BufferManager*
BufferManager::getInstance()
{
	return bm_delegate;
}

unsigned char*
BufferManager::getblock
(Pager* pager, const int block_num , const int dirty)
{
	int block_id;

	// The pager is not registered
	if (bm_filePool.count(pager) == 0) { // undefined behavior, must open file first
		exit(1);
	}
	else {
        if(block_num >= pager->getNPages() ) {
            return NULL;        // Illegal,
        }
		std::map<int,int> & filemap = bm_filePool[pager];
		// The block is not registered
		if(filemap.count(block_num) == 0) {
			block_id = bm_readblock(pager, block_num);
		}
		else { // The block is registered
			block_id = filemap[block_num];
		}
	}

	blockInfo & rblock = bm_blockPool[block_id];
	if(dirty)
		rblock.setDirty();

	bm_setRecentlyUsed(block_id);

	return rblock.getblock();
}

int
BufferManager::createFile(const char* filename)
{
	if(_access(filename, 0) == 0 ) {
		exit(1);
	}
	else {
		FILE* file;

		file = fopen(filename, "w");

		fclose(file);
		// bm_readblock(filename, 0);

		return 0;
	}
}

int
BufferManager::deleteFile(const char* filename)
{
	remove(filename);

	if(_access(filename, 0) == 0) {
		return 0;
	}
	return 1;
}

int
BufferManager::bm_readblock(Pager* pager, const int block_num)
{
	int nblock_id  = bm_LRU();

	blockInfo & rblock = bm_blockPool[nblock_id];

#if _DEBUG
    std::cout << "Block " << nblock_id << " is chosen in LRU." << std::endl;
#endif // _DEBUG
	if(!rblock.isDirty()) {

        // If rblock is registered, unregister it
        if( rblock.isSet() ){
            bm_filePool[rblock.getPager()].erase(rblock.getBlockNum() );
#if _DEBUG
    std::cout << nblock_id << " is swapped." << std::endl;
#endif // _DEBUG
        }

		rblock.setblock(pager, block_num);

		fseek(pager->getHandler(), block_num* BLOCK_SIZE, SEEK_SET);
		fread(rblock.getblock(), 1, BLOCK_SIZE, pager->getHandler() );

		bm_filePool[pager][block_num] = nblock_id;

		return nblock_id;
	}
	else {  // Dirty block, which must have been registered
#if _DEBUG
    std::cout << nblock_id << " is swapped." << std::endl;
#endif // _DEBUG
		bm_writeblock(nblock_id);

        // Unregister the block in the filePool.
		bm_filePool[rblock.getPager() ].erase( rblock.getBlockNum() );

        // Reset block.
		rblock.setblock(pager, block_num);
		fread(rblock.getblock(), 1, BLOCK_SIZE, pager->getHandler() );

        // Register the block
        // The pager must be registered in openFile();
		bm_filePool[pager][block_num] = nblock_id;
	}

	return nblock_id;
}

int
BufferManager::bm_writeblock(const int block_id)
{
	blockInfo & rblock = bm_blockPool[block_id];

	int blockn = rblock.getBlockNum();
	FILE* file = rblock.getPager()->getHandler();
	fseek( file, blockn * BLOCK_SIZE, SEEK_SET);
	fwrite(rblock.getblock(), 1, BLOCK_SIZE, file);
	rblock.setDirty(0);

    return 0;
}

int
BufferManager::bm_flush()
{
	for(int i=0; i<BLOCK_NUMBER; i++) {
		if(bm_blockPool[i].isDirty() ) {
			bm_writeblock(i);
		}
	}

	return 0;
}

int
BufferManager::bm_LRU()
{
	// The least recently used block for switching.
	// When the block is pinned, it cannot be selected to switch.
    #if _DEBUG
    int* local_LRU_clock = LRU_clock;
    #endif // _DEBUG

    int local_ptr = LRU_ptr;
    // if not find LRU or is pinned, continue
    int counter=0;
	while(LRU_clock[local_ptr] != 0 || bm_blockPool[local_ptr].isPinned() ) {
		LRU_clock[local_ptr++] = 0;
		local_ptr %= BLOCK_NUMBER;
		counter++;
		if(counter > BLOCK_NUMBER*2) {
            printf("FATAL: All blocks pinned in the pool.");
            exit(1);	// all pinned error
		}
	}
    LRU_ptr = (local_ptr+1)%BLOCK_NUMBER;
	return local_ptr;
}


// int
// BufferManager::bm_getFileSize(std::fstream & ifs) const
// {
// 	if( !ifs.is_open() ) {
// 		return -1;	//fatal error
// 	}
// 	else {
// 		// always use the read ptr
// 		unsigned int curpos = ifs.tellg();	// saved
// 		ifs.seekg(0, ios::end);
// 		int filesz = ifs.tellg();
// 		ifs.seekg(curpos, ios::beg);

// 		return filesz;		// in bytes
// 	}
// }


void
BufferManager::bm_setRecentlyUsed(const int block_id)
{
	LRU_clock[block_id] = 1;
}

// Pager registered
Pager*
BufferManager::getPager(const char* filename)	// The file must exist
{
	if(_access(filename, 0) != 0) {
        printf("FATAL: File missing.");

		exit(1);
	}
	for(auto pagerPair: bm_filePool) {
        if(strcmp(pagerPair.first->getFileName(), filename) == 0) {
            return (Pager*)(pagerPair.first);
        }
	}
	Pager* pager = new Pager(filename);

	bm_filePool.insert( std::pair<Pager*, std::map<int,int> >(pager, *(new std::map<int, int>)) );

	return pager;
}

// Pager unregistered, blocks flushed
//int
//BufferManager::closeFile(Pager* pager)
//{
//    int block_id;
//	for(auto blockPair : bm_filePool[pager]) {
//	    block_id = blockPair.second;
//        if(bm_blockPool[block_id].isDirty())
//            bm_writeblock(blockPair.second);
//		bm_blockPool[blockPair.second].clear();
//	}
//
//	bm_filePool.erase(pager);
//
//	delete pager;
//
//	return 1;
//}

// Must ensure that the pager is registered
unsigned char*
BufferManager::newblock(Pager* pager, const int dirty)
{
	unsigned char emptyBlock[BLOCK_SIZE];
	memset(emptyBlock, 0, BLOCK_SIZE);

	fseek(pager->getHandler(), 0, SEEK_END);
	fwrite(emptyBlock, 1, BLOCK_SIZE, pager->getHandler() );

	int block_id = bm_readblock(pager, pager->getNPages() );

	pager->newblock();
    if(dirty)
        bm_blockPool[block_id].setDirty();
	bm_setRecentlyUsed(block_id);

	return bm_blockPool[block_id].getblock();
}

//!! Called right after getblock() to ensure that the block is in the blockpool.
int
BufferManager::pinblock(Pager * pager, const int block)
{
    blockInfo &rblock = bm_blockPool[ bm_filePool[pager][block] ];

    rblock.setPin();

    return 1;
}

int
BufferManager::unpinblock(Pager* pager, const int block)
{
    blockInfo &rblock = bm_blockPool[bm_filePool[pager][block]];

    rblock.setPin(0);

    return 1;
}
