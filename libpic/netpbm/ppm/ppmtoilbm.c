/* ppmtoilbm.c - read a portable pixmap and produce an IFF ILBM file
**
** Copyright (C) 1989 by Jef Poskanzer.
** Modified by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**  20/Jun/93:
**  - 24bit support (new options -24if, -24force)
**  - HAM8 support (well, anything from HAM3 to HAM(MAXPLANES))
**  - now writes up to 8 (16) planes (new options -maxplanes, -fixplanes)
**  - colormap file (new option -map)
**  - write colormap only (new option -cmaponly)
**  - only writes CAMG chunk if it is a HAM-picture
**  29/Aug/93:
**  - operates row-by-row whenever possible
**  - faster colorscaling with lookup-table (~20% faster on HAM pictures)
**  - options -ham8 and -ham6 now imply -hamforce
**  27/Nov/93:
**  - byterun1 compression (this is now default) with new options:
**    -compress, -nocompress, -cmethod, -savemem
**  - floyd-steinberg error diffusion (for std+mapfile and HAM)
**  - new options: -lace and -hires --> write CAMG chunk
**  - LUT for luminance calculation (used by ppm_to_ham)
**
**
**           std   HAM  24bit cmap  direct
**  -------+-----+-----+-----+-----+-----
**  BMHD     yes   yes   yes   yes   yes
**  CMAP     yes   (1)   no    yes   no
**  BODY     yes   yes   yes   no    yes
**  CAMG     (2)   yes   (2)   no    (2)
**  other    -     -     -     -     DCOL
**  nPlanes  1-8   3-8   24    0     3-24   if configured without ILBM_BIGRAW
**  nPlanes  1-16  3-16  24    0     3-48   if configured with ILBM_BIGRAW
**
**  (1): grayscale colormap
**  (2): only if "-lace" or "-hires" option used
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"
#include "ppmcmap.h"
#include "ilbm.h"

/*#define DEBUG*/

#define MODE_DIRECT     4   /* direct color ILBM */
#define MODE_CMAP       3   /* write normal file, but colormap only */
#define MODE_24         2   /* write a 24bit (deep) ILBM */
#define MODE_HAM        1   /* write a HAM */
#define MODE_NONE       0   /* write a normal picture */

#define ECS_MAXPLANES   5
#define ECS_HAMPLANES   6
#define AGA_MAXPLANES   8
#define AGA_HAMPLANES   8

#ifdef AMIGA_AGA
#define DEF_MAXPLANES   AGA_MAXPLANES
#define DEF_HAMPLANES   AGA_HAMPLANES
#else
#define DEF_MAXPLANES   ECS_MAXPLANES
#define DEF_HAMPLANES   ECS_HAMPLANES
#endif
#define DEF_DCOLPLANES  5
#define DEF_COMPRESSION cmpByteRun1


typedef struct {
    int             len;
    unsigned char * row;
} bodyrow;
typedef struct {
    long *thisrederr, *thisgreenerr, *thisblueerr;
    long *nextrederr, *nextgreenerr, *nextblueerr;
    int lefttoright;    /* 1 for left-to-right scan, 0 for right-to-left */
    int cols;
    pixel *pixrow;
    pixval maxval;
    int col, col_end;
    int alternate;
    pixval red, green, blue;    /* values of current pixel */
} floydinfo;


static int colorstobpp ARGS((int colors));
#define put_fourchars(str)  (void)(fputs(str, stdout))
static void put_big_short ARGS((short s));
static void put_big_long ARGS((long l));
#define put_byte(b)     (void)(putc((unsigned char)(b), stdout))
static void ppm_to_ham ARGS((FILE *fp, int cols, int rows, int maxval, int hambits, int nocolor));
static void ppm_to_24  ARGS((FILE *fp, int cols, int rows, int maxval));
static void ppm_to_direct  ARGS((FILE *fp, int cols, int rows, int maxval, DirectColor *direct));
static void ppm_to_std ARGS((FILE *fp, int cols, int rows, int maxval, colorhist_vector chv, int colors, int nPlanes));
static void ppm_to_cmap ARGS((int maxval, colorhist_vector chv, int colors));
static void write_form_ilbm ARGS((int size));
static void write_bmhd ARGS((int cols, int rows, int nPlanes));
static void write_std_cmap ARGS((colorhist_vector chv, int colors, int maxval));
static int encode_row ARGS((FILE *outfile, rawtype *rawrow, int cols, int nPlanes));
static int compress_row ARGS((int bytes));
static int runbyte1 ARGS((int bytes));
static pixel * next_pixrow ARGS((FILE *fp, int row));
static pixval * make_val_table ARGS((pixval oldmaxval, pixval newmaxval));
static void * xmalloc ARGS((int bytes));
#define MALLOC(n, type)     (type *)xmalloc((n) * sizeof(type))
static void init_read ARGS((FILE *fp, int *colsP, int *rowsP, pixval *maxvalP, int *formatP, int readall));
static void write_body ARGS((void));
static void write_camg ARGS((void));
static void alloc_body_array ARGS((int rows, int nPlanes));
static void free_body_array ARGS((void));
#define PAD(n)      odd(n)  /* pad to a word */
static int closest_color ARGS((colorhist_vector chv, int colors, pixval cmaxval, pixel *pP));
static floydinfo *init_floyd ARGS((int cols, pixval maxval, int alternate));
static void free_floyd ARGS((floydinfo *fi));
static void begin_floyd_row ARGS((floydinfo *fi, pixel *prow));
static pixel *next_floyd_pixel ARGS((floydinfo *fi));
static void update_floyd_pixel ARGS((floydinfo *fi, int r, int g, int b));
static void end_floyd_row ARGS((floydinfo *fi));

/* global data */
static unsigned char *coded_rowbuf; /* buffer for uncompressed scanline */
static unsigned char *compr_rowbuf; /* buffer for compressed scanline */
static pixel **pixels;  /* PPM image (NULL for row-by-row operation) */
static pixel *pixrow;   /* current row in PPM image (pointer into pixels array, or buffer for row-by-row operation) */
static bodyrow *ilbm_body = NULL;   /* compressed ILBM BODY */

static long viewportmodes = 0;
static int compr_type = DEF_COMPRESSION;

/* flags */
static short savemem = 0;       /* slow operation, but uses less memory */
static short compr_force = 0;   /* force compressed output, even if the image got larger  - NOT USED */
static short floyd = 0;         /* apply floyd-steinberg error diffusion */

#define WORSTCOMPR(bytes)       ((bytes) + (bytes)/128 + 1)
#define DO_COMPRESS             (compr_type != cmpNone)
#define CAMGSIZE                (viewportmodes == 0 ? 0 : (4 + 4 + CAMGChunkSize))



/* Lookup tables for fast RGB -> luminance calculation. */
/* taken from ppmtopgm.c   -IUW */

static int times77[256] = {
            0,    77,   154,   231,   308,   385,   462,   539,
          616,   693,   770,   847,   924,  1001,  1078,  1155,
         1232,  1309,  1386,  1463,  1540,  1617,  1694,  1771,
         1848,  1925,  2002,  2079,  2156,  2233,  2310,  2387,
         2464,  2541,  2618,  2695,  2772,  2849,  2926,  3003,
         3080,  3157,  3234,  3311,  3388,  3465,  3542,  3619,
         3696,  3773,  3850,  3927,  4004,  4081,  4158,  4235,
         4312,  4389,  4466,  4543,  4620,  4697,  4774,  4851,
         4928,  5005,  5082,  5159,  5236,  5313,  5390,  5467,
         5544,  5621,  5698,  5775,  5852,  5929,  6006,  6083,
         6160,  6237,  6314,  6391,  6468,  6545,  6622,  6699,
         6776,  6853,  6930,  7007,  7084,  7161,  7238,  7315,
         7392,  7469,  7546,  7623,  7700,  7777,  7854,  7931,
         8008,  8085,  8162,  8239,  8316,  8393,  8470,  8547,
         8624,  8701,  8778,  8855,  8932,  9009,  9086,  9163,
         9240,  9317,  9394,  9471,  9548,  9625,  9702,  9779,
         9856,  9933, 10010, 10087, 10164, 10241, 10318, 10395,
        10472, 10549, 10626, 10703, 10780, 10857, 10934, 11011,
        11088, 11165, 11242, 11319, 11396, 11473, 11550, 11627,
        11704, 11781, 11858, 11935, 12012, 12089, 12166, 12243,
        12320, 12397, 12474, 12551, 12628, 12705, 12782, 12859,
        12936, 13013, 13090, 13167, 13244, 13321, 13398, 13475,
        13552, 13629, 13706, 13783, 13860, 13937, 14014, 14091,
        14168, 14245, 14322, 14399, 14476, 14553, 14630, 14707,
        14784, 14861, 14938, 15015, 15092, 15169, 15246, 15323,
        15400, 15477, 15554, 15631, 15708, 15785, 15862, 15939,
        16016, 16093, 16170, 16247, 16324, 16401, 16478, 16555,
        16632, 16709, 16786, 16863, 16940, 17017, 17094, 17171,
        17248, 17325, 17402, 17479, 17556, 17633, 17710, 17787,
        17864, 17941, 18018, 18095, 18172, 18249, 18326, 18403,
        18480, 18557, 18634, 18711, 18788, 18865, 18942, 19019,
        19096, 19173, 19250, 19327, 19404, 19481, 19558, 19635 };
