/* pnmnlfilt.c - 4 in 1 (2 non-linear) filter
**             - smooth an anyimage
**             - do alpha trimmed mean filtering on an anyimage
**             - do optimal estimation smoothing on an anyimage
**             - do edge enhancement on an anyimage
**
** Version 1.0
**
** The implementation of an alpha-trimmed mean filter
** is based on the description in IEEE CG&A May 1990
** Page 23 by Mark E. Lee and Richard A. Redner.
**
** The paper recommends using a hexagon sampling region around each
** pixel being processed, allowing an effective sub pixel radius to be
** specified. The hexagon values are sythesised by area sampling the
** rectangular pixels with a hexagon grid. The seven hexagon values
** obtained from the 3x3 pixel grid are used to compute the alpha
** trimmed mean. Note that an alpha value of 0.0 gives a conventional
** mean filter (where the radius controls the contribution of
** surrounding pixels), while a value of 0.5 gives a median filter.
** Although there are only seven values to trim from before finding
** the mean, the algorithm has been extended from that described in
** CG&A by using interpolation, to allow a continuous selection of
** alpha value between and including 0.0 to 0.5  The useful values
** for radius are between 0.3333333 (where the filter will have no
** effect because only one pixel is sampled), to 1.0, where all
** pixels in the 3x3 grid are sampled.
**
** The optimal estimation filter is taken from an article "Converting Dithered
** Images Back to Gray Scale" by Allen Stenger, Dr Dobb's Journal, November
** 1992, and this article references "Digital Image Enhancement andNoise Filtering by
** Use of Local Statistics", Jong-Sen Lee, IEEE Transactions on Pattern Analysis and
** Machine Intelligence, March 1980.
**
** Also borrow the  technique used in pgmenhance(1) to allow edge
** enhancement if the alpha value is negative.
**
** Author:
**         Graeme W. Gill, 30th Jan 1993
**         graeme@labtam.oz.au
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <math.h>
#include "pnm.h"

double hex_area ARGS((double, double, double, double, double));
int atfilt_setup ARGS((double, double, double));
int atfilt0 ARGS((int *)), atfilt1 ARGS((int *)), atfilt2 ARGS((int *));
int atfilt3 ARGS((int *)), atfilt4 ARGS((int *)), atfilt5 ARGS((int *));
int (*atfuncs[6]) ARGS((int *)) = {atfilt0,atfilt1,atfilt2,atfilt3,atfilt4,atfilt5};

xelval omaxval; /* global so that pixel processing code can get at it quickly */
int noisevariance;      /* global so that pixel processing code can get at it quickly */

