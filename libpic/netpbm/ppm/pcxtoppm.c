/*
 * pcxtoppm.c - Converts from a PC Paintbrush PCX file to a PPM file.
 *
 * Copyright (c) 1990 by Michael Davidson
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Modifications by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 * 20/Apr/94:
 *  - checks if 16-color-palette is completely black -> use standard palette
 *  - "-stdpalette" option to enforce this
 *  - row-by-row operation (PPM output)
 *  11/Dec/94:
 *  - support for 24bit and 32bit (24bit + 8bit intensity) images
 *  - row-by-row operation (PCX input, for 16-color and truecolor images)
 *  - some code restructuring
 *  15/Feb/95:
 *  - bugfix for 16 color-images: few bytes allocated for rawrow in some cases
 *  - added sanity checks for cols<->BytesPerLine
 *  17/Jul/95:
 *  - moved check of 16-color-palette into pcx_16col_to_ppm(),
 *    now checks if it contains only a single color
 */
#include        "ppm.h"

#define DEBUG

#define PCX_MAGIC       0x0a            /* PCX magic number             */
#define PCX_HDR_SIZE    128             /* size of PCX header           */
#define PCX_256_COLORS  0x0c            /* magic number for 256 colors  */

#define PCX_MAXVAL      (pixval)255

/* prototypes */
static void pcx_16col_to_ppm ARGS((FILE *ifp, int cols, int rows,
                int BytesPerLine, int BitsPerPixel, int Planes, pixel *cmap));
static void pcx_256col_to_ppm ARGS((FILE *ifp, int cols, int rows, int BytesPerLine));
static void pcx_truecol_to_ppm ARGS((FILE *ifp, int cols, int rows, int BytesPerLine, int Planes));
static void GetPCXRow ARGS((FILE *ifp, unsigned char *pcxrow, int bytesperline));
static void pcx_planes_to_pixels ARGS(( unsigned char *pixels, unsigned char *bitplanes, int bytesperline, int planes, int bitsperpixel ));
static void pcx_unpack_pixels ARGS(( unsigned char *pixels, unsigned char *bitplanes, int bytesperline, int planes, int bitsperpixel ));
static int GetByte ARGS(( FILE *fp ));
static int GetWord ARGS(( FILE *fp ));

/* standard palette */
static unsigned char StdRed[]   = { 0, 255,   0,   0, 170, 170, 170, 170, 85,  85,  85,  85, 255, 255, 255, 255 };
static unsigned char StdGreen[] = { 0, 255, 170, 170,   0,   0, 170, 170, 85,  85, 255, 255,  85,  85, 255, 255 };
static unsigned char StdBlue[]  = { 0, 255,   0, 170,   0, 170,   0, 170, 85, 255,  85, 255,  85, 255,  85, 255 };


