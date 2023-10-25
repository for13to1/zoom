# $Header: /.dub/dub/Repository/src/zoom/libpic/Makefile,v 1.3 1998/12/14 05:06:15 pahvant Exp $

DEST = ..
L = $(DEST)/lib
IPATH = -I. -I$(DEST)/include -I../../../include
CC = gcc -g
CFLAGS = $(IPATH) 



SRC = window.c pic.c pic_all.c pic_file.c swap.c \
	dump.c \
	jpg.c \
	pnm.c \
	png.c \
	gif.c readGIF.c \
	tif.c
#	rle.c  rle_pic.c  



OBJ = window.o pic.o pic_all.o pic_file.o swap.o \
	dump.o jpg.o pnm.o png.o gif.o readGIF.o tif.o
#        rle.o  rle_pic.o  



HDR = pixel.h window.h pic.h \
	dump.h
#	dump.h  picrle.h 

.SUFFIXES: .o .s .i .c

.c.i:
	$(CC) $(CFLAGS) -P $*.c
	cat -s <$*.i >temp
	mv temp $*.i

all: libpic.a

libpic.a: $(OBJ)
	ar cru libpic.a $(OBJ)
	ranlib libpic.a

install: libpic.a
	mv libpic.a $L
#	cp -p $(HDR) $(DEST)/include
	cp $(HDR) $(DEST)/include

libpic.lint: GHOST
	lint $(IPATH) $(SRC) >libpic.lint

clean:
	-rm -f libpic.a *.o *.lint

GHOST:

pic.o: pic.h pixel.h
window.o: window.h
pic_all.o: pic.h
pic_file.o: pic.h

dump.o: dump.h pixel.h pic.h

pnm.o: pic.h
