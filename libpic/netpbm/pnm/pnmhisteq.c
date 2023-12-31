/*
			     pnmhisteq.c

	       Equalise histogram for a PGM or PPM file

   Options:	-gray:	  modify gray pixels only; leave colours unchanged
		-rmap fn: read luminosity map from PGM file fn
		-wmap wn: write luminosity map to PGM file fn
		-verbose: print histogram and luminosity map

    Accepts PGM and PPM as input.  PBM input is allowed, but histogram
    equalisation does not modify a PBM file.

	  by John Walker (kelvin@fourmilab.ch) -- March MVM.
	       WWW home page: http://www.fourmilab.ch/

		  Copyright (C) 1995 by John Walker

    Permission	to use, copy, modify, and distribute this software and
    its documentation for  any	purpose  and  without  fee  is	hereby
    granted, without any conditions or restrictions.  This software is
    provided "as is" without express or implied warranty.

*/

#include "pnm.h"

/* Prototypes */

static void hsv_rgb ARGS((double h, double s, double v,
			  double *r, double *g, double *b));
static void rgb_hsv ARGS((double r, double g, double b,
			  double *h, double *s, double *v));

/*  HSV_RGB  --  Convert HSV colour specification to RGB  intensities.
		 Hue is specified as a	real  value  from  0  to  360,
		 Saturation  and  Intensity as reals from 0 to 1.  The
		 RGB components are returned as reals from 0 to 1. */

static void hsv_rgb(h, s, v, r, g, b)
  double h, s, v;
  double *r, *g, *b;
{
    int i;
    double f, p, q, t;

    if (s == 0) {
	*r = *g = *b = v;
    } else {
	if (h == 360.0) {
	    h = 0;
	}
	h /= 60.0;

	i = h;
	f = h - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * f));
	t = v * (1.0 - (s * (1.0 - f)));
	switch (i) {

	    case 0:
		*r = v;
		*g = t;
		*b = p;
		break;

	    case 1:
		*r = q;
		*g = v;
		*b = p;
		break;

	    case 2:
		*r = p;
		*g = v;
		*b = t;
		break;

	    case 3:
		*r = p;
		*g = q;
		*b = v;
		break;

	    case 4:
		*r = t;
		*g = p;
		*b = v;
		break;

	    case 5:
		*r = v;
		*g = p;
		*b = q;
		break;
	 }
    }
}

/*  RGB_HSV  --  Map R, G, B intensities in the range from 0 to 1 into
		 Hue, Saturation,  and	Value:	Hue  from  0  to  360,
		 Saturation  from  0  to  1,  and  Value  from 0 to 1.
                 Special case: if Saturation is 0 (it's a  grey  scale
		 tone), Hue is undefined and is returned as -1.

		 This follows Foley & van Dam, section 17.4.4. */

static void rgb_hsv(r, g, b, h, s, v)
  double r, g, b;
  double *h, *s, *v;
{
    double imax = max(r, max(g, b)),
	   imin = min(r, min(g, b)),
	   rc, gc, bc;

    *v = imax;
    if (imax != 0) {
	*s = (imax - imin) / imax;
    } else {
	*s = 0;
    }

    if (*s == 0) {
	*h = -1;
    } else {
	rc = (imax - r) / (imax - imin);
	gc = (imax - g) / (imax - imin);
	bc = (imax - b) / (imax - imin);
	if (r == imax) {
	    *h = bc - gc;
	} else if (g == imax) {
	    *h = 2.0 + rc - bc;
	} else {
	    *h = 4.0 + gc - rc;
	}
	*h *= 60.0;
	if (*h < 0.0) {
	    *h += 360.0;
	}
    }
}

