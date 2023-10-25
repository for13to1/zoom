/*\
 * $Id: bmptoppm.c,v 1.1.1.1 2001/01/05 06:15:51 legalize Exp $
 * 
 * bmptoppm.c - Converts from a Microsoft Windows or OS/2 .BMP file to a
 * PPM file.
 * 
 * The current implementation is probably not complete, but it works for
 * all the BMP files I have.  I welcome feedback.
 * 
 * Copyright (C) 1992 by David W. Sanderson.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  This software is provided "as is"
 * without express or implied warranty.
 * 
 * $Log: bmptoppm.c,v $
 * Revision 1.1.1.1  2001/01/05  06:15:51  legalize
 * Initial checkin
 *
 * Revision 1.10  1992/11/24  19:38:17  dws
 * Added code to verify that reading occurred at the correct offsets.
 * Added copyright.
 *
 * Revision 1.9  1992/11/17  02:15:24  dws
 * Changed to include bmp.h.
 * Eliminated need for fseek(), and therefore the need for a
 * temporary file.
 *
 * Revision 1.8  1992/11/13  23:48:57  dws
 * Made definition of Seekable() static, to match its prototype.
 *
 * Revision 1.7  1992/11/11  00:17:50  dws
 * Generalized to use bitio routines.
 *
 * Revision 1.6  1992/11/10  23:51:44  dws
 * Enhanced command-line handling.
 *
 * Revision 1.5  1992/11/08  00:38:46  dws
 * Changed some names to help w/ addition of ppmtobmp.
 *
 * Revision 1.4  1992/10/27  06:28:28  dws
 * Corrected stupid typo.
 *
 * Revision 1.3  1992/10/27  06:17:10  dws
 * Removed a magic constant value.
 *
 * Revision 1.2  1992/10/27  06:09:58  dws
 * Made stdin seekable.
 *
 * Revision 1.1  1992/10/27  05:31:41  dws
 * Initial revision
\*/

#include	"bmp.h"
#include	"ppm.h"
#include	"bitio.h"

#define	MAXCOLORS   	256

static char    *ifname;

/*
 * Utilities
 */

static int GetByte ARGS((FILE * fp));
static short GetShort ARGS((FILE * fp));
static long GetLong ARGS((FILE * fp));
static void readto ARGS((FILE *fp, unsigned long *ppos, unsigned long dst));
static void BMPreadfileheader ARGS((FILE *fp, unsigned long *ppos,
    unsigned long *poffBits));
static void BMPreadinfoheader ARGS((FILE *fp, unsigned long *ppos,
    unsigned long *pcx, unsigned long *pcy, unsigned short *pcBitCount,
    int *pclass));
static int BMPreadrgbtable ARGS((FILE *fp, unsigned long *ppos,
    unsigned short cBitCount, int class, pixval *R, pixval *G, pixval *B));
static int BMPreadrow ARGS((FILE *fp, unsigned long *ppos, pixel *row,
    unsigned long cx, unsigned short cBitCount, pixval *R, pixval *G, pixval *B));
static pixel ** BMPreadbits ARGS((FILE *fp, unsigned long *ppos,
    unsigned long offBits, unsigned long cx, unsigned long cy,
    unsigned short cBitCount, int class, pixval *R, pixval *G, pixval *B));

static char     er_read[] = "%s: read error";
static char     er_seek[] = "%s: seek error";

static int
GetByte(fp)
	FILE           *fp;
{
	int             v;

	if ((v = getc(fp)) == EOF)
	{
		pm_error(er_read, ifname);
	}

	return v;
}

static short
GetShort(fp)
	FILE           *fp;
{
	short           v;

	if (pm_readlittleshort(fp, &v) == -1)
	{
		pm_error(er_read, ifname);
	}

	return v;
}

static long
GetLong(fp)
	FILE           *fp;
{
	long            v;

	if (pm_readlittlelong(fp, &v) == -1)
	{
		pm_error(er_read, ifname);
	}

	return v;
}

/*
 * readto - read as many bytes as necessary to position the
 * file at the desired offset.
 */

static void
readto(fp, ppos, dst)
	FILE           *fp;
	unsigned long  *ppos;	/* pointer to number of bytes read from fp */
	unsigned long   dst;
{
	unsigned long   pos;

	if(!fp || !ppos)
		return;

	pos = *ppos;

	if(pos > dst)
		pm_error("%s: internal error in readto()", ifname);

	for(; pos < dst; pos++)
	{
		if (getc(fp) == EOF)
		{
			pm_error(er_read, ifname);
		}
	}

	*ppos = pos;
}

