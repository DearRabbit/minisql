#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <iostream>
#include <vector>
#include "NodeManager.h"

#if 0
#include "CatalogManager.h"
#include "IndexManager.h"
#include "RecordManager.h"
#endif

using std::vector;

class Database
{
	private:
		static Database* db_delegate;
		NodeManager m_ast;

#if 0
		CatalogManager m_catMgr;
		IndexManager m_idxMgr;
		RecordManager m_recMgr;
#endif
		
	public:
		Database();
		~Database();
		static Database* getInstance();


		// All things start from here!!
		//   v
		void run();
		
		// methods of AST Processing
		// maybe we can return a boolean
		void processAST();

	private:

};
#endif //Database.h