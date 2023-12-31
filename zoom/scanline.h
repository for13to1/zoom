/* scanline.h: definitions for generic scanline data type and routines */

#ifndef SCANLINE_HDR
#define SCANLINE_HDR

/* $Header: /.dub/dub/Repository/src/zoom/zoom/scanline.h,v 1.1.1.1 1998/12/11 01:15:34 pahvant Exp $ */

#include <pixel.h>

/* scanline type is composed of #bytes per channel and #channels */

/* alternatives for number of bytes per channel */
#define PIXEL_BYTESMASK 3
#define PIXEL1 0
#define PIXEL2 1
#define PIXEL4 2

/* alternatives for number of channels per pixel */
#define PIXEL_CHANMASK 4
#define PIXEL_MONO 0
#define PIXEL_RGB  4

/*
 * for 1-byte rgb we use Pixel1_rgba, not Pixel1_rgb, since some compilers
 * (e.g. mips) don't pad 3-byte structures to 4 bytes as most compilers do
 */
typedef Pixel1_rgba Scanline_rgb1;
typedef Pixel2_rgba Scanline_rgb2;
typedef Pixel4_rgba  Scanline_rgb4;

typedef struct {		/* GENERIC SCANLINE */
    int type;			/* scanline type: #bytes ored with #channels */
    int len;			/* length of row array */
    int y;			/* y coordinate of this scanline (if we care) */
    union {			/* one of the following is valid dep. on type */
	Pixel1     *row1;
	Pixel2     *row2;
	Pixel4     *row4;
	Scanline_rgb1 *row1_rgb;
	Scanline_rgb2 *row2_rgb;
	Scanline_rgb4 *row4_rgb;
    } u;
} Scanline;

typedef struct {		/* SAMPLED FILTER WEIGHT TABLE */
    short i0, i1;		/* range of samples is [i0..i1-1] */
    short *weight;		/* weight[i] goes with pixel at i0+i */
} Weighttab;

#define CHANBITS 8		/* # bits per image channel */
#define CHANWHITE 255		/* pixel value of white */

extern void scanline_alloc(Scanline *buf, int type, int len);
extern void scanline_free(Scanline *buf);
extern void scanline_zero(Scanline *buf);
extern void scanline_read(Pic *p, int x0, int y0, Scanline *buf);
extern void scanline_filter(int shift, Weighttab *wtab,
                            Scanline *abuf, Scanline *bbuf);
extern void scanline_accum(int weight, Scanline *abuf, Scanline *bbuf);
extern void scanline_shift(int shift, Scanline *abuf, Scanline *bbuf);
extern void scanline_write(Pic *p, int x0, int y0, Scanline *buf);

#endif
