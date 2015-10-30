#pragma once

// MINISQL_MACROS

typedef void CurseT;

#define	MINISQL_OK			(0)				// query success
#define	MINISQL_ETYPE		(-1)			// Type error
#define	MINISQL_EIO			(-2)			// Table not found 
#define	MINISQL_ECONSTRAINT	(-3)			// Constraints error 