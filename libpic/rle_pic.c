/* pic_iris: hooking iris package into pic package */

static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/rle_pic.c,v 1.1.1.1 1998/12/11 01:15:33 pahvant Exp $ ";

#include "pic.h"
#include "picrle.h"

static Pic_procs pic_rle_procs = {
    (char *(*)())rle_open, rle_close,
    rle_get_name,
    rle_clear, rle_clear_rgba,

    rle_set_nchan, rle_set_box,
    rle_write_pixel, rle_write_pixel_rgba,
    rle_write_row, rle_write_row_rgba,

    rle_get_nchan, rle_get_box,
    rle_read_pixel, rle_read_pixel_rgba,
    rle_read_row, rle_read_row_rgba,
    rle_next_pic,
};

Pic pic_rle = {"rle", &pic_rle_procs};
