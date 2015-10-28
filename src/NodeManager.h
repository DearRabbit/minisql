#ifndef __NODEMGR_H__
#define __NODEMGR_H__

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

typedef struct node
{
	node *leftSon;
	node *rightSon;

	// different simultaneously
	int operation;

	// data, in val_list of inserting,
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
		vector<Node*> m_manageRoot;
		// Storing ALL of the node, simplifies
		// the management of memory.
		vector<Node*> m_manageArray;

	public:
		NodeManager();
		~NodeManager();

		Node* newEmptyNode();
		void setRootNode(Node* root);
		Node* getRootNode(size_t pos);

		vector<Node*>& getRootTree();
		size_t getRootTreeSize();
		void clean();
};

#endif