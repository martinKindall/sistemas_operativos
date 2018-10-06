# Observaciones importantes:
#
# - Para usar este Makefile es necesario definir la variable
#   de ambiente NSYSTEM con el directorio en donde se encuentra
#   la raiz de nSystem.  Por ejemplo en csh esto se hace con:
#
#   setenv NSYSTEM ~/nsystem64
#
# - Compile ingresando el comando make
#
# - Depure usando gdb o ddd.

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

CFLAGS= -ggdb -pedantic -Wall -std=c99 -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LFLAGS= -ggdb

all: test-transbordo

.SUFFIXES:
.SUFFIXES: .o .c .s

.c.o:
	gcc -c $(CFLAGS) $<

test-transbordo.o test-transbordo2.o transbordo.o: transbordo.h

test-transbordo: test-transbordo.o transbordo.o
	gcc $(LFLAGS) test-transbordo.o transbordo.o -o $@ $(LIBNSYS)

test-transbordo2: test-transbordo2.o transbordo.o
	gcc $(LFLAGS) test-transbordo2.o transbordo.o -o $@ $(LIBNSYS)

clean:
	rm -f *.o *~

cleanall:
	rm -f *.o *~ test-transbordo test-transbordo2
