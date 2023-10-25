/* $Header: /.dub/dub/Repository/src/zoom/zoom/zoom.h,v 1.1.1.1 1998/12/11 01:15:34 pahvant Exp $ */

typedef struct {	/* SOURCE TO DEST COORDINATE MAPPING */
    double sx, sy;	/* x and y scales */
    double tx, ty;	/* x and y translations */
    double ux, uy;	/* x and y offset used by MAP, private fields */
} Mapping;

/* see explanation in zoom.c */

extern int zoom_debug;
extern int zoom_quiet;
extern int zoom_coerce;	/* simplify filters if possible? */
extern int zoom_xy;	/* filter x before y (1) or vice versa (0)? */
extern int zoom_trimzeros;	/* trim zeros from filter weight table? */