#if 0
static void
Seek(fp, off)
	FILE           *fp;
	long            off;
{
	if (fseek(fp, off, 0) == -1)
	{
		pm_error(er_seek, ifname);
	}
}

/*
 * Seekable(f) - makes sure the given FILE* is seekable (for
 * reading). returns f if it is, and a new, seekable FILE* if f is
 * stdin.
 */

static FILE    *
Seekable(f)
	FILE           *f;
{
	int             c;
	FILE           *t;

	if (f != stdin)
	{
		return f;
	}

	t = tmpfile();

	while ((c = getc(f)) != EOF)
	{
		putc(c, t);
	}

	rewind(t);

	return t;
}
#endif


/*
 * BMP reading routines
 */

static void
BMPreadfileheader(fp, ppos, poffBits)
	FILE           *fp;
	unsigned long  *ppos;	/* number of bytes read from fp */
	unsigned long  *poffBits;
{
	unsigned long   cbSize;
	unsigned short  xHotSpot;
	unsigned short  yHotSpot;
	unsigned long   offBits;

	if (GetByte(fp) != 'B')
	{
		pm_error("%s is not a BMP file", ifname);
	}
	if (GetByte(fp) != 'M')
	{
		pm_error("%s is not a BMP file", ifname);
	}

	cbSize = GetLong(fp);
	xHotSpot = GetShort(fp);
	yHotSpot = GetShort(fp);
	offBits = GetLong(fp);

	*poffBits = offBits;

	*ppos += 14;
}

static void
BMPreadinfoheader(fp, ppos, pcx, pcy, pcBitCount, pclass)
	FILE           *fp;
	unsigned long  *ppos;	/* number of bytes read from fp */
	unsigned long  *pcx;
	unsigned long  *pcy;
	unsigned short *pcBitCount;
	int            *pclass;
{
	unsigned long   cbFix;
	unsigned short  cPlanes;

	unsigned long   cx;
	unsigned long   cy;
	unsigned short  cBitCount;
	int             class;

	cbFix = GetLong(fp);

	switch (cbFix)
	{
	case 12:
		class = C_OS2;

		cx = GetShort(fp);
		cy = GetShort(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);

		break;
	case 40:
		class = C_WIN;

		cx = GetLong(fp);
		cy = GetLong(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);

		/*
		 * We've read 16 bytes so far, need to read 24 more
		 * for the required total of 40.
		 */

		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);

		break;
	default:
		pm_error("%s: unknown cbFix: %d", ifname, cbFix);
		break;
	}

	if (cPlanes != 1)
	{
		pm_error("%s: don't know how to handle cPlanes = %d"
			 ,ifname
			 ,cPlanes);
	}

	switch (class)
	{
	case C_WIN:
		pm_message("Windows BMP, %dx%dx%d"
			   ,cx
			   ,cy
			   ,cBitCount);
		break;
	case C_OS2:
		pm_message("OS/2 BMP, %dx%dx%d"
			   ,cx
			   ,cy
			   ,cBitCount);
		break;
	}

#ifdef DEBUG
	pm_message("cbFix: %d", cbFix);
	pm_message("cx: %d", cx);
	pm_message("cy: %d", cy);
	pm_message("cPlanes: %d", cPlanes);
	pm_message("cBitCount: %d", cBitCount);
#endif

	*pcx = cx;
	*pcy = cy;
	*pcBitCount = cBitCount;
	*pclass = class;

	*ppos += cbFix;
}

/*
 * returns the number of bytes read, or -1 on error.
 */
static int
BMPreadrgbtable(fp, ppos, cBitCount, class, R, G, B)
	FILE           *fp;
	unsigned long  *ppos;	/* number of bytes read from fp */
	unsigned short  cBitCount;
	int             class;
	pixval         *R;
	pixval         *G;
	pixval         *B;
{
	int             i;
	int		nbyte = 0;

	long            ncolors = (1 << cBitCount);

	for (i = 0; i < ncolors; i++)
	{
		B[i] = (pixval) GetByte(fp);
		G[i] = (pixval) GetByte(fp);
		R[i] = (pixval) GetByte(fp);
		nbyte += 3;

		if (class == C_WIN)
		{
			(void) GetByte(fp);
			nbyte++;
		}
	}

	*ppos += nbyte;
	return nbyte;
}

/*
 * returns the number of bytes read, or -1 on error.
 */
