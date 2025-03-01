/**
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991, 2000, 2001 by Lucent Technologies.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 */

/**
 * Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").
 */

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "dtoa.h"
#include "mem.h"
#include "format.h" // Nst_fprintf
#include "global_consts.h" // Nst_io.err

#if Nst_BYTEORDER == Nst_LITTLE_ENDIAN
#define IEEE_8087
#else
#define IEEE_MC68k
#endif

#ifdef _DEBUG
#define DEBUG
#endif

#ifdef DEBUG
#define Bug(x) { Nst_fprintf(Nst_io.err, "%s\n", x); exit(1); }
int dtoa_stats[7]; /* strtod_{64,96,bigcomp},dtoa_{exact,64,96,bigcomp} */
#endif

#define PRIVATE_MEM 2304
#define PRIVATE_mem ((PRIVATE_MEM+sizeof(double)-1)/sizeof(double))
static double private_mem[PRIVATE_mem], *pmem_next = private_mem;

#define Exp_shift 20
#define Exp_msk1 0x100000
#define Exp_msk11 0x100000
#define Exp_mask 0x7ff00000
#define P 53
#define Nbits 53
#define Bias 1023
#define Emax 1023
#define Emin (-1022)
#define Exp_1 0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask 0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask 0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define strtod_diglim 40
#define Kmax 7
#define Scale_Bit 0x10
#define n_bigtens 5
#define ULbits 32
#define kshift 5
#define kmask 31

#ifdef IEEE_8087
#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#else
#define word0(x) (x)->L[0]
#define word1(x) (x)->L[1]
#endif
#define dval(x) (x)->d
#define LLval(x) (x)->LL

#define Storeinc(a,b,c) (*a++ = (b << 16) | (c & 0xffff))

#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

#define Bcopy(x,y)                                                            \
    memcpy(                                                                   \
        (char *)&x->sign,                                                     \
        (char *)&y->sign,                                                     \
        y->wds*sizeof(Long) + 2*sizeof(int))

#define FFFFFFFF 0xffffffffUL

enum {
    Round_zero = 0,
    Round_near = 1,
    Round_up = 2,
    Round_down = 3
};

typedef int Long;
typedef uint ULong;

typedef union {
    double d;
    ULong L[2];
} U;
typedef struct {
    int dp0, dp1, dplen, dsign, e0, inexact, nd, nd0, rounding, scale, uflchk;
} BCinfo;

typedef struct _Bigint {
    struct _Bigint *next;
    int k, maxwds, sign, wds;
    ULong x[1];
} Bigint;

typedef struct {
    Bigint *Freelist[Kmax+1];
    Bigint *P5s;
} ThInfo;

static ThInfo TI0;

static const double tens[] = {
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22
};

static const double bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static const double tinytens[] = {
    1e-16, 1e-32, 1e-64, 1e-128, 9007199254740992.*9007199254740992.e-256
};

static unsigned char hexdig[256] = {
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    16,17,18,19,20,21,22,23,24,25, 0, 0, 0, 0, 0, 0,
     0,26,27,28,29,30,31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,26,27,28,29,30,31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define freelist TI0.Freelist
#define p5s TI0.P5s

static Bigint *Balloc(int k)
{
    int x;
    Bigint *rv;
    unsigned int len;

    if (k <= Kmax && (rv = freelist[k]))
        freelist[k] = rv->next;
    else {
        x = 1 << k;
        len = (sizeof(Bigint) + (x-1)*sizeof(ULong) + sizeof(double) - 1)
            /sizeof(double);
        if (k <= Kmax && pmem_next - private_mem + len <= (uint)PRIVATE_mem) {
            rv = (Bigint *)pmem_next;
            pmem_next += len;
        } else
            rv = (Bigint *)Nst_raw_malloc(len*sizeof(double));
        rv->k = k;
        rv->maxwds = x;
    }
    rv->sign = rv->wds = 0;
    return rv;
}

static void Bfree(Bigint *v)
{
    if (v) {
        if (v->k > Kmax)
            Nst_raw_free((void *)v);
        else {
            v->next = freelist[v->k];
            freelist[v->k] = v;
        }
    }
}

static Bigint *multadd(Bigint *b, int m, int a)
{
    int i, wds;
    ULong carry, *x, y;
    ULong xi, z;
    Bigint *b1;

    wds = b->wds;
    x = b->x;
    i = 0;
    carry = a;
    do {
        xi = *x;
        y = (xi & 0xffff) * m + carry;
        z = (xi >> 16) * m + (y >> 16);
        carry = z >> 16;
        *x++ = (z << 16) + (y & 0xffff);
    } while (++i < wds);
    if (carry) {
        if (wds >= b->maxwds) {
            b1 = Balloc(b->k+1);
            Bcopy(b1, b);
            Bfree(b);
            b = b1;
        }
        b->x[wds++] = carry;
        b->wds = wds;
    }
    return b;
}

static Bigint *s2b(const char *s, int nd0, int nd, ULong y9, int dplen)
{
    Bigint *b;
    int i, k;
    Long x, y;

    x = (nd + 8) / 9;
    for (k = 0, y = 1; x > y; y <<= 1, k++);
    b = Balloc(k);
    b->x[0] = y9;
    b->wds = 1;

    i = 9;
    if (9 < nd0) {
        s += 9;
        do b = multadd(b, 10, *s++ - '0');
        while (++i < nd0);
        s += dplen;
    } else
        s += dplen + 9;
    for (; i < nd; i++)
        b = multadd(b, 10, *s++ - '0');
    return b;
}

static int hi0bits(ULong x)
{
    int k = 0;

    if (!(x & 0xffff0000)) {
        k = 16;
        x <<= 16;
    }
    if (!(x & 0xff000000)) {
        k += 8;
        x <<= 8;
    }
    if (!(x & 0xf0000000)) {
        k += 4;
        x <<= 4;
    }
    if (!(x & 0xc0000000)) {
        k += 2;
        x <<= 2;
    }
    if (!(x & 0x80000000)) {
        k++;
        if (!(x & 0x40000000))
            return 32;
    }
    return k;
}

static int lo0bits(ULong *y)
{
    int k;
    ULong x = *y;

    if (x & 7) {
        if (x & 1)
            return 0;
        if (x & 2) {
            *y = x >> 1;
            return 1;
        }
        *y = x >> 2;
        return 2;
    }
    k = 0;
    if (!(x & 0xffff)) {
        k = 16;
        x >>= 16;
    }
    if (!(x & 0xff)) {
        k += 8;
        x >>= 8;
    }
    if (!(x & 0xf)) {
        k += 4;
        x >>= 4;
    }
    if (!(x & 0x3)) {
        k += 2;
        x >>= 2;
    }
    if (!(x & 1)) {
        k++;
        x >>= 1;
        if (!x)
            return 32;
    }
    *y = x;
    return k;
}

static Bigint *i2b(int i)
{
    Bigint *b;

    b = Balloc(1);
    b->x[0] = i;
    b->wds = 1;
    return b;
}

static Bigint *mult(Bigint *a, Bigint *b)
{
    Bigint *c;
    int k, wa, wb, wc;
    ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
    ULong y;
    ULong carry, z;
    ULong z2;

    if (a->wds < b->wds) {
        c = a;
        a = b;
        b = c;
    }
    k = a->k;
    wa = a->wds;
    wb = b->wds;
    wc = wa + wb;
    if (wc > a->maxwds)
        k++;
    c = Balloc(k);
    for (x = c->x, xa = x + wc; x < xa; x++)
        *x = 0;
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;

    for (; xb < xbe; xb++, xc0++) {
        if ((y = *xb & 0xffff)) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
                carry = z >> 16;
                z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
                carry = z2 >> 16;
                Storeinc(xc, z2, z);
            } while (x < xae);
            *xc = carry;
        }
        if ((y = *xb >> 16)) {
            x = xa;
            xc = xc0;
            carry = 0;
            z2 = *xc;
            do {
                z = (*x & 0xffff) * y + (*xc >> 16) + carry;
                carry = z >> 16;
                Storeinc(xc, z, z2);
                z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
                carry = z2 >> 16;
            } while (x < xae);
            *xc = z2;
        }
    }

    for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc);
    c->wds = wc;
    return c;
}

