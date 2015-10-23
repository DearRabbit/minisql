#include <iostream>
#include <cstring>
#include "Database.h"

void Database::addRootNode(NodeAST* root)
{
	m_manageRoot.push_back(root);
}
void Database::delErrRootNode()
{
	m_manageRoot.pop_back();
}

NodeAST* Database::newEmptyNode()
{
	NodeAST* ptr = new NodeAST;
	m_manageArray.push_back(ptr);
	ptr->leftSon = nullptr;
	ptr->rightSon = nullptr;
	ptr->strval = nullptr;

	ptr->operation = EMPTY;
	ptr->numval = 0;
	return ptr;
}

void Database::processAST()
{
	int i = 0;
	printf("Do something\n");
	for (auto it : m_manageRoot)
	{
		printf("---------%03d---------:\n", ++i);
		if (it->strval)
			printf("String:%s\n", it->strval);
		printf("num:%lf\n", it->numval);
		printf("op:%d\n", it->operation);
	}
	cleanAST();
}

void Database::cleanAST()
{
	for (auto it : m_manageArray)
	{
		delete [] it->strval;
		delete it;
	}
	m_manageArray.clear();
	m_manageRoot.clear();
}

Database::~Database()
{
	if (m_manageRoot.size() != 0)
		cleanAST();
}