static int times150[256] = {
            0,   150,   300,   450,   600,   750,   900,  1050,
         1200,  1350,  1500,  1650,  1800,  1950,  2100,  2250,
         2400,  2550,  2700,  2850,  3000,  3150,  3300,  3450,
         3600,  3750,  3900,  4050,  4200,  4350,  4500,  4650,
         4800,  4950,  5100,  5250,  5400,  5550,  5700,  5850,
         6000,  6150,  6300,  6450,  6600,  6750,  6900,  7050,
         7200,  7350,  7500,  7650,  7800,  7950,  8100,  8250,
         8400,  8550,  8700,  8850,  9000,  9150,  9300,  9450,
         9600,  9750,  9900, 10050, 10200, 10350, 10500, 10650,
        10800, 10950, 11100, 11250, 11400, 11550, 11700, 11850,
        12000, 12150, 12300, 12450, 12600, 12750, 12900, 13050,
        13200, 13350, 13500, 13650, 13800, 13950, 14100, 14250,
        14400, 14550, 14700, 14850, 15000, 15150, 15300, 15450,
        15600, 15750, 15900, 16050, 16200, 16350, 16500, 16650,
        16800, 16950, 17100, 17250, 17400, 17550, 17700, 17850,
        18000, 18150, 18300, 18450, 18600, 18750, 18900, 19050,
        19200, 19350, 19500, 19650, 19800, 19950, 20100, 20250,
        20400, 20550, 20700, 20850, 21000, 21150, 21300, 21450,
        21600, 21750, 21900, 22050, 22200, 22350, 22500, 22650,
        22800, 22950, 23100, 23250, 23400, 23550, 23700, 23850,
        24000, 24150, 24300, 24450, 24600, 24750, 24900, 25050,
        25200, 25350, 25500, 25650, 25800, 25950, 26100, 26250,
        26400, 26550, 26700, 26850, 27000, 27150, 27300, 27450,
        27600, 27750, 27900, 28050, 28200, 28350, 28500, 28650,
        28800, 28950, 29100, 29250, 29400, 29550, 29700, 29850,
        30000, 30150, 30300, 30450, 30600, 30750, 30900, 31050,
        31200, 31350, 31500, 31650, 31800, 31950, 32100, 32250,
        32400, 32550, 32700, 32850, 33000, 33150, 33300, 33450,
        33600, 33750, 33900, 34050, 34200, 34350, 34500, 34650,
        34800, 34950, 35100, 35250, 35400, 35550, 35700, 35850,
        36000, 36150, 36300, 36450, 36600, 36750, 36900, 37050,
        37200, 37350, 37500, 37650, 37800, 37950, 38100, 38250 };
static int times29[256] = {
            0,    29,    58,    87,   116,   145,   174,   203,
          232,   261,   290,   319,   348,   377,   406,   435,
          464,   493,   522,   551,   580,   609,   638,   667,
          696,   725,   754,   783,   812,   841,   870,   899,
          928,   957,   986,  1015,  1044,  1073,  1102,  1131,
         1160,  1189,  1218,  1247,  1276,  1305,  1334,  1363,
         1392,  1421,  1450,  1479,  1508,  1537,  1566,  1595,
         1624,  1653,  1682,  1711,  1740,  1769,  1798,  1827,
         1856,  1885,  1914,  1943,  1972,  2001,  2030,  2059,
         2088,  2117,  2146,  2175,  2204,  2233,  2262,  2291,
         2320,  2349,  2378,  2407,  2436,  2465,  2494,  2523,
         2552,  2581,  2610,  2639,  2668,  2697,  2726,  2755,
         2784,  2813,  2842,  2871,  2900,  2929,  2958,  2987,
         3016,  3045,  3074,  3103,  3132,  3161,  3190,  3219,
         3248,  3277,  3306,  3335,  3364,  3393,  3422,  3451,
         3480,  3509,  3538,  3567,  3596,  3625,  3654,  3683,
         3712,  3741,  3770,  3799,  3828,  3857,  3886,  3915,
         3944,  3973,  4002,  4031,  4060,  4089,  4118,  4147,
         4176,  4205,  4234,  4263,  4292,  4321,  4350,  4379,
         4408,  4437,  4466,  4495,  4524,  4553,  4582,  4611,
         4640,  4669,  4698,  4727,  4756,  4785,  4814,  4843,
         4872,  4901,  4930,  4959,  4988,  5017,  5046,  5075,
         5104,  5133,  5162,  5191,  5220,  5249,  5278,  5307,
         5336,  5365,  5394,  5423,  5452,  5481,  5510,  5539,
         5568,  5597,  5626,  5655,  5684,  5713,  5742,  5771,
         5800,  5829,  5858,  5887,  5916,  5945,  5974,  6003,
         6032,  6061,  6090,  6119,  6148,  6177,  6206,  6235,
         6264,  6293,  6322,  6351,  6380,  6409,  6438,  6467,
         6496,  6525,  6554,  6583,  6612,  6641,  6670,  6699,
         6728,  6757,  6786,  6815,  6844,  6873,  6902,  6931,
         6960,  6989,  7018,  7047,  7076,  7105,  7134,  7163,
         7192,  7221,  7250,  7279,  7308,  7337,  7366,  7395 };


/************ parse options and figure out what kind of ILBM to write ************/


static int get_int_val ARGS((char *string, char *option, int bot, int top));
static int get_compr_type ARGS((char *string));
#define NEWDEPTH(pix, table)    PPM_ASSIGN((pix), (table)[PPM_GETR(pix)], (table)[PPM_GETG(pix)], (table)[PPM_GETB(pix)])


