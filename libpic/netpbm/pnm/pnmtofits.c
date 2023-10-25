/* pnmtofits.c - read a portable anymap and produce a FITS file
**
** Copyright (C) 1989 by Wilson H. Bent (whb@hoh-2.att.com).
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** Modified by Alberto Accomazzi (alberto@cfa.harvard.edu), Dec 1, 1992.
**
** Added support for PPM files, the program is renamed pnmtofits.
** This program produces files with NAXIS = 2 if input file is in PBM
** or PGM format, and NAXIS = 3, NAXIS3 = 3 if input file is a PPM file.
** Data is written out as either 8 bits/pixel or 16 bits/pixel integers,
** depending on the value of maxval in the input file.
** Flags -max, -min can be used to set DATAMAX, DATAMIN, BSCALE and BZERO
** in the FITS header, but do not cause the data to be rescaled.
*/

#include "pnm.h"
#define write_card(s)    fwrite( s, sizeof(char), 80, stdout )

int
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    xel** xels;
    xel* xelrow;
    register xel* xP;
    int argn, row, col, rows, cols, planes, format, i, npad,
        bitpix, forcemin, forcemax;
    double datamin, datamax, bscale, fits_bzero, frmax, frmin;
    xelval maxval;
    register unsigned short color;
    char card[81];
    char* usage = "[-max f] [-min f] [pnmfile]";

    pnm_init( &argc, argv );

    forcemin = 0;
    forcemax = 0;
    argn = 1;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
        if ( pm_keymatch( argv[argn], "-max", 3 ) )
	  {
	  ++argn;
	  forcemax = 1;
	  if ( argn == argc || sscanf( argv[argn], "%lf", &frmax ) != 1 )
	    pm_usage( usage );
	  }
        else if ( pm_keymatch( argv[argn], "-min", 3 ) )
	  {
	  ++argn;
	  forcemin = 1;
	  if ( argn == argc || sscanf( argv[argn], "%lf", &frmin ) != 1 )
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

    xels = pnm_readpnm( ifp, &cols, &rows, &maxval, &format );

    datamin = 0.0;
    datamax = (double) maxval;
    if (forcemin) datamin = frmin;
    if (forcemax) datamax = frmax;
    fits_bzero = datamin;
    bscale = ( datamax - datamin ) / ( double ) maxval;

    bitpix = 8;
    if (maxval > 255) bitpix = 16;

    pm_close( ifp );

    /* Figure out the proper depth */
    switch ( PNM_FORMAT_TYPE(format) )
	{
	case PPM_TYPE:
	planes = 3;
	break;

	default:
	planes = 1;
	break;
	}

    /* write out fits header */
    i = 0;
    sprintf( card, "SIMPLE  =                    T                                                  " );
    write_card( card ); ++i;
    sprintf( card, "BITPIX  =           %10d                                                  ", bitpix );
    write_card( card ); ++i;
    sprintf( card, "NAXIS   =           %10d                                                  ", ( planes == 3 ) ? 3 : 2 );
    write_card( card ); ++i;
    sprintf( card, "NAXIS1  =           %10d                                                  ", cols );
    write_card( card ); ++i;
    sprintf( card, "NAXIS2  =           %10d                                                  ", rows );
    write_card( card ); ++i;
    if ( planes == 3 )
	{
    	sprintf( card, "NAXIS3  =                    3                                                  " );
    	write_card( card ); ++i;
	}
    sprintf( card, "BSCALE  =         %lE                                                   ", bscale );
    write_card( card ); ++i;
    sprintf( card, "BZERO   =         %lE                                                   ", fits_bzero );
    write_card( card ); ++i;
    sprintf( card, "DATAMAX =         %lE                                                   ", datamax );
    write_card( card ); ++i;
    sprintf( card, "DATAMIN =         %lE                                                   ", datamin );
    write_card( card ); ++i;
    sprintf( card, "HISTORY Created by pnmtofits.                                                   " );
    write_card( card ); ++i;
    sprintf( card, "END                                                                             " );
    write_card( card ); ++i;

    /* pad end of header with blanks */
    npad = ( i * 80 ) % 2880;
    if ( npad == 0 )
	npad = 2880;
    while ( npad++ < 2880 )
	putchar ( ' ' );
    
    for ( i = 0; i < planes; i++ )
    	for ( row = 0; row < rows; ++row )
	    {
	    xelrow = xels[row];
	    for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
		{
		if ( planes == 3 )
		    {
		    /* we are reading a ppm file */
		    if ( i == 0 ) 	color = PPM_GETR( *xP );
		    else if ( i == 1 ) 	color = PPM_GETG( *xP );
		    else 		color = PPM_GETB( *xP );
		    }
		else
		    color = PNM_GET1( *xP );
		
		if ( bitpix == 16 )
		    putchar( ( color >> 8 ) & 0xff );

		putchar( color & 0xff );
		}
	    }
	
    /* pad end of file with nulls */
    npad = ( rows * cols * planes * bitpix / 8 ) % 2880;
    if ( npad == 0 )
	npad = 2880;
    while ( npad++ < 2880 )
	putchar ( 0 );

    exit( 0 );
    }

