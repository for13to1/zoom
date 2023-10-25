/* pic_iris: hooking iris package into pic package */

static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/iris_pic.c,v 1.1.1.1 1998/12/11 01:15:32 pahvant Exp $ ";

#include "pic.h"
#include "iris.h"

static Pic_procs pic_iris_procs = {
    (char *(*)())iris_open, iris_close,
    iris_get_name,
    iris_clear, iris_clear_rgba,

    iris_set_nchan, iris_set_box,
    iris_write_pixel, iris_write_pixel_rgba,
    iris_write_row, iris_write_row_rgba,

    iris_get_nchan, iris_get_box,
    iris_read_pixel, iris_read_pixel_rgba,
    iris_read_row, iris_read_row_rgba,
};

Pic pic_iris = {"iris", &pic_iris_procs};
