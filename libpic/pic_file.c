static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/pic_file.c,v 1.2 1998/12/13 23:07:14 pahvant Exp $";

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <simple.h>
#include "pic.h"
#include "swap.h"

typedef enum {DUNNO, SHORT, LONG} Magic_type;
typedef enum {NA, myLITTLE_ENDIAN, myBIG_ENDIAN} Magic_byteorder;

#if defined(vax) || defined(WIN32)
#   define MACHINE_BYTEORDER myLITTLE_ENDIAN
#else
#   define MACHINE_BYTEORDER myBIG_ENDIAN
#endif

int amg_recog(), dat_recog(), ais_recog(), rad_recog();

typedef struct {
    char *dev;			/* device name */
    char *suffix;		/* file suffix */
    long magic;			/* magic number */
    Magic_type type;		/* type of magic number (DUNNO|SHORT|LONG) */
    Magic_byteorder byteorder;	/* NA, myLITTLE_ENDIAN, or myBIG_ENDIAN */
    int (*recogproc)();		/* procedure to recognize, if needed */
} Dev_info;

static Dev_info dev[] = {
 /*  DEV      SUFFIX	MAGIC#		TYPE	BYTEORDER	     RECOGPROC */
    "bmp",    "bmp",    0x424d,     SHORT,  myBIG_ENDIAN,    0,
    "jpg",    "jpg",	0,          DUNNO,  NA,              0,
    "jpg",    "jpeg",	0,          DUNNO,  NA,              0,
    "jpg",    "pjpg",	0,          DUNNO,  NA,              0,
    "jpg",    "pjpeg",	0,          DUNNO,  NA,              0,
    "png",    "png",    0,          DUNNO,  NA,              0,
    "gif",    "gif",    0,          DUNNO,  NA,              0,
    "tif",    "tif",    0,          DUNNO,  NA,              0,
    "tif",    "tiff",   0,          DUNNO,  NA,              0,
    "pnm",    "pbm",    0,          DUNNO,  NA,              0,
    "pnm",    "pgm",    0,          DUNNO,  NA,              0,
    "pnm",    "ppm",    0,          DUNNO,  NA,              0,
    "pnm",    "pnm",    0,          DUNNO,  NA,              0,
    "dump",   "dump",   0x5088,     SHORT,  myBIG_ENDIAN,    0,
    "iris",   "iris",   0,          DUNNO,  NA,              0,
    "rle",    "rle",    0xcc52,     SHORT,  myLITTLE_ENDIAN, 0,
    "rle",    "Z",      0x1f9d,     SHORT,  NA,              0,
    "rle",    "z",      0x1f8b,     SHORT,  NA,              0,
};
#define NDEV (sizeof dev / sizeof dev[0])

/*
 * pic_file_dev: given file name, try to determine its device type.
 * First examine the file (if it exists);
 * then try special type-specific recognizers,
 * if those fail look at file suffix.
 * Returns 0 if unrecognized.
 */

char *pic_file_dev(file)
char *file;
{
    char *suffix;
    union {
	unsigned short s;
	long l;
    } u, v;
    Dev_info *d;
    FILE *fp;
    struct stat sb;

    /* first try examining the file */
    if ((fp = fopen(file, "r")) != NULL && fstat(fileno(fp), &sb) == 0 &&
	(sb.st_mode&S_IFMT) == S_IFREG) {
	    if (fread(&u, sizeof u, 1, fp) != 1)
		u.l = 0;			/* no magic number */
	    fclose(fp);
	    for (d=dev; d<dev+NDEV; d++) {
		if (d->byteorder != NA) {	/* check file's magic number */
		    if (d->type == SHORT) {	/* short magic number */
			v.s = u.s;
			/* if file byte order diff. from machine's then swap: */
			if (d->byteorder != MACHINE_BYTEORDER)
			    swap_short(&v.s);
			if (v.s==d->magic) return d->dev;
		    }
		    else {			/* long magic number */
			v.l = u.l;
			/* if file byte order diff. from machine's then swap: */
			if (d->byteorder != MACHINE_BYTEORDER)
			    swap_long(&v.l);
			if (v.l==d->magic) return d->dev;
		    }
		}
	    }
    }
    
    /* if magic number didn't identify, try type-specific recognizers: */
    for (d=dev; d<dev+NDEV; d++)
	if (d->recogproc)		/* call device's recognition proc */
	    if ((*d->recogproc)(file, d)) return d->dev;

    /* if we couldn't recognize by file contents, try file name */
    suffix = strrchr(file, '.');
    if (suffix) suffix++;
    else {
	suffix = strrchr(file, '/');
	suffix = suffix ? suffix+1 : file;
    }
    for (d=dev; d<dev+NDEV; d++)
	if (str_eq(d->suffix, suffix)) return d->dev;

    /* else failure */
    return 0;
}
