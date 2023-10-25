static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/pic_all.c,v 1.2 1998/12/13 23:07:13 pahvant Exp $";
#include <pic.h>

extern Pic pic_jpg;
extern Pic pic_pnm;
extern Pic pic_dump;
extern Pic pic_png;
extern Pic pic_gif;
extern Pic pic_tif;
extern Pic pic_bmp;

/*
 * A pic_list for those programs that want everything.
 * If the application doesn't define space for pic_list then the
 * linker will grab this.
 */

Pic *pic_list[PIC_LISTMAX] = {
  &pic_dump,
  &pic_pnm,
  &pic_jpg,
  &pic_png,
  &pic_gif,
  &pic_tif,
  &pic_bmp,
  0
};
