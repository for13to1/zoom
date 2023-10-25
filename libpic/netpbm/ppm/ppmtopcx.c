/* ppmtopcx.c - convert a portable pixmap to PCX
**
** Copyright (C) 1994 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
** based on ppmtopcx.c by Michael Davidson
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** 11/Dec/94: first version
** 12/Dec/94: added support for "packed" format (16 colors or less)
*/
#include "ppm.h"
#include "ppmcmap.h"

/*#define DEBUG*/

#define MAXCOLORS       256

#define PCX_MAGIC       0x0a            /* PCX magic number             */
#define PCX_256_COLORS  0x0c            /* magic number for 256 colors  */
#define PCX_MAXVAL      (pixval)255


/* prototypes */
static void ppm_to_16col_pcx ARGS((pixel **pixels, int cols, int rows,
                                   pixel *cmap, int colors, colorhash_table cht));
static void ppm_to_256col_pcx ARGS((pixel **pixels, int cols, int rows,
                                   pixel *cmap, int colors, colorhash_table cht));
static void ppm_to_truecol_pcx ARGS((pixel **pixels, int cols, int rows, int maxval));
static void write_header ARGS((FILE *fp, int cols, int rows, int BitsPerPixel, int Planes, pixel *cmap16));
static void PCXEncode ARGS(( FILE* fp, unsigned char* buf, int Size ));
static void ToPlanes ARGS((unsigned char *rawrow, int width, unsigned char *buf, int planes));
static void PackBits ARGS((unsigned char *rawrow, int width, unsigned char *buf, int bits));
static void Putword ARGS(( int w, FILE* fp ));
static void Putbyte ARGS(( int b, FILE* fp ));

static short force24 = 0;
static short packbits = 0;


int
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    int argn, rows, cols, colors, i;
    pixval maxval;
    pixel black_pixel, *cmap, **pixels;
    colorhist_vector chv;
    colorhash_table cht;
    char* usage = "[-24bit] [-packed] [ppmfile]";

    ppm_init(&argc, argv);

    argn = 1;
    while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
        if( pm_keymatch(argv[argn], "-24bit", 3) )
            force24 = 1;
        else
        if( pm_keymatch(argv[argn], "-packed", 2) )
            packbits = 1;
        else
            pm_usage(usage);
        ++argn;
    }
    if( argn < argc ) {
        ifp    = pm_openr(argv[argn]);
        ++argn;
    }
    else
        ifp    = stdin;
    if( argn != argc )
        pm_usage(usage);


    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );
    pm_close( ifp );

    if( !force24 ) {
        /* Figure out the colormap. */
        pm_message( "computing colormap..." );
        chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &colors );
        if ( chv == (colorhist_vector) 0 ) {
            pm_message("too many colors - writing a 24bit PCX file");
            pm_message("if you want a non-24bit file, do a \'ppmquant %d\'", MAXCOLORS);
            force24 = 1;
        }
    }

    if( maxval != PCX_MAXVAL )
        pm_message("maxval is not %d - automatically rescaling colors", PCX_MAXVAL);

    if( force24 )
        ppm_to_truecol_pcx(pixels, cols, rows, maxval);
    else {
        pm_message("%d colors found", colors);

        /* Force black to slot 0 if possible. */
        PPM_ASSIGN(black_pixel, 0, 0, 0 );
        ppm_addtocolorhist(chv, &colors, MAXCOLORS, &black_pixel, 0, 0 );


        /* build colormap */
        cmap = ppm_allocrow(MAXCOLORS);
        for( i = 0; i < colors; i++ ) {
            pixval r, g, b;
            r = PPM_GETR(chv[i].color); g = PPM_GETG(chv[i].color); b = PPM_GETB(chv[i].color);
            if( maxval != PCX_MAXVAL ) {
                r = r * PCX_MAXVAL / maxval; g = g * PCX_MAXVAL / maxval; b = b * PCX_MAXVAL / maxval;
            }
            PPM_ASSIGN(cmap[i], r, g, b);
        }
        for( ; i < MAXCOLORS; i++ )
            PPM_ASSIGN(cmap[i], 0,0,0);

        /* make a hash table for fast color lookup */
        cht = ppm_colorhisttocolorhash(chv, colors);
        ppm_freecolorhist( chv );

        /* convert image */
        if( colors <= 16 )
            ppm_to_16col_pcx(pixels, cols, rows, cmap, colors, cht);
        else
            ppm_to_256col_pcx(pixels, cols, rows, cmap, colors, cht);
    }
    exit(0);
}


