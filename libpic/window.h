#ifndef WINDOW_HDR
#define WINDOW_HDR

/* $Header: /.dub/dub/Repository/src/zoom/libpic/window.h,v 1.2 1998/12/13 23:07:17 pahvant Exp $ */

typedef struct {	/* WINDOW: A DISCRETE 2-D RECTANGLE */
    int x0, y0;		/* xmin and ymin */
    int x1, y1;		/* xmax and ymax (inclusive) */
} Window;

typedef struct {	/* WINDOW_BOX: A DISCRETE 2-D RECTANGLE */
    int x0, y0;		/* xmin and ymin */
    int x1, y1;		/* xmax and ymax (inclusive) */
    int nx, ny;		/* xsize=x1-x0+1 and ysize=y1-y0+1 */
} Window_box;

/*
 * note: because of the redundancy in the above structure, nx and ny should
 * be recomputed with window_box_set_size() when they cannot be trusted
 */

/* caution: we exploit positional coincidences in the following: */
#define window_box_overlap(a, b) \
    window_overlap((Window *)(a), (Window *)(b))

extern void
window_set(int x0, int y0, int x1, int y1, Window *a);

extern int
window_clip(Window *a, const Window *b);

extern void
window_intersect(const Window *a, const Window *b, Window *c);

extern int
window_overlap(const Window *a, const Window *b);

extern void
window_print(const char *str, const Window *a);

extern void
window_box_intersect(const Window_box *a, const Window_box *b, Window_box *c);

extern void
window_box_print(const char *str, const Window_box *a);

extern void
window_box_set_max(Window_box *a);

extern void
window_box_set_size(Window_box *a);

#endif
