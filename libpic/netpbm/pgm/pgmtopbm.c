/* pgmtopbm.c - read a portable graymap and write a portable bitmap
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

#include "pgm.h"
#include "dithers.h"

static void init_hilbert ARGS((int w, int h));
static int hilbert ARGS((int *px, int *py));


int
main( argc, argv )
    int argc;
    char* argv[];
{
    FILE* ifp;
    gray* grayrow;
    register gray* gP;
    bit* bitrow;
    register bit* bP;
    int argn, rows, cols, format, row, col, limitcol;
    float fthreshval;
    int clump_size;
    gray maxval;
    char* usage = "[-floyd|-fs  | -hilbert | -threshold | -dither8|-d8 |\n     -cluster3|-c3|-cluster4|-c4|-cluster8|-c8] [-value <val>] [-clump <size>] [pgmfile]";
    int halftone;
#define QT_FS 1
#define QT_THRESH 2
#define QT_DITHER8 3
#define QT_CLUSTER3 4
#define QT_CLUSTER4 5
#define QT_CLUSTER8 6
#define QT_HILBERT 7
    long threshval, sum;
    long* thiserr;
    long* nexterr;
    long* temperr;
#define FS_SCALE 1024
#define HALF_FS_SCALE 512
    int fs_direction;

    pgm_init( &argc, argv );

    argn = 1;
    halftone = QT_FS;	/* default quantization is Floyd-Steinberg */
    fthreshval = 0.5;
    clump_size = 5;	/* default hilbert curve clump size */

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
    {
	if ( pm_keymatch( argv[argn], "-fs", 2 ) ||
	    pm_keymatch( argv[argn], "-floyd", 2 ) )
	    halftone = QT_FS;
	else if ( pm_keymatch( argv[argn], "-hilbert", 2 ))
	    halftone = QT_HILBERT;
	else if ( pm_keymatch( argv[argn], "-threshold", 2 ) )
	    halftone = QT_THRESH;
	else if ( pm_keymatch( argv[argn], "-dither8", 2 ) ||
		  pm_keymatch( argv[argn], "-d8", 3 ) )
	    halftone = QT_DITHER8;
	else if ( pm_keymatch( argv[argn], "-cluster3", 9 ) ||
		  pm_keymatch( argv[argn], "-c3", 3 ) )
	    halftone = QT_CLUSTER3;
	else if ( pm_keymatch( argv[argn], "-cluster4", 9 ) ||
	          pm_keymatch( argv[argn], "-c4", 3 ) )
	    halftone = QT_CLUSTER4;
	else if ( pm_keymatch( argv[argn], "-cluster8", 9 ) ||
	          pm_keymatch( argv[argn], "-c8", 3 ) )
	    halftone = QT_CLUSTER8;
	else if ( pm_keymatch( argv[argn], "-value", 2 ) )
	{
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%f", &fthreshval ) != 1 ||
		fthreshval < 0.0 || fthreshval > 1.0 )
		pm_usage( usage );
	}
	else if ( pm_keymatch( argv[argn], "-clump", 2 ) )
	{
	    ++argn;
	    if ( argn == argc || sscanf( argv[argn], "%d", &clump_size ) != 1 ||
		clump_size < 2)
		pm_usage( usage );
	}
	else
	    pm_usage( usage );
	++argn;
    }

    if ( argn != argc )
    {
	ifp = pm_openr( argv[argn] );
	++argn;
    }
    else
	ifp = stdin;

    if ( argn != argc )
	pm_usage( usage );

    if ( halftone != QT_HILBERT )
    {
	pgm_readpgminit( ifp, &cols, &rows, &maxval, &format );
	grayrow = pgm_allocrow( cols );

	pbm_writepbminit( stdout, cols, rows, 0 );
	bitrow = pbm_allocrow( cols );

	/* Initialize. */
	switch ( halftone )
	{
	case QT_FS:
	    /* Initialize Floyd-Steinberg error vectors. */
	    thiserr = (long*) pm_allocrow( cols + 2, sizeof(long) );
	    nexterr = (long*) pm_allocrow( cols + 2, sizeof(long) );
	    srandom( (int) ( time( 0 ) ^ getpid( ) ) );
	    for ( col = 0; col < cols + 2; ++col )
		thiserr[col] = ( random( ) % FS_SCALE - HALF_FS_SCALE ) / 4;
	    /* (random errors in [-FS_SCALE/8 .. FS_SCALE/8]) */
	    fs_direction = 1;
	    threshval = fthreshval * FS_SCALE;
	    break;

	case QT_HILBERT:
	    break;

	case QT_THRESH:
	    threshval = fthreshval * maxval + 0.999999;
	    break;

	case QT_DITHER8:
	    /* Scale dither matrix. */
	    for ( row = 0; row < 16; ++row )
		for ( col = 0; col < 16; ++col )
		    dither8[row][col] = dither8[row][col] * ( maxval + 1 ) / 256;
	    break;

	case QT_CLUSTER3:
	    /* Scale order-3 clustered dither matrix. */
	    for ( row = 0; row < 6; ++row )
		for ( col = 0; col < 6; ++col )
		    cluster3[row][col] = cluster3[row][col] * ( maxval + 1 ) / 18;
	    break;

	case QT_CLUSTER4:
	    /* Scale order-4 clustered dither matrix. */
	    for ( row = 0; row < 8; ++row )
		for ( col = 0; col < 8; ++col )
		    cluster4[row][col] = cluster4[row][col] * ( maxval + 1 ) / 32;
	    break;

	case QT_CLUSTER8:
	    /* Scale order-8 clustered dither matrix. */
	    for ( row = 0; row < 16; ++row )
		for ( col = 0; col < 16; ++col )
		    cluster8[row][col] = cluster8[row][col] * ( maxval + 1 ) / 128;
	    break;

	default:
	    pm_error( "can't happen" );
	    exit( 1 );
	}

	for ( row = 0; row < rows; ++row )
	{
	    pgm_readpgmrow( ifp, grayrow, cols, maxval, format );
    
	    switch ( halftone )
	    {
	    case QT_FS:
		for ( col = 0; col < cols + 2; ++col )
		    nexterr[col] = 0;
		if ( fs_direction )
		{
		    col = 0;
		    limitcol = cols;
		    gP = grayrow;
		    bP = bitrow;
		}
		else
		{
		    col = cols - 1;
		    limitcol = -1;
		    gP = &(grayrow[col]);
		    bP = &(bitrow[col]);
		}
		do
		{
		    sum = ( (long) *gP * FS_SCALE ) / maxval + thiserr[col + 1];
		    if ( sum >= threshval )
		    {
			*bP = PBM_WHITE;
			sum = sum - threshval - HALF_FS_SCALE;
		    }
		    else
			*bP = PBM_BLACK;
    
		    if ( fs_direction )
		    {
			thiserr[col + 2] += ( sum * 7 ) / 16;
			nexterr[col    ] += ( sum * 3 ) / 16;
			nexterr[col + 1] += ( sum * 5 ) / 16;
			nexterr[col + 2] += ( sum     ) / 16;
    
			++col;
			++gP;
			++bP;
		    }
		    else
		    {
			thiserr[col    ] += ( sum * 7 ) / 16;
			nexterr[col + 2] += ( sum * 3 ) / 16;
			nexterr[col + 1] += ( sum * 5 ) / 16;
			nexterr[col    ] += ( sum     ) / 16;
    
			--col;
			--gP;
			--bP;
		    }
		}
		while ( col != limitcol );
		temperr = thiserr;
		thiserr = nexterr;
		nexterr = temperr;
		fs_direction = ! fs_direction;
		break;
    
	    case QT_THRESH:
		for ( col = 0, gP = grayrow, bP = bitrow; col < cols; ++col, ++gP, ++bP )
		    if ( *gP >= threshval )
			*bP = PBM_WHITE;
		    else
			*bP = PBM_BLACK;
		break;
    
	    case QT_DITHER8:
		for ( col = 0, gP = grayrow, bP = bitrow; col < cols; ++col, ++gP, ++bP )
		    if ( *gP >= dither8[row % 16][col % 16] )
			*bP = PBM_WHITE;
		    else
			*bP = PBM_BLACK;
		break;
    
	    case QT_CLUSTER3:
		for ( col = 0, gP = grayrow, bP = bitrow; col < cols; ++col, ++gP, ++bP )
		    if ( *gP >= cluster3[row % 6][col % 6] )
			*bP = PBM_WHITE;
		    else
			*bP = PBM_BLACK;
		break;
    
	    case QT_CLUSTER4:
		for ( col = 0, gP = grayrow, bP = bitrow; col < cols; ++col, ++gP, ++bP )
		    if ( *gP >= cluster4[row % 8][col % 8] )
			*bP = PBM_WHITE;
		    else
			*bP = PBM_BLACK;
		break;
    
	    case QT_CLUSTER8:
		for ( col = 0, gP = grayrow, bP = bitrow; col < cols; ++col, ++gP, ++bP )
		    if ( *gP >= cluster8[row % 16][col % 16] )
			*bP = PBM_WHITE;
		    else
			*bP = PBM_BLACK;
		break;
    
	    default:
		pm_error( "can't happen" );
		exit( 1 );
	    }
    
	    pbm_writepbmrow( stdout, bitrow, cols, 0 );
	}
    }
    else	/* else use hilbert space filling curve dithering */
	/*
	 * This is taken from the article "Digital Halftoning with
	 * Space Filling Curves" by Luiz Velho, proceedings of
	 * SIGRAPH '91, page 81.
	 *
	 * This is not a terribly efficient or quick version of
	 * this algorithm, but it seems to work. - Graeme Gill.
	 * graeme@labtam.labtam.OZ.AU
	 *
	 */
    {
    	gray **grays;
    	bit **bits;
	int xx,yy;
	int end;
	int *x,*y;
	int sum = 0;

	grays = pgm_readpgm( ifp, &cols,&rows, &maxval );
	bits = pbm_allocarray(cols,rows);

	x = (int *) malloc( sizeof(int) * clump_size);
	y = (int *) malloc( sizeof(int) * clump_size);
	if (!x || !y)
	    pm_error( "Run out of memory" );
	init_hilbert(cols,rows);

	end = clump_size;
	while (end == clump_size)
	{
	    int i;
	    /* compute the next clust co-ordinates along hilbert path */
	    for (i = 0; i < end; i++)
	    {
		if (hilbert(&x[i],&y[i])==0)
		    end = i;	/* we reached the end */
	    }
	    /* sum levels */
	    for (i = 0; i < end; i++)
		sum += grays[y[i]][x[i]];
	    /* dither half and half along path */
	    for (i = 0; i < end; i++)
	    {
		if (sum >= maxval)
		{
		    bits[y[i]][x[i]] = PBM_WHITE;
		    sum -= maxval;
		}
		else
		    bits[y[i]][x[i]] = PBM_BLACK;
	    }
	}
	pbm_writepbm( stdout, bits, cols, rows, 0 );
    }

    pm_close( ifp );

    exit( 0 );
}


