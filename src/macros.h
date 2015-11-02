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

///--- Don't know if this is the right place.
///--- If anything, please point out.
#define  IDX_BLOCKHEADER_SIZE   (25)
#define  IDX_FILEHEADER_SIZE	(44)
#define  IDX_LEFTPTR_SIZE	 	(8)
#define  IDX_FLAG_NOROOT		(-1)
#define  IDX_FLAG_NONPAGE		(-2)
#define  IDX_FLAG_LEAF 			(1)
#define  IDX_FLAG_NONLEAF		(0)

#define  IDX_TYPE_INT			(1)
#define  IDX_TYPE_FLOAT			(2)
#define  IDX_TYPE_STRING		(3)


#define  BUFFER_FLAG_DIRTY		(1)
#define  BUFFER_FLAG_NONDIRTY   (0)
