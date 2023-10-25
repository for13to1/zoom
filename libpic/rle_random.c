/*
 * rle.c: subroutine package to read and write RLE files
 *         glue between the URT (rle)  library and Paul Heckbert's Pic library
 *
 *  This version allows random access by buffering the whole image in memory.
 *  Pretty wasteful of memory and not needed by zoom (except in the case of
 *  in-place updates).
 *
 * James Painter   11 Aug 1989
 */
#include <simple.h>
#include "picrle.h"

#define UNDEF PIXEL_UNDEFINED

Rle *rle_open_read(), *rle_open_write();

Rle *rle_open(file, mode)
char *file, *mode;
{
    if ('r' == mode[0]) return rle_open_read(file);
    if ('w' == mode[0]) return rle_open_write(file);
    fprintf(stderr, "rle_open: can't do mode %s\n", mode);
    exit(1); /*NOTREACHED*/
}

static Rle *rle_open_write(file)
char *file;
{
    Rle *p;

    ALLOC(p, Rle, 1);
    if (strcmp(file,"-.rle") == 0)
      p->fp = stdout;
    else if ((p->fp = fopen(file, "w")) == NULL) {
	fprintf(stderr, "rle_open_write: can't write %s\n", file);
	free(p);
	return 0;
    }

    p->nchan = 4;
    p->ox = p->oy = UNDEF;
    p->dx = p->dy = 512;
    p->image = 0;
    p->mode   = 'w';
    strcpy(p->name, file);
    return p;
}

static void rle_write_head(p, subrname)
Rle *p;
char *subrname;
{
  int i;
  int aflag = (p->nchan == 4);

  p->fb = rle_dflt_hdr;
  p->fb.alpha = aflag;
  p->fb.ncolors = p->nchan - aflag;
  p->fb.xmin = p->ox;
  p->fb.ymin = p->oy;
  p->fb.xmax = p->ox + p->dx -1;
  p->fb.ymax = p->oy + p->dy -1;
  p->fb.rle_file  = p->fp;
  
  /* Allocate memory for the image */
  if (p->image == 0)
  {
    unsigned dims[3];

    dims[0] = p->dy;
    dims[1] = p->nchan;
    dims[2] = p->dx;
    p->image = (Pixel1 ***) mallocNd( 3, dims, sizeof(Pixel1) );
  }
}

static Rle *rle_open_read(file)
char *file;
{
    Rle *p;
    unsigned dims[3];
    int i,x, y;
    unsigned char **inprow;
    int aflag;

    ALLOC(p, Rle, 1);
    if (strcmp(file,"-.rle") == 0) 
      p->fp = stdin;
    else if ((p->fp = fopen(file, "r")) == NULL) {
	fprintf(stderr, "rle_open_read: can't find %s\n", file);
	free(p);
	return 0;
    }
    p->fb = rle_dflt_hdr;
    p->fb.rle_file = p->fp;
    rle_get_setup( &p->fb );
    aflag = p->fb.alpha;

    p->nchan = p->fb.ncolors + aflag;
    if (p->nchan != 1 && p->nchan != 3 && p->nchan != 4) {
      fprintf( stderr, "rle_open_read: unsupported color system: \n");
      exit(0);
    }
    
    p->oy = 0;
    p->ox = 0;
    p->dy = p->fb.ymax - p->fb.ymin + 1;
    p->dx = p->fb.xmax - p->fb.xmin + 1;
    strcpy(p->name, file);

    dims[0] = p->dy;
    dims[1] = p->nchan;
    dims[2] = p->dx;
    p->image = (Pixel1 ***) mallocNd( 3, dims, sizeof(Pixel1) );

    /* Allocate input buffer. */
  
    if (rle_row_alloc( &p->fb, &inprow ))
      {
	fprintf(stderr, "rle.c: Out of memory.\n");
	exit(-2);
      }
  

    /* Read the image into memory */
    for(y=p->dy-1; y>=0; y--) {
      Pixel1  *iptr, *end;

      rle_getrow( &p->fb, inprow );
      iptr = p->image[y][0];
      for(i=0; i<p->nchan-aflag; i++)
	for(x=p->fb.xmin; x<=p->fb.xmax; x++)
	  *iptr++ = inprow[i][x];
      if (aflag) 
	for(x=p->fb.xmin; x<=p->fb.xmax; x++)
	  *iptr++ = inprow[-1][x];
    }
    /* free the row structure */
    rle_row_free( &p->fb, inprow );
    return p;
}


