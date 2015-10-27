#ifndef __TYPES_H__
#define __TYPES_H__

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

#endif