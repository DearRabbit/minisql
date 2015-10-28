CC = clang++ -std=c++11
LEX = flex
YACC = bison -dtv
LDFLAGS =
DBGFLAGS = -g

all: Database.o NodeManager.o yacc.tab.o lex.yy.o main.o
	$(CC) $(LDFLAGS) $^

main.o: main.cpp
	$(CC) $(DBGFLAGS) -c $^

Database.o: Database.cpp
	$(CC) $(DBGFLAGS) -c $^

NodeManager.o: NodeManager.cpp
	$(CC) $(DBGFLAGS) -c $^

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