int
main( argc, argv )
int argc;
char* argv[];
        {
        double radius=0.0,alpha= -1.0;
        FILE* ifp;
        int rows, cols, format, oformat, row, col;
        xelval maxval;
        int (*atfunc) ARGS((int *));
        char* usage = "alpha radius pnmfile\n\
 0.0 <= alpha <= 0.5 for alpha trimmed mean -or- \n\
 1.0 <= alpha <= 2.0 for optimal estimation -or- \n\
 -0.1 >= alpha >= -0.9 for edge enhancement\n\
 0.3333 <= radius <= 1.0 specify effective radius\n";


        pnm_init( &argc, argv );

        if ( argc < 3 || argc > 4 )
                pm_usage( usage );

        if ( sscanf( argv[1], "%lf", &alpha ) != 1 )
                pm_usage( usage );
        if ( sscanf( argv[2], "%lf", &radius ) != 1 )
                pm_usage( usage );

        if ((alpha > -0.1 && alpha < 0.0) || (alpha > 0.5 && alpha < 1.0))
                pm_error( "Alpha must be in range 0.0 <= alpha <= 0.5 for alpha trimmed mean" );
        if (alpha > 2.0)
                pm_error( "Alpha must be in range 1.0 <= alpha <= 2.0 for optimal estimation" );
        if (alpha < -0.9 || (alpha > -0.1 && alpha < 0.0))
                pm_error( "Alpha must be in range -0.9 <= alpha <= -0.1 for edge enhancement" );
        if (radius < 0.333 || radius > 1.0)
                pm_error( "Radius must be in range 0.333333333 <= radius <= 1.0" );

        if ( argc == 4 )
                ifp = pm_openr( argv[3] );
        else
                ifp = stdin;

        pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );

        oformat = PNM_FORMAT_TYPE(format);
        omaxval = PPM_MAXMAXVAL;        /* force output to max precision */

        atfunc = atfuncs[atfilt_setup(alpha,radius,(double)omaxval/(double)maxval)];

        if ( oformat < PGM_TYPE )
                {
                if (PGM_MAXMAXVAL > PPM_MAXMAXVAL)
                        pm_error( "Can't handle pgm input file as maxval is too big" );
                oformat = RPGM_FORMAT;
                pm_message( "promoting file to PGM" );
                }
        pnm_writepnminit( stdout, cols, rows, omaxval, oformat, 0 );

        if ( PNM_FORMAT_TYPE(oformat) == PPM_TYPE )
                {
                xel *irows[3];
                xel *irow0, *irow1, *irow2, *orow;
                int pr[9],pg[9],pb[9];          /* 3x3 neighbor pixel values */
                int r,g,b;

                irows[0] = pnm_allocrow( cols );
                irows[1] = pnm_allocrow( cols );
                irows[2] = pnm_allocrow( cols );
                irow0 = irows[0];
                irow1 = irows[1];
                irow2 = irows[2];
                orow = pnm_allocrow( cols );

                for ( row = 0; row < rows; row++ )
                        {
                        int po,no;              /* offsets for left and right colums in 3x3 */
                        register xel *ip0, *ip1, *ip2, *op;

                        if (row == 0)
                                {
                                irow0 = irow1;
                                pnm_readpnmrow( ifp, irow1, cols, maxval, format );
                                }
                        if (row == (rows-1))
                                irow2 = irow1;
                        else
                                pnm_readpnmrow( ifp, irow2, cols, maxval, format );

                        for (col = cols-1,po= col>0?1:0,no=0,ip0=irow0,ip1=irow1,ip2=irow2,op=orow;
                             col >= 0;
                             col--,ip0++,ip1++,ip2++,op++, no |= 1,po = col!= 0 ? po : 0)
                                {
                                /* grab 3x3 pixel values */
                                pr[0] = PPM_GETR( *ip1 );
                                pg[0] = PPM_GETG( *ip1 );
                                pb[0] = PPM_GETB( *ip1 );
                                pr[1] = PPM_GETR( *(ip1-no) );
                                pg[1] = PPM_GETG( *(ip1-no) );
                                pb[1] = PPM_GETB( *(ip1-no) );
                                pr[5] = PPM_GETR( *(ip1+po) );
                                pg[5] = PPM_GETG( *(ip1+po) );
                                pb[5] = PPM_GETB( *(ip1+po) );
                                pr[3] = PPM_GETR( *(ip2) );
                                pg[3] = PPM_GETG( *(ip2) );
                                pb[3] = PPM_GETB( *(ip2) );
                                pr[2] = PPM_GETR( *(ip2-no) );
                                pg[2] = PPM_GETG( *(ip2-no) );
                                pb[2] = PPM_GETB( *(ip2-no) );
                                pr[4] = PPM_GETR( *(ip2+po) );
                                pg[4] = PPM_GETG( *(ip2+po) );
                                pb[4] = PPM_GETB( *(ip2+po) );
                                pr[6] = PPM_GETR( *(ip0+po) );
                                pg[6] = PPM_GETG( *(ip0+po) );
                                pb[6] = PPM_GETB( *(ip0+po) );
                                pr[8] = PPM_GETR( *(ip0-no) );
                                pg[8] = PPM_GETG( *(ip0-no) );
                                pb[8] = PPM_GETB( *(ip0-no) );
                                pr[7] = PPM_GETR( *(ip0) );
                                pg[7] = PPM_GETG( *(ip0) );
                                pb[7] = PPM_GETB( *(ip0) );
                                r = (*atfunc)(pr);
                                g = (*atfunc)(pg);
                                b = (*atfunc)(pb);
                                PPM_ASSIGN( *op, r, g, b );
                                }
                        pnm_writepnmrow( stdout, orow, cols, omaxval, oformat, 0 );
                        if (irow1 == irows[2])
                                {
                                irow1 = irows[0];
                                irow2 = irows[1];
                                irow0 = irows[2];
                                }
                        else if (irow1 == irows[1])
                                {
                                irow2 = irows[0];
                                irow0 = irows[1];
                                irow1 = irows[2];
                                }
                        else    /* must be at irows[0] */
                                {
                                irow0 = irows[0];
                                irow1 = irows[1];
                                irow2 = irows[2];
                                }
                        }
                }
        else    /* Else must be PGM */
                {
                xel *irows[3];
                xel *irow0, *irow1, *irow2, *orow;
                int p[9];               /* 3x3 neighbor pixel values */
                int pv;
                int promote;

                irows[0] = pnm_allocrow( cols );
                irows[1] = pnm_allocrow( cols );
                irows[2] = pnm_allocrow( cols );
                irow0 = irows[0];
                irow1 = irows[1];
                irow2 = irows[2];
                orow = pnm_allocrow( cols );
                /* we scale maxval to omaxval */
                promote = ( PNM_FORMAT_TYPE(format) != PNM_FORMAT_TYPE(oformat) );

                for ( row = 0; row < rows; row++ )
                        {
                        int po,no;              /* offsets for left and right colums in 3x3 */
                        register xel *ip0, *ip1, *ip2, *op;

                        if (row == 0)
                                {
                                irow0 = irow1;
                                pnm_readpnmrow( ifp, irow1, cols, maxval, format );
                                if ( promote )
                                        pnm_promoteformatrow( irow1, cols, maxval, format, maxval, oformat );
                                }
                        if (row == (rows-1))
                                irow2 = irow1;
                        else
                                {
                                pnm_readpnmrow( ifp, irow2, cols, maxval, format );
                                if ( promote )
                                        pnm_promoteformatrow( irow2, cols, maxval, format, maxval, oformat );
                                }

                        for (col = cols-1,po= col>0?1:0,no=0,ip0=irow0,ip1=irow1,ip2=irow2,op=orow;
                             col >= 0;
                             col--,ip0++,ip1++,ip2++,op++, no |= 1,po = col!= 0 ? po : 0)
                                {
                                /* grab 3x3 pixel values */
                                p[0] = PNM_GET1( *ip1 );
                                p[1] = PNM_GET1( *(ip1-no) );
                                p[5] = PNM_GET1( *(ip1+po) );
                                p[3] = PNM_GET1( *(ip2) );
                                p[2] = PNM_GET1( *(ip2-no) );
                                p[4] = PNM_GET1( *(ip2+po) );
                                p[6] = PNM_GET1( *(ip0+po) );
                                p[8] = PNM_GET1( *(ip0-no) );
                                p[7] = PNM_GET1( *(ip0) );
                                pv = (*atfunc)(p);
                                PNM_ASSIGN1( *op, pv );
                                }
                        pnm_writepnmrow( stdout, orow, cols, omaxval, oformat, 0 );
                        if (irow1 == irows[2])
                                {
                                irow1 = irows[0];
                                irow2 = irows[1];
                                irow0 = irows[2];
                                }
                        else if (irow1 == irows[1])
                                {
                                irow2 = irows[0];
                                irow0 = irows[1];
                                irow1 = irows[2];
                                }
                        else    /* must be at irows[0] */
                                {
                                irow0 = irows[0];
                                irow1 = irows[1];
                                irow2 = irows[2];
                                }
                        }
                }
        pm_close( ifp );

        exit( 0 );
        }

