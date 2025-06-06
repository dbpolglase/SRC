CFLAGS = -c -g -I./hdrs -I../hdrs -D_LEER_
LFLAGS = -g -I.
CC = gcc -ansi  -Wall -D__USE_GNU -D_GNU_SOURCE
#cc = gcc 
AR = ar r
LINT= lint
LEX=lex
YACC=yacc
#LFLAGS=-d -o
LFLAGS=-o
#YFLAGS=-t -y -H -o
YFLAGS=-H -o 
DIR_RESULTADOS=../liberacion
DIR_CABECERAS=../includes
ODIR=obj

_OBJETOS=codi.o  parser.o  scaner.o  src_gen.o
OBJETOS = $(patsubst %,$(ODIR)/%,$(_OBJETOS))


$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

todo: compila install

compila: $(OBJETOS)
	$(CC) -o $(ODIR)/src $(ODIR)/*.o  

install:
	cp $(ODIR)/src ../install

clean:	
	rm -f $(ODIR)/*.o
	rm -f $(ODIR)/*.a
