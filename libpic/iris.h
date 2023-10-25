#ifndef IRIS_HDR
#define IRIS_HDR

/* $Header: /.dub/dub/Repository/src/zoom/libpic/iris.h,v 1.1.1.1 1998/12/11 01:15:31 pahvant Exp $ */
#include <stdio.h>
#include <pixel.h>
#define IRIS_NAMEMAX 80

typedef struct {
    char name[IRIS_NAMEMAX];	/* picture name */
    short nchan;		/* number of channels (1=monochrome, 3=RGB) */
    short ox, oy;		/* origin (upper left corner) of screen */
    short dx, dy;		/* width and height of picture in pixels */
    short init;			/* window initialized yet? */
    long id;			/* iris window number */
} Iris;

Iris	*iris_open(/* file, mode */);
void	iris_close(/* p */);

char	*iris_get_name(/* p */);
void	iris_clear(/* p, pv */);
void	iris_clear_rgba(/* p, r, g, b, a */);

void	iris_set_nchan(/* p, nchan */);
void	iris_set_box(/* p, ox, oy, dx, dy */);
void	iris_write_pixel(/* p, x, y, pv */);
void	iris_write_pixel_rgba(/* p, x, y, r, g, b, a */);
void	iris_write_row(/* p, y, x0, nx, buf */);
void	iris_write_row_rgba(/* p, y, x0, nx, buf */);

int	iris_get_nchan(/* p */);
void	iris_get_box(/* p, ox, oy, dx, dy */);
Pixel1	iris_read_pixel(/* p, x, y */);
void	iris_read_pixel_rgba(/* p, x, y, pv */);
void	iris_read_row(/* p, y, x0, nx, buf */);
void	iris_read_row_rgba(/* p, y, x0, nx, buf */);

#endif
