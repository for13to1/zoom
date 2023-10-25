static char rcsid[] = "$Header: /.dub/dub/Repository/src/zoom/libpic/swap.c,v 1.1.1.1 1998/12/11 01:15:33 pahvant Exp $";

#include <simple.h>

void
swap_long(void *pv)
{
    register char *p = pv;
    char t;

    SWAP(p[0], p[3], t);
    SWAP(p[1], p[2], t);
}

void
swap_short(void *pv)
{
    register char *p = pv;
    char t;

    SWAP(p[0], p[1], t);
}