/* Hilbert curve tracer */

#define MAXORD 18

static int hil_order,hil_ord;
static int hil_turn;
static int hil_dx,hil_dy;
static int hil_x,hil_y;
static int hil_stage[MAXORD];
static int hil_width,hil_height;

/* Initialise the Hilbert curve tracer */
static void init_hilbert(w,h)
    int w,h;
{
    int big,ber;
    hil_width = w;
    hil_height = h;
    big = w > h ? w : h;
    for(ber = 2, hil_order = 1; ber < big; ber <<= 1, hil_order++);
    if (hil_order > MAXORD)
	pm_error( "Sorry, hilbert order is too large" );
    hil_ord = hil_order;
    hil_order--;
}

/* Return non-zero if got another point */
static int hilbert(px,py)
    int *px,*py;
{
    int temp;
    if (hil_ord > hil_order)    /* have to do first point */
    {
	hil_ord--;
	hil_stage[hil_ord] = 0;
	hil_turn = -1;
	hil_dy = 1;
	hil_dx = hil_x = hil_y = 0;
	*px = *py = 0;
	return 1;
    }
    for(;;) /* Operate the state machine */
    {
	switch (hil_stage[hil_ord])
	{
	case 0:
	    hil_turn = -hil_turn;
	    temp = hil_dy;
	    hil_dy = -hil_turn * hil_dx;
	    hil_dx = hil_turn * temp;
	    if (hil_ord > 0) 
	    {
		hil_stage[hil_ord] = 1;
		hil_ord--;
		hil_stage[hil_ord]=0;
		continue;
	    }
	case 1:
	    hil_x += hil_dx;
	    hil_y += hil_dy;
	    if (hil_x < hil_width && hil_y < hil_height)
	    {
		hil_stage[hil_ord] = 2;
		*px = hil_x;
		*py = hil_y;
		return 1;
	    }
	case 2:
	    hil_turn = -hil_turn;
	    temp = hil_dy;
	    hil_dy = -hil_turn * hil_dx;
	    hil_dx = hil_turn * temp;
	    if (hil_ord > 0) /* recurse */
	    {
		hil_stage[hil_ord] = 3;
		hil_ord--;
		hil_stage[hil_ord]=0;
		continue;
	    }
	case 3:
	    hil_x += hil_dx;
	    hil_y += hil_dy;
	    if (hil_x < hil_width && hil_y < hil_height)
	    {
		hil_stage[hil_ord] = 4;
		*px = hil_x;
		*py = hil_y;
		return 1;
	    }
	case 4:
	    if (hil_ord > 0) /* recurse */
	    {
		hil_stage[hil_ord] = 5;
		hil_ord--;
		hil_stage[hil_ord]=0;
		continue;
	    }
	case 5:
	    temp = hil_dy;
	    hil_dy = -hil_turn * hil_dx;
	    hil_dx = hil_turn * temp;
	    hil_turn = -hil_turn;
	    hil_x += hil_dx;
	    hil_y += hil_dy;
	    if (hil_x < hil_width && hil_y < hil_height)
	    {
		hil_stage[hil_ord] = 6;
		*px = hil_x;
		*py = hil_y;
		return 1;
	    }
	case 6:
	    if (hil_ord > 0) /* recurse */
	    {
		hil_stage[hil_ord] = 7;
		hil_ord--;
		hil_stage[hil_ord]=0;
		continue;
	    }
	case 7:
	    temp = hil_dy;
	    hil_dy = -hil_turn * hil_dx;
	    hil_dx = hil_turn * temp;
	    hil_turn = -hil_turn;
	    /* Return from a recursion */
	    if (hil_ord < hil_order)
		hil_ord++;
	    else
		return 0;
	}
    }
}