int
main(argc, argv)
    int         argc;
    char        *argv[];
{
    register int  i;
    FILE *ifp;
    int Version, Xmin, Ymin, Xmax, Ymax, Width, Height, Encoding;
    int Planes, BitsPerPixel, BytesPerLine, PaletteInfo;
    pixel cmap16[16];
    int argn;
    short use_std_palette = 0;
    char *usage = "[-stdpalette] [pcxfile]";

    ppm_init( &argc, argv );

    argn = 1;
    while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
        if( pm_keymatch(argv[argn], "-stdpalette", 2) ) {
            use_std_palette = 1;
        }
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

    /*
     * read the PCX header
     */
    if (GetByte(ifp) != PCX_MAGIC)
         pm_error("bad magic number - not a PCX file");

    Version = GetByte(ifp);  /* get version # */

    Encoding = GetByte(ifp);
    if( Encoding != 1 )       /* check for PCX run length encoding   */
         pm_error("unknown encoding scheme: %d", Encoding);

    BitsPerPixel= GetByte(ifp);
    Xmin        = GetWord(ifp);
    Ymin        = GetWord(ifp);
    Xmax        = GetWord(ifp);
    Ymax        = GetWord(ifp);

    Width       = (Xmax - Xmin) + 1;
    Height      = (Ymax - Ymin) + 1;

    (void) GetWord(ifp);                /* ignore horizontal resolution */
    (void) GetWord(ifp);                /* ignore vertical resolution   */

    /*
     * get the 16-color color map
     */
    for (i = 0; i < 16; i++) {
        int r, g, b;
        r = GetByte(ifp);
        g = GetByte(ifp);
        b = GetByte(ifp);
        PPM_ASSIGN(cmap16[i], r, g, b);
    }

    (void) GetByte(ifp);                /* skip reserved byte       */
    Planes      = GetByte(ifp);         /* # of color planes        */
    BytesPerLine= GetWord(ifp);         /* # of bytes per line      */
    PaletteInfo = GetWord(ifp);         /* palette info (ignored)   */

#ifdef DEBUG
    pm_message("Version: %d", Version);
    pm_message("BitsPerPixel: %d", BitsPerPixel);
    pm_message("Xmin: %d   Ymin: %d   Xmax: %d   Ymax: %d", Xmin, Ymin, Xmax, Ymax);
    pm_message("Planes: %d    BytesPerLine: %d    PaletteInfo: %d", Planes, BytesPerLine, PaletteInfo);
#endif

    if( fseek(ifp, (long)PCX_HDR_SIZE, 0) != 0 )
        pm_error("error seeking past header");

    if( use_std_palette ) {
        for( i = 0; i < 16; i++ )
            PPM_ASSIGN(cmap16[i], StdRed[i], StdGreen[i], StdBlue[i]);
    }

    ppm_writeppminit(stdout, Width, Height, PCX_MAXVAL, 0);
    switch (BitsPerPixel) {
        case 1:
            if( Planes >= 1 && Planes <= 4 )
                pcx_16col_to_ppm(ifp, Width, Height, BytesPerLine, BitsPerPixel, Planes, cmap16);
            else
                goto fail;
            break;
        case 2:
        case 4:
            if( Planes == 1 )
                pcx_16col_to_ppm(ifp, Width, Height, BytesPerLine, BitsPerPixel, Planes, cmap16);
            else
                goto fail;
            break;
        case 8:
            switch( Planes ) {
                case 1:
                    pcx_256col_to_ppm(ifp, Width, Height, BytesPerLine);
                    break;
                case 3:
                case 4:
                    pcx_truecol_to_ppm(ifp, Width, Height, BytesPerLine, Planes);
                    break;
                default:
                    goto fail;
            }
            break;
        default:
fail:
            pm_error("can't handle %d bits per pixel image with %d planes",
                        BitsPerPixel,Planes);
    }
    pm_close(ifp);
    exit(0);
}


static void
pcx_16col_to_ppm(ifp, cols, rows, BytesPerLine, BitsPerPixel, Planes, cmap)
    FILE *ifp;
    int cols, rows;
    int BytesPerLine, BitsPerPixel, Planes;
    pixel *cmap;    /* colormap from header */
{
    int row, col, rawcols, colors;
    unsigned char *pcxrow, *rawrow;
    pixel *ppmrow;
    short palette_ok = 0;

#ifdef DEBUG
    pm_message("16 color -> ppm");
#endif

    /* check if palette is ok  */
    colors = (1 << BitsPerPixel) * (1 << Planes);
    for( col = 0; col < colors-1; col++ ) {
        if( !PPM_EQUAL(cmap[col], cmap[col+1]) ) {
            palette_ok = 1;
            break;
        }
    }
    if( !palette_ok ) {
        pm_message("warning - useless header palette, using builtin standard palette");
        for( col = 0; col < colors; col++ )
            PPM_ASSIGN(cmap[col], StdRed[col], StdGreen[col], StdBlue[col]);
    }

    /*  BytesPerLine should be >= BitsPerPixel * cols / 8  */
    rawcols = BytesPerLine * 8 / BitsPerPixel;
    if( cols > rawcols ) {
        pm_message("warning - BytesPerLine = %d, truncating image to %d pixels",
                    BytesPerLine, rawcols);
        cols = rawcols;
    }
    pcxrow = (unsigned char *)pm_allocrow(Planes * BytesPerLine, sizeof(unsigned char));
    rawrow = (unsigned char *)pm_allocrow(rawcols, sizeof(unsigned char));
    ppmrow = ppm_allocrow(cols);

    for( row = 0; row < rows; row++ ) {
        GetPCXRow(ifp, pcxrow, Planes * BytesPerLine);

        if (Planes == 1)
            pcx_unpack_pixels(rawrow, pcxrow, BytesPerLine, Planes, BitsPerPixel);
        else
            pcx_planes_to_pixels(rawrow, pcxrow, BytesPerLine, Planes, BitsPerPixel);

        for( col = 0; col < cols; col++ )
            ppmrow[col] = cmap[rawrow[col]];
        ppm_writeppmrow(stdout, ppmrow, cols, PCX_MAXVAL, 0);
    }

#ifdef DEBUG
    pm_message("done!");
#endif
    ppm_freerow(ppmrow);
    pm_freerow(rawrow);
    pm_freerow(pcxrow);
}


