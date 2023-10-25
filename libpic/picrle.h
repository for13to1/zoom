#ifndef RLE_HDR
#define RLE_HDR

/* $Header: /.dub/dub/Repository/src/zoom/libpic/picrle.h,v 1.1.1.1 1998/12/11 01:15:32 pahvant Exp $ */
#include <stdio.h>
#include <rle.h>
#include <pixel.h>
#define RLE_NAMEMAX 80

typedef struct {
    char name[RLE_NAMEMAX];	/* picture name */
    short nchan;		/* number of channels (1=monochrome, 3=RGB
                                                          4=RGBA          */
    short ox, oy;		/* origin (upper left corner) of screen */
    short dx, dy;		/* width and height of picture in pixels */
    struct rle_hdr fb;		/* The rle structure */
    FILE *fp;
    char mode;			/* r (READ) or w (WRITE) */
    unsigned char **row;        /* scanline buffer */
    int nexty;			/* next scanline number to read */
} Rle;

Rle	*rle_open(/* file, mode */);
void	rle_close(/* p */);

char	*rle_get_name(/* p */);
void	rle_clear(/* p, pv */);
void	rle_clear_rgba(/* p, r, g, b, a */);

void	rle_set_nchan(/* p, nchan */);
void	rle_set_box(/* p, ox, oy, dx, dy */);
void	rle_write_pixel(/* p, x, y, pv */);
void	rle_write_pixel_rgba(/* p, x, y, r, g, b, a */);
void	rle_write_row(/* p, y, x0, nx, buf */);
void	rle_write_row_rgba(/* p, y, x0, nx, buf */);

int	rle_get_nchan(/* p */);
void	rle_get_box(/* p, ox, oy, dx, dy */);
Pixel1	rle_read_pixel(/* p, x, y */);
void	rle_read_pixel_rgba(/* p, x, y, pv */);
void	rle_read_row(/* p, y, x0, nx, buf */);
void	rle_read_row_rgba(/* p, y, x0, nx, buf */);

int     rle_next_pic( /* p */ );
#endif
