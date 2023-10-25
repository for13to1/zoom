/*
 * iris: subroutine package to read and write iris pictures
 * we flip y so to the user it points down
 * note: this implementation is currently quite crufty: doesn't handle
 *	iris window events and doesn't allow pixel reading.
 *
 * Paul Heckbert	27 July 1989
 */

static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/iris.c,v 1.1.1.1 1998/12/11 01:15:31 pahvant Exp $ ";

#include <gl.h>

#include <simple.h>
#include "iris.h"

#define XMAX 1280	/* screen size */
#define YMAX 1024
#define XFUDGE 5
#define YFUDGE -25
#define UNDEF PIXEL_UNDEFINED

#define CHECK_UNINIT(p, subrname) {	\
    if ((p)->init) {			\
	fprintf(stderr, "%s: can't change state once writing commences\n", \
	    subrname);				\
	exit(1);				\
    }						\
}
#define CHECK_INIT(p, subrname) \
    if (!(p)->init) iris_init(p, subrname); else

static Colorindex sbuf[XMAX];
static RGBvalue rbuf[XMAX];
static RGBvalue gbuf[XMAX];
static RGBvalue bbuf[XMAX];

Iris *iris_open_write();

Iris *iris_open(file, mode)
char *file, *mode;
{
    if ('w' == mode[0]) return iris_open_write(file);
    fprintf(stderr, "iris_open: can't do mode %s\n", mode);
    exit(1); /*NOTREACHED*/
}

static Iris *iris_open_write(file)
char *file;
{
    Iris *p;

    ALLOC(p, Iris, 1);
    p->nchan = 3;
    /* p->ox = p->oy = UNDEF; */
    /* p->dx = p->dy = UNDEF; */
    p->ox = 0;
    p->oy = 0;
    p->dx = XMAX;
    p->dy = YMAX;
    strcpy(p->name, file);
    p->init = 0;
    return p;
}

static iris_init(p, subrname)
Iris *p;
char *subrname;
{
    if (p->dx==UNDEF) {
	fprintf(stderr, "%s: size of %s is uninitialized\n", subrname, p->name);
	exit(1);
    }
    foreground();		/* keeps process from being forked */
    if (p->ox!=UNDEF) {
	printf("prefposition(%d,%d, %d,%d)\n",
	    XFUDGE+p->ox, XFUDGE+p->ox+p->dx-1,
	    YFUDGE+YMAX-p->oy-p->dy, YFUDGE+YMAX-1-p->oy);
	prefposition(XFUDGE+p->ox, XFUDGE+p->ox+p->dx-1,
	    YFUDGE+YMAX-p->oy-p->dy, YFUDGE+YMAX-1-p->oy);
    }
				/* window size & position for winopen */
    else prefsize(p->dx, p->dy);/* window size for winopen */
    p->id = winopen(p->name);	/* create a screen window */
    if (p->nchan>=3) {
	RGBmode();		/* window is 3-channel (not 1) */
	gconfig();
	RGBcolor(0, 0, 0);
    }
    else
	color(0);
    clear();
    printf("%s: %dx%d %d-chan\n", p->name, p->dx, p->dy, p->nchan);
    p->init = 1;
}

void iris_close(p)
Iris *p;
{
    /* KLUDGE so window doesn't disappear when process dies! */
    for (;;) sleep(9999);

    /* if (p->init) winclose(p->id); */
    /* free(p); */
}

char *iris_get_name(p)
Iris *p;
{
    return p->name;
}

void iris_clear(p, pv)
Iris *p;
Pixel1 pv;
{
    color(pv);
    clear();
}

void iris_clear_rgba(p, r, g, b, a)
Iris *p;
Pixel1 r, g, b, a;
{
    RGBcolor(r, g, b);
    clear();
}

/*-------------------- file writing routines --------------------*/

void iris_set_nchan(p, nchan)
Iris *p;
int nchan;
{
    CHECK_UNINIT(p, "iris_set_nchan");
    if (nchan!=1 && nchan!=3 && nchan!=4) {
	fprintf(stderr, "iris_set_nchan: can't handle nchan=%d\n", nchan);
	exit(1);
    }
    p->nchan = nchan;
}

void iris_set_box(p, ox, oy, dx, dy)
Iris *p;
int ox, oy, dx, dy;
{
    CHECK_UNINIT(p, "iris_set_box");
    p->ox = ox;
    p->oy = oy;
    p->dx = dx;
    p->dy = dy;
}

void iris_write_pixel(p, x, y, pv)
Iris *p;
int x, y;
Pixel1 pv;
{
    Colorindex pv2;

    CHECK_INIT(p, "iris_write_row");
    cmov2i(x, p->dy-1-y);
    pv2 = pv;
    writepixels(1, &pv2);
}

void iris_write_pixel_rgba(p, x, y, r, g, b, a)
Iris *p;
int x, y;
Pixel1 r, g, b, a;
{
    CHECK_INIT(p, "iris_write_row");
    /* note: RGBvalue is an unsigned char, just like Pixel1 */
    cmov2i(x, p->dy-1-y);
    writeRGB(1, &r, &g, &b);
}

void iris_write_row(p, y, x0, nx, buf)
Iris *p;
int y, x0, nx;
register Pixel1 *buf;
{
    register int i;
    register Colorindex *s;

    CHECK_INIT(p, "iris_write_row");
    for (s=sbuf, i=nx; i>0; i--)
	*s++ = *buf++;
    cmov2i(x0, p->dy-1-y);
    writepixels(nx, sbuf);
}

void iris_write_row_rgba(p, y, x0, nx, buf)
Iris *p;
int y, x0, nx;
register Pixel1_rgba *buf;
{
    register int i;
    register RGBvalue *r, *g, *b;

    CHECK_INIT(p, "iris_write_row_rgba");
    for (r=rbuf, g=gbuf, b=bbuf, i=nx; i>0; i--, buf++) {
	*r++ = buf->r;
	*g++ = buf->g;
	*b++ = buf->b;
    }
    cmov2i(x0, p->dy-1-y);
    writeRGB(nx, rbuf, gbuf, bbuf);
}

/*-------------------- file reading routines --------------------*/

int iris_get_nchan(p)
Iris *p;
{
    return p->nchan;
}

void iris_get_box(p, ox, oy, dx, dy)
Iris *p;
int *ox, *oy, *dx, *dy;
{
    *ox = p->ox;
    *oy = p->oy;
    *dx = p->dx;
    *dy = p->dy;
}

Pixel1 iris_read_pixel(p, x, y)
Iris *p;
int x, y;
{
    fprintf(stderr, "iris_read_pixel: unimplemented\n");
}

void iris_read_pixel_rgba(p, x, y, pv)
Iris *p;
int x, y;
Pixel1_rgba *pv;
{
    fprintf(stderr, "iris_read_pixel_rgba: unimplemented\n");
}

void iris_read_row(p, y, x0, nx, buf)
Iris *p;
int y, x0, nx;
Pixel1 *buf;
{
    fprintf(stderr, "iris_read_row: unimplemented\n");
}

void iris_read_row_rgba(p, y, x0, nx, buf)
Iris *p;
int y, x0, nx;
Pixel1_rgba *buf;
{
    fprintf(stderr, "iris_read_row_rgba: unimplemented\n");
}
