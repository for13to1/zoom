/* pbmtoepsi.c
**
**    by Doug Crabill, based heavily on pbmtoascii
**
**    Converts a pbm file to an encapsulated PostScript style bitmap.
**    Note that it does NOT covert the pbm file to PostScript, only to
**    a bitmap to be added to a piece of PostScript generated elsewhere.
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"

#if !defined(MAXINT)
#define MAXINT (0x7fffffff)
#endif

int
main( argc, argv )
	int argc;
	char *argv[];
{
	FILE *ifd;
	register bit **bits;
	int argn, rows, cols, row, col, tot, count;
	int top = MAXINT, bottom = -MAXINT, left = MAXINT, right = -MAXINT;
	int bbonly = 0;
	char *usage = "[-bbonly] [pbmfile]";
	

	pbm_init( &argc, argv );
	
	argn = 1; 

	if ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
		if ( pm_keymatch( argv[argn], "-bbonly", 2 ) ) {
			bbonly = 1;
		} else {
			pm_usage( usage );
		}
		++argn;
	}

	if ( argn != argc ) {
		ifd = pm_openr( argv[argn] );
		++argn;
	} else {
		ifd = stdin;
	}
    
	if ( argn != argc )
		pm_usage( usage );

	bits = pbm_readpbm( ifd, &cols, &rows );
	
	pm_close( ifd );
	
	for (row = 0; row < rows; row++) {
		for (col = 0; col < cols; col++) {
			if (bits[row][col] == PBM_BLACK) {
				if (row < top) {
					top = row;
				}
				if (row > bottom) {
					bottom = row;
				}
				if (col < left) {
					left = col;
				}
				if (col > right) {
					right = col;
				}
			}
		}
	}

	printf("%%!PS-Adobe-2.0 EPSF-1.2\n");
 	printf("%%%%BoundingBox: %d %d %d %d\n", left, rows - bottom, right, rows - top);

	if (bbonly) {
		exit(0);
	}

	printf("%%%%BeginPreview: %d %d 1 %d\n", right - left + 1, bottom - top + 1, bottom - top + 1);

	for (row = top; row <= bottom; row++) {
		printf("%% ");
		count = 0;
		for (col = left; col <= right; col += 4) {
			tot = 0;
			if (bits[row][col] == PBM_BLACK) {
				tot += 8;
			}
			if (bits[row][col+1] == PBM_BLACK) {
				tot += 4;
			}
			if (bits[row][col+2] == PBM_BLACK) {
				tot += 2;
			}
			if (bits[row][col+3] == PBM_BLACK) {
				tot++;
			}
			printf("%x", tot);
			count++;
		}
		printf((count % 2) == 0 ? "\n" : "0\n");
	}
	printf("%%%%EndImage\n");
	printf("%%%%EndPreview\n");

	exit( 0 );
}