static void
ppm_to_16col_pcx(pixels, cols, rows, cmap, colors, cht)
    pixel **pixels;
    int cols, rows;
    pixel *cmap;
    int colors;
    colorhash_table cht;
{
    int Planes, BytesPerLine, BitsPerPixel;
    unsigned char *rawrow, *planesrow;
    register int col, row;

#ifdef DEBUG
    pm_message("ppm -> %d colors", colors);
#endif

    if( packbits ) {
        Planes = 1;
        if( colors > 4 )        BitsPerPixel = 4;
        else if( colors > 2 )   BitsPerPixel = 2;
        else                    BitsPerPixel = 1;
    }
    else {
        BitsPerPixel = 1;
        if( colors > 8 )        Planes = 4;
        else if( colors > 4 )   Planes = 3;
        else if( colors > 2 )   Planes = 2;
        else                    Planes = 1;
    }

    BytesPerLine    = ((cols * BitsPerPixel) + 7) / 8;
    rawrow = (unsigned char *)pm_allocrow(cols, sizeof(unsigned char));
    planesrow = (unsigned char *)pm_allocrow(BytesPerLine, sizeof(unsigned char));

#ifdef DEBUG
    pm_message("Planes = %d, BitsPerPixel = %d, BytesPerLine = %d",
                Planes, BitsPerPixel, BytesPerLine);
#endif

    write_header(stdout, cols, rows, BitsPerPixel, Planes, cmap);
    for( row = 0; row < rows; row++ ) {
        register pixel *pP = pixels[row];
        for( col = 0; col < cols; col++, pP++ )
            rawrow[col] = (unsigned char)ppm_lookupcolor(cht, pP) & 0x0f;
        if( packbits ) {
            PackBits(rawrow, cols, planesrow, BitsPerPixel);
            PCXEncode(stdout, planesrow, BytesPerLine);
        }
        else {
            for( col = 0; col < Planes; col++ ) {
                ToPlanes(rawrow, cols, planesrow, col);
                PCXEncode(stdout, planesrow, BytesPerLine);
            }
        }
    }
#ifdef DEBUG
    pm_message("done!");
#endif
    pm_freerow(planesrow);
    pm_freerow(rawrow);
}


static void
ppm_to_256col_pcx(pixels, cols, rows, cmap, colors, cht)
    pixel **pixels;
    int cols, rows;
    pixel *cmap;
    int colors;
    colorhash_table cht;
{
    register int col, row, i;
    unsigned char *rawrow;

#ifdef DEBUG
    pm_message("ppm -> 256 color, writing index array...");
#endif

    rawrow = (unsigned char *)pm_allocrow(cols, sizeof(unsigned char));

    /* 8 bits per pixel, 1 plane */
    write_header(stdout, cols, rows, 8, 1, (pixel *)NULL);
    for( row = 0; row < rows; row++ ) {
        register pixel *pP = pixels[row];
        for( col = 0; col < cols; col++, pP++ )
            rawrow[col] = ppm_lookupcolor(cht, pP);
        PCXEncode(stdout, rawrow, cols);
    }
#ifdef DEBUG
    pm_message("ok, writing colormap...");
#endif
    Putbyte(PCX_256_COLORS, stdout);
    for( i = 0; i < MAXCOLORS; i++ ) {
        Putbyte(PPM_GETR(cmap[i]), stdout);
        Putbyte(PPM_GETG(cmap[i]), stdout);
        Putbyte(PPM_GETB(cmap[i]), stdout);
    }
#ifdef DEBUG
    pm_message("done!");
#endif
    pm_freerow(rawrow);
}


static void
ppm_to_truecol_pcx(pixels, cols, rows, maxval)
    pixel **pixels;
    int cols, rows, maxval;
{
    unsigned char *redrow, *greenrow, *bluerow;
    register int col, row;

#ifdef DEBUG
    pm_message("ppm -> 24bit");
#endif

    redrow   = (unsigned char *)pm_allocrow(cols, sizeof(unsigned char));
    greenrow = (unsigned char *)pm_allocrow(cols, sizeof(unsigned char));
    bluerow  = (unsigned char *)pm_allocrow(cols, sizeof(unsigned char));

    /* 8 bits per pixel, 3 planes */
    write_header(stdout, cols, rows, 8, 3, (pixel *)NULL);
    for( row = 0; row < rows; row++ ) {
        register pixel *pP = pixels[row];
        for( col = 0; col < cols; col++, pP++ ) {
            if( maxval != PCX_MAXVAL ) {
                redrow[col]   = (long)PPM_GETR(*pP) * PCX_MAXVAL / maxval;
                greenrow[col] = (long)PPM_GETG(*pP) * PCX_MAXVAL / maxval;
                bluerow[col]  = (long)PPM_GETB(*pP) * PCX_MAXVAL / maxval;
            }
            else {
                redrow[col]   = PPM_GETR(*pP);
                greenrow[col] = PPM_GETG(*pP);
                bluerow[col]  = PPM_GETB(*pP);
            }
        }
        PCXEncode(stdout, redrow, cols);
        PCXEncode(stdout, greenrow, cols);
        PCXEncode(stdout, bluerow, cols);
    }
#ifdef DEBUG
    pm_message("done!");
#endif
    pm_freerow(bluerow);
    pm_freerow(greenrow);
    pm_freerow(redrow);
}


