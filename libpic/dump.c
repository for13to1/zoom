/*
 * dump: subroutine package to read and write my dump picture file format
 *
 * Paul Heckbert	1 Oct 1988
 */

static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/dump.c,v 1.3 1998/12/14 04:47:58 pahvant Exp $";

#include <simple.h>
#include "dump.h"
#include <pic.h>

#ifdef vax		/* swap bytes in header if little-endian */
#   define BYTESWAP
#endif

#define UNDEF PIXEL_UNDEFINED

#define CHECK_HEAD_UNWRITTEN(p, subrname) {	\
    if ((p)->headwritten) {			\
	fprintf(stderr, "%s: can't change state once writing commences\n", \
	    subrname);				\
	exit(1);				\
    }						\
}
#define CHECK_HEAD_WRITTEN(p, subrname) \
    if (!(p)->headwritten) dump_write_head(p, subrname); else

static void *dump_open_read(const char *file);
static void *dump_open_write(const char *file);

void *
dump_open(const char *file, const char *mode)
{
    if ('r' == mode[0]) return dump_open_read(file);
    if ('w' == mode[0]) return dump_open_write(file);
    fprintf(stderr, "dump_open: can't do mode %s\n", mode);
    exit(1); /*NOTREACHED*/
}

static void *
dump_open_write(const char *file)
{
    Dump *p;

    ALLOC(p, Dump, 1);
    if ((p->fp = fopen(file, "w")) == NULL) {
	fprintf(stderr, "dump_open_write: can't write %s\n", file);
	free(p);
	return 0;
    }
    p->h.magic = DUMP_MAGIC;
    p->h.nchan = 3;
    p->h.dx = UNDEF;
    strcpy(p->name, file);
    p->headsize = sizeof(Dump_head);
    p->headwritten = 0;
    p->curx = p->cury = 0;
    return p;
}

static void
dump_write_head(void *vp, const char *subrname)
{
  Dump *p = (Dump *) vp;
    if (p->h.dx==UNDEF) {
	fprintf(stderr, "%s: size of %s is uninitialized\n", p->name);
	exit(1);
    }
#   ifdef BYTESWAP
	headswap(&p->h);
#   endif
    if (fwrite(&p->h, sizeof(Dump_head), 1, p->fp) != 1) {
	fprintf(stderr, "%s: write error on %s\n", subrname, p->name);
	exit(1);
    }
    printf("%s: %dx%d %d-chan\n", p->name, p->h.dx, p->h.dy, p->h.nchan);
    p->headwritten = 1;
}

static void *
dump_open_read(const char *file)
{
    int badhead;
    Dump *p;

    ALLOC(p, Dump, 1);
    if ((p->fp = fopen(file, "r")) == NULL) {
	fprintf(stderr, "dump_open_read: can't find %s\n", file);
	free(p);
	return 0;
    }

    badhead = fread(&p->h, sizeof(Dump_head), 1, p->fp) != 1;
#   ifdef BYTESWAP
	headswap(&p->h);
#   endif
    if (badhead || p->h.magic!=DUMP_MAGIC) {
	fprintf(stderr, "dump_open_read: %s is not a dump file\n", file);
	free(p);
	return 0;
    }
    printf("%s: %dx%d %d-chan\n", file, p->h.dx, p->h.dy, p->h.nchan);
    strcpy(p->name, file);
    p->headsize = sizeof(Dump_head);
    p->headwritten = 1;
    p->curx = p->cury = 0;
    return p;
}

#ifdef BYTESWAP

static void
headswap(Dump_head *h)
{
    swap_short(&h->magic);
    swap_short(&h->nchan);
    swap_short(&h->dx);
    swap_short(&h->dy);
}

#endif

void
dump_close(void *pv)
{
  Dump *p = (Dump *) pv;
  if (p->fp) fclose(p->fp);
  free(p);
}

char *
dump_get_name(void *pv)
{
  Dump *p = (Dump *) pv;
  return p->name;
}

void
dump_clear(void *vp, Pixel1 pv)
{
    fprintf(stderr, "dump_clear: unimplemented\n");
}

void
dump_clear_rgba(void *vp, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
    fprintf(stderr, "dump_clear_rgba: unimplemented\n");
}

/*-------------------- file writing routines --------------------*/

void
dump_set_nchan(void *vp, int nchan)
{
  Dump *p = (Dump *) vp;
    CHECK_HEAD_UNWRITTEN(p, "dump_set_nchan");
    if (nchan!=1 && nchan!=3) {
	fprintf(stderr, "dump_set_nchan: can't handle nchan=%d\n", nchan);
	exit(1);
    }
    p->h.nchan = nchan;
}

void
dump_set_box(void *vp, int ox, int oy, int dx, int dy)
{
  Dump *p = (Dump *) vp;
    CHECK_HEAD_UNWRITTEN(p, "dump_set_box");
    /* ignore ox, oy */
    p->h.dx = dx;
    p->h.dy = dy;
}

