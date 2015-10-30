CC = clang++ 
LEX = flex
YACC = bison -dtv
COMFLAGS = -std=c++11
LDFLAGS =
DBGFLAGS = -g

all: Database.o NodeManager.o yacc.tab.o lex.yy.o main.o
	$(CC) $(LDFLAGS) $^

main.o: main.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^

Database.o: Database.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^

NodeManager.o: NodeManager.cpp
	$(CC) $(DBGFLAGS) $(COMFLAGS) -c $^

lex.yy.o: lex.yy.c
	$(CC) $(DBGFLAGS) -c $^

yacc.tab.o: yacc.tab.cc
	$(CC) $(DBGFLAGS) -c $^

lex.yy.c: lex.l
	$(LEX) lex.l

yacc.tab.cc yacc.tab.h: yacc.yy
	$(YACC) yacc.yy

clean: 
	rm lex.yy.* yacc.tab.* *.o a.out yacc.output