static void
pcx_256col_to_ppm(ifp, cols, rows, BytesPerLine)
    FILE *ifp;
    int cols, rows;
    int BytesPerLine;
{
    pixel colormap[256];
    pixel *ppmrow;
    unsigned char **image;
    int row, col;

#ifdef DEBUG
    pm_message("256 color -> ppm, reading index array...");
#endif

    if( cols > BytesPerLine ) {
        pm_message("warning - BytesPerLine = %d, truncating image to %d pixels",
                    BytesPerLine,  BytesPerLine);
        cols = BytesPerLine;
    }

    image = (unsigned char **)pm_allocarray(BytesPerLine, rows, sizeof(unsigned char));
    for( row = 0; row < rows; row++ )
        GetPCXRow(ifp, image[row], BytesPerLine);

#ifdef DEBUG
    pm_message("ok, now reading colormap...");
#endif

    /*
     * 256 color images have their color map at the end of the file
     * preceeded by a magic byte
     */
    if (GetByte(ifp) != PCX_256_COLORS)
        pm_error("bad color map signature" );
    for( col = 0; col < 256; col++ ) {
        int r, g, b;
        r = GetByte(ifp);
        g = GetByte(ifp);
        b = GetByte(ifp);
        PPM_ASSIGN(colormap[col], r, g, b);
    }

#ifdef DEBUG
    pm_message("ok, converting...");
#endif

    ppmrow = ppm_allocrow(cols);
    for( row = 0; row < rows; row++ ) {
        for( col = 0; col < cols; col++ )
            ppmrow[col] = colormap[image[row][col]];
        ppm_writeppmrow(stdout, ppmrow, cols, PCX_MAXVAL, 0);
    }
#ifdef DEBUG
    pm_message("done!");
#endif

    ppm_freerow(ppmrow);
    pm_freearray(image, rows);
}


static void
pcx_truecol_to_ppm(ifp, cols, rows, BytesPerLine, Planes)
    FILE *ifp;
    int cols, rows;
    int BytesPerLine, Planes;
{
    unsigned char *redrow, *greenrow, *bluerow, *intensityrow;
    pixel *ppmrow;
    int row, col;
    int r, g, b, i;

#ifdef DEBUG
    pm_message("%dbit truecolor -> ppm", Planes * 8);
#endif

    if( cols > BytesPerLine ) {
        pm_message("warning - BytesPerLine = %d, truncating image to %d pixels",
                    BytesPerLine,  BytesPerLine);
        cols = BytesPerLine;
    }

    redrow       = (unsigned char *)pm_allocrow(BytesPerLine, sizeof(unsigned char*));
    greenrow     = (unsigned char *)pm_allocrow(BytesPerLine, sizeof(unsigned char*));
    bluerow      = (unsigned char *)pm_allocrow(BytesPerLine, sizeof(unsigned char*));
    if( Planes == 4 )
        intensityrow = (unsigned char *)pm_allocrow(BytesPerLine, sizeof(unsigned char*));
    else
        intensityrow = (unsigned char *)NULL;

    ppmrow = ppm_allocrow(cols);
    for( row = 0; row < rows; row++ ) {
        GetPCXRow(ifp, redrow, BytesPerLine);
        GetPCXRow(ifp, greenrow, BytesPerLine);
        GetPCXRow(ifp, bluerow, BytesPerLine);
        if( intensityrow )
            GetPCXRow(ifp, intensityrow, BytesPerLine);
        for( col = 0; col < cols; col++ ) {
            r = redrow[col];
            g = greenrow[col];
            b = bluerow[col];
            if( intensityrow ) {
                i = intensityrow[col];
                r = r * i / 256;
                g = g * i / 256;
                b = b * i / 256;
            }
            PPM_ASSIGN(ppmrow[col], r, g, b);
        }
        ppm_writeppmrow(stdout, ppmrow, cols, PCX_MAXVAL, 0);
    }
#ifdef DEBUG
    pm_message("done!");
#endif

    ppm_freerow(ppmrow);
    if( intensityrow )
        pm_freerow(intensityrow);
    pm_freerow(bluerow);
    pm_freerow(greenrow);
    pm_freerow(redrow);
}


