/*
 * rle.c: Package to read and write RLE files
 *         glue between the URT (rle)  library and Paul Heckbert's Pic library
 *  This version is restricted to sequential access through the read/write_row
 *  functions.
 *
 * James Painter   11 Aug 1989
 */
#include <simple.h>
#include "picrle.h"

#define UNDEF PIXEL_UNDEFINED

Rle *rle_open_read(), *rle_open_write();

static Rle *infile = NULL;
static char **Argv;

Rle *rle_open(file, mode,argv)
char *file, *mode;
char *argv[];
{
    if ('r' == mode[0]) return rle_open_read(file);
    if ('w' == mode[0]) return rle_open_write(file,argv);
    fprintf(stderr, "rle_open: can't do mode %s\n", mode);
    exit(1); /*NOTREACHED*/
}

static Rle *rle_open_write(file,argv)
char *file;
char *argv[];
{
    Rle *p;

    Argv = argv;		/* stash command line args for addhist */

    ALLOC(p, Rle, 1);
    if (strcmp(file,"-.rle") == 0) file = "-";
    p->fp = rle_open_f( "piclib(rle.c):", file, "w" );
    if(p->fp == NULL)
      {
	fprintf(stderr, "rle_open_write: can't write %s\n", file);
	free((char *)p);
	return 0;
      }
    p->fb.rle_file = p->fp;


    p->ox = p->oy = UNDEF;
    p->dx = p->dy = 512;
    p->nchan = 4;
    p->row = 0;
    p->mode   = 'w';
    strcpy(p->name, file);
    return p;
}

/*ARGSUSED*/
static void rle_write_head(p, subrname)
Rle *p;
char *subrname;
{
  int i;
  int aflag = (p->nchan == 4 || p->nchan == 2);
  struct rle_hdr *in_hdr = (infile) ? &infile->fb : NULL;

  p->fb = rle_dflt_hdr;
  p->fb.alpha = aflag;
  p->fb.ncolors = p->nchan - aflag;
  p->fb.xmin = p->ox;
  p->fb.ymin = p->oy;
  p->fb.xmax = p->ox + p->dx -1;
  p->fb.ymax = p->oy + p->dy -1;
  p->fb.rle_file  = p->fp;

  rle_addhist( Argv, in_hdr, &p->fb );

  for(i= 0; i < p->fb.ncolors; i++)
    RLE_SET_BIT( p->fb, i );
  if (aflag) RLE_SET_BIT( p->fb, RLE_ALPHA );
  rle_put_setup( &p->fb );


  /* Allocate the output buffer. */
  
  if (rle_row_alloc( &p->fb, &p->row ))
    {
      fprintf(stderr, "rle.c: Out of memory.\n");
      exit(-2);
    }
  p->nexty = p->fb.ymin;
  
}

static void setup_from_header( p )
Rle *p;
{
  int aflag;

  aflag = p->fb.alpha;
  p->nchan = p->fb.ncolors;
  if (p->nchan != 1 && p->nchan != 3) 
   {
     fprintf( stderr, "rle_open_read: unsupported color system.\n%s",
	     "must be I, RGB or RGBA\n" );
     exit(0);
   }
  p->mode   = 'r';
  p->nchan += aflag;    
  
  p->oy = p->fb.ymin;
  p->ox = p->fb.xmin;
  p->dy = p->fb.ymax - p->fb.ymin + 1;
  p->dx = p->fb.xmax - p->fb.xmin + 1;
  
  /* Allocate input buffer. */
  
  if (rle_row_alloc( &p->fb, &p->row ))
   {
     fprintf(stderr, "piclib(rle.c): Out of memory.\n");
     exit(-2);
   }
  p->nexty = p->fb.ymin;
}


static Rle *rle_open_read(file)
     char *file;
{
  Rle *p;

  ALLOC(p, Rle, 1);
  p->fb = rle_dflt_hdr;
  if (strcmp(file,"-.rle") == 0) file = "-";
  p->fp = p->fb.rle_file = rle_open_f( "piclib(rle.c)", file, "r" );
  
  if (p->fp == NULL) 
   {
     fprintf(stderr, "rle_open_read: can't find %s\n", file);
     free( (char *) p);
     return 0;
   }
  strcpy(p->name, file);
  rle_get_setup( &p->fb );
  setup_from_header( p );
  infile = p;
  return p;
}


static void rle_finalize_pic(p)
Rle *p;
{
  if (p->mode == 'w')  {
    /* flush current line */
    rle_putrow( p->row, p->dx, &p->fb );
    p->nexty++;
    if (p->nexty < p->oy+p->dy) {
      memset( (char *) p->row[0], 0, p->dx );
      /* clear line buffer */
      if (p->nchan > 1) {
	memset( (char *) p->row[1], 0, p->dx );
	memset( (char *) p->row[2], 0, p->dx );
	if (p->nchan > 3)
	  memset( (char *) p->row[-1], 0, p->dx );
	while(p->nexty < p->oy+p->dy) {
	  rle_putrow(p->row, p->dx, &p->fb);
	  p->nexty++;
	}
      }
    }
    rle_puteof( &p->fb );
  }

  /* free the row structure */
  if (p->row != 0) 
    rle_row_free( &p->fb, p->row );
  p->row = 0;
}