int
main(argc, argv)
    int argc;
    char *argv[];
{
    FILE *ifp;
    int argn, rows, cols, format, colors, nPlanes;
    int ifmode, forcemode, maxplanes, fixplanes, hambits, mode;
#define MAXCOLORS       (1<<maxplanes)
    pixval maxval;
    colorhist_vector chv;
    DirectColor dcol;
    char *mapfile;
    char *usage =
"[-ecs|-aga] [-ham6|-ham8] [-maxplanes|-mp n] [-fixplanes|-fp n]\
 [-normal|-hamif|-hamforce|-24if|-24force|-dcif|-dcforce|-cmaponly]\
 [-hambits|-hamplanes n] [-dcbits|-dcplanes r g b] [-hires] [-lace]\
 [-floyd|-fs] [-compress|-nocompress] [-cmethod none|byterun1]\
 [-map ppmfile] [-savemem] [ppmfile]";

    ppm_init(&argc, argv);

    ifmode = MODE_NONE; forcemode = MODE_NONE;
    maxplanes = DEF_MAXPLANES; fixplanes = 0;
    hambits = DEF_HAMPLANES;
    mapfile = NULL;
    dcol.r = dcol.g = dcol.b = DEF_DCOLPLANES;

    argn = 1;
    while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
        if( pm_keymatch(argv[argn], "-maxplanes", 4) || pm_keymatch(argv[argn], "-mp", 3) ) {
            if( ++argn >= argc )
                pm_usage(usage);
            maxplanes = get_int_val(argv[argn], argv[argn-1], 1, MAXPLANES);
            fixplanes = 0;
        }
        else
        if( pm_keymatch(argv[argn], "-fixplanes", 4) || pm_keymatch(argv[argn], "-fp", 3) ) {
            if( ++argn >= argc )
                pm_usage(usage);
            fixplanes = get_int_val(argv[argn], argv[argn-1], 1, MAXPLANES);
            maxplanes = fixplanes;
        }
        else
        if( pm_keymatch(argv[argn], "-map", 4) ) {
            if( ++argn >= argc )
                pm_usage(usage);
            mapfile = argv[argn];
        }
        else
        if( pm_keymatch(argv[argn], "-cmaponly", 3) ) {
            forcemode = MODE_CMAP;
        }
        else
        if( pm_keymatch(argv[argn], "-hambits", 5) || pm_keymatch(argv[argn], "-hamplanes", 5) ) {
            if( ++argn > argc )
                pm_usage(usage);
            hambits = get_int_val(argv[argn], argv[argn-1], 3, MAXPLANES);
        }
        else
        if( pm_keymatch(argv[argn], "-ham6", 5) ) {
            hambits = ECS_HAMPLANES;
            forcemode = MODE_HAM;
        }
        else
        if( pm_keymatch(argv[argn], "-ham8", 5) ) {
            hambits = AGA_HAMPLANES;
            forcemode = MODE_HAM;
        }
        else
        if( pm_keymatch(argv[argn], "-lace", 2) ) {
#ifdef ILBM_PCHG
            slicesize = 2;
#endif
            viewportmodes |= vmLACE;
        }
        else
        if( pm_keymatch(argv[argn], "-nolace", 4) ) {
#ifdef ILBM_PCHG
            slicesize = 1;
#endif
            viewportmodes &= ~vmLACE;
        }
        else
        if( pm_keymatch(argv[argn], "-hires", 3) )
            viewportmodes |= vmHIRES;
        else
        if( pm_keymatch(argv[argn], "-nohires", 5) )
            viewportmodes &= ~vmHIRES;
        else
        if( pm_keymatch(argv[argn], "-ecs", 2) ) {
            maxplanes = ECS_MAXPLANES;
            hambits = ECS_HAMPLANES;
        }
        else
        if( pm_keymatch(argv[argn], "-aga", 2) ) {
            maxplanes = AGA_MAXPLANES;
            hambits = AGA_HAMPLANES;
        }
        else
        if( pm_keymatch(argv[argn], "-hamif", 5) )
            ifmode = MODE_HAM;
        else
        if( pm_keymatch(argv[argn], "-nohamif", 7) ) {
            if( ifmode == MODE_HAM )
                ifmode = MODE_NONE;
        }
        else
        if( pm_keymatch(argv[argn], "-hamforce", 5) )
            forcemode = MODE_HAM;
        else
        if( pm_keymatch(argv[argn], "-nohamforce", 7) ) {
            if( forcemode == MODE_HAM )
                forcemode = MODE_NONE;
        }
        else
        if( pm_keymatch(argv[argn], "-24if", 4) )
            ifmode = MODE_24;
        else
        if( pm_keymatch(argv[argn], "-no24if", 6) ) {
            if( ifmode == MODE_24 )
                ifmode = MODE_NONE;
        }
        else
        if( pm_keymatch(argv[argn], "-24force", 4) )
            forcemode = MODE_24;
        else
        if( pm_keymatch(argv[argn], "-no24force", 6) ) {
            if( forcemode == MODE_24 )
                forcemode = MODE_NONE;
        }
        else
        if( pm_keymatch(argv[argn], "-dcif", 4) ) {
            ifmode = MODE_DIRECT;
        }
        else
        if( pm_keymatch(argv[argn], "-nodcif", 6) ) {
            if( ifmode == MODE_DIRECT )
                ifmode = MODE_NONE;
        }
        else
        if( pm_keymatch(argv[argn], "-dcforce", 4) ) {
            forcemode = MODE_DIRECT;
        }
        else
        if( pm_keymatch(argv[argn], "-nodcforce", 6) ) {
            if( forcemode == MODE_DIRECT )
                forcemode = MODE_NONE;
        }
        else
        if( pm_keymatch(argv[argn], "-dcbits", 4) || pm_keymatch(argv[argn], "-dcplanes", 4) ) {
            char *option = argv[argn];

            if( ++argn >= argc )
                pm_usage(usage);
            dcol.r = (unsigned char) get_int_val(argv[argn], option, 1, MAXPLANES);
            if( ++argn >= argc )
                pm_usage(usage);
            dcol.g = (unsigned char) get_int_val(argv[argn], option, 1, MAXPLANES);
            if( ++argn >= argc )
                pm_usage(usage);
            dcol.b = (unsigned char) get_int_val(argv[argn], option, 1, MAXPLANES);
        }
        else
        if( pm_keymatch(argv[argn], "-normal", 4) ) {
            ifmode = forcemode = MODE_NONE;
            compr_type = DEF_COMPRESSION;
        }
        else
        if( pm_keymatch(argv[argn], "-compress", 3) ) {
            compr_force = 1;
            if( compr_type == cmpNone )
                if( DEF_COMPRESSION == cmpNone )
                    compr_type = cmpByteRun1;
                else
                    compr_type = DEF_COMPRESSION;
        }
        else
        if( pm_keymatch(argv[argn], "-nocompress", 4) ) {
            compr_force = 0;
            compr_type = cmpNone;
        }
        else
        if( pm_keymatch(argv[argn], "-cmethod", 4) ) {
            if( ++argn >= argc )
                pm_usage(usage);
            compr_type = get_compr_type(argv[argn]);
        }
        else
        if( pm_keymatch(argv[argn], "-savemem", 2) )
            savemem = 1;
        else
        if( pm_keymatch(argv[argn], "-fs1", 4) )    /* EXPERIMENTAL */
            floyd = 2;
        else
        if( pm_keymatch(argv[argn], "-floyd", 3) || pm_keymatch(argv[argn], "-fs", 3) )
            floyd = 1;
        else
        if( pm_keymatch(argv[argn], "-nofloyd", 5) || pm_keymatch(argv[argn], "-nofs", 5) )
            floyd = 0;
        else
            pm_usage(usage);
        ++argn;
    }

    if( argn < argc ) {
        ifp = pm_openr(argv[argn]);
        ++argn;
    }
    else
        ifp = stdin;

    if( argn != argc )
        pm_usage( usage );

    if( forcemode != MODE_NONE && mapfile != NULL )
        pm_message("warning - mapfile only used for normal ILBMs");

    mode = forcemode;
    switch( forcemode ) {
        case MODE_HAM:
            /* grayscale colormap for now - we don't need to read the whole
               file into memory and can use row-by-row operation */
            init_read(ifp, &cols, &rows, &maxval, &format, 0);
            pm_message("hamforce option used - proceeding to write a HAM%d file", hambits);
            break;
        case MODE_24:
            init_read(ifp, &cols, &rows, &maxval, &format, 0);
            pm_message("24force option used - proceeding to write a 24bit file");
            break;
        case MODE_DIRECT:
            init_read(ifp, &cols, &rows, &maxval, &format, 0);
            pm_message("dcforce option used - proceeding to write a %d:%d:%d direct color file",
                        dcol.r, dcol.g, dcol.b);
            break;
        case MODE_CMAP:
            /* must read the whole file into memory */
            init_read(ifp, &cols, &rows, &maxval, &format, 1);

            /* Figure out the colormap. */
            pm_message("computing colormap...");
            chv = ppm_computecolorhist(pixels, cols, rows, MAXCMAPCOLORS, &colors);
            if( chv == (colorhist_vector)NULL )
                pm_error("too many colors - try doing a 'ppmquant %d'", MAXCMAPCOLORS);
            pm_message("%d colors found", colors);
            break;
        default:
            /* must read the whole file into memory */
            init_read(ifp, &cols, &rows, &maxval, &format, 1);

            /* Figure out the colormap. */
            if( mapfile ) {
                int mapcols, maprows, row, col;
                pixel **mappixels, *pP;
                pixval mapmaxval;
                FILE *mapfp;

                pm_message("reading colormap file...");
                mapfp = pm_openr(mapfile);
                mappixels = ppm_readppm(mapfp, &mapcols, &maprows, &mapmaxval);
                pm_close(mapfp);
                if( mapcols == 0 || maprows == 0 )
                    pm_error("null colormap??");

                /* if the maxvals of the ppmfile and the mapfile are the same,
                 * then the scaling to MAXCOLVAL (if necessary) will be done by
                 * the write_std_cmap() function.
                 * Otherwise scale them both to MAXCOLVAL.
                 */
                if( maxval != mapmaxval ) {
                    if( mapmaxval != MAXCOLVAL ) {
                        pixval *table;
                        pm_message("colormap maxval is not %d - rescaling colormap...", MAXCOLVAL);
                        table = make_val_table(mapmaxval, MAXCOLVAL);
                        for( row = 0; row < maprows; ++row )
                            for( col = 0, pP = mappixels[row]; col < mapcols; ++col, ++pP )
                                NEWDEPTH(*pP, table);   /* was PPM_DEPTH( *pP, *pP, mapmaxval, MAXCOLVAL ); */
                        mapmaxval = MAXCOLVAL;
                        free(table);
                    }

                    if( maxval != mapmaxval ) {
                        pixval *table;
                        pm_message("rescaling colors of picture...");
                        table = make_val_table(maxval, mapmaxval);
                        for( row = 0; row < rows; ++row )
                            for( col = 0, pP = pixels[row]; col < cols; ++col, ++pP )
                                NEWDEPTH(*pP, table);   /* was PPM_DEPTH( *pP, *pP, maxval, mapmaxval ); */
                        maxval = mapmaxval;
                        free(table);
                    }
                }

                pm_message("computing colormap...");
                chv = ppm_computecolorhist(mappixels, mapcols, maprows, MAXCMAPCOLORS, &colors);
                ppm_freearray(mappixels, maprows);
                if( chv == (colorhist_vector)0 )
                    pm_error("too many colors in colormap!");
                pm_message("%d colors found in colormap", colors);

                nPlanes = fixplanes = maxplanes = colorstobpp(colors);
            }
            else {  /* no mapfile */
                pm_message("computing colormap...");
                chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &colors);
                if( chv == (colorhist_vector)0 ) {
                    /* too many colors */
                    mode = ifmode;
                    switch( ifmode ) {
                        case MODE_HAM:
                            pm_message("too many colors - proceeding to write a HAM%d file", hambits);
                            pm_message("if you want a non-HAM file, try doing a 'ppmquant %d'", MAXCOLORS);
                            break;
                        case MODE_24:
                            pm_message("too many colors - proceeding to write a 24bit file" );
                            pm_message("if you want a non-24bit file, try doing a 'ppmquant %d'", MAXCOLORS);
                            break;
                        case MODE_DIRECT:
                            pm_message("too many colors - proceeding to write a %d:%d:%d direct color file",
                                        dcol.r, dcol.g, dcol.b);
                            pm_message("if you want a non-direct-color file, try doing a 'ppmquant %d'", MAXCOLORS);
                            break;
                        default:
                            pm_message( "too many colors for %d planes", maxplanes );
                            pm_message( "either use -hamif/-hamforce/-24if/-24force/-dcif/-dcforce/-maxplanes,");
                            pm_error( "or try doing a 'ppmquant %d'", MAXCOLORS );
                            break;
                    }
                }
                else {
                    pm_message("%d colors found", colors);
                    nPlanes = colorstobpp(colors);
                    if( fixplanes > nPlanes )
                        nPlanes = fixplanes;
                }
            }
            break;
    }

    if( mode != MODE_CMAP ) {
        register int i;
        coded_rowbuf = MALLOC(RowBytes(cols), unsigned char);
        for( i = 0; i < RowBytes(cols); i++ )
            coded_rowbuf[i] = 0;
        if( DO_COMPRESS )
            compr_rowbuf = MALLOC(WORSTCOMPR(RowBytes(cols)), unsigned char);
    }

    switch( mode ) {
        case MODE_HAM: {
            int nocolor;

            nocolor = !(PPM_FORMAT_TYPE(format) == PPM_TYPE);
            if( nocolor )
                floyd = 0;

            viewportmodes |= vmHAM;
            ppm_to_ham(ifp, cols, rows, maxval, hambits, nocolor);
            }
            break;
        case MODE_24:
            ppm_to_24(ifp, cols, rows, maxval);
            break;
        case MODE_DIRECT:
            ppm_to_direct(ifp, cols, rows, maxval, &dcol);
            break;
        case MODE_CMAP:
            ppm_to_cmap(maxval, chv, colors);
            break;
        default:
            if( mapfile == NULL )
                floyd = 0;          /* would only slow down conversion */
            ppm_to_std(ifp, cols, rows, maxval, chv, colors, nPlanes);
            break;
    }
    pm_close(ifp);
    exit(0);
    /*NOTREACHED*/
}


