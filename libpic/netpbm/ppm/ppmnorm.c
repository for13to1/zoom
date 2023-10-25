/* ppmnorm.c - read a portable pixmap and normalize the contrast
**
** by Wilson H. Bent, Jr. (whb@usc.edu)
** Extensively hacked from pgmnorm.c, which carries the following note:
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** (End of note from pgmnorm.c)
*/

#include "ppm.h"

#include "lum.h"

#define MAXMAXVAL 1023

int
main( argc, argv )
int argc;
char *argv[];
    {
    FILE *ifp;
    pixel **pixels, *pixelrow;
    register pixel *pP;
    pixval maxval;
    int argn, rows, cols, format, row;
    int i, size, cutoff, count, val;
    int hist[MAXMAXVAL+1];

    float bpercent, wpercent;
    int range, bvalue, wvalue;
    int specbpercent, specbvalue, specwpercent, specwvalue;
    register int col;
    char *usage = "[-bpercent N | -bvalue N] [-wpercent N | -wvalue N] [ppmfile]";

    ppm_init( &argc, argv );

    argn = 1;
    bpercent = 2.0;
    wpercent = 1.0;
    specbpercent = specbvalue = specwpercent = specwvalue = 0;

    while ( argn + 1 < argc && argv[argn][0] == '-' )
	{
	if ( pm_keymatch( argv[argn], "-bpercent", 3 ) )
	    {
	    if ( specbvalue )
		pm_error( "only one of -bpercent and -bvalue may be specified",
			0,0,0,0,0 );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &bpercent ) != 1 )
		pm_usage( usage );
	    if ( bpercent < 0.0  || bpercent > 100.0 )
		pm_error( "black percentage must between 0 and 100",
			0,0,0,0,0 );
	    specbpercent = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-bvalue", 3 ) )
	    {
	    if ( specbpercent )
		pm_error( "only one of -bpercent and -bvalue may be specified",
			0,0,0,0,0 );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &bvalue ) != 1 )
		pm_usage( usage );
	    if ( bvalue < 0 )
		pm_error( "black value must be >= 0", 0,0,0,0,0 );
	    specbvalue = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-wpercent", 3 ) )
	    {
	    if ( specbvalue )
		pm_error( "only one of -wpercent and -wvalue may be specified",
			0,0,0,0,0 );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &wpercent ) != 1 )
		pm_usage( usage );
	    if ( wpercent < 0.0  || wpercent > 100.0 )
		pm_error( "white percentage must between 0 and 100",
			0,0,0,0,0 );
	    specwpercent = 1;
	    }
	else if ( pm_keymatch( argv[argn], "-wvalue", 3 ) )
	    {
	    if ( specwpercent )
		pm_error( "only one of -wpercent and -wvalue may be specified",
			0,0,0,0,0 );
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &wvalue ) != 1 )
		pm_usage( usage );
	    if ( wvalue < 0 )
		pm_error( "white value must be >= 0", 0,0,0,0,0 );
	    specwvalue = 1;
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn < argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    if ( specbvalue && specwvalue )
	{
	/* Rescale so that bvalue maps to 0, wvalue maps to maxval. */
	ppm_readppminit( ifp, &cols, &rows, &maxval, &format );
	pixelrow = ppm_allocrow( cols );
	ppm_writeppminit( stdout, cols, rows, maxval, 0 );
	for ( i = 0; i <= bvalue; ++i )
	   hist[i] = 0;
	for ( i = wvalue; i <= maxval; ++i )
	    hist[i] = maxval;
	range = wvalue - bvalue;
	for ( i = bvalue+1, val = maxval; i < wvalue; ++i, val += maxval )
	    hist[i] = val / range;
	for ( row = 0; row < rows; row++ )
	    {
	    ppm_readppmrow( ifp, pixelrow, cols, maxval, format );
	    for ( col = 0, pP = pixelrow; col < cols; col++, pP++ )
		{
		PPM_ASSIGN(*pP, hist[PPM_GETR(*pP)], hist[PPM_GETG(*pP)], hist[PPM_GETB(*pP)]);
		}
	    ppm_writeppmrow( stdout, pixelrow, cols, maxval, 0 );
	    }
	pm_close( ifp );
	}
    else
	{
	pixels = ppm_readppm( ifp, &cols, &rows, &maxval );
	pm_close( ifp );

	/* Build histogram. */
	for ( i = 0; i <= maxval; i++ )
	    hist[i] = 0;
	for ( row = 0; row < rows; row++ )
	    for ( col = 0, pP = pixels[row]; col < cols; col++, pP++ )
		{
#ifdef MAX_METHOD
#define max(a,b) ((a) > (b) ? (a) : (b))
		i = max (pP->r, max (pP->g, pP->b));
#else
		i = ( ( times77 [PPM_GETR ( *pP )] +
			times150[PPM_GETG ( *pP )] +
			times29 [PPM_GETB ( *pP )]) >> 8);
#endif
		++hist[i];
		}
	size = rows * cols;
	if ( ! specbvalue )
	    { /* Compute bvalue from bpercent. */
	    cutoff = size * bpercent / 100.0;
	    count = 0;
	    for ( bvalue = 0; bvalue <= maxval; bvalue++ )
		{
		count += hist[bvalue];
		if ( count > cutoff )
		break;
		}
	    }
	if ( ! specwvalue )
	    { /* Compute wvalue from wpercent. */
	    cutoff = size * wpercent / 100.0;
	    count = 0;
	    for ( wvalue = maxval; wvalue >= 0; wvalue-- )
		{
		count += hist[wvalue];
		if ( count > cutoff )
		    break;
		}
	    }

	/* Now rescale so that bvalue maps to 0, wvalue maps to maxval. */
	pm_message(
	    "remapping %d..%d to %d..%d", bvalue, wvalue, 0, maxval, 0 );
	ppm_writeppminit( stdout, cols, rows, maxval, 0 );
	for ( i = 0; i <= bvalue; i++ )
	    hist[i] = 0;
	for ( i = wvalue; i <= maxval; ++i )
	    hist[i] = maxval;
	range = wvalue - bvalue;
	for ( i = bvalue+1, val = maxval; i < wvalue; ++i, val += maxval )
	    hist[i] = val / range;
	for ( row = 0; row < rows; row++ )
	    {
	    for ( col = 0, pP = pixels[row]; col < cols; col++, pP++ )
		{
		PPM_ASSIGN(*pP, hist[PPM_GETR(*pP)], hist[PPM_GETG(*pP)], hist[PPM_GETB(*pP)]);
		}
	    ppm_writeppmrow( stdout, pixels[row], cols, maxval, 0 );
	    }
	}

    exit( 0 );
    }
