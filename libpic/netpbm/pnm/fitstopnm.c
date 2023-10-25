 /* fitstopnm.c - read a FITS file and produce a portable anymap
 **
 ** Copyright (C) 1989 by Jef Poskanzer.
 **
 ** Permission to use, copy, modify, and distribute this software and its
 ** documentation for any purpose and without fee is hereby granted, provided
 ** that the above copyright notice appear in all copies and that both that
 ** copyright notice and this permission notice appear in supporting
 ** documentation.  This software is provided "as is" without express or
 ** implied warranty.
 **
 ** Hacked up version by Daniel Briggs  (dbriggs@nrao.edu)  20-Oct-92
 **
 ** Include floating point formats, more or less.  Will only work on
 ** machines that understand IEEE-754.  Added -scanmax -printmax
 ** -min -max and -noraw.  Ignore axes past 3, instead of error (many packages
 ** use pseudo axes).  Use a finite scale when max=min.  NB: Min and max
 ** are the real world FITS values (scaled), so watch out when bzer & bscale
 ** are not 0 & 1.  Datamin & datamax interpreted correctly in scaled case,
 ** and initialization changed to less likely values.  If datamin & max are
 ** not present in the header, the a first pass is made to determine them
 ** from the array values.
 **
 ** Modified by Alberto Accomazzi (alberto@cfa.harvard.edu), Dec 1, 1992.
 **
 ** Added support for 3-planes FITS files, the program is renamed fitstopnm.
 ** Fixed some non-ansi declarations (DBL_MAX and FLT_MAX replace MAXDOUBLE
 ** and MAXFLOAT), fixed some scaling parameters to map the full FITS data
 ** resolution to the maximum PNM resolution, disabled min max scanning when
 ** reading from stdin.
 */

#include "pnm.h"

/* Do what you have to, to get a sensible value here.  This may not be so */
/* portable as the rest of the program.  We want to use MAXFLOAT and */
/* MAXDOUBLE, so you could define them manually if you have to. */
/* #include <values.h> */
#include <float.h>

static struct FITS_Header {
  int simple;		/* basic format or not */
  int bitpix;		/* number of bits per pixel */
  int naxis;		/* number of axes */
  int naxis1;		/* number of points on axis 1 */
  int naxis2;		/* number of points on axis 2 */
  int naxis3;		/* number of points on axis 3 */
  double datamin;	/* min # (Physical value!) */
  double datamax;	/* max #     "       "     */
  double bzer;		/* Physical value = Array value*bscale + bzero */
  double bscale;
};

static void read_fits_header ARGS(( FILE* fp, struct FITS_Header* hP ));
static void read_card ARGS(( FILE* fp, char* buf ));
static void read_val ARGS(( FILE* fp, int bitpix, double* vp ));
     