void rle_close(p)
Rle *p;
{
  int aflag = (p->nchan == 4);
  int i, x, y;

  if (p->mode == 'w') {
    Pixel1  *iptr;
    unsigned char **outrow;
    int width = p->dx;

    for(i= 0; i < p->fb.ncolors; i++)
      RLE_SET_BIT( p->fb, i );
    if (aflag) RLE_SET_BIT( p->fb, RLE_ALPHA );
    rle_put_setup( &p->fb );


    /* Write out the file */
    if (rle_row_alloc( &p->fb, &outrow ))
      {
	fprintf( stderr, "%s: No memory for scanline buffer!\n" );
	exit(1);
      }

    for(y=p->dy-1; y>=0; y--) {
      iptr = p->image[y][0];
      for(i=0; i<p->nchan-aflag; i++)
	for(x=0; x<width; x++) 
	  outrow[i][x] = *iptr++;
      if (aflag)
	for(x=0; x<width; x++) 
	  outrow[-1][x] = *iptr++;
      rle_putrow( outrow, width, &p->fb );
    }
    rle_puteof( &p->fb );
  }
  if (p->fp) fclose(p->fp);
  if (p->image)  free(p->image);
  free(p);
}

char *rle_get_name(p)
Rle *p;
{
    return p->name;
}

void rle_clear(p, pv)
Rle *p;
Pixel1 pv;
{
  int x, y, count;

  if (p->nchan != 1) {
    fprintf( stderr, "rle_clear: not a 1 channel image file\n" );
    return;
  }
  if (p->mode != 'w') {
    fprintf( stderr, "rle_clear: operation not allowed unless opened for write\n" );
    return;
  }
  for(x=0; x<p->dx; x++)
    p->image[0][0][x] = pv;
  count = p->dx*sizeof(Pixel1);
  for(y=1; y<p->dy; y++)
    bcopy( p->image[0][0], p->image[y][0], count );
}

void rle_clear_rgba(p, r, g, b, a)
Rle *p;
Pixel1 r, g, b, a;
{
  int x, y, count;
  Pixel1_rgba pv;

  if (p->nchan != 3 && p->nchan != 4) {
    fprintf( stderr, "rle_clear_rgba: not a 3 or 4 channel image file\n" );
    return;
  }
  if (p->mode != 'w') {
    fprintf( stderr, "rle_clear_rgba: operation not allowed unless opened for write\n" );
    return;
  }

  pv.r = r; pv.g = g; pv.b = b; pv.a = a;
  for(x=0; x<p->dx; x++)
    {
      p->image[0][0][x] = r;
      p->image[0][1][x] = g;
      p->image[0][2][x] = b;
    }
  if (p->nchan == 4)
    for(x=0; x<p->dx; x++)
      p->image[0][3][x] = a; 
  count = p->dx*p->nchan*sizeof(Pixel1);
  for(y=1; y<p->dy; y++)
    bcopy( p->image[0][0], p->image[y][0], count );
}

/*-------------------- file writing routines --------------------*/

void rle_set_nchan(p, nchan)
Rle *p;
int nchan;
{
  if (p->image != 0) {
    fprintf( stderr, "rle_set_nchan: to late, header already written\n" );
    return;
  }
  if (nchan!=1 && nchan!=3 && nchan!=4) {
    fprintf(stderr, "rle_set_nchan: can't handle nchan=%d\n", nchan);
    exit(1);
  }
  p->nchan = nchan;
}

void rle_set_box(p, ox, oy, dx, dy)
Rle *p;
int ox, oy, dx, dy;
{
  if (p->image != 0) {
    fprintf( stderr, "rle_set_nchan: to late, header already written\n" );
    return;
  }
  p->dx = dx;
  p->dy = dy;
  p->ox = ox;
  p->oy = oy;
}

void rle_write_pixel(p, x, y, pv)
Rle *p;
int x, y;
Pixel1 pv;
{
  if (p->nchan != 1) {
    fprintf( stderr, 
     "rle_write_pixel: operation only supported on 1 channel images\n" );
    return;
  }
  if (p->image == 0)
    rle_write_head( p, "rle_write_pixel" );

  p->image[y][0][x] = pv;
}

void rle_write_pixel_rgba(p, x, y, r, g, b, a)
Rle *p;
int x, y;
Pixel1 r, g, b, a;
{
  if (p->nchan != 3 && p->nchan != 4) {
    fprintf( stderr, 
    "rle_write_pixel: operation only supported on 3 or 4 channel images\n" );
    return;
  }
  if (p->image == 0)
    rle_write_head( p, "rle_write_pixel" );

  p->image[y][0][x] = r;
  p->image[y][1][x] = g;
  p->image[y][2][x] = b;
  if (p->nchan == 4) 
    p->image[y][3][x] = a;

}

