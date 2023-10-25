/* simple.h: definitions of some simple, common constants and macros */

#ifndef SIMPLE_HDR
#define SIMPLE_HDR

/* $Header: /.dub/dub/Repository/src/zoom/libsys/simple.h,v 1.1.1.1 1998/12/11 01:15:33 pahvant Exp $ */

#include <stdio.h>

/* better than standard assert.h: doesn't gag on 'if (p) assert(q); else r;' */
#ifndef NDEBUG
#   define assert(p) if (!(p)) \
    { \
    fprintf(stderr, "Assertion failed: %s line %d: p\n", __FILE__, __LINE__); \
    exit(1); \
    } \
    else
# else
#   define assert(p) p
#endif

#define str_eq(a, b)	(strcmp(a, b) == 0)
#define MIN(a, b)	((a)<(b) ? (a) : (b))
#define MAX(a, b)	((a)>(b) ? (a) : (b))
#define ABS(a)		((a)>=0 ? (a) : -(a))
#define SWAP(a, b, t)	{t = a; a = b; b = t;}
#define LERP(t, a, b)	((a)+(t)*((b)-(a)))
#define ALLOC(ptr, type, n)  assert(ptr = (type *)malloc((n)*sizeof(type)))
#define ALLOC_ZERO(ptr, type, n)  assert(ptr = (type *)calloc(n, sizeof(type)))

#define PI 3.14159265358979323846264338
#define RAD_TO_DEG(x) ((x)*(180./PI))
#define DEG_TO_RAD(x) ((x)*(PI/180.))

/* note: the following are machine dependent! (ifdef them if possible) */
#if !defined(WIN32)
#define MINSHORT -32768
#define MINLONG -2147483648
#define MININT MINLONG
#ifndef MAXINT	/* sgi has these in values.h */
#   define MAXSHORT 32767
#   define MAXLONG 2147483647
#   define MAXINT MAXLONG
#endif
#endif

#ifdef hpux	/* hp's unix doesn't have bzero */
#   define bzero(a, n) memset(a, 0, n)
#endif

#if defined(WIN32)
#define bzero(ptr_, size_) memset(ptr_, 0, size_)
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#endif

#endif
