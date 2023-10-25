/*\
 * $Id: ppmtobmp.c,v 1.1.1.1 2001/01/05 06:15:55 legalize Exp $
 *
 * ppmtobmp.c - Converts from a PPM file to a Microsoft Windows or OS/2
 * .BMP file.
 *
 * The current implementation is probably not complete, but it works for
 * me.  I welcome feedback.
 *
 * Copyright (C) 1992 by David W. Sanderson.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  This software is provided "as is"
 * without express or implied warranty.
 *
 * $Log: ppmtobmp.c,v $
 * Revision 1.1.1.1  2001/01/05  06:15:55  legalize
 * Initial checkin
 *
 * Revision 1.9  1992/11/24  19:39:33  dws
 * Added copyright.
 *
 * Revision 1.8  1992/11/17  02:16:52  dws
 * Moved length functions to bmp.h.
 *
 * Revision 1.7  1992/11/11  23:18:16  dws
 * Modified to adjust the bits per pixel to 1, 4, or 8.
 *
 * Revision 1.6  1992/11/11  22:43:39  dws
 * Commented out a superfluous message.
 *
 * Revision 1.5  1992/11/11  05:58:06  dws
 * First version that works.
 *
 * Revision 1.4  1992/11/11  03:40:32  dws
 * Moved calculation of bits per pixel to BMPEncode.
 *
 * Revision 1.3  1992/11/11  03:02:34  dws
 * Added BMPEncode function.
 *
 * Revision 1.2  1992/11/08  01:44:35  dws
 * Added option processing and reading of PPM file.
 *
 * Revision 1.1  1992/11/08  00:46:07  dws
 * Initial revision
\*/

#include        "bmp.h"
#include        "ppm.h"
#include        "ppmcmap.h"
#include        "bitio.h"

#define MAXCOLORS 256

/*
 * Utilities
 */

static char     er_write[] = "stdout: write error";

/* prototypes */
static void PutByte ARGS((FILE *fp, char v));
static void PutShort ARGS((FILE *fp, short v));
static void PutLong ARGS((FILE *fp, long v));
static int BMPwritefileheader ARGS((FILE *fp, int class, unsigned long bitcount,
    unsigned long x, unsigned long y));
static int BMPwriteinfoheader ARGS((FILE *fp, int class, unsigned long bitcount,
    unsigned long x, unsigned long y));
static int BMPwritergb ARGS((FILE *fp, int class, pixval R, pixval G, pixval B));
static int BMPwritergbtable ARGS((FILE *fp, int class, int bpp, int colors,
    pixval *R, pixval *G, pixval *B));
static int BMPwriterow ARGS((FILE *fp, pixel *row, unsigned long cx,
    unsigned short bpp, colorhash_table cht));
static int BMPwritebits ARGS((FILE *fp, unsigned long cx, unsigned long cy,
    unsigned short cBitCount, pixel **pixels, colorhash_table cht));
static int colorstobpp ARGS((int colors));
static void BMPEncode ARGS((FILE *fp, int class, int x, int y, pixel **pixels,
    int colors, colorhash_table cht, pixval *R, pixval *G, pixval *B));
static void
PutByte(fp, v)
        FILE           *fp;
        char            v;
{
        if (putc(v, fp) == EOF)
        {
                pm_error(er_write);
        }
}

static void
PutShort(fp, v)
        FILE           *fp;
        short           v;
{
        if (pm_writelittleshort(fp, v) == -1)
        {
                pm_error(er_write);
        }
}

static void
PutLong(fp, v)
        FILE           *fp;
        long            v;
{
        if (pm_writelittlelong(fp, v) == -1)
        {
                pm_error(er_write);
        }
}