void rle_write_row(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
Pixel1 *buf;
{
  int x;

  if (p->nchan != 1) {
    fprintf( stderr, 
	    "rle_write_row: operation only supported on 1 channel images\n" );
    return;
  }
  if (p->image == 0)
    rle_write_head( p, "rle_write_row" );
  bcopy( buf, p->image[y][0]+x0, nx*sizeof(Pixel1) );
}

void rle_write_row_rgba(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
register Pixel1_rgba *buf;
{
  register Pixel1 *rp, *gp, *bp, *ap, *ptr;
  register int x;

  if (p->nchan != 3 && p->nchan !=4 ) {
    fprintf( stderr, 
    "rle_write_row_rgba: operation only supported on 3 or 4 channel  images\n" );
    return;
  }
  if (p->image == 0)
    rle_write_head( p, "rle_write_row_rgba" );
  nx += x0;
  ptr = &(buf->r); 
  rp = p->image[y][0]+x0;
  gp = p->image[y][1]+x0;
  bp = p->image[y][2]+x0;
  if (p->nchan == 3) {
    for(x=x0; x<nx; x++) {
      *rp++ = *ptr++;
      *gp++ = *ptr++;
      *bp++ = *ptr++;
      ptr++;
    }
  } else {
    ap = p->image[y][3]+x0;
    for(x=x0; x<nx; x++ ) {
      *rp++ = *ptr++;
      *gp++ = *ptr++;
      *bp++ = *ptr++;
      *ap++ = *ptr++;
    }

  }
}

/*-------------------- file reading routines --------------------*/

int rle_get_nchan(p)
Rle *p;
{
    return p->nchan;
}

void rle_get_box(p, ox, oy, dx, dy)
Rle *p;
int *ox, *oy, *dx, *dy;
{
  *ox = p->ox;
  *oy = p->oy;
  *dx = p->dx;
  *dy = p->dy;
}

Pixel1 rle_read_pixel(p, x, y)
Rle *p;
int x, y;
{
  if (p->nchan != 1) {
    fprintf( stderr, 
"rle_read_pixel: operation only supported on 1 channel images\n" );
    return 0;
  }
  if (p->image == 0) {
    fprintf( stderr, "rle_read_pixel:  no image!\n" );
    return 0;
  }
  return  p->image[y][0][x];
  
}

void rle_read_pixel_rgba(p, x, y, pv)
Rle *p;
int x, y;
Pixel1_rgba *pv;
{
  if (p->nchan != 3 && p->nchan != 4) {
    fprintf( stderr, 
    "rle_read_pixel_rgba: operation only supported on 3 or 4 channel images\n" );
    return;
  }
  if (p->image == 0) {
    fprintf( stderr, "rle_read_pixel_rgba: no image!\n" );
    return;
  }
  pv->r = p->image[y][0][x];
  pv->g = p->image[y][1][x];
  pv->b = p->image[y][2][x];
  pv->a = (p->nchan == 3) ? 255 : p->image[y][3][x];

}

void rle_read_row(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
Pixel1 *buf;
{
  if (p->nchan != 1) {
    fprintf( stderr, 
	    "rle_read_row: operation only supported on 1 channel images\n" );
    return;
  }
  if (p->image == 0) {
    fprintf( stderr, "rle_read_row: no image!\n" );
    return;
  }
  bcopy( p->image[y][0]+x0, buf, nx*sizeof(Pixel1) );
}

void rle_read_row_rgba(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
register Pixel1_rgba *buf;
{
  register int x;
  register Pixel1 *ptr, *rp, *gp, *bp, *ap;
  
  if (p->nchan != 3 && p->nchan != 4) {
    fprintf( stderr, 
"rle_read_row_rgba: operation only supported on 3 or 4 channel images\n" );
    return;
  }
  if (p->image == 0) {
    fprintf( stderr, "rle_read_row_rgba: no image!\n" );
    return;
  }
  rp = p->image[y][0]+x0;
  gp = p->image[y][1]+x0;
  bp = p->image[y][2]+x0;
  ptr = &(buf->r);
  if (p->nchan == 3)
    for(x=x0; x<nx; x++)
      {
	*ptr++ = *rp++;
	*ptr++ = *gp++;
	*ptr++ = *bp++;
	*ptr++ = 255;
      }
  else {
    ap = p->image[y][3]+x0;
    for(x=x0; x<nx; x++)
      {
	*ptr++ = *rp++;
	*ptr++ = *gp++;
	*ptr++ = *bp++;
	*ptr++ = *ap++;
      }
  }
}

