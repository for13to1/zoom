/* xvminitoppm - convert XV thumbnail picture to PPM
**
** Copyright (C) 1993 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"
#define BUFSIZE 256

static void getline ARGS((FILE *fp, char *buf));

int 
main(argc, argv)
    int argc;
    char *argv[];
{
    unsigned int brow[256], rrow[256], grow[256];
    char buf[BUFSIZE];
    int r, g, b, i;
    pixel *pixrow;
    FILE *ifp;
    char *usage = "[xvminipic]";
    int cols, rows, maxval, col, row;
 
    ppm_init(&argc, argv);

    if( argc > 2 ) 
        pm_usage(usage);
    if( argc == 2 ) 
        ifp = pm_openr(argv[1]);
    else
        ifp = stdin;

    i = 0;
    for( r = 0; r < 8; r++ ) 
        for( g = 0; g < 8; g++ )
            for( b = 0; b < 4; b++ ) {
                rrow[i] = (r*255)/7;
                grow[i] = (g*255)/7;
                brow[i] = (b*255)/3;
                i++;
            }
    
    getline(ifp, buf);
    if( strncmp(buf, "P7 332", 6) != 0 )
        pm_error("bad magic number - not a XV thumbnail picture");

    while(1) {
        getline(ifp, buf);
        if( strncmp(buf, "#END_OF_COMMENTS", 16)==0 )
            break;
        if( strncmp(buf, "#BUILTIN", 8)==0 )
            pm_error("cannot convert builtin XV thumbnail pictures");
    }
    getline(ifp, buf);
    if( sscanf(buf, "%d %d %d", &cols, &rows, &maxval) != 3 ) 
        pm_error("error parsing dimension info");
    if( maxval != 255 )
        pm_error("bogus XV thumbnail maxval");

    pixrow = ppm_allocrow(cols);
    ppm_writeppminit(stdout, cols, rows, (pixval)maxval, 0);

    for( row = 0; row < rows; row++ ) {
        for( col = 0; col < cols; col++ ) {
            int byte;
            byte = fgetc(ifp);
            if( byte == EOF )
                pm_error("unexpected EOF");

                PPM_ASSIGN(pixrow[col], rrow[byte], grow[byte], brow[byte]);
	}
        ppm_writeppmrow(stdout, pixrow, cols, (pixval)maxval, 0);
    }
    pm_close(ifp);
    exit(0);
}


static void
getline(fp, buf)
    FILE *fp;
    char *buf;
{
    if( fgets(buf, BUFSIZE, fp) == NULL ) {
        if( ferror(fp) )
            pm_perror("read error");
        else
            pm_error("unexpected EOF");
    }
}
