/* pnmconvol.c - general MxN convolution on a portable anymap
**
** Version 2.0.1 January 30, 1995
**
** Major rewriting by Mike Burns
** Copyright (C) 1994, 1995 by Mike Burns (burns@chem.psu.edu)
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/* Version 2.0.1 Changes
** ---------------------
** Fixed four lines that were improperly allocated as sizeof( float ) when they
** should have been sizeof( long ).
**
** Version 2.0 Changes
** -------------------
** Reduce run time by general optimizations and handling special cases of
** convolution matrices.  Program automatically determines if convolution 
** matrix is one of the types it can make use of so no extra command line
** arguments are necessary.
**
** Examples of convolution matrices for the special cases are
**
**    Mean       Horizontal    Vertical
**    x x x        x x x        x y z
**    x x x        y y y        x y z
**    x x x        z z z        x y z
**
** I don't know if the horizontal and vertical ones are of much use, but
** after working on the mean convolution, it gave me ideas for the other two.
**
** Some other compiler dependent optimizations
** -------------------------------------------
** Created separate functions as code was getting too large to put keep both
** PGM and PPM cases in same function and also because SWITCH statement in
** inner loop can take progressively more time the larger the size of the 
** convolution matrix.  GCC is affected this way.
**
** Removed use of MOD (%) operator from innermost loop by modifying manner in
** which the current xelbuf[] is chosen.
** 
*/

#include "pnm.h"

/* Macros to verify that r,g,b values are within proper range */

#define CHECK_GRAY \
    if ( tempgsum < 0L ) g = 0; \
    else if ( tempgsum > maxval ) g = maxval; \
    else g = tempgsum;

#define CHECK_RED \
    if ( temprsum < 0L ) r = 0; \
    else if ( temprsum > maxval ) r = maxval; \
    else r = temprsum;

#define CHECK_GREEN \
    if ( tempgsum < 0L ) g = 0; \
    else if ( tempgsum > maxval ) g = maxval; \
    else g = tempgsum;

#define CHECK_BLUE \
    if ( tempbsum < 0L ) b = 0; \
    else if ( tempbsum > maxval ) b = maxval; \
    else b = tempbsum;


static int check_convolve_type ARGS((xel **cxels));
static void pgm_general_convolve ARGS((void));
static void ppm_general_convolve ARGS((void));
static void pgm_mean_convolve ARGS((void));
static void ppm_mean_convolve ARGS((void));
static void pgm_vertical_convolve ARGS((void));
static void ppm_vertical_convolve ARGS((void));
static void pgm_horizontal_convolve ARGS((void));
static void ppm_horizontal_convolve ARGS((void));

#define TRUE	1
#define FALSE	0

#define GENERAL_CONVOLVE	0
#define MEAN_CONVOLVE		1
#define HORIZONTAL_CONVOLVE	2
#define VERTICAL_CONVOLVE	3

static FILE* ifp;
static float** rweights;
static float** gweights;
static float** bweights;
static int crows, ccols, ccolso2, crowso2;
static int cols, rows;
static xelval maxval;
static int format, newformat;
static float rmeanweight, gmeanweight, bmeanweight;

