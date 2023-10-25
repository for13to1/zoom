/* pbmtext.c - render text into a bitmap
**
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pbm.h"
#include "pbmfont.h"

static void fix_control_chars ARGS(( char* buf ));
static void fill_rect ARGS(( bit** bits, int row0, int col0, int height, int width, bit color ));
static void copy_rect ARGS(( bit** fbits, int frow0, int fcol0, int height, int width, bit** tbits, int trow0, int tcol0 ));

int
main( argc, argv )
    int argc;
    char* argv[];
    {
    bit** bits;
    int argn, rows, cols, row, col;
    struct font* fn;
    char* fontname;
    int frows, fcols;
    FILE* ifp;
    int dump;
    int char_width, char_height, char_awidth, char_aheight, vmargin, hmargin;
    int char_row0[95];
    int char_col0[95];
    char buf[5000];
    char** lp;
    struct glyph* glyph;
    int lines, maxlines, line;
    int maxwidth, maxleftb;
    char* cp;
    char* usage = "[-font <fontfile>] [-builtin <fontname>] [text]";

    pbm_init( &argc, argv );

    /* Set up default parameters. */
    argn = 1;
    fn = 0;
    fontname = "bdf";
    dump = 0;

    /* Check for flags. */
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
	{
	if ( pm_keymatch( argv[argn], "-font", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc )
		pm_usage( usage );
	    
	    fn = pbm_loadfont( argv[argn] );
	    }
	else if ( pm_keymatch( argv[argn], "-builtin", 2 ) )
	    {
	    ++argn;
	    if ( argn == argc )
		pm_usage( usage );
	    fontname = argv[argn];
	    }
	else if ( pm_keymatch( argv[argn], "-dump", 2 ) )
	    /* Undocumented dump flag for installing a new built-in font. */
	    dump = 1;
	else
	    pm_usage( usage );
	++argn;
	}

    if (fn == 0)
	fn = pbm_defaultfont( fontname );

    if ( dump )
	{
	pbm_dumpfont( fn );
	exit( 0 );
	}
    
    maxlines = 50;
    lp = (char**) malloc( maxlines * sizeof(char*) );
    if ( lp == (char**) 0 )
	pm_error( "out of memory" );

    if ( argn < argc )
	{ /* Get text from the command line. */
	(void) strcpy( buf, argv[argn] );
	++argn;
	while ( argn < argc )
	    {
	    (void) strcat( buf, " " );
	    (void) strcat( buf, argv[argn] );
	    ++argn;
	    }
	fix_control_chars( buf );
	lp[0] = buf;
	lines = 1;
	}
    else
	{ /* Read text from stdin. */
	lines = 0;
	while ( gets( buf ) != NULL )
	    {
	    int l;

	    fix_control_chars( buf );
	    l = strlen( buf );
	    if ( lines >= maxlines )
		{
		maxlines *= 2;
		lp = (char**) realloc( (char*) lp, maxlines * sizeof(char*) );
		if ( lp == (char**) 0 )
		    pm_error( "out of memory" );
		}
	    lp[lines] = (char*) malloc( l + 1 );
	    if ( lp[lines] == 0 )
		pm_error( "out of memory" );
	    (void) strcpy( lp[lines], buf );
	    ++lines;
	    }
	}

    if ( lines == 1 )
	{
	vmargin = fn->maxheight / 2;
	hmargin = fn->maxwidth;
	}
    else
	{
	vmargin = fn->maxheight;
	hmargin = 2 * fn->maxwidth;
	}
    
    /* The total height is easy to figure out */
    rows = 2 * vmargin + lines * fn->maxheight;

    /* The total width is not so easy */
    maxwidth = 0;
    maxleftb = 0;
    for ( line = 0; line < lines; ++line ) {
	int isfirst = 1;
	int x = 0;
	int bwid = 0;
	char lastch;

	for ( cp = lp[line]; *cp != '\0'; ++cp ) {
	    if (!fn->glyph[*cp])
		continue;
	    if (isfirst) {
		isfirst = 0;
		if (fn->glyph[*cp]->x < 0)
			x = -fn->glyph[*cp]->x;
		    else
			bwid += fn->glyph[*cp]->x;

		    bwid += x;
	    }
	    bwid += fn->glyph[*cp]->xadd;
	    lastch = *cp;
	}
	if (!isfirst) {
	    bwid -= fn->glyph[lastch]->xadd;
	    bwid += fn->glyph[lastch]->width + fn->glyph[lastch]->x;
	}

	if (bwid > maxwidth)
	    maxwidth = bwid;
	if (x > maxleftb)
	    maxleftb = x;
    }

    cols = 2 * hmargin + maxwidth + maxleftb;
    bits = pbm_allocarray( cols, rows );

    /* Fill background with white */
    fill_rect( bits, 0, 0, rows, cols, PBM_WHITE );
    
    /* Render characters. */
    for ( line = 0; line < lines; ++line ) {
	row = vmargin + line * fn->maxheight;
	col = hmargin + maxleftb;

	for ( cp = lp[line]; *cp != '\0'; ++cp )
	    {
	    int h, w, y;

	    if (!(glyph = fn->glyph[*cp]))
		continue;

	    y = row + fn->maxheight + fn->y - glyph->height - glyph->y;

	    for (h = 0; h < glyph->height; h++) {
		for (w = 0; w < glyph->width; w++) {
		    if (glyph->bmap[h * glyph->width + w])
			bits[y][w + col + glyph->x] = PBM_BLACK;
		}
		y++;
	    }
	    col += glyph->xadd;
	}
    }

    /* All done. */
    pbm_writepbm( stdout, bits, cols, rows, 0 );
    pm_close( stdout );

    exit( 0 );
    }

static void
fix_control_chars( buf )
    char* buf;
    {
    int i, j, n, l;

    for ( i = 0; buf[i] != '\0'; ++i )
	{
	if ( buf[i] == '\t' )
	    { /* Turn tabs into the right number of spaces. */
	    n = ( i + 8 ) / 8 * 8;
	    l = strlen( buf );
	    for ( j = l; j > i; --j )
		buf[j + n - i - 1] = buf[j];
	    for ( ; i < n; ++i )
		buf[i] = ' ';
	    --i;
	    }
	else if ( buf[i] < ' ' || buf[i] > '~' )
	    /* Turn other control chars into a single space. */
	    buf[i] = ' ';
	}
    }

#if __STDC__
static void
fill_rect( bit** bits, int row0, int col0, int height, int width, bit color )
#else /*__STDC__*/
static void
fill_rect( bits, row0, col0, height, width, color )
    bit** bits;
    int row0, col0, height, width;
    bit color;
#endif /*__STDC__*/
    {
    int row, col;

    for ( row = row0; row < row0 + height; ++row )
	for ( col = col0; col < col0 + width; ++col )
	    bits[row][col] = color;
    }

static void
copy_rect( fbits, frow0, fcol0, height, width, tbits, trow0, tcol0 )
    bit** fbits;
    int frow0, fcol0, height, width;
    bit** tbits;
    int trow0, tcol0;
    {
    int row, col;

    for ( row = 0; row < height; ++row )
	for ( col = 0; col < width; ++col )
	    tbits[trow0 + row][tcol0 + col] = fbits[frow0 + row][fcol0 + col];
    }