static Bigint *pow5mult(Bigint *b, int k)
{
    Bigint *b1, *p5, *p51;
    int i;
    static int p05[3] = { 5, 25, 125 };

    if ((i = k & 3))
        b = multadd(b, p05[i-1], 0);

    if (!(k >>= 2))
        return b;
    if (!(p5 = p5s)) {
        p5 = p5s = i2b(625);
        p5->next = 0;
    }
    for (;;) {
        if (k & 1) {
            b1 = mult(b, p5);
            Bfree(b);
            b = b1;
        }
        if (!(k >>= 1))
            break;
        if (!(p51 = p5->next)) {
            p51 = p5->next = mult(p5, p5);
            p51->next = 0;
        }
        p5 = p51;
    }
    return b;
}

static Bigint *lshift(Bigint *b, int k)
{
    int i, k1, n, n1;
    Bigint *b1;
    ULong *x, *x1, *xe, z;

    n = k >> 5;
    k1 = b->k;
    n1 = n + b->wds + 1;
    for (i = b->maxwds; n1 > i; i <<= 1)
        k1++;
    b1 = Balloc(k1);
    x1 = b1->x;
    for (i = 0; i < n; i++)
        *x1++ = 0;
    x = b->x;
    xe = x + b->wds;
    if (k &= 0x1f) {
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        } while (x < xe);
        if ((*x1 = z))
            ++n1;
    } else do
        *x1++ = *x++;
    while (x < xe);
    b1->wds = n1 - 1;
    Bfree(b);
    return b1;
}

static int cmp(Bigint *a, Bigint *b)
{
    ULong *xa, *xa0, *xb, *xb0;
    int i, j;

    i = a->wds;
    j = b->wds;
#ifdef DEBUG
    if (i > 1 && !a->x[i-1])
        Bug("cmp called with a->x[a->wds-1] == 0");
    if (j > 1 && !b->x[j-1])
        Bug("cmp called with b->x[b->wds-1] == 0");
#endif
    if (i -= j)
        return i;
    xa0 = a->x;
    xa = xa0 + j;
    xb0 = b->x;
    xb = xb0 + j;
    for (;;) {
        if (*--xa != *--xb)
            return *xa < *xb ? -1 : 1;
        if (xa <= xa0)
            break;
    }
    return 0;
}

static Bigint *diff(Bigint *a, Bigint *b)
{
    Bigint *c;
    int i, wa, wb;
    ULong *xa, *xae, *xb, *xbe, *xc;
    ULong borrow, y;
    ULong z;

    i = cmp(a, b);
    if (!i) {
        c = Balloc(0);
        c->wds = 1;
        c->x[0] = 0;
        return c;
    }
    if (i < 0) {
        c = a;
        a = b;
        b = c;
        i = 1;
    } else
        i = 0;
    c = Balloc(a->k);
    c->sign = i;
    wa = a->wds;
    xa = a->x;
    xae = xa + wa;
    wb = b->wds;
    xb = b->x;
    xbe = xb + wb;
    xc = c->x;
    borrow = 0;

    do {
        y = (*xa & 0xffff) - (*xb & 0xffff) - borrow;
        borrow = (y & 0x10000) >> 16;
        z = (*xa++ >> 16) - (*xb++ >> 16) - borrow;
        borrow = (z & 0x10000) >> 16;
        Storeinc(xc, z, y);
    } while (xb < xbe);
    while (xa < xae) {
        y = (*xa & 0xffff) - borrow;
        borrow = (y & 0x10000) >> 16;
        z = (*xa++ >> 16) - borrow;
        borrow = (z & 0x10000) >> 16;
        Storeinc(xc, z, y);
    }

    while (!*--xc)
        wa--;
    c->wds = wa;
    return c;
}

static double ulp(U *x)
{
    Long L;
    U u;

    L = (word0(x) & Exp_mask) - (P-1)*Exp_msk1;
    word0(&u) = L;
    word1(&u) = 0;

    return dval(&u);
}

static double b2d(Bigint *a, int *e)
{
    ULong *xa, *xa0, w, y, z;
    int k;
    U d;

#define d0 word0(&d)
#define d1 word1(&d)

    xa0 = a->x;
    xa = xa0 + a->wds;
    y = *--xa;
#ifdef DEBUG
    if (!y) Bug("zero y in b2d");
#endif
    k = hi0bits(y);
    *e = 32 - k;
    if (k < Ebits) {
        d0 = Exp_1 | y >> (Ebits - k);
        w = xa > xa0 ? *--xa : 0;
        d1 = y << ((32-Ebits) + k) | w >> (Ebits - k);
        goto ret_d;
    }
    z = xa > xa0 ? *--xa : 0;
    if (k -= Ebits) {
        d0 = Exp_1 | y << k | z >> (32 - k);
        y = xa > xa0 ? *--xa : 0;
        d1 = z << k | y >> (32 - k);
    } else {
        d0 = Exp_1 | y;
        d1 = z;
    }

ret_d:
#undef d0
#undef d1
    return dval(&d);
}

static Bigint *d2b(U *d, int *e, int *bits)
{
    Bigint *b;
    int de, k;
    ULong *x, y, z;
    int i;

#define d0 word0(d)
#define d1 word1(d)

    b = Balloc(1);
    x = b->x;

    z = d0 & Frac_mask;
    d0 &= 0x7fffffff;
    if ((de = (int)(d0 >> Exp_shift)))
        z |= Exp_msk1;

    if ((y = d1)) {
        if ((k = lo0bits(&y))) {
            x[0] = y | z << (32 - k);
            z >>= k;
        } else
            x[0] = y;
        i = b->wds = (x[1] = z) ? 2 : 1;
    } else {
        k = lo0bits(&z);
        x[0] = z;
        i = b->wds = 1;
        k += 32;
    }

    if (de) {
        *e = de - Bias - (P-1) + k;
        *bits = P - k;
    } else {
        *e = de - Bias - (P-1) + 1 + k;
        *bits = 32*i - hi0bits(x[i-1]);
    }

#undef d0
#undef d1

    return b;
}

static double ratio(Bigint *a, Bigint *b)
{
    U da, db;
    int k, ka, kb;

    dval(&da) = b2d(a, &ka);
    dval(&db) = b2d(b, &kb);
    k = ka - kb + 32*(a->wds - b->wds);
    if (k > 0)
        word0(&da) += k*Exp_msk1;
    else {
        k = -k;
        word0(&db) += k*Exp_msk1;
    }
    return dval(&da) / dval(&db);
}

