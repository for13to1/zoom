/*
 * Convert a GEM .img file to a portable bitmap file.
 *
 * Author: Diomidis D. Spinellis
 * (C) Copyright 1988 Diomidis D. Spinellis.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Comments and additions should be sent to the author:
 *
 *                     Diomidis D. Spinellis
 *                     1 Myrsinis Str.
 *                     GR-145 62 Kifissia
 *                     GREECE
 *
 * 92/07/11 Johann Haider
 * Changed to read from stdin if file is omitted
 * Changed to handle line length not a multipe of 8
 *
 * 94/01/31 Andreas Schwab (schwab@ls5.informatik.uni-dortmund.de)
 * Changed to remove architecture dependency and conform to
 * PBM coding standard.
 * Added more tests for garbage.
 */

#include <assert.h>
#include "pbm.h"

char pattern[8];

static void getinit ARGS ((FILE *file, int *colsP, int *rowsP, int *padrightP,
			   int *patlenP));

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             debug = 0;
	FILE           *f;
	int             x;
	int             i, j, k, l;
	int             c, cc, linerep;
	int		rows, cols;
	bit		*bitrow;
	int padright, patlen;
	char *usage = "[-debug] [gemfile]";
	int argn = 1;

	pbm_init( &argc, argv );

	while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0')
	  {
	    if (pm_keymatch(argv[1], "-debug", 2))
	      debug = 1;
	    else
	      pm_usage (usage);
	    ++argn;
	  }

	if (argc == argn)
		f = stdin;
	else {
		f = pm_openr (argv[argn]);
		++argn;
	}

	if (argn != argc)
	  pm_usage (usage);

	getinit (f, &cols, &rows, &padright, &patlen);

	pbm_writepbminit( stdout, cols, rows, 0 );
	bitrow = pbm_allocrow (cols + padright);

	for (i = 0; i < rows; ) {
		x = 0;
		linerep = 1;
		while (x < cols) {
			switch (c = getc(f)) {
			case 0x80:	/* Bit String */
				c = getc(f);	/* Byte count */
				if (debug)
					pm_message(
						"bit string of %d bytes",
						c );
				
				if (x + c * 8 > cols + padright)
				  pm_error ("bad byte count");
				for (j = 0; j < c; ++j) {
					cc = getc(f);
					for (k = 0x80; k; k >>= 1) {
						bitrow[x] = (k & cc) ? PBM_BLACK : PBM_WHITE;
						++x;
					}
				}
				break;
			case 0:		/* Pattern run */
				c = getc(f);	/* Repeat count */
				if (debug)
					pm_message(
						"pattern run of %d repetitions",
						c );
                                /* line repeat */
                                if (c == 0) {
                                        c = getc(f);
                                        if (c != 0x00ff)
                                                pm_error( "badly formed line repeat" );
                                        linerep = getc(f);
                                        break;
                                }
				fread (pattern, 1, patlen, f);
				if (x + c * patlen * 8 > cols + padright)
				  pm_error ("bad pattern repeat count");
				for (j = 0; j < c; ++j)
					for (l = 0; l < patlen; ++l)
						for (k = 0x80; k; k >>= 1) {
							bitrow[x] = (k & pattern[l]) ? PBM_BLACK : PBM_WHITE;
							++x;
						}
				break;

			default:	/* Solid run */
				if (debug)
					pm_message(
						"solid run of %d bytes %s",
						c & 0x7f,
						c & 0x80 ? "on" : "off" );
                                /* each byte had eight bits DSB */
                                l = (c & 0x80) ? PBM_BLACK : PBM_WHITE;
                                c = (c & 0x7f) * 8;
                                if (x + c > cols + padright)
				  pm_error ("bad solid run repeat count");
				for (j = 0; j < c; ++j) {
                                        bitrow[x] = l;
					++x;
                                }
				break;

			case EOF:	/* End of file */
				pm_error( "end of file reached" );

			}
		}
                if ( debug )
                        pm_message( "EOL row %d", i );
                if (x != cols + padright)
                        pm_error( "EOL beyond edge" );
		while (linerep--)
		  {
			pbm_writepbmrow( stdout, bitrow, cols, 0 );
			++i;
		  }
	}
	pm_close( f );
	pm_close( stdout );
	exit(0);
}

static void
getinit (file, colsP, rowsP, padrightP, patlenP)
     FILE *file;
     int *colsP;
     int *rowsP;
     int *padrightP;
     int *patlenP;
{
  short s;
  short headlen;

  if (pm_readbigshort (file, &s) == -1) /* Image file version */
    pm_error ("EOF / read error");
  if (s != 1)
    pm_error ("unknown version number (%d)", (int) s);
  if (pm_readbigshort (file, &headlen) == -1) /* Header length in words */
    pm_error ("EOF / read error");
  if (headlen < 8)
    pm_error ("short header (%d)", (int) headlen);
  if (pm_readbigshort (file, &s) == -1) /* Number of planes */
    pm_error ("EOF / read error");
  if (s != 1)
    pm_error ("color IMG not supported");
  if (pm_readbigshort (file, &s) == -1) /* Pattern definition length (bytes) */
    pm_error ("EOF / read error");
  if (s < 1 || s > 8)
    pm_error ("illegal pattern length (%d)", (int) s);
  *patlenP = (int) s;
  if (pm_readbigshort (file, &s) == -1 /* Pixel height (microns) */
      || pm_readbigshort (file, &s) == -1 /* Pixel height (microns) */
      || pm_readbigshort (file, &s) == -1) /* Scan line width */
    pm_error ("EOF / read error");
  *colsP = (int) s;
  if (pm_readbigshort (file, &s) == -1) /* Number of scan line items */
    pm_error ("EOF / read error");
  *rowsP = (int) s;
  *padrightP = 7 - ((*colsP + 7) & 7);

  headlen -= 8;
  while (headlen-- > 0)
    {
      (void) getc (file);
      (void) getc (file);
    }
}

