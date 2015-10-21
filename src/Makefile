CC = clang++
LEX = flex
YACC = bison -dtv
LDFLAGS =
DBGFLAGS = -g

all: yacc.tab.o lex.yy.o main.o Database.o
	$(CC) $(LDFLAGS) $^

main.o: main.cpp
	$(CC) $(DBGFLAGS) -c $^

Database.o: Database.cpp
	$(CC) $(DBGFLAGS) -c $^

lex.yy.o: lex.yy.cpp
	$(CC) $(DBGFLAGS) -c $^

yacc.tab.o: yacc.tab.cpp
	$(CC) $(DBGFLAGS) -c $^

lex.yy.cpp: lex.l
	$(LEX) lex.l
	mv lex.yy.c lex.yy.cpp

yacc.tab.cpp yacc.tab.h: yacc.yy
	$(YACC) yacc.yy
	mv yacc.tab.cc yacc.tab.cpp
	mv yacc.tab.hh yacc.tab.h

clean: 
	rm lex.yy.* yacc.tab.* a.out main.o yacc.output
