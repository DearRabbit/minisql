#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <iostream>
#include <vector>
#include "NodeManager.h"

using std::vector;

class Database
{
	private:
		static Database* db_delegate;
		NodeManager m_ast;
		
	public:
		Database();
		~Database();
		static Database* getInstance();

		void run();
		
		// methods of AST Processing
		void processAST();
};
#endif //Database.h