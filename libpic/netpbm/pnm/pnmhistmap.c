/* pnmhistmap.c -
 *	Draw a histogram for a PGM or PPM file
 *
 * Options:	-verbose: the usual
 *		-max N: force scaling value to N
 *		-black: ignore all-black count
 *		-white: ignore all-white count
 *
 * - PGM histogram is a PBM file, PPM histogram is a PPM file
 * - No conditional code - assumes all three: PBM, PGM, PPM
 *
 * Copyright (C) 1993 by Wilson H. Bent, Jr (whb@usc.edu)
 *
 */

#include "pnm.h"

/* prototypes */
static void ppm_hist ARGS((FILE *fp));
static void pgm_hist ARGS((FILE *fp));

#define HIST_WIDTH	256
#define HIST_HEIGHT	200

int rows, hist_cols;
xelval maxval;
int format;

int verbose = 0;
int no_black = 0;
int no_white = 0;

int rhist[HIST_WIDTH];
int ghist[HIST_WIDTH];
int bhist[HIST_WIDTH];
int hmax = -1;
double scale;

int
main (argc, argv)
int argc;
char *argv[];
{
    FILE *ifp;
    int argn = 1;
    char *usage = "[-white] [-black] [-max maxvalue] [-verbose] [pnmfile]";

    pnm_init (&argc, argv);

    /* Check for flags. */
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
	if ( pm_keymatch( argv[argn], "-white", 2 ) )
	    no_white = 1;
	else if ( pm_keymatch( argv[argn], "-black", 2 ) )
	    no_black = 1;
	else if ( pm_keymatch( argv[argn], "-max", 2 ) ) {
	    if ((hmax = atoi (argv[++argn])) <= 0)
		pm_error ("Max. value (%d) must be positive", hmax, 0, 0, 0, 0);
	}
	else if ( pm_keymatch( argv[argn], "-verbose", 2 ) )
	    verbose = 1;
	else
	    pm_usage( usage );
	++argn;
    }

    if (--argc > argn)
	pm_usage (usage);
    else if (argc == argn)
	ifp = pm_openr (argv[argn]);
    else
	ifp = stdin;

    pnm_readpnminit (ifp, &hist_cols, &rows, &maxval, &format);
    switch (PNM_FORMAT_TYPE (format)) {
      case PPM_TYPE:
	ppm_hist (ifp);
	break;
      case PGM_TYPE:
	pgm_hist (ifp);
	break;
      case PBM_TYPE:
	pm_error ("no histograms for PBM files", 0, 0, 0, 0, 0);
	break;
    }
    exit (0);
}