#define MXIVAL PPM_MAXMAXVAL    /* maximum input value */
#define NOIVAL (MXIVAL + 1)             /* number of possible input values */

#define SCALEB 8                                /* scale bits */
#define SCALE (1 << SCALEB)     /* scale factor */
#define MXSVAL (MXIVAL * SCALE) /* maximum scaled values */

#define CSCALEB 2                               /* coarse scale bits */
#define CSCALE (1 << CSCALEB)   /* coarse scale factor */
#define MXCSVAL (MXIVAL * CSCALE)       /* maximum coarse scaled values */
#define NOCSVAL (MXCSVAL + 1)   /* number of coarse scaled values */
#define SCTOCSC(x) ((x) >> (SCALEB - CSCALEB))  /* convert from scaled to coarse scaled */
#define CSCTOSC(x) ((x) << (SCALEB - CSCALEB))  /* convert from course scaled to scaled */

#ifndef MAXINT
# define MAXINT 0x7fffffff      /* assume this is a 32 bit machine */
#endif

/* round and scale floating point to scaled integer */
#define ROUND(x) ((int)(((x) * (double)SCALE) + 0.5))
/* round and un-scale scaled integer value */
#define RUNSCALE(x) (((x) + (1 << (SCALEB-1))) >> SCALEB)       /* rounded un-scale */
#define UNSCALE(x) ((x) >> SCALEB)


/* We restrict radius to the values: 0.333333 <= radius <= 1.0 */
/* so that no fewer and no more than a 3x3 grid of pixels around */
/* the pixel in question needs to be read. Given this, we only */
/* need 3 or 4 weightings per hexagon, as follows: */
/*                  _ _                         */
/* Virtical hex:   |_|_|  1 2                   */
/*                 |X|_|  0 3                   */
/*                                       _      */
/*              _                      _|_|   1 */
/* Middle hex: |_| 1  Horizontal hex: |X|_| 0 2 */
/*             |X| 0                    |_|   3 */
/*             |_| 2                            */

