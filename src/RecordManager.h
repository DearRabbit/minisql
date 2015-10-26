#ifndef __RECORD__MANAGER__
#define __RECORD__MANAGER__

#include <string>

using std::string;

class RecordManager
{
private:
	static RecordManager* rm_delegate;
public:
	RecordManager();
	~RecordManager();

	int new_table( string& table_name );		// Must be called after catalog file is created
	int delete_table( string& table_name );		
	
	int new_record( string& table_name, void** vals, unsigned int ncol);		// insert record
	
	// - cond_types: condition type list
	// - ncond: condition number
	// - conds: condition list(ConditionStruct is still pseudocode)
	int delete_record( string& table_name, int* cond_types , ConditionStruct* conds, unsigned int ncond = 1);

	// printing happens while searching
	int select_record( string& table_name, int* cond_types, ConditionStruct* conds, unsigned int ncond = 1);

	static RecordManager* getInstance();
};


#endif