int
main( argc, argv )
int argc;
char* argv[];
{
  FILE* ifp;
  register gray* gP;
  int argn, imagenum, image, row;
  register int col;
  xelval maxval;
  double val, fmaxval, frmin, frmax, dmax, dmin, scale, t;
  int rows, cols, images, format;
  struct FITS_Header h;
  xel** pnmarray;
  xelval tx, txv[4];
  char* fits_name;
  char* usage = "[-image N] [-noraw] [-scanmax] [-printmax] [-min f] [-max f] [FITSfile]";

  int doscan = 0;
  int forceplain = 0;
  int forcemin = 0;
  int forcemax = 0;
  int printmax = 0;
  
  pnm_init( &argc, argv );
  
  argn = 1;
  imagenum = 0;
  
  while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' )
    {
      if ( pm_keymatch( argv[argn], "-image", 2 ) )
	{
	  ++argn;
	  if ( argn == argc || sscanf( argv[argn], "%d", &imagenum ) != 1 )
	    pm_usage( usage );
	}
      else if ( pm_keymatch( argv[argn], "-max", 3 ) )
	{
	  ++argn;
	  forcemax = 1;
	  if ( argn == argc || sscanf( argv[argn], "%lf", &frmax ) != 1 )
	    pm_usage( usage );
	}
      else if ( pm_keymatch( argv[argn], "-min", 3 ) )
	{
	  ++argn;
	  forcemin = 1;
	  if ( argn == argc || sscanf( argv[argn], "%lf", &frmin ) != 1 )
	    pm_usage( usage );
	}
      else if ( pm_keymatch( argv[argn], "-scanmax", 2 ) )
	doscan = 1;
      else if ( pm_keymatch( argv[argn], "-noraw", 2 ) )
	forceplain = 1;
      else if ( pm_keymatch( argv[argn], "-printmax", 2 ) )
	printmax = 1;
      else
	pm_usage( usage );
      ++argn;
    }
  
  if ( argn < argc )
    {
      fits_name = argv[argn];
      ifp = pm_openr( fits_name );
      ++argn;
    }
  else
    ifp = stdin;

  if ( argn != argc )
    pm_usage( usage );
  
  read_fits_header( ifp, &h );
  
  if ( ! h.simple )
    pm_error( "FITS file is not in simple format, can't read" );
  switch ( h.bitpix )
    {
    case 8:
      fmaxval = 255.0;
      break;
      
    case 16:
      fmaxval = 65535.0;
      break;
      
    case 32:
      fmaxval = 4294967295.0;
      break;
      
    case -32:
      fmaxval = FLT_MAX;
      break;
      
    case -64:
      fmaxval = DBL_MAX;
      break;
      
    default:
      pm_error( "unusual bits per pixel (%d), can't read", h.bitpix );
    }

  if ( h.naxis != 2 && h.naxis != 3 )
    pm_message( "Warning: FITS file has %d axes", h.naxis );
  cols = h.naxis1;
  rows = h.naxis2;
  if ( h.naxis == 2 )
    images = imagenum = 1;
  else
    images = h.naxis3;
  if ( imagenum > images )
    pm_error( "only %d plane%s in this file", images, images > 1 ? "s" : "" );
  if ( images != 3 && images > 1 && imagenum == 0 )
    pm_error( "need to specify a plane using the -imagenum flag" );

  format = PGM_FORMAT;
  if ( images == 3 && imagenum == 0 ) 
    format = PPM_FORMAT;

  if (forcemin) h.datamin = frmin;
  if (forcemax) h.datamax = frmax;

  if ( doscan || h.datamin == - DBL_MAX || h.datamax == DBL_MAX )
    {
    if ( ifp == stdin )
      pm_message( "warning: cannot scan max and read from stdin" );
    else 
      {
      /* OK, We have to scale this the hard way. */
      pm_message( "Scanning file for scaling parameters" );
      dmax = -fmaxval;
      dmin = fmaxval;
      for ( image = 1; image <= images; ++image )
	{
	  for ( row = 0; row < rows; ++row)
	    {
	      for ( col = 0; col < cols; ++col)
		{
		  read_val (ifp, h.bitpix, &val);
		  if ( image == imagenum || format == PPM_FORMAT ) {
		    if ( val > dmax ) dmax = val;
		    if ( val < dmin ) dmin = val;
		  }
	        }
	    }
	  pm_close( ifp );
	  if (h.bscale < 0.0) {
	    t = dmax;
	    dmax = dmin;
	    dmin = t;
	  }
	  ifp = pm_openr( fits_name );
	  read_fits_header( ifp, &h );
	  h.datamin = forcemin ? frmin : dmin * h.bscale + h.bzer;
	  h.datamax = forcemax ? frmax : dmax * h.bscale + h.bzer;
	}
      }
    }


  maxval = min( h.datamax - h.datamin, PNM_MAXMAXVAL );
  scale = 1.0;
  /* Just in case we have a file with constant value. */
  if ( h.datamin != h.datamax )
    scale = maxval / ( h.datamax - h.datamin );

  /* If printmax option is true, just print and exit. */
  /* For use in shellscripts.  Ex:                    */
  /* eval `fitstopnm -printmax $filename | \          */
  /*   awk '{min = $1; max = $2}\                     */
  /*         END {print "min=" min; " max=" max}'`    */
  if (printmax) {
    printf( "%lf %lf\n", h.datamin, h.datamax);
    pm_close( ifp );
    pm_close( stdout );
    exit( 0 );
    }

  pnmarray = pnm_allocarray( cols, rows );

  switch( PNM_FORMAT_TYPE( format ))
    {
    case PGM_TYPE:
      pm_message( "writing PGM file" );
      for ( image = 1; image <= imagenum; ++image )
        {
        if ( image != imagenum )
          pm_message( "skipping image plane %d of %d", image, images );
        else if ( images > 1 )
          pm_message( "reading image plane %d of %d", image, images );
        for ( row = 0; row < rows; ++row)
          for ( col = 0; col < cols; ++col )
            {
            read_val (ifp, h.bitpix, &val);
            t = scale * ( val * h.bscale + h.bzer - h.datamin );
	    tx = max( 0, min( t, maxval ) );
	    if ( image == imagenum )
              PNM_ASSIGN1( pnmarray[row][col], tx );
            }
	}
      break;
    case PPM_TYPE:
      pm_message( "writing PPM file" );
      for ( image = 1; image <= images; image++ )
	{
	pm_message( "reading image plane %d of %d", image, images );
	for ( row = 0; row < rows; row++ )
	  for ( col = 0; col < cols; col++ )
            {
            read_val (ifp, h.bitpix, &val);
	    txv[1] = PPM_GETR( pnmarray[row][col] );
	    txv[2] = PPM_GETG( pnmarray[row][col] );
	    txv[3] = PPM_GETB( pnmarray[row][col] );
            t = scale * ( val * h.bscale + h.bzer - h.datamin );
	    txv[image] = max( 0, min( t, maxval ));
            PPM_ASSIGN( pnmarray[row][col], txv[1], txv[2], txv[3] );
            }
	}
      break;
    default:
      pm_error( "can't happen" );
      break;
    }

  pnm_writepnm( stdout, pnmarray, cols, rows, maxval, format, forceplain );
  pnm_freearray( pnmarray, rows );

  pm_close( ifp );
  pm_close( stdout );
  
  exit( 0 );
}

