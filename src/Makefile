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

lex.yy.o: lex.yy.c
	$(CC) $(DBGFLAGS) -c $^

yacc.tab.o: yacc.tab.cc
	$(CC) $(DBGFLAGS) -c $^

lex.yy.c: lex.l
	$(LEX) lex.l

yacc.tab.cc yacc.tab.h: yacc.yy
	$(YACC) yacc.yy

clean: 
	rm lex.yy.* yacc.tab.* a.out main.o Database.o yacc.output