void
dump_write_pixel(void *vp, int x, int y, Pixel1 pv)
{
    fprintf(stderr, "dump_write_pixel: unimplemented\n");
}

void
dump_write_pixel_rgba(void *vp, int x, int y,
		      Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
    fprintf(stderr, "dump_write_pixel_rgba: unimplemented\n");
}

void
dump_write_row(void *vp, int y, int x0, int nx, const Pixel1 *buf)
{
  Dump *p = (Dump *) vp;
    CHECK_HEAD_WRITTEN(p, "dump_write_row");
    if (x0!=p->curx || y!=p->cury)
	dump_jump_to_pixel(p, x0, y);
    if (fwrite(buf, nx*sizeof(Pixel1), 1, p->fp) != 1) {
	fprintf(stderr, "dump_write_row: write error on %s\n", p->name);
	exit(1);
    }
    dump_advance(p, nx);
}

void
dump_write_row_rgba(void *vp, int y, int x0, int nx, const Pixel1_rgba *buf)
{
    int x;

  Dump *p = (Dump *) vp;

    CHECK_HEAD_WRITTEN(p, "dump_write_row_rgba");
    if (x0!=p->curx || y!=p->cury)
	dump_jump_to_pixel(p, x0, y);
    for (x=nx; --x>=0; buf++) {
	putc(buf->r, p->fp);
	putc(buf->g, p->fp);
	putc(buf->b, p->fp);
    }
    dump_advance(p, nx);
}

/*-------------------- file reading routines --------------------*/

int
dump_get_nchan(void *vp)
{
  Dump *p = (Dump *) vp;
    return p->h.nchan;
}

void
dump_get_box(void *vp, int *ox, int *oy, int *dx, int *dy)
{
  Dump *p = (Dump *) vp;
    if (p->h.dx==UNDEF) {
	*ox = UNDEF;		/* used by some programs (e.g. zoom) */
	*oy = UNDEF;
    }
    else {
	*ox = 0;
	*oy = 0;
    }
    *dx = p->h.dx;
    *dy = p->h.dy;
}

Pixel1
dump_read_pixel(void *vp, int x, int y)
{
    fprintf(stderr, "dump_read_pixel: unimplemented\n");
    return 0;
}

void
dump_read_pixel_rgba(void *vp, int x, int y, Pixel1_rgba *pv)
{
    fprintf(stderr, "dump_read_pixel_rgba: unimplemented\n");
}

void
dump_read_row(void *vp, int y, int x0, int nx, Pixel1 *buf)
{
  Dump *p = (Dump *) vp;
    if (x0!=p->curx || y!=p->cury)
	dump_jump_to_pixel(p, x0, y);
    if (fread(buf, nx*sizeof(Pixel1), 1, p->fp) != 1) {
	fprintf(stderr, "dump_read_row: read error on %s\n", p->name);
	exit(1);
    }
    dump_advance(p, nx);
}

void
dump_read_row_rgba(void *vp, int y, int x0, int nx, Pixel1_rgba *buf)
{
    int x;
  Dump *p = (Dump *) vp;

    if (x0!=p->curx || y!=p->cury)
	dump_jump_to_pixel(p, x0, y);
    for (x=nx; --x>=0; buf++) {
	buf->r = getc(p->fp);
	buf->g = getc(p->fp);
	buf->b = getc(p->fp);
	buf->a = PIXEL1_MAX;
    }
    dump_advance(p, nx);
}

void
dump_jump_to_pixel(void *vp, int x, int y)
{
  Dump *p = (Dump *) vp;
    /* fprintf(stderr, "jumping from (%d,%d) to (%d,%d) in %s\n", */
	/* p->curx, p->cury, x, y, p->name); */
    p->curx = x;
    p->cury = y;
    assert(fseek(p->fp, p->headsize+(y*p->h.dx+x)*p->h.nchan*sizeof(Pixel1), 0) == 0);
}

void
dump_advance(void *vp, int nx)
{
  Dump *p = (Dump *) vp;
    p->curx += nx;
    if (p->curx >= p->h.dx) {
	p->curx -= p->h.dx;
	p->cury++;
    }
}

static Pic_procs
pic_dump_procs = {
  dump_open, dump_close,
  dump_get_name,
  dump_clear, dump_clear_rgba,

  dump_set_nchan, dump_set_box,
  dump_write_pixel, dump_write_pixel_rgba,
  dump_write_row, dump_write_row_rgba,

  dump_get_nchan, dump_get_box,
  dump_read_pixel, dump_read_pixel_rgba,
  dump_read_row, dump_read_row_rgba,
};

Pic
pic_dump = {
  "dump",
  &pic_dump_procs
};
