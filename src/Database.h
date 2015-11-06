#pragma once

#include <iostream>
#include "NodeManager.h"

#include "CatalogManager.h"
#include "IndexManager.h"
#include "RecordManager.h"
#include "BufferManager.h"

#define CHECK_TYPE(def, data)\
	(((data) == nullptr && (def) == VAL_CHAR) ||\
	 ((data) != nullptr && (def) != VAL_CHAR))

class Database
{
	private:
		static Database* db_delegate;
		NodeManager m_ast;
		BufferManager m_bufMgr;

		CatalogManager m_catMgr;
		IndexManager m_idxMgr;
		RecordManager m_recMgr;
		
	public:
		Database();
		~Database();
		static Database* getInstance();

		// All things start from here!!
		//   v:
		void run();
		
		// methods of AST Processing
		// maybe we can return a boolean
		void processAST();

	private:

		// parameter: Query Error node; 
		// errorTpye in "macros.h"
		// void processQueryError(Node* root, int errorType);

		// API
		bool db_createTable(Node *root);
		bool db_createIndex(Node *root);
		bool db_dropTable(Node *root);
		bool db_dropIndex(Node *root);
		bool db_insertVal(Node *root);
		bool db_selectVal(Node *root);
		bool db_deleteVal(Node *root);

		// returns 1 on success
		// returns 0 on failure	
		//   v: process each query
		bool processSingleAST(Node* root);
};
