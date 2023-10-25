#include <simple.h>
#include <pic.h>

#if defined(WIN32)
#include <windows.h>
#else
/* 2-byte, 4-byte unsigned */
typedef unsigned short WORD;
typedef unsigned long DWORD;
/* 4-byte integer */
typedef int LONG;
typedef struct tag_bfi {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
struct tag_bih {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
#endif

/*------------------------------------------------------------*/
struct tagPicBmp {
    char *name;
    FILE *file;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    DWORD palette[256];
    int read_not_write;
    int seekable;
    int width;
    int height;
    int nchan;
    int bit_depth;
    int scanline;
    void *row;
    int row_len;
    void *image;
};
typedef struct tagPicBmp PicBmp;

#define TEST(expr_) assert(expr_)

#define FILE_READ(addr_, count_) \
TEST(1 == fread(addr_, count_, 1, bmp->file))
#define FILE_READ_MEMBER(mem_) \
FILE_READ(&bmp->mem_, sizeof(bmp->mem_))

static void
test(int expr, const char *filename, unsigned line)
{
    if (!expr)
    {
        fprintf(stderr, "Unexpected condition at line %d in file %s\n",
                line, filename);
        assert(0);
    }
}

static int
read_header(PicBmp *bmp)
{
    // read BITMAPFILEHEADER
    FILE_READ_MEMBER(fh);
    if (bmp->fh.bfType != ('B' | ('M' << 8)))
    {
        fprintf(stderr, "%s doesn't begin with 'BM'.\n", bmp->name);
        return FALSE;
    }

    // read BITMAPINFOHEADER
    FILE_READ_MEMBER(ih.biSize);
    TEST(1 ==
        fread(((unsigned char *) &bmp->ih) + sizeof(bmp->ih.biSize),
        sizeof(bmp->ih) - sizeof(bmp->ih.biSize), 1, bmp->file));
    if (bmp->ih.biCompression != BI_RGB)
    {
        fprintf(stderr, "%s is compressed; only BI_RGB supported.\n",
                bmp->name);
        return FALSE;
    }
    if (bmp->ih.biSize > sizeof(bmp->ih))
    {
        // skip remaining part of info header we didn't read
        fseek(bmp->file, bmp->ih.biSize - sizeof(bmp->ih), SEEK_CUR);
    }
    bmp->width = bmp->ih.biWidth;
    bmp->height = bmp->ih.biHeight;
    if (bmp->height < 0)
    {
        bmp->height = -bmp->height;
    }
    bmp->nchan = 3;

    // now ready to read image data, so allocate row buffer
    switch (bmp->ih.biBitCount)
    {
    case 1:  bmp->row_len = (bmp->width + 7) >> 3;  break;
    case 2:  bmp->row_len = (bmp->width + 3) >> 2;  break;
    case 4:  bmp->row_len = (bmp->width + 1) >> 1;  break;
    case 8:  bmp->row_len = bmp->width;             break;
    case 16: bmp->row_len = 2*bmp->width;           break;
    case 24: bmp->row_len = 3*bmp->width;           break;
    case 32: bmp->row_len = 4*bmp->width;           break;

    default:
        fprintf(stderr, "%s has unsupported bit depth %d\n",
                bmp->name, bmp->ih.biBitCount);
        return FALSE;
        break;
    }
    bmp->row_len = (bmp->row_len + 3) & ~3;

    // read palette for indexed formats, which comes after info header
    if (bmp->ih.biBitCount < 16)
    {
        int count = 1L << bmp->ih.biBitCount;
        int i;
        TEST((size_t) count ==
             fread(&bmp->palette[0], sizeof(RGBQUAD), count, bmp->file));
        for (i = 0; i < count; i++)
        {
            bmp->palette[i] |= (255 << 24);
        }
    }

    // skip to bits
    {
        const long pos = ftell(bmp->file);
        if ((long) bmp->fh.bfOffBits > pos)
        {
            fseek(bmp->file, bmp->fh.bfOffBits - pos, SEEK_CUR);
        }
        else if (bmp->seekable)
        {
            fseek(bmp->file, bmp->fh.bfOffBits, SEEK_SET);
        }
    }

    if (bmp->ih.biHeight > 0 && !bmp->seekable)
    {
        // have to read it all now
        int count = bmp->row_len*bmp->height;
        bmp->image = malloc(count);
        TEST(bmp->image);
        TEST(1 == fread(bmp->image, count, 1, bmp->file));
    }
    else
    {
        bmp->row = malloc(bmp->row_len);
    }

    return TRUE;
}

/*------------------------------------------------------------*/
static void *
picbmp_open(const char *filename, const char *mode)
{
    PicBmp *bmp = NULL;
    FILE *infile;
    int seekable = TRUE;

    if ('r' == mode[0] && str_eq(filename, "-.bmp"))
    {
        infile = fdopen(dup(fileno(stdin)), "rb");
        seekable = FALSE;
    }
    else if ('w' == mode[0] && str_eq(filename, "-.bmp"))
    {
        infile = fdopen(dup(fileno(stdout)), "wb");
        seekable = FALSE;
    }
    else
        infile = fopen(filename, mode);

    if (infile) {
        ALLOC(bmp, PicBmp, 1);
        memset(bmp, 0, sizeof(PicBmp));
        bmp->file = infile;
        bmp->name = strdup(filename);
        if ('r' == mode[0])
        {
            bmp->read_not_write = 1;
        }
        bmp->seekable = seekable;
        if (bmp->read_not_write)
        {
            if (!read_header(bmp))
            {
                fprintf(stderr, "%s: read header failed.\n", bmp->name);
                free(bmp->name);
                fclose(infile);
                free(bmp);
                return NULL;
            }
        }
    }

    return bmp;
}


/*------------------------------------------------------------*/
static void
picbmp_close(void *p)
{
    PicBmp *bmp = (PicBmp *) p;

#define PURGE(member_) if (member_) (free(member_), member_ = NULL); else
    if (bmp->seekable)
    {       
        PURGE(bmp->row);
        bmp->image = NULL;
    }
    else
    {
        PURGE(bmp->image);
        bmp->row = NULL;
    }
    PURGE(bmp->name);
#undef PURGE

    if (bmp->file)
    {
        fclose(bmp->file);
        bmp->file = NULL;
    }
    free(bmp);
}

/*------------------------------------------------------------*/
static char *
picbmp_get_name(void *p)
{
    PicBmp *bmp = (PicBmp *) p;
    return bmp->name;
}

/*------------------------------------------------------------*/
static void
picbmp_clear(void *vp, Pixel1 pv)
{
    /* hmm... ignore? */
}

/*------------------------------------------------------------*/
static void
picbmp_clear_rgba(void *vp, Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
    /* hmm... ignore? */
}

/*------------------------------------------------------------*/
static void
picbmp_set_nchan(void *vp, int nchan)
{
    PicBmp *bmp = (PicBmp *) vp;

    bmp->nchan = nchan;
}

/*------------------------------------------------------------*/
static void
picbmp_set_box(void *vp, int ox, int oy, int dx, int dy)
{
    PicBmp *bmp = (PicBmp *) vp;
    if (! bmp->read_not_write) {
        bmp->width = ox + dx;
        bmp->height = oy + dy;
    }
}

/*------------------------------------------------------------*/
static void
picbmp_write_pixel(void *vp, int x, int y, Pixel1 pv)
{
    fprintf(stderr, "?picbmp_write_pixel\n");
}

/*------------------------------------------------------------*/
static void
picbmp_write_pixel_rgba(void *vp, int x, int y,
                        Pixel1 r, Pixel1 g, Pixel1 b, Pixel1 a)
{
    fprintf(stderr, "?picbmp_write_pixel_rgba\n");
}

/*------------------------------------------------------------*/
#define WRITE_INIT(picbmp_) ((picbmp_)->scanline || write_init(picbmp_))
static int
write_init(PicBmp *bmp)
{
    bmp->row_len = (3*bmp->width + 3) & ~3;

    {
        // write BITMAPFILEHEADER
        const BITMAPFILEHEADER fh =
        {
            'B' | ('M' << 8),
            sizeof(bmp->fh) + sizeof(bmp->ih) + bmp->height*bmp->row_len,
            0, 0, sizeof(bmp->ih) + sizeof(bmp->fh)
        };
        memcpy(&bmp->fh, &fh, sizeof(bmp->fh));
        TEST(1 == fwrite(&bmp->fh, sizeof(bmp->fh), 1, bmp->file));
    }
    {
        const BITMAPINFOHEADER ih =
        {
            sizeof(ih), bmp->width, -bmp->height, 1, 24, BI_RGB
        };
        // write BITMAPINFOHEADER
        memcpy(&bmp->ih, &ih, sizeof(bmp->ih));
        TEST(1 == fwrite(&bmp->ih, sizeof(bmp->ih), 1, bmp->file));
    }

    // no palette mode support, so don't write palette

    // now ready to write pixel data
    TEST(!bmp->row);
    bmp->row = malloc(bmp->row_len);

    return 1;
}

/*------------------------------------------------------------*/
static void
seek_row_write(PicBmp *bmp, int y)
{
    memset(bmp->row, 0, bmp->row_len);
    while (bmp->scanline < y) {
        TEST(1 == fwrite(bmp->row, bmp->row_len, 1, bmp->file));
        bmp->scanline++;
    }
}

/*------------------------------------------------------------*/
static void
picbmp_write_row(void *vp, int y, int x0, int nx, const Pixel1 *buf)
{
    PicBmp *bmp = (PicBmp *) vp;

    TEST(0);
    if (! bmp->read_not_write && 1 == bmp->nchan && WRITE_INIT(bmp)) {
        int i;
        Pixel1 *row = (Pixel1 *) bmp->row;

        seek_row_write(bmp, y);
        for (i = 0; i < nx; i++)
            row[x0+i] = buf[i];
        TEST(1 == fwrite(buf, bmp->row_len, 1, bmp->file));
        bmp->scanline++;
    }
}

/*------------------------------------------------------------*/
static void
picbmp_write_row_rgba(void *vp, int y, int x0, int nx, const Pixel1_rgba *buf)
{
    PicBmp *bmp = (PicBmp *) vp;

    if (! bmp->read_not_write && 3 == bmp->nchan && WRITE_INIT(bmp)) {
        int i, j;
        Pixel1 *row = (Pixel1 *) bmp->row;

        seek_row_write(bmp, y);
        for (i = 0, j = x0; i < nx; i++, j += 3) {
            row[j+0] = buf[i].b;
            row[j+1] = buf[i].g;
            row[j+2] = buf[i].r;
        }
        TEST(1 == fwrite(bmp->row, bmp->row_len, 1, bmp->file));
        bmp->scanline++;
    }
}

/*------------------------------------------------------------*/
static int
picbmp_get_nchan(void *vp)
{
    PicBmp *bmp = (PicBmp *) vp;
    return bmp->nchan;
}

/*------------------------------------------------------------*/
static void
picbmp_get_box(void *vp, int *ox, int *oy, int *dx, int *dy)
{
    PicBmp *bmp = (PicBmp *) vp;

    if (bmp->read_not_write || bmp->scanline) {
        *ox = 0;
        *oy = 0;
        *dx = bmp->width;
        *dy = bmp->height;
    } else {
        *ox = PIXEL_UNDEFINED;
        *oy = PIXEL_UNDEFINED;
        *dx = PIXEL_UNDEFINED;
        *dy = PIXEL_UNDEFINED;
    }
}

/*------------------------------------------------------------*/
static Pixel1
picbmp_read_pixel(void *vp, int x, int y)
{
    fprintf(stderr, "?picbmp_read_pixel\n");
    return 0;
}

/*------------------------------------------------------------*/
static void
picbmp_read_pixel_rgba(void *vp, int x, int y, Pixel1_rgba *pv)
{
    fprintf(stderr, "?picbmp_read_pixel_rgba\n");
}

/*------------------------------------------------------------*/
static void
seek_row_read(PicBmp *bmp, int y)
{
    if (bmp->ih.biHeight > 0)
    {
        // bottommost scanline is first in file
        if (bmp->seekable)
        {
            int count;
            size_t offset =
                bmp->fh.bfOffBits + (bmp->height - (y+1))*bmp->row_len;
            fseek(bmp->file, offset, SEEK_SET);
            count = fread(bmp->row, 1, bmp->row_len, bmp->file);
            TEST(bmp->row_len == count);
        }
        else
        {
            bmp->row =
                ((Pixel1 *) bmp->image) + (bmp->height - (y+1))*bmp->row_len;
        }
        bmp->scanline = y+1;
    }
    else
    {
        // topmost scanline is first
        while (bmp->scanline <= y)
        {
            int count = fread(bmp->row, 1, bmp->row_len, bmp->file);
            TEST(bmp->row_len == count);
            bmp->scanline++;
        }
    }
}

/*------------------------------------------------------------*/
static void
picbmp_read_row(void *vp, int y, int x0, int nx, Pixel1 *buf)
{
    PicBmp *bmp = (PicBmp *) vp;

    if (bmp->read_not_write && 1 == bmp->nchan) {
        int i;
        Pixel1 *row = (Pixel1 *) bmp->row;

        seek_row_read(bmp, y);
        for (i = 0; i < nx; i++)
            buf[i] = row[x0+i];
    }
}

/*------------------------------------------------------------*/
static void
picbmp_read_row_rgba(void *vp, int y, int x0, int nx, Pixel1_rgba *buf)
{
    static const Pixel1 clut16[32] = {
        0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115,
            123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214,
            222, 230, 239, 247, 255
    };
    PicBmp *bmp = (PicBmp *) vp;

    if (bmp->read_not_write && 3 == bmp->nchan) {
        int i;
        int j;
        Pixel1 *row = (Pixel1 *) bmp->row;

        seek_row_read(bmp, y);
        switch (bmp->ih.biBitCount)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            for (i = 0, j = x0; i < nx; i++, j++)
            {
                const DWORD pixel = bmp->palette[row[j]];
                buf[i].b = (Pixel1) pixel & 0xFF;
                buf[i].g = (Pixel1) (pixel >> 8) & 0xFF;
                buf[i].r = (Pixel1) (pixel >> 16) & 0xFF;
                buf[i].a = (Pixel1) (pixel >> 24) & 0xFF;
            }
            break;

        case 16:
            for (i = 0, j = x0; i < nx; i++, j += 2)
            {
                buf[i].b = clut16[row[j+0] & 31];
                buf[i].g = clut16[((row[j+1] & 3) << 3) + ((row[j+0] >> 5) & 7)];
                buf[i].r = clut16[(row[j+1] >> 2) & 31];
                buf[i].a = 255;
            }
            break;

        case 24:
            for (i = 0, j = x0; i < nx; i++, j += 3)
            {
                buf[i].r = row[j+2];
                buf[i].g = row[j+1];
                buf[i].b = row[j+0];
                buf[i].a = 255;
            }
            break;
        case 32:
            for (i = 0, j = x0; i < nx; i++, j += 4)
            {
                buf[i].r = row[j+2];
                buf[i].g = row[j+1];
                buf[i].b = row[j+0];
                buf[i].a = 255;
            }
            break;
        default:
            TEST(0);
        }
    }
}

/*------------------------------------------------------------*/
static int
picbmp_next_pic(void *vp)
{
    return 0;
}

/*------------------------------------------------------------*/
static Pic_procs
pic_picbmp_procs = {
    picbmp_open,
    picbmp_close,
    picbmp_get_name,

    picbmp_clear,
    picbmp_clear_rgba,

    picbmp_set_nchan,
    picbmp_set_box,

    picbmp_write_pixel,
    picbmp_write_pixel_rgba,
    picbmp_write_row,
    picbmp_write_row_rgba,

    picbmp_get_nchan,
    picbmp_get_box,
    picbmp_read_pixel,
    picbmp_read_pixel_rgba,
    picbmp_read_row,
    picbmp_read_row_rgba,
    picbmp_next_pic
};

/*------------------------------------------------------------*/
Pic pic_bmp =
{
    "bmp", &pic_picbmp_procs
};
