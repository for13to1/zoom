#include <simple.h>
#include <pic.h>
#include <string.h>
#include <jpeglib.h>

struct tagJpg {
  struct jpeg_error_mgr jerr;
  char *name;
  FILE *file;
  int read_not_write;
  struct jpeg_decompress_struct in;
  struct jpeg_compress_struct out;
  int format;
  int scanline;
  int row_stride;
  JSAMPARRAY row;
};
typedef struct tagJpg Jpg;

static void *
jpg_open(const char *filename, const char *mode)
{
  Jpg *jpg = NULL;
  FILE *try;

  if ('r' == mode[0] && str_eq(filename, "-.jpg"))
    try = fdopen(dup(fileno(stdin)), "rb");
  else if ('w' == mode[0] && str_eq(filename, "-.jpg"))
    try = fdopen(dup(fileno(stdout)), "wb");
  else
    try = fopen(filename, mode);
  
  if (try) {
    ALLOC(jpg, Jpg, 1);
    memset(jpg, 0, sizeof(Jpg));
    jpg->file = try;
    jpg->name = strdup(filename);
    if ('r' == mode[0]) {
      jpg->read_not_write = 1;
      jpg->in.err = jpeg_std_error(&jpg->jerr);
      jpeg_create_decompress(&jpg->in);
      jpeg_stdio_src(&jpg->in, jpg->file);
      jpeg_read_header(&jpg->in, TRUE);
      jpeg_start_decompress(&jpg->in);
      jpg->row_stride = jpg->in.output_width * jpg->in.output_components;
      jpg->row = (*jpg->in.mem->alloc_sarray)((j_common_ptr) &jpg->in,
					      JPOOL_IMAGE, jpg->row_stride, 1);
    } else {
      jpg->read_not_write = 0;
      jpg->out.err = jpeg_std_error(&jpg->jerr);
      jpeg_create_compress(&jpg->out);
      jpeg_stdio_dest(&jpg->out, jpg->file);
      jpg->out.in_color_space = JCS_RGB;
    }
  }

  return jpg;
}

static void
jpg_close(void *p)
{
  Jpg *jpg = (Jpg *) p;

  if (jpg->read_not_write) {
    jpeg_finish_decompress(&jpg->in);
    jpeg_destroy_decompress(&jpg->in);
  } else {
    jpeg_finish_compress(&jpg->out);
    jpeg_destroy_compress(&jpg->out);
  }

  assert(jpg->name);
  free(jpg->name);
  assert(jpg->file);
  fclose(jpg->file);
  free(jpg);
}

static char *
jpg_get_name(void *p)
{
  Jpg *jpg = (Jpg *) p;
  return jpg->name;
}

static void
jpg_clear(void *vp, Pixel1 pv)
{
  /* hmm... ignore? */
}

static void
jpg_clear_rgba(void *vp, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  /* hmm... ignore? */
}

static void
jpg_set_nchan(void *vp, int nchan)
{
  Jpg *jpg = (Jpg *) vp;
  
  if (! jpg->read_not_write) {
    jpg->out.input_components = nchan;
  }
}

static void
jpg_set_box(void *vp, int ox, int oy, int dx, int dy)
{
  Jpg *jpg = (Jpg *) vp;
  if (! jpg->read_not_write) {
    jpg->out.image_width = ox + dx;
    jpg->out.image_height = oy + dy;
  }    
}

static void
jpg_write_pixel(void *vp, int x, int y, Pixel1 pv)
{
  fprintf(stderr, "?jpg_write_pixel\n");
  /* output scanlines until we reach scanline y */
  /* set pixel x of scanline to pv */
}

static void
jpg_write_pixel_rgba(void *vp, int x, int y,
		     Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  fprintf(stderr, "?jpg_write_pixel_rgba\n");
}

#define WRITE_INIT(jpg_) ((jpg_)->scanline || write_init(jpg_))
static int
write_init(Jpg *jpg)
{
  jpg->row_stride = jpg->out.input_components * jpg->out.image_width;
  jpeg_set_defaults(&jpg->out);
  jpg->row = (*jpg->out.mem->alloc_sarray)((j_common_ptr) &jpg->out,
					   JPOOL_IMAGE, jpg->row_stride, 1);
  jpeg_start_compress(&jpg->out, TRUE);

  return 1;
}

static void
seek_row_write(Jpg *jpg, int y)
{
  while (jpg->scanline < y) {
    jpeg_write_scanlines(&jpg->out, jpg->row, 1);
    jpg->scanline++;
  }
}

