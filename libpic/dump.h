#ifndef DUMP_HDR
#define DUMP_HDR

/* $Header: /.dub/dub/Repository/src/zoom/libpic/dump.h,v 1.3 1998/12/14 04:47:59 pahvant Exp $ */
#include <stdio.h>
#include <pixel.h>
#define DUMP_NAMEMAX 80

typedef struct {
    short magic;		/* magic number */
    short nchan;		/* number of channels (1=monochrome, 3=RGB) */
    short dx, dy;		/* width and height of picture in pixels */
} Dump_head;

typedef struct {
    char name[DUMP_NAMEMAX];	/* picture name */
    Dump_head h;		/* file header */
    FILE *fp;			/* stream for current file */
    int headsize;		/* size of head in bytes (for fseek) */
    int headwritten;		/* has header been written? */
    int curx, cury;		/* current x and y */
} Dump;

#define DUMP_MAGIC 0x5088	/* dump magic number */

void	*dump_open(const char *file, const char *mode);
void	dump_close(void *p);

char	*dump_get_name(void *p);
void	dump_clear(void *p, Pixel1 pv);
void	dump_clear_rgba(void *p, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a);

void	dump_set_nchan(void *p, int nchan);
void	dump_set_box(void *p, int ox, int oy, int dx, int dy);
void	dump_write_pixel(void *p, int x, int y, Pixel1 pv);
void	dump_write_pixel_rgba(void *p, int x, int y,
			      Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a);
void	dump_write_row(void *p, int y, int x0, int nx, const Pixel1 *buf);
void	dump_write_row_rgba(void *p, int y, int x0, int nx,
			    const Pixel1_rgba *buf);

int	dump_get_nchan(void *p);
void	dump_get_box(void *p, int *ox, int *oy, int *dx, int *dy);
Pixel1	dump_read_pixel(void *p, int x, int y);
void	dump_read_pixel_rgba(void *p, int x, int y, Pixel1_rgba *pv);
void	dump_read_row(void *p, int y, int x0, int nx, Pixel1 *buf);
void	dump_read_row_rgba(void *p, int y, int x0, int nx, Pixel1_rgba *buf);

void dump_jump_to_pixel(void *p, int x, int y);
void dump_advance(void *p, int nx);

#endif
