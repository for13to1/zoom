#include <simple.h>
#include <pic.h>
#include <string.h>
#include <tiffio.h>

struct tagPicTif {
  char *name;
  int read_not_write;
  int format;
  int scanline;
  int width;
  int height;
  int num_channels;
  int row_stride;
  int rows_per_strip;
  unsigned char *row;
  TIFF *tif;
};
typedef struct tagPicTif PicTif;

static void *
pictif_open(const char *filename, const char *mode)
{
  PicTif *tif = NULL;
  TIFF *try = NULL;

  /* TIFF doesn't support streaming I/O */
  if (('r' == mode[0] && str_eq(filename, "-.tif")) ||
      ('w' == mode[0] && str_eq(filename, "-.tif")))
    try = NULL;
  else
    try = TIFFOpen(filename, mode);
  
  if (try) {
    ALLOC(tif, PicTif, 1);
    memset(tif, 0, sizeof(PicTif));
    tif->tif = try;
    tif->name = strdup(filename);
    if ('r' == mode[0]) {
      uint32 width = 0;
      uint32 height = 0;
      uint16 chans = 0;

      tif->read_not_write = 1;
      /* read TIF header and determine image size */
      tif->row_stride = TIFFScanlineSize(tif->tif);
      tif->row = (unsigned char *) _TIFFmalloc(tif->row_stride);

      TIFFGetField(tif->tif, TIFFTAG_IMAGEWIDTH, &width);
      tif->width = width;
      TIFFGetField(tif->tif, TIFFTAG_IMAGELENGTH, &height);
      tif->height = height;
      TIFFGetField(tif->tif, TIFFTAG_SAMPLESPERPIXEL, &chans);
      tif->num_channels = chans;
    } else {
      tif->read_not_write = 0;
      /* initialize header for writing */
    }
  }

  return tif;
}

static void
pictif_close(void *p)
{
  PicTif *tif = (PicTif *) p;

  TIFFClose(tif->tif);
  assert(tif->name);
  free(tif->name);
  free(tif);
}

static char *
pictif_get_name(void *p)
{
  PicTif *tif = (PicTif *) p;
  return tif->name;
}

static void
pictif_clear(void *vp, Pixel1 pv)
{
  /* hmm... ignore? */
}

static void
pictif_clear_rgba(void *vp, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  /* hmm... ignore? */
}

static void
pictif_set_nchan(void *vp, int nchan)
{
  PicTif *tif = (PicTif *) vp;
  
  if (! tif->read_not_write) {
    tif->num_channels = nchan;
  }
}

static void
pictif_set_box(void *vp, int ox, int oy, int dx, int dy)
{
  PicTif *tif = (PicTif *) vp;
  if (! tif->read_not_write) {
    tif->width = ox + dx;
    tif->height = oy + dy;
  }    
}

static void
pictif_write_pixel(void *vp, int x, int y, Pixel1 pv)
{
  fprintf(stderr, "?pictif_write_pixel\n");
  /* output scanlines until we reach scanline y */
  /* set pixel x of scanline to pv */
}

static void
pictif_write_pixel_rgba(void *vp, int x, int y,
		     Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  fprintf(stderr, "?pictif_write_pixel_rgba\n");
}

