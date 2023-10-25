/*
 *  ppmqvga.c - quantize the colors in a pixmap down to a VGA
 *    (256 colors, 6 bits per pixel)
 *
 *  original by Lyle Rains (lrains@netcom.com) as ppmq256 and ppmq256fs
 *  combined, commented, and enhanced by Bill Davidsen (davidsen@crd.ge.com)
 *  changed options parsing to PBM standards - Ingo Wilken 13/Oct/93
*/

#define DUMPCOLORS 0
#define DUMPERRORS 0

#include <stdio.h>
#include <math.h>
#include "ppm.h"
#if 0   /* this is definded by pbmplus.h (brought in by ppm.h) */
#ifdef SYSV
#include <string.h>
#define srandom srand
#define random rand
#else /*SYSV*/
#include <strings.h>
#define strchr index
#define strrchr rindex
#endif /*SYSV*/
#endif

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define RED_BITS   5
#define GREEN_BITS 6
#define BLUE_BITS  5

#define MAX_RED    (1 << RED_BITS)
#define MAX_GREEN  (1 << GREEN_BITS)
#define MAX_BLUE   (1 << BLUE_BITS)

#define MAXWEIGHT  128
#define STDWEIGHT_DIV  (2 << 8)
#define STDWEIGHT_MUL  (2 << 10)
#define COLORS     256
#define GAIN       4

#define BARGRAPH     "__________\b\b\b\b\b\b\b\b\b\b"
#define BARGRAPHLEN  10


typedef int fs_err_array[2][3];

/* prototypes */
void diffuse ARGS((void));
int nearest_color ARGS((register pixel *pP));
void fs_diffuse ARGS((fs_err_array *fs_err, int line, int color, int err));


int color_cube[MAX_RED][MAX_GREEN][MAX_BLUE];

unsigned char clut[COLORS][4];
int erropt[COLORS][4];
enum { red, green, blue, count };
int clutx;

int weight_convert[MAXWEIGHT];
int total_weight, cum_weight[MAX_GREEN];
int rep_weight, rep_threshold;
int r, g, b, dr, dg, db;
int dither = 0, verbose = 0;

pixval maxval;

/*
** 3-D error diffusion dither routine for points in the color cube; used to
** select the representative colors.
*/
void diffuse()
{
  int _7_32nds, _3_32nds, _1_16th;

  if (clutx < COLORS) {
    if (color_cube[r][g][b] > rep_threshold) {

      clut[clutx][red]   = ((2 * r + 1) * (maxval + 1)) / (2 * MAX_RED);
      clut[clutx][green] = ((2 * g + 1) * (maxval + 1)) / (2 * MAX_GREEN);
      clut[clutx][blue]  = ((2 * b + 1) * (maxval + 1)) / (2 * MAX_BLUE);
#if DUMPCOLORS
      if (verbose > 2) {
        /* Dump new color */
        if ((clutx & 3) == 0) {
          fprintf(stderr, "\n  %3d (%2d): ", clutx, rep_threshold);
        }
        fprintf(stderr,
          " (%03d,%03d,%03d)", clut[clutx][red],
          clut[clutx][green], clut[clutx][blue]
        );
      }
#endif
      ++clutx;
      color_cube[r][g][b] -= rep_weight;
    }
    _7_32nds = (7 * color_cube[r][g][b]) / 32;
    _3_32nds = (3 * color_cube[r][g][b]) / 32;
    _1_16th = color_cube[r][g][b] - 3 * (_7_32nds + _3_32nds);
    color_cube[ r  ][ g  ][ b  ]  = 0;
    /* spread error evenly in color space. */
    color_cube[ r  ][ g  ][b+db] += _7_32nds;
    color_cube[ r  ][g+dg][ b  ] += _7_32nds;
    color_cube[r+dr][ g  ][ b  ] += _7_32nds;
    color_cube[ r  ][g+dg][b+db] += _3_32nds;
    color_cube[r+dr][ g  ][b+db] += _3_32nds;
    color_cube[r+dr][g+dg][ b  ] += _3_32nds;
    color_cube[r+dr][g+dg][b+db] += _1_16th;
    /* Conserve the error at edges if possible (which it is, except the last pixel) */
    if (color_cube[r][g][b] != 0) {
      if      (dg != 0)   color_cube[r][g+dg][b] += color_cube[r][g][b];
      else if (dr != 0)   color_cube[r+dr][g][b] += color_cube[r][g][b];
      else if (db != 0)   color_cube[r][g][b+db] += color_cube[r][g][b];
      else fprintf(stderr, "\nlost error term\n");
    }
  }
  color_cube[r][g][b] = -1;
}

