/* sgitopnm.c - read an SGI image and and produce a portable anymap
**
** Copyright (C) 1994 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**
** Based on the SGI image description v0.9 by Paul Haeberli (paul@sgi.comp)
** Available via ftp from sgi.com:graphics/SGIIMAGESPEC
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** 29Jan94: first version
** 08Feb94: minor bugfix
*/
#include "pnm.h"
#include "sgi.h"
#ifndef VMS
#include <unistd.h>
#endif

/*#define DEBUG*/

#ifndef SEEK_SET
#define SEEK_SET    0
#endif


/* entry in RLE offset table */
typedef struct {
    long start;     /* offset in file */
    long length;    /* length of compressed scanline */
} TabEntry;

typedef short       ScanElem;
typedef ScanElem *  ScanLine;

/* prototypes */
static unsigned char get_byte ARGS(( FILE* f ));
static long get_big_long ARGS((FILE *f));
static short get_big_short ARGS((FILE *f));
static short get_byte_as_short ARGS((FILE *f));
static void readerr ARGS((FILE *f));
static void * xmalloc ARGS((int bytes));
#define MALLOC(n, type)     (type *)xmalloc((n) * sizeof(type))
static char * compression_name ARGS((char compr));
static void       read_bytes ARGS((FILE *ifp, int n, char *buf));
static Header *   read_header ARGS((FILE *ifp));
static TabEntry * read_table ARGS((FILE *ifp, int tablen));
static ScanLine * read_channels ARGS((FILE *ifp, Header *head, TabEntry *table, short (*func) ARGS((FILE *)) ));
static void       image_to_pnm ARGS((Header *head, ScanLine *image, xelval maxval));
static void       rle_decompress ARGS((ScanElem *src, int srclen, ScanElem *dest, int destlen));

#define WORSTCOMPR(x)   (2*(x) + 2)


static short verbose = 0;


int
main(argc, argv)
    int argc;
    char *argv[];
{
    FILE *ifp;
    int argn;
    char *usage = "[-verbose] [sgifile]";
    TabEntry *table = NULL;
    ScanLine *image;
    Header *head;
    long maxval;

    pnm_init(&argc, argv);

    argn = 1;
    while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
        if( pm_keymatch(argv[argn], "-verbose", 2) )
            verbose++;
        else
        if( pm_keymatch(argv[argn], "-noverbose", 4) )
            verbose = 0;
        else
            pm_usage(usage);
        ++argn;
    }

    if( argn < argc ) {
        ifp = pm_openr( argv[argn] );
        argn++;
    }
    else
        ifp = stdin;

    if( argn != argc )
        pm_usage(usage);

    head = read_header(ifp);
    maxval = head->pixmax - head->pixmin;
    if( maxval > PNM_MAXMAXVAL )
        pm_error("pixel values too large - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS");

    if( head->storage != STORAGE_VERBATIM )
        table = read_table(ifp, head->ysize * head->zsize);
    if( head->bpc == 1 )
        image = read_channels(ifp, head, table, get_byte_as_short);
    else
        image = read_channels(ifp, head, table, get_big_short);

    image_to_pnm(head, image, (xelval)maxval);
    pm_close(ifp);

    exit(0);
}


static Header *
read_header(ifp)
    FILE *ifp;
{
    Header *head;

    head = MALLOC(1, Header);

    head->magic     = get_big_short(ifp);
    head->storage   = get_byte(ifp);
    head->bpc       = get_byte(ifp);
    head->dimension = get_big_short(ifp);
    head->xsize     = get_big_short(ifp);
    head->ysize     = get_big_short(ifp);
    head->zsize     = get_big_short(ifp);
    head->pixmin    = get_big_long(ifp);
    head->pixmax    = get_big_long(ifp);
    read_bytes(ifp, 4, head->dummy1);
    read_bytes(ifp, 80, head->name);
    head->colormap  = get_big_long(ifp);
    read_bytes(ifp, 404, head->dummy2);

    if( head->magic != SGI_MAGIC )
        pm_error("bad magic number - not an SGI image");
    if( head->storage < 0 || head->storage > 1 )
        pm_error("unknown compression type");
    if( head->bpc < 1 || head->bpc > 2 )
        pm_error("illegal precision value %d (only 1-2 allowed)", head->bpc );
    if( head->colormap != CMAP_NORMAL )
        pm_error("unsupported non-normal pixel data");

    /* adjust ysize/zsize to dimension, just to be sure */
    switch( head->dimension ) {
        case 1:
            head->ysize = 1;
            break;
        case 2:
            head->zsize = 1;
            break;
        case 3:
            switch( head->zsize ) {
                case 1:
                    head->dimension = 2;
                    break;
                case 2:
                    pm_error("don\'t know how to interpret 2-channel image");
                    break;
                case 3:
                    break;
                default:
                    pm_message("%d-channel image, using only first 3 channels", head->zsize);
                    head->zsize = 3;
                    break;
            }
            break;
        default:
            pm_error("illegal dimension value %d (only 1-3 allowed)", head->dimension);
    }

    if( verbose ) {
        pm_message("raster size %dx%d, %d channels", head->xsize, head->ysize, head->zsize);
        pm_message("compression: %d = %s", head->storage, compression_name(head->storage));
        head->name[79] = '\0';  /* just to be safe */
        pm_message("Image name: \"%s\"", head->name);
#ifdef DEBUG
        pm_message("bpc: %d    dimension: %d    zsize: %d", head->bpc, head->dimension, head->zsize);
        pm_message("pixmin: %ld    pixmax: %ld    colormap: %ld", head->pixmin, head->pixmax, head->colormap);
#endif
    }

    return head;
}


