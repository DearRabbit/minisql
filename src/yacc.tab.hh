/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FILENAME = 258,
     NUMBER = 259,
     NAME = 260,
     STRING = 261,
     AND = 262,
     CHAR = 263,
     CMD_FINISH = 264,
     CREATE = 265,
     DELETE = 266,
     DROP = 267,
     ERRORTOKEN = 268,
     EXECFILE = 269,
     FLOAT = 270,
     FROM = 271,
     INDEX = 272,
     INSERT = 273,
     INTEGER = 274,
     INTO = 275,
     KEY = 276,
     ON = 277,
     OR = 278,
     PRIMARY = 279,
     SHOW = 280,
     SELECT = 281,
     TABLE = 282,
     TABLES_IN_SHOW = 283,
     UNIQUE = 284,
     VALUES = 285,
     QUIT = 286,
     WHERE = 287,
     EQ = 288,
     NEQ = 289,
     LT = 290,
     GT = 291,
     LE = 292,
     GE = 293
   };
#endif
/* Tokens.  */
#define FILENAME 258
#define NUMBER 259
#define NAME 260
#define STRING 261
#define AND 262
#define CHAR 263
#define CMD_FINISH 264
#define CREATE 265
#define DELETE 266
#define DROP 267
#define ERRORTOKEN 268
#define EXECFILE 269
#define FLOAT 270
#define FROM 271
#define INDEX 272
#define INSERT 273
#define INTEGER 274
#define INTO 275
#define KEY 276
#define ON 277
#define OR 278
#define PRIMARY 279
#define SHOW 280
#define SELECT 281
#define TABLE 282
#define TABLES_IN_SHOW 283
#define UNIQUE 284
#define VALUES 285
#define QUIT 286
#define WHERE 287
#define EQ 288
#define NEQ 289
#define LT 290
#define GT 291
#define LE 292
#define GE 293




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 26 "yacc.yy"
{
	char *strval;
	Node *treeNode;
	/* warning: use double? */
	double numval;
}
/* Line 1529 of yacc.c.  */
#line 132 "yacc.tab.hh"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

