#pragma once

// MINISQL_MACROS

#define MINISQL_PROMPT1() printf("minisql> ")
#define MINISQL_PROMPT2() printf("       > ")

/* throw exception when failed */
#define STRDUP_NEW(dest, src) \
{\
	dest = new char[strlen(src) + 1];\
	strcpy(dest, src);\
}


typedef void CurseT;

#define	MINISQL_OK			(0)				// query success
#define	MINISQL_ETYPE		(-1)			// Type error
#define	MINISQL_EIO			(-2)			// Table not found 
#define	MINISQL_ECONSTRAINT	(-3)			// Constraints error 