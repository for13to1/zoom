/* ilbmtoppm.c - read an IFF ILBM file and produce a portable pixmap
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** Modified by Mark Thompson on 10/4/90 to accomodate 24 bit IFF files
** as used by ASDG, NewTek, etc.
**
** Modified by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**  20/Jun/93:
**  - row-by-row operation
**  - better de-interleave algorithm
**  - colormap files
**  - direct color
**  04/Oct/93:
**  - multipalette support (PCHG chunk)
**  - options -ignore, -isham, -isehb and -adjustcolors
*/

#include "ppm.h"
#include "ilbm.h"

/*#define DEBUG*/

/* prototypes */
static void getfourchars ARGS(( FILE* f, char fourchars[4] ));
static unsigned char get_byte ARGS(( FILE* f ));
static long get_big_long ARGS((FILE *f));
static short get_big_short ARGS((FILE *f));
static void readerr ARGS((FILE *f));

static void skip_chunk ARGS((FILE *f, long chunksize));
static void display_chunk ARGS((FILE *ifp, char *iffid, long chunksize));
static pixel * read_colormap ARGS((FILE *f, long colors));
static BitMapHeader * read_bmhd ARGS((FILE *f));
static PCHGInfo *read_pchg ARGS((FILE *ifp, unsigned long chunksize));

static void ham_to_ppm ARGS((FILE *ifp, BitMapHeader *bmhd, pixel *colormap, int colors, PCHGInfo *pchginfo));
static void deep_to_ppm ARGS((FILE *ifp, BitMapHeader *bmhd));
static void cmap_to_ppm ARGS((pixel *colormap, int colors));
static void std_to_ppm ARGS((FILE *ifp, BitMapHeader *bmhd, pixel *colormap, int colors, PCHGInfo *pchginfo, long viewportmodes));
static void direct_to_ppm ARGS((FILE *ifp, BitMapHeader *bmhd, DirectColor *dcol));

static void init_pchg ARGS((PCHGInfo *pchginfo, pixel *colormap, int colors, pixval newmaxval));
static void adjust_colormap ARGS((PCHGInfo *pchginfo, int row));
static void scale_colormap ARGS((pixel *colormap, int colors, pixval oldmaxval, pixval newmaxval));
static pixel * ehb_to_cmap ARGS((pixel *colormap, int *colors));
static void read_ilbm_plane ARGS((FILE *ifp, int cols, int compression));
static void decode_row ARGS((FILE *ifp, rawtype *chunkyrow, int planes, BitMapHeader *bmhd));

static rawtype * alloc_rawrow ARGS((int cols));
static void * xmalloc ARGS((int bytes));
#define MALLOC(n, type)     (type *)xmalloc((n) * sizeof(type))


static short verbose = 0;
static short adjustcolors = 0;
static unsigned char *ilbmrow;
static pixel *pixelrow;


