#include <simple.h>
#include <pic.h>
#include <pnm.h>
//#include <assert.h>

struct tagPicPnm {
  char *name;
  FILE *file;
  int read_not_write;
  int width;
  int height;
  int nchan;
  xelval maxval;
  int format;
  int scanline;
  union {
    pixel *ppm;
    gray *pgm;
    bit *pbm;
  } row;
};
typedef struct tagPicPnm PicPnm;
#undef const
static void *
picpnm_open(const char *filename, const char *mode)
{
  PicPnm *result = NULL;
  FILE *try;

  if ('r' == mode[0] && str_eq(filename, "-.pnm"))
    try = fdopen(dup(fileno(stdin)), "rb");
  else if ('w' == mode[0] && str_eq(filename, "-.pnm"))
    try = fdopen(dup(fileno(stdout)), "wb");
  else
    try = fopen(filename, mode);
  
  if (try) {
    ALLOC(result, PicPnm, 1);
    memset(result, 0, sizeof(PicPnm));
    result->file = try;
    result->name = strdup(filename);
    if ('r' == mode[0]) {
      result->read_not_write = 1;
      pnm_readpnminit(result->file, &result->width, &result->height,
		      &result->maxval, &result->format);
      switch (PNM_FORMAT_TYPE(result->format)) {
      case PPM_TYPE:
	result->nchan = 3;
	result->row.ppm = ppm_allocrow(result->width);
	//assert(result->row.ppm);
	break;

      case PGM_TYPE:
	result->nchan = 1;
	result->row.pgm = pgm_allocrow(result->width);
	//assert(result->row.pgm);
	break;

      case PBM_TYPE:
	result->nchan = 1;
	result->row.pbm = pbm_allocrow(result->width);
	//assert(result->row.pbm);
	break;
      }
    }
  }

  return result;
}

static void
picpnm_close(void *p)
{
  PicPnm *pnm = (PicPnm *) p;
  //assert(pnm->name);
  free(pnm->name);
  //assert(pnm->file);
  fclose(pnm->file);
  switch (PNM_FORMAT_TYPE(pnm->format)) {
  case PBM_TYPE:
    pbm_freerow(pnm->row.pbm);
    pnm->row.pbm = NULL;
    break;

  case PGM_TYPE:
    pgm_freerow(pnm->row.pgm);
    pnm->row.pgm = NULL;
    break;

  case PPM_TYPE:
    ppm_freerow(pnm->row.ppm);
    pnm->row.ppm = NULL;
    break;
  }
  free(pnm);
}

static char *
picpnm_get_name(void *p)
{
  PicPnm *pnm = (PicPnm *) p;
  return pnm->name;
}

static void
picpnm_clear(void *vp, Pixel1 pv)
{
  /* hmm... ignore? */
}

static void
picpnm_clear_rgba(void *vp, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  /* hmm... ignore? */
}

static void
picpnm_set_nchan(void *vp, int nchan)
{
  PicPnm *pnm = (PicPnm *) vp;
  
  pnm->nchan = nchan;
  switch (nchan) {
  case 3:
    pnm->format = PPM_TYPE;
    pnm->maxval = 255;
    break;

  case 1:
    pnm->format = PGM_TYPE;
    pnm->maxval = 255;
    break;

  default:
    fprintf(stderr, "?picpnm_set_nchan can't handle %d channels.\n",
	    nchan);
  }
}

static void
picpnm_set_box(void *vp, int ox, int oy, int dx, int dy)
{
  PicPnm *pnm = (PicPnm *) vp;
  if (! pnm->read_not_write) {
    pnm->width = ox + dx;
    pnm->height = oy + dy;
  }    
}

static void
picpnm_write_pixel(void *vp, int x, int y, Pixel1 pv)
{
  fprintf(stderr, "?picpnm_write_pixel\n");
}

static void
picpnm_write_pixel_rgba(void *vp, int x, int y,
		     Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  fprintf(stderr, "?picpnm_write_pixel_rgba\n");
}

#define WRITE_INIT(pnm_) ((pnm_)->scanline || write_init(pnm_))
static int
write_init(PicPnm *pnm)
{
  switch (pnm->nchan) {
  case 3:
    ppm_writeppminit(pnm->file, pnm->width, pnm->height, pnm->maxval, 0);
    pnm->row.ppm = ppm_allocrow(pnm->width);
    //assert(pnm->row.ppm);
    break;

  case 1:
    pgm_writepgminit(pnm->file, pnm->width, pnm->height, pnm->maxval, 0);
    pnm->row.pgm = pgm_allocrow(pnm->width);
    //assert(pnm->row.pgm);
    break;
  }

  return 1;
}

static void
seek_row_write(PicPnm *pnm, int y)
{
  switch (PNM_FORMAT_TYPE(pnm->format)) {
  case PGM_TYPE:
    memset(pnm->row.pgm, 0, sizeof(gray)*pnm->width);
    while (pnm->scanline < y) {
      pgm_writepgmrow(pnm->file, pnm->row.pgm, pnm->width, pnm->maxval, 0);
      pnm->scanline++;
    }
    break;

  case PPM_TYPE:
    memset(pnm->row.ppm, 0, sizeof(pixel)*pnm->width);
    while (pnm->scanline < y) {
      ppm_writeppmrow(pnm->file, pnm->row.ppm, pnm->width, pnm->maxval, 0);
      pnm->scanline++;
    }
    break;
  }
}

