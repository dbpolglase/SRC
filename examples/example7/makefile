CFLAGS = -c -g -I. -I./../../src.b/hdrs 
BDTFLAGS = -I. -I./hdrs -I../hdrs 
LFLAGS = -g -I.
CC = gcc -ansi  -Wall -D__USE_GNU -D_GNU_SOURCE
AR = ar r
LINT= lint
LEX=lex
YACC=yacc
#LFLAGS=-d
LFLAGS=-o
#YFLAGS=-t -y -H
YFLAGS=-H -o 
DIR_RESULTADOS=../liberacion
DIR_CABECERAS=../includes

OBJETOS=main.o 
LBFLAGS=-L../../install


.SUFFIXES : .src .c
.src.c:
	../../install/src $<

.SUFFIXES : .c .o
.c.o:
	$(CC) $(CFLAGS) $*.c

todo: clean server1 server2

server1: ex1.o ex2.c server1.o
	$(CC) -o server1 ex1.o server1.o $(LBFLAGS)  -l:libsrc.a -ltirpc

server2: ex2.o ex1.c server2.o
	$(CC) -o server2 ex2.o server2.o $(LBFLAGS)  -l:libsrc.a -ltirpc

executa:
	@echo " Running server1 and server2 in diferent terminals"
	@echo " Enter something "
clean:	
	rm -f *.o
	rm -f *.a
	rm -f ex1.c
	rm -f ex2.c
	rm -f ex1.h
	rm -f ex2.h
	rm -f server1 server2
