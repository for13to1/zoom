/* ppmchange.c - change a given color to another
**
** Copyright (C) 1991 by Wilson H. Bent, Jr.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** Modified by Alberto Accomazzi (alberto@cfa.harvard.edu).
**     28 Jan 94 -  Added support for multiple color substitution.
*/

#include "ppm.h"
#define TCOLS 256


int
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    int argn, format, row;
    register int col, i, replaced;
    int rows, cols, ncolors;
    pixel* prow;
    pixel color0[TCOLS], color1[TCOLS];
    pixval maxval;
    char* usage = "<oldcolor> <newcolor> [...] [ppmfile]";

    ppm_init( &argc, argv );

    argn = 1;

    if ( argn == argc )
	pm_usage( usage );
    for ( i = 0; argn < argc - 1 && i < TCOLS; i++ )
        {
	color0[i] = ppm_parsecolor( argv[argn], PPM_MAXMAXVAL );
	++argn;
        if ( argn == argc )
            pm_usage( usage );
	color1[i] = ppm_parsecolor( argv[argn], PPM_MAXMAXVAL );
	++argn;
      }
    ncolors = i;

    if ( argn == argc - 1 )
	ifp = pm_openr( argv[argn] );
    else
	ifp = stdin;

    ppm_readppminit( ifp, &cols, &rows, &maxval, &format );
    ppm_writeppminit( stdout, cols, rows, maxval, 0 );
    prow = ppm_allocrow( cols );

    /* Scan for the desired color */
    for ( row = 0; row < rows; ++row )
	{
	ppm_readppmrow( ifp, prow, cols, maxval, format );
	for ( col = 0; col < cols; ++col )
            for ( i = 0, replaced = 0; i < ncolors && !replaced; i++ )
	        if ( replaced = PPM_EQUAL( prow[col], color0[i] ) )
		    PPM_ASSIGN( prow[col],
			       PPM_GETR( color1[i] ),
			       PPM_GETG( color1[i] ),
			       PPM_GETB( color1[i] ) );
	ppm_writeppmrow( stdout, prow, cols, maxval, 0 );
	}

    pm_close( ifp );

    exit( 0 );
    }
