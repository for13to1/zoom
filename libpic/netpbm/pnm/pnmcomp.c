/* +-------------------------------------------------------------------+ */
/* | Copyright 1992, David Koblas.                                     | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */

#include "pnm.h"
#include "pgm.h"
#include "ppm.h"

static gray     **alpha = NULL;
static int      alphaCols, alphaRows;
static xelval   alphaMax;
static pixel    **image = NULL;
static int      imageCols, imageRows, imageType;
static xelval   imageMax;
static int      InvertFlag = 0;

static char     *usage =
        "[-invert] [-xoff N] [-yoff N] [-alpha file] overlay [image] [output]";

/* prototypes */
void composite ARGS((int xoff, int yoff, FILE *ifp, FILE *ofp));

int
main(argc, argv)
int     argc;
char    *argv[];
{
        int     xoff = 0, yoff = 0;
        FILE    *ifp, *ofp, *fp;
        int     argn = 1;


        pnm_init(&argc, argv);

        while (argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0') {
                if (pm_keymatch(argv[argn], "-xoff", 2)) {
                        if (argn == argc ||
                            sscanf(argv[++argn], "%d", &xoff) != 1)
                                pm_usage(usage);
                } else if (pm_keymatch(argv[argn], "-yoff", 2)) {
                        if (argn == argc ||
                            sscanf(argv[++argn], "%d", &yoff) != 1)
                                pm_usage(usage);
                } else if (pm_keymatch(argv[argn], "-alpha", 2)) {
                        if (argn == argc || alpha != NULL)
                                pm_usage(usage);

                        fp = pm_openr(argv[++argn]);
                        alpha = pgm_readpgm(fp, &alphaCols,
                                                &alphaRows, &alphaMax);
                        pm_close(fp);
                } else if (pm_keymatch(argv[argn], "-invert", 2)) {
                        InvertFlag = 1;
                } else {
                        pm_usage(usage);
                }
                argn++;
        }

        /*
        **  Read the overlay file
        */
        if (argc == argn)
                pm_usage(usage);

        fp = pm_openr(argv[argn++]);
        image = pnm_readpnm(fp, &imageCols, &imageRows, &imageMax, &imageType);
        pm_close(fp);

        /*
        **  If there is an alphamap check to make sure it is the
        **    same size as the image.
        */
        if (alpha != NULL && (imageCols!=alphaCols || imageRows!=alphaRows))
                pm_error("Alpha map and Image are not the same size");

        /*
        **  Now get the data file
        */
        if (argc != argn)
                ifp = pm_openr(argv[argn++]);
        else
                ifp = stdin;

        /*
        **  And the output file
        */
        if (argc != argn)
                ofp = pm_openw(argv[argn++]);
        else
                ofp = stdout;

        /*
        **  Composite the images together
        */
        composite(xoff, yoff, ifp, ofp);

        pm_close(ifp);
        pm_close(ofp);
}

void composite(xoff, yoff, ifp, ofp)
int     xoff, yoff;
FILE    *ifp, *ofp;
{
        int     x, y, x0, y0;
        int     r,g,b;
        xelval  v, maxv, omaxv;
        xel     *pixels;
        double  f;
        int     rows, cols, type, otype;

        pnm_readpnminit(ifp, &cols, &rows, &maxv, &type);

        pixels = pnm_allocrow(cols);

        pnm_writepnminit(ofp, cols, rows, maxv, type, 0);

        /*
        **  Convert overlay image to common type & max
        */
        otype = (imageType < type) ? type : imageType;
        omaxv = (imageMax  < maxv) ? maxv : imageMax;

        if (imageType != otype || imageMax != omaxv) {
                pnm_promoteformat(image, imageCols, imageRows,
                                imageMax, imageType, omaxv, otype);
                imageType = otype;
                imageMax  = omaxv;
        }

        for (y = 0; y < rows; y++) {
                /*
                **  Read a row and convert it to the output type
                */
                pnm_readpnmrow(ifp, pixels, cols, maxv, type);

                if (type != otype || maxv != omaxv)
                        pnm_promoteformatrow(pixels, cols, maxv,
                                                type, omaxv, otype);

                /*
                **  Now overlay the overlay with alpha (if defined)
                */
                for (x = 0; x < cols; x++) {
                        x0 = x - xoff;
                        y0 = y - yoff;

                        if (x0 < 0 || x0 >= imageCols)
                                continue;
                        if (y0 < 0 || y0 >= imageRows)
                                continue;

                        if (alpha == NULL) {
                                f = 1.0;
                        } else {
                                f = (double)alpha[y0][x0] / (double)alphaMax;
                                if (InvertFlag)
                                        f = 1.0 - f;
                        }

                        r = PPM_GETR(pixels[x])     * (1.0 - f) +
                            PPM_GETR(image[y0][x0]) * f;
                        g = PPM_GETG(pixels[x])     * (1.0 - f) +
                            PPM_GETG(image[y0][x0]) * f;
                        b = PPM_GETB(pixels[x])     * (1.0 - f) +
                            PPM_GETB(image[y0][x0]) * f;

                        PPM_ASSIGN(pixels[x], r, g, b);
                }

                pnm_writepnmrow(ofp, pixels, cols, maxv, otype, 0);
        }

        pnm_freerow(pixels);
}
