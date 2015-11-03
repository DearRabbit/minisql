#include "NodeManager.h"

NodeManager::NodeManager()
{

}
NodeManager::~NodeManager()
{

}

Node* NodeManager::newEmptyNode()
{
	Node* ptr = new Node;
	m_manageArray.push_back(ptr);
	ptr->leftSon = nullptr;
	ptr->rightSon = nullptr;
	ptr->strval = nullptr;

	ptr->operation = EMPTY;
	ptr->numval = 0;
	return ptr;
}
void NodeManager::setRootNode(Node* root)
{
	m_manageRoot.push_back(root);
}
Node* NodeManager::getRootNode(size_t pos)const
{
	return m_manageRoot[pos];
}
void NodeManager::delLastRootNode()
{
	m_manageRoot.pop_back();
}
const std::vector<Node*>& NodeManager::getRootTree()const
{
	return m_manageRoot;
}
size_t NodeManager::getRootTreeSize()const
{
	return m_manageRoot.size();
}

void NodeManager::clean()
{
	for (auto it : m_manageArray)
	{
		delete [] it->strval;
		delete it;
	}
	m_manageArray.clear();
	m_manageRoot.clear();
}