static int match(const char **sp, const char *t)
{
    int c, d;
    const char *s = *sp;

    while ((d = *t++)) {
        if ((c = *++s) >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        if (c != d)
            return 0;
    }
    *sp = s + 1;
    return 1;
}

static void hexnan(U *rvp, const char **sp)
{
    ULong c, x[2];
    const char *s;
    int c1, havedig, udx0, xshift;

    x[0] = x[1] = 0;
    havedig = xshift = 0;
    udx0 = 1;
    s = *sp;

    while ((c = *(const unsigned char *)(s+1)) && c <= ' ')
        ++s;
    if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))
        s += 2;
    while ((c = *(const unsigned char *)++s)) {
        if ((c1 = hexdig[c]))
            c = c1 & 0xf;
        else if (c <= ' ') {
            if (udx0 && havedig) {
                udx0 = 0;
                xshift = 1;
            }
            continue;
        } else {
            do {
                if (c == ')') {
                    *sp = s + 1;
                    break;
                }
            } while ((c = *++s));
            break;
        }
        havedig = 1;
        if (xshift) {
            xshift = 0;
            x[0] = x[1];
            x[1] = 0;
        }
        if (udx0)
            x[0] = (x[0] << 4) | (x[1] >> 28);
        x[1] = (x[1] << 4) | c;
    }
    if ((x[0] &= 0xfffff) || x[1]) {
        word0(rvp) = Exp_mask | x[0];
        word1(rvp) = x[1];
    }
}

static Bigint *increment(Bigint *b)
{
    ULong *x, *xe;
    Bigint *b1;

    x = b->x;
    xe = x + b->wds;
    do {
        if (*x < (ULong)0xffffffffL) {
            ++*x;
            return b;
        }
        *x++ = 0;
    } while (x < xe);
    {
        if (b->wds >= b->maxwds) {
            b1 = Balloc(b->k+1);
            Bcopy(b1, b);
            Bfree(b);
            b = b1;
        }
        b->x[b->wds++] = 1;
    }
    return b;
}

static void rshift(Bigint *b, int k)
{
    ULong *x, *x1, *xe, y;
    int n;

    x = x1 = b->x;
    n = k >> kshift;
    if (n < b->wds) {
        xe = x + b->wds;
        x += n;
        if (k &= kmask) {
            n = 32 - k;
            y = *x++ >> k;
            while (x < xe) {
                *x1++ = (y | (*x << n)) & 0xffffffff;
                y = *x++ >> k;
            }
            if ((*x1 = y) !=0)
                x1++;
        } else
            while (x < xe)
                *x1++ = *x++;
    }
    if ((b->wds = (int)(x1 - b->x)) == 0)
        b->x[0] = 0;
}

static ULong any_on(Bigint *b, int k)
{
    int n, nwds;
    ULong *x, *x0, x1, x2;

    x = b->x;
    nwds = b->wds;
    n = k >> kshift;
    if (n > nwds)
        n = nwds;
    else if (n < nwds && (k &= kmask)) {
        x1 = x2 = x[n];
        x1 >>= k;
        x1 <<= k;
        if (x1 != x2)
            return 1;
    }
    x0 = x;
    x += n;
    while (x > x0)
        if (*--x)
            return 1;
    return 0;
}

void gethex(const char **sp, U *rvp, int rounding, int sign)
{
    Bigint *b;
    char d;
    const unsigned char *decpt, *s0, *s, *s1;
    Long e, e1;
    ULong L, lostbits, *x;
    int big, denorm, esign, havedig, k, n, nb, nbits, nz, up, zret;
    enum {
        emax = 0x7fe - Bias - P + 1,
        emin = Emin - P + 1
    };
    int check_denorm = 0;

    havedig = 0;
    s0 = *(const unsigned char **)sp + 2;
    while (s0[havedig] == '0')
        havedig++;
    s0 += havedig;
    s = s0;
    decpt = 0;
    zret = 0;
    e = 0;
    if (hexdig[*s])
        havedig++;
    else {
        zret = 1;
        if (*s != '.')
            goto pcheck;
        decpt = ++s;
        if (!hexdig[*s])
            goto pcheck;
        while (*s == '0')
            s++;
        if (hexdig[*s])
            zret = 0;
        havedig = 1;
        s0 = s;
    }
    while (hexdig[*s])
        s++;
    if (*s == '.' && !decpt) {
        decpt = ++s;
        while (hexdig[*s])
            s++;
    }
    if (decpt)
        e = -(((Long)(s-decpt)) << 2);
pcheck:
    s1 = s;
    big = esign = 0;
    switch (*s) {
    case 'p':
    case 'P':
        switch (*++s) {
        case '-':
            esign = 1;
            /* fallthrough */
        case '+':
            s++;
        }
        if ((n = hexdig[*s]) == 0 || n > 0x19) {
            s = s1;
            break;
        }
        e1 = n - 0x10;
        while ((n = hexdig[*++s]) !=0 && n <= 0x19) {
            if (e1 & 0xf8000000)
                big = 1;
            e1 = 10*e1 + n - 0x10;
        }
        if (esign)
            e1 = -e1;
        e += e1;
    }
    *sp = (char *)s;
    if (!havedig)
        *sp = (char *)s0 - 1;
    if (zret)
        goto retz1;
    if (big) {
        if (esign) {
            switch (rounding) {
            case Round_up:
                if (sign)
                    break;
                goto ret_tiny;
            case Round_down:
                if (!sign)
                    break;
                goto ret_tiny;
            }
            goto retz;
        ret_tinyf:
            Bfree(b);
        ret_tiny:
            word0(rvp) = 0;
            word1(rvp) = 1;
            return;
        }
        switch (rounding) {
        case Round_near:
            goto ovfl1;
        case Round_up:
            if (!sign)
                goto ovfl1;
            goto ret_big;
        case Round_down:
            if (sign)
                goto ovfl1;
            goto ret_big;
        }
    ret_big:
        word0(rvp) = Big0;
        word1(rvp) = Big1;
        return;
    }
    n = (int)(s1 - s0 - 1);
    for (k = 0; n > (1 << (kshift-2)) - 1; n >>= 1)
        k++;
    b = Balloc(k);
    x = b->x;
    havedig = n = nz = 0;
    L = 0;

    while (s1 > s0) {
        if (*--s1 == '.')
            continue;
        if ((d = hexdig[*s1]))
            havedig = 1;
        else if (!havedig) {
            e += 4;
            continue;
        }
        if (n == ULbits) {
            *x++ = L;
            L = 0;
            n = 0;
        }
        L |= (d & 0x0f) << n;
        n += 4;
    }
    *x++ = L;
    b->wds = n = (int)(x - b->x);
    nb = ULbits*n - hi0bits(L);
    nbits = Nbits;
    lostbits = 0;
    x = b->x;
    if (nb > nbits) {
        n = nb - nbits;
        if (any_on(b, n)) {
            lostbits = 1;
            k = n - 1;
            if (x[k>>kshift] & 1 << (k & kmask)) {
                lostbits = 2;
                if (k > 0 && any_on(b, k))
                    lostbits = 3;
            }
        }
        rshift(b, n);
        e += n;
    } else if (nb < nbits) {
        n = nbits - nb;
        b = lshift(b, n);
        e -= n;
        x = b->x;
    }
    if (e > emax) {
    ovfl:
        Bfree(b);
    ovfl1:
        word0(rvp) = Exp_mask;
        word1(rvp) = 0;
        return;
    }
    denorm = 0;
    if (e < emin) {
        denorm = 1;
        n = emin - e;
        if (n >= nbits) {
            switch (rounding) {
            case Round_near:
                if (n == nbits && (n < 2 || lostbits || any_on(b, n-1)))
                    goto ret_tinyf;
                break;
            case Round_up:
                if (!sign)
                    goto ret_tinyf;
                break;
            case Round_down:
                if (sign)
                    goto ret_tinyf;
            }
            Bfree(b);
        retz:
        retz1:
            rvp->d = 0.;
            return;
        }
        k = n - 1;
        if (!k) {
            switch (rounding) {
            case Round_near:
                if (((b->x[0] & 3) == 3) || (lostbits && (b->x[0] & 1))) {
                    multadd(b, 1, 1);
                emin_check:
                    if (b->x[1] == (1 << (Exp_shift + 1))) {
                        rshift(b, 1);
                        e = emin;
                        goto normal;
                    }
                }
                break;
            case Round_up:
                if (!sign && (lostbits || (b->x[0] & 1))) {
                incr_denorm:
                    multadd(b, 1, 2);
                    check_denorm = 1;
                    lostbits = 0;
                    goto emin_check;
                }
                break;
            case Round_down:
                if (sign && (lostbits || (b->x[0] & 1)))
                    goto incr_denorm;
                break;
            }
        }
        if (lostbits)
            lostbits = 1;
        else if (k > 0)
            lostbits = any_on(b, k);
        else if (check_denorm)
            goto no_lostbits;

        if (x[k>>kshift] & 1 << (k & kmask))
            lostbits |= 2;
    no_lostbits:
        nbits -= n;
        rshift(b, n);
        e = emin;
    }
    if (lostbits) {
        up = 0;
        switch (rounding) {
        case Round_zero:
            break;
        case Round_near:
            if (lostbits & 2
                && (lostbits & 1) | (x[0] & 1))
                up = 1;
            break;
        case Round_up:
            up = 1 - sign;
            break;
        case Round_down:
            up = sign;
        }
        if (up) {
            k = b->wds;
            b = increment(b);
            x = b->x;
            if (!denorm && (b->wds > k
                || ((n = nbits & kmask) !=0
                && hi0bits(x[k-1]) < 32-n))) {
                rshift(b, 1);
                if (++e > Emax)
                    goto ovfl;
            }
        }
    }

    if (denorm)
        word0(rvp) = b->wds > 1 ? b->x[1] & ~0x100000 : 0;
    else {
    normal:
        word0(rvp) = (b->x[1] & ~0x100000) | ((e + 0x3ff + 52) << 20);
    }
    word1(rvp) = b->x[0];

    Bfree(b);
}