/*
** Find representative color nearest to requested color.  Check color cube
** for a cached color index.  If not cached, compute nearest and cache result.
*/
int nearest_color(pP)
  register pixel *pP;
{
  register unsigned char *test;
  register unsigned i;
  unsigned long min_dist_sqd, dist_sqd;
  int nearest;
  int *cache;
  int r, g, b;

  r = ((int)(PPM_GETR(*pP)));
  g = ((int)(PPM_GETG(*pP)));
  b = ((int)(PPM_GETB(*pP)));
  if ((i = maxval + 1) == 256) {
    cache = &(color_cube[r>>(8-RED_BITS)][g>>(8-GREEN_BITS)][b>>(8-BLUE_BITS)]);
  }
  else {
    cache = &(color_cube[(r<<RED_BITS)/i][(g<<GREEN_BITS)/i][(b<<BLUE_BITS)/i]);
  }
  if (*cache >= 0) return *cache;
  min_dist_sqd = ~0;
  for (i = 0; i < COLORS; ++i) {
    test = clut[i];
    dist_sqd = 3 * (r - test[red])   * (r - test[red])
             + 4 * (g - test[green]) * (g - test[green])
             + 2 * (b - test[blue])  * (b - test[blue]);
    if (dist_sqd < min_dist_sqd) {
      nearest = i;
      min_dist_sqd = dist_sqd;
    }
  }
  return (*cache = nearest);
}


/* Errors are carried at FS_SCALE times actual size for accuracy */
#define _7x16ths(x)   ((7 * (x)) / 16)
#define _5x16ths(x)   ((5 * (x)) / 16)
#define _3x16ths(x)   ((3 * (x)) / 16)
#define _1x16th(x)    ((x) / 16)
#define NEXT(line)    (!(line))
#define FS_SCALE      1024


void fs_diffuse (fs_err, line, color, err)
  fs_err_array *fs_err;
  int line, color;
  int err;
{
  fs_err[1] [line]       [color] += _7x16ths(err);
  fs_err[-1][NEXT(line)] [color] += _3x16ths(err);
  fs_err[0] [NEXT(line)] [color] += _5x16ths(err);
  fs_err[1] [NEXT(line)] [color]  = _1x16th(err); /* straight assignment */
}