static void
write_header(fp, cols, rows, BitsPerPixel, Planes, cmap16)
    FILE *fp;
    int cols, rows;
    int BitsPerPixel, Planes;
    pixel *cmap16;
{
    int i, BytesPerLine;

    Putbyte( PCX_MAGIC, fp);        /* .PCX magic number            */
    Putbyte( 0x05, fp);             /* PC Paintbrush version        */
    Putbyte( 0x01, fp);             /* .PCX run length encoding     */
    Putbyte( BitsPerPixel, fp);     /* bits per pixel               */

    Putword( 0, fp );               /* x1   - image left            */
    Putword( 0, fp );               /* y1   - image top             */
    Putword( cols-1, fp );          /* x2   - image right           */
    Putword( rows-1, fp );          /* y2   - image bottom          */

    Putword( cols, fp );            /* horizontal resolution        */
    Putword( rows, fp );            /* vertical resolution          */

    /* Write out the Color Map for images with 16 colors or less */
    if( cmap16 )
        for (i = 0; i < 16; ++i) {
            Putbyte( PPM_GETR(cmap16[i]), fp );
            Putbyte( PPM_GETG(cmap16[i]), fp );
            Putbyte( PPM_GETB(cmap16[i]), fp );
        }
    else
        for (i = 0; i < 16; ++i) {
            Putbyte( 0, fp );
            Putbyte( 0, fp );
            Putbyte( 0, fp );
        }

    Putbyte( 0, fp);                /* reserved byte                */
    Putbyte( Planes, fp);           /* number of color planes       */

    BytesPerLine    = ((cols * BitsPerPixel) + 7) / 8;
    Putword( BytesPerLine, fp );    /* number of bytes per scanline */

    Putword( 1, fp);                /* pallette info                */

    for (i = 0; i < 58; ++i)        /* fill to end of header        */
        Putbyte( 0, fp );
}


static void
PCXEncode(fp, buf, Size)
FILE            *fp;
unsigned char   *buf;
int             Size;
{
    unsigned char   *end;
    int c, previous, count;

    end = buf + Size;
    previous = *buf++;
    count    = 1;

    while (buf < end) {
        c = *buf++;
        if (c == previous && count < 63) {
            ++count;
            continue;
        }

        if (count > 1 || (previous & 0xc0) == 0xc0) {
            count |= 0xc0;
            Putbyte ( count , fp );
        }
        Putbyte(previous, fp);
        previous = c;
        count   = 1;
    }

    if (count > 1 || (previous & 0xc0) == 0xc0) {
        count |= 0xc0;
        Putbyte ( count , fp );
    }
    Putbyte(previous, fp);
}


static void
PackBits(rawrow, width, buf, bits)
    unsigned char *rawrow;
    int width;
    unsigned char *buf;
    int bits;
{
    register int x, i, shift;

    shift = i = -1;
    for( x = 0; x < width; x++ ) {
        if( shift < 0 ) {
            shift = 8-bits;
            buf[++i] = 0;
        }

        buf[i] |= (rawrow[x] << shift);
        shift -= bits;
    }
}


static const unsigned char bitmask[] = {1, 2, 4, 8, 16, 32, 64, 128};


static void
ToPlanes(rawrow, width, buf, plane)
    unsigned char *rawrow;
    int width;
    unsigned char *buf;
    int plane;
{
    int cbit, x, mask;
    unsigned char *cp = buf-1;

    mask = 1 << plane;
    cbit = -1;
    for( x = 0; x < width; x++ ) {
        if( cbit < 0 ) {
            cbit = 7;
            *++cp = 0;
        }
        if( rawrow[x] & mask )
            *cp |= bitmask[cbit];
        --cbit;
    }
}


/*
 * Write out a word to the PCX file
 */
static void
Putword( w, fp )
int w;
FILE *fp;
{
    Putbyte( w & 0xff, fp );
    Putbyte( (w / 256) & 0xff, fp );
}

/*
 * Write out a byte to the PCX file
 */
static void
Putbyte( b, fp )
int b;
FILE *fp;
{
    if( fputc( b & 0xff, fp ) == EOF )
        pm_error("write error");
}