/*
 * convert multi-plane format into 1 pixel per byte
 */
static void
pcx_planes_to_pixels(pixels, bitplanes, bytesperline, planes, bitsperpixel)
unsigned char   *pixels;
unsigned char   *bitplanes;
int             bytesperline;
int             planes;
int             bitsperpixel;
{
    int  i, j;
    int  npixels;
    unsigned char    *p;

    if (planes > 4)
         pm_error("can't handle more than 4 planes" );
    if (bitsperpixel != 1)
         pm_error("can't handle more than 1 bit per pixel" );

    /*
     * clear the pixel buffer
     */
    npixels = (bytesperline * 8) / bitsperpixel;
    p    = pixels;
    while (--npixels >= 0)
         *p++ = 0;

    /*
     * do the format conversion
     */
    for (i = 0; i < planes; i++)
    {
         int pixbit, bits, mask;

         p    = pixels;
         pixbit    = (1 << i);
         for (j = 0; j < bytesperline; j++)
         {
             bits = *bitplanes++;
             for (mask = 0x80; mask != 0; mask >>= 1, p++)
                 if (bits & mask)
                     *p |= pixbit;
         }
     }
}

/*
 * convert packed pixel format into 1 pixel per byte
 */
static void
pcx_unpack_pixels(pixels, bitplanes, bytesperline, planes, bitsperpixel)
unsigned char   *pixels;
unsigned char   *bitplanes;
int             bytesperline;
int             planes;
int             bitsperpixel;
{
    register int        bits;

    if (planes != 1)
         pm_error("can't handle packed pixels with more than 1 plane" );
#if 0
    if (bitsperpixel == 8)
    {
        while (--bytesperline >= 0)
            *pixels++ = *bitplanes++;
    }
    else
#endif
    if (bitsperpixel == 4)
    {
        while (--bytesperline >= 0)
        {
            bits        = *bitplanes++;
            *pixels++   = (bits >> 4) & 0x0f;
            *pixels++   = (bits     ) & 0x0f;
        }
    }
    else if (bitsperpixel == 2)
    {
        while (--bytesperline >= 0)
        {
            bits        = *bitplanes++;
            *pixels++   = (bits >> 6) & 0x03;
            *pixels++   = (bits >> 4) & 0x03;
            *pixels++   = (bits >> 2) & 0x03;
            *pixels++   = (bits     ) & 0x03;
        }
    }
    else if (bitsperpixel == 1)
    {
        while (--bytesperline >= 0)
        {
            bits        = *bitplanes++;
            *pixels++   = ((bits & 0x80) != 0);
            *pixels++   = ((bits & 0x40) != 0);
            *pixels++   = ((bits & 0x20) != 0);
            *pixels++   = ((bits & 0x10) != 0);
            *pixels++   = ((bits & 0x08) != 0);
            *pixels++   = ((bits & 0x04) != 0);
            *pixels++   = ((bits & 0x02) != 0);
            *pixels++   = ((bits & 0x01) != 0);
        }
    }
    else
        pm_error("pcx_unpack_pixels - can't handle %d bits per pixel", bitsperpixel);
}

static int
GetByte(fp)
FILE    *fp;
{
    int    c;

    if ((c = fgetc(fp)) == EOF)
         pm_error("unexpected end of file" );

    return c;
}

static int
GetWord(fp)
FILE    *fp;
{
    int c;
    c  = GetByte(fp);
    c |= (GetByte(fp) << 8);
    return c;
}


/*
 *  read a single row encoded row, handles encoding across rows
 */
static void
GetPCXRow(ifp, pcxrow, bytesperline)
    FILE *ifp;
    unsigned char *pcxrow;
    int bytesperline;
{
    static int count = 0;
    static int c;
    int i;

    i = 0;
    while( i < bytesperline ) {
        if( count ) {
            pcxrow[i++] = c;
            --count;
        }
        else {
            c = GetByte(ifp);
            if( (c & 0xc0) != 0xc0 )
                pcxrow[i++] = c;
            else {
                count = c & 0x3f;
                c = GetByte(ifp);
            }
        }
    }
}