/* all filters */
int V0[NOIVAL],V1[NOIVAL],V2[NOIVAL],V3[NOIVAL];        /* vertical hex */
int M0[NOIVAL],M1[NOIVAL],M2[NOIVAL];           /* middle hex */
int H0[NOIVAL],H1[NOIVAL],H2[NOIVAL],H3[NOIVAL];        /* horizontal hex */

/* alpha trimmed and edge enhancement only */
int ALFRAC[NOIVAL * 8];                 /* fractional alpha divider table */

/* optimal estimation only */
int AVEDIV[7 * NOCSVAL];                /* divide by 7 to give average value */
int SQUARE[2 * NOCSVAL];                /* scaled square lookup table */

/* Table initialisation function - return alpha range */
int
atfilt_setup(alpha,radius,maxscale)
double alpha,radius,maxscale;   /* alpha, radius and amount to scale input pixel values */
        {
        /* other function variables */
        int alpharange;                 /* alpha range value 0 - 3 */
        double meanscale;               /* scale for finding mean */
        double mmeanscale;              /* scale for finding mean - midle hex */
        double alphafraction;   /* fraction of next largest/smallest to subtract from sum */

        /* do setup */

        if (alpha >= 0.0 && alpha < 1.0)        /* alpha trimmed mean */
                {
                double noinmean;
                /* number of elements (out of a possible 7) used in the mean */
                noinmean = ((0.5 - alpha) * 12.0) + 1.0;
                mmeanscale = meanscale = maxscale/noinmean;
                if (alpha == 0.0)                    /* mean filter */
                        {
                        alpharange = 0;
                        alphafraction = 0.0;            /* not used */
                        }
                else if (alpha < (1.0/6.0))     /* mean of 5 to 7 middle values */
                        {
                        alpharange = 1;
                        alphafraction = (7.0 - noinmean)/2.0;
                        }
                else if (alpha < (1.0/3.0))     /* mean of 3 to 5 middle values */
                        {
                        alpharange = 2;
                        alphafraction = (5.0 - noinmean)/2.0;
                        }
                else                            /* mean of 1 to 3 middle values */
                        {                                                       /* alpha == 0.5 == median filter */
                        alpharange = 3;
                        alphafraction = (3.0 - noinmean)/2.0;
                        }
                }
        else if (alpha > 0.5)   /* optimal estimation - alpha controls noise variance threshold. */
                {
                int i;
                double noinmean = 7.0;
                alpharange = 5;                 /* edge enhancement function */
                alpha -= 1.0;                   /* normalise it to 0.0 -> 1.0 */
                mmeanscale = meanscale = maxscale;      /* compute scaled hex values */
                alphafraction = 1.0/noinmean;   /* Set up 1:1 division lookup - not used */
                noisevariance = alpha * (double)omaxval;
                noisevariance = noisevariance * noisevariance / 8.0;    /* estimate of noise variance */
                /* set yp optimal estimation specific stuff */
                for (i=0; i < (7 * NOCSVAL); i++)       /* divide scaled value by 7 lookup */
                        {
                        AVEDIV[i] = CSCTOSC(i)/7;       /* scaled divide by 7 */
                        }
                for (i=0; i < (2 * NOCSVAL); i++)  /* compute square and rescale by (val >> (2 * SCALEB + 2)) table */
                        {
                        int val;
                        val = CSCTOSC(i - NOCSVAL); /* NOCSVAL offset to cope with -ve input values */
                        SQUARE[i] = (val * val) >> (2 * SCALEB + 2);
                        }
                }
        else    /* edge enhancement function */
                {
                alpharange = 4;                 /* edge enhancement function */
                alpha = -alpha;                 /* turn it the right way up */
                meanscale = maxscale * (-alpha/((1.0 - alpha) * 7.0)); /* mean of 7 and scaled by -alpha/(1-alpha) */
                mmeanscale = maxscale * (1.0/(1.0 - alpha) + meanscale);        /* middle pixel has 1/(1-alpha) as well */
                alphafraction = 0.0;    /* not used */
                }

                /* Setup pixel weighting tables - note we pre-compute mean division here too. */
                {
                int i;
                double hexhoff,hexvoff;
                double tabscale,mtabscale;
                double v0,v1,v2,v3,m0,m1,m2,me0,me1,me2,h0,h1,h2,h3;

                hexhoff = radius/2;                 /* horizontal offset of virtical hex centers */
                hexvoff = 3.0 * radius/sqrt(12.0);      /* virtical offset of virtical hex centers */
                /* scale tables to normalise by hexagon area, and number of hexes used in mean */
                tabscale = meanscale / (radius * hexvoff);
                mtabscale = mmeanscale / (radius * hexvoff);
                v0 = hex_area(0.0,0.0,hexhoff,hexvoff,radius) * tabscale;
                v1 = hex_area(0.0,1.0,hexhoff,hexvoff,radius) * tabscale;
                v2 = hex_area(1.0,1.0,hexhoff,hexvoff,radius) * tabscale;
                v3 = hex_area(1.0,0.0,hexhoff,hexvoff,radius) * tabscale;
                m0 = hex_area(0.0,0.0,0.0,0.0,radius) * mtabscale;
                m1 = hex_area(0.0,1.0,0.0,0.0,radius) * mtabscale;
                m2 = hex_area(0.0,-1.0,0.0,0.0,radius) * mtabscale;
                h0 = hex_area(0.0,0.0,radius,0.0,radius) * tabscale;
                h1 = hex_area(1.0,1.0,radius,0.0,radius) * tabscale;
                h2 = hex_area(1.0,0.0,radius,0.0,radius) * tabscale;
                h3 = hex_area(1.0,-1.0,radius,0.0,radius) * tabscale;

                for (i=0; i <= MXIVAL; i++)
                        {
                        double fi;
                        fi = (double)i;
                        V0[i] = ROUND(fi * v0);
                        V1[i] = ROUND(fi * v1);
                        V2[i] = ROUND(fi * v2);
                        V3[i] = ROUND(fi * v3);
                        M0[i] = ROUND(fi * m0);
                        M1[i] = ROUND(fi * m1);
                        M2[i] = ROUND(fi * m2);
                        H0[i] = ROUND(fi * h0);
                        H1[i] = ROUND(fi * h1);
                        H2[i] = ROUND(fi * h2);
                        H3[i] = ROUND(fi * h3);
                        }
                /* set up alpha fraction lookup table used on big/small */
                for (i=0; i < (NOIVAL * 8); i++)
                        {
                        ALFRAC[i] = ROUND((double)i * alphafraction);
                        }
                }
        return alpharange;
        }


