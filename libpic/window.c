#include <simple.h>
#include "window.h"

static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/window.c,v 1.2 1998/12/13 23:07:16 pahvant Exp $";

void
window_set(int x0, int y0, int x1, int y1, Window *a)
{
    a->x0 = x0;
    a->y0 = y0;
    a->x1 = x1;
    a->y1 = y1;
}

/* a=intersect(a,b), return overlap bit */
int
window_clip(Window *a, const Window *b)
{
    int overlap;

    overlap = window_overlap(a, b);
    window_intersect(a, b, a);
    return overlap;
}

/* c = intersect(a,b) */
void
window_intersect(const Window *a, const Window *b, Window *c)
{
    c->x0 = MAX(a->x0, b->x0);
    c->y0 = MAX(a->y0, b->y0);
    c->x1 = MIN(a->x1, b->x1);
    c->y1 = MIN(a->y1, b->y1);
}

int
window_overlap(const Window *a, const Window *b)
{
    return a->x0<=b->x1 && a->x1>=b->x0 && a->y0<=b->y1 && a->y1>=b->y0;
}

void
window_print(const char *str, const Window *a)
{
    fprintf(stderr,"%s{%d,%d,%d,%d}%dx%d",
	str, a->x0, a->y0, a->x1, a->y1, a->x1-a->x0+1, a->y1-a->y0+1);
}

/*----------------------------------------------------------------------*/

void
window_box_intersect(const Window_box *a, const Window_box *b, Window_box *c)
{
    c->x0 = MAX(a->x0, b->x0);
    c->y0 = MAX(a->y0, b->y0);
    c->x1 = MIN(a->x1, b->x1);
    c->y1 = MIN(a->y1, b->y1);
    window_box_set_size(c);
}

void
window_box_print(const char *str, const Window_box *a)
{
    fprintf(stderr,"%s{%d,%d,%d,%d}%dx%d",
	str, a->x0, a->y0, a->x1, a->y1, a->nx, a->ny);
}

void
window_box_set_max(Window_box *a)
{
    a->x1 = a->x0+a->nx-1;
    a->y1 = a->y0+a->ny-1;
}

void
window_box_set_size(Window_box *a)
{
    a->nx = a->x1-a->x0+1;
    a->ny = a->y1-a->y0+1;
}
