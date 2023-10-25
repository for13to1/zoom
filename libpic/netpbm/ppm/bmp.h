/*\
 * $Id: bmp.h,v 1.1.1.1 2001/01/05 06:15:51 legalize Exp $
 * 
 * bmp.h - routines to calculate sizes of parts of BMP files
 *
 * Some fields in BMP files contain offsets to other parts
 * of the file.  These routines allow us to calculate these
 * offsets, so that we can read and write BMP files without
 * the need to fseek().
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
 * $Log: bmp.h,v $
 * Revision 1.1.1.1  2001/01/05  06:15:51  legalize
 * Initial checkin
 *
 * Revision 1.3  1992/11/24  19:39:56  dws
 * Added copyright.
 *
 * Revision 1.2  1992/11/17  02:13:37  dws
 * Adjusted a string's name.
 *
 * Revision 1.1  1992/11/16  19:54:44  dws
 * Initial revision
 *
\*/

#ifndef _BMP_H_
#define _BMP_H_

#include	"pbmplus.h"

/* prototypes */
static unsigned long BMPlenfileheader ARGS((int class));
static unsigned long BMPleninfoheader ARGS((int class));
static unsigned long BMPlenrgbtable ARGS((int class, unsigned long bitcount));
static unsigned long BMPlenline ARGS((int class, unsigned long bitcount, unsigned long x));
static unsigned long BMPlenbits ARGS((int class, unsigned long bitcount, unsigned long x, unsigned long y));
static unsigned long BMPlenfile ARGS((int class, unsigned long bitcount, unsigned long x, unsigned long y));
static unsigned long BMPoffbits ARGS((int class, unsigned long bitcount));
/*
 * Classes of BMP files
 */

#define C_WIN	1
#define C_OS2	2

static char     er_internal[] = "%s: internal error!";

static unsigned long
BMPlenfileheader(class)
	int             class;
{
	switch (class)
	{
	case C_WIN:
		return 14;
	case C_OS2:
		return 14;
	default:
		pm_error(er_internal, "BMPlenfileheader");
		return 0;
	}
}

static unsigned long
BMPleninfoheader(class)
	int             class;
{
	switch (class)
	{
	case C_WIN:
		return 40;
	case C_OS2:
		return 12;
	default:
		pm_error(er_internal, "BMPleninfoheader");
		return 0;
	}
}

static unsigned long
BMPlenrgbtable(class, bitcount)
	int             class;
	unsigned long   bitcount;
{
	unsigned long   lenrgb;

	if (bitcount < 1)
	{
		pm_error(er_internal, "BMPlenrgbtable");
		return 0;
	}
	switch (class)
	{
	case C_WIN:
		lenrgb = 4;
		break;
	case C_OS2:
		lenrgb = 3;
		break;
	default:
		pm_error(er_internal, "BMPlenrgbtable");
		return 0;
	}

	return (1 << bitcount) * lenrgb;
}

/*
 * length, in bytes, of a line of the image
 * 
 * Evidently each row is padded on the right as needed to make it a
 * multiple of 4 bytes long.  This appears to be true of both
 * OS/2 and Windows BMP files.
 */
static unsigned long
BMPlenline(class, bitcount, x)
	int             class;
	unsigned long   bitcount;
	unsigned long   x;
{
	unsigned long   bitsperline;

	switch (class)
	{
	case C_WIN:
		break;
	case C_OS2:
		break;
	default:
		pm_error(er_internal, "BMPlenline");
		return 0;
	}

	bitsperline = x * bitcount;

	/*
	 * if bitsperline is not a multiple of 32, then round
	 * bitsperline up to the next multiple of 32.
	 */
	if ((bitsperline % 32) != 0)
	{
		bitsperline += (32 - (bitsperline % 32));
	}

	if ((bitsperline % 32) != 0)
	{
		pm_error(er_internal, "BMPlenline");
		return 0;
	}

	/* number of bytes per line == bitsperline/8 */
	return bitsperline >> 3;
}

/* return the number of bytes used to store the image bits */
static unsigned long
BMPlenbits(class, bitcount, x, y)
	int             class;
	unsigned long   bitcount;
	unsigned long   x;
	unsigned long   y;
{
	return y * BMPlenline(class, bitcount, x);
}

/* return the offset to the BMP image bits */
static unsigned long
BMPoffbits(class, bitcount)
	int             class;
	unsigned long   bitcount;
{
	return BMPlenfileheader(class)
		+ BMPleninfoheader(class)
		+ BMPlenrgbtable(class, bitcount);
}

/* return the size of the BMP file in bytes */
static unsigned long
BMPlenfile(class, bitcount, x, y)
	int             class;
	unsigned long   bitcount;
	unsigned long   x;
	unsigned long   y;
{
	return BMPoffbits(class, bitcount)
		+ BMPlenbits(class, bitcount, x, y);
}

#endif /* _BMP_H_ */
