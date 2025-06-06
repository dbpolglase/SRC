CFLAGS = -g -I./hdrs -I/usr/include/tirpc -std=c99
LFLAGS =  -g -I.
CC = gcc -ansi  -g -Wall -D__USE_GNU -D_GNU_SOURCE -fPIC -c 
#CC = /opt/SUNWspro/bin/cc
GCC = gcc
#CC = purify -chain-length=10 -batch -logfile=pure.out -first-only -inuse-at-exit -cache-dir=./tmp gcc 
#cc = gcc 
AR = ar r
LINT= lint
DIR_RESULTADOS=../liberacion
DIR_CABECERAS=../includes
ODIR=./obj

_OBJETOS_LIB=codifica.o  direc.o   excepcion.o  principal.o  tiempo.o \
correos.o   emisor.o  l_correo.o   recep.o      vacio.o

OBJETOS_LIB = $(patsubst %,$(ODIR)/%,$(_OBJETOS_LIB))

$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

todo: libsrc install

libsrc: $(OBJETOS_LIB)
	$(AR) $(ODIR)/libsrc.a $(OBJETOS_LIB);
	$(GCC) -shared -o $(ODIR)/libsrc.so $(OBJETOS_LIB);


install:	
	cp $(ODIR)/*.a ../install
	cp $(ODIR)/*.so ../install
clean:	
	rm -f $(ODIR)/*.o
	rm -f $(ODIR)/*.a
	rm -f $(ODIR)/*.so
