#include <simple.h>
#include <pic.h>
#include <string.h>
#include <gif.h>

struct tagGif {
  char *name;
  FILE *file;
  int read_not_write;
  int format;
  int scanline;
  int width;
  int height;
  int num_channels;
  GIFData *image;
  GIFStream *gif;
};
typedef struct tagGif Gif;

typedef struct {
  int len;
  unsigned char (*rgb)[3];
} gif_colormap_t;

static gif_colormap_t
colormap(GIFStream *gif, GIFData *image)
{
  gif_colormap_t result;

  memset(&result, 0, sizeof(result));
  if (image->data.image.cmapSize) {
    result.len = image->data.image.cmapSize;
    result.rgb = &image->data.image.cmapData[0];
  } else if (gif->cmapSize) {
    result.len = gif->cmapSize;
    result.rgb = &gif->cmapData[0];
  }

  return result;
}

static int
bw_colormap_p(GIFStream *gif, GIFData *image)
{
  int i;
  gif_colormap_t cmap = colormap(gif, image);

  for (i = 0; i < cmap.len; i++)
    if ((cmap.rgb[i][0] ^ cmap.rgb[i][1]) || (cmap.rgb[i][0] ^ cmap.rgb[i][2]))
      return 0;

  return 1;
}

static GIFData *
next_gif_image(GIFData *cur)
{
  while (cur && gif_image != cur->type)
    cur = cur->next;

  return cur;
}

static void *
gif_open(const char *filename, const char *mode)
{
  Gif *gif = NULL;
  FILE *try;

  /* don't support writing of GIFs */
  if ('w' == mode[0])
    return NULL;

  if (str_eq(filename, "-.gif"))
    try = fdopen(dup(fileno(stdin)), "rb");
  else
    try = fopen(filename, "rb");
  
  if (try) {
    ALLOC(gif, Gif, 1);
    memset(gif, 0, sizeof(Gif));
    gif->file = try;
    gif->name = strdup(filename);
    if ('r' == mode[0]) {
      gif->read_not_write = 1;
      /* read GIF header and determine image size */
      gif->gif = GIFReadFP(gif->file);
      gif->image = next_gif_image(gif->gif->data);
      gif->num_channels = bw_colormap_p(gif->gif, gif->image) ? 1 : 3;
      gif->width = gif->gif->width;
      gif->height = gif->gif->height;
    } else {
      gif->read_not_write = 0;
      /* initialize header for writing */
    }
  }

  return gif;
}

static void
gif_close(void *p)
{
  Gif *gif = (Gif *) p;

  if (gif->read_not_write) {
    /* done reading */
  } else {
    /* done writing */
  }

  assert(gif->name);
  free(gif->name);
  assert(gif->file);
  fclose(gif->file);
  free(gif);
}

static char *
gif_get_name(void *p)
{
  Gif *gif = (Gif *) p;
  return gif->name;
}

static void
gif_clear(void *vp, Pixel1 pv)
{
  /* hmm... ignore? */
}

static void
gif_clear_rgba(void *vp, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  /* hmm... ignore? */
}

static void
gif_set_nchan(void *vp, int nchan)
{
  Gif *gif = (Gif *) vp;
  
  if (! gif->read_not_write) {
    gif->num_channels = nchan;
  }
}

static void
gif_set_box(void *vp, int ox, int oy, int dx, int dy)
{
  Gif *gif = (Gif *) vp;
  if (! gif->read_not_write) {
    gif->width = ox + dx;
    gif->height = oy + dy;
  }    
}

static void
gif_write_pixel(void *vp, int x, int y, Pixel1 pv)
{
  fprintf(stderr, "?gif_write_pixel\n");
  /* output scanlines until we reach scanline y */
  /* set pixel x of scanline to pv */
}

static void
gif_write_pixel_rgba(void *vp, int x, int y,
		     Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
  fprintf(stderr, "?gif_write_pixel_rgba\n");
}

static void
gif_write_row(void *vp, int y, int x0, int nx, const Pixel1 *buf)
{
  fprintf(stderr, "?GIF output not supported.\n");
}

static void
gif_write_row_rgba(void *vp, int y, int x0, int nx, const Pixel1_rgba *buf)
{
  fprintf(stderr, "?GIF output not supported.\n");
}

static int
gif_get_nchan(void *vp)
{
  Gif *gif = (Gif *) vp;
  return gif->num_channels;
}

static void
gif_get_box(void *vp, int *ox, int *oy, int *dx, int *dy)
{
  Gif *gif = (Gif *) vp;

  *ox = 0;
  *oy = 0;
  *dx = gif->width;
  *dy = gif->height;
}

static Pixel1
gif_read_pixel(void *vp, int x, int y)
{
  fprintf(stderr, "?gif_read_pixel\n");
  return 0;
}

static void
gif_read_pixel_rgba(void *vp, int x, int y, Pixel1_rgba *pv)
{
  fprintf(stderr, "?gif_read_pixel_rgba\n");
}

static void
gif_read_row(void *vp, int y, int x0, int nx, Pixel1 *buf)
{
  Gif *gif = (Gif *) vp;
  
  if (gif->read_not_write && 1 == gif->num_channels) {
    int i;
    for (i = 0; i < nx; i++) {
      buf[i] = 0;
    }
  }
}

static void
gif_read_row_rgba(void *vp, int y, int x0, int nx, Pixel1_rgba *buf)
{
  Gif *gif = (Gif *) vp;

  if (gif->read_not_write && 3 == gif->num_channels) {
    int i;
    gif_colormap_t cmap = colormap(gif->gif, gif->image);
    unsigned char *pixels =
      gif->image->data.image.data + y*gif->image->width + x0;

    for (i = 0; i < nx; i++) {
      buf[i].r = cmap.rgb[pixels[i]][0];
      buf[i].g = cmap.rgb[pixels[i]][1];
      buf[i].b = cmap.rgb[pixels[i]][2];
    }
  }
}

static int
gif_next_pic(void *vp)
{
  Gif *gif = (Gif *) vp;

  gif->image = next_gif_image(gif->image->next);

  return gif->image != NULL;
}

static Pic_procs
pic_gif_procs = {
  gif_open,
  gif_close,
  gif_get_name,

  gif_clear,
  gif_clear_rgba,

  gif_set_nchan,
  gif_set_box,

  gif_write_pixel,
  gif_write_pixel_rgba,
  gif_write_row,
  gif_write_row_rgba,

  gif_get_nchan,
  gif_get_box,
  gif_read_pixel,
  gif_read_pixel_rgba,
  gif_read_row,
  gif_read_row_rgba,
  gif_next_pic
};

Pic pic_gif =
{
  "gif", &pic_gif_procs
};