int
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* cifp;
    xel** cxels;
    int argn, cformat;
    int crow, row;
    register int ccol, col;
    xelval cmaxval;
    xelval g;
    float gsum;
    float rsum, bsum;
    char* usage = "<convolutionfile> [pnmfile]";
    int convolve_type;

    pnm_init( &argc, argv );

    argn = 1;

    if ( argn == argc )
	pm_usage( usage );
    cifp = pm_openr( argv[argn] );
    ++argn;

    if ( argn != argc )
	{
	ifp = pm_openr( argv[argn] );
	++argn;
	}
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    pnm_pbmmaxval = PNM_MAXMAXVAL;  /* use larger value for better results */

    /* Read in the convolution matrix. */
    cxels = pnm_readpnm( cifp, &ccols, &crows, &cmaxval, &cformat );
    pm_close( cifp );
    if ( ccols % 2 != 1 || crows % 2 != 1 )
	pm_error(
	 "the convolution matrix must have an odd number of rows and columns" );
    ccolso2 = ccols / 2;
    crowso2 = crows / 2;

    pnm_readpnminit( ifp, &cols, &rows, &maxval, &format );
    if ( cols < ccols || rows < crows )
	pm_error(
	    "the image is smaller than the convolution matrix" );

    newformat = max( PNM_FORMAT_TYPE(cformat), PNM_FORMAT_TYPE(format) );
    if ( PNM_FORMAT_TYPE(cformat) != newformat )
	pnm_promoteformat( cxels, ccols, crows, cmaxval, cformat, cmaxval, newformat );
    if ( PNM_FORMAT_TYPE(format) != newformat )
        {
        switch ( PNM_FORMAT_TYPE(newformat) )
            {
            case PPM_TYPE:
            if ( PNM_FORMAT_TYPE(format) != newformat )
                pm_message( "promoting to PPM" );
            break;
            case PGM_TYPE:
            if ( PNM_FORMAT_TYPE(format) != newformat )
                pm_message( "promoting to PGM" );
            break;
            }
        }


    /* Set up the normalized weights. */
    rweights = (float**) pm_allocarray( ccols, crows, sizeof(float) );
    gweights = (float**) pm_allocarray( ccols, crows, sizeof(float) );
    bweights = (float**) pm_allocarray( ccols, crows, sizeof(float) );
    rsum = gsum = bsum = 0;
    for ( crow = 0; crow < crows; ++crow )
	for ( ccol = 0; ccol < ccols; ++ccol )
	    {
	    switch ( PNM_FORMAT_TYPE(format) )
		{
		case PPM_TYPE:
		rsum += rweights[crow][ccol] =
		    ( PPM_GETR(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		gsum += gweights[crow][ccol] =
		    ( PPM_GETG(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		bsum += bweights[crow][ccol] =
		    ( PPM_GETB(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		break;

		default:
		gsum += gweights[crow][ccol] =
		    ( PNM_GET1(cxels[crow][ccol]) * 2.0 / cmaxval - 1.0 );
		break;
		}
	    }

    /* For mean_convolve routines.  All weights of a single color are the same
    ** so just grab any one of them.
    */
    rmeanweight = rweights[0][0];
    gmeanweight = gweights[0][0];
    bmeanweight = bweights[0][0];

    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	if ( rsum < 0.9 || rsum > 1.1 || gsum < 0.9 || gsum > 1.1 ||
	     bsum < 0.9 || bsum > 1.1 )
	    pm_message(
		"WARNING - this convolution matrix is biased" );
	break;

	default:
	if ( gsum < 0.9 || gsum > 1.1 )
	    pm_message(
		 "WARNING - this convolution matrix is biased" );
	break;
	}

    /* Handle certain special cases when runtime can be improved. */

    convolve_type = check_convolve_type(cxels);

    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	switch (convolve_type)
	    {
	    case MEAN_CONVOLVE:
	    ppm_mean_convolve();
	    break;

	    case HORIZONTAL_CONVOLVE:
	    ppm_horizontal_convolve();
	    break;

	    case VERTICAL_CONVOLVE:
	    ppm_vertical_convolve();
	    break;

	    default: /* GENERAL_CONVOLVE */
	    ppm_general_convolve();
	    break;  
	    }
	break;

	default:
	switch (convolve_type)
	    {
	    case MEAN_CONVOLVE:
	    pgm_mean_convolve();
	    break;

	    case HORIZONTAL_CONVOLVE:
	    pgm_horizontal_convolve();
	    break;

	    case VERTICAL_CONVOLVE:
	    pgm_vertical_convolve();
	    break;

	    default: /* GENERAL_CONVOLVE */
	    pgm_general_convolve();
	    break;  
	    }
	break;
	} /* PNM_TYPE */

    pm_close( ifp );
    exit( 0 );
    }



/* check_convolve_type
**
** Determine if the convolution matrix is one of the special cases that
** can be processed faster than the general form.
**
** Does not check for the case where one of the PPM colors can have differing 
** types.  Only handles cases where all PPM's are of the same special case.
*/

static int
check_convolve_type (cxels)
    xel **cxels;
    {
    int convolve_type;
    int horizontal, vertical;
    int tempcxel, rtempcxel, gtempcxel, btempcxel;
    int crow, ccol;

    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	horizontal = TRUE;
	crow = 0;
	while ( horizontal && (crow < crows) )
	    {
	    ccol = 1;
	    rtempcxel = PPM_GETR(cxels[crow][0]);
	    gtempcxel = PPM_GETG(cxels[crow][0]);
	    btempcxel = PPM_GETB(cxels[crow][0]);
	    while ( horizontal && (ccol < ccols) )
		{
		if (( PPM_GETR(cxels[crow][ccol]) != rtempcxel ) |
		    ( PPM_GETG(cxels[crow][ccol]) != gtempcxel ) |
		    ( PPM_GETB(cxels[crow][ccol]) != btempcxel )) 
		    horizontal = FALSE;
		++ccol;
		}
	    ++crow;
	    }

	vertical = TRUE;
	ccol = 0;
	while ( vertical && (ccol < ccols) )
	    {
	    crow = 1;
	    rtempcxel = PPM_GETR(cxels[0][ccol]);
	    gtempcxel = PPM_GETG(cxels[0][ccol]);
	    btempcxel = PPM_GETB(cxels[0][ccol]);
	    while ( vertical && (crow < crows) )
		{
		if (( PPM_GETR(cxels[crow][ccol]) != rtempcxel ) |
		    ( PPM_GETG(cxels[crow][ccol]) != gtempcxel ) |
		    ( PPM_GETB(cxels[crow][ccol]) != btempcxel ))
		    vertical = FALSE;
		++crow;
		}
	    ++ccol;
	    }
	break;

	default:
	horizontal = TRUE;
	crow = 0;
	while ( horizontal && (crow < crows) )
	    {
	    ccol = 1;
	    tempcxel = PNM_GET1(cxels[crow][0]);
	    while ( horizontal && (ccol < ccols) )
		{
		if ( PNM_GET1(cxels[crow][ccol]) != tempcxel )
		    horizontal = FALSE;
		++ccol;
		}
	    ++crow;
	    }

	vertical = TRUE;
	ccol = 0;
	while ( vertical && (ccol < ccols) )
	    {
	    crow = 1;
	    tempcxel = PNM_GET1(cxels[0][ccol]);
	    while ( vertical && (crow < crows) )
		{
		if ( PNM_GET1(cxels[crow][ccol]) != tempcxel )
		    vertical = FALSE;
		++crow;
		}
	    ++ccol;
	    }
	break;
	}

    /* Which type do we have? */
    if ( horizontal && vertical )
	convolve_type = MEAN_CONVOLVE;
    else if ( horizontal )
	convolve_type = HORIZONTAL_CONVOLVE;
    else if ( vertical )
	convolve_type = VERTICAL_CONVOLVE;
    else
	convolve_type = GENERAL_CONVOLVE;

    return (convolve_type);
    }




/* General PGM Convolution
**
** No useful redundancy in convolution matrix.
*/

static void
pgm_general_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xel x, y;
    xelval g;
    int row, crow;
    float gsum;
    xel **rowptr, *temprptr;
    int toprow, temprow;
    int i, irow;
    int leftcol;
    long tempgsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. */
    xelbuf = pnm_allocarray( cols, crows );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf */
    rowptr = (xel **) pnm_allocarray( 1, crows );

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Now the rest of the image - read in the row at the end of
    ** xelbuf, and convolve and write out the row in the middle.
    */
    for ( ; row < rows; ++row )
	{
	toprow = row + 1;
	temprow = row % crows;
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	/* Arrange rowptr to eliminate the use of mod function to determine
	** which row of xelbuf is 0...crows.  Mod function can be very costly.
	*/
	temprow = toprow % crows;
	i = 0;
	for (irow = temprow; irow < crows; ++i, ++irow)
	    rowptr[i] = xelbuf[irow];
	for (irow = 0; irow < temprow; ++irow, ++i)
	    rowptr[i] = xelbuf[irow];

	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else
		{
		leftcol = col - ccolso2;
		gsum = 0.0;
		for ( crow = 0; crow < crows; ++crow )
		    {
		    temprptr = rowptr[crow] + leftcol;
		    for ( ccol = 0; ccol < ccols; ++ccol )
			gsum += PNM_GET1( *(temprptr + ccol) )
			    * gweights[crow][ccol];
		    }
		    tempgsum = gsum + 0.5;
		    CHECK_GRAY;
		    PNM_ASSIGN1( outputrow[col], g );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }


/* PGM Mean Convolution
**
** This is the common case where you just want the target pixel replaced with
** the average value of its neighbors.  This can work much faster than the
** general case because you can reduce the number of floating point operations
** that are required since all the weights are the same.  You will only need
** to multiply by the weight once, not for every pixel in the convolution
** matrix.
**
** This algorithm works by creating sums for each column of crows height for
** the whole width of the image.  Then add ccols column sums together to obtain
** the total sum of the neighbors and multiply that sum by the weight.  As you
** move right to left to calculate the next pixel, take the total sum you just
** generated, add in the value of the next column and subtract the value of the
** leftmost column.  Multiply that by the weight and that's it.  As you move
** down a row, calculate new column sums by using previous sum for that column
** and adding in pixel on current row and subtracting pixel in top row.
**
*/


static void
pgm_mean_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xelval g;
    int row, crow;
    xel **rowptr, *temprptr;
    int leftcol;
    int i, irow;
    int toprow, temprow;
    int subrow, addrow;
    int subcol, addcol;
    long gisum;
    int tempcol, crowsp1;
    long tempgsum;
    long *gcolumnsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer.  MEAN uses an extra row. */
    xelbuf = pnm_allocarray( cols, crows + 1 );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf. MEAN uses an extra row. */
    rowptr = (xel **) pnm_allocarray( 1, crows + 1);

    /* Allocate space for intermediate column sums */
    gcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    for ( col = 0; col < cols; ++col )
	gcolumnsum[col] = 0L;

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Do first real row only */
    subrow = crows;
    addrow = crows - 1;
    toprow = row + 1;
    temprow = row % crows;
    pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
    if ( PNM_FORMAT_TYPE(format) != newformat )
	pnm_promoteformatrow(
	    xelbuf[temprow], cols, maxval, format, maxval, newformat );

    temprow = toprow % crows;
    i = 0;
    for (irow = temprow; irow < crows; ++i, ++irow)
	rowptr[i] = xelbuf[irow];
    for (irow = 0; irow < temprow; ++irow, ++i)
	rowptr[i] = xelbuf[irow];

    gisum = 0L;
    for ( col = 0; col < cols; ++col )
	{
	if ( col < ccolso2 || col >= cols - ccolso2 )
	    outputrow[col] = rowptr[crowso2][col];
	else if ( col == ccolso2 )
	    {
	    leftcol = col - ccolso2;
	    for ( crow = 0; crow < crows; ++crow )
		{
		temprptr = rowptr[crow] + leftcol;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    gcolumnsum[leftcol + ccol] += 
			PNM_GET1( *(temprptr + ccol) );
		}
	    for ( ccol = 0; ccol < ccols; ++ccol)
		gisum += gcolumnsum[leftcol + ccol];
	    tempgsum = (float) gisum * gmeanweight + 0.5;
	    CHECK_GRAY;
	    PNM_ASSIGN1( outputrow[col], g );
	    }
	else
	    {
	    /* Column numbers to subtract or add to isum */
	    subcol = col - ccolso2 - 1;
	    addcol = col + ccolso2;  
	    for ( crow = 0; crow < crows; ++crow )
		gcolumnsum[addcol] += PNM_GET1( rowptr[crow][addcol] );
	    gisum = gisum - gcolumnsum[subcol] + gcolumnsum[addcol];
	    tempgsum = (float) gisum * gmeanweight + 0.5;
	    CHECK_GRAY;
	    PNM_ASSIGN1( outputrow[col], g );
	    }
	}
    pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );

    ++row;
    /* For all subsequent rows do it this way as the columnsums have been
    ** generated.  Now we can use them to reduce further calculations.
    */
    crowsp1 = crows + 1;
    for ( ; row < rows; ++row )
	{
	toprow = row + 1;
	temprow = row % (crows + 1);
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	/* This rearrangement using crows+1 rowptrs and xelbufs will cause
	** rowptr[0..crows-1] to always hold active xelbufs and for 
	** rowptr[crows] to always hold the oldest (top most) xelbuf.
	*/
	temprow = (toprow + 1) % crowsp1;
	i = 0;
	for (irow = temprow; irow < crowsp1; ++i, ++irow)
	    rowptr[i] = xelbuf[irow];
	for (irow = 0; irow < temprow; ++irow, ++i)
	    rowptr[i] = xelbuf[irow];

	gisum = 0L;
	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else if ( col == ccolso2 )
		{
		leftcol = col - ccolso2;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    tempcol = leftcol + ccol;
		    gcolumnsum[tempcol] = gcolumnsum[tempcol]
			- PNM_GET1( rowptr[subrow][ccol] )
			+ PNM_GET1( rowptr[addrow][ccol] );
		    gisum += gcolumnsum[tempcol];
		    }
		tempgsum = (float) gisum * gmeanweight + 0.5;
		CHECK_GRAY;
		PNM_ASSIGN1( outputrow[col], g );
		}
	    else
		{
		/* Column numbers to subtract or add to isum */
		subcol = col - ccolso2 - 1;
		addcol = col + ccolso2;  
		gcolumnsum[addcol] = gcolumnsum[addcol]
		    - PNM_GET1( rowptr[subrow][addcol] )
		    + PNM_GET1( rowptr[addrow][addcol] );
		gisum = gisum - gcolumnsum[subcol] + gcolumnsum[addcol];
		tempgsum = (float) gisum * gmeanweight + 0.5;
		CHECK_GRAY;
		PNM_ASSIGN1( outputrow[col], g );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }


/* PGM Horizontal Convolution
**
** Similar idea to using columnsums of the Mean and Vertical convolution,
** but uses temporary sums of row values.  Need to multiply by weights crows
** number of times.  Each time a new line is started, must recalculate the
** initials rowsums for the newest row only.  Uses queue to still access
** previous row sums.
**
*/

static void
pgm_horizontal_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xel x;
    xelval g;
    int row, crow;
    xel **rowptr, *temprptr;
    int leftcol;
    int i, irow;
    int temprow;
    int subcol, addcol;
    float gsum;
    int addrow, subrow;
    long **growsum, **growsumptr;
    int crowsp1;
    long tempgsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. */
    xelbuf = pnm_allocarray( cols, crows + 1 );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf */
    rowptr = (xel **) pnm_allocarray( 1, crows + 1);

    /* Allocate intermediate row sums.  HORIZONTAL uses an extra row. */
    /* crows current rows and 1 extra for newest added row.           */
    growsum = (long **) pm_allocarray( cols, crows + 1, sizeof(long) );
    growsumptr = (long **) pnm_allocarray( 1, crows + 1);

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* First row only */
    temprow = row % crows;
    pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
    if ( PNM_FORMAT_TYPE(format) != newformat )
	pnm_promoteformatrow(
	    xelbuf[temprow], cols, maxval, format, maxval, newformat );

    temprow = (row + 1) % crows;
    i = 0;
    for (irow = temprow; irow < crows; ++i, ++irow)
	rowptr[i] = xelbuf[irow];
    for (irow = 0; irow < temprow; ++irow, ++i)
	rowptr[i] = xelbuf[irow];

    for ( crow = 0; crow < crows; ++crow )
	growsumptr[crow] = growsum[crow];
 
    for ( col = 0; col < cols; ++col )
	{
	if ( col < ccolso2 || col >= cols - ccolso2 )
	    outputrow[col] = rowptr[crowso2][col];
	else if ( col == ccolso2 )
	    {
	    leftcol = col - ccolso2;
	    gsum = 0.0;
	    for ( crow = 0; crow < crows; ++crow )
		{
		temprptr = rowptr[crow] + leftcol;
		growsumptr[crow][leftcol] = 0L;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    growsumptr[crow][leftcol] += 
		        PNM_GET1( *(temprptr + ccol) );
		gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		}
	    tempgsum = gsum + 0.5;
	    CHECK_GRAY;
	    PNM_ASSIGN1( outputrow[col], g );
	    }
	else
	    {
	    gsum = 0.0;
	    leftcol = col - ccolso2;
	    subcol = col - ccolso2 - 1;
	    addcol = col + ccolso2;
	    for ( crow = 0; crow < crows; ++crow )
		{
		growsumptr[crow][leftcol] = growsumptr[crow][subcol]
		    - PNM_GET1( rowptr[crow][subcol] )
		    + PNM_GET1( rowptr[crow][addcol] );
		gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		}
	    tempgsum = gsum + 0.5;
	    CHECK_GRAY;
	    PNM_ASSIGN1( outputrow[col], g );
	    }
        }
    pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );


    /* For all subsequent rows */

    subrow = crows;
    addrow = crows - 1;
    crowsp1 = crows + 1;
    ++row;
    for ( ; row < rows; ++row )
	{
	temprow = row % crowsp1;
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	temprow = (row + 2) % crowsp1;
	i = 0;
	for (irow = temprow; irow < crowsp1; ++i, ++irow)
	    {
	    rowptr[i] = xelbuf[irow];
	    growsumptr[i] = growsum[irow];
	    }
	for (irow = 0; irow < temprow; ++irow, ++i)
	    {
	    rowptr[i] = xelbuf[irow];
	    growsumptr[i] = growsum[irow];
	    }

	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else if ( col == ccolso2 )
		{
		gsum = 0.0;
		leftcol = col - ccolso2;
		growsumptr[addrow][leftcol] = 0L;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    growsumptr[addrow][leftcol] += 
			PNM_GET1( rowptr[addrow][leftcol + ccol] );
		for ( crow = 0; crow < crows; ++crow )
		    gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		tempgsum = gsum + 0.5;
		CHECK_GRAY;
		PNM_ASSIGN1( outputrow[col], g );
		}
	    else
		{
		gsum = 0.0;
		leftcol = col - ccolso2;
		subcol = col - ccolso2 - 1;
		addcol = col + ccolso2;  
		growsumptr[addrow][leftcol] = growsumptr[addrow][subcol]
		    - PNM_GET1( rowptr[addrow][subcol] )
		    + PNM_GET1( rowptr[addrow][addcol] );
		for ( crow = 0; crow < crows; ++crow )
		    gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		tempgsum = gsum + 0.5;
		CHECK_GRAY;
		PNM_ASSIGN1( outputrow[col], g );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }


/* PGM Vertical Convolution
**
** Uses column sums as in Mean Convolution.
**
*/


static void
pgm_vertical_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xelval g;
    int row, crow;
    xel **rowptr, *temprptr;
    int leftcol;
    int i, irow;
    int toprow, temprow;
    int subrow, addrow;
    int tempcol;
    float gsum;
    long *gcolumnsum;
    int crowsp1;
    int addcol;
    long tempgsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. VERTICAL uses an extra row. */
    xelbuf = pnm_allocarray( cols, crows + 1 );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf */
    rowptr = (xel **) pnm_allocarray( 1, crows + 1 );

    /* Allocate space for intermediate column sums */
    gcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    for ( col = 0; col < cols; ++col )
	gcolumnsum[col] = 0L;

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Now the rest of the image - read in the row at the end of
    ** xelbuf, and convolve and write out the row in the middle.
    */
    /* For first row only */

    toprow = row + 1;
    temprow = row % crows;
    pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
    if ( PNM_FORMAT_TYPE(format) != newformat )
	pnm_promoteformatrow(
	    xelbuf[temprow], cols, maxval, format, maxval, newformat );

    /* Arrange rowptr to eliminate the use of mod function to determine
    ** which row of xelbuf is 0...crows.  Mod function can be very costly.
    */
    temprow = toprow % crows;
    i = 0;
    for (irow = temprow; irow < crows; ++i, ++irow)
	rowptr[i] = xelbuf[irow];
    for (irow = 0; irow < temprow; ++irow, ++i)
	rowptr[i] = xelbuf[irow];

    for ( col = 0; col < cols; ++col )
	{
	if ( col < ccolso2 || col >= cols - ccolso2 )
	    outputrow[col] = rowptr[crowso2][col];
	else if ( col == ccolso2 )
	    {
	    gsum = 0.0;
	    leftcol = col - ccolso2;
	    for ( crow = 0; crow < crows; ++crow )
		{
		temprptr = rowptr[crow] + leftcol;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    gcolumnsum[leftcol + ccol] += 
			PNM_GET1( *(temprptr + ccol) );
		}
	    for ( ccol = 0; ccol < ccols; ++ccol)
		gsum += gcolumnsum[leftcol + ccol] * gweights[0][ccol];
	    tempgsum = gsum + 0.5;
	    CHECK_GRAY;
	    PNM_ASSIGN1( outputrow[col], g );
	    }
	else
	    {
	    gsum = 0.0;
	    leftcol = col - ccolso2;
	    addcol = col + ccolso2;  
	    for ( crow = 0; crow < crows; ++crow )
		gcolumnsum[addcol] += PNM_GET1( rowptr[crow][addcol] );
	    for ( ccol = 0; ccol < ccols; ++ccol )
		gsum += gcolumnsum[leftcol + ccol] * gweights[0][ccol];
	    tempgsum = gsum + 0.5;
	    CHECK_GRAY;
	    PNM_ASSIGN1( outputrow[col], g );
	    }
	}
    pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );

    /* For all subsequent rows */
    subrow = crows;
    addrow = crows - 1;
    crowsp1 = crows + 1;
    ++row;
    for ( ; row < rows; ++row )
	{
	toprow = row + 1;
	temprow = row % (crows +1);
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	/* Arrange rowptr to eliminate the use of mod function to determine
	** which row of xelbuf is 0...crows.  Mod function can be very costly.
	*/
	temprow = (toprow + 1) % crowsp1;
	i = 0;
	for (irow = temprow; irow < crowsp1; ++i, ++irow)
	    rowptr[i] = xelbuf[irow];
	for (irow = 0; irow < temprow; ++irow, ++i)
	    rowptr[i] = xelbuf[irow];

	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else if ( col == ccolso2 )
		{
		gsum = 0.0;
		leftcol = col - ccolso2;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    tempcol = leftcol + ccol;
		    gcolumnsum[tempcol] = gcolumnsum[tempcol] 
			- PNM_GET1( rowptr[subrow][ccol] )
			+ PNM_GET1( rowptr[addrow][ccol] );
		    gsum = gsum + gcolumnsum[tempcol] * gweights[0][ccol];
		    }
		tempgsum = gsum + 0.5;
		CHECK_GRAY;
		PNM_ASSIGN1( outputrow[col], g );
		}
	    else
		{
		gsum = 0.0;
		leftcol = col - ccolso2;
		addcol = col + ccolso2;
		gcolumnsum[addcol] = gcolumnsum[addcol]
		    - PNM_GET1( rowptr[subrow][addcol] )
		    + PNM_GET1( rowptr[addrow][addcol] );
		for ( ccol = 0; ccol < ccols; ++ccol )
		    gsum += gcolumnsum[leftcol + ccol] * gweights[0][ccol];
		tempgsum = gsum + 0.5;
		CHECK_GRAY;
		PNM_ASSIGN1( outputrow[col], g );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }




/* PPM General Convolution Algorithm
**
** No redundancy in convolution matrix.  Just use brute force.
** See pgm_general_convolve() for more details.
*/

static void
ppm_general_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xel x, y;
    xelval r, g, b;
    int row, crow;
    float rsum, gsum, bsum;
    xel **rowptr, *temprptr;
    int toprow, temprow;
    int i, irow;
    int leftcol;
    long temprsum, tempgsum, tempbsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. */
    xelbuf = pnm_allocarray( cols, crows );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf */
    rowptr = (xel **) pnm_allocarray( 1, crows );

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Now the rest of the image - read in the row at the end of
    ** xelbuf, and convolve and write out the row in the middle.
    */
    for ( ; row < rows; ++row )
	{
	toprow = row + 1;
	temprow = row % crows;
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	/* Arrange rowptr to eliminate the use of mod function to determine
	** which row of xelbuf is 0...crows.  Mod function can be very costly.
	*/
	temprow = toprow % crows;
	i = 0;
	for (irow = temprow; irow < crows; ++i, ++irow)
	    rowptr[i] = xelbuf[irow];
	for (irow = 0; irow < temprow; ++irow, ++i)
	    rowptr[i] = xelbuf[irow];

	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else
		{
		leftcol = col - ccolso2;
		rsum = gsum = bsum = 0.0;
		for ( crow = 0; crow < crows; ++crow )
		    {
		    temprptr = rowptr[crow] + leftcol;
		    for ( ccol = 0; ccol < ccols; ++ccol )
			{
			rsum += PPM_GETR( *(temprptr + ccol) )
			    * rweights[crow][ccol];
			gsum += PPM_GETG( *(temprptr + ccol) )
			    * gweights[crow][ccol];
			bsum += PPM_GETB( *(temprptr + ccol) )
			    * bweights[crow][ccol];
			}
		    }
		    temprsum = rsum + 0.5;
		    tempgsum = gsum + 0.5;
		    tempbsum = bsum + 0.5;
		    CHECK_RED;
		    CHECK_GREEN;
		    CHECK_BLUE;
		    PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }


/* PPM Mean Convolution
**
** Same as pgm_mean_convolve() but for PPM.
**
*/

static void
ppm_mean_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xelval r, g, b;
    int row, crow;
    xel **rowptr, *temprptr;
    int leftcol;
    int i, irow;
    int toprow, temprow;
    int subrow, addrow;
    int subcol, addcol;
    long risum, gisum, bisum;
    float rsum, gsum, bsum;
    long temprsum, tempgsum, tempbsum;
    int tempcol, crowsp1;
    long *rcolumnsum, *gcolumnsum, *bcolumnsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer.  MEAN uses an extra row. */
    xelbuf = pnm_allocarray( cols, crows + 1 );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf. MEAN uses an extra row. */
    rowptr = (xel **) pnm_allocarray( 1, crows + 1);

    /* Allocate space for intermediate column sums */
    rcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    gcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    bcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    for ( col = 0; col < cols; ++col )
	{
	rcolumnsum[col] = 0L;
	gcolumnsum[col] = 0L;
	bcolumnsum[col] = 0L;
	}

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Do first real row only */
    subrow = crows;
    addrow = crows - 1;
    toprow = row + 1;
    temprow = row % crows;
    pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
    if ( PNM_FORMAT_TYPE(format) != newformat )
	pnm_promoteformatrow(
	    xelbuf[temprow], cols, maxval, format, maxval, newformat );

    temprow = toprow % crows;
    i = 0;
    for (irow = temprow; irow < crows; ++i, ++irow)
	rowptr[i] = xelbuf[irow];
    for (irow = 0; irow < temprow; ++irow, ++i)
	rowptr[i] = xelbuf[irow];

    risum = 0L;
    gisum = 0L;
    bisum = 0L;
    for ( col = 0; col < cols; ++col )
	{
	if ( col < ccolso2 || col >= cols - ccolso2 )
	    outputrow[col] = rowptr[crowso2][col];
	else if ( col == ccolso2 )
	    {
	    leftcol = col - ccolso2;
	    for ( crow = 0; crow < crows; ++crow )
		{
		temprptr = rowptr[crow] + leftcol;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    rcolumnsum[leftcol + ccol] += 
			PPM_GETR( *(temprptr + ccol) );
		    gcolumnsum[leftcol + ccol] += 
			PPM_GETG( *(temprptr + ccol) );
		    bcolumnsum[leftcol + ccol] += 
			PPM_GETB( *(temprptr + ccol) );
		    }
		}
	    for ( ccol = 0; ccol < ccols; ++ccol)
		{
		risum += rcolumnsum[leftcol + ccol];
		gisum += gcolumnsum[leftcol + ccol];
		bisum += bcolumnsum[leftcol + ccol];
		}
	    temprsum = (float) risum * rmeanweight + 0.5;
	    tempgsum = (float) gisum * gmeanweight + 0.5;
	    tempbsum = (float) bisum * bmeanweight + 0.5;
	    CHECK_RED;
	    CHECK_GREEN;
	    CHECK_BLUE;
	    PPM_ASSIGN( outputrow[col], r, g, b );
	    }
	else
	    {
	    /* Column numbers to subtract or add to isum */
	    subcol = col - ccolso2 - 1;
	    addcol = col + ccolso2;  
	    for ( crow = 0; crow < crows; ++crow )
		{
		rcolumnsum[addcol] += PPM_GETR( rowptr[crow][addcol] );
		gcolumnsum[addcol] += PPM_GETG( rowptr[crow][addcol] );
		bcolumnsum[addcol] += PPM_GETB( rowptr[crow][addcol] );
		}
	    risum = risum - rcolumnsum[subcol] + rcolumnsum[addcol];
	    gisum = gisum - gcolumnsum[subcol] + gcolumnsum[addcol];
	    bisum = bisum - bcolumnsum[subcol] + bcolumnsum[addcol];
	    temprsum = (float) risum * rmeanweight + 0.5;
	    tempgsum = (float) gisum * gmeanweight + 0.5;
	    tempbsum = (float) bisum * bmeanweight + 0.5;
	    CHECK_RED;
	    CHECK_GREEN;
	    CHECK_BLUE;
	    PPM_ASSIGN( outputrow[col], r, g, b );
	    }
	}
    pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );

    ++row;
    /* For all subsequent rows do it this way as the columnsums have been
    ** generated.  Now we can use them to reduce further calculations.
    */
    crowsp1 = crows + 1;
    for ( ; row < rows; ++row )
	{
	toprow = row + 1;
	temprow = row % (crows + 1);
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	/* This rearrangement using crows+1 rowptrs and xelbufs will cause
	** rowptr[0..crows-1] to always hold active xelbufs and for 
	** rowptr[crows] to always hold the oldest (top most) xelbuf.
	*/
	temprow = (toprow + 1) % crowsp1;
	i = 0;
	for (irow = temprow; irow < crowsp1; ++i, ++irow)
	    rowptr[i] = xelbuf[irow];
	for (irow = 0; irow < temprow; ++irow, ++i)
	    rowptr[i] = xelbuf[irow];

	risum = 0L;
	gisum = 0L;
	bisum = 0L;
	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else if ( col == ccolso2 )
		{
		leftcol = col - ccolso2;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    tempcol = leftcol + ccol;
		    rcolumnsum[tempcol] = rcolumnsum[tempcol]
			- PPM_GETR( rowptr[subrow][ccol] )
			+ PPM_GETR( rowptr[addrow][ccol] );
		    risum += rcolumnsum[tempcol];
		    gcolumnsum[tempcol] = gcolumnsum[tempcol]
			- PPM_GETG( rowptr[subrow][ccol] )
			+ PPM_GETG( rowptr[addrow][ccol] );
		    gisum += gcolumnsum[tempcol];
		    bcolumnsum[tempcol] = bcolumnsum[tempcol]
			- PPM_GETB( rowptr[subrow][ccol] )
			+ PPM_GETB( rowptr[addrow][ccol] );
		    bisum += bcolumnsum[tempcol];
		    }
		temprsum = (float) risum * rmeanweight + 0.5;
		tempgsum = (float) gisum * gmeanweight + 0.5;
		tempbsum = (float) bisum * bmeanweight + 0.5;
		CHECK_RED;
		CHECK_GREEN;
		CHECK_BLUE;
		PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    else
		{
		/* Column numbers to subtract or add to isum */
		subcol = col - ccolso2 - 1;
		addcol = col + ccolso2;  
		rcolumnsum[addcol] = rcolumnsum[addcol]
		    - PPM_GETR( rowptr[subrow][addcol] )
		    + PPM_GETR( rowptr[addrow][addcol] );
		risum = risum - rcolumnsum[subcol] + rcolumnsum[addcol];
		gcolumnsum[addcol] = gcolumnsum[addcol]
		    - PPM_GETG( rowptr[subrow][addcol] )
		    + PPM_GETG( rowptr[addrow][addcol] );
		gisum = gisum - gcolumnsum[subcol] + gcolumnsum[addcol];
		bcolumnsum[addcol] = bcolumnsum[addcol]
		    - PPM_GETB( rowptr[subrow][addcol] )
		    + PPM_GETB( rowptr[addrow][addcol] );
		bisum = bisum - bcolumnsum[subcol] + bcolumnsum[addcol];
		temprsum = (float) risum * rmeanweight + 0.5;
		tempgsum = (float) gisum * gmeanweight + 0.5;
		tempbsum = (float) bisum * bmeanweight + 0.5;
		CHECK_RED;
		CHECK_GREEN;
		CHECK_BLUE;
		PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }


/* PPM Horizontal Convolution
**
** Same as pgm_horizontal_convolve()
**
**/

static void
ppm_horizontal_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xel x;
    xelval r, g, b;
    int row, crow;
    xel **rowptr, *temprptr;
    int leftcol;
    int i, irow;
    int temprow;
    int subcol, addcol;
    float rsum, gsum, bsum;
    int addrow, subrow;
    long **rrowsum, **rrowsumptr;
    long **growsum, **growsumptr;
    long **browsum, **browsumptr;
    int crowsp1;
    long temprsum, tempgsum, tempbsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. */
    xelbuf = pnm_allocarray( cols, crows + 1 );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf */
    rowptr = (xel **) pnm_allocarray( 1, crows + 1);

    /* Allocate intermediate row sums.  HORIZONTAL uses an extra row */
    rrowsum = (long **) pm_allocarray( cols, crows + 1, sizeof(long) );
    rrowsumptr = (long **) pnm_allocarray( 1, crows + 1);
    growsum = (long **) pm_allocarray( cols, crows + 1, sizeof(long) );
    growsumptr = (long **) pnm_allocarray( 1, crows + 1);
    browsum = (long **) pm_allocarray( cols, crows + 1, sizeof(long) );
    browsumptr = (long **) pnm_allocarray( 1, crows + 1);

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* First row only */
    temprow = row % crows;
    pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
    if ( PNM_FORMAT_TYPE(format) != newformat )
	pnm_promoteformatrow(
	    xelbuf[temprow], cols, maxval, format, maxval, newformat );

    temprow = (row + 1) % crows;
    i = 0;
    for (irow = temprow; irow < crows; ++i, ++irow)
	rowptr[i] = xelbuf[irow];
    for (irow = 0; irow < temprow; ++irow, ++i)
	rowptr[i] = xelbuf[irow];

    for ( crow = 0; crow < crows; ++crow )
	{
	rrowsumptr[crow] = rrowsum[crow];
	growsumptr[crow] = growsum[crow];
	browsumptr[crow] = browsum[crow];
	}
 
    for ( col = 0; col < cols; ++col )
	{
	if ( col < ccolso2 || col >= cols - ccolso2 )
	    outputrow[col] = rowptr[crowso2][col];
	else if ( col == ccolso2 )
	    {
	    leftcol = col - ccolso2;
	    rsum = 0.0;
	    gsum = 0.0;
	    bsum = 0.0;
	    for ( crow = 0; crow < crows; ++crow )
		{
		temprptr = rowptr[crow] + leftcol;
		rrowsumptr[crow][leftcol] = 0L;
		growsumptr[crow][leftcol] = 0L;
		browsumptr[crow][leftcol] = 0L;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    rrowsumptr[crow][leftcol] += 
		        PPM_GETR( *(temprptr + ccol) );
		    growsumptr[crow][leftcol] += 
		        PPM_GETG( *(temprptr + ccol) );
		    browsumptr[crow][leftcol] += 
		        PPM_GETB( *(temprptr + ccol) );
		    }
		rsum += rrowsumptr[crow][leftcol] * rweights[crow][0];
		gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		bsum += browsumptr[crow][leftcol] * bweights[crow][0];
		}
	    temprsum = rsum + 0.5;
	    tempgsum = gsum + 0.5;
	    tempbsum = bsum + 0.5;
	    CHECK_RED;
	    CHECK_GREEN;
	    CHECK_BLUE;
	    PPM_ASSIGN( outputrow[col], r, g, b );
	    }
	else
	    {
	    rsum = 0.0;
	    gsum = 0.0;
	    bsum = 0.0;
	    leftcol = col - ccolso2;
	    subcol = col - ccolso2 - 1;
	    addcol = col + ccolso2;
	    for ( crow = 0; crow < crows; ++crow )
		{
		rrowsumptr[crow][leftcol] = rrowsumptr[crow][subcol]
		    - PPM_GETR( rowptr[crow][subcol] )
		    + PPM_GETR( rowptr[crow][addcol] );
		rsum += rrowsumptr[crow][leftcol] * rweights[crow][0];
		growsumptr[crow][leftcol] = growsumptr[crow][subcol]
		    - PPM_GETG( rowptr[crow][subcol] )
		    + PPM_GETG( rowptr[crow][addcol] );
		gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		browsumptr[crow][leftcol] = browsumptr[crow][subcol]
		    - PPM_GETB( rowptr[crow][subcol] )
		    + PPM_GETB( rowptr[crow][addcol] );
		bsum += browsumptr[crow][leftcol] * bweights[crow][0];
		}
	    temprsum = rsum + 0.5;
	    tempgsum = gsum + 0.5;
	    tempbsum = bsum + 0.5;
	    CHECK_RED;
	    CHECK_GREEN;
	    CHECK_BLUE;
	    PPM_ASSIGN( outputrow[col], r, g, b );
	    }
        }
    pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );


    /* For all subsequent rows */

    subrow = crows;
    addrow = crows - 1;
    crowsp1 = crows + 1;
    ++row;
    for ( ; row < rows; ++row )
	{
	temprow = row % crowsp1;
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	temprow = (row + 2) % crowsp1;
	i = 0;
	for (irow = temprow; irow < crowsp1; ++i, ++irow)
	    {
	    rowptr[i] = xelbuf[irow];
	    rrowsumptr[i] = rrowsum[irow];
	    growsumptr[i] = growsum[irow];
	    browsumptr[i] = browsum[irow];
	    }
	for (irow = 0; irow < temprow; ++irow, ++i)
	    {
	    rowptr[i] = xelbuf[irow];
	    rrowsumptr[i] = rrowsum[irow];
	    growsumptr[i] = growsum[irow];
	    browsumptr[i] = browsum[irow];
	    }

	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else if ( col == ccolso2 )
		{
		rsum = 0.0;
		gsum = 0.0;
		bsum = 0.0;
		leftcol = col - ccolso2;
		rrowsumptr[addrow][leftcol] = 0L;
		growsumptr[addrow][leftcol] = 0L;
		browsumptr[addrow][leftcol] = 0L;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    rrowsumptr[addrow][leftcol] += 
			PPM_GETR( rowptr[addrow][leftcol + ccol] );
		    growsumptr[addrow][leftcol] += 
			PPM_GETG( rowptr[addrow][leftcol + ccol] );
		    browsumptr[addrow][leftcol] += 
			PPM_GETB( rowptr[addrow][leftcol + ccol] );
		    }
		for ( crow = 0; crow < crows; ++crow )
		    {
		    rsum += rrowsumptr[crow][leftcol] * rweights[crow][0];
		    gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		    bsum += browsumptr[crow][leftcol] * bweights[crow][0];
		    }
		temprsum = rsum + 0.5;
		tempgsum = gsum + 0.5;
		tempbsum = bsum + 0.5;
		CHECK_RED;
		CHECK_GREEN;
		CHECK_BLUE;
		PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    else
		{
		rsum = 0.0;
		gsum = 0.0;
		bsum = 0.0;
		leftcol = col - ccolso2;
		subcol = col - ccolso2 - 1;
		addcol = col + ccolso2;  
		rrowsumptr[addrow][leftcol] = rrowsumptr[addrow][subcol]
		    - PPM_GETR( rowptr[addrow][subcol] )
		    + PPM_GETR( rowptr[addrow][addcol] );
		growsumptr[addrow][leftcol] = growsumptr[addrow][subcol]
		    - PPM_GETG( rowptr[addrow][subcol] )
		    + PPM_GETG( rowptr[addrow][addcol] );
		browsumptr[addrow][leftcol] = browsumptr[addrow][subcol]
		    - PPM_GETB( rowptr[addrow][subcol] )
		    + PPM_GETB( rowptr[addrow][addcol] );
		for ( crow = 0; crow < crows; ++crow )
		    {
		    rsum += rrowsumptr[crow][leftcol] * rweights[crow][0];
		    gsum += growsumptr[crow][leftcol] * gweights[crow][0];
		    bsum += browsumptr[crow][leftcol] * bweights[crow][0];
		    }
		temprsum = rsum + 0.5;
		tempgsum = gsum + 0.5;
		tempbsum = bsum + 0.5;
		CHECK_RED;
		CHECK_GREEN;
		CHECK_BLUE;
		PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }


/* PPM Vertical Convolution
**
** Same as pgm_vertical_convolve()
**
*/

static void
ppm_vertical_convolve()
    {
    register int ccol, col;
    xel** xelbuf;
    xel* outputrow;
    xelval r, g, b;
    int row, crow;
    xel **rowptr, *temprptr;
    int leftcol;
    int i, irow;
    int toprow, temprow;
    int subrow, addrow;
    int tempcol;
    float rsum, gsum, bsum;
    long *rcolumnsum, *gcolumnsum, *bcolumnsum;
    int crowsp1;
    int addcol;
    long temprsum, tempgsum, tempbsum;

    /* Allocate space for one convolution-matrix's worth of rows, plus
    ** a row output buffer. VERTICAL uses an extra row. */
    xelbuf = pnm_allocarray( cols, crows + 1 );
    outputrow = pnm_allocrow( cols );

    /* Allocate array of pointers to xelbuf */
    rowptr = (xel **) pnm_allocarray( 1, crows + 1 );

    /* Allocate space for intermediate column sums */
    rcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    gcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    bcolumnsum = (long *) pm_allocrow( cols, sizeof(long) );
    for ( col = 0; col < cols; ++col )
	{
	rcolumnsum[col] = 0L;
	gcolumnsum[col] = 0L;
	bcolumnsum[col] = 0L;
	}

    pnm_writepnminit( stdout, cols, rows, maxval, newformat, 0 );

    /* Read in one convolution-matrix's worth of image, less one row. */
    for ( row = 0; row < crows - 1; ++row )
	{
	pnm_readpnmrow( ifp, xelbuf[row], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[row], cols, maxval, format, maxval, newformat );
	/* Write out just the part we're not going to convolve. */
	if ( row < crowso2 )
	    pnm_writepnmrow( stdout, xelbuf[row], cols, maxval, newformat, 0 );
	}

    /* Now the rest of the image - read in the row at the end of
    ** xelbuf, and convolve and write out the row in the middle.
    */
    /* For first row only */

    toprow = row + 1;
    temprow = row % crows;
    pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
    if ( PNM_FORMAT_TYPE(format) != newformat )
	pnm_promoteformatrow(
	    xelbuf[temprow], cols, maxval, format, maxval, newformat );

    /* Arrange rowptr to eliminate the use of mod function to determine
    ** which row of xelbuf is 0...crows.  Mod function can be very costly.
    */
    temprow = toprow % crows;
    i = 0;
    for (irow = temprow; irow < crows; ++i, ++irow)
	rowptr[i] = xelbuf[irow];
    for (irow = 0; irow < temprow; ++irow, ++i)
	rowptr[i] = xelbuf[irow];

    for ( col = 0; col < cols; ++col )
	{
	if ( col < ccolso2 || col >= cols - ccolso2 )
	    outputrow[col] = rowptr[crowso2][col];
	else if ( col == ccolso2 )
	    {
	    rsum = 0.0;
	    gsum = 0.0;
	    bsum = 0.0;
	    leftcol = col - ccolso2;
	    for ( crow = 0; crow < crows; ++crow )
		{
		temprptr = rowptr[crow] + leftcol;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    rcolumnsum[leftcol + ccol] += 
			PPM_GETR( *(temprptr + ccol) );
		    gcolumnsum[leftcol + ccol] += 
			PPM_GETG( *(temprptr + ccol) );
		    bcolumnsum[leftcol + ccol] += 
			PPM_GETB( *(temprptr + ccol) );
		    }
		}
	    for ( ccol = 0; ccol < ccols; ++ccol)
		{
		rsum += rcolumnsum[leftcol + ccol] * rweights[0][ccol];
		gsum += gcolumnsum[leftcol + ccol] * gweights[0][ccol];
		bsum += bcolumnsum[leftcol + ccol] * bweights[0][ccol];
		}
	    temprsum = rsum + 0.5;
	    tempgsum = gsum + 0.5;
	    tempbsum = bsum + 0.5;
	    CHECK_RED;
	    CHECK_GREEN;
	    CHECK_BLUE;
	    PPM_ASSIGN( outputrow[col], r, g, b );
	    }
	else
	    {
	    rsum = 0.0;
	    gsum = 0.0;
	    bsum = 0.0;
	    leftcol = col - ccolso2;
	    addcol = col + ccolso2;  
	    for ( crow = 0; crow < crows; ++crow )
		{
		rcolumnsum[addcol] += PPM_GETR( rowptr[crow][addcol] );
		gcolumnsum[addcol] += PPM_GETG( rowptr[crow][addcol] );
		bcolumnsum[addcol] += PPM_GETB( rowptr[crow][addcol] );
		}
	    for ( ccol = 0; ccol < ccols; ++ccol )
		{
		rsum += rcolumnsum[leftcol + ccol] * rweights[0][ccol];
		gsum += gcolumnsum[leftcol + ccol] * gweights[0][ccol];
		bsum += bcolumnsum[leftcol + ccol] * bweights[0][ccol];
		}
	    temprsum = rsum + 0.5;
	    tempgsum = gsum + 0.5;
	    tempbsum = bsum + 0.5;
	    CHECK_RED;
	    CHECK_GREEN;
	    CHECK_BLUE;
	    PPM_ASSIGN( outputrow[col], r, g, b );
	    }
	}
    pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );

    /* For all subsequent rows */
    subrow = crows;
    addrow = crows - 1;
    crowsp1 = crows + 1;
    ++row;
    for ( ; row < rows; ++row )
	{
	toprow = row + 1;
	temprow = row % (crows +1);
	pnm_readpnmrow( ifp, xelbuf[temprow], cols, maxval, format );
	if ( PNM_FORMAT_TYPE(format) != newformat )
	    pnm_promoteformatrow(
		xelbuf[temprow], cols, maxval, format, maxval, newformat );

	/* Arrange rowptr to eliminate the use of mod function to determine
	** which row of xelbuf is 0...crows.  Mod function can be very costly.
	*/
	temprow = (toprow + 1) % crowsp1;
	i = 0;
	for (irow = temprow; irow < crowsp1; ++i, ++irow)
	    rowptr[i] = xelbuf[irow];
	for (irow = 0; irow < temprow; ++irow, ++i)
	    rowptr[i] = xelbuf[irow];

	for ( col = 0; col < cols; ++col )
	    {
	    if ( col < ccolso2 || col >= cols - ccolso2 )
		outputrow[col] = rowptr[crowso2][col];
	    else if ( col == ccolso2 )
		{
		rsum = 0.0;
		gsum = 0.0;
		bsum = 0.0;
		leftcol = col - ccolso2;
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    tempcol = leftcol + ccol;
		    rcolumnsum[tempcol] = rcolumnsum[tempcol] 
			- PPM_GETR( rowptr[subrow][ccol] )
			+ PPM_GETR( rowptr[addrow][ccol] );
		    rsum = rsum + rcolumnsum[tempcol] * rweights[0][ccol];
		    gcolumnsum[tempcol] = gcolumnsum[tempcol] 
			- PPM_GETG( rowptr[subrow][ccol] )
			+ PPM_GETG( rowptr[addrow][ccol] );
		    gsum = gsum + gcolumnsum[tempcol] * gweights[0][ccol];
		    bcolumnsum[tempcol] = bcolumnsum[tempcol] 
			- PPM_GETB( rowptr[subrow][ccol] )
			+ PPM_GETB( rowptr[addrow][ccol] );
		    bsum = bsum + bcolumnsum[tempcol] * bweights[0][ccol];
		    }
		temprsum = rsum + 0.5;
		tempgsum = gsum + 0.5;
		tempbsum = bsum + 0.5;
		CHECK_RED;
		CHECK_GREEN;
		CHECK_BLUE;
		PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    else
		{
		rsum = 0.0;
		gsum = 0.0;
		bsum = 0.0;
		leftcol = col - ccolso2;
		addcol = col + ccolso2;
		rcolumnsum[addcol] = rcolumnsum[addcol]
		    - PPM_GETR( rowptr[subrow][addcol] )
		    + PPM_GETR( rowptr[addrow][addcol] );
		gcolumnsum[addcol] = gcolumnsum[addcol]
		    - PPM_GETG( rowptr[subrow][addcol] )
		    + PPM_GETG( rowptr[addrow][addcol] );
		bcolumnsum[addcol] = bcolumnsum[addcol]
		    - PPM_GETB( rowptr[subrow][addcol] )
		    + PPM_GETB( rowptr[addrow][addcol] );
		for ( ccol = 0; ccol < ccols; ++ccol )
		    {
		    rsum += rcolumnsum[leftcol + ccol] * rweights[0][ccol];
		    gsum += gcolumnsum[leftcol + ccol] * gweights[0][ccol];
		    bsum += bcolumnsum[leftcol + ccol] * bweights[0][ccol];
		    }
		temprsum = rsum + 0.5;
		tempgsum = gsum + 0.5;
		tempbsum = bsum + 0.5;
		CHECK_RED;
		CHECK_GREEN;
		CHECK_BLUE;
		PPM_ASSIGN( outputrow[col], r, g, b );
		}
	    }
	pnm_writepnmrow( stdout, outputrow, cols, maxval, newformat, 0 );
	}

    /* Now write out the remaining unconvolved rows in xelbuf. */
    for ( irow = crowso2 + 1; irow < crows; ++irow )
	pnm_writepnmrow(
            stdout, rowptr[irow], cols, maxval, newformat, 0 );

    pm_close( stdout );
    }