static int
get_int_val(string, option, bot, top)
    char *string, *option;
    int bot, top;
{
    int val;

    if( sscanf(string, "%d", &val) != 1 )
        pm_error("option \"%s\" needs integer argument", option);

    if( val < bot || val > top )
        pm_error("option \"%s\" argument value out of range (%d..%d)", option, bot, top);

    return val;
}


static int
get_compr_type(string)
    char *string;
{
    int i;

    for( i = 0; i <= cmpMAXKNOWN; i++ ) {
        if( strcmp(string, cmpNAME[i]) == 0 )
            return i;
    }
    pm_message("unknown compression method: %s", string);
    pm_message("using default compression (%s)", cmpNAME[DEF_COMPRESSION]);
    return DEF_COMPRESSION;
}


/************ colormap file ************/

static void
ppm_to_cmap(maxval, chv, colors)
    int maxval;
    colorhist_vector chv;
    int colors;
{
    int formsize, cmapsize;

    cmapsize = colors * 3;

    formsize =
        4 +                                 /* ILBM */
        4 + 4 + BitMapHeaderSize +          /* BMHD size header */
        4 + 4 + cmapsize + PAD(cmapsize);   /* CMAP size colormap */

    write_form_ilbm(formsize);
    write_bmhd(0, 0, 0);
    write_std_cmap(chv, colors, maxval);
}

/************ HAM ************/

static long do_ham_body     ARGS((FILE *ifp, FILE *ofp, int cols, int rows, pixval maxval, pixval hammaxval, int nPlanes, int colbits, int no));