static void
ppm_hist (fp)
FILE *fp;
{
    pixel *pixrow;
    pixel **pixels;
    int i, j;
    int start, finish;

    if ((pixels = ppm_allocarray (HIST_WIDTH, HIST_HEIGHT)) == NULL)
	pm_error ("no space for output array (%d pixels)",
	    HIST_WIDTH * HIST_HEIGHT, 0,0,0,0);
    for (i = 0; i < HIST_HEIGHT; ++i)
	bzero (pixels[i], HIST_WIDTH * sizeof (pixel));
    bzero (rhist, sizeof (rhist));
    bzero (ghist, sizeof (ghist));
    bzero (bhist, sizeof (bhist));

    /* read the pixel values into the histogram arrays */
    pixrow = ppm_allocrow (hist_cols);
    /*XX error-check! */
    if (verbose) pm_message ("making histogram...", 0,0,0,0,0);
    for (i = rows; i > 0; --i) {
	ppm_readppmrow (fp, pixrow, hist_cols, maxval, format);
	for (j = hist_cols-1; j >= 0; --j) {
	    rhist[PPM_GETR(pixrow[j])]++;
	    ghist[PPM_GETG(pixrow[j])]++;
	    bhist[PPM_GETB(pixrow[j])]++;
	}
    }
    ppm_freerow (pixrow);
    fclose (fp);

    /* find the highest-valued slot and set the scale value */
    if (verbose)
	pm_message ("finding max. slot height...", 0, 0, 0, 0, 0);
    if (hmax == -1) {
	start = (no_black ? 1 : 0);
	finish = (no_white ? HIST_WIDTH - 1 : HIST_WIDTH);
	for (hmax = 0, i = start; i < finish; ++i) {
	    if (hmax < rhist[i])
		hmax = rhist[i];
	    if (hmax < ghist[i])
		hmax = ghist[i];
	    if (hmax < bhist[i])
		hmax = bhist[i];
	}
    }
    for (i = 0; i < HIST_WIDTH; ++i) {
	if (rhist[i] > hmax)
	    rhist[i] = hmax;
	if (ghist[i] > hmax)
	    ghist[i] = hmax;
	if (bhist[i] > hmax)
	    bhist[i] = hmax;
    }
    if (verbose)
	pm_message ("Done: height = %d", hmax, 0, 0, 0, 0);
    scale = (double) HIST_HEIGHT / hmax;

    for (i = 0; i < HIST_WIDTH; ++i) {
	for (j = HIST_HEIGHT - (int)(scale * rhist[i]); j < HIST_HEIGHT; ++j)
	    PPM_PUTR(pixels[j][i], maxval);
	for (j = HIST_HEIGHT - (int)(scale * ghist[i]); j < HIST_HEIGHT; ++j)
	    PPM_PUTG(pixels[j][i], maxval);
	for (j = HIST_HEIGHT - (int)(scale * bhist[i]); j < HIST_HEIGHT; ++j)
	    PPM_PUTB(pixels[j][i], maxval);
    }

    ppm_writeppm (stdout, pixels, HIST_WIDTH, HIST_HEIGHT, maxval, 0);
}

static void
pgm_hist (fp)
FILE *fp;
{
    gray *grayrow;
    bit **bits;
    int i, j;
    int start, finish;

    if ((bits = pbm_allocarray (HIST_WIDTH, HIST_HEIGHT)) == NULL)
	pm_error ("no space for output array (%d bits)",
	    HIST_WIDTH * HIST_HEIGHT, 0,0,0,0);
    bzero (ghist, sizeof (ghist));

    /* read the pixel values into the histogram arrays */
    grayrow = pgm_allocrow (hist_cols);
    /*XX error-check! */
    if (verbose) pm_message ("making histogram...", 0,0,0,0,0);
    for (i = rows; i > 0; --i) {
	pgm_readpgmrow (fp, grayrow, hist_cols, maxval, format);
	for (j = hist_cols-1; j >= 0; --j)
	    ghist[grayrow[j]]++;
    }
    pgm_freerow (grayrow);
    fclose (fp);

    /* find the highest-valued slot and set the scale value */
    if (verbose)
	pm_message ("finding max. slot height...", 0, 0, 0, 0, 0);
    if (hmax == -1) {
	start = (no_black ? 1 : 0);
	finish = (no_white ? HIST_WIDTH - 1 : HIST_WIDTH);
	for (hmax = 0, i = start; i < finish; ++i)
	    if (hmax < ghist[i])
		hmax = ghist[i];
    }
    for (i = 0; i < HIST_WIDTH; ++i)
	if (ghist[0] > hmax)
	    ghist[0] = hmax;
    if (verbose)
	pm_message ("Done: height = %d", hmax, 0, 0, 0, 0);
    scale = (double) HIST_HEIGHT / hmax;

    for (i = 0; i < HIST_WIDTH; ++i) {
	int mark = HIST_HEIGHT - (int)(scale * ghist[i]);
	for (j = 0; j < mark; ++j)
	    bits[j][i] = PBM_BLACK;
	for ( ; j < HIST_HEIGHT; ++j)
	    bits[j][i] = PBM_WHITE;
    }

    pbm_writepbm (stdout, bits, HIST_WIDTH, HIST_HEIGHT, 0);
}