void rle_close(p)
Rle *p;
{
  rle_finalize_pic(p);
  if (p->fp) fclose(p->fp);
  free( (char *)p);
}

int rle_next_pic(p)
Rle *p;
{
  rle_finalize_pic(p);
  if (p->mode == 'r') 
   {
     /* Look for the next image  in the file */
     if ( rle_get_setup( &p->fb ) != RLE_SUCCESS) return 0;
     setup_from_header(p);
   }
  else
   {
     p->ox = p->oy = UNDEF;
     p->dx = p->dy = 512;
     p->nchan = 4;
     p->row = 0;
   }
  return 1;
}

char *rle_get_name(p)
Rle *p;
{
    return p->name;
}


/*ARGSUSED*/
void rle_clear(p, pv)
Rle *p;
Pixel1 pv;
{
  fprintf( stderr, "rle_clear: operation not supported\n");
}

/*ARGSUSED*/
void rle_clear_rgba(p, r, g, b, a)
Rle *p;
Pixel1 r, g, b, a;
{
  fprintf( stderr, "rle_clear_rgba: operation not supported\n");
}

/*-------------------- file writing routines --------------------*/

void rle_set_nchan(p, nchan)
Rle *p;
int nchan;
{
  if (p->row != 0) {
    fprintf( stderr, "rle_set_nchan: to late, header already written\n" );
    return;
  }
  if (nchan!=1 && nchan!=2 && nchan!=3 && nchan!=4) {
    fprintf(stderr, "rle_set_nchan: can't handle nchan=%d\n", nchan);
    exit(1);
  }
  p->nchan = nchan;
}

void rle_set_box(p, ox, oy, dx, dy)
Rle *p;
int ox, oy, dx, dy;
{
  if (p->row != 0) {
    fprintf( stderr, "rle_set_box: to late, header already written\n" );
    return;
  }
  if (ox < 0 || oy < 0) {
    fprintf( stderr, "rle_set_box:  offsets must be positive: (%d,%d)\n", ox, oy );
    if (ox < 0) ox = 0;
    if (oy < 0) oy = 0;
  }

  p->dx = dx;
  p->dy = dy;
  p->ox = ox;
  p->oy = oy;
}

/*ARGSUSED*/
void rle_write_pixel(p, x, y, pv)
Rle *p;
int x, y;
Pixel1 pv;
{
  fprintf( stderr, "rle_write_pixel: operation not supported\n");
}

/*ARGSUSED*/
void rle_write_pixel_rgba(p, x, y, r, g, b, a)
Rle *p;
int x, y;
Pixel1 r, g, b, a;
{
  fprintf( stderr, "rle_write_pixel_rgba: operation not supported\n");
}