static void
ppm_to_ham(fp, cols, rows, maxval, hambits, nocolor)
    FILE *fp;
    int cols, rows, maxval, hambits, nocolor;
{
    int colors, colbits, nPlanes, i, hammaxval;
    long oldsize, bodysize, formsize, cmapsize;
    pixval *table = NULL;

    colbits = hambits-2;
    colors = 1 << colbits;
    hammaxval = pm_bitstomaxval(colbits);
    nPlanes = hambits;
    cmapsize = colors * 3;

    bodysize = oldsize = rows * nPlanes * RowBytes(cols);
    if( DO_COMPRESS ) {
        alloc_body_array(rows, nPlanes);
        bodysize = do_ham_body(fp, NULL, cols, rows, maxval, hammaxval, nPlanes, colbits, nocolor);
        if( bodysize > oldsize )
            pm_message("warning - %s compression increases BODY size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
        else
            pm_message("BODY compression (%s): %d%%", cmpNAME[compr_type], 100*(oldsize-bodysize)/oldsize);
#if 0
        if( bodysize > oldsize && !compr_force ) {
            pm_message("%s compression would increase body size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
            pm_message("writing uncompressed image");
            free_body_array();
            compr_type = cmpNone;
            bodysize = oldsize;
        }
#endif
    }


    formsize =
        4 +                                 /* ILBM */
        4 + 4 + BitMapHeaderSize +          /* BMHD size header */
        CAMGSIZE +                          /* 0 or CAMG size val */
        4 + 4 + cmapsize + PAD(cmapsize) +  /* CMAP size colormap */
        4 + 4 + bodysize + PAD(bodysize);   /* BODY size data */

    write_form_ilbm(formsize);
    write_bmhd(cols, rows, nPlanes);
    write_camg();

    /* write grayscale colormap */
    put_fourchars("CMAP");
    put_big_long(cmapsize);
    table = make_val_table(hammaxval, MAXCOLVAL);
    for( i = 0; i < colors; i++ ) {
        put_byte( table[i] );   /* red */
        put_byte( table[i] );   /* green */
        put_byte( table[i] );   /* blue */
    }
    free(table);
    if( odd(cmapsize) )
        put_byte(0);

    /* write body */
    put_fourchars("BODY");
    put_big_long(bodysize);
    if( DO_COMPRESS )
        write_body();
    else
        do_ham_body(fp, stdout, cols, rows, maxval, hammaxval, nPlanes, colbits, nocolor);
    if( odd(bodysize) )
        put_byte(0);
}


static long
do_ham_body(ifp, ofp, cols, rows, maxval, hammaxval, nPlanes, colbits, nocolor)
    FILE *ifp, *ofp;
    int cols, rows;
    pixval maxval, hammaxval;
    int nPlanes, colbits, nocolor;
{
    register int col, row;
    pixel *pP;
    pixval *table = NULL;
    rawtype *raw_rowbuf;
    floydinfo *fi;
    long bodysize = 0;

    raw_rowbuf = MALLOC(cols, rawtype);

    if( hammaxval != maxval )
        table = make_val_table(maxval, hammaxval);

    if( floyd ) {
        fi = init_floyd(cols, maxval, 0);
    }

    for( row = 0; row < rows; row++ ) {
        register int noprev, pr, pg, pb, r, g, b, l;
        int fpr, fpg, fpb;      /* unscaled previous color values, for floyd */

        pP = next_pixrow(ifp, row);
        if( floyd )
            begin_floyd_row(fi, pP);

        noprev = 1;
        for( col = 0; col < cols; col++, pP++ ) {
            int fr, fg, fb, fl;     /* unscaled color values, for floyd */
            if( floyd )
                pP = next_floyd_pixel(fi);

            r = fr = PPM_GETR( *pP );
            g = fg = PPM_GETG( *pP );
            b = fb = PPM_GETB( *pP );
            if( maxval <= 255 ) /* Use fast approximation to 0.299 r + 0.587 g + 0.114 b. */
                l = fl = (int)((times77[r] + times150[g] + times29[b] + 128) >> 8);
            else /* Can't use fast approximation, so fall back on floats. */
                l = fl = (int)(PPM_LUMIN(*pP) + 0.5); /* -IUW added '+ 0.5' */

            if( table ) {
                r = table[r];
                g = table[g];
                b = table[b];
                l = table[l];
            }

            if( noprev || nocolor ) {
                /* No previous pixels, gotta use the gray option. */
                raw_rowbuf[col] = l /* + (HAMCODE_CMAP << colbits) */;
                pr = pg = pb = l;
                fpr = fpg = fpb = fl;
                noprev = 0;
            }
            else {
                register int dred, dgreen, dblue, dgray;
                /* Compute distances for the four options. */
                dred = abs( g - pg ) + abs( b - pb );
                dgreen = abs( r - pr ) + abs( b - pb );
                dblue = abs( r - pr ) + abs( g - pg );
                dgray = abs( r - l ) + abs( g - l ) + abs( b - l );

                /* simply doing  raw_rowbuf[col] = ...
                 * is ok here because there is no fs-alternation performed
                 * for HAM.  Otherwise we would have to do
                 *     if( floyd )  raw_rowbuf[fi->col] = ...
                 *     else         raw_rowbuf[col] = ...
                 */
                if( dgray <= dred && dgray <= dgreen && dgray <= dblue ) {      /* -IUW  '<=' was '<'  */
                    raw_rowbuf[col] = l /* + (HAMCODE_CMAP << colbits) */;
                    pr = pg = pb = l;
                    fpr = fpg = fpb = fl;
                }
                else
                if( dblue <= dred && dblue <= dgreen ) {
                    raw_rowbuf[col] = b + (HAMCODE_BLUE << colbits);
                    pb = b;
                    fpb = fb;
                }
                else
                if( dred <= dgreen ) {
                    raw_rowbuf[col] = r + (HAMCODE_RED << colbits);
                    pr = r;
                    fpr = fr;
                }
                else {
                    raw_rowbuf[col] = g + (HAMCODE_GREEN << colbits);
                    pg = g;
                    fpg = fg;
                }
            }
            if( floyd )
                update_floyd_pixel(fi, fpr, fpg, fpb);
        }
        bodysize += encode_row(ofp, raw_rowbuf, cols, nPlanes);
        if( floyd )
            end_floyd_row(fi);
    }
    /* clean up */
    if( table )
        free(table);
    free(raw_rowbuf);
    if( floyd )
        free_floyd(fi);

    return bodysize;
}

/************ 24bit ************/

static long do_24_body      ARGS((FILE *ifp, FILE *ofp, int cols, int rows, pixval maxval));

static void
ppm_to_24(fp, cols, rows, maxval)
    FILE *fp;
    int cols, rows, maxval;
{
    int nPlanes;
    long bodysize, oldsize, formsize;

    nPlanes = 24;

    bodysize = oldsize = rows * nPlanes * RowBytes(cols);
    if( DO_COMPRESS ) {
        alloc_body_array(rows, nPlanes);
        bodysize = do_24_body(fp, NULL, cols, rows, maxval);
        if( bodysize > oldsize )
            pm_message("warning - %s compression increases BODY size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
        else
            pm_message("BODY compression (%s): %d%%", cmpNAME[compr_type], 100*(oldsize-bodysize)/oldsize);
#if 0
        if( bodysize > oldsize && !compr_force ) {
            pm_message("%s compression would increase body size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
            pm_message("writing uncompressed image");
            free_body_array();
            compr_type = cmpNone;
            bodysize = oldsize;
        }
#endif
    }


    formsize =
        4 +                                 /* ILBM */
        4 + 4 + BitMapHeaderSize +          /* BMHD size header */
        CAMGSIZE +                          /* 0 or CAMG size val */
        4 + 4 + bodysize + PAD(bodysize);   /* BODY size data */

    write_form_ilbm(formsize);
    write_bmhd(cols, rows, nPlanes);
    write_camg();

    /* write body */
    put_fourchars("BODY");
    put_big_long(bodysize);
    if( DO_COMPRESS )
        write_body();
    else
        do_24_body(fp, stdout, cols, rows, maxval);
    if( odd(bodysize) )
        put_byte(0);
}


static long
do_24_body(ifp, ofp, cols, rows, maxval)
    FILE *ifp, *ofp;
    int cols, rows;
    pixval maxval;
{
    register int row, col;
    pixel *pP;
    pixval *table = NULL;
    long bodysize = 0;
    rawtype *redbuf, *greenbuf, *bluebuf;

    redbuf   = MALLOC(cols, rawtype);
    greenbuf = MALLOC(cols, rawtype);
    bluebuf  = MALLOC(cols, rawtype);

    if( maxval != MAXCOLVAL ) {
        pm_message("maxval is not %d - automatically rescaling colors", MAXCOLVAL);
        table = make_val_table(maxval, MAXCOLVAL);
    }

    for( row = 0; row < rows; row++ ) {
        pP = next_pixrow(ifp, row);
        if( table ) {
            for( col = 0; col < cols; col++, pP++ ) {
                redbuf[col]     = table[PPM_GETR(*pP)];
                greenbuf[col]   = table[PPM_GETG(*pP)];
                bluebuf[col]    = table[PPM_GETB(*pP)];
            }
        }
        else {
            for( col = 0; col < cols; col++, pP++ ) {
                redbuf[col]     = PPM_GETR(*pP);
                greenbuf[col]   = PPM_GETG(*pP);
                bluebuf[col]    = PPM_GETB(*pP);
            }
        }
        bodysize += encode_row(ofp, redbuf,   cols, 8);
        bodysize += encode_row(ofp, greenbuf, cols, 8);
        bodysize += encode_row(ofp, bluebuf,  cols, 8);
    }
    /* clean up */
    if( table )
        free(table);
    free(redbuf);
    free(greenbuf);
    free(bluebuf);

    return bodysize;
}


/************ direct color ************/

static long do_direct_body  ARGS((FILE *ifp, FILE *ofp, int cols, int rows, pixval maxval, DirectColor *dcol));

static void
ppm_to_direct(fp, cols, rows, maxval, dcol)
    FILE *fp;
    int cols, rows, maxval;
    DirectColor *dcol;
{
    int nPlanes;
    long formsize, bodysize, oldsize;

    nPlanes = dcol->r + dcol->g + dcol->b;

    bodysize = oldsize = rows * nPlanes * RowBytes(cols);
    if( DO_COMPRESS ) {
        alloc_body_array(rows, nPlanes);
        bodysize = do_direct_body(fp, NULL, cols, rows, maxval, dcol);
        if( bodysize > oldsize )
            pm_message("warning - %s compression increases BODY size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
        else
            pm_message("BODY compression (%s): %d%%", cmpNAME[compr_type], 100*(oldsize-bodysize)/oldsize);
#if 0
        if( bodysize > oldsize && !compr_force ) {
            pm_message("%s compression would increase body size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
            pm_message("writing uncompressed image");
            free_body_array();
            compr_type = cmpNone;
            bodysize = oldsize;
        }
#endif
    }

    formsize =
        4 +                                 /* ILBM */
        4 + 4 + BitMapHeaderSize +          /* BMHD size header */
        CAMGSIZE +                          /* 0 or CAMG size val */
        4 + 4 + DirectColorSize +           /* DCOL size description */
        4 + 4 + bodysize + PAD(bodysize);   /* BODY size data */

    write_form_ilbm(formsize);
    write_bmhd(cols, rows, nPlanes);
    write_camg();

    /* write DCOL */
    put_fourchars("DCOL");
    put_big_long(DirectColorSize);
    put_byte(dcol->r);
    put_byte(dcol->g);
    put_byte(dcol->b);
    put_byte(0);    /* pad */

    /* write body */
    put_fourchars("BODY");
    put_big_long(bodysize);
    if( DO_COMPRESS )
        write_body();
    else
        do_direct_body(fp, stdout, cols, rows, maxval, dcol);
    if( odd(bodysize) )
        put_byte(0);
}


static long
do_direct_body(ifp, ofp, cols, rows, maxval, dcol)
    FILE *ifp, *ofp;
    int cols, rows;
    pixval maxval;
    DirectColor *dcol;
{
    register int row, col;
    pixel *pP;
    pixval *redtable = NULL, *greentable = NULL, *bluetable = NULL;
    pixval redmaxval, greenmaxval, bluemaxval;
    rawtype *redbuf, *greenbuf, *bluebuf;
    long bodysize = 0;

    redbuf   = MALLOC(cols, rawtype);
    greenbuf = MALLOC(cols, rawtype);
    bluebuf  = MALLOC(cols, rawtype);

    redmaxval   = pm_bitstomaxval(dcol->r);
    if( redmaxval != maxval ) {
        pm_message("rescaling reds to %d bits", dcol->r);
        redtable = make_val_table(maxval, redmaxval);
    }
    greenmaxval = pm_bitstomaxval(dcol->g);
    if( greenmaxval != maxval ) {
        pm_message("rescaling greens to %d bits", dcol->g);
        greentable = make_val_table(maxval, greenmaxval);
    }
    bluemaxval  = pm_bitstomaxval(dcol->b);
    if( bluemaxval != maxval ) {
        pm_message("rescaling blues to %d bits", dcol->b);
        bluetable = make_val_table(maxval, bluemaxval);
    }

    for( row = 0; row < rows; row++ ) {
        pP = next_pixrow(ifp, row);
        for( col = 0; col < cols; col++, pP++ ) {
            register pixval r, g, b;

            r = PPM_GETR(*pP); if( redtable ) r = redtable[r];
            g = PPM_GETG(*pP); if( greentable ) g = greentable[g];
            b = PPM_GETB(*pP); if( bluetable ) b = bluetable[b];

            redbuf[col] = r;
            greenbuf[col] = g;
            bluebuf[col] = b;
        }
        bodysize += encode_row(ofp, redbuf,   cols, dcol->r);
        bodysize += encode_row(ofp, greenbuf, cols, dcol->g);
        bodysize += encode_row(ofp, bluebuf,  cols, dcol->b);
    }
    /* clean up */
    if( redtable )
        free(redtable);
    if( greentable )
        free(greentable);
    if( bluetable )
        free(bluetable);
    free(redbuf);
    free(greenbuf);
    free(bluebuf);

    return bodysize;
}


/************ normal colormapped ************/

static long do_std_body     ARGS((FILE *ifp, FILE *ofp, int cols, int rows, pixval maxval, colorhist_vector chv, int colors, int nPlanes));

static void
ppm_to_std(fp, cols, rows, maxval, chv, colors, nPlanes)
    FILE *fp;
    int cols, rows, maxval;
    colorhist_vector chv;
    int colors, nPlanes;
{
    long formsize, cmapsize, bodysize, oldsize;

    bodysize = oldsize = rows * nPlanes * RowBytes(cols);
    if( DO_COMPRESS ) {
        alloc_body_array(rows, nPlanes);
        bodysize = do_std_body(fp, NULL, cols, rows, maxval, chv, colors, nPlanes);
        if( bodysize > oldsize )
            pm_message("warning - %s compression increases BODY size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
        else
            pm_message("BODY compression (%s): %d%%", cmpNAME[compr_type], 100*(oldsize-bodysize)/oldsize);
#if 0
        if( bodysize > oldsize && !compr_force ) {
            pm_message("%s compression would increase body size by %d%%", cmpNAME[compr_type], 100*(bodysize-oldsize)/oldsize);
            pm_message("writing uncompressed image");
            free_body_array();
            compr_type = cmpNone;
            bodysize = oldsize;
        }
#endif
    }

    cmapsize = colors * 3;

    formsize =
        4 +                                 /* ILBM */
        4 + 4 + BitMapHeaderSize +          /* BMHD size header */
        CAMGSIZE +                          /* 0 or CAMG size val */
        4 + 4 + cmapsize + PAD(cmapsize) +  /* CMAP size colormap */
        4 + 4 + bodysize + PAD(bodysize);   /* BODY size data */

    write_form_ilbm(formsize);
    write_bmhd(cols, rows, nPlanes);
    write_camg();
    write_std_cmap(chv, colors, maxval);

    /* write body */
    put_fourchars("BODY");
    put_big_long(bodysize);
    if( DO_COMPRESS )
        write_body();
    else
        do_std_body(fp, stdout, cols, rows, maxval, chv, colors, nPlanes);
    if( odd(bodysize) )
        put_byte(0);
}


static long
do_std_body(ifp, ofp, cols, rows, maxval, chv, colors, nPlanes)
    FILE *ifp, *ofp;
    int cols, rows;
    pixval maxval;
    colorhist_vector chv;
    int colors, nPlanes;
{
    colorhash_table cht;
    register int row, col;
    pixel *pP;
    rawtype *raw_rowbuf;
    floydinfo *fi;
    long bodysize = 0;
    short usehash = !savemem;

    raw_rowbuf = MALLOC(cols, rawtype);

    /* Make a hash table for fast color lookup. */
    cht = ppm_colorhisttocolorhash(chv, colors);

    if( floyd )
        fi = init_floyd(cols, maxval, 1);

    for( row = 0; row < rows; row++ ) {
        pP = next_pixrow(ifp, row);
        if( floyd )
            begin_floyd_row(fi, pP);

        for( col = 0; col < cols; col++, pP++ ) {
            int ind;

            if( floyd )
                pP = next_floyd_pixel(fi);

            /* Check hash table to see if we have already matched this color. */
            ind = ppm_lookupcolor(cht, pP);
            if( ind == -1 ) {
                ind = closest_color(chv, colors, maxval, pP);   /* No; search colormap for closest match. */
                if( usehash ) {
                    if( ppm_addtocolorhash(cht, pP, ind) < 0 ) {
                        pm_message("out of memory adding to hash table, proceeding without it");
                        usehash = 0;
                    }
                }
            }
            if( floyd ) {
                raw_rowbuf[fi->col] = ind;
                update_floyd_pixel(fi, (int)PPM_GETR(chv[ind].color), (int)PPM_GETG(chv[ind].color), (int)PPM_GETB(chv[ind].color));
            }
            else
                raw_rowbuf[col] = ind;
        }
        if( floyd )
            end_floyd_row(fi);
        bodysize += encode_row(ofp, raw_rowbuf, cols, nPlanes);
    }
    /* clean up */
    free(raw_rowbuf);
    ppm_freecolorhash(cht);
    if( floyd )
        free_floyd(fi);

    return bodysize;
}

/************ multipalette ************/

#ifdef ILBM_PCHG
static pixel *ppmslice[2];  /* need 2 for laced ILBMs, else 1 */

void ppm_to_pchg()
{
/*
    read first slice
    build a colormap from this slice
    select upto <maxcolors> colors
    build colormap from selected colors
    map slice to colormap
    write slice
    while( !finished ) {
        read next slice
        compute distances for each pixel and select upto
            <maxchangesperslice> unused colors in this slice
        modify selected colors to the ones with maximum(?) distance
        map slice to colormap
        write slice
    }


    for HAM use a different mapping:
        compute distance to closest color in colormap
        if( there is no matching color in colormap ) {
            compute distances for the three "modify" cases
            use the shortest distance from the four cases
        }
*/
}
#endif

/************ ILBM functions ************/

static void
write_std_cmap(chv, colors, maxval)
    colorhist_vector chv;
    int colors, maxval;
{
    int cmapsize, i;

    cmapsize = 3 * colors;

    /* write colormap */
    put_fourchars("CMAP");
    put_big_long(cmapsize);
    if( maxval != MAXCOLVAL ) {
        pixval *table;
        pm_message("maxval is not %d - automatically rescaling colors", MAXCOLVAL);
        table = make_val_table(maxval, MAXCOLVAL);
        for( i = 0; i < colors; i++ ) {
            put_byte(table[PPM_GETR(chv[i].color)]);
            put_byte(table[PPM_GETG(chv[i].color)]);
            put_byte(table[PPM_GETB(chv[i].color)]);
        }
        free(table);
    }
    else {
        for( i = 0; i < colors; i++ ) {
            put_byte(PPM_GETR(chv[i].color));
            put_byte(PPM_GETG(chv[i].color));
            put_byte(PPM_GETB(chv[i].color));
        }
    }
    if( odd(cmapsize) )
        put_byte(0);
}


static void
write_form_ilbm(size)
    int size;
{
    put_fourchars("FORM");
    put_big_long(size);
    put_fourchars("ILBM");
}


static void
write_bmhd(cols, rows, nPlanes)
    int cols, rows, nPlanes;
{
    unsigned int xasp = 10, yasp = 10;

    if( viewportmodes & vmLACE )
        xasp *= 2;
    if( viewportmodes & vmHIRES )
        yasp *= 2;

    put_fourchars("BMHD");
    put_big_long(BitMapHeaderSize);

    put_big_short(cols);
    put_big_short(rows);
    put_big_short(0);                       /* x-offset */
    put_big_short(0);                       /* y-offset */
    put_byte(nPlanes);                      /* no of planes */
    put_byte(mskNone);                      /* masking type */
    put_byte((unsigned char)compr_type);    /* compression type */
    put_byte(0);                            /* pad1 */
    put_big_short(0);                       /* tranparentColor */
    put_byte(xasp);                         /* x-aspect */
    put_byte(yasp);                         /* y-aspect */
    put_big_short(cols);                    /* pageWidth */
    put_big_short(rows);                    /* pageHeight */
}


/* encode algorithm by Johan Widen (jw@jwdata.se) */
const unsigned char ppmtoilbm_bitmask[] = {1, 2, 4, 8, 16, 32, 64, 128};

static int
encode_row(outfile, rawrow, cols, nPlanes)
    FILE *outfile;  /* if non-NULL, write uncompressed row to this file */
    rawtype *rawrow;
    int cols, nPlanes;
{
    int plane, bytes;
    long retbytes = 0;

    bytes = RowBytes(cols);

    /* Encode and write raw bytes in plane-interleaved form. */
    for( plane = 0; plane < nPlanes; plane++ ) {
        register int col, cbit;
        register rawtype *rp;
        register unsigned char *cp;
        int mask;

        mask = 1 << plane;
        cbit = -1;
        cp = coded_rowbuf-1;
        rp = rawrow;
        for( col = 0; col < cols; col++, cbit--, rp++ ) {
            if( cbit < 0 ) {
                cbit = 7;
                *++cp = 0;
            }
            if( *rp & mask )
                *cp |= ppmtoilbm_bitmask[cbit];
        }
        if( outfile ) {
            retbytes += bytes;
            if( fwrite(coded_rowbuf, 1, bytes, stdout) != bytes )
                pm_error("write error");
        }
        else
            retbytes += compress_row(bytes);
    }
    return retbytes;
}


static int
compress_row(bytes)
    int bytes;
{
    static int count;
    int newbytes;

    /* if new compression methods are defined, do a switch here */
    newbytes = runbyte1(bytes);

    if( savemem ) {
        ilbm_body[count].row = MALLOC(newbytes, unsigned char);
        bcopy(compr_rowbuf, ilbm_body[count].row, newbytes);
    }
    else {
        ilbm_body[count].row = compr_rowbuf;
        compr_rowbuf = MALLOC(WORSTCOMPR(bytes), unsigned char);
    }
    ilbm_body[count].len = newbytes;
    ++count;

    return newbytes;
}


static void
write_body ARGS((void))
{
    bodyrow *p;

    for( p = ilbm_body; p->row != NULL; p++ ) {
        if( fwrite(p->row, 1, p->len, stdout) != p->len )
            pm_error("write error");
    }
    /* pad byte (if neccessary) is written by do_xxx_body() function */
}


static void
write_camg ARGS((void))
{
    if( viewportmodes ) {
        put_fourchars("CAMG");
        put_big_long(CAMGChunkSize);
        put_big_long(viewportmodes);
    }
}


/************ compression ************/


/* runbyte1 algorithm by Robert A. Knop (rknop@mop.caltech.edu) */
static int
runbyte1(size)
   int size;
{
    int in,out,count,hold;
    register unsigned char *inbuf = coded_rowbuf;
    register unsigned char *outbuf = compr_rowbuf;


    in=out=0;
    while( in<size ) {
        if( (in<size-1) && (inbuf[in]==inbuf[in+1]) ) {     /*Begin replicate run*/
            for( count=0,hold=in; in<size && inbuf[in]==inbuf[hold] && count<128; in++,count++)
                ;
            outbuf[out++]=(unsigned char)(char)(-count+1);
            outbuf[out++]=inbuf[hold];
        }
        else {  /*Do a literal run*/
            hold=out; out++; count=0;
            while( ((in>=size-2)&&(in<size)) || ((in<size-2) && ((inbuf[in]!=inbuf[in+1])||(inbuf[in]!=inbuf[in+2]))) ) {
                outbuf[out++]=inbuf[in++];
                if( ++count>=128 )
                    break;
            }
            outbuf[hold]=count-1;
        }
    }
    return(out);
}


/************ PPM functions ************/


static int
closest_color(chv, colors, cmaxval, pP)
    colorhist_vector chv;
    int colors;
    pixval cmaxval;
    pixel *pP;
{
    /* Search colormap for closest match.       */
    /* algorithm taken from ppmquant.c   -IUW   */
    register int i, r1, g1, b1;
    int ind;
    long dist;

    r1 = PPM_GETR(*pP);
    g1 = PPM_GETG(*pP);
    b1 = PPM_GETB(*pP);
    dist = 2000000000;
    for( i = 0; i < colors; i++ ) {
        register int r2, g2, b2;
        long newdist;

        r2 = PPM_GETR(chv[i].color);
        g2 = PPM_GETG(chv[i].color);
        b2 = PPM_GETB(chv[i].color);
        newdist = ( r1 - r2 ) * ( r1 - r2 ) +
                  ( g1 - g2 ) * ( g1 - g2 ) +
                  ( b1 - b2 ) * ( b1 - b2 );

        if( newdist < dist ) {
            ind = i;
            dist = newdist;
        }
    }
    return ind;
}


/************ floyd-steinberg error diffusion ************/

static floydinfo *
init_floyd(cols, maxval, alternate)
    int cols;
    pixval maxval;
    int alternate;
{
    register int i;
    floydinfo *fi;

    fi = MALLOC(1, floydinfo);

    fi->thisrederr  = MALLOC(cols + 2, long);
    fi->thisgreenerr= MALLOC(cols + 2, long);
    fi->thisblueerr = MALLOC(cols + 2, long);
    fi->nextrederr  = MALLOC(cols + 2, long);
    fi->nextgreenerr= MALLOC(cols + 2, long);
    fi->nextblueerr = MALLOC(cols + 2, long);
    fi->lefttoright = 1;
    fi->cols = cols;
    fi->maxval = maxval;
    fi->alternate = alternate;

    for( i = 0; i < cols + 2; i++ )
        fi->thisrederr[i] = fi->thisgreenerr[i] = fi->thisblueerr[i] = 0;

    return fi;
}


static void
free_floyd(fi)
    floydinfo *fi;
{
    free(fi->thisrederr); free(fi->thisgreenerr); free(fi->thisblueerr);
    free(fi->nextrederr); free(fi->nextgreenerr); free(fi->nextblueerr);
    free(fi);
}


static void
begin_floyd_row(fi, prow)
    floydinfo *fi;
    pixel *prow;
{
    register int i;

    fi->pixrow = prow;

    for( i = 0; i < fi->cols + 2; i++ )
        fi->nextrederr[i] = fi->nextgreenerr[i] = fi->nextblueerr[i] = 0;

    if( fi->lefttoright ) {
        fi->col = 0;
        fi->col_end = fi->cols;
    }
    else {
        fi->col = fi->cols - 1;
        fi->col_end = -1;
    }
}


#define FS_GREEN_WEIGHT     1
#define FS_RED_WEIGHT       2   /* luminance of component relative to green */
#define FS_BLUE_WEIGHT      5

static pixel *
next_floyd_pixel(fi)
    floydinfo *fi;
{
    register long r, g, b;
    register pixel *pP;
    int errcol = fi->col+1;
    pixval maxval = fi->maxval;

#ifdef DEBUG
    if( fi->col == fi->col_end )
        pm_error("fs - access out of array bounds");    /* should never happen */
#endif

    pP = &(fi->pixrow[fi->col]);

    /* Use Floyd-Steinberg errors to adjust actual color. */
    r = fi->thisrederr  [errcol]; if( r < 0 ) r -= 8; else r += 8; r /= 16;
    g = fi->thisgreenerr[errcol]; if( g < 0 ) g -= 8; else g += 8; g /= 16;
    b = fi->thisblueerr [errcol]; if( b < 0 ) b -= 8; else b += 8; b /= 16;

    if( floyd == 2 ) {       /* EXPERIMENTAL */
        r /= FS_RED_WEIGHT;  b /= FS_BLUE_WEIGHT;
    }

    r += PPM_GETR(*pP); if ( r < 0 ) r = 0; else if ( r > maxval ) r = maxval;
    g += PPM_GETG(*pP); if ( g < 0 ) g = 0; else if ( g > maxval ) g = maxval;
    b += PPM_GETB(*pP); if ( b < 0 ) b = 0; else if ( b > maxval ) b = maxval;

    PPM_ASSIGN(*pP, r, g, b);

    fi->red = r;
    fi->green = g;
    fi->blue = b;

    return pP;
}


static void
update_floyd_pixel(fi, r, g, b)
    floydinfo *fi;
    int r, g, b;
{
    register long rerr, gerr, berr, err;
    int col = fi->col;
    int errcol = col+1;
    long two_err;

    rerr = (long)(fi->red)   - r;
    gerr = (long)(fi->green) - g;
    berr = (long)(fi->blue)  - b;

    if( fi->lefttoright ) {
        two_err = 2*rerr;
        err = rerr;     fi->nextrederr[errcol+1] += err;    /* 1/16 */
        err += two_err; fi->nextrederr[errcol-1] += err;    /* 3/16 */
        err += two_err; fi->nextrederr[errcol  ] += err;    /* 5/16 */
        err += two_err; fi->thisrederr[errcol+1] += err;    /* 7/16 */

        two_err = 2*gerr;
        err = gerr;     fi->nextgreenerr[errcol+1] += err;    /* 1/16 */
        err += two_err; fi->nextgreenerr[errcol-1] += err;    /* 3/16 */
        err += two_err; fi->nextgreenerr[errcol  ] += err;    /* 5/16 */
        err += two_err; fi->thisgreenerr[errcol+1] += err;    /* 7/16 */

        two_err = 2*berr;
        err = berr;     fi->nextblueerr[errcol+1] += err;    /* 1/16 */
        err += two_err; fi->nextblueerr[errcol-1] += err;    /* 3/16 */
        err += two_err; fi->nextblueerr[errcol  ] += err;    /* 5/16 */
        err += two_err; fi->thisblueerr[errcol+1] += err;    /* 7/16 */

        fi->col++;
    }
    else {
        two_err = 2*rerr;
        err = rerr;     fi->nextrederr[errcol-1] += err;    /* 1/16 */
        err += two_err; fi->nextrederr[errcol+1] += err;    /* 3/16 */
        err += two_err; fi->nextrederr[errcol  ] += err;    /* 5/16 */
        err += two_err; fi->thisrederr[errcol-1] += err;    /* 7/16 */

        two_err = 2*gerr;
        err = gerr;     fi->nextgreenerr[errcol-1] += err;    /* 1/16 */
        err += two_err; fi->nextgreenerr[errcol+1] += err;    /* 3/16 */
        err += two_err; fi->nextgreenerr[errcol  ] += err;    /* 5/16 */
        err += two_err; fi->thisgreenerr[errcol-1] += err;    /* 7/16 */

        two_err = 2*berr;
        err = berr;     fi->nextblueerr[errcol-1] += err;    /* 1/16 */
        err += two_err; fi->nextblueerr[errcol+1] += err;    /* 3/16 */
        err += two_err; fi->nextblueerr[errcol  ] += err;    /* 5/16 */
        err += two_err; fi->thisblueerr[errcol-1] += err;    /* 7/16 */

        fi->col--;
    }
}


static void
end_floyd_row(fi)
    floydinfo *fi;
{
    long *tmp;

    tmp = fi->thisrederr;   fi->thisrederr   = fi->nextrederr;   fi->nextrederr   = tmp;
    tmp = fi->thisgreenerr; fi->thisgreenerr = fi->nextgreenerr; fi->nextgreenerr = tmp;
    tmp = fi->thisblueerr;  fi->thisblueerr  = fi->nextblueerr;  fi->nextblueerr  = tmp;
    if( fi->alternate )
        fi->lefttoright = !(fi->lefttoright);
}


/************ other utility functions ************/

static void
alloc_body_array(rows, nPlanes)
    int rows, nPlanes;
{
    ilbm_body = MALLOC(rows * nPlanes + 1, bodyrow);
    ilbm_body[rows * nPlanes].row = NULL;
}

#if 0   /* not used for now */
static void
free_body_array ARGS((void))
{
    bodyrow *p;

    for( p = ilbm_body; p->row != NULL; p++ )
        free(p->row);
    free(ilbm_body);
}
#endif

static int
colorstobpp(colors)
    int colors;
{
    int i;

    for( i = 1; i <= MAXPLANES; i++ ) {
        if( colors <= (1 << i) )
            return i;
    }
    pm_error("too many planes (max %d)", MAXPLANES);
    /*NOTREACHED*/
}


#if 0
static void
put_fourchars(str)
    char* str;
{
    fputs( str, stdout );
}
#endif


static void
put_big_short(s)
    short s;
{
    if ( pm_writebigshort( stdout, s ) == -1 )
        pm_error( "write error" );
}


static void
put_big_long(l)
    long l;
{
    if ( pm_writebiglong( stdout, l ) == -1 )
        pm_error( "write error" );
}


#if 0
static void
put_byte(b)
    unsigned char b;
{
    (void) putc( b, stdout );
}
#endif


static pixval *
make_val_table(oldmaxval, newmaxval)
    pixval oldmaxval, newmaxval;
{
    int i;
    pixval *table;

    table = MALLOC(oldmaxval + 1, pixval);
    for(i = 0; i <= oldmaxval; i++ )
        table[i] = (i * newmaxval + oldmaxval/2) / oldmaxval;

    return table;
}


static void *
xmalloc(bytes)
    int bytes;
{
    void *mem;

    mem = malloc(bytes);
    if( mem == NULL )
        pm_error("out of memory allocating %d bytes", bytes);
    return mem;
}


static int  gFormat;
static int  gCols;
static int  gMaxval;

static void
init_read(fp, colsP, rowsP, maxvalP, formatP, readall)
    FILE *fp;
    int *colsP, *rowsP;
    pixval *maxvalP;
    int *formatP;
    int readall;
{
    ppm_readppminit(fp, colsP, rowsP, maxvalP, formatP);
    if( readall ) {
        int row;

        pixels = ppm_allocarray(*colsP, *rowsP);
        for( row = 0; row < *rowsP; row++ )
            ppm_readppmrow(fp, pixels[row], *colsP, *maxvalP, *formatP);
        /* pixels = ppm_readppm(fp, colsP, rowsP, maxvalP); */
    }
    else {
        pixrow = ppm_allocrow(*colsP);
    }
    gCols = *colsP;
    gMaxval = *maxvalP;
    gFormat = *formatP;
}


static pixel *
next_pixrow(fp, row)
    FILE *fp;
    int row;
{
    if( pixels )
        pixrow = pixels[row];
    else {
#ifdef DEBUG
        static int rowcnt;
        if( row != rowcnt )
            pm_error("big mistake");
        rowcnt++;
#endif
        ppm_readppmrow(fp, pixrow, gCols, gMaxval, gFormat);
    }
    return pixrow;
}