int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *ifd;
  pixel **pixels;
  register pixel *pP;
  int rows, cols, row;
  register int col;
  int limitcol;
  int i, j, k;
  char *ccP;
  int *errP;
  unsigned char *clutP;
  int nearest;
  fs_err_array *fs_err_lines, *fs_err;
  int fs_line = 0;
  char *usage = "[-dither] [-verbose] [ppmfile]";
  char *pm_progname;
  int argn;

  ppm_init( &argc, argv );

  /* option parsing */
  argn = 1;
  while( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
    if( pm_keymatch(argv[argn], "-dither", 2) ) {
      dither = 1;
    }
    else
    if( pm_keymatch(argv[argn], "-verbose", 2) ) {
      ++verbose;
    }
    /* no quiet option - 'quiet' is now default.  Any -quiet option is
     * swallowed by p?m_init() to silence pm_message().
     * TODO: Change fprintf(stderr,...) calls to pm_message() or pm_error()
     */
    else
      pm_usage(usage);
    ++argn;
  }

  if( argn < argc ) {
    ifd = pm_openr( argv[argn] );
    argn++;
  }
  else
    ifd = stdin;

  if( argn != argc )
    pm_usage(usage);

  if ((pm_progname = strrchr(argv[0], '/')) != NULL) ++pm_progname;
  else pm_progname = argv[0];

  /*
  ** Step 0: read in the image.
  */
  pixels = ppm_readppm( ifd, &cols, &rows, &maxval );
  pm_close( ifd );

  /*
  ** Step 1: catalog the colors into a color cube.
  */
  if (verbose) {
    fprintf( stderr, "%s: building color tables %s", pm_progname, BARGRAPH);
    j = (i = rows / BARGRAPHLEN) / 2;
  }

  /* Count all occurances of each color */
  for (row = 0; row < rows; ++row) {

    if (verbose) {
      if (row > j) {
        putc('*', stderr);
        j += i;
      }
    }

    if (maxval == 255) {
      for (col = 0, pP = pixels[row]; col < cols; ++col, ++pP) {
        ++(color_cube[PPM_GETR(*pP) / (256 / MAX_RED)]
                     [PPM_GETG(*pP) / (256 / MAX_GREEN)]
                     [PPM_GETB(*pP) / (256 / MAX_BLUE)]
        );
      }
    }
    else {
      for (col = 0, pP = pixels[row]; col < cols; ++col, ++pP) {
        r = (PPM_GETR(*pP) * MAX_RED)  / (maxval + 1);
        g = (PPM_GETG(*pP) * MAX_GREEN)/ (maxval + 1);
        b = (PPM_GETB(*pP) * MAX_BLUE) / (maxval + 1);
        ++(color_cube[r][g][b]);
      }
    }
  }

  /*
  ** Step 2: Determine weight of each color and the weight of a representative color.
  */
  /* Initialize logarithmic weighing table */
  for (i = 2; i < MAXWEIGHT; ++i) {
    weight_convert[i] = (int) (100.0 * log((double)(i)));
  }

  k = rows * cols;
  if ((k /= STDWEIGHT_DIV) == 0) k = 1;
  total_weight = i = 0;
  for (g = 0; g < MAX_GREEN; ++g) {
    for (r = 0; r < MAX_RED; ++r) {
      for (b = 0; b < MAX_BLUE; ++b) {
        register int weight;
        /* Normalize the weights, independent of picture size. */
        weight = color_cube[r][g][b] * STDWEIGHT_MUL;
        weight /= k;
        if (weight) ++i;
        if (weight >= MAXWEIGHT) weight = MAXWEIGHT - 1;
        total_weight += (color_cube[r][g][b] = weight_convert[weight]);
      }
    }
    cum_weight[g] = total_weight;
  }
  rep_weight = total_weight / COLORS;

  if (verbose) {
    putc('\n', stderr);
    if (verbose > 1) {
      fprintf(stderr, "  found %d colors with total weight %d\n",
        i, total_weight);
      fprintf(stderr, "  avg weight for colors used  = %7.2f\n",
        (float)total_weight/i);
      fprintf(stderr, "  avg weight for all colors   = %7.2f\n",
        (float)total_weight/(MAX_RED * MAX_GREEN * MAX_BLUE));
      fprintf(stderr, "  avg weight for final colors = %4d\n", rep_weight);
    }
    fprintf( stderr, "%s: selecting new colors...", pm_progname);
  }

  /* Magic foo-foo dust here.  What IS the correct way to select threshold? */
  rep_threshold = total_weight * (28 + 110000/i) / 95000;

  /*
  ** Step 3: Do a 3-D error diffusion dither on the data in the color cube
  ** to select the representative colors.  Do the dither back and forth in
  ** such a manner that all the error is conserved (none lost at the edges).
  */
#if !DUMPCOLORS
  if (verbose > 2) {
    fprintf(stderr, "\nrep_threshold: %d", rep_threshold);
  }
#endif
  dg = 1;
  for (g = 0; g < MAX_GREEN; ++g) {
    dr = 1;
    for (r = 0; r < MAX_RED; ++r) {
      db = 1;
      for (b = 0; b < MAX_BLUE - 1; ++b) diffuse();
      db = 0;
      diffuse();
      ++b;
      if (++r == MAX_RED - 1) dr = 0;
      db = -1;
      while (--b > 0) diffuse();
      db = 0;
      diffuse();
    }
    /* Modify threshold to keep rep points proportionally distribited */
    if ((j = clutx - (COLORS * cum_weight[g]) / total_weight) != 0) {
      rep_threshold += j * GAIN;
#if !DUMPCOLORS
      if (verbose > 2) {
        fprintf(stderr, " %d", rep_threshold);
      }
#endif
    }
    if (++g == MAX_GREEN - 1) dg = 0;
    dr = -1;
    while (r-- > 0) {
      db = 1;
      for (b = 0; b < MAX_BLUE - 1; ++b) diffuse();
      db = 0;
      diffuse();
      ++b;
      if (--r == 0) dr = 0;
      db = -1;
      while (--b > 0) diffuse();
      db = 0;
      diffuse();
    }
    /* Modify threshold to keep rep points proportionally distribited */
    if ((j = clutx - (COLORS * cum_weight[g]) / total_weight) != 0) {
      rep_threshold += j * GAIN;
#if !DUMPCOLORS
      if (verbose > 2) {
        fprintf(stderr, " %d", rep_threshold);
      }
#endif
    }
  }

  /*
  ** Step 4: check the error associted with the use of each color, and
  ** change the value of the color to minimize the error.
  */
  if (verbose) {
    fprintf( stderr, "\n%s: Reducing errors in the color map %s",
      pm_progname, BARGRAPH);
    j = (i = rows / BARGRAPHLEN) / 2;
  }
  for (row = 0; row < rows; ++row) {

    if (verbose) {
      if (row > j) {
        putc('*', stderr);
        j += i;
      }
    }

    pP = pixels[row];
    for (col = 0; col < cols; ++col) {
      nearest = nearest_color(pP);
      errP = erropt[nearest]; clutP = clut[nearest];
      errP[red]   += PPM_GETR(*pP) - clutP[red];
      errP[green] += PPM_GETG(*pP) - clutP[green];
      errP[blue]  += PPM_GETB(*pP) - clutP[blue];
      ++errP[count];
      ++pP;
    }
  }
