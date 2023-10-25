/* pbmtopgm.c - convert bitmap to greymap by totalling pixels over sample area
 * AJCD 12/12/90
 */

#include <stdio.h>
#include "pgm.h"

int
main(argc, argv)
     int argc;
     char *argv[];
{
   register gray *outrow, maxval;
   register int right, left, down, up;
   register bit **inbits;
   int rows, cols;
   FILE *ifd;
   int col, row, width, height;
   char *usage = "<w> <h> [pbmfile]";
   

   pgm_init( &argc, argv );

   if (argc > 4 || argc < 3)
      pm_usage(usage);

   width = atoi(argv[1]);
   height = atoi(argv[2]);
   if (width < 1 || height < 1)
      pm_error("width and height must be > 0");
   left=width/2; right=width-left;
   up=width/2; down=height-up;

   if (argc == 4)
      ifd = pm_openr(argv[3]);
   else
      ifd = stdin ;

   inbits = pbm_readpbm(ifd, &cols, &rows) ;

   if (width > cols || height > rows)
      pm_error("sample size greater than bitmap size");

   outrow = pgm_allocrow(cols) ;
   maxval = width*height;
   pgm_writepgminit(stdout, cols, rows, maxval, 0) ;

   for (row = 0; row < rows; row++) {
      int t = (row > up) ? (row-up) : 0;
      int b = (row+down < rows) ? (row+down) : rows;
      int onv = height - (t-row+up) - (row+down-b);
      for (col = 0; col < cols; col++) {
	 int l = (col > left) ? (col-left) : 0;
	 int r = (col+right < cols) ? (col+right) : cols;
	 int onh = width - (l-col+left) - (col+right-r);
	 int value = 0, x, y;
	 for (x = l; x < r; x++)
	    for (y = t; y < b; y++)
	       if (inbits[y][x] == PBM_WHITE) value++;
	 outrow[col] = maxval*value/(onh*onv);
      }
      pgm_writepgmrow(stdout, outrow, cols, maxval, 0) ;
   }
   pm_close(ifd);
   exit(0);
}
