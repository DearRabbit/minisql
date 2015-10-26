#ifndef __INDEX__MANAGER__
#define __INDEX__MANAGER__

#include <string>

using std::string;

class IndexManager
{
private:
	static IndexManager * im_delegate;
public:
	IndexManager();
	~IndexManager();
	
	int new_index(string& idx_name);		//other info mnged by catalog mgr
	int delete_index(string& idx_name);

	int new_leaf_idx(string& idx_name, void* val);
	int delete_leaf_idx(string& idx_name, void* val);

	//  for record mgr
	int select_index(string& idx_name, void* min = NULL, void* max = NULL); // any search that involves index

	static IndexManager* getInstance();
private:
};


#endif
