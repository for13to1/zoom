/* rawtopgm.c - convert raw grayscale bytes into a portable graymap
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <math.h>
#include "pgm.h"


int
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    int argn, headerskip, row, i;
    float rowskip, toskip;
    register int col, val;
    int rows=0, cols=0, topbottom=0;
    char* buf = NULL;
    char* pos;
    long nread = 0;
    char* usage = "[-headerskip N] [-rowskip N] [-tb|-topbottom] [<width> <height>] [rawfile]";
    /* double atof();   should be declared by math.h */


    pgm_init( &argc, argv );

    argn = 1;
    headerskip = 0;
    rowskip = 0.0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-headerskip", 2 ) )
	    {
	    ++argn;
	    if ( argn >= argc )
		pm_usage( usage );
	    headerskip = atoi( argv[argn] );
	    }
	else if ( pm_keymatch( argv[argn], "-rowskip", 2 ) )
	    {
	    ++argn;
	    if ( argn >= argc )
		pm_usage( usage );
	    rowskip = atof( argv[argn] );
	    }
	else if ( pm_keymatch( argv[argn], "-tb", 2 ) ||
		 pm_keymatch( argv[argn], "-topbottom", 2 ))
	    {
		topbottom=1;
	    }
	else
	    pm_usage( usage );
	++argn;
	}

    if ( argn + 2 <= argc ) { /* Read cols and rows */

	cols = atoi( argv[argn++] );
	rows = atoi( argv[argn++] );
	if ( cols <= 0 || rows <= 0 )
	    pm_usage( usage );
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

    if (cols==0 || topbottom) {
	buf = pm_read_unknown_size( ifp, &nread );
	if (cols==0) {
	    rows = cols = (int) sqrt((double) nread);
	    if (rows*cols+headerskip != nread)
		pm_error( "Not a quadratic input picture" );
	    pos = buf;
	    pm_message( "Image size: %d cols, %d rows", cols, rows);
	}
    }

    for ( i = 0; i < headerskip; ++i )
	if (nread)
	    pos++;
	else
	{
	    val = getc( ifp );
	    if ( val == EOF )
		pm_error( "EOF / read error" );
	}
    toskip = 0.00001;

    pgm_writepgminit( stdout, cols, rows, (gray) 255, 0 );
    grayrow = pgm_allocrow( cols );

    for ( row = 0; row < rows; ++row)
    {
	if (topbottom)
	    pos = buf + (rows-row-1) * cols + headerskip;
	for ( col = 0, gP = grayrow; col < cols; ++col )
	    if (nread)
	    {
		*gP++ = *pos++;
	    }
	    else
	    {
		val = getc( ifp );
		if ( val == EOF )
		    pm_error( "EOF / read error" );
		*gP++ = val;
	    }
	for ( toskip += rowskip; toskip >= 1.0; toskip -= 1.0 )
	    if (nread)
	    {
		pos++;
	    }
	    else
	    {
		val = getc( ifp );
		if ( val == EOF )
		    pm_error( "EOF / read error" );
	    }
	pgm_writepgmrow( stdout, grayrow, cols, (gray) 255, 0 );
    }

    if (nread)
	free(buf);
    pm_close( ifp );
    pm_close( stdout );

    exit( 0 );
}
