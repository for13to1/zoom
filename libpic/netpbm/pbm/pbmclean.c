/* pbmclean.c - pixel cleaning. Remove pixel if less than n connected
 *              identical neighbours, n=1 default.
 * AJCD 20/9/90
 */

#include <stdio.h>
#include "pbm.h"

/* prototypes */
void nextrow ARGS((FILE *ifd, int row));

#define PBM_INVERT(p) ((p) == PBM_WHITE ? PBM_BLACK : PBM_WHITE)

/* input bitmap size and storage */
int rows, columns, format ;
bit *inrow[3] ;

#define thisrow (1)

/* compass directions from west clockwise */
int xd[] = { -1, -1,  0,  1, 1, 1, 0, -1 } ;
int yd[] = {  0, -1, -1, -1, 0, 1, 1,  1 } ;

/* get a new row
 */

void nextrow(ifd, row)
     FILE *ifd;
     int row;
{
   bit *shuffle = inrow[0] ;
   inrow[0] = inrow[1];
   inrow[1] = inrow[2];
   inrow[2] = shuffle ;
   if (row < rows) {
      if (shuffle == NULL)
         inrow[2] = shuffle = pbm_allocrow(columns);
      pbm_readpbmrow(ifd, inrow[2], columns, format) ;
   } else inrow[2] = NULL; /* discard storage */

}

int
main(argc, argv)
     int argc;
     char *argv[];
{
   FILE *ifd;
   register bit *outrow;
   register int row, col, i;
   int connect ;


   pbm_init( &argc, argv );

   if (argc > 3)
      pm_usage("[-connect] [pbmfile]");

   if (argv[1][0] == '-') {
      connect = atoi(argv[1]+1);
      argv++; argc--;
   }
   else connect = 1;

   if (argc == 2)
      ifd = pm_openr(argv[1]);
   else
      ifd = stdin ;

   inrow[0] = inrow[1] = inrow[2] = NULL;
   pbm_readpbminit(ifd, &columns, &rows, &format) ;

   outrow = pbm_allocrow(columns) ;

   pbm_writepbminit(stdout, columns, rows, 0) ;

   nextrow(ifd, 0);
   for (row = 0; row < rows; row++) {
      nextrow(ifd, row+1);
      for (col = 0; col < columns; col++) {
         int point = inrow[thisrow][col];
         int joined = 0 ;
         for (i = 0; i < 8; i++) {
            int x = col + xd[i] ;
            int y = thisrow + yd[i] ;
            if (x < 0 || x >= columns) {
               if (point == PBM_WHITE) joined++;
            }
            else if (inrow[y] && inrow[y][x] == point) joined++ ;
         }
         outrow[col] = (joined < connect) ? PBM_INVERT(point) : point;
      }
      pbm_writepbmrow(stdout, outrow, columns, 0) ;
   }
   pm_close(ifd);
   exit(0);
}
