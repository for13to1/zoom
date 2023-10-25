/* filt.h: definitions for filter data types and routines */

#ifndef FILT_HDR
#define FILT_HDR

/* $Header: /.dub/dub/Repository/src/zoom/zoom/filt.h,v 1.2 1998/12/13 23:07:31 pahvant Exp $ */

typedef double (fn_proc)(double x, void *data);
typedef void (init_proc)(double a, double b, void *data);
typedef void (print_proc)(void *data);

typedef struct {		/* A 1-D FILTER */
    char *name;			/* name of filter */
    fn_proc *func;		/* filter function */
    double supp;		/* radius of nonzero portion */
    double blur;		/* blur factor (1=normal) */
    char windowme;		/* should filter be windowed? */
    char cardinal;		/* is this filter cardinal?
				   ie, does func(x) = (x==0) for integer x? */
    char unitrange;		/* does filter stay within the range [0..1] */
    init_proc *initproc;	/* initialize client data, if any */
    print_proc *printproc;	/* print client data, if any */
    void *clientdata;		/* client info to be passed to func */
} Filt;

#define filt_func(f, x) (*(f)->func)(x, (f)->clientdata)
#define filt_print_client(f) (*(f)->printproc)((f)->clientdata)

extern Filt *filt_find(const char *name);
extern Filt *filt_window(Filt *f, const char *windowname);
extern void filt_print(const Filt *f);
extern void filt_catalog(void);


/* the filter collection: */
extern fn_proc filt_box;	/* box, pulse, Fourier window, */
extern fn_proc filt_triangle;	/* triangle, Bartlett window, */
extern fn_proc filt_quadratic;	/* 3rd order (quadratic) b-spline */
extern fn_proc filt_cubic;	/* 4th order (cubic) b-spline */
extern fn_proc filt_catrom;	/* Catmull-Rom spline, Overhauser spline */
extern fn_proc filt_mitchell;	/* Mitchell & Netravali's two-param cubic */
extern fn_proc filt_gaussian;	/* Gaussian (infinite) */
extern fn_proc filt_sinc;	/* Sinc, perfect lowpass filter (infinite) */
extern fn_proc filt_bessel;   /* Bessel (for circularly symm. 2-d filt, inf)*/
extern fn_proc filt_hanning;	/* Hanning window */
extern fn_proc filt_hamming;	/* Hamming window */
extern fn_proc filt_blackman;	/* Blackman window */
extern fn_proc filt_kaiser;	/* parameterized Kaiser window */

extern fn_proc filt_normal;	/* normal distribution (infinite) */

/* support routines */
extern double bessel_i0(double x);

#endif