static int dshift(Bigint *b, int p2)
{
    int rv = hi0bits(b->x[b->wds-1]) - 4;
    if (p2 > 0)
        rv -= p2;
    return rv & kmask;
}

static int quorem(Bigint *b, Bigint *S)
{
    int n;
    ULong *bx, *bxe, q, *sx, *sxe;
    ULong borrow, carry, y, ys;
    ULong si, z, zs;

    n = S->wds;
#ifdef DEBUG
    if (b->wds > n)
        Bug("oversize b in quorem");
#endif
    if (b->wds < n)
        return 0;
    sx = S->x;
    sxe = sx + --n;
    bx = b->x;
    bxe = bx + n;
    q = *bxe / (*sxe + 1);
#ifdef DEBUG
    if (q > 15)
        Bug("oversized quotient in quorem");
#endif
    if (q) {
        borrow = 0;
        carry = 0;
        do {
            si = *sx++;
            ys = (si & 0xffff) * q + carry;
            zs = (si >> 16) * q + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            z = (*bx >> 16) - (zs & 0xffff) - borrow;
            borrow = (z & 0x10000) >> 16;
            Storeinc(bx, z, y);
        } while (sx <= sxe);
        if (!*bxe) {
            bx = b->x;
            while (--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    if (cmp(b, S) >= 0) {
        q++;
        borrow = 0;
        carry = 0;
        bx = b->x;
        sx = S->x;
        do {
            si = *sx++;
            ys = (si & 0xffff) + carry;
            zs = (si >> 16) + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            z = (*bx >> 16) - (zs & 0xffff) - borrow;
            borrow = (z & 0x10000) >> 16;
            Storeinc(bx, z, y);
        } while (sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if (!*bxe) {
            while (--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    return q;
}

static double sulp(U *x, BCinfo *bc)
{
    U u;
    double rv;
    int i;

    rv = ulp(x);
    if (!bc->scale || (i = 2*P + 1 - ((word0(x) & Exp_mask) >> Exp_shift)) <= 0)
        return rv;
    word0(&u) = Exp_1 + (i << Exp_shift);
    word1(&u) = 0;
    return rv * u.d;
}

static void bigcomp(U *rv, const char *s0, BCinfo *bc)
{
    Bigint *b, *d;
    int b2, bbits, d2, dd = 0, dig, dsign, i, j, nd, nd0, p2, p5, speccase;

    dsign = bc->dsign;
    nd = bc->nd;
    nd0 = bc->nd0;
    p5 = nd + bc->e0 - 1;
    speccase = 0;
    if (rv->d == 0.) {
        b = i2b(1);
        p2 = Emin - P + 1;
        bbits = 1;
        word0(rv) = (P+2) << Exp_shift;
        i = 0;
        speccase = 1;
        --p2;
        dsign = 0;
        goto have_i;
    } else
        b = d2b(rv, &p2, &bbits);
    p2 -= bc->scale;
    i = P - bbits;
    if (i > (j = P - Emin - 1 + p2)) {
        i = j;
    }
    b = lshift(b, ++i);
    b->x[0] |= 1;
have_i:
    p2 -= p5 + i;
    d = i2b(1);
    if (p5 > 0)
        d = pow5mult(d, p5);
    else if (p5 < 0)
        b = pow5mult(b, -p5);
    if (p2 > 0) {
        b2 = p2;
        d2 = 0;
    } else {
        b2 = 0;
        d2 = -p2;
    }
    i = dshift(d, d2);
    if ((b2 += i) > 0)
        b = lshift(b, b2);
    if ((d2 += i) > 0)
        d = lshift(d, d2);

    if (!(dig = quorem(b, d))) {
        b = multadd(b, 10, 0);
        dig = quorem(b, d);
    }

    for (i = 0; i < nd0; ) {
        if ((dd = s0[i++] - '0' - dig))
            goto ret;
        if (!b->x[0] && b->wds == 1) {
            if (i < nd)
                dd = 1;
            goto ret;
        }
        b = multadd(b, 10, 0);
        dig = quorem(b, d);
    }
    for (j = bc->dp1; i++ < nd;) {
        if ((dd = s0[j++] - '0' - dig))
            goto ret;
        if (!b->x[0] && b->wds == 1) {
            if (i < nd)
                dd = 1;
            goto ret;
        }
        b = multadd(b, 10, 0);
        dig = quorem(b, d);
    }
    if (dig > 0 || b->x[0] || b->wds > 1)
        dd = -1;
ret:
    Bfree(b);
    Bfree(d);

    if (speccase) {
        if (dd <= 0)
            rv->d = 0.;
    } else if (dd < 0) {
        if (!dsign)
            retlow1:
        dval(rv) -= sulp(rv, bc);
    } else if (dd > 0) {
        if (dsign) {
        rethi1:
            dval(rv) += sulp(rv, bc);
        }
    } else {
        if ((j = ((word0(rv) & Exp_mask) >> Exp_shift) - bc->scale) <= 0) {
            i = 1 - j;
            if (i <= 31) {
                if (word1(rv) & (0x1 << i))
                    goto odd;
            } else if (word0(rv) & (0x1 << (i-32)))
                goto odd;
        } else if (word1(rv) & 1) {
        odd:
            if (dsign)
                goto rethi1;
            goto retlow1;
        }
    }
}

f64 Nst_strtod(const char *s00, char **se)
{
    int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, e, e1;
    int esign, i, j, k, nd, nd0, nf, nz, nz0, nz1, sign;
    const char *s, *s0, *s1;
    double aadj, aadj1;
    Long L;
    U aadj2, adj, rv, rv0;
    ULong y, z;
    BCinfo bc;
    Bigint *bb = NULL, *bb1, *bd = NULL, *bd0, *bs = NULL, *delta = NULL;
    ULong Lsb, Lsb1;
    int req_bigcomp = 0;

    sign = nz0 = nz1 = nz = bc.dplen = bc.uflchk = 0;
    dval(&rv) = 0.;
    for (s = s00;; s++) switch (*s) {
    case '-':
        sign = 1;
        /* fallthrough */
    case '+':
        if (*++s)
            goto break2;
        /* fallthrough */
    case 0:
        goto ret0;
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
        continue;
    default:
        goto break2;
    }
break2:
    if (*s == '0') {
        switch (s[1]) {
        case 'x':
        case 'X':
            gethex(&s, &rv, 1, sign);
            goto ret;
        }
        nz0 = 1;
        while (*++s == '0');
        if (!*s)
            goto ret;
    }
    s0 = s;
    nd = nf = 0;
    y = z = 0;
    for (; (c = *s) >= '0' && c <= '9'; nd++, s++)
        if (nd < 9)
            y = 10*y + c - '0';
        else if (nd < DBL_DIG + 2)
            z = 10*z + c - '0';
    nd0 = nd;
    bc.dp0 = bc.dp1 = (int)(s - s0);
    for (s1 = s; s1 > s0 && *--s1 == '0'; )
        ++nz1;
    if (c == '.') {
        c = *++s;
        bc.dp1 = (int)(s - s0);
        bc.dplen = bc.dp1 - bc.dp0;
        if (!nd) {
            for (; c == '0'; c = *++s)
                nz++;
            if (c > '0' && c <= '9') {
                bc.dp0 = (int)(s0 - s);
                bc.dp1 = bc.dp0 + bc.dplen;
                s0 = s;
                nf += nz;
                nz = 0;
                goto have_dig;
            }
            goto dig_done;
        }
        for (; c >= '0' && c <= '9'; c = *++s) {
        have_dig:
            nz++;
            if (c -= '0') {
                nf += nz;
                i = 1;
                for (; i < nz; ++i) {
                    if (nd++ < 9)
                        y *= 10;
                    else if (nd <= DBL_DIG + 2)
                        z *= 10;
                }
                if (nd++ < 9)
                    y = 10*y + c;
                else if (nd <= DBL_DIG + 2)
                    z = 10*z + c;
                nz = nz1 = 0;
            }
        }
    }
dig_done:
    e = 0;
    if (c == 'e' || c == 'E') {
        if (!nd && !nz && !nz0) {
            goto ret0;
        }
        s00 = s;
        esign = 0;
        switch (c = *++s) {
        case '-':
            esign = 1;
            /* fallthrough */
        case '+':
            c = *++s;
        }
        if (c >= '0' && c <= '9') {
            while (c == '0')
                c = *++s;
            if (c > '0' && c <= '9') {
                L = c - '0';
                while ((c = *++s) >= '0' && c <= '9') {
                    if (L <= 19999)
                        L = 10*L + c - '0';
                }
                if (L > 19999)
                    e = 19999;
                else
                    e = (int)L;
                if (esign)
                    e = -e;
            } else
                e = 0;
        } else
            s = s00;
    }
    if (!nd) {
        if (!nz && !nz0) {
            if (!bc.dplen)
                switch (c) {
                case 'i':
                case 'I':
                    if (match(&s, "nf")) {
                        --s;
                        if (!match(&s, "inity"))
                            ++s;
                        word0(&rv) = 0x7ff00000;
                        word1(&rv) = 0;
                        goto ret;
                    }
                    break;
                case 'n':
                case 'N':
                    if (match(&s, "an")) {
                        word0(&rv) = 0x7ff80000;
                        word1(&rv) = 0;
                        if (*s == '(') /*)*/
                            hexnan(&rv, &s);
                        goto ret;
                    }
                }
        ret0:
            s = s00;
            sign = 0;
        }
        goto ret;
    }
    bc.e0 = e1 = e -= nf;

    if (!nd0)
        nd0 = nd;
    k = nd < DBL_DIG + 2 ? nd : DBL_DIG + 2;
    dval(&rv) = y;
    if (k > 9)
        dval(&rv) = tens[k - 9] * dval(&rv) + z;
    bd0 = 0;
    if (nd <= DBL_DIG) {
        if (!e)
            goto ret;
        if (e > 0) {
            if (e <= Ten_pmax) {
                rounded_product(dval(&rv), tens[e]);
                goto ret;
            }
            i = DBL_DIG - nd;
            if (e <= Ten_pmax + i) {
                e -= i;
                dval(&rv) *= tens[i];
                rounded_product(dval(&rv), tens[e]);
                goto ret;
            }
        } else if (e >= -Ten_pmax) {
            rounded_quotient(dval(&rv), tens[-e]);
            goto ret;
        }
    }
    e1 += nd - k;

    bc.scale = 0;

    /* Get starting approximation = rv * 10**e1 */

    if (e1 > 0) {
        if ((i = e1 & 15))
            dval(&rv) *= tens[i];
        if (e1 &= ~15) {
            if (e1 > DBL_MAX_10_EXP) {
            ovfl:
                /* Can't trust HUGE_VAL */
                word0(&rv) = Exp_mask;
                word1(&rv) = 0;
            range_err:
                if (bd0) {
                    Bfree(bb);
                    Bfree(bd);
                    Bfree(bs);
                    Bfree(bd0);
                    Bfree(delta);
                }
                goto ret;
            }
            e1 >>= 4;
            for (j = 0; e1 > 1; j++, e1 >>= 1)
                if (e1 & 1)
                    dval(&rv) *= bigtens[j];
            word0(&rv) -= P*Exp_msk1;
            dval(&rv) *= bigtens[j];
            if ((z = word0(&rv) & Exp_mask)
             > Exp_msk1*(DBL_MAX_EXP+Bias-P))
                goto ovfl;
            if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
                word0(&rv) = Big0;
                word1(&rv) = Big1;
            } else
                word0(&rv) += P*Exp_msk1;
        }
    } else if (e1 < 0) {
        e1 = -e1;
        if ((i = e1 & 15))
            dval(&rv) /= tens[i];
        if (e1 >>= 4) {
            if (e1 >= 1 << n_bigtens)
                goto undfl;
            if (e1 & Scale_Bit)
                bc.scale = 2*P;
            for (j = 0; e1 > 0; j++, e1 >>= 1)
                if (e1 & 1)
                    dval(&rv) *= tinytens[j];
            if (bc.scale && (j = 2*P + 1 - ((word0(&rv) & Exp_mask)
                >> Exp_shift)) > 0) {
                if (j >= 32) {
                    if (j > 54)
                        goto undfl;
                    word1(&rv) = 0;
                    if (j >= 53)
                        word0(&rv) = (P+2)*Exp_msk1;
                    else
                        word0(&rv) &= 0xffffffff << (j-32);
                } else
                    word1(&rv) &= 0xffffffff << j;
            }
            if (!dval(&rv)) {
            undfl:
                dval(&rv) = 0.;
                goto range_err;
            }
        }
    }

    bc.nd = nd - nz1;
    bc.nd0 = nd0;
    if (nd > strtod_diglim) {
        i = j = 18;
        if (i > nd0)
            j += bc.dplen;
        for (;;) {
            if (--j < bc.dp1 && j >= bc.dp0)
                j = bc.dp0 - 1;
            if (s0[j] != '0')
                break;
            --i;
        }
        e += nd - i;
        nd = i;
        if (nd0 > nd)
            nd0 = nd;
        if (nd < 9) {
            y = 0;
            for (i = 0; i < nd0; ++i)
                y = 10*y + s0[i] - '0';
            for (j = bc.dp1; i < nd; ++i)
                y = 10*y + s0[j++] - '0';
        }
    }
    bd0 = s2b(s0, nd0, nd, y, bc.dplen);

    for (;;) {
        bd = Balloc(bd0->k);
        Bcopy(bd, bd0);
        bb = d2b(&rv, &bbe, &bbbits);
        bs = i2b(1);

        if (e >= 0) {
            bb2 = bb5 = 0;
            bd2 = bd5 = e;
        } else {
            bb2 = bb5 = -e;
            bd2 = bd5 = 0;
        }
        if (bbe >= 0)
            bb2 += bbe;
        else
            bd2 -= bbe;
        bs2 = bb2;
        Lsb = LSB;
        Lsb1 = 0;
        j = bbe - bc.scale;
        i = j + bbbits - 1;
        j = P + 1 - bbbits;
        if (i < Emin) {
            i = Emin - i;
            j -= i;
            if (i < 32)
                Lsb <<= i;
            else if (i < 52)
                Lsb1 = Lsb << (i-32);
            else
                Lsb1 = Exp_mask;
        }
        bb2 += j;
        bd2 += j;
        bd2 += bc.scale;
        i = bb2 < bd2 ? bb2 : bd2;
        if (i > bs2)
            i = bs2;
        if (i > 0) {
            bb2 -= i;
            bd2 -= i;
            bs2 -= i;
        }
        if (bb5 > 0) {
            bs = pow5mult(bs, bb5);
            bb1 = mult(bs, bb);
            Bfree(bb);
            bb = bb1;
        }
        if (bb2 > 0)
            bb = lshift(bb, bb2);
        if (bd5 > 0)
            bd = pow5mult(bd, bd5);
        if (bd2 > 0)
            bd = lshift(bd, bd2);
        if (bs2 > 0)
            bs = lshift(bs, bs2);
        delta = diff(bb, bd);
        bc.dsign = delta->sign;
        delta->sign = 0;
        i = cmp(delta, bs);
        if (bc.nd > nd && i <= 0) {
            if (bc.dsign) {
                req_bigcomp = 1;
                break;
            }
            i = -1;
        }

        if (i < 0) {
            if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask
                || (word0(&rv) & Exp_mask) <= (2*P+1)*Exp_msk1
                ) {
                break;
            }
            if (!delta->x[0] && delta->wds <= 1) {
                break;
            }
            delta = lshift(delta, Log2P);
            if (cmp(delta, bs) > 0)
                goto drop_down;
            break;
        }
        if (i == 0) {
            if (bc.dsign) {
                if ((word0(&rv) & Bndry_mask1) == Bndry_mask1
                    &&  word1(&rv) == (
                    (bc.scale && (y = word0(&rv) & Exp_mask) <= 2*P*Exp_msk1)
                    ? (0xffffffff & (0xffffffff << (2*P+1-(y>>Exp_shift)))) :
                    0xffffffff)) {
                    if (word0(&rv) == Big0 && word1(&rv) == Big1)
                        goto ovfl;
                    word0(&rv) = (word0(&rv) & Exp_mask) + Exp_msk1;
                    word1(&rv) = 0;
                    bc.dsign = 0;
                    break;
                }
            } else if (!(word0(&rv) & Bndry_mask) && !word1(&rv)) {
            drop_down:
                if (bc.scale) {
                    L = word0(&rv) & Exp_mask;
                    if (L <= (2*P+1)*Exp_msk1) {
                        if (L > (P+2)*Exp_msk1)
                            /* round even ==> */
                            /* accept rv */
                            break;
                        /* rv = smallest denormal */
                        if (bc.nd >nd) {
                            bc.uflchk = 1;
                            break;
                        }
                        goto undfl;
                    }
                }
                L = (word0(&rv) & Exp_mask) - Exp_msk1;
                word0(&rv) = L | Bndry_mask1;
                word1(&rv) = 0xffffffff;
                if (bc.nd > nd)
                    goto cont;
                break;
            }
            if (Lsb1) {
                if (!(word0(&rv) & Lsb1))
                    break;
            } else if (!(word1(&rv) & Lsb))
                break;
            if (bc.dsign)
                dval(&rv) += sulp(&rv, &bc);
            else {
                dval(&rv) -= sulp(&rv, &bc);
                if (!dval(&rv)) {
                    if (bc.nd >nd) {
                        bc.uflchk = 1;
                        break;
                    }
                    goto undfl;
                }
            }
            bc.dsign = 1 - bc.dsign;
            break;
        }
        if ((aadj = ratio(delta, bs)) <= 2.) {
            if (bc.dsign)
                aadj = aadj1 = 1.;
            else if (word1(&rv) || word0(&rv) & Bndry_mask) {
                if (word1(&rv) == Tiny1 && !word0(&rv)) {
                    if (bc.nd >nd) {
                        bc.uflchk = 1;
                        break;
                    }
                    goto undfl;
                }
                aadj = 1.;
                aadj1 = -1.;
            } else {
                if (aadj < 2./FLT_RADIX)
                    aadj = 1./FLT_RADIX;
                else
                    aadj *= 0.5;
                aadj1 = -aadj;
            }
        } else {
            aadj *= 0.5;
            aadj1 = bc.dsign ? aadj : -aadj;
        }
        y = word0(&rv) & Exp_mask;

        if (y == Exp_msk1*(DBL_MAX_EXP+Bias-1)) {
            dval(&rv0) = dval(&rv);
            word0(&rv) -= P*Exp_msk1;
            adj.d = aadj1 * ulp(&rv);
            dval(&rv) += adj.d;
            if ((word0(&rv) & Exp_mask) >=
                Exp_msk1*(DBL_MAX_EXP+Bias-P)) {
                if (word0(&rv0) == Big0 && word1(&rv0) == Big1)
                    goto ovfl;
                word0(&rv) = Big0;
                word1(&rv) = Big1;
                goto cont;
            } else
                word0(&rv) += P*Exp_msk1;
        } else {
            if (bc.scale && y <= 2*P*Exp_msk1) {
                if (aadj <= 0x7fffffff) {
                    if ((z = (uint)aadj) <= 0)
                        z = 1;
                    aadj = z;
                    aadj1 = bc.dsign ? aadj : -aadj;
                }
                dval(&aadj2) = aadj1;
                word0(&aadj2) += (2*P+1)*Exp_msk1 - y;
                aadj1 = dval(&aadj2);
                adj.d = aadj1 * ulp(&rv);
                dval(&rv) += adj.d;
                if (rv.d == 0.) {
                    req_bigcomp = 1;
                    break;
                }
            } else {
                adj.d = aadj1 * ulp(&rv);
                dval(&rv) += adj.d;
            }
        }
        z = word0(&rv) & Exp_mask;
        if (bc.nd == nd) {
            if (!bc.scale) {
                if (y == z) {
                    L = (Long)aadj;
                    aadj -= L;
                    if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask) {
                        if (aadj < .4999999 || aadj > .5000001)
                            break;
                    } else if (aadj < .4999999/FLT_RADIX)
                        break;
                }
            }
        }
    cont:
        Bfree(bb);
        Bfree(bd);
        Bfree(bs);
        Bfree(delta);
    }
    Bfree(bb);
    Bfree(bd);
    Bfree(bs);
    Bfree(bd0);
    Bfree(delta);
    if (req_bigcomp) {
        bd0 = 0;
        bc.e0 += nz1;
        bigcomp(&rv, s0, &bc);
        y = word0(&rv) & Exp_mask;
        if (y == Exp_mask)
            goto ovfl;
        if (y == 0 && rv.d == 0.)
            goto undfl;
    }
    if (bc.scale) {
        word0(&rv0) = Exp_1 - 2*P*Exp_msk1;
        word1(&rv0) = 0;
        dval(&rv) *= dval(&rv0);
    }
ret:
    if (se)
        *se = (char *)s;
    return sign ? -dval(&rv) : dval(&rv);
}

static char *rv_alloc(int i)
{
    int j, k, *r;

    j = sizeof(ULong);
    for (k = 0;
         (int)(sizeof(Bigint) - sizeof(ULong) - sizeof(int) + j) <= i;
         j <<= 1)
        k++;
    r = (int *)Balloc(k);
    *r = k;
    return (char *)(r+1);
}

static char *nrv_alloc(const char *s, char *s0, size_t s0len, char **rve, int n)
{
    char *rv, *t;

    if (!s0)
        s0 = rv_alloc(n);
    else if ((isize)s0len <= n) {
        rv = 0;
        t = rv + n;
        goto rve_chk;
    }
    t = rv = s0;
    while ((*t = *s++))
        ++t;
rve_chk:
    if (rve)
        *rve = t;
    return rv;
}

void Nst_freedtoa(char *s)
{
    Bigint *b = (Bigint *)((int *)s - 1);
    b->maxwds = 1 << (b->k = *(int *)b);
    Bfree(b);
}

char *Nst_dtoa(f64 dd, int mode, int ndigits, int *decpt, int *sign, char **rve)
{
    char *buf = NULL;
    size_t blen = 0;
    int bbits, b2, b5, be, dig, i, ilim, ilim1,
        j, j1, k, leftright, m2, m5, s2, s5, spec_case;
    int denorm;
    Bigint *b, *b1, *delta, *mlo = 0, *mhi = 0, *S;
    U u;
    char *s;
    ULong x;
    Long L;
    U d2, eps;
    double ds;
    int ieps, ilim0, k0, k_check, try_quick;
    U eps1;

    u.d = dd;
    if (word0(&u) & Sign_bit) {
        /* set sign for everything, including 0's and NaNs */
        if (sign)
            *sign = 1;
        word0(&u) &= ~Sign_bit; /* clear sign bit */
    } else if (sign)
        *sign = 0;

    if ((word0(&u) & Exp_mask) == Exp_mask) {
        /* Infinity or NaN */
        *decpt = 9999;
        if (!word1(&u) && !(word0(&u) & 0xfffff))
            return nrv_alloc("Infinity", buf, blen, rve, 8);
        return nrv_alloc("NaN", buf, blen, rve, 3);
    }

    if (!dval(&u)) {
        *decpt = 1;
        return nrv_alloc("0", buf, blen, rve, 1);
    }

    b = d2b(&u, &be, &bbits);
    if ((i = (int)(word0(&u) >> Exp_shift & (Exp_mask>>Exp_shift)))) {
        dval(&d2) = dval(&u);
        word0(&d2) &= Frac_mask1;
        word0(&d2) |= Exp_11;

        i -= Bias;
        denorm = 0;
    } else {
        /* d is denormalized */

        i = bbits + be + (Bias + (P-1) - 1);
        x = i > 32 ? word0(&u) << (64 - i) | word1(&u) >> (i - 32)
            : word1(&u) << (32 - i);
        dval(&d2) = x;
        word0(&d2) -= 31*Exp_msk1; /* adjust exponent */
        i -= (Bias + (P-1) - 1) + 1;
        denorm = 1;
    }
    ds = (dval(&d2)-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
    k = (int)ds;
    if (ds < 0. && ds != k)
        k--; /* want k = floor(ds) */
    k_check = 1;
    if (k >= 0 && k <= Ten_pmax) {
        if (dval(&u) < tens[k])
            k--;
        k_check = 0;
    }
    j = bbits - i - 1;
    if (j >= 0) {
        b2 = 0;
        s2 = j;
    } else {
        b2 = -j;
        s2 = 0;
    }
    if (k >= 0) {
        b5 = 0;
        s5 = k;
        s2 += k;
    } else {
        b2 -= k;
        b5 = -k;
        s5 = 0;
    }
    if (mode < 0 || mode > 9)
        mode = 0;

    try_quick = 1;

    if (mode > 5) {
        mode -= 4;
        try_quick = 0;
    }
    leftright = 1;
    ilim = ilim1 = -1; /* Values for cases 0 and 1; done here to */
    /* silence erroneous "gcc -Wall" warning. */
    switch (mode) {
    case 0:
    case 1:
        i = 18;
        ndigits = 0;
        break;
    case 2:
        leftright = 0;
        /* fallthrough */
    case 4:
        if (ndigits <= 0)
            ndigits = 1;
        ilim = ilim1 = i = ndigits;
        break;
    case 3:
        leftright = 0;
        /* fallthrough */
    case 5:
        i = ndigits + k + 1;
        ilim = i;
        ilim1 = i - 1;
        if (i <= 0)
            i = 1;
    }
    if (!buf) {
        buf = rv_alloc(i);
        blen = sizeof(Bigint) + ((1 << ((int *)buf)[-1]) - 1)*sizeof(ULong) - sizeof(int);
    } else if ((isize)blen <= i) {
        buf = 0;
        if (rve)
            *rve = buf + i;
        return buf;
    }
    s = buf;

    /* Check for special case that d is a normalized power of 2. */

    spec_case = 0;
    if (mode < 2 || (leftright)) {
        if (!word1(&u) && !(word0(&u) & Bndry_mask)
            && word0(&u) & (Exp_mask & ~Exp_msk1)
            ) {
            /* The special case */
            spec_case = 1;
        }
    }

    if (ilim >= 0 && ilim <= Quick_max && try_quick) {

        /* Try to get by with floating-point arithmetic. */

        i = 0;
        dval(&d2) = dval(&u);
        j1 = -(k0 = k);
        ilim0 = ilim;
        ieps = 2; /* conservative */
        if (k > 0) {
            ds = tens[k&0xf];
            j = k >> 4;
            if (j & Bletch) {
                /* prevent overflows */
                j &= Bletch - 1;
                dval(&u) /= bigtens[n_bigtens-1];
                ieps++;
            }
            for (; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    ds *= bigtens[i];
                }
            dval(&u) /= ds;
        } else if (j1 > 0) {
            dval(&u) *= tens[j1 & 0xf];
            for (j = j1 >> 4; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    dval(&u) *= bigtens[i];
                }
        }
        if (k_check && dval(&u) < 1. && ilim > 0) {
            if (ilim1 <= 0)
                goto fast_failed;
            ilim = ilim1;
            k--;
            dval(&u) *= 10.;
            ieps++;
        }
        dval(&eps) = ieps*dval(&u) + 7.;
        word0(&eps) -= (P-1)*Exp_msk1;
        if (ilim == 0) {
            S = mhi = 0;
            dval(&u) -= 5.;
            if (dval(&u) > dval(&eps))
                goto one_digit;
            if (dval(&u) < -dval(&eps))
                goto no_digits;
            goto fast_failed;
        }
        if (leftright) {
            /* Use Steele & White method of only
             * generating digits needed.
             */
            dval(&eps) = 0.5/tens[ilim-1] - dval(&eps);
            if (j1 >= 307) {
                eps1.d = 1.01e256; /* 1.01 allows roundoff in the next few lines */
                word0(&eps1) -= Exp_msk1 * (Bias+P-1);
                dval(&eps1) *= tens[j1 & 0xf];
                for (i = 0, j = (j1-256) >> 4; j; j >>= 1, i++)
                    if (j & 1)
                        dval(&eps1) *= bigtens[i];
                if (eps.d < eps1.d)
                    eps.d = eps1.d;
                if (10. - u.d < 10.*eps.d && eps.d < 1.) {
                    /* eps.d < 1. excludes trouble with the tiniest denormal */
                    *s++ = '1';
                    ++k;
                    goto ret1;
                }
            }
            for (i = 0;;) {
                L = (int)(dval(&u));
                dval(&u) -= L;
                *s++ = (char)('0' + (int)L);
                if (1. - dval(&u) < dval(&eps))
                    goto bump_up;
                if (dval(&u) < dval(&eps))
                    goto retc;
                if (++i >= ilim)
                    break;
                dval(&eps) *= 10.;
                dval(&u) *= 10.;
            }
        } else {
            /* Generate ilim digits, then fix them up. */
            dval(&eps) *= tens[ilim-1];
            for (i = 1;; i++, dval(&u) *= 10.) {
                L = (Long)(dval(&u));
                if (!(dval(&u) -= L))
                    ilim = i;
                *s++ = (char)('0' + (int)L);
                if (i == ilim) {
                    if (dval(&u) > 0.5 + dval(&eps))
                        goto bump_up;
                    else if (dval(&u) < 0.5 - dval(&eps))
                        goto retc;
                    break;
                }
            }
        }
    fast_failed:
        s = buf;
        dval(&u) = dval(&d2);
        k = k0;
        ilim = ilim0;
    }

    /* Do we have a "small" integer? */

    if (be >= 0 && k <= Int_max) {
        /* Yes. */
        ds = tens[k];
        if (ndigits < 0 && ilim <= 0) {
            S = mhi = 0;
            if (ilim < 0 || dval(&u) <= 5*ds)
                goto no_digits;
            goto one_digit;
        }
        for (i = 1;; i++, dval(&u) *= 10.) {
            L = (Long)(dval(&u) / ds);
            dval(&u) -= L*ds;
            *s++ = (char)('0' + (int)L);
            if (!dval(&u)) {
                break;
            }
            if (i == ilim) {
                dval(&u) += dval(&u);
                if (dval(&u) > ds || (dval(&u) == ds && L & 1)) {
                bump_up:
                    while (*--s == '9')
                        if (s == buf) {
                            k++;
                            *s = '0';
                            break;
                        }
                    ++*s++;
                }
                break;
            }
        }
        goto retc;
    }
    m2 = b2;
    m5 = b5;
    mhi = mlo = 0;
    if (leftright) {
        i = denorm ? be + (Bias + (P-1) - 1 + 1) : 1 + P - bbits;
        b2 += i;
        s2 += i;
        mhi = i2b(1);
    }
    if (m2 > 0 && s2 > 0) {
        i = m2 < s2 ? m2 : s2;
        b2 -= i;
        m2 -= i;
        s2 -= i;
    }
    if (b5 > 0) {
        if (leftright) {
            if (m5 > 0) {
                mhi = pow5mult(mhi, m5);
                b1 = mult(mhi, b);
                Bfree(b);
                b = b1;
            }
            if ((j = b5 - m5))
                b = pow5mult(b, j);
        } else
            b = pow5mult(b, b5);
    }
    S = i2b(1);
    if (s5 > 0)
        S = pow5mult(S, s5);

    if (spec_case) {
        b2 += Log2P;
        s2 += Log2P;
    }

    i = dshift(S, s2);
    b2 += i;
    m2 += i;
    s2 += i;
    if (b2 > 0)
        b = lshift(b, b2);
    if (s2 > 0)
        S = lshift(S, s2);
    if (k_check) {
        if (cmp(b, S) < 0) {
            k--;
            b = multadd(b, 10, 0);
            if (leftright)
                mhi = multadd(mhi, 10, 0);
            ilim = ilim1;
        }
    }
    if (ilim <= 0 && (mode == 3 || mode == 5)) {
        if (ilim < 0 || cmp(b, S = multadd(S, 5, 0)) <= 0) {
        no_digits:
            k = -1 - ndigits;
            goto ret;
        }
    one_digit:
        *s++ = '1';
        ++k;
        goto ret;
    }
    if (leftright) {
        if (m2 > 0)
            mhi = lshift(mhi, m2);

        mlo = mhi;
        if (spec_case) {
            mhi = Balloc(mhi->k);
            Bcopy(mhi, mlo);
            mhi = lshift(mhi, Log2P);
        }

        for (i = 1;; i++) {
            dig = quorem(b, S) + '0';
            j = cmp(b, mlo);
            delta = diff(S, mhi);
            j1 = delta->sign ? 1 : cmp(b, delta);
            Bfree(delta);
            if (j1 == 0 && mode != 1 && !(word1(&u) & 1)) {
                if (dig == '9')
                    goto round_9_up;
                if (j > 0)
                    dig++;
                *s++ = (char)dig;
                goto ret;
            }
            if (j < 0 || (j == 0 && mode != 1 && !(word1(&u) & 1))) {
                if (!b->x[0] && b->wds <= 1) {
                    goto accept_dig;
                }
                if (j1 > 0) {
                    b = lshift(b, 1);
                    j1 = cmp(b, S);
                    if ((j1 > 0 || (j1 == 0 && dig & 1))
                        && dig++ == '9')
                        goto round_9_up;
                }
            accept_dig:
                *s++ = (char)dig;
                goto ret;
            }
            if (j1 > 0) {
                if (dig == '9') { /* possible if i == 1 */
                round_9_up:
                    *s++ = '9';
                    goto roundoff;
                }
                *s++ = (char)(dig + 1);
                goto ret;
            }
            *s++ = (char)dig;
            if (i == ilim)
                break;
            b = multadd(b, 10, 0);
            if (mlo == mhi)
                mlo = mhi = multadd(mhi, 10, 0);
            else {
                mlo = multadd(mlo, 10, 0);
                mhi = multadd(mhi, 10, 0);
            }
        }
    } else {
        for (i = 1;; i++) {
            dig = quorem(b, S) + '0';
            *s++ = (char)dig;
            if (!b->x[0] && b->wds <= 1) {
                goto ret;
            }
            if (i >= ilim)
                break;
            b = multadd(b, 10, 0);
        }
    }

    b = lshift(b, 1);
    j = cmp(b, S);
    if (j > 0 || (j == 0 && dig & 1)) {
    roundoff:
        while (*--s == '9')
            if (s == buf) {
                k++;
                *s++ = '1';
                goto ret;
            }
        ++*s++;
    }
ret:
    Bfree(S);
    if (mhi) {
        if (mlo && mlo != mhi)
            Bfree(mlo);
        Bfree(mhi);
    }
retc:
    while (s > buf && s[-1] == '0')
        --s;
ret1:
    if (b)
        Bfree(b);
    *s = 0;
    *decpt = k + 1;
    if (rve)
        *rve = s;
    return buf;
}