static void
jpg_write_row(void *vp, int y, int x0, int nx, const Pixel1 *buf)
{
  Jpg *jpg = (Jpg *) vp;

  if (! jpg->read_not_write &&
      1 == jpg->out.input_components &&
      WRITE_INIT(jpg)) {
    int i;

    seek_row_write(jpg, y);
    memset(jpg->row[0], 0, jpg->row_stride);
    for (i = 0; i < nx; i++)
      jpg->row[0][x0+i] = buf[i];
    jpeg_write_scanlines(&jpg->out, jpg->row, 1);
    jpg->scanline++;
  }
}

static void
jpg_write_row_rgba(void *vp, int y, int x0, int nx, const Pixel1_rgba *buf)
{
  Jpg *jpg = (Jpg *) vp;

  if (! jpg->read_not_write &&
      3 == jpg->out.input_components &&
      WRITE_INIT(jpg)) {
    int i, j;

    seek_row_write(jpg, y);
    memset(jpg->row[0], 0, jpg->row_stride);
    for (i = 0, j = x0; i < nx; i++, j += 3) {
      jpg->row[0][j+0] = buf[i].r;
      jpg->row[0][j+1] = buf[i].g;
      jpg->row[0][j+2] = buf[i].b;
    }
    jpeg_write_scanlines(&jpg->out, jpg->row, 1);
    jpg->scanline++;
  }
}

static int
jpg_get_nchan(void *vp)
{
  Jpg *jpg = (Jpg *) vp;
  return jpg->read_not_write ?
    jpg->in.output_components : jpg->out.input_components;
}

static void
jpg_get_box(void *vp, int *ox, int *oy, int *dx, int *dy)
{
  Jpg *jpg = (Jpg *) vp;

  if (jpg->read_not_write) {
    *ox = 0;
    *oy = 0;
    *dx = jpg->in.image_width;
    *dy = jpg->in.image_height;
  } else if (jpg->scanline) {
    *ox = 0;
    *oy = 0;
    *dx = jpg->out.image_width;
    *dy = jpg->out.image_height;
  } else {
    *ox = PIXEL_UNDEFINED;
    *oy = PIXEL_UNDEFINED;
    *dx = PIXEL_UNDEFINED;
    *dy = PIXEL_UNDEFINED;
  }
}

static Pixel1
jpg_read_pixel(void *vp, int x, int y)
{
  fprintf(stderr, "?jpg_read_pixel\n");
  return 0;
}

static void
jpg_read_pixel_rgba(void *vp, int x, int y, Pixel1_rgba *pv)
{
  fprintf(stderr, "?jpg_read_pixel_rgba\n");
}

static void
seek_row_read(Jpg *jpg, int y)
{
  while (jpg->scanline <= y) {
    jpeg_read_scanlines(&jpg->in, jpg->row, 1);
    jpg->scanline++;
  }
}

static void
jpg_read_row(void *vp, int y, int x0, int nx, Pixel1 *buf)
{
  Jpg *jpg = (Jpg *) vp;
  
  if (jpg->read_not_write && 1 == jpg->in.output_components) {
    int i;

    seek_row_read(jpg, y);
    for (i = 0; i < nx; i++)
      buf[i] = jpg->row[0][x0+i];
  }
}

static void
jpg_read_row_rgba(void *vp, int y, int x0, int nx, Pixel1_rgba *buf)
{
  Jpg *jpg = (Jpg *) vp;

  if (jpg->read_not_write && 3 == jpg->in.output_components) {
    int i;

    seek_row_read(jpg, y);
    if (1 == jpg->in.output_components)
      for (i = 0; i < nx; i++) {
	const int luminance = jpg->row[0][x0+i];
	buf[i].r = luminance;
	buf[i].g = luminance;
	buf[i].b = luminance;
      }
    else
      for (i = 0; i < nx; i++) {
	buf[i].r = jpg->row[0][(x0+i)*3+0];
	buf[i].g = jpg->row[0][(x0+i)*3+1];
	buf[i].b = jpg->row[0][(x0+i)*3+2];
      }
  }
}

static int
jpg_next_pic(void *vp)
{
  return 0;
}

static Pic_procs
pic_jpg_procs = {
  jpg_open,
  jpg_close,
  jpg_get_name,

  jpg_clear,
  jpg_clear_rgba,

  jpg_set_nchan,
  jpg_set_box,

  jpg_write_pixel,
  jpg_write_pixel_rgba,
  jpg_write_row,
  jpg_write_row_rgba,

  jpg_get_nchan,
  jpg_get_box,
  jpg_read_pixel,
  jpg_read_pixel_rgba,
  jpg_read_row,
  jpg_read_row_rgba,
  jpg_next_pic
};

Pic pic_jpg =
{
  "jpg", &pic_jpg_procs
};
