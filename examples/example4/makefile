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

todo: clean server client

server: example4.o server.o
	$(CC) -o server *.o  $(LBFLAGS)  -l:libsrc.a -ltirpc

client: example4.c client.o
	$(CC) -o client client.o  $(LBFLAGS)  -l:libsrc.a -ltirpc

execute:
	@echo " Running example4 server"
	./server &
	@echo " Running example4 client"
	./client 
	pkill server
	@echo " Now Play with server in several hosts  "
clean:	
	rm -f *.o
	rm -f *.a
	rm -f example4.c
	rm -f example4.h
	rm -f server client
