# $Header: /.dub/dub/Repository/src/zoom/zoom/Makefile,v 1.3 1998/12/14 04:18:33 pahvant Exp $

DEST = ..
L = $(DEST)/lib
BIN = $(DEST)/bin
DEBUG = -g
CC = gcc $(DEBUG)
IPATH = -I$(DEST)/include
CFLAGS = $(IPATH) $(COPTS)
OBJ = zoom_main.o zoom.o scanline.o filt.o
SRC = zoom_main.c zoom.c scanline.c filt.c
# EXTRALIB = -L../../../lib -lrle
EXTRALIB = -L$(HOME)/lib -lpnm -lppm -lpgm -lpbm -ljpeg -lpng -lz -ltiff
# to pick up iris graphics library, use "make EXTRALIB=-lgl_s ..."

zoom: $(OBJ) $L/libpic.a
	$(CC) $(COPTS) -o zoom $(OBJ) $L/libpic.a $(EXTRALIB) -lm

zoom_main.o:
	$(CC) $(CFLAGS) -c zoom_main.c \
	    -DDEFAULT_FILE='"-.pnm"'
#	    -DDEFAULT_FILE='"-.rle"'
#	"note: we'll pick up pic_list from libpic.a(pic_all.o)"

zoom.lint: GHOST
	lint $(IPATH) $(SRC) >zoom.lint

install: zoom
	mv zoom $(BIN)

GHOST:

zoom_main.o: zoom.h filt.h
zoom.o: zoom.h scanline.h filt.h
scanline.o: scanline.h
filt.o: filt.h

clean:
	-rm -f zoom *.o *.lint