static int
BMPreadrow(fp, ppos, row, cx, cBitCount, R, G, B)
	FILE           *fp;
	unsigned long  *ppos;	/* number of bytes read from fp */
	pixel          *row;
	unsigned long   cx;
	unsigned short  cBitCount;
	pixval         *R;
	pixval         *G;
	pixval         *B;
{
	BITSTREAM       b;
	unsigned        nbyte = 0;
	int             rc;
	unsigned        x;

	if ((b = pm_bitinit(fp, "r")) == (BITSTREAM) 0)
	{
		return -1;
	}

	for (x = 0; x < cx; x++, row++)
	{
		unsigned long   v;

		if ((rc = pm_bitread(b, cBitCount, &v)) == -1)
		{
			return -1;
		}
		nbyte += rc;

		PPM_ASSIGN(*row, R[v], G[v], B[v]);
	}

	if ((rc = pm_bitfini(b)) != 0)
	{
		return -1;
	}

	/*
	 * Make sure we read a multiple of 4 bytes.
	 */
	while (nbyte % 4)
	{
		GetByte(fp);
		nbyte++;
	}

	*ppos += nbyte;
	return nbyte;
}

static pixel **
BMPreadbits(fp, ppos, offBits, cx, cy, cBitCount, class, R, G, B)
	FILE           *fp;
	unsigned long  *ppos;	/* number of bytes read from fp */
	unsigned long   offBits;
	unsigned long   cx;
	unsigned long   cy;
	unsigned short  cBitCount;
	int             class;
	pixval         *R;
	pixval         *G;
	pixval         *B;
{
	pixel         **pixels;	/* output */
	long            y;

	readto(fp, ppos, offBits);

	pixels = ppm_allocarray(cx, cy);

	if(cBitCount > 24)
	{
		pm_error("%s: cannot handle cBitCount: %d"
			 ,ifname
			 ,cBitCount);
	}

	/*
	 * The picture is stored bottom line first, top line last
	 */

	for (y = cy - 1; y >= 0; y--)
	{
		int rc;
		rc = BMPreadrow(fp, ppos, pixels[y], cx, cBitCount, R, G, B);

		if(rc == -1)
		{
			pm_error("%s: couldn't read row %d"
				 ,ifname
				 ,y);
		}
		if(rc%4)
		{
			pm_error("%s: row had bad number of bytes: %d"
				 ,ifname
				 ,rc);
		}
	}

	return pixels;
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	FILE           *ifp = stdin;
	char           *usage = "[bmpfile]";
	int             argn;

	int             rc;
	unsigned long	pos = 0;

	unsigned long   offBits;

	unsigned long   cx;
	unsigned long   cy;
	unsigned short  cBitCount;
	int             class;

	pixval          R[MAXCOLORS];	/* reds */
	pixval          G[MAXCOLORS];	/* greens */
	pixval          B[MAXCOLORS];	/* blues */

	pixel         **pixels;


	ppm_init(&argc, argv);

	/*
	 * Since this command takes no flags, produce an error message
	 * if the user tries to give any.
	 * This is friendlier than if the command were to say
	 * 'no such file: -help'.
	 */

	argn = 1;
	while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0')
	{
		pm_usage(usage);
		++argn;
	}

	if (argn < argc)
	{
		ifname = argv[argn];
		ifp = pm_openr(ifname);
		++argn;
	}
	else
	{
		ifname = "standard input";
		ifp = stdin;
	}

	if (argn != argc)
	{
		pm_usage(usage);
	}

	BMPreadfileheader(ifp, &pos, &offBits);
	BMPreadinfoheader(ifp, &pos, &cx, &cy, &cBitCount, &class);

	if(offBits != BMPoffbits(class, cBitCount))
	{
		pm_message("warning: offBits is %d, expected %d"
			, pos
			, BMPoffbits(class, cBitCount));
	}

	rc = BMPreadrgbtable(ifp, &pos, cBitCount, class, R, G, B);

	if(rc != BMPlenrgbtable(class, cBitCount))
	{
		pm_message("warning: %d-byte RGB table, expected %d bytes"
			, rc
			, BMPlenrgbtable(class, cBitCount));
	}


	pixels = BMPreadbits(ifp, &pos, offBits, cx, cy
			, cBitCount, class, R, G, B);

	if(pos != BMPlenfile(class, cBitCount, cx, cy))
	{
		pm_message("warning: read %d bytes, expected to read %d bytes"
			, pos
			, BMPlenfile(class, cBitCount, cx, cy));
	}

	pm_close(ifp);
	ppm_writeppm(stdout, pixels, cx, cy, (pixval) (MAXCOLORS-1), 0);
	pm_close(stdout);

	exit(0);
}
