#pragma once

#include "macros.h"

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
	OP_SHOWTABLES,

	OP_AND,
	OP_OR,

	CMP_EQ,
	CMP_NEQ,
	CMP_LT,
	CMP_GT,
	CMP_LE,
	CMP_GE,

	VAL_CHAR,
	VAL_NUMBER,
	VAL_NAME,
	VAL_INT,
	VAL_FLOAT,

	DEF_UNIQUE,
	DEF_PRIMARY
};

typedef struct node
{
	node *leftSon;
	node *rightSon;

	// simultaneous
	int operation;

	// data, in val_list of creating,
	// numval stores the len of char. 
	// (1 <= n <=255)
	char *strval;
	double numval;
} Node;

class NodeManager
{
	private:
		// Do not use unique ptr

		// For a single line, multiple queries
		// may generate more than one AST.
		// 
		std::vector<Node*> m_manageRoot;
		// Storing ALL of the node, simplifies
		// the management of memory.
		std::vector<Node*> m_manageArray;

	public:
		NodeManager();
		~NodeManager();

		Node* newEmptyNode();
		void setRootNode(Node* root);
		Node* getRootNode(size_t pos) const;
		//   v: delete the last unfinished node,
		//   v: for error processing in yyparse
		void delLastRootNode();

		const std::vector<Node*>& getRootTree() const;
		size_t getRootTreeSize() const;
		void clean();
};
