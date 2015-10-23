#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <iostream>
#include <vector>

using std::vector;

enum operationName
{	
	EMPTY,
	OP_CREATE_TABLE,
	OP_CREATE_INDEX,
	OP_DROP_TABLE,
	OP_DROP_INDEX,
	OP_SELECT,
	OP_INSERT,
	OP_DELECT,
	OP_AND,
	OP_OR,

	VAL_CHAR,
	VAL_NUMBER,
	VAL_NAME,
	VAL_INT,
	VAL_FLOAT,

	CMP_EQ,
	CMP_NEQ,
	CMP_LT,
	CMP_GT,
	CMP_LE,
	CMP_GE,

	DEF_UNIQUE,
	DEF_PRIMARY
};

typedef struct nodeAST
{
	nodeAST *leftSon;
	nodeAST *rightSon;

	// different simultaneously
	int operation;

	// data, in val_list of inserting,
	// numval stores the len of char. 
	// (1 <= n <=255)
	char *strval;
	double numval;
} NodeAST;

class Database
{
	private:
		// For a single line, multiple queries
		// may generate more than one AST.
		vector<NodeAST*> m_manageRoot;
		// Storing ALL of the node, simplifies
		// the management of memory.
		vector<NodeAST*> m_manageArray;
		
	public:
		~Database();
		void addRootNode(NodeAST* root);
		void delErrRootNode();
		NodeAST* newEmptyNode();
		void processAST();
		void cleanAST();

};
#endif //Database.h