#if DUMPERRORS
    if (verbose) {
      fprintf( stderr, "\n  Color    Red Err  Green Err   Blue Err  Count");
    }
#endif
  for (i = 0; i < COLORS; ++i) {
    clutP = clut[i]; errP = erropt[i];
    j = errP[count];
    if (j > 0) {
      j *= 4;
#if DUMPERRORS
      if (verbose) {
        fprintf( stderr, "\n   %4d %10d %10d %10d %6d",
          i, errP[red]/j, errP[green]/j, errP[blue]/j, j);
      }
#endif
      clutP[red]   += (errP[red]   / j) * 4;
      clutP[green] += (errP[green] / j) * 4;
      clutP[blue]  += (errP[blue]  / j) * 4;
    }
  }
  /* Reset the color cache. */
  for (r = 0; r < MAX_RED; ++r)
    for (g = 0; g < MAX_GREEN; ++g)
      for (b = 0; b < MAX_BLUE; ++b)
        color_cube[r][g][b] = -1;


  /*
  ** Step 5: map the colors in the image to their closest match in the
  ** new colormap, and write 'em out.
  */
  if (verbose) {
    fprintf( stderr, "\n%s: Mapping image to new colors %s",
      pm_progname, BARGRAPH);
    j = (i = rows / BARGRAPHLEN) / 2;
  }
  ppm_writeppminit( stdout, cols, rows, maxval, 0 );

  if (dither) {
    fs_err_lines = (fs_err_array *) calloc((cols + 2), sizeof(fs_err_array));

    if (fs_err_lines == NULL) {
      fprintf(stderr, "\n%s: can't allocate Floyd-Steinberg error array.\n",
        pm_progname);
      exit(1);
    }
  }

  for (row = 0; row < rows; ++row) {

    if (verbose) {
      if (row > j) {
        putc('*', stderr);
        j += i;
      }
    }

    if (dither) {
      fs_err = fs_err_lines + 1;
      fs_err[0][NEXT(fs_line)][red]   = 0;
      fs_err[0][NEXT(fs_line)][green] = 0;
      fs_err[0][NEXT(fs_line)][blue]  = 0;
    }

    pP = pixels[row];
    for (col = 0; col < cols; ++col) {

      if (dither) {
        r = FS_SCALE * (int)(PPM_GETR(*pP)) + fs_err[0][fs_line][red];
        if (r > FS_SCALE * (int)maxval) r = FS_SCALE * (int)maxval;
        if (r < 0) r = 0;
        g = FS_SCALE * (int)(PPM_GETG(*pP)) + fs_err[0][fs_line][green];
        if (g > FS_SCALE * (int)maxval) g = FS_SCALE * (int)maxval;
        if (g < 0) g = 0;
        b = FS_SCALE * (int)(PPM_GETB(*pP)) + fs_err[0][fs_line][blue];
        if (b > FS_SCALE * (int)maxval) b = FS_SCALE * (int)maxval;
        if (b < 0) b = 0;

        PPM_ASSIGN(
          *pP, (pixval)(r/FS_SCALE), (pixval)(g/FS_SCALE),
          (pixval)(b/FS_SCALE)
        );
      }
      nearest = nearest_color(pP);
      if (nearest < 0 || nearest > COLORS - 1) {
        fprintf(stderr, "  nearest = %d; out of range\n", nearest);
        exit(1);
      }
      clutP = clut[nearest];

      if (dither) {
        r -= FS_SCALE * (int)clutP[red];
        g -= FS_SCALE * (int)clutP[green];
        b -= FS_SCALE * (int)clutP[blue];

        fs_diffuse(fs_err, fs_line, red,   r);
        fs_diffuse(fs_err, fs_line, green, g);
        fs_diffuse(fs_err, fs_line, blue,  b);
      }

      PPM_ASSIGN( *pP, clutP[red], clutP[green], clutP[blue]);

      if (dither) ++fs_err;
      ++pP;
    }

    ppm_writeppmrow( stdout, pixels[row], cols, maxval, 0 );

    fs_line = NEXT(fs_line);
  }
  if (verbose) {
    fprintf( stderr, "\n%s: done.\n", pm_progname);
  }

  exit(0);
}