/*
 * BMP writing
 */

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritefileheader(fp, class, bitcount, x, y)
        FILE           *fp;
        int             class;
        unsigned long   bitcount;
        unsigned long   x;
        unsigned long   y;
{
        PutByte(fp, 'B');
        PutByte(fp, 'M');

        /* cbSize */
        PutLong(fp, BMPlenfile(class, bitcount, x, y));

        /* xHotSpot */
        PutShort(fp, 0);

        /* yHotSpot */
        PutShort(fp, 0);

        /* offBits */
        PutLong(fp, BMPoffbits(class, bitcount));

        return 14;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwriteinfoheader(fp, class, bitcount, x, y)
        FILE           *fp;
        int             class;
        unsigned long   bitcount;
        unsigned long   x;
        unsigned long   y;
{
        long    cbFix;

        /* cbFix */
        switch (class)
        {
        case C_WIN:
                cbFix = 40;
                PutLong(fp, cbFix);

                /* cx */
                PutLong(fp, x);
                /* cy */
                PutLong(fp, y);
                /* cPlanes */
                PutShort(fp, 1);
                /* cBitCount */
                PutShort(fp, bitcount);

                /*
                 * We've written 16 bytes so far, need to write 24 more
                 * for the required total of 40.
                 */

                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);
                PutLong(fp, 0);


                break;
        case C_OS2:
                cbFix = 12;
                PutLong(fp, cbFix);

                /* cx */
                PutShort(fp, x);
                /* cy */
                PutShort(fp, y);
                /* cPlanes */
                PutShort(fp, 1);
                /* cBitCount */
                PutShort(fp, bitcount);

                break;
        default:
                pm_error(er_internal, "BMPwriteinfoheader");
        }

        return cbFix;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritergb(fp,class,R,G,B)
        FILE           *fp;
        int             class;
        pixval          R;
        pixval          G;
        pixval          B;
{
        switch (class)
        {
        case C_WIN:
                PutByte(fp, B);
                PutByte(fp, G);
                PutByte(fp, R);
                PutByte(fp, 0);
                return 4;
        case C_OS2:
                PutByte(fp, B);
                PutByte(fp, G);
                PutByte(fp, R);
                return 3;
        default:
                pm_error(er_internal, "BMPwritergb");
        }
        return -1;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritergbtable(fp,class,bpp,colors,R,G,B)
        FILE           *fp;
        int             class;
        int             bpp;
        int             colors;
        pixval         *R;
        pixval         *G;
        pixval         *B;
{
        int             nbyte = 0;
        int             i;
        long            ncolors;

        for (i = 0; i < colors; i++)
        {
                nbyte += BMPwritergb(fp,class,R[i],G[i],B[i]);
        }

        ncolors = (1 << bpp);

        for (; i < ncolors; i++)
        {
                nbyte += BMPwritergb(fp,class,0,0,0);
        }

        return nbyte;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwriterow(fp, row, cx, bpp, cht)
        FILE           *fp;
        pixel          *row;
        unsigned long   cx;
        unsigned short  bpp;
        colorhash_table cht;
{
        BITSTREAM       b;
        unsigned        nbyte = 0;
        int             rc;
        unsigned        x;

        if ((b = pm_bitinit(fp, "w")) == (BITSTREAM) 0)
        {
                return -1;
        }

        for (x = 0; x < cx; x++, row++)
        {
                if ((rc = pm_bitwrite(b, bpp, ppm_lookupcolor(cht, row))) == -1)
                {
                        return -1;
                }
                nbyte += rc;
        }

        if ((rc = pm_bitfini(b)) == -1)
        {
                return -1;
        }
        nbyte += rc;

        /*
         * Make sure we write a multiple of 4 bytes.
         */
        while (nbyte % 4)
        {
                PutByte(fp, 0);
                nbyte++;
        }

        return nbyte;
}

/*
 * returns the number of bytes written, or -1 on error.
 */
static int
BMPwritebits(fp, cx, cy, cBitCount, pixels, cht)
        FILE           *fp;
        unsigned long   cx;
        unsigned long   cy;
        unsigned short  cBitCount;
        pixel         **pixels;
        colorhash_table cht;
{
        int             nbyte = 0;
        long            y;

        if(cBitCount > 24)
        {
                pm_error("cannot handle cBitCount: %d"
                         ,cBitCount);
        }

        /*
         * The picture is stored bottom line first, top line last
         */

        for (y = cy - 1; y >= 0; y--)
        {
                int rc;
                rc = BMPwriterow(fp, pixels[y], cx, cBitCount, cht);

                if(rc == -1)
                {
                        pm_error("couldn't write row %d"
                                 ,y);
                }
                if(rc%4)
                {
                        pm_error("row had bad number of bytes: %d"
                                 ,rc);
                }
                nbyte += rc;
        }

        return nbyte;
}

/*
 * Return the number of bits per pixel required to represent the
 * given number of colors.
 */

static int
colorstobpp(colors)
        int             colors;
{
        int             bpp;

        if (colors < 1)
        {
                pm_error("can't have less than one color");
        }

        if ((bpp = pm_maxvaltobits(colors - 1)) > 8)
        {
                pm_error("can't happen");
        }

        return bpp;
}

/*
 * Write a BMP file of the given class.
 *
 * Note that we must have 'colors' in order to know exactly how many
 * colors are in the R, G, B, arrays.  Entries beyond those in the
 * arrays are undefined.
 */
static void
BMPEncode(fp, class, x, y, pixels, colors, cht, R, G, B)
        FILE           *fp;
        int             class;
        int             x;
        int             y;
        pixel         **pixels;
        int             colors; /* number of valid entries in R,G,B */
        colorhash_table cht;
        pixval         *R;
        pixval         *G;
        pixval         *B;
{
        int             bpp;    /* bits per pixel */
        unsigned long   nbyte = 0;

        bpp = colorstobpp(colors);

        /*
         * I have found empirically at least one BMP-displaying program
         * that can't deal with (for instance) using 3 bits per pixel.
         * I have seen no programs that can deal with using 3 bits per
         * pixel.  I have seen programs which can deal with 1, 4, and
         * 8 bits per pixel.
         *
         * Based on this, I adjust actual the number of bits per pixel
         * as follows.  If anyone knows better, PLEASE tell me!
         */
        switch(bpp)
        {
        case 2:
        case 3:
                bpp = 4;
                break;
        case 5:
        case 6:
        case 7:
                bpp = 8;
                break;
        }

        pm_message("Using %d bits per pixel", bpp);

        nbyte += BMPwritefileheader(fp, class, bpp, x, y);
        nbyte += BMPwriteinfoheader(fp, class, bpp, x, y);
        nbyte += BMPwritergbtable(fp, class, bpp, colors, R, G, B);

        if(nbyte !=     ( BMPlenfileheader(class)
                        + BMPleninfoheader(class)
                        + BMPlenrgbtable(class, bpp)))
        {
                pm_error(er_internal, "BMPEncode");
        }

        nbyte += BMPwritebits(fp, x, y, bpp, pixels, cht);
        if(nbyte != BMPlenfile(class, bpp, x, y))
        {
                pm_error(er_internal, "BMPEncode");
        }
}

#if 0
report(class, bitcount, x, y)
        int             class;
        unsigned long   bitcount;
        unsigned long   x;
        unsigned long   y;
{
        char *name;
        switch (class)
        {
        case C_WIN:
                name = "Win";
                break;
        case C_OS2:
                name = "OS/2";
                break;
        default:
                pm_error(er_internal, "report");
                return 0;
        }

        pm_message("For class %s, bitcount %d, x %d, y %d:"
                , name
                , bitcount
                , x
                , y);
        pm_message("\tlenrgbtable: %d"
                , BMPlenrgbtable(class, bitcount));
        pm_message("\tlenline:       %d"
                , BMPlenline(class, bitcount, x));
        pm_message("\tlenbits:       %d"
                , BMPlenbits(class, bitcount, x, y));
        pm_message("\toffbits:       %d"
                , BMPoffbits(class, bitcount));
        pm_message("\tlenfile:       %d"
                , BMPlenfile(class, bitcount, x, y));
}

int
main(ac, av)
    int             ac;
    char          **av;
{
    ppm_init(&ac, av);

    if(ac != 5)
    {
	pm_message("usage: ppmtobmp class bitcount x y");
	exit(1);
    }

    report(atoi(av[1]), atoi(av[2]), atoi(av[3]), atoi(av[4]));

    exit(0);
}
#endif

int
main(argc, argv)
        int             argc;
        char          **argv;
{
        FILE           *ifp = stdin;
        char           *usage = "[-windows] [-os2] [ppmfile]";
        int             class = C_OS2;

        int             argn;
        int             rows;
        int             cols;
        int             colors;
        int             i;
        pixval          maxval;
        colorhist_vector chv;
        pixval          Red[MAXCOLORS];
        pixval          Green[MAXCOLORS];
        pixval          Blue[MAXCOLORS];

        pixel** pixels;
        colorhash_table cht;


        ppm_init(&argc, argv);

        argn = 1;

        while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0')
        {
                if (pm_keymatch(argv[argn], "-windows", 2))
                        class = C_WIN;
                else if (pm_keymatch(argv[argn], "-os2", 2))
                        class = C_OS2;
                else
                        pm_usage(usage);
                ++argn;
        }

        if (argn < argc)
        {
                ifp = pm_openr(argv[argn]);
                ++argn;
        }

        if (argn != argc)
        {
                pm_usage(usage);
        }

        pixels = ppm_readppm(ifp, &cols, &rows, &maxval);

        pm_close(ifp);

#if 0
        {
                char *name;
                switch (class)
                {
                case C_WIN:
                        name = "a Windows";
                        break;
                case C_OS2:
                        name = "an OS/2";
                        break;
                default:
                        pm_error(er_internal, "report");
                        break;
                }
                pm_message("generating %s BMP file", name);
        }
#endif

        /* Figure out the colormap. */
        pm_message("computing colormap...");
        chv = ppm_computecolorhist(pixels, cols, rows, MAXCOLORS, &colors);
        if (chv == (colorhist_vector) 0)
                pm_error("too many colors - try doing a 'ppmquant %d'"
                        , MAXCOLORS);
        pm_message("%d colors found", colors);

        /*
         * Now turn the ppm colormap into the appropriate GIF
         * colormap.
         */
        if (maxval > 255)
        {
                pm_message("maxval is not 255 - automatically rescaling colors");
        }
        for (i = 0; i < colors; ++i)
        {
                if (maxval == 255)
                {
                        Red[i] = PPM_GETR(chv[i].color);
                        Green[i] = PPM_GETG(chv[i].color);
                        Blue[i] = PPM_GETB(chv[i].color);
                }
                else
                {
                        Red[i] = (pixval) PPM_GETR(chv[i].color) * 255 / maxval;
                        Green[i] = (pixval) PPM_GETG(chv[i].color) * 255 / maxval;
                        Blue[i] = (pixval) PPM_GETB(chv[i].color) * 255 / maxval;
                }
        }

        /* And make a hash table for fast lookup. */
        cht = ppm_colorhisttocolorhash(chv, colors);
        ppm_freecolorhist(chv);

        /* All set, let's do it. */
        BMPEncode(stdout, class
                , cols, rows, pixels, colors, cht
                ,Red, Green, Blue);

        pm_close(stdout);

        exit(0);
}
