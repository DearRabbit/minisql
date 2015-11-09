CC = clang++ 
LEX = flex
YACC = bison -dtv
COMFLAGS = -std=c++11
LDFLAGS =
DBGFLAGS = -g

all: CatalogManager.o IndexManager.o RecordManager.o NodeManager.o BufferManager.o Database.o\
 	bptree.o yacc.tab.o lex.yy.o main.o
	$(CC) $(LDFLAGS) $^ -o minisql

win: CC=g++
win: all

main.o: main.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
Database.o: Database.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
RecordManager.o: RecordManager.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
IndexManager.o: IndexManager.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
CatalogManager.o: CatalogManager.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
NodeManager.o: NodeManager.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
BufferManager.o: BufferManager.cc
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^
bptree.o: bptree.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^

lex.yy.o: lex.yy.c
	$(CC) $(DBGFLAGS) -c $^

yacc.tab.o: yacc.tab.cc
	$(CC) $(DBGFLAGS) -c $^ $(COMFLAGS)

lex.yy.c: lex.l
	$(LEX) lex.l

yacc.tab.cc yacc.tab.h: yacc.yy
	$(YACC) yacc.yy

clean: 
	rm -f lex.yy.* yacc.tab.* *.o yacc.output minisql
cleandb:
	rm -f *.db *.idx *.frm
