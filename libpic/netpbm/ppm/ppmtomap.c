/* ppmtomap.c - read a portable pixmap and produce a minimal size portable
** pixmap containing all colors.
**
** Based on ppmtogif.c
**
** By Marcel Wijkstra < wijkstra@fwi.uva.nl>
**
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** The Graphics Interchange Format(c) is the Copyright property of
** CompuServe Incorporated.  GIF(sm) is a Service Mark property of
** CompuServe Incorporated.
*/

#include <math.h>
#include "ppm.h"
#include "ppmcmap.h"

#define MAXCOLORS 65536 /* May be smaller, may be larger (?) */

static int Red[MAXCOLORS],Green[MAXCOLORS],Blue[MAXCOLORS];

int
main( argc, argv )
    int argc;
    char* argv[];
    {
    FILE* ifp;
    pixel **pixels;
    pixel *pixelrow;
    int colors;
    int argn, rows, cols, i,j,k,l, BitsPerPixel;
    int sort, square;
    char *mapfile;
    pixval maxval;
    colorhist_vector chv;
    char* usage = "[-sort] [-square] [ppmfile]";


    ppm_init( &argc, argv );

    argn = 1;
    sort = 0;
    square = 0;

    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
        {
        if ( pm_keymatch( argv[argn], "-sort", 3 ) )
            sort = 1;
        else if ( pm_keymatch( argv[argn], "-square", 3 ) )
            square = 1;
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

    pixels = ppm_readppm( ifp, &cols, &rows, &maxval );

    pm_close( ifp );

    /* Figure out the colormap. */
    chv = ppm_computecolorhist( pixels, cols, rows, MAXCOLORS, &colors );

    if ( chv == (colorhist_vector) 0 )
	pm_error(
	    "too many colors - try doing a 'ppmquant %d'", MAXCOLORS );
    pm_message( "%d colors found", colors );

    for ( i = 0; i < colors; ++i )
	    {
            Red[i] = PPM_GETR( chv[i].color );
            Green[i] = PPM_GETG( chv[i].color );
            Blue[i] = PPM_GETB( chv[i].color );
	    }

    /* Sort the colormap */
    if (sort) {
      pm_message("sorting colormap");
      for (i=0;i<colors;i++)
        for (j=i+1;j<colors;j++)
	  l=maxval+1;
          if (((Red[i]*l)+Green[i])*l+Blue[i] >
              ((Red[j]*l)+Green[j])*l+Blue[j]) {
            k=Red[i]; Red[i]=Red[j]; Red[j]=k;
            k=Green[i]; Green[i]=Green[j]; Green[j]=k;
            k=Blue[i]; Blue[i]=Blue[j]; Blue[j]=k; } }

#if 0
    ppm_init(&argc, argv);
#endif

    if (!square) {

      pixelrow = ((pixel*) pm_allocrow( colors, sizeof(pixel) ));

      for (i=0;i<colors;i++)
        PPM_ASSIGN(pixelrow[i],
		(pixval)Red[i], (pixval)Green[i], (pixval)Blue[i]);

      ppm_writeppminit(stdout, colors, 1, maxval, 0);
      ppm_writeppmrow(stdout, pixelrow, colors, maxval, 0); }

    else {

      cols = (int)sqrt((float)colors);
      rows = colors/cols;
      ppm_writeppminit(stdout, cols, rows, maxval, 0);
      pixelrow = ((pixel*) pm_allocrow(cols, sizeof(pixel) ));

      for (i=0;i<rows;i++) {
        for (j=0;j<cols;j++) {
	  k=i*cols+j;
          PPM_ASSIGN(pixelrow[j],
		(pixval)Red[k], (pixval)Green[k], (pixval)Blue[k]); }
        ppm_writeppmrow(stdout, pixelrow, cols, maxval, 0); }

      if (colors%cols) {
	k=i*cols;
        for (j=0;j<colors%cols;j++,k++)
          PPM_ASSIGN(pixelrow[j],
		(pixval)Red[k], (pixval)Green[k], (pixval)Blue[k]);
        for (;j<cols;j++)
          PPM_ASSIGN(pixelrow[j],
		(pixval)Red[k], (pixval)Green[k], (pixval)Blue[k]);
        ppm_writeppmrow(stdout, pixelrow, cols, maxval, 0); } }
      
    pm_close(stdout);
}