/* Core pixel processing function - hand it 3x3 pixels and return result. */
/* Mean filter */
int
atfilt0(p)
int *p;         /* 9 pixel values from 3x3 neighbors */
        {
        int retv;
        /* map to scaled hexagon values */
        retv = M0[p[0]] + M1[p[3]] + M2[p[7]];
        retv += H0[p[0]] + H1[p[2]] + H2[p[1]] + H3[p[8]];
        retv += V0[p[0]] + V1[p[3]] + V2[p[2]] + V3[p[1]];
        retv += V0[p[0]] + V1[p[3]] + V2[p[4]] + V3[p[5]];
        retv += H0[p[0]] + H1[p[4]] + H2[p[5]] + H3[p[6]];
        retv += V0[p[0]] + V1[p[7]] + V2[p[6]] + V3[p[5]];
        retv += V0[p[0]] + V1[p[7]] + V2[p[8]] + V3[p[1]];
        return UNSCALE(retv);
        }

/* Mean of 5 - 7 middle values */
int
atfilt1(p)
int *p;         /* 9 pixel values from 3x3 neighbors */
        {
        int h0,h1,h2,h3,h4,h5,h6;       /* hexagon values    2 3   */
                                    /*                  1 0 4  */
                                    /*                   6 5   */
        int big,small;
        /* map to scaled hexagon values */
        h0 = M0[p[0]] + M1[p[3]] + M2[p[7]];
        h1 = H0[p[0]] + H1[p[2]] + H2[p[1]] + H3[p[8]];
        h2 = V0[p[0]] + V1[p[3]] + V2[p[2]] + V3[p[1]];
        h3 = V0[p[0]] + V1[p[3]] + V2[p[4]] + V3[p[5]];
        h4 = H0[p[0]] + H1[p[4]] + H2[p[5]] + H3[p[6]];
        h5 = V0[p[0]] + V1[p[7]] + V2[p[6]] + V3[p[5]];
        h6 = V0[p[0]] + V1[p[7]] + V2[p[8]] + V3[p[1]];
        /* sum values and also discover the largest and smallest */
        big = small = h0;
#define CHECK(xx) \
        h0 += xx; \
        if (xx > big) \
                big = xx; \
        else if (xx < small) \
                small = xx;
        CHECK(h1)
        CHECK(h2)
        CHECK(h3)
        CHECK(h4)
        CHECK(h5)
        CHECK(h6)
#undef CHECK
        /* Compute mean of middle 5-7 values */
        return UNSCALE(h0 -ALFRAC[(big + small)>>SCALEB]);
        }