void rle_write_row(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
register Pixel1 *buf;
{
  register Pixel1 *rowp, *end;

  if (p->nchan != 1) {
    fprintf( stderr, 
	    "rle_write_row: operation only supported on 1 channel images\n" );
    return;
  }

  if (p->row == 0)
    rle_write_head( p, "rle_write_row" );

  if (y >= p->oy+p->dy) {
    fprintf( stderr, "rle_write_row: y out of range: %d\n", y );
    return;
  }

  if (y > p->nexty) {
    rle_putrow( p->row, p->dx, &p->fb );
    p->nexty++;
    memset( (char *) p->row[0], 0, p->dx );
    while (y > p->nexty) {
      rle_putrow( p->row, p->dx, &p->fb );
      p->nexty++;
      
    }
  }

  if (y != p->nexty) {
    fprintf( stderr, 
   "rle_read_row_rgba: Sorry, only forward access allowed. want line:%d have:%d\n",
	    y, p->nexty-1 );
    return;
  }


  rowp = p->row[0] + x0 - p->ox;
  end = rowp + nx;
  while (rowp < end)
    *rowp++ = *buf++;
}

void rle_write_row_rgba(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
Pixel1_rgba *buf;
{
  register Pixel1 *ptr, *end, *rp, *gp, *bp, *ap;
  rle_pixel r,g,b;

  if (p->row == 0)
    rle_write_head( p, "rle_write_row_rgba" );

  if (y >= p->oy+p->dy) {
    fprintf( stderr, "rle_write_row_rgba: y out of range: %d\n", y );
    return;
  }

  if (y > p->nexty) {
    rle_putrow( p->row, p->dx, &p->fb );
    p->nexty++;
    memset( (char *) p->row[0], 0, p->dx );
    if (p->nchan > 2) 
      {
	memset( (char *) p->row[1], 0, p->dx );
	memset( (char *) p->row[2], 0, p->dx );
      }
    if (p->nchan == 2 || p->nchan == 4) 
      memset( (char *)p->row[-1], 0, p->dx );
    while (y > p->nexty) {
      rle_putrow( p->row, p->dx, &p->fb );
      p->nexty++;
    }
  }

  if (y != p->nexty) {
    fprintf( stderr, 
   "rle_read_row_rgba: Sorry, only forward access allowed. want line:%d have:%d\n",
	    y, p->nexty-1 );
    return;
  }

  ptr = &(buf->r);
  rp = p->row[0] + x0 - p->ox;
  end = rp+nx;
  switch (p->nchan)
    {
    case 1:
      while (rp < end)
	{
	  r = *ptr++; g = *ptr++; b = *ptr++;
	  *rp++ = (30*r + 59*g + 11*b) / 100;
	  ptr ++;
	}
      break;
    case 2:
      ap = p->row[-1] + x0 - p->ox;
      while (rp < end)
	{
	  r = *ptr++; g = *ptr++; b = *ptr++;
	  *rp++ = (30*r + 59*g + 11*b) / 100;
	  *ap++ = *ptr++;
	}
      break;
    case 3:
      gp = p->row[1] + x0 - p->ox;
      bp = p->row[2] + x0 - p->ox;
      while (rp < end) 
	{
	  *rp++ = *ptr++;
	  *gp++ = *ptr++;
	  *bp++ = *ptr++;
	  ptr++;
	}
    case 4:
      gp = p->row[1] + x0 - p->ox;
      bp = p->row[2] + x0 - p->ox;
      ap = p->row[-1] + x0 - p->ox;
      while (rp < end) 
	{
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


/*ARGSUSED*/
Pixel1 rle_read_pixel(p, x, y)
Rle *p;
int x, y;
{
  fprintf( stderr, "rle_read_pixel: operation not supported\n");
}

/*ARGSUSED*/
void rle_read_pixel_rgba(p, x, y, pv)
Rle *p;
int x, y;
Pixel1_rgba *pv;
{
  fprintf( stderr, "rle_read_pixel: operation not supported\n");
}

void rle_read_row(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
register Pixel1 *buf;
{
  register Pixel1 *rowp, *end;

  if (p->nchan != 1) {
    fprintf( stderr, 
	    "rle_read_row: operation only supported on 1 channel images\n" );
    return;
  }
  if (p->row == 0) {
    fprintf( stderr, "rle_read_row: no image!\n" );
    return;
  }
  while (y >= p->nexty) {
    rle_getrow( &p->fb, p->row );
    p->nexty++;
  }
  if (y != p->nexty-1) {
    fprintf( stderr, 
    "rle_read_row: Sorry, only forward access allowed. want line:%d have:%d\n", 
	    y, p->nexty-1 );
    return;
  }
  rowp = p->row[0] + x0;
  end = rowp + nx;
  while (rowp < end)
    *buf++ = *rowp++;
}

void rle_read_row_rgba(p, y, x0, nx, buf)
register Rle *p;
int y, x0, nx;
register Pixel1_rgba *buf;
{
  register Pixel1 *ptr, *end, *rp, *gp, *bp, *ap;
  
  while (y >= p->nexty) {
    rle_getrow( &p->fb, p->row );
    p->nexty++;
  }
  if (y != p->nexty-1) {
    fprintf( stderr, 
"rle_read_row_rgba: Sorry, only forward access allowed. want line:%d have:%d\n",
	    y, p->nexty-1 );
    return;
  }

  ptr = &(buf->r);
  rp = p->row[0] + x0;
  end = rp + nx;
  switch (p->nchan)
    {
    case 1:
      while (rp < end) {
	*ptr++ = *rp;
	*ptr++ = *rp;
	*ptr++ = *rp++;
	ptr++;
      }
      break;
    case 2:
      ap = p->row[-1] + x0;
      while (rp < end) {
	*ptr++ = *rp;
	*ptr++ = *rp;
	*ptr++ = *rp++;
	*ptr++ = *ap++;
      }
      break;
    case 3:
      gp = p->row[1] + x0;
      bp = p->row[2] + x0;
      end = rp+nx;
      while (rp < end) {
	*ptr++ = *rp++;
	*ptr++ = *gp++;
	*ptr++ = *bp++;
	ptr++;
      }
      break;
    case 4:
      gp = p->row[1] + x0;
      bp = p->row[2] + x0;
      ap = p->row[-1] + x0;
      while (rp < end) {
	*ptr++ = *rp++;
	*ptr++ = *gp++;
	*ptr++ = *bp++;
	*ptr++ = *ap++;
      }
      break;
    }
}

