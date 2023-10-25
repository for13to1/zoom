/* yuvsplittoppm.c - construct a portable pixmap from 3 raw files:
** - basename.Y : The Luminance chunk at the size of the Image
** - basename.U : The Chrominance chunk U at 1/4
** - basename.V : The Chrominance chunk V at 1/4
** The subsampled U and V values are made by arithmetic mean.
**
** If ccir601 is defined, the produced YUV triples have been scaled again
** to fit into the smaller range of values for this standard.
**
** by Marcel Wijkstra <wijkstra@fwi.uva.nl>
**
** Based on ppmtoyuvsplit.c
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "ppm.h"

/* x must be signed for the following to work correctly */
#define limit(x) (((x>0xffffff)?0xff0000:((x<=0xffff)?0:x&0xff0000))>>16)

int
main(argc, argv)
char **argv;
{
	FILE *ifp,*vf,*uf,*yf;
	pixel          *pixelrow1,*pixelrow2;
	register pixel *pP1,*pP2;
	int             rows, cols, format, row;
	register int    col;
	pixval          maxval;
	char		*usage="<basename> <width> <height> [-ccir601]";
	long int  y,u,v,y0,y1,y2,y3,u0,u1,u2,u3,v0,v1,v2,v3;
	unsigned char  *y1buf,*y2buf,*ubuf,*vbuf;
	char 		ufname[256],vfname[256],yfname[256];
/* Whether to create YUV in JFIF(JPEG) or CCIR.601(MPEG) scale */
	int		ccir601=0;


	ppm_init(&argc, argv);

	if ((argc>5) || (argc<4)) pm_usage(usage);

        if (argc==5)
                if (!strncmp(argv[4],"-c",2))
                        ccir601 = 1;
		else
			pm_usage(usage);

	strcpy(ufname,argv[1]);
	strcpy(vfname,argv[1]);
	strcpy(yfname,argv[1]);
 
	strcat(ufname,".U");
	strcat(vfname,".V");
	strcat(yfname,".Y");

	uf = fopen(ufname,"rb");
	vf = fopen(vfname,"rb");
	yf = fopen(yfname,"rb");

	if(!(uf && vf && yf)) {
 	 perror("error opening input files");
 	 exit(0);
	}

        cols = atoi(argv[2]);
        rows = atoi(argv[3]);
        if (cols <= 0 || rows <= 0)
                pm_usage(usage);

        ppm_writeppminit(stdout, cols, rows, (pixval) 255, 0);

        if(cols & 1) fprintf(stderr,
                             "%s: Warning: odd columns count, exceed ignored\n",
                             argv[0]);
        if(rows & 1) fprintf(stderr,
                             "%s: Warning: odd rows count, exceed ignored\n",
                             argv[0]);

	pixelrow1 = ((pixel*) pm_allocrow( cols, sizeof(pixel) ));
	pixelrow2 = ((pixel*) pm_allocrow( cols, sizeof(pixel) ));

	y1buf = (unsigned char *) pm_allocrow( cols, 1 );
	y2buf = (unsigned char *) pm_allocrow( cols, 1 );
	ubuf = (unsigned char *) pm_allocrow( cols, 1 );
        vbuf = (unsigned char *) pm_allocrow( cols, 1 );

	for (row = 0; row < (rows & ~1); row += 2) {
		unsigned char *y1ptr,*y2ptr,*uptr,*vptr;

		fread(y1buf, (cols & ~1), 1, yf);
		fread(y2buf, (cols & ~1), 1, yf);
		fread(ubuf, cols/2, 1, uf);
		fread(vbuf, cols/2, 1, vf);

		y1ptr = y1buf; y2ptr = y2buf; vptr = vbuf; uptr = ubuf;

                pP1 = pixelrow1; pP2 = pixelrow2;

		for (col = 0 ; col < (cols & ~1); col += 2) {
			long int r0,g0,b0,r1,g1,b1,r2,g2,b2,r3,g3,b3;

			y0 = (long int) *y1ptr++;
			y1 = (long int) *y1ptr++;
			y2 = (long int) *y2ptr++;
			y3 = (long int) *y2ptr++;

			u = (long int) ((*uptr++) - 128);
			v = (long int) ((*vptr++) - 128);

			if (ccir601) {
				y0 = ((y0-16)*255)/219;
				y1 = ((y1-16)*255)/219;
				y2 = ((y2-16)*255)/219;
				y3 = ((y3-16)*255)/219;

				u  = (u*255)/224 ;
				v  = (v*255)/224 ;
			}
			/* mean the chroma for subsampling */

			u0=u1=u2=u3=u;
			v0=v1=v2=v3=v;


/* The inverse of the JFIF RGB to YUV Matrix for $00010000 = 1.0

[Y]   [65496        0   91880][R]
[U] = [65533   -22580  -46799[G]
[V]   [65537   116128      -8][B]

*/

			r0 = 65536 * y0               + 91880 * v0;
			g0 = 65536 * y0 -  22580 * u0 - 46799 * v0;
                        b0 = 65536 * y0 + 116128 * u0             ;

			r1 = 65536 * y1               + 91880 * v1;
			g1 = 65536 * y1 -  22580 * u1 - 46799 * v1;
                        b1 = 65536 * y1 + 116128 * u1             ;

			r2 = 65536 * y2               + 91880 * v2;
			g2 = 65536 * y2 -  22580 * u2 - 46799 * v2;
                        b2 = 65536 * y2 + 116128 * u2             ;

			r3 = 65536 * y3               + 91880 * v3;
			g3 = 65536 * y3 -  22580 * u3 - 46799 * v3;
                        b3 = 65536 * y3 + 116128 * u3             ;

			r0 = limit(r0);
			r1 = limit(r1);
			r2 = limit(r2);
			r3 = limit(r3);
			g0 = limit(g0);
			g1 = limit(g1);
			g2 = limit(g2);
			g3 = limit(g3);
			b0 = limit(b0);
			b1 = limit(b1);
			b2 = limit(b2);
			b3 = limit(b3);

			/* first pixel */
			PPM_ASSIGN(*pP1, (pixval)r0, (pixval)g0, (pixval)b0);
			pP1++;
			/* 2nd pixval */
			PPM_ASSIGN(*pP1, (pixval)r1, (pixval)g1, (pixval)b1);
			pP1++;
			/* 3rd pixval */
			PPM_ASSIGN(*pP2, (pixval)r2, (pixval)g2, (pixval)b2);
			pP2++;
			/* 4th pixval */
			PPM_ASSIGN(*pP2, (pixval)r3, (pixval)g3, (pixval)b3);
			pP2++;
		}
		ppm_writeppmrow(stdout, pixelrow1, cols, (pixval) 255, 0);
		ppm_writeppmrow(stdout, pixelrow2, cols, (pixval) 255, 0);
	}
	pm_close(stdout);

        fclose(yf);
	fclose(uf);
	fclose(vf);
	exit(0);
}
