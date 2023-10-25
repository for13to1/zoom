/* pnmpad.c - add border to sides of a portable anymap
 ** AJCD 4/9/90
 */

#include <stdio.h>
#include "pnm.h"

int
main(argc, argv)
     int argc;
     char *argv[];
{
   FILE *ifd;
   xel *xelrow, *bgrow, background;
   xelval maxval;
   int rows, cols, newcols, row, col, format;
   int left, right, top, bot, black;
   char *usage = "[-white|-black] [-l#] [-r#] [-t#] [-b#] [pnmfile]";


   pnm_init( &argc, argv );

   black = 1;
   left = right = top = bot = 0;

   while (argc >= 2 && argv[1][0] == '-') {
      if (strcmp(argv[1]+1,"black") == 0) black = 1;
      else if (strcmp(argv[1]+1,"white") == 0) black = 0;
      else switch (argv[1][1]) {
      case 'l':
	 if ((left = atoi(argv[1]+2)) < 0)
	    pm_error("left border too small");
	 break;
      case 'r':
	 if ((right = atoi(argv[1]+2)) < 0)
	    pm_error("right border too small");
	 break;
      case 'b':
	 if ((bot = atoi(argv[1]+2)) < 0)
	    pm_error("bottom border too small");
	 break;
      case 't':
	 if ((top = atoi(argv[1]+2)) < 0)
	    pm_error("top border too small");
	 break;
      default:
	 pm_usage(usage);
      }
      argc--, argv++;
   }

   if (argc > 2)
      pm_usage(usage);
   
   if (argc == 2)
      ifd = pm_openr(argv[1]);
   else
      ifd = stdin;



   pnm_readpnminit(ifd, &cols, &rows, &maxval, &format);
   if (black)
      background = pnm_blackxel(maxval, format);
   else
      background = pnm_whitexel(maxval, format);

   if (cols == 0 || rows == 0) {
      pm_message("empty bitmap");
      left = right = bot = top = 0;
   }
   newcols = cols+left+right;
   xelrow = pnm_allocrow(newcols);
   bgrow = pnm_allocrow(newcols);

   for (col = 0; col < newcols; col++)
      xelrow[col] = bgrow[col] = background;

   pnm_writepnminit(stdout, newcols, rows+top+bot, maxval, format, 0);

   for (row = 0; row < top; row++)
      pnm_writepnmrow(stdout, bgrow, newcols, maxval, format, 0);

   for (row = 0; row < rows; row++) {
      pnm_readpnmrow(ifd, xelrow+left, cols, maxval, format);
      pnm_writepnmrow(stdout, xelrow, newcols, maxval, format, 0);
   }

   for (row = 0; row < bot; row++)
      pnm_writepnmrow(stdout, bgrow, newcols, maxval, format, 0);

   pm_close(ifd);

   exit(0);
}