static void
picpnm_write_row(void *vp, int y, int x0, int nx, const Pixel1 *buf)
{
  PicPnm *pnm = (PicPnm *) vp;

  if (! pnm->read_not_write && 1 == pnm->nchan && WRITE_INIT(pnm)) {
    int i;

    seek_row_write(pnm, y);
    memset(pnm->row.pgm, 0, sizeof(gray)*pnm->width);
    for (i = 0; i < nx; i++)
      pnm->row.pgm[x0+i] = buf[i];
    pgm_writepgmrow(pnm->file, pnm->row.pgm, pnm->width, pnm->maxval, 0);
    pnm->scanline++;
  }
}

static void
picpnm_write_row_rgba(void *vp, int y, int x0, int nx, const Pixel1_rgba *buf)
{
  PicPnm *pnm = (PicPnm *) vp;

  if (! pnm->read_not_write && 3 == pnm->nchan && WRITE_INIT(pnm)) {
    int i;

    seek_row_write(pnm, y);
    memset(pnm->row.ppm, 0, sizeof(pixel)*pnm->width);
    for (i = 0; i < nx; i++)
      PPM_ASSIGN(pnm->row.ppm[x0+i], buf[i].r, buf[i].g, buf[i].b);
    ppm_writeppmrow(pnm->file, pnm->row.ppm, pnm->width, pnm->maxval, 0);
    pnm->scanline++;
  }
}

static int
picpnm_get_nchan(void *vp)
{
  PicPnm *pnm = (PicPnm *) vp;
  return pnm->nchan;
}

static void
picpnm_get_box(void *vp, int *ox, int *oy, int *dx, int *dy)
{
  PicPnm *pnm = (PicPnm *) vp;

  if (pnm->read_not_write || pnm->scanline) {
    *ox = 0;
    *oy = 0;
    *dx = pnm->width;
    *dy = pnm->height;
  } else {
    *ox = PIXEL_UNDEFINED;
    *oy = PIXEL_UNDEFINED;
    *dx = PIXEL_UNDEFINED;
    *dy = PIXEL_UNDEFINED;
  }
}

static Pixel1
picpnm_read_pixel(void *vp, int x, int y)
{
  fprintf(stderr, "?picpnm_read_pixel\n");
  return 0;
}

static void
picpnm_read_pixel_rgba(void *vp, int x, int y, Pixel1_rgba *pv)
{
  fprintf(stderr, "?picpnm_read_pixel_rgba\n");
}

static void
seek_row_read(PicPnm *pnm, int y)
{
  while (pnm->scanline < y) {
    switch (PNM_FORMAT_TYPE(pnm->format)) {
    case PBM_TYPE:
      pbm_readpbmrow(pnm->file, pnm->row.pbm, pnm->width, pnm->format);
      break;

    case PGM_TYPE:
      pgm_readpgmrow(pnm->file, pnm->row.pgm, pnm->width,
		     pnm->maxval, pnm->format);
      break;

    case PPM_TYPE:
      ppm_readppmrow(pnm->file, pnm->row.ppm, pnm->width,
		     pnm->maxval, pnm->format);
      break;
    }
    pnm->scanline++;
  }
}

static void
picpnm_read_row(void *vp, int y, int x0, int nx, Pixel1 *buf)
{
  PicPnm *pnm = (PicPnm *) vp;
  
  if (pnm->read_not_write && 1 == pnm->nchan) {
    int i;

    seek_row_read(pnm, y);

    switch (PNM_FORMAT_TYPE(pnm->format)) {
    case PBM_TYPE:
      pbm_readpbmrow(pnm->file, pnm->row.pbm, pnm->width, pnm->format);
      for (i = 0; i < nx; i++)
	buf[i] = pnm->row.pbm[x0+i] ? 255 : 0;
      break;

    case PGM_TYPE:
      pgm_readpgmrow(pnm->file, pnm->row.pgm, pnm->width,
		     pnm->maxval, pnm->format);
      for (i = 0; i < nx; i++)
	buf[i] = pnm->row.pgm[x0+i];
      break;
    }
    pnm->scanline++;
  }
}

static void
picpnm_read_row_rgba(void *vp, int y, int x0, int nx, Pixel1_rgba *buf)
{
  PicPnm *pnm = (PicPnm *) vp;

  if (pnm->read_not_write && 3 == pnm->nchan) {
    int i;

    seek_row_read(pnm, y);
    ppm_readppmrow(pnm->file, pnm->row.ppm, pnm->width,
		   pnm->maxval, pnm->format);
    for (i = 0; i < nx; i++) {
      const pixel p = pnm->row.ppm[x0+i];
      buf[i].r = PPM_GETR(p);
      buf[i].g = PPM_GETG(p);
      buf[i].b = PPM_GETB(p);
      buf[i].a = 255;
    }
    pnm->scanline++;
  }
}

static int
picpnm_next_pic(void *vp)
{
  return 0;
}

static Pic_procs
picpnm_procs = {
  picpnm_open,
  picpnm_close,
  picpnm_get_name,

  picpnm_clear,
  picpnm_clear_rgba,

  picpnm_set_nchan,
  picpnm_set_box,

  picpnm_write_pixel,
  picpnm_write_pixel_rgba,
  picpnm_write_row,
  picpnm_write_row_rgba,

  picpnm_get_nchan,
  picpnm_get_box,
  picpnm_read_pixel,
  picpnm_read_pixel_rgba,
  picpnm_read_row,
  picpnm_read_row_rgba,
  picpnm_next_pic
};

Pic pic_pnm =
{
  "pnm", &picpnm_procs
};