#define WRITE_INIT(tif_) ((tif_)->scanline || write_init(tif_))
static int
write_init(PicTif *tif)
{
  /* prepare TIF header for writing */
  TIFFSetField(tif->tif, TIFFTAG_IMAGEWIDTH, tif->width);
  TIFFSetField(tif->tif, TIFFTAG_IMAGELENGTH, tif->height);
  TIFFSetField(tif->tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif->tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif->tif, TIFFTAG_SAMPLESPERPIXEL, tif->num_channels);
  TIFFSetField(tif->tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif->tif, TIFFTAG_ROWSPERSTRIP,
	       TIFFDefaultStripSize(tif->tif, 0));
  TIFFSetField(tif->tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
  TIFFSetField(tif->tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

  tif->row_stride = TIFFScanlineSize(tif->tif);
  tif->row = (unsigned char *) _TIFFmalloc(tif->row_stride);

  return 1;
}

static void
pictif_write_scanline(PicTif *tif)
{
  TIFFWriteScanline(tif->tif, tif->row, tif->scanline, 0);
}

static void
pictif_read_scanline(PicTif *tif)
{
  TIFFReadScanline(tif->tif, tif->row, tif->scanline, 0);
}

static void
seek_row_write(PicTif *tif, int y)
{
  while (tif->scanline < y) {
    pictif_write_scanline(tif);
    tif->scanline++;
  }
}

static void
pictif_write_row(void *vp, int y, int x0, int nx, const Pixel1 *buf)
{
  PicTif *tif = (PicTif *) vp;

  if (! tif->read_not_write &&
      1 == tif->num_channels &&
      WRITE_INIT(tif)) {
    int i;

    seek_row_write(tif, y);
    memset(tif->row, 0, tif->row_stride);
    for (i = 0; i < nx; i++)
      tif->row[x0+i] = buf[i];
    pictif_write_scanline(tif);
    tif->scanline++;
  }
}

static void
pictif_write_row_rgba(void *vp, int y, int x0, int nx, const Pixel1_rgba *buf)
{
  PicTif *tif = (PicTif *) vp;

  if (! tif->read_not_write &&
      3 == tif->num_channels &&
      WRITE_INIT(tif)) {
    int i, j;

    seek_row_write(tif, y);
    memset(tif->row, 0, tif->row_stride);
    for (i = 0, j = x0; i < nx; i++, j += 3) {
      tif->row[j+0] = buf[i].r;
      tif->row[j+1] = buf[i].g;
      tif->row[j+2] = buf[i].b;
    }
    pictif_write_scanline(tif);
    tif->scanline++;
  }
}

static int
pictif_get_nchan(void *vp)
{
  PicTif *tif = (PicTif *) vp;

  return tif->num_channels;
}

static void
pictif_get_box(void *vp, int *ox, int *oy, int *dx, int *dy)
{
  PicTif *tif = (PicTif *) vp;

  if (tif->read_not_write || tif->row) {
    *ox = 0;
    *oy = 0;
    *dx = tif->width;
    *dy = tif->height;
  } else {
    *ox = PIXEL_UNDEFINED;
    *oy = PIXEL_UNDEFINED;
    *dx = PIXEL_UNDEFINED;
    *dy = PIXEL_UNDEFINED;
  }    
}

static Pixel1
pictif_read_pixel(void *vp, int x, int y)
{
  fprintf(stderr, "?pictif_read_pixel\n");
  return 0;
}

static void
pictif_read_pixel_rgba(void *vp, int x, int y, Pixel1_rgba *pv)
{
  fprintf(stderr, "?pictif_read_pixel_rgba\n");
}

static void
seek_row_read(PicTif *tif, int y)
{
  while (tif->scanline <= y) {
    pictif_read_scanline(tif);
    tif->scanline++;
  }
}

static void
pictif_read_row(void *vp, int y, int x0, int nx, Pixel1 *buf)
{
  PicTif *tif = (PicTif *) vp;
  
  if (tif->read_not_write && 1 == tif->num_channels) {
    int i;

    seek_row_read(tif, y);
    for (i = 0; i < nx; i++)
      buf[i] = tif->row[x0+i];
  }
}

static void
pictif_read_row_rgba(void *vp, int y, int x0, int nx, Pixel1_rgba *buf)
{
  PicTif *tif = (PicTif *) vp;

  if (tif->read_not_write && 3 == tif->num_channels) {
    int i;

    seek_row_read(tif, y);
    for (i = 0; i < nx; i++) {
      buf[i].r = tif->row[(x0+i)*3+0];
      buf[i].g = tif->row[(x0+i)*3+1];
      buf[i].b = tif->row[(x0+i)*3+2];
    }
  }
}

static int
pictif_next_pic(void *vp)
{
  return 0;
}

static Pic_procs
pictif_procs = {
  pictif_open,
  pictif_close,
  pictif_get_name,

  pictif_clear,
  pictif_clear_rgba,

  pictif_set_nchan,
  pictif_set_box,

  pictif_write_pixel,
  pictif_write_pixel_rgba,
  pictif_write_row,
  pictif_write_row_rgba,

  pictif_get_nchan,
  pictif_get_box,
  pictif_read_pixel,
  pictif_read_pixel_rgba,
  pictif_read_row,
  pictif_read_row_rgba,
  pictif_next_pic
};

Pic pic_tif =
{
  "tif", &pictif_procs
};
