/* ppmbrighten.c - allow user control over Value and Saturation of PPM file
**
** Copyright (C) 1989 by Jef Poskanzer.
** Copyright (C) 1990 by Brian Moffet.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

static int GetHSV ARGS((long r, long g, long b, long *h, long *s, long *v));
static int GetRGB ARGS((long h, long s, long v, long *r, long *g, long *b));

#define MULTI   1000
#define FIND_MINMAX     1
#define SET_VALUE       2
#define SET_SATUR       4

static int GetHSV( r, g, b, h, s, v )
    long r, g, b;
    long *h, *s, *v;
{
    long t;

    *v = max( r, max( g, b ) );

    t = min( r, min( g, b ) );

    if ( *v == 0 )
        *s = 0;
    else
        *s = ( (*v - t)*MULTI ) / *v;

    if ( *s == 0 )
        *h = 0;
    else
    {
        long cr, cg, cb;
        cr = (MULTI * ( *v - r ))/( *v - t );
        cg = (MULTI * ( *v - g ))/( *v - t );
        cb = (MULTI * ( *v - b ))/( *v - t );

        if ( r == *v )
            *h = cb - cg;

        if ( g == *v )
            *h = (2*MULTI) + cr - cb;

        if ( b == *v )
            *h = (4*MULTI) + cg - cr;

        *h = *h * 60;
        if ( *h < 0 )
            *h += (360 * MULTI);
    }
}

static int
    GetRGB( h, s, v, r, g, b )
long h, s, v;
long *r, *g, *b;
{
    if ( s == 0 )
    {
        *r = v;
        *g = v;
        *b = v;
    } else {
        long f, m, n, k;
        long i;

        if (h == (360 * MULTI))
            h = 0;
        h = h / 60;
        i = (h - (h % MULTI));
        f = h - i;
        m = (v * (MULTI - s)) / MULTI;
        n = (v * (MULTI - (s * f)/MULTI)) / MULTI;
        k = (v * (MULTI - (s * (MULTI - f))/MULTI)) / MULTI;

        switch (i)
        {
        case 0:
            *r = v;
            *g = k;
            *b = m;
            break;
        case MULTI:
            *r = n;
            *g = v;
            *b = m;
            break;
        case (2*MULTI):
            *r = m;
            *g = v;
            *b = k;
            break;
        case (3*MULTI):
            *r = m;
            *g = n;
            *b = v;
            break;
        case (4*MULTI):
            *r = k;
            *g = m;
            *b = v;
            break;
        case (5*MULTI):
            *r = v;
            *g = m;
            *b = n;
            break;
        }
    }
}

int
main( argc, argv )
    int argc;
    char *argv[];
{
    FILE *fp = stdin;
    extern char *optarg;
    extern int optind;
    int value = 100, saturation = 100, c, error = 0;
    int min_value, max_value;
    int flags = 0;
    int argn;
    char *usage = "[-saturation <+-s>] [-value <+-v>] [-normalize] [<ppmfile>]";
    pixel *pixelP;
    pixval maxval;
    int rows, cols, format, indexv;


    ppm_init(&argc, argv);

    argn = 1;
    while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
        {
        if( pm_keymatch( argv[argn], "-saturation", 2 ) )
            {
            if( ++argn > argc )
                pm_usage( usage );
            flags |= SET_SATUR;
            saturation = 100 + atoi( argv[argn] );
            }
        else
        if( pm_keymatch( argv[argn], "-value", 2 ) )
            {
            if( ++argn > argc )
                pm_usage( usage );
            flags |= SET_SATUR;
            value = 100 + atoi( argv[argn] );
            }
        else
        if( pm_keymatch( argv[argn], "-normalize", 2 ) )
            {
            flags |= FIND_MINMAX;
            }
        else
            pm_usage(usage);
        ++argn;
        }

    if (value < 0) value = 0;

    if (saturation < 0) saturation = 0;

    if( argn != argc )
        {
        fp = pm_openr( argv[argn] );
        ++argn;
        }
    else
        fp = stdin;



    if (flags & FIND_MINMAX)
    {
        ppm_readppminit( fp, &cols, &rows, &maxval, &format );
        pixelP = ppm_allocrow( cols );
        max_value = 0;
        min_value = MULTI;
        for (indexv = 0; indexv < rows; indexv++)
        {
            int i;
            ppm_readppmrow( fp, pixelP, cols, maxval, format );
            for (i = 0; i < cols; i++)
            {
                int r, g, b;
                long R, G, B, H, S, V;

                r = PPM_GETR( pixelP[i] );
                g = PPM_GETG( pixelP[i] );
                b = PPM_GETB( pixelP[i] );

                R = (MULTI * r + maxval - 1) / maxval;
                G = (MULTI * g + maxval - 1) / maxval;
                B = (MULTI * b + maxval - 1) / maxval;

                GetHSV( R, G, B, &H, &S, &V );
                if ( V > max_value)     max_value = V;
                if ( V < min_value)     min_value = V;
            }
        }
        ppm_freerow( pixelP );
        pm_message("Min is %4d\tMax = %4d", min_value, max_value );
        rewind( fp );
    }

    ppm_readppminit( fp, &cols, &rows, &maxval, &format );

    ppm_writeppminit( stdout, cols, rows, maxval, 0 );

    pixelP = ppm_allocrow( cols );

    if (pixelP == NULL)
        pm_error( "Error allocating Pixel row" );

    for (indexv = 0; indexv < rows; indexv++)
    {
        int i;
        ppm_readppmrow( fp, pixelP, cols, maxval, format );
        for (i = 0; i < cols; i++)
        {
            int r, g, b;
            long R, G, B, H, S, V;

            r = PPM_GETR( pixelP[i] );
            g = PPM_GETG( pixelP[i] );
            b = PPM_GETB( pixelP[i] );

            R = (MULTI * r + maxval - 1) / maxval;
            G = (MULTI * g + maxval - 1) / maxval;
            B = (MULTI * b + maxval - 1) / maxval;

            GetHSV( R, G, B, &H, &S, &V );

            if (flags & FIND_MINMAX)
            {
                V -= min_value;
                V = (V * MULTI) /
                    (MULTI - (min_value+MULTI-max_value));
            }

            S = ( S * saturation ) / 100;
            V = ( V * value ) / 100;

            if (V > MULTI)
                V = MULTI;
            if (S > MULTI)
                S = MULTI;

            GetRGB( H, S, V, &R, &G, &B );

            r = (R * maxval) / MULTI;
            g = (G * maxval) / MULTI;
            b = (B * maxval) / MULTI;

            PPM_ASSIGN( pixelP[i], r, g, b );
        }

        ppm_writeppmrow( stdout, pixelP, cols, maxval, 0 );
    }
    ppm_freerow( pixelP );

    pm_close( fp );
}

/*
  #define       min(x,y)        ((x < y) ? x : y)
  #define       max(x,y)        ((x > y) ? x : y)
  */