int main(argc, argv)
  int argc;
  char *argv[];
{
    FILE *ifp;
    int argn = 1, i, j, verbose = 0, mono_only = 0;
    gray lmin, lmax;
    gray **lumamap;		      /* Luminosity map */
    long *lumahist;		      /* Histogram of luminosity values */
    int rows, hist_cols;	      /* Rows, columns of input image */
    xelval maxval;		      /* Maxval of input image */
    int format; 		      /* Format indicator (PBM/PGM/PPM) */
    xel** xels; 		      /* Pixel array */
    unsigned long pixels = 0, pixsum = 0, maxluma = 0;
    double lscale;
    xel *grayrow;
    pixel *pixrow;
    FILE *rmap = NULL, *wmap = NULL;
    char *usage = "[-gray] [-verbose] [-rmap pgmfile] [-wmap pgmfile] [pnmfile]";

    pnm_init(&argc, argv);

    /* Check for flags. */

    while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0') {
        if (pm_keymatch(argv[argn], "-gray", 1)) {
	    mono_only = 1;
        } else if (pm_keymatch(argv[argn], "-verbose", 1)) {
	    verbose = 1;
        } else if (pm_keymatch(argv[argn], "-rmap", 1)) {
	    if (rmap != NULL) {
                pm_error("already specified an input map");
	    }
	    argn++;
            if (argn == argc || strcmp(argv[argn], "-") == 0) {
		pm_usage(usage);
	    }
	    rmap = pm_openr(argv[argn]);
        } else if (pm_keymatch(argv[argn], "-wmap", 1)) {
	    if (wmap != NULL) {
                pm_error("already specified an output map");
	    }
	    argn++;
	    if (argn == argc) {
		pm_usage(usage);
	    }
	    wmap = pm_openw(argv[argn]);
	} else {
	    pm_usage(usage);
	}
	argn++;
    }

    if (--argc > argn) {
	pm_usage(usage);
    } else if (argc == argn) {
	ifp = pm_openr(argv[argn]);
    } else {
	ifp = stdin;
    }

    xels = pnm_readpnm(ifp, &hist_cols, &rows, &maxval, &format);
    pm_close(ifp);

    /* Allocate histogram and luminosity map arrays.  If the
       user has specified an input map file, read it in at
       this point. */

    lumahist = (long *) pm_allocrow(maxval + 1, sizeof(long));
    bzero((char *) lumahist, (maxval + 1) * sizeof(long));

    if (rmap == NULL) {
	lumamap = pgm_allocarray(maxval + 1, 1);
    } else {
	int rmcols, rmrows; 
	gray rmmaxv;

	lumamap = pgm_readpgm(rmap, &rmcols, &rmrows, &rmmaxv);
	if (rmmaxv != maxval) {
            pm_error("maxval in map file (%d) different from input (%d)",
		rmmaxv, maxval);
	}
	if (rmrows != 1 || rmcols != rmmaxv) {
            pm_error("map size (%d by %d) wrong; must be (%d by 1)",
		rmcols, rmrows, maxval);
	}
    }

    /* Scan the image and build the luminosity histogram.  If
       the input is a PPM, we calculate the luminosity of each
       pixel from its RGB components. */

    lmin = maxval;
    lmax = 0;
    if (PNM_FORMAT_TYPE(format) == PGM_TYPE ||
	PNM_FORMAT_TYPE(format) == PBM_TYPE) {

	/* Compute intensity histogram */

	pixels = ((unsigned long) rows) * ((unsigned long) hist_cols);
	for (i = 0; i < rows; i++) {
	    xel *grayrow = xels[i];
	    for (j = 0; j < hist_cols; j++) {
		gray l = PNM_GET1(grayrow[j]);
		lmin = min(lmin, l);
		lmax = max(lmax, l);
		lumahist[l]++;
	    }
	}
    } else if (PNM_FORMAT_TYPE(format) == PPM_TYPE) {
	for (i = 0; i < rows; i++) {
	    pixel *pixrow = (pixel *) xels[i];

	    for (j = 0; j < hist_cols; j++) {
		if (!mono_only ||
		    ((PPM_GETR(pixrow[j]) == PPM_GETG(pixrow[j])) &&
		     (PPM_GETR(pixrow[j]) == PPM_GETB(pixrow[j])))) {
		    gray l = (gray) PPM_LUMIN(pixrow[j]);
		    lmin = min(lmin, l);
		    lmax = max(lmax, l);
		    lumahist[l]++;
		    pixels++;
		}
	    }
	}
    } else {
        pm_error("unknown input format");
    }

    /* The PGM and PPM branches rejoin here to calculate the
       luminosity mapping table which gives the histogram-equalised
       luminosity for each original luminosity. */

    /* Calculate initial histogram equalisation curve. */

    for (i = 0; i <= (int) maxval; i++) {

	/* Yick.  If PGM_BIGGRAYS is defined (I thought they were little
	   guys, about four foot, with funny eyes...) the following
	   calculation can overflow a 32 bit long.  So, we do it in
	   floating point.  Since this happens only maxval times, the
	   inefficiency is trivial compared to the every-pixel code above
	   and below. */

	lumamap[0][i] = (gray) (((((double) pixsum * maxval)) / pixels) + 0.5);
	if (lumahist[i] > 0) {
	    maxluma = i;
	}
	pixsum += lumahist[i];
    }

    /* Normalise so that the brightest pixels are set to
       maxval. */

    lscale = ((double) maxval) / ((lumahist[maxluma] > 0) ?
	     ((double) lumamap[0][maxluma]) : ((double) maxval));
    for (i = 0; i <= (int) maxval; i++) {
	lumamap[0][i] = (gray)
	    min(((long) maxval), ((long) (lumamap[0][i] * lscale + 0.5)));
    }

    /* If requested, print the luminosity map and original histogram. */

    if (verbose) {
	fprintf(stderr,
            "  Luminosity map    Number of\n Original    New     Pixels\n");
	for (i = 0; i <= (int) maxval; i++) {
	    if (lumahist[i] > 0) {
                fprintf(stderr,"%6d -> %6d  %8d\n", i,
			lumamap[0][i], lumahist[i]);
	    }
	}
    }

    switch (PNM_FORMAT_TYPE(format)) {
	case PBM_TYPE:
	case PPM_TYPE:
	    for (i = 0; i < rows; i++) {
		pixrow = (pixel *) xels[i];
		for (j = 0; j < hist_cols; j++) {
		    if (!mono_only ||
			((PPM_GETR(pixrow[j]) == PPM_GETG(pixrow[j])) &&
			 (PPM_GETR(pixrow[j]) == PPM_GETB(pixrow[j])))) {
			double r, g, b, h, s, v;
			int iv;

			r = (double) PPM_GETR(pixrow[j]) / ((double) maxval);
			g = (double) PPM_GETG(pixrow[j]) / ((double) maxval);
			b = (double) PPM_GETB(pixrow[j]) / ((double) maxval);
			rgb_hsv(r, g, b, &h, &s, &v);
			iv = (int) ((v * maxval) + 0.5);

			if (iv > ((int) maxval)) {
			    iv = maxval;
			}
			v = ((double) lumamap[0][iv]) / ((double) maxval);
			if (v > 1.0) {
			    v = 1.0;
			}
			hsv_rgb(h, s, v, &r, &g, &b);
			PPM_ASSIGN(pixrow[j], (int) (r * maxval),
			    (int) (g * maxval), (int) (b * maxval));
		    }
		}
	    }
	    break;

	case PGM_TYPE:
	    for (i = 0; i < rows; i++) {
		grayrow = xels[i];
		for (j = 0; j < hist_cols; j++) {
		    PNM_ASSIGN1(grayrow[j], lumamap[0][PNM_GET1(grayrow[j])]);
		}
	    }
	    break;
    }

    pnm_writepnm(stdout, xels, hist_cols, rows, maxval, format, 0);

    /* If requested, save the map as a PGM file. */

    if (wmap != NULL) {
	pgm_writepgm(wmap, lumamap, maxval, 1, maxval, 0);
	fclose(wmap);
    }

    return 0;
}
