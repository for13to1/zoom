/* ilbm.h - header file for IFF ILBM files
*/

#define RowBytes(cols)          ( ( ( (cols) + 15 ) / 16 ) * 2 )


/* definitions for BMHD */

typedef struct {
    unsigned short w, h;
    short x, y;
    unsigned char nPlanes, masking, compression, pad1;
    unsigned short transparentColor;
    unsigned char xAspect, yAspect;
    short pageWidth, pageHeight;
} BitMapHeader;
#define BitMapHeaderSize    20

#define mskNone                 0
#define mskHasMask              1
#define mskHasTransparentColor  2
#define mskLasso                3

#define cmpNone                 0
#define cmpByteRun1             1
#define cmpMAXKNOWN             cmpByteRun1
static char *  cmpNAME[] = { "none", "byterun1" };

/* definitions for CMAP */

#if 0   /* not used */
typedef struct {
    unsigned char r, g, b;
} ColorRegister;
#endif


/* definitions for CAMG */

#define CAMGChunkSize       4

#define vmLACE              0x0004  /* not used */
#define vmEXTRA_HALFBRITE   0x0080
#define vmHAM               0x0800
#define vmHIRES             0x8000  /* not used */


#define HAMCODE_CMAP      0     /* look up color in colormap */
#define HAMCODE_BLUE      1     /* new blue component */
#define HAMCODE_RED       2     /* new red component */
#define HAMCODE_GREEN     3     /* new green component */


/* unofficial DCOL chunk for direct-color */

typedef struct {
    unsigned char r, g, b, pad1;
} DirectColor;
#define DirectColorSize     4


/* multipalette PCHG chunk definitions */

/* get number of longwords in line mask from PCHG.LineCount */
#define MaskLongWords(x)    (((x) + 31) / 32)

typedef struct {
    unsigned short  Compression;
    unsigned short  Flags;
    short           StartLine;      /* may be negative */
    unsigned short  LineCount;
    unsigned short  ChangedLines;
    unsigned short  MinReg;
    unsigned short  MaxReg;
    unsigned short  MaxChanges;
    unsigned long   TotalChanges;
} PCHGHeader;
#define PCHGHeaderSize      20

/* Compression modes */
#define PCHG_COMP_NONE      0
#define PCHG_COMP_HUFFMAN   1

/* Flags */
#define PCHGF_12BIT         (1 << 0)    /* use SmallLineChanges */
#define PCHGF_32BIT         (1 << 1)    /* use BigLineChanges */
#define PCHGF_USE_ALPHA     (1 << 2)    /* meaningful only if PCHG_32BIT is on:
                                           use the Alpha channel info */
typedef struct {
    unsigned long   CompInfoSize;
    unsigned long   OriginalDataSize;
} PCHGCompHeader;
#define PCHGCompHeaderSize  8

#if 0   /* not used */
typedef struct {
    unsigned char   ChangeCount16;
    unsigned char   ChangeCount32;
    unsigned short  *PaletteChange;
} SmallLineChanges;

typedef struct {
    unsigned short  Register;
    unsigned char   Alpha, Red, Blue, Green;    /* ARBG, not ARGB */
} BigPaletteChange;

typedef struct {
    unsigned short      ChangeCount;
    BigPaletteChange    *PaletteChange;
} BigLineChanges;
#endif /* 0 */

/* the next three structures are used internally by ilbmtoppm
 * The PCHG BigLineChanges and SmallLineChanges are converted
 * to these structures
 */
typedef struct {
    unsigned short  Register;
    pixval          Alpha, Red, Green, Blue;
} PaletteChange;

typedef struct {
    unsigned short  Count;
    PaletteChange   *Palette;
} LineChanges;

typedef struct {
    PCHGHeader          *PCHG;
    unsigned char       *LineMask;
    LineChanges         *Change;
    PaletteChange       *Palette;
    pixval              maxval;     /* maxval of colors in Palette */
    pixel *             colormap;   /* original colormap */
    int                 colors;     /* colors in colormap */
} PCHGInfo;


/* other stuff */

#define ILBM_BIGRAW

#ifdef ILBM_BIGRAW
#   define MAXPLANES   16
typedef unsigned short  rawtype;
#else
#   define MAXPLANES   8
typedef unsigned char   rawtype;
#endif

#define MAXCMAPCOLORS   (1 << MAXPLANES)
#define MAXCOLVAL       255     /* max value of color component */