int
main(argc, argv)
    int argc;
    char *argv[];
{
    FILE *ifp;
    pixel *colormap = 0;
    int argn, colors;
    char iffid[5];
    short body = 0;
    long formsize, bytesread, chunksize, viewportmodes = 0, fakeviewport = 0;
    char *usage = "[-verbose] [-ignore <chunkID>] [-isham|-isehb] [-adjustcolors] [ilbmfile]";
    BitMapHeader *bmhd = NULL;
    DirectColor *dcol = NULL;
    PCHGInfo *pchginfo = NULL;
#define MAX_IGNORE  16
    char *ignorelist[MAX_IGNORE];
    int ignorecount = 0;


    ppm_init(&argc, argv);

    argn = 1;
    while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
        if( pm_keymatch(argv[argn], "-verbose", 2) )
            verbose = 1;
        else
        if( pm_keymatch(argv[argn], "-noverbose", 4) )
            verbose = 0;
        else
        if( pm_keymatch(argv[argn], "-isham", 4) )
            fakeviewport |= vmHAM;
        else
        if( pm_keymatch(argv[argn], "-isehb", 4) )
            fakeviewport |= vmEXTRA_HALFBRITE;
        else
        if( pm_keymatch(argv[argn], "-adjustcolors", 2) )
            adjustcolors = 1;
        else
        if( pm_keymatch(argv[argn], "-noadjustcolors", 4) )
            adjustcolors = 0;
        else
        if( pm_keymatch(argv[argn], "-ignore", 2) ) {
            if( ++argn >= argc )
                pm_usage(usage);
            if( strlen(argv[argn]) != 4 )
                pm_error("\"-ignore\" option needs a 4 byte chunk ID string as argument");
            if( ignorecount >= MAX_IGNORE )
                pm_error("max %d chunk IDs to ignore", MAX_IGNORE);
            ignorelist[ignorecount++] = argv[argn];
        }
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

    /* Read in the ILBM file. */
    iffid[4] = '\0';
    getfourchars(ifp, iffid);
    if( strcmp(iffid, "FORM") != 0 )
        pm_error("input is not a FORM type IFF file");
    formsize = get_big_long(ifp);
    getfourchars(ifp, iffid);
    if( strcmp(iffid, "ILBM") != 0 )
        pm_error( "input is not an ILBM type FORM IFF file" );
    bytesread = 4;  /* FORM and formsize do not count */

    /* Main loop, parsing the IFF FORM. */
    while( bytesread < formsize ) {
        short i, ignore = 0;

        getfourchars(ifp, iffid);
        chunksize = get_big_long(ifp);
        bytesread += 8;

        for( i = 0; i < ignorecount && !ignore; i++ ) {
            if( strcmp(ignorelist[i], iffid) == 0 )
                ignore = 1;
        }

        if( ignore ) {
            ignore = 0;
            pm_message("ignoring \"%s\" chunk", iffid);
            skip_chunk(ifp, chunksize);
        }
        else
        if( body != 0 ) {
            pm_message("\"%s\" chunk found after BODY chunk - skipping", iffid);
            skip_chunk(ifp, chunksize);
        }
        else
        if( strcmp(iffid, "BMHD") == 0 ) {
            if( chunksize != BitMapHeaderSize )
                pm_error("BMHD chunk size mismatch");
            bmhd = read_bmhd(ifp);
        }
        else
        if( strcmp(iffid, "CMAP") == 0 ) {
            colors = chunksize / 3;
            if( colors == 0 ) {
                pm_error("warning - empty colormap");
                skip_chunk(ifp, chunksize);
            }
            else {
                long r = 3 * colors;
                colormap = read_colormap(ifp, colors);
                while( r++ < chunksize )
                    (void)get_byte(ifp);
            }
        }
        else
        if( strcmp(iffid, "CAMG") == 0 ) {
            if( chunksize != CAMGChunkSize )
                pm_error("CAMG chunk size mismatch");
            viewportmodes = get_big_long(ifp);
        }
        else
        if( strcmp(iffid, "DCOL") == 0 ) {
            if( chunksize != DirectColorSize )
                pm_error("DCOL chunk size mismatch");
            dcol = MALLOC(1, DirectColor);
            dcol->r = get_byte(ifp);
            dcol->g = get_byte(ifp);
            dcol->b = get_byte(ifp);
            (void) get_byte(ifp);
        }
        else
        if( strcmp(iffid, "PCHG") == 0 ) {
            pchginfo = read_pchg(ifp, chunksize);
        }
        else
        if( strcmp(iffid, "BODY") == 0 ) {
            if( bmhd == NULL )
                pm_error("\"BODY\" chunk without \"BMHD\" chunk");

            ilbmrow = MALLOC(RowBytes(bmhd->w), unsigned char);
            pixelrow = ppm_allocrow(bmhd->w);

            viewportmodes |= fakeviewport;

            if( viewportmodes & vmHAM )
                ham_to_ppm(ifp, bmhd, colormap, colors, pchginfo);
            else
            if( dcol != NULL )
                direct_to_ppm(ifp, bmhd, dcol);
            else
            if( bmhd->nPlanes == 24 )
                deep_to_ppm(ifp, bmhd);
            else
                std_to_ppm(ifp, bmhd, colormap, colors, pchginfo, viewportmodes);
            body = 1;
        }
        else
        if( strcmp(iffid, "GRAB") == 0 || strcmp(iffid, "DEST") == 0 ||
            strcmp(iffid, "SPRT") == 0 || strcmp(iffid, "CRNG") == 0 ||
            strcmp(iffid, "CCRT") == 0 || strcmp(iffid, "CLUT") == 0 ||
            strcmp(iffid, "DPPV") == 0 || strcmp(iffid, "DRNG") == 0 ||
            strcmp(iffid, "EPSF") == 0 ) {
            skip_chunk(ifp, chunksize);
        }
        else
        if( strcmp(iffid, "(c) ") == 0 || strcmp(iffid, "AUTH") == 0 ||
            strcmp(iffid, "NAME") == 0 || strcmp(iffid, "ANNO") == 0 ||
            strcmp(iffid, "TEXT") == 0 ) {
            if( verbose )
                display_chunk(ifp, iffid, chunksize);
            else
                skip_chunk(ifp, chunksize);
        }
        else
        if( strcmp(iffid, "DPI ") == 0 ) {
            int x, y;

            x = get_big_short(ifp);
            y = get_big_short(ifp);
            if( verbose )
                pm_message("\"DPI \" chunk:  dpi_x = %d    dpi_y = %d", x, y);
        }
        else {
            pm_message("unknown chunk type \"%s\" - skipping", iffid);
            skip_chunk(ifp, chunksize);
        }

        bytesread += chunksize;
        if( odd(chunksize) ) {
                (void) get_byte(ifp);
                ++bytesread;
        }
    }
    pm_close(ifp);

    if( body == 0 ) {
        if( colormap )
            cmap_to_ppm(colormap, colors);
        else
            pm_error("no \"BODY\" or \"CMAP\" chunk found");
    }

    if( bytesread != formsize ) {
        pm_message("warning - file length (%ld bytes) does not match FORM size field (%ld bytes) +8",
                    bytesread, formsize);
    }

    exit(0);
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
getfourchars(ifp, fourchars)
    FILE* ifp;
    char fourchars[4];
{
    fourchars[0] = get_byte(ifp);
    fourchars[1] = get_byte(ifp);
    fourchars[2] = get_byte(ifp);
    fourchars[3] = get_byte(ifp);
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

static short
get_big_short(ifp)
    FILE *ifp;
{
    short s;

    if( pm_readbigshort(ifp, &s) == -1 )
        readerr(ifp);

    return s;
}


static void
skip_chunk(ifp, chunksize)
    FILE *ifp;
    long chunksize;
{
    int i;

    for( i = 0; i < chunksize; i++ )
        (void) get_byte(ifp);
}


static pixel *
read_colormap(ifp, colors)
    FILE *ifp;
    long colors;
{
    pixel *colormap;
    int i, r, g, b;
    pixval colmaxval = 0;

    colormap = ppm_allocrow(colors);
    for( i = 0; i < colors; i++ ) {
        r = get_byte(ifp); if( r > colmaxval ) colmaxval = r;
        g = get_byte(ifp); if( g > colmaxval ) colmaxval = g;
        b = get_byte(ifp); if( b > colmaxval ) colmaxval = b;
        PPM_ASSIGN(colormap[i], r, g, b);
    }
#ifdef DEBUG
    pm_message("colormap maxval is %d", colmaxval);
#endif
    if( colmaxval == 0 )
        pm_message("warning - black colormap");
    else
    if( colmaxval <= 15 ) {
        if( !adjustcolors ) {
            pm_message("warning - probably 4 bit colormap");
            pm_message("use \"-adjustcolors\" to scale colormap to 8 bits");
        }
        else {
            pm_message("scaling colormap to 8 bits");
            scale_colormap(colormap, colors, 15, MAXCOLVAL);
        }
    }
    return colormap;
}


static BitMapHeader *
read_bmhd(ifp)
    FILE *ifp;
{
    BitMapHeader *bmhd;

    bmhd = MALLOC(1, BitMapHeader);

    bmhd->w = get_big_short(ifp);           /* cols */
    bmhd->h = get_big_short(ifp);           /* rows */
    bmhd->x = get_big_short(ifp);
    bmhd->y = get_big_short(ifp);
    bmhd->nPlanes = get_byte(ifp);
    bmhd->masking = get_byte(ifp);
    bmhd->compression = get_byte(ifp);
    bmhd->pad1 = get_byte(ifp);             /* (ignored) */
    bmhd->transparentColor = get_big_short(ifp);
    bmhd->xAspect = get_byte(ifp);
    bmhd->yAspect = get_byte(ifp);
    bmhd->pageWidth = get_big_short(ifp);
    bmhd->pageHeight = get_big_short(ifp);

    if( verbose ) {
        pm_message("dimensions: %dx%d", bmhd->w, bmhd->h);
        pm_message("BODY compression: %s", bmhd->compression <= cmpMAXKNOWN ?
                    cmpNAME[bmhd->compression] : "unknown");
    }

    /* fix aspect ratio */
    if( bmhd->xAspect == 0 ) {
        if( bmhd->yAspect == 0 ) {
            pm_message("warning - xAspect:yAspect = 0:0, using 1:1");
            bmhd->xAspect = bmhd->yAspect = 1;
        }
        else {
            pm_message("warning - xAspect = 0, setting to yAspect");
            bmhd->xAspect = bmhd->yAspect;
        }
    }
    else {
        if( bmhd->yAspect == 0 ) {
            pm_message("warning - yAspect = 0, setting to xAspect");
            bmhd->yAspect = bmhd->xAspect;
        }
    }
    if( bmhd->xAspect != bmhd->yAspect ) {
        pm_message("warning - non-square pixels; to fix do a 'pnmscale -%cscale %g'",
            bmhd->xAspect > bmhd->yAspect ? 'x' : 'y',
            bmhd->xAspect > bmhd->yAspect ? (float)(bmhd->xAspect)/bmhd->yAspect : (float)(bmhd->yAspect)/bmhd->xAspect);
    }

    return bmhd;
}


static void
ham_to_ppm(ifp, bmhd, colormap, colors, pchginfo)
    FILE *ifp;
    BitMapHeader *bmhd;
    pixel *colormap;
    int colors;
    PCHGInfo *pchginfo;
{
    int cols, rows, hambits, hammask, col, row;
    pixval maxval;
    rawtype *rawrow;
    int pchgflag = (pchginfo && colormap);

    cols = bmhd->w;
    rows = bmhd->h;
    hambits = bmhd->nPlanes - 2;
    hammask = (1 << hambits) - 1;

    pm_message("input is a %sHAM%d file", pchgflag ? "multipalette " : "", bmhd->nPlanes);

    if( hambits > MAXPLANES )
        pm_error("too many planes (max %d)", MAXPLANES);
    if( hambits < 0 ) {
        pm_message("HAM requires 2 or more planes");
        pm_error("try \"-ignore CAMG\" to treat this file as a normal ILBM");
    }

    maxval = pm_bitstomaxval(hambits);
    if( maxval > PPM_MAXMAXVAL )
        pm_error("nPlanes is too large - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS" );

    /* scale colormap to new maxval */
    if( colormap && maxval != MAXCOLVAL )
        scale_colormap(colormap, colors, MAXCOLVAL, maxval);

    if( pchgflag )
        init_pchg(pchginfo, colormap, colors, maxval);

    rawrow = alloc_rawrow(cols);

    ppm_writeppminit(stdout, cols, rows, maxval, 0);
    for( row = 0; row < rows; row++ ) {
        pixval r, g, b;

        if( pchgflag )
            adjust_colormap(pchginfo, row);

        decode_row(ifp, rawrow, bmhd->nPlanes, bmhd);

        r = g = b = 0;
        for( col = 0; col < cols; col++ ) {
            switch((rawrow[col] >> hambits) & 0x03) {
                case HAMCODE_CMAP:
                    if( colormap && colors >= maxval )
                        pixelrow[col] = colormap[rawrow[col] & hammask];
                    else
                        PPM_ASSIGN(pixelrow[col], rawrow[col] & hammask,
                                   rawrow[col] & hammask, rawrow[col] & hammask);
                    r = PPM_GETR(pixelrow[col]);
                    g = PPM_GETG(pixelrow[col]);
                    b = PPM_GETB(pixelrow[col]);
                    break;
                case HAMCODE_BLUE:
                    b = rawrow[col] & hammask;
                    PPM_ASSIGN(pixelrow[col], r, g, b);
                    break;
                case HAMCODE_RED:
                    r = rawrow[col] & hammask;
                    PPM_ASSIGN(pixelrow[col], r, g, b);
                    break;
                case HAMCODE_GREEN:
                    g = rawrow[col] & hammask;
                    PPM_ASSIGN(pixelrow[col], r, g, b);
                    break;
                default:
                    pm_error("impossible HAM code");
            }
        }
        ppm_writeppmrow(stdout, pixelrow, cols, (pixval) maxval, 0);
    }
}


static void
deep_to_ppm(ifp, bmhd)
    FILE *ifp;
    BitMapHeader *bmhd;
{
    int cols, rows, col, row;
    rawtype *Rrow, *Grow, *Brow;

    cols = bmhd->w;
    rows = bmhd->h;

    pm_message("input is a deep (24bit) ILBM");

    Rrow = alloc_rawrow(cols);
    Grow = alloc_rawrow(cols);
    Brow = alloc_rawrow(cols);

    ppm_writeppminit(stdout, cols, rows, MAXCOLVAL, 0);
    for( row = 0; row < rows; row++ ) {
        decode_row(ifp, Rrow, 8, bmhd);
        decode_row(ifp, Grow, 8, bmhd);
        decode_row(ifp, Brow, 8, bmhd);
        for( col = 0; col < cols; col++ )
            PPM_ASSIGN(pixelrow[col], Rrow[col], Grow[col], Brow[col]);
        ppm_writeppmrow(stdout, pixelrow, cols, MAXCOLVAL, 0);
    }
    pm_close(stdout);
}


static void
direct_to_ppm(ifp, bmhd, dcol)
    FILE *ifp;
    BitMapHeader *bmhd;
    DirectColor *dcol;
{
    int cols, rows, col, row, redplanes, greenplanes, blueplanes;
    rawtype *Rrow, *Grow, *Brow;
    pixval maxval, redmaxval, greenmaxval, bluemaxval;
    int scale;

    cols = bmhd->w;
    rows = bmhd->h;

    redplanes = dcol->r; greenplanes = dcol->g; blueplanes = dcol->b;

    pm_message("input is a %d:%d:%d direct color ILBM",
                redplanes, greenplanes, blueplanes);

    if( redplanes > MAXPLANES || blueplanes > MAXPLANES || greenplanes > MAXPLANES )
        pm_error("too many planes (max %d per color)", MAXPLANES);

    if( bmhd->nPlanes != (redplanes + greenplanes + blueplanes) )
        pm_error("BMHD/DCOL plane number mismatch");

    if( redplanes == blueplanes && redplanes == greenplanes ) {
        scale = 0;
        maxval = pm_bitstomaxval(redplanes);
    }
    else {
        scale = 1;
        redmaxval   = pm_bitstomaxval(redplanes);
        greenmaxval = pm_bitstomaxval(greenplanes);
        bluemaxval  = pm_bitstomaxval(blueplanes);

        maxval = max(redmaxval, max(greenmaxval, bluemaxval));
        pm_message("rescaling colors to %d bits", pm_maxvaltobits(maxval));
    }

    if( maxval > PPM_MAXMAXVAL )
        pm_error("too many planes - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS" );

    Rrow = alloc_rawrow(cols);
    Grow = alloc_rawrow(cols);
    Brow = alloc_rawrow(cols);

    ppm_writeppminit(stdout, cols, rows, maxval, 0);
    for( row = 0; row < rows; row++ ) {
        decode_row(ifp, Rrow, dcol->r, bmhd);
        decode_row(ifp, Grow, dcol->g, bmhd);
        decode_row(ifp, Brow, dcol->b, bmhd);

        if( scale ) {
            for( col = 0; col < cols; col++ ) {
                PPM_ASSIGN(pixelrow[col],
                            Rrow[col] * maxval / redmaxval,
                            Grow[col] * maxval / greenmaxval,
                            Brow[col] * maxval / bluemaxval);
            }
        }
        else {
            for( col = 0; col < cols; col++ )
                PPM_ASSIGN(pixelrow[col], Rrow[col], Grow[col], Brow[col]);
        }
        ppm_writeppmrow(stdout, pixelrow, cols, maxval, 0);
    }
    pm_close(stdout);
}


static void
cmap_to_ppm(colormap, colors)
    pixel *colormap;
    int colors;
{
    pm_message("input is a colormap file");

    ppm_writeppminit(stdout, colors, 1, MAXCOLVAL, 0);
    ppm_writeppmrow(stdout, colormap, colors, MAXCOLVAL, 0);
    pm_close(stdout);
}


static void
std_to_ppm(ifp, bmhd, colormap, colors, pchginfo, viewportmodes)
    FILE *ifp;
    BitMapHeader *bmhd;
    pixel *colormap;
    int colors;
    PCHGInfo *pchginfo;
    long viewportmodes;
{
    rawtype *rawrow;
    pixval maxval;
    int row, rows, col, cols;
    int pchgflag = (pchginfo && colormap);

    cols = bmhd->w;
    rows = bmhd->h;

    pm_message("input is a %d-plane %s%sILBM", bmhd->nPlanes,
                pchgflag ? "multipalette " : "",
                viewportmodes & vmEXTRA_HALFBRITE ? "EHB " : ""
              );

    if( bmhd->nPlanes > MAXPLANES )
        pm_error("too many planes (max %d)", MAXPLANES);

    if( colormap )
        maxval = MAXCOLVAL;
    else {
        maxval = pm_bitstomaxval(bmhd->nPlanes);
        pm_message("no colormap - interpreting values as grayscale");
    }
    if( maxval > PPM_MAXMAXVAL )
        pm_error("nPlanes is too large - try reconfiguring with PGM_BIGGRAYS\n    or without PPM_PACKCOLORS" );

    if( pchgflag )
        init_pchg(pchginfo, colormap, colors, maxval);

    rawrow = alloc_rawrow(cols);

    if( viewportmodes & vmEXTRA_HALFBRITE )
        colormap = ehb_to_cmap(colormap, &colors);

    ppm_writeppminit( stdout, cols, rows, (pixval) maxval, 0 );
    for( row = 0; row < rows; row++ ) {

        if( pchgflag )
            adjust_colormap(pchginfo, row);

        decode_row(ifp, rawrow, bmhd->nPlanes, bmhd);
        for( col = 0; col < cols; col++ ) {
            if( colormap )
                pixelrow[col] = colormap[rawrow[col]];
            else
                PPM_ASSIGN(pixelrow[col], rawrow[col], rawrow[col], rawrow[col]);
        }
        ppm_writeppmrow(stdout, pixelrow, cols, maxval, 0);
    }
}


static pixel *
ehb_to_cmap(colormap, colors)
    pixel *colormap;
    int *colors;
{
    pixel *tempcolormap = NULL;
    int i, col;

    if( colormap ) {
        col = *colors;
        tempcolormap = ppm_allocrow(col * 2);
        for( i = 0; i < col; i++ ) {
            tempcolormap[i] = colormap[i];
            PPM_ASSIGN(tempcolormap[col + i], PPM_GETR(colormap[i]) / 2,
                        PPM_GETG(colormap[i]) / 2, PPM_GETB(colormap[i]) / 2 );
        }
        ppm_freerow(colormap);
        *colors *= 2;
    }
    return tempcolormap;
}


static void
read_ilbm_plane(ifp, cols, compression)
    FILE *ifp;
    int cols, compression;
{
    unsigned char *ubp;
    int bytes, j, byte;

    bytes = RowBytes(cols);

    switch(compression) {
        case cmpNone:
            j = fread(ilbmrow, 1, bytes, ifp);
            if( j != bytes )
                readerr(ifp);
            break;
        case cmpByteRun1:
            ubp = ilbmrow;
            do {
                byte = (int)get_byte(ifp);
                if( byte <= 127 ) {
                    j = byte;
                    bytes -= (j+1);
                    if( bytes < 0 )
                        pm_error("error doing ByteRun1 decompression");
                    for( ; j >= 0; j-- )
                        *ubp++ = get_byte(ifp);
                }
                else
                if ( byte != 128 ) {
                    j = 256 - byte;
                    bytes -= (j+1);
                    if( bytes < 0 )
                        pm_error("error doing ByteRun1 decompression");
                    byte = (int)get_byte(ifp);
                    for( ; j >= 0; j-- )
                        *ubp++ = (unsigned char)byte;
                }
                /* 128 is a NOP */
            }
            while( bytes > 0 );
            break;
        default:
                pm_error("unknown compression type");
    }
}


const unsigned char bit_mask[] = {1, 2, 4, 8, 16, 32, 64, 128};

static void
decode_row(ifp, chunkyrow, nPlanes, bmhd)
    FILE *ifp;
    rawtype *chunkyrow;
    int nPlanes;
    BitMapHeader *bmhd;
{
    int plane, col, cols;
    unsigned char *ilp;
    rawtype *chp;

    cols = bmhd->w;
    for( plane = 0; plane < nPlanes; plane++ ) {
        int mask, cbit;

        mask = 1 << plane;
        read_ilbm_plane(ifp, cols, bmhd->compression);

        ilp = ilbmrow;
        chp = chunkyrow;

        cbit = 7;
        for( col = 0; col < cols; col++, cbit--, chp++ ) {
            if( cbit < 0 ) {
                cbit = 7;
                ilp++;
            }
            if( *ilp & bit_mask[cbit] )
                *chp |= mask;
            else
                *chp &= ~mask;
        }
    }
    /* skip mask plane */
    if( bmhd->masking == mskHasMask )
        read_ilbm_plane(ifp, cols, bmhd->compression);
}


static rawtype *
alloc_rawrow(cols)
    int cols;
{
    rawtype *r;
    int i;

    r = MALLOC(cols, rawtype);

    for( i = 0; i < cols; i++ )
        r[i] = 0;

    return r;
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


static void
display_chunk(ifp, iffid, chunksize)
    FILE *ifp;
    char *iffid;
    long chunksize;
{
    int byte;

    pm_message("contents of \"%s\" chunk:", iffid);

    while( chunksize-- ) {
        byte = get_byte(ifp);
        if( fputc(byte, stderr) == EOF )
            pm_error("write error");
    }
    if( fputc('\n', stderr) == EOF )
        pm_error("write error");
}

/*
 * PCHG stuff
 */

static void PCHG_Decompress ARGS((PCHGHeader *PCHG, PCHGCompHeader *CompHdr, unsigned char *compdata, unsigned long compsize, unsigned char *comptree, unsigned char *data));
static unsigned char * PCHG_MakeMask ARGS((PCHGHeader *PCHG, unsigned char *data, unsigned long datasize, unsigned char **newdata));
static void PCHG_ConvertSmall ARGS((PCHGInfo *Info, unsigned char *data, unsigned long datasize));
static void PCHG_ConvertBig ARGS((PCHGInfo *Info, unsigned char *data, unsigned long datasize));
static void PCHG_DecompHuff ARGS((unsigned char *src, unsigned char *dest, short *tree, unsigned long origsize));
static void pchgerr ARGS((char *when));

/* Turn big-endian 4-byte long and 2-byte short stored at x (unsigned char *)
 * into the native format of the CPU
 */
#define BIG_LONG(x) (   ((unsigned long)((x)[0]) << 24) + \
                        ((unsigned long)((x)[1]) << 16) + \
                        ((unsigned long)((x)[2]) <<  8) + \
                        ((unsigned long)((x)[3]) <<  0) )
#define BIG_WORD(x) (   ((unsigned short)((x)[0]) << 8) + \
                        ((unsigned short)((x)[1]) << 0) )


static PCHGInfo *
read_pchg(ifp, bytesleft)
    FILE *ifp;
    unsigned long bytesleft;
{
    static PCHGInfo Info;
    PCHGCompHeader  CompHdr;
    PCHGHeader      *PCHG;
    unsigned char   *data, *chdata;
    unsigned long   datasize;

#ifdef DEBUG
    pm_message("PCHG chunk found");
#endif

    if( bytesleft < PCHGHeaderSize )
        pchgerr("while reading PCHGHeader");

    Info.PCHG = PCHG = MALLOC(1, PCHGHeader);
    PCHG->Compression = get_big_short(ifp);
    PCHG->Flags       = get_big_short(ifp);
    PCHG->StartLine   = get_big_short(ifp);
    PCHG->LineCount   = get_big_short(ifp);
    PCHG->ChangedLines= get_big_short(ifp);
    PCHG->MinReg      = get_big_short(ifp);
    PCHG->MaxReg      = get_big_short(ifp);
    PCHG->MaxChanges  = get_big_short(ifp);
    PCHG->TotalChanges= get_big_long(ifp);
    bytesleft -= PCHGHeaderSize;

#ifdef DEBUG
    pm_message("PCHG StartLine   : %d", PCHG->StartLine);
    pm_message("PCHG LineCount   : %d", PCHG->LineCount);
    pm_message("PCHG ChangedLines: %d", PCHG->ChangedLines);
    pm_message("PCHG TotalChanges: %d", PCHG->TotalChanges);
#endif

    if( PCHG->Compression != PCHG_COMP_NONE ) {
        unsigned char *compdata, *comptree;
        unsigned long treesize;

        if( bytesleft < PCHGCompHeaderSize )
            pchgerr("while reading PCHGCompHeader");

        CompHdr.CompInfoSize     = get_big_long(ifp);
        CompHdr.OriginalDataSize = get_big_long(ifp);
        bytesleft -= PCHGCompHeaderSize;
        treesize = CompHdr.CompInfoSize;
        datasize = CompHdr.OriginalDataSize;

        if( bytesleft < treesize )
            pchgerr("while reading compression info data");

        comptree = MALLOC(treesize, unsigned char);
        if( fread(comptree, 1, treesize, ifp) != treesize )
            readerr(ifp);

        bytesleft -= treesize;
        if( bytesleft == 0 )
            pchgerr("while reading compressed change structure data");

        compdata= MALLOC(bytesleft, unsigned char);
        data    = MALLOC(datasize, unsigned char);

        if( fread(compdata, 1, bytesleft, ifp) != bytesleft )
            readerr(ifp);

        PCHG_Decompress(PCHG, &CompHdr, compdata, bytesleft, comptree, data);
        free(comptree);
        free(compdata);
        bytesleft = 0;
    }
    else {
#ifdef DEBUG
        pm_message("uncompressed PCHG");
#endif
        if( bytesleft == 0 )
            pchgerr("while reading uncompressed change structure data");

        datasize = bytesleft;
        data = MALLOC(datasize, unsigned char);
        if( fread(data, 1, datasize, ifp) != datasize )
            readerr(ifp);
        bytesleft = 0;
    }

    Info.LineMask = PCHG_MakeMask(PCHG, data, datasize, &chdata);
    datasize -= (chdata - data);

    Info.Palette = MALLOC(PCHG->TotalChanges, PaletteChange);
    Info.Change  = MALLOC(PCHG->ChangedLines, LineChanges);

    if( PCHG->Flags & PCHGF_USE_ALPHA )
        pm_message("warning - PCHG alpha channel not supported");

    if( PCHG->Flags & PCHGF_12BIT ) {
#ifdef DEBUG
        pm_message("SmallLineChanges");
#endif
        PCHG_ConvertSmall(&Info, chdata, datasize);
    }
    else
    if( PCHG->Flags & PCHGF_32BIT ) {
#ifdef DEBUG
        pm_message("BigLineChanges");
#endif
        PCHG_ConvertBig(&Info, chdata, datasize);
    }
    else
        pm_error("unknown palette changes structure format in PCHG chunk");

    free(data);
    return &Info;
}


static void
PCHG_Decompress(PCHG, CompHdr, compdata, compsize, comptree, data)
    PCHGHeader *PCHG;
    PCHGCompHeader *CompHdr;
    unsigned char *compdata;
    unsigned long compsize;
    unsigned char *comptree;
    unsigned char *data;
{
    short *hufftree;
    unsigned long huffsize, i;
    unsigned long treesize = CompHdr->CompInfoSize;

    switch( PCHG->Compression ) {
        case PCHG_COMP_HUFFMAN:

#ifdef DEBUG
            pm_message("PCHG Huffman compression");
#endif
            /* turn big-endian 2-byte shorts into native format */
            huffsize = treesize/2;
            hufftree = MALLOC(huffsize, short);
            for( i = 0; i < huffsize; i++ ) {
                hufftree[i] = (short)BIG_WORD(comptree);
                comptree += 2;
            }

            /* decompress the change structure data */
            PCHG_DecompHuff(compdata, data, &hufftree[huffsize-1], CompHdr->OriginalDataSize);

            free(hufftree);
            break;
        default:
            pm_error("unknown PCHG compression type");
    }
}


static unsigned char *
PCHG_MakeMask(PCHG, data, datasize, newdata)
    PCHGHeader *PCHG;
    unsigned char *data;
    unsigned long datasize;
    unsigned char **newdata;
{
    unsigned long bytes;
    unsigned char *mask;

    /* the mask at 'data' is in 4-byte big-endian longword format,
     * thus we can simply treat it at unsigned char and don't have
     * to convert it, just copy it to a new mem block so we can
     * free the original data
     */
    bytes = MaskLongWords(PCHG->LineCount) * 4;
    if( datasize < bytes )
        pchgerr("for line mask");
    mask = MALLOC(bytes, unsigned char);

#ifdef DEBUG
    pm_message("%ld bytes for line mask", bytes);
#endif
    bcopy(data, mask, bytes);

    *newdata = (data + bytes);
    return mask;
}


static void
PCHG_ConvertSmall(Info, data, datasize)
    PCHGInfo *Info;
    unsigned char *data;
    unsigned long datasize;
{
    PCHGHeader *PCHG        = Info->PCHG;
    LineChanges *Change     = Info->Change;
    PaletteChange *Palette  = Info->Palette;
    unsigned long i, palettecount = 0;
    unsigned char ChangeCount16, ChangeCount32;
    unsigned short SmallChange;

    Info->maxval = 15;  /* 4 bit values */

    for( i = 0; i < PCHG->ChangedLines; i++ ) {
        int n;

        if( datasize < 2 ) goto fail;
        ChangeCount16 = *data++;
        ChangeCount32 = *data++;
        datasize -= 2;

        Change[i].Count = ChangeCount16 + ChangeCount32;
        Change[i].Palette = &Palette[palettecount];

        for(n = 0; n < Change[i].Count; n++ ) {
            if( palettecount >= PCHG->TotalChanges ) goto fail;
            if( datasize < 2 ) goto fail;
            SmallChange = BIG_WORD(data);
            data += 2; datasize -= 2;

            Palette[palettecount].Register = (SmallChange >> 12) & 0x0f;
            if( n >= ChangeCount16 )
                Palette[palettecount].Register += 16;
            Palette[palettecount].Alpha = 0;
            Palette[palettecount].Red   = (SmallChange >> 8) & 0x0f;
            Palette[palettecount].Green = (SmallChange >> 4) & 0x0f;
            Palette[palettecount].Blue  = SmallChange & 0x0f;
            palettecount++;
        }
    }
#ifdef DEBUG
    pm_message("%ld palette change structures", palettecount);
#endif
    return;
fail:
    pchgerr("while building SmallLineChanges array");
}


static void
PCHG_ConvertBig(Info, data, datasize)
    PCHGInfo *Info;
    unsigned char *data;
    unsigned long datasize;
{
    PCHGHeader *PCHG        = Info->PCHG;
    LineChanges *Change     = Info->Change;
    PaletteChange *Palette  = Info->Palette;
    unsigned long i, palettecount = 0;

    Info->maxval = MAXCOLVAL;

    for( i = 0; i < PCHG->ChangedLines; i++ ) {
        int n;

        if( datasize < 2 ) goto fail;
        Change[i].Count = BIG_WORD(data);
        data += 2; datasize -= 2;

        Change[i].Palette = &Palette[palettecount];

        for( n = 0; n < Change[i].Count; n++ ) {
            if( palettecount >= PCHG->TotalChanges ) goto fail;
            if( datasize < 6 ) goto fail;
            Palette[palettecount].Register = BIG_WORD(data);
            data += 2;
            Palette[palettecount].Alpha = *data++;
            Palette[palettecount].Red   = *data++;
            Palette[palettecount].Blue  = *data++;  /* yes, RBG */
            Palette[palettecount].Green = *data++;
            palettecount++;
            datasize -= 6;
        }
    }
#ifdef DEBUG
    pm_message("%ld palette change structures", palettecount);
#endif
    return;
fail:
    pchgerr("while building BigLineChanges array");
}


static void
pchgerr(when)
    char *when;
{
    pm_message("insufficient data in PCHG chunk %s", when);
    pm_error("try the \"-ignore\" option to skip this chunk");
}


static void
PCHG_DecompHuff(src, dest, tree, origsize)
    unsigned char *src, *dest;
    short *tree;
    unsigned long origsize;
{
    unsigned long i = 0, bits = 0;
    unsigned char thisbyte;
    short *p;

    p = tree;
    while( i < origsize ) {
        if( bits == 0 ) {
            thisbyte = *src++;
            bits = 8;
        }
        if( thisbyte & (1 << 7) ) {
            if( *p >= 0 ) {
                *dest++ = (unsigned char)*p;
                i++;
                p = tree;
            }
            else
                p += (*p / 2);
        }
        else {
            p--;
            if( *p > 0 && (*p & 0x100) ) {
                *dest++ = (unsigned char )*p;
                i++;
                p = tree;
            }
        }
        thisbyte <<= 1;
        bits--;
    }
}


static void
init_pchg(pchginfo, colormap, colors, newmaxval)
    PCHGInfo *pchginfo;
    pixel *colormap;
    int colors;
    pixval newmaxval;
{
    PCHGHeader    *PCHG    = pchginfo->PCHG;
    pixval oldmaxval       = pchginfo->maxval;
    int row;

    pchginfo->colormap = colormap;
    pchginfo->colors   = colors;

    if( oldmaxval != newmaxval ) {
        PaletteChange *Palette = pchginfo->Palette;
        unsigned long i;

#ifdef DEBUG
        pm_message("scaling PCHG palette from %d to %d", oldmaxval, newmaxval);
#endif

        for( i = 0; i < PCHG->TotalChanges; i++ ) {
            Palette[i].Red  = Palette[i].Red   * newmaxval / oldmaxval;
            Palette[i].Green= Palette[i].Green * newmaxval / oldmaxval;
            Palette[i].Blue = Palette[i].Blue  * newmaxval / oldmaxval;
        }
        pchginfo->maxval = newmaxval;
    }

    for( row = PCHG->StartLine; row < 0; row++ )
        adjust_colormap(pchginfo, row);
}


static void
adjust_colormap(pchginfo, row)
    PCHGInfo *pchginfo;
    int row;
{
    static unsigned long maskcount, changecount;
    static unsigned char thismask;
    static int bits;

    PCHGHeader *PCHG = pchginfo->PCHG;

    if( row < PCHG->StartLine || changecount >= PCHG->ChangedLines )
        return;

    if( bits == 0 ) {
        thismask = pchginfo->LineMask[maskcount++];
        bits = 8;
    }

    if( thismask & (1 << 7) ) {
        int i;

        for( i = 0; i < pchginfo->Change[changecount].Count; i++ ) {
            PaletteChange *pal = &(pchginfo->Change[changecount].Palette[i]);
            int reg = pal->Register;

            if( reg >= pchginfo->colors ) {
                pm_message("warning - PCHG palette change register value out of range");
                pm_message("    row %d  change structure %ld  palette %d", row, changecount, i);
                pm_message("    ignoring it... colors might get messed up from here");
            }
	    else
	      PPM_ASSIGN(pchginfo->colormap[reg], pal->Red, pal->Green, pal->Blue);
	}
        changecount++;
    }
    thismask <<= 1;
    bits--;
}


static void
scale_colormap(colormap, colors, oldmaxval, newmaxval)
    pixel *colormap;
    int colors;
    pixval oldmaxval, newmaxval;
{
    int i, r, g, b;

    for( i = 0; i < colors; i++ ) {
        r = PPM_GETR(colormap[i]) * newmaxval / oldmaxval;
        g = PPM_GETG(colormap[i]) * newmaxval / oldmaxval;
        b = PPM_GETB(colormap[i]) * newmaxval / oldmaxval;
        PPM_ASSIGN(colormap[i], r, g, b);
    }
}