/*
 ** This code will deal properly with integers, no matter what the byte order
 ** or integer size of the host machine.  Sign extension is handled manually
 ** to prevent problems with signed/unsigned characters.  Floating point
 ** values will only be read properly when the host architecture is IEEE-754
 ** conformant.  If you need to tweak this code for other machines, you might
 ** want to snag a copy of the FITS documentation from nssdca.gsfc.nasa.gov
 */

static void
  read_val (fp, bitpix, vp)
FILE *fp;
int bitpix;
double *vp;

{
  int i, ich, ival;
  long int lval;
  unsigned char c[8];
  
  switch ( bitpix )
    {
      /* 8 bit FITS integers are unsigned */
    case 8:
      ich = getc( fp );
      if ( ich == EOF )
	pm_error( "EOF / read error" );
      *vp = ich;
      break;
      
    case 16:
      ich = getc( fp );
      if ( ich == EOF )
	pm_error( "EOF / read error" );
      c[0] = ich;
      ich = getc( fp );
      if ( ich == EOF )
	pm_error( "EOF / read error" );
      c[1] = ich;
      if (c[0] & 0x80)
	ival = ~0xFFFF | c[0]<<8 | c[1];
      else
	ival = c[0]<<8 | c[1];
      *vp = ival;
      break;
      
    case 32:
      for (i=0; i<4; i++) {
	ich = getc( fp );
	if ( ich == EOF )
	  pm_error( "EOF / read error" );
	c[i] = ich;
      }
      if (c[0] & 0x80)
	lval = ~0xFFFFFFFF |
	  c[0]<<24 | c[1]<<16 | c[2]<<8 | c[3];
      else
	lval = c[0]<<24 | c[1]<<16 | c[2]<<8 | c[3];
      *vp = lval;
      
    case -32:
      for (i=0; i<4; i++) {
	ich = getc( fp );
	if ( ich == EOF )
	  pm_error( "EOF / read error" );
	c[i] = ich;
      }
      *vp = *( (float *) c);
      break;
      
    case -64:
      for (i=0; i<8; i++) {
	ich = getc( fp );
	if ( ich == EOF )
	  pm_error( "EOF / read error" );
	c[i] = ich;
      }
      *vp = *( (double *) c);
      break;
      
    default:
      pm_error( "Strange bitpix in read_value" );
    }
}

static void
  read_fits_header( fp, hP )
FILE* fp;
struct FITS_Header* hP;
{
  char buf[80];
  int seen_end;
  int i;
  char c;
  
  seen_end = 0;
  hP->simple = 0;
  hP->bzer = 0.0;
  hP->bscale = 1.0;
  hP->datamin = - DBL_MAX;
  hP->datamax = DBL_MAX;
  
  while ( ! seen_end )
    for ( i = 0; i < 36; ++i )
      {
	read_card( fp, buf );
	
	if ( sscanf( buf, "SIMPLE = %c", &c ) == 1 )
	  {
	    if ( c == 'T' || c == 't' )
	      hP->simple = 1;
	  }
	else if ( sscanf( buf, "BITPIX = %d", &(hP->bitpix) ) == 1 );
	else if ( sscanf( buf, "NAXIS = %d", &(hP->naxis) ) == 1 );
	else if ( sscanf( buf, "NAXIS1 = %d", &(hP->naxis1) ) == 1 );
	else if ( sscanf( buf, "NAXIS2 = %d", &(hP->naxis2) ) == 1 );
	else if ( sscanf( buf, "NAXIS3 = %d", &(hP->naxis3) ) == 1 );
	else if ( sscanf( buf, "DATAMIN = %lf", &(hP->datamin) ) == 1 );
	else if ( sscanf( buf, "DATAMAX = %lf", &(hP->datamax) ) == 1 );
	else if ( sscanf( buf, "BZERO = %lf", &(hP->bzer) ) == 1 );
	else if ( sscanf( buf, "BSCALE = %lf", &(hP->bscale) ) == 1 );
	else if ( strncmp( buf, "END ", 4 ) == 0 ) seen_end = 1;
      }
}

static void
  read_card( fp, buf )
FILE* fp;
char* buf;
{
  if ( fread( buf, 1, 80, fp ) == 0 )
    pm_error( "error reading header" );
}