/* Mean of 3 - 5 middle values */
int
atfilt2(p)
int *p;         /* 9 pixel values from 3x3 neighbors */
        {
        int h0,h1,h2,h3,h4,h5,h6;       /* hexagon values    2 3   */
                                    /*                  1 0 4  */
                                    /*                   6 5   */
        int big0,big1,small0,small1;
        /* map to scaled hexagon values */
        h0 = M0[p[0]] + M1[p[3]] + M2[p[7]];
        h1 = H0[p[0]] + H1[p[2]] + H2[p[1]] + H3[p[8]];
        h2 = V0[p[0]] + V1[p[3]] + V2[p[2]] + V3[p[1]];
        h3 = V0[p[0]] + V1[p[3]] + V2[p[4]] + V3[p[5]];
        h4 = H0[p[0]] + H1[p[4]] + H2[p[5]] + H3[p[6]];
        h5 = V0[p[0]] + V1[p[7]] + V2[p[6]] + V3[p[5]];
        h6 = V0[p[0]] + V1[p[7]] + V2[p[8]] + V3[p[1]];
        /* sum values and also discover the 2 largest and 2 smallest */
        big0 = small0 = h0;
        small1 = MAXINT;
        big1 = 0;
#define CHECK(xx) \
        h0 += xx; \
        if (xx > big1) \
                { \
                if (xx > big0) \
                        { \
                        big1 = big0; \
                        big0 = xx; \
                        } \
                else \
                        big1 = xx; \
                } \
        if (xx < small1) \
                { \
                if (xx < small0) \
                        { \
                        small1 = small0; \
                        small0 = xx; \
                        } \
                else \
                        small1 = xx; \
                }
        CHECK(h1)
        CHECK(h2)
        CHECK(h3)
        CHECK(h4)
        CHECK(h5)
        CHECK(h6)
#undef CHECK
        /* Compute mean of middle 3-5 values */
        return UNSCALE(h0 -big0 -small0 -ALFRAC[(big1 + small1)>>SCALEB]);
        }

/* Mean of 1 - 3 middle values. If only 1 value, then this is a median filter. */
int
atfilt3(p)
int *p;         /* 9 pixel values from 3x3 neighbors */
        {
        int h0,h1,h2,h3,h4,h5,h6;       /* hexagon values    2 3   */
                                    /*                  1 0 4  */
                                    /*                   6 5   */
        int big0,big1,big2,small0,small1,small2;
        /* map to scaled hexagon values */
        h0 = M0[p[0]] + M1[p[3]] + M2[p[7]];
        h1 = H0[p[0]] + H1[p[2]] + H2[p[1]] + H3[p[8]];
        h2 = V0[p[0]] + V1[p[3]] + V2[p[2]] + V3[p[1]];
        h3 = V0[p[0]] + V1[p[3]] + V2[p[4]] + V3[p[5]];
        h4 = H0[p[0]] + H1[p[4]] + H2[p[5]] + H3[p[6]];
        h5 = V0[p[0]] + V1[p[7]] + V2[p[6]] + V3[p[5]];
        h6 = V0[p[0]] + V1[p[7]] + V2[p[8]] + V3[p[1]];
        /* sum values and also discover the 3 largest and 3 smallest */
        big0 = small0 = h0;
        small1 = small2 = MAXINT;
        big1 = big2 = 0;
#define CHECK(xx) \
        h0 += xx; \
        if (xx > big2) \
                { \
                if (xx > big1) \
                        { \
                        if (xx > big0) \
                                { \
                                big2 = big1; \
                                big1 = big0; \
                                big0 = xx; \
                                } \
                        else \
                                { \
                                big2 = big1; \
                                big1 = xx; \
                                } \
                        } \
                else \
                        big2 = xx; \
                } \
        if (xx < small2) \
                { \
                if (xx < small1) \
                        { \
                        if (xx < small0) \
                                { \
                                small2 = small1; \
                                small1 = small0; \
                                small0 = xx; \
                                } \
                        else \
                                { \
                                small2 = small1; \
                                small1 = xx; \
                                } \
                        } \
                else \
                        small2 = xx; \
                }
        CHECK(h1)
        CHECK(h2)
        CHECK(h3)
        CHECK(h4)
        CHECK(h5)
        CHECK(h6)
#undef CHECK
        /* Compute mean of middle 1-3 values */
        return  UNSCALE(h0 -big0 -big1 -small0 -small1 -ALFRAC[(big2 + small2)>>SCALEB]);
        }