static TabEntry *
read_table(ifp, tablen)
    FILE *ifp;
    int tablen;
{
    TabEntry *table;
    int i;

    table = MALLOC(tablen, TabEntry);

#ifdef DEBUG
    pm_message("reading offset table");
#endif

    for( i = 0; i < tablen; i++ )
        table[i].start = get_big_long(ifp);
    for( i = 0; i < tablen; i++ )
        table[i].length = get_big_long(ifp);

    return table;
}



static ScanLine *
read_channels(ifp, head, table, func)
    FILE *ifp;
    Header *head;
    TabEntry *table;
    short (*func) ARGS((FILE *));
{
    ScanLine *image;
    ScanElem *temp;
    int channel, maxchannel, row, sgi_index, i;
    long offset, length;

#ifdef DEBUG
    pm_message("reading channels");
#endif

    maxchannel = head->zsize;
    image = MALLOC(head->ysize * maxchannel, ScanLine);
    if( table ) temp = MALLOC(WORSTCOMPR(head->xsize), ScanElem);

    for( channel = 0; channel < maxchannel;  channel++ ) {
#ifdef DEBUG
        pm_message("    channel %d", channel);
#endif
        for( row = 0; row < head->ysize; row++ ) {
            sgi_index = channel * head->ysize + row;
            image[sgi_index] = MALLOC(head->xsize, ScanElem);
            if( table ) {
                offset = table[sgi_index].start;
                length = table[sgi_index].length;
                if( head->bpc == 2 )
                    length /= 2;   /* doc says length is in bytes, we are reading words */
                if( fseek(ifp, offset, SEEK_SET) != 0 )
                    pm_error("seek error for offset %ld", offset);

                for( i = 0; i < length; i++ )
                    temp[i] = (*func)(ifp);
                rle_decompress(temp, length, image[sgi_index], head->xsize);
            }
            else {
                for( i = 0; i < head->xsize; i++ )
                    image[sgi_index][i] = (*func)(ifp);
            }
        }
    }

    if( table ) free(temp);
    return image;
}


static void
image_to_pnm(head, image, maxval)
    Header *head;
    ScanLine *image;
    xelval maxval;
{
    int col, row, format;
    xel *pnmrow = pnm_allocrow(head->xsize);
    int sub = head->pixmin;

    if( head->zsize == 1 ) {
        pm_message("writing PGM image");
        format = PGM_TYPE;
    }
    else {
        pm_message("writing PPM image");
        format = PPM_TYPE;
    }

    pnm_writepnminit(stdout, head->xsize, head->ysize, (xelval)maxval, format, 0);
    for( row = head->ysize-1; row >= 0; row-- ) {
        for( col = 0; col < head->xsize; col++ ) {
            if( format == PGM_TYPE )
                PNM_ASSIGN1(pnmrow[col], image[row][col] - sub);
            else {
                pixval r, g, b;
                r = image[row][col] - sub;
                g = image[head->ysize + row][col] - sub;
                b = image[2* head->ysize + row][col] - sub;
                PPM_ASSIGN(pnmrow[col], r, g, b);
            }
        }
        pnm_writepnmrow(stdout, pnmrow, head->xsize, (xelval)maxval, format, 0);
    }
    pnm_freerow(pnmrow);
}


static void
rle_decompress(src, srcleft, dest, destleft)
    ScanElem *src;
    int srcleft;
    ScanElem *dest;
    int destleft;
{
    int count;
    unsigned char el;

    while( srcleft ) {
        el = (unsigned char)(*src++ & 0xff);
        --srcleft;
        count = (int)(el & 0x7f);

        if( count == 0 )
            return;
        if( destleft < count )
            pm_error("RLE error: too much input data (space left %d, need %d)", destleft, count);
        destleft -= count;
        if( el & 0x80 ) {
            if( srcleft < count )
                pm_error("RLE error: not enough data for literal run (data left %d, need %d)", srcleft, count);
            srcleft -= count;
            while( count-- )
                *dest++ = *src++;
        }
        else {
            if( srcleft == 0 )
                pm_error("RLE error: not enough data for replicate run");
            while( count-- )
                *dest++ = *src;
            ++src;
            --srcleft;
        }
    }
    pm_error("RLE error: no terminating 0-byte");
}


/* basic I/O functions, taken from ilbmtoppm.c */

static short
get_big_short(ifp)
    FILE *ifp;
{
    short s;

    if( pm_readbigshort(ifp, &s) == -1 )
        readerr(ifp);

    return s;
}

static long
get_big_long(ifp)
    FILE *ifp;
{
    long l;

    if( pm_readbiglong(ifp, &l) == -1 )
        readerr(ifp);

    return l;
}

static unsigned char
get_byte(ifp)
    FILE* ifp;
{
    int i;

    i = getc(ifp);
    if( i == EOF )
        readerr(ifp);

    return (unsigned char) i;
}


static void
readerr(f)
    FILE *f;
{
    if( ferror(f) )
        pm_error("read error");
    else
        pm_error("premature EOF");
}


static void
read_bytes(ifp, n, buf)
    FILE *ifp;
    int n;
    char *buf;
{
    int r;

    r = fread((void *)buf, 1, n, ifp);
    if( r != n )
        readerr(ifp);
}


static short
get_byte_as_short(ifp)
    FILE *ifp;
{
    return (short)get_byte(ifp);
}


static void *
xmalloc(bytes)
    int bytes;
{
    void *mem;

    if( bytes == 0 )
        return NULL;

    mem = malloc(bytes);
    if( mem == NULL )
        pm_error("out of memory allocating %d bytes", bytes);
    return mem;
}


static char *
compression_name(compr)
    char compr;
{
    switch( compr ) {
        case STORAGE_VERBATIM:
            return "none";
        case STORAGE_RLE:
            return "RLE";
        default:
            return "unknown";
    }
}

