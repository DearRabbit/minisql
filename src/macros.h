#pragma once

#include <vector>
#include <string>
#include <iostream>
using std::string;
using std::vector;
using std::cout;

// MINISQL_MACROS

#define MINISQL_PROMPT1() printf("minisql> ")
#define MINISQL_PROMPT2() printf("       > ")
// #define _SHOW_TIME

/* throw exception when failed */
#define STRDUP_NEW(dest, src) \
{\
	dest = new char[strlen(src) + 1];\
	strcpy(dest, src);\
}

// Is it okay to put it here?
// block no.+block offset
typedef std::pair<int, int> CursePair;

#define	MINISQL_OK			(0)				// query success
#define	MINISQL_ETYPE		(-1)			// Type error
#define	MINISQL_EIO			(-2)			// Table not found 
#define	MINISQL_ECONSTRAINT	(-3)			// Constraints error 

#define SHOW_TIME

///--- Don't know if this is the right place.
///--- If anything, please point out.
#define  IDX_BLOCKHEADER_SIZE   (28)
#define  IDX_FILEHEADER_SIZE	(44)
#define  IDX_LEFTPTR_SIZE	 	(8)

#define  IDX_FLAG_NOROOT		(-1)
#define  IDX_FLAG_NONPAGE		(-1)
#define  IDX_FLAG_LEAF 			(1)
#define  IDX_FLAG_NONLEAF		(0)

#define  IDX_TYPE_INT			(1)
#define  IDX_TYPE_FLOAT			(2)
#define  IDX_TYPE_STRING		(3)

#define  CAT_FLAG_NONBLOCK		(-1)

#define  BUFFER_FLAG_DIRTY		(1)
#define  BUFFER_FLAG_NONDIRTY   (0)

class TableExistException {};
class IndexExistException {};
class TableNonExistException {};
class IndexNonExistException {};
class ColumnNonExistException {};
class NotUniqueKeyException {};
class MulPrimaryKeyException {};
class UndefinedPriKeyException
{
public:
	UndefinedPriKeyException(char* node)
	{
		m_str = node;
	}
	char* m_str;
};
class TypeMismatchException
{
public:
	// -if mismatch at column 'name'
	//  then str = "at column 'name'"
	// -else if data > def
	//  then str = "too many values"
	// -else if data < def
	//  then str = "too few values"
	TypeMismatchException(string str)
	{
		columnName = str;
	};
	string columnName;
};
class MultipleKeyException
{
public:
	MultipleKeyException(char* tableName, char* columnName)
	{
		name = string(tableName)+string(".")+string(columnName);
	};
	string name;
};

typedef struct {
    char  Header_string[16];
	int   Type;		// int, float, or varchar(n)
	int   Val_size;
	int   Degree;
	int   Root;
	int   Free_list;
	int   N_freepages;
	int   Version_number;
} IDXFileHeader;