/* Edge enhancement */
/* notice we use the global omaxval */
int
atfilt4(p)
int *p;         /* 9 pixel values from 3x3 neighbors */
        {
        int hav;
        /* map to scaled hexagon values and compute enhance value */
        hav = M0[p[0]] + M1[p[3]] + M2[p[7]];
        hav += H0[p[0]] + H1[p[2]] + H2[p[1]] + H3[p[8]];
        hav += V0[p[0]] + V1[p[3]] + V2[p[2]] + V3[p[1]];
        hav += V0[p[0]] + V1[p[3]] + V2[p[4]] + V3[p[5]];
        hav += H0[p[0]] + H1[p[4]] + H2[p[5]] + H3[p[6]];
        hav += V0[p[0]] + V1[p[7]] + V2[p[6]] + V3[p[5]];
        hav += V0[p[0]] + V1[p[7]] + V2[p[8]] + V3[p[1]];
        if (hav < 0)
                hav = 0;
        hav = UNSCALE(hav);
        if (hav > omaxval)
                hav = omaxval;
        return hav;
        }

/* Optimal estimation - do smoothing in inverse proportion */
/* to the local variance. */
/* notice we use the globals noisevariance and omaxval*/
int
atfilt5(p)
int *p;         /* 9 pixel values from 3x3 neighbors */
        {
        int mean,variance,temp;
        int h0,h1,h2,h3,h4,h5,h6;       /* hexagon values    2 3   */
                                    /*                  1 0 4  */
                                    /*                   6 5   */
        /* map to scaled hexagon values */
        h0 = M0[p[0]] + M1[p[3]] + M2[p[7]];
        h1 = H0[p[0]] + H1[p[2]] + H2[p[1]] + H3[p[8]];
        h2 = V0[p[0]] + V1[p[3]] + V2[p[2]] + V3[p[1]];
        h3 = V0[p[0]] + V1[p[3]] + V2[p[4]] + V3[p[5]];
        h4 = H0[p[0]] + H1[p[4]] + H2[p[5]] + H3[p[6]];
        h5 = V0[p[0]] + V1[p[7]] + V2[p[6]] + V3[p[5]];
        h6 = V0[p[0]] + V1[p[7]] + V2[p[8]] + V3[p[1]];
        mean = h0 + h1 + h2 + h3 + h4 + h5 + h6;
        mean = AVEDIV[SCTOCSC(mean)];   /* compute scaled mean by dividing by 7 */
        temp = (h1 - mean); variance = SQUARE[NOCSVAL + SCTOCSC(temp)];  /* compute scaled variance */
        temp = (h2 - mean); variance += SQUARE[NOCSVAL + SCTOCSC(temp)]; /* and rescale to keep */
        temp = (h3 - mean); variance += SQUARE[NOCSVAL + SCTOCSC(temp)]; /* within 32 bit limits */
        temp = (h4 - mean); variance += SQUARE[NOCSVAL + SCTOCSC(temp)];
        temp = (h5 - mean); variance += SQUARE[NOCSVAL + SCTOCSC(temp)];
        temp = (h6 - mean); variance += SQUARE[NOCSVAL + SCTOCSC(temp)];
        temp = (h0 - mean); variance += SQUARE[NOCSVAL + SCTOCSC(temp)];        /* (temp = h0 - mean) */
        if (variance != 0)      /* avoid possible divide by 0 */
                temp = mean + (variance * temp) / (variance + noisevariance);   /* optimal estimate */
        else temp = h0;
        if (temp < 0)
                temp = 0;
        temp = RUNSCALE(temp);
        if (temp > omaxval)
                temp = omaxval;
        return temp;
        }

/* ************************************************** */
/* Hexagon intersecting square area functions */
/* Compute the area of the intersection of a triangle */
/* and a rectangle */

double triang_area ARGS((double, double, double, double, double, double, double, double, int));
double rectang_area ARGS((double, double, double, double, double, double, double, double));

/* Triangle orientation is per geometric axes (not graphical axies) */

#define NW 0    /* North west triangle /| */
#define NE 1    /* North east triangle |\ */
#define SW 2    /* South west triangle \| */
#define SE 3    /* South east triangle |/ */
#define STH 2
#define EST 1

#define SWAPI(a,b) (t = a, a = -b, b = -t)

/* compute the area of overlap of a hexagon diameter d, */
/* centered at hx,hy, with a unit square of center sx,sy. */
double
hex_area(sx,sy,hx,hy,d)
double sx,sy;   /* square center */
double hx,hy,d; /* hexagon center and diameter */
        {
        double hx0,hx1,hx2,hy0,hy1,hy2,hy3;
        double sx0,sx1,sy0,sy1;

        /* compute square co-ordinates */
        sx0 = sx - 0.5;
        sy0 = sy - 0.5;
        sx1 = sx + 0.5;
        sy1 = sy + 0.5;
        /* compute hexagon co-ordinates */
        hx0 = hx - d/2.0;
        hx1 = hx;
        hx2 = hx + d/2.0;
        hy0 = hy - 0.5773502692 * d;    /* d / sqrt(3) */
        hy1 = hy - 0.2886751346 * d;    /* d / sqrt(12) */
        hy2 = hy + 0.2886751346 * d;    /* d / sqrt(12) */
        hy3 = hy + 0.5773502692 * d;    /* d / sqrt(3) */

        return
                triang_area(sx0,sy0,sx1,sy1,hx0,hy2,hx1,hy3,NW) +
                triang_area(sx0,sy0,sx1,sy1,hx1,hy2,hx2,hy3,NE) +
                rectang_area(sx0,sy0,sx1,sy1,hx0,hy1,hx2,hy2) +
                triang_area(sx0,sy0,sx1,sy1,hx0,hy0,hx1,hy1,SW) +
                triang_area(sx0,sy0,sx1,sy1,hx1,hy0,hx2,hy1,SE);
        }

double
triang_area(rx0,ry0,rx1,ry1,tx0,ty0,tx1,ty1,tt)
double rx0,ry0,rx1,ry1;         /* rectangle boundaries */
double tx0,ty0,tx1,ty1;         /* triangle boundaries */
int tt;                                         /* triangle type */
        {
        double a,b,c,d;
        double lx0,ly0,lx1,ly1;
        /* Convert everything to a NW triangle */
        if (tt & STH)
                {
                double t;
        SWAPI(ry0,ry1);
        SWAPI(ty0,ty1);
                }
        if (tt & EST)
                {
                double t;
        SWAPI(rx0,rx1);
        SWAPI(tx0,tx1);
                }
        /* Compute overlapping box */
        if (tx0 > rx0)
                rx0 = tx0;
        if (ty0 > ry0)
                ry0 = ty0;
        if (tx1 < rx1)
                rx1 = tx1;
        if (ty1 < ry1)
                ry1 = ty1;
        if (rx1 <= rx0 || ry1 <= ry0)
                return 0.0;
        /* Need to compute diagonal line intersection with the box */
        /* First compute co-efficients to formulas x = a + by and y = c + dx */
        b = (tx1 - tx0)/(ty1 - ty0);
        a = tx0 - b * ty0;
        d = (ty1 - ty0)/(tx1 - tx0);
        c = ty0 - d * tx0;

        /* compute top or right intersection */
        tt = 0;
        ly1 = ry1;
        lx1 = a + b * ly1;
        if (lx1 <= rx0)
                return (rx1 - rx0) * (ry1 - ry0);
        else if (lx1 > rx1)     /* could be right hand side */
                {
                lx1 = rx1;
                ly1 = c + d * lx1;
                if (ly1 <= ry0)
                        return (rx1 - rx0) * (ry1 - ry0);
                tt = 1; /* right hand side intersection */
                }
        /* compute left or bottom intersection */
        lx0 = rx0;
        ly0 = c + d * lx0;
        if (ly0 >= ry1)
                return (rx1 - rx0) * (ry1 - ry0);
        else if (ly0 < ry0)     /* could be right hand side */
                {
                ly0 = ry0;
                lx0 = a + b * ly0;
                if (lx0 >= rx1)
                        return (rx1 - rx0) * (ry1 - ry0);
                tt |= 2;        /* bottom intersection */
                }

        if (tt == 0)    /* top and left intersection */
                {       /* rectangle minus triangle */
                return ((rx1 - rx0) * (ry1 - ry0))
                     - (0.5 * (lx1 - rx0) * (ry1 - ly0));
                }
        else if (tt == 1)       /* right and left intersection */
                {
                return ((rx1 - rx0) * (ly0 - ry0))
                     + (0.5 * (rx1 - rx0) * (ly1 - ly0));
                }
        else if (tt == 2)       /* top and bottom intersection */
                {
                return ((rx1 - lx1) * (ry1 - ry0))
                     + (0.5 * (lx1 - lx0) * (ry1 - ry0));
                }
        else /* tt == 3 */      /* right and bottom intersection */
                {       /* triangle */
                return (0.5 * (rx1 - lx0) * (ly1 - ry0));
                }
        }

/* Compute rectangle area */
double
rectang_area(rx0,ry0,rx1,ry1,tx0,ty0,tx1,ty1)
double rx0,ry0,rx1,ry1;         /* rectangle boundaries */
double tx0,ty0,tx1,ty1;         /* rectangle boundaries */
        {
        /* Compute overlapping box */
        if (tx0 > rx0)
                rx0 = tx0;
        if (ty0 > ry0)
                ry0 = ty0;
        if (tx1 < rx1)
                rx1 = tx1;
        if (ty1 < ry1)
                ry1 = ty1;
        if (rx1 <= rx0 || ry1 <= ry0)
                return 0.0;
        return (rx1 - rx0) * (ry1 - ry0);
        }

