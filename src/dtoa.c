/****************************************************************
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
 ***************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").  */

/* On a machine with IEEE extended-precision registers, it is
 * necessary to specify double-precision (53-bit) rounding precision
 * before invoking strtod or dtoa.  If the machine uses (the equivalent
 * of) Intel 80x87 arithmetic, the call
 *  _control87(PC_53, MCW_PC);
 * does this with many compilers.  Whether this or another call is
 * appropriate depends on the compiler; for this to work, it may be
 * necessary to #include "float.h" or another system-dependent header
 * file.
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 * (Note that IEEE arithmetic is disabled by gcc's -ffast-math flag.)
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *  1. We only require IEEE, IBM, or VAX double-precision
 *      arithmetic (not IEEE double-extended).
 *  2. We get by with floating-point arithmetic in a case that
 *      Clinger missed -- when we're computing d * 10^n
 *      for a small integer d and the integer n is not too
 *      much larger than 22 (the maximum integer k for which
 *      we can represent 10^k exactly), we may be able to
 *      compute (d*10^k) * 10^(e-k) with just one roundoff.
 *  3. Rather than a bit-at-a-time adjustment of the binary
 *      result in the hard case, we use floating-point
 *      arithmetic to determine the adjustment to within
 *      one bit; only in really hard cases do we need to
 *      compute a second residual.
 *  4. Because of 3., we don't need a large table of powers of 10
 *      for ten-to-e (just some small tables, e.g. of 10^k
 *      for 0 <= k <= 22).
 */

/* Modifications made to adapt to Nest:
 *
 * 1. Renamed added `Nst_` prefix to `dtoa`, `strtod` and `freedtoa`.
 *
 * 2. Removed static variable `dtoa_result`, use `Nst_freedtoa` to free the
 * memory allocated by `Nst_dtoa`.
 *
 *
 */

#include "dtoa.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include "mem.h"
#include <stdlib.h>
#include <string.h>

#if Nst_BYTEORDER == Nst_LITTLE_ENDIAN
#define IEEE_8087
#else
#define IEEE_MC68k
#endif

#define MALLOC Nst_raw_malloc
#define REALLOC Nst_raw_realloc
#define FREE Nst_raw_free

#define NO_BF96

#ifdef _DEBUG
#define DEBUG
#endif

#ifdef Nst_WIN
#pragma warning( disable:4706 4244 4701 )
#else
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#ifndef Long
#define Long int
#endif
#ifndef ULong
typedef unsigned Long ULong;
#endif

#ifdef DEBUG
#include <assert.h>
#include "stdio.h"
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#define Debug(x) x
int dtoa_stats[7]; /* strtod_{64,96,bigcomp},dtoa_{exact,64,96,bigcomp} */
#else
#ifndef assert
#define assert(x) /*nothing*/
#endif
#define Debug(x) /*nothing*/
#endif

#ifndef Omit_Private_Memory
#ifndef PRIVATE_MEM
#define PRIVATE_MEM 2304
#endif
#define PRIVATE_mem ((PRIVATE_MEM+sizeof(double)-1)/sizeof(double))
static double private_mem[PRIVATE_mem], *pmem_next = private_mem;
#endif

#undef IEEE_Arith
#undef Avoid_Underflow
#ifdef IEEE_MC68k
#define IEEE_Arith
#endif
#ifdef IEEE_8087
#define IEEE_Arith
#endif

#define INFNAN_CHECK

#undef Set_errno
#define Set_errno(x)

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined.
#endif

#ifndef Llong
#define Llong long long
#endif
#ifndef ULLong
#define ULLong unsigned Llong
#endif

typedef union {
    double d;
    ULong L[2];
} U;

#ifdef IEEE_8087
#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#else
#define word0(x) (x)->L[0]
#define word1(x) (x)->L[1]
#endif
#define dval(x) (x)->d
#define LLval(x) (x)->LL

#ifndef STRTOD_DIGLIM
#define STRTOD_DIGLIM 40
#endif

#ifdef DIGLIM_DEBUG
extern int strtod_diglim;
#else
#define strtod_diglim STRTOD_DIGLIM
#endif

/* The following definition of Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 * #define Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)
 */
#if defined(IEEE_8087)
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
((unsigned short *)a)[0] = (unsigned short)c, a++)
#else
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
((unsigned short *)a)[1] = (unsigned short)c, a++)
#endif

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Nbits 53
#define Bias 1023
#define Emax 1023
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Avoid_Underflow

#ifndef Flt_Rounds
#ifdef FLT_ROUNDS
#define Flt_Rounds FLT_ROUNDS
#else
#define Flt_Rounds 1
#endif
#endif /*Flt_Rounds*/

#define Rounding Flt_Rounds

#ifndef IEEE_Arith
#define ROUND_BIASED
#else
#ifdef ROUND_BIASED_without_Round_Up
#undef  ROUND_BIASED
#define ROUND_BIASED
#endif
#endif

#ifdef RND_PRODQUOT
#define rounded_product(a,b) a = rnd_prod(a, b)
#define rounded_quotient(a,b) a = rnd_quot(a, b)
extern double rnd_prod(double, double), rnd_quot(double, double);
#else
#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b
#endif

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

#ifndef Pack_32
#define Pack_32
#endif

typedef struct BCinfo BCinfo;
 struct
BCinfo { int dp0, dp1, dplen, dsign, e0, inexact, nd, nd0, rounding, scale, uflchk; };

#define FFFFFFFF 0xffffffffUL

#define Kmax 7

typedef struct Bigint {
    struct Bigint *next;
    int k, maxwds, sign, wds;
    ULong x[1];
} Bigint;

typedef struct ThInfo {
    Bigint *Freelist[Kmax+1];
    Bigint *P5s;
} ThInfo;

static ThInfo TI0;

#define freelist TI0.Freelist
#define p5s TI0.P5s

static Bigint *Balloc(int k)
{
    int x;
    Bigint *rv;
    unsigned int len;
    /* The k > Kmax case does not need ACQUIRE_DTOA_LOCK(0), */
    /* but this case seems very unlikely. */
    if (k <= Kmax && (rv = freelist[k]))
        freelist[k] = rv->next;
    else {
        x = 1 << k;
        len = (sizeof(Bigint) + (x-1)*sizeof(ULong) + sizeof(double) - 1)
            / sizeof(double);
        if (k <= Kmax && pmem_next - private_mem + len <= PRIVATE_mem) {
            rv = (Bigint*)pmem_next;
            pmem_next += len;
        } else
            rv = (Bigint*)MALLOC(len*sizeof(double));
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
            FREE((void*)v);
        else {
            v->next = freelist[v->k];
            freelist[v->k] = v;
        }
    }
}

#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign, \
y->wds*sizeof(Long) + 2*sizeof(int))

/* multiply by m and add a */
static Bigint *multadd(Bigint *b, int m, int a)
{
    int i, wds;
    ULong *x;
    ULLong carry, y;
    Bigint *b1;

    wds = b->wds;
    x = b->x;
    i = 0;
    carry = a;
    do {
        y = *x * (ULLong)m + carry;
        carry = y >> 32;
        *x++ = y & FFFFFFFF;
    } while(++i < wds);
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
    for(k = 0, y = 1; x > y; y <<= 1, k++) ;
    b = Balloc(k);
    b->x[0] = y9;
    b->wds = 1;

    i = 9;
    if (9 < nd0) {
        s += 9;
        do b = multadd(b, 10, *s++ - '0');
            while(++i < nd0);
        s += dplen;
    } else
        s += dplen + 9;
    for(; i < nd; i++)
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
    ULLong carry, z;

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
    for(x = c->x, xa = x + wc; x < xa; x++)
        *x = 0;
    xa = a->x;
    xae = xa + wa;
    xb = b->x;
    xbe = xb + wb;
    xc0 = c->x;
    for(; xb < xbe; xc0++) {
        if ((y = *xb++)) {
            x = xa;
            xc = xc0;
            carry = 0;
            do {
                z = *x++ * (ULLong)y + *xc + carry;
                carry = z >> 32;
                *xc++ = z & FFFFFFFF;
            } while(x < xae);
            *xc = carry;
        }
    }
    for(xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) ;
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
        /* first time */
        p5 = p5s = i2b(625);
        p5->next = 0;
    }
    for(;;) {
        if (k & 1) {
            b1 = mult(b, p5);
            Bfree(b);
            b = b1;
        }
        if (!(k >>= 1))
            break;
        if (!(p51 = p5->next)) {
            p51 = p5->next = mult(p5,p5);
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
    for(i = b->maxwds; n1 > i; i <<= 1)
        k1++;
    b1 = Balloc(k1);
    x1 = b1->x;
    for(i = 0; i < n; i++)
        *x1++ = 0;
    x = b->x;
    xe = x + b->wds;
    if (k &= 0x1f) {
        k1 = 32 - k;
        z = 0;
        do {
            *x1++ = *x << k | z;
            z = *x++ >> k1;
        } while(x < xe);
        if ((*x1 = z))
            ++n1;
    } else {
        do {
            *x1++ = *x++;
        } while(x < xe);
    }
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
    for(;;) {
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
    ULLong borrow, y;

    i = cmp(a,b);
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
        y = (ULLong)*xa++ - *xb++ - borrow;
        borrow = y >> 32 & (ULong)1;
        *xc++ = y & FFFFFFFF;
    } while(xb < xbe);
    while(xa < xae) {
        y = *xa++ - borrow;
        borrow = y >> 32 & (ULong)1;
        *xc++ = y & FFFFFFFF;
    }
    while(!*--xc)
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

#undef d0
#undef d1

ret_d:
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
    d0 &= 0x7fffffff;   /* clear sign bit, which we ignore */
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

static const double tens[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8,
                               1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16,
                               1e17, 1e18, 1e19, 1e20, 1e21, 1e22 };

static const double bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static const double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128,
                                   9007199254740992.*9007199254740992.e-256 };
                                   /* = 2^106 * 1e-256 */
/* The factor of 2^53 in tinytens[4] helps us avoid setting the underflow */
/* flag unnecessarily.  It leads to a song and dance at the end of strtod. */

#define Scale_Bit 0x10
#define n_bigtens 5

#undef Need_Hexdig
#ifdef INFNAN_CHECK
#ifndef No_Hex_NaN
#define Need_Hexdig
#endif
#endif

#ifndef Need_Hexdig
#ifndef NO_HEX_FP
#define Need_Hexdig
#endif
#endif

#ifdef Need_Hexdig /*{*/
#if 0
static unsigned char hexdig[256];

 static void
htinit(unsigned char *h, unsigned char *s, int inc)
{
    int i, j;
    for(i = 0; (j = s[i]) !=0; i++)
        h[j] = i + inc;
    }

 static void
hexdig_init(void)   /* Use of hexdig_init omitted 20121220 to avoid a */
            /* race condition when multiple threads are used. */
{
#define USC (unsigned char *)
    htinit(hexdig, USC "0123456789", 0x10);
    htinit(hexdig, USC "abcdef", 0x10 + 10);
    htinit(hexdig, USC "ABCDEF", 0x10 + 10);
    }
#else
static unsigned char hexdig[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    16,17,18,19,20,21,22,23,24,25, 0, 0, 0, 0, 0, 0,
    0,26,27,28,29,30,31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0,
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
#endif
#endif /* } Need_Hexdig */

#ifdef INFNAN_CHECK

#ifndef NAN_WORD0
#define NAN_WORD0 0x7ff80000
#endif

#ifndef NAN_WORD1
#define NAN_WORD1 0
#endif

static int match(const char **sp, const char *t)
{
    int c, d;
    const char *s = *sp;

    while((d = *t++)) {
        if ((c = *++s) >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        if (c != d)
            return 0;
    }
    *sp = s + 1;
    return 1;
}

#ifndef No_Hex_NaN
static void hexnan(U *rvp, const char **sp)
{
    ULong c, x[2];
    const char *s;
    int c1, havedig, udx0, xshift;

    /**** if (!hexdig['0']) hexdig_init(); ****/
    x[0] = x[1] = 0;
    havedig = xshift = 0;
    udx0 = 1;
    s = *sp;
    /* allow optional initial 0x or 0X */
    while((c = *(const unsigned char*)(s+1)) && c <= ' ')
        ++s;
    if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))
        s += 2;
    while((c = *(const unsigned char*)++s)) {
        if ((c1 = hexdig[c]))
            c  = c1 & 0xf;
        else if (c <= ' ') {
            if (udx0 && havedig) {
                udx0 = 0;
                xshift = 1;
            }
            continue;
        }
#ifdef GDTOA_NON_PEDANTIC_NANCHECK
        else if (/*(*/ c == ')' && havedig) {
            *sp = s + 1;
            break;
        }
        else
            return; /* invalid form: don't change *sp */
#else
        else {
            do {
                if (/*(*/ c == ')') {
                    *sp = s + 1;
                    break;
                }
            } while((c = *++s));
            break;
        }
#endif
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
#endif /*No_Hex_NaN*/
#endif /* INFNAN_CHECK */

#ifdef Pack_32
#define ULbits 32
#define kshift 5
#define kmask 31
#else
#define ULbits 16
#define kshift 4
#define kmask 15
#endif

#if !defined(NO_HEX_FP) /*{*/
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
    } while(x < xe);
    if (b->wds >= b->maxwds) {
        b1 = Balloc(b->k+1);
        Bcopy(b1,b);
        Bfree(b);
        b = b1;
    }
    b->x[b->wds++] = 1;
    return b;
}

#endif /*}*/

#ifndef NO_HEX_FP /*{*/

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
            while(x < xe) {
                *x1++ = (y | (*x << n)) & 0xffffffff;
                y = *x++ >> k;
            }
            if ((*x1 = y) != 0)
                x1++;
        } else {
            while(x < xe)
                *x1++ = *x++;
        }
    }
    if ((b->wds = x1 - b->x) == 0)
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
    while(x > x0)
        if (*--x)
            return 1;
    return 0;
    }

/* rounding values: same as FLT_ROUNDS */
enum {
    Round_zero = 0,
    Round_near = 1,
    Round_up = 2,
    Round_down = 3
};

void gethex( const char **sp, U *rvp, int rounding, int sign)
{
    Bigint *b;
    const unsigned char *decpt, *s0, *s, *s1;
    Long e, e1;
    ULong L, lostbits, *x;
    int big, denorm, esign, havedig, k, n, nbits, up, zret;
    enum {
        emax = 0x7fe - Bias - P + 1,
        emin = Emin - P + 1
    };

    /**** if (!hexdig['0']) hexdig_init(); ****/
    havedig = 0;
    s0 = *(const unsigned char **)sp + 2;
    while(s0[havedig] == '0')
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
        while(*s == '0')
            s++;
        if (hexdig[*s])
            zret = 0;
        havedig = 1;
        s0 = s;
    }
    while(hexdig[*s])
        s++;
    if (*s == '.' && !decpt) {
        decpt = ++s;
        while(hexdig[*s])
            s++;
    }
    if (decpt)
        e = -(((Long)(s-decpt)) << 2);
 pcheck:
    s1 = s;
    big = esign = 0;
    switch(*s) {
    case 'p':
    case 'P':
        switch(*++s) {
        case '-':
            esign = 1;
            /* no break */
        case '+':
            s++;
        }
        if ((n = hexdig[*s]) == 0 || n > 0x19) {
            s = s1;
            break;
            }
        e1 = n - 0x10;
        while((n = hexdig[*++s]) !=0 && n <= 0x19) {
            if (e1 & 0xf8000000)
                big = 1;
            e1 = 10*e1 + n - 0x10;
        }
        if (esign)
            e1 = -e1;
        e += e1;
    }
    *sp = (char*)s;
    if (!havedig)
        *sp = (char*)s0 - 1;
    if (zret)
        goto retz1;
    if (big) {
        if (esign) {
            switch(rounding) {
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
            Set_errno(ERANGE);
            word0(rvp) = 0;
            word1(rvp) = 1;
            return;
        }
        switch(rounding) {
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
    n = s1 - s0 - 1;
    for(k = 0; n > (1 << (kshift-2)) - 1; n >>= 1)
        k++;
    b = Balloc(k);
    x = b->x;
    n = 0;
    L = 0;

    while(s1 > s0) {
        if (*--s1 == '.')
            continue;
        if (n == ULbits) {
            *x++ = L;
            L = 0;
            n = 0;
        }
        L |= (hexdig[*s1] & 0x0f) << n;
        n += 4;
    }
    *x++ = L;
    b->wds = n = x - b->x;
    n = ULbits*n - hi0bits(L);
    nbits = Nbits;
    lostbits = 0;
    x = b->x;
    if (n > nbits) {
        n -= nbits;
        if (any_on(b,n)) {
            lostbits = 1;
            k = n - 1;
            if (x[k>>kshift] & 1 << (k & kmask)) {
                lostbits = 2;
                if (k > 0 && any_on(b,k))
                    lostbits = 3;
            }
        }
        rshift(b, n);
        e += n;
    } else if (n < nbits) {
        n = nbits - n;
        b = lshift(b, n);
        e -= n;
        x = b->x;
    }
    if (e > emax) {
 ovfl:
        Bfree(b);
 ovfl1:
        Set_errno(ERANGE);
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
                if (n == nbits && (n < 2 || lostbits || any_on(b,n-1)))
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
            Set_errno(ERANGE);
 retz1:
            rvp->d = 0.;
            return;
        }
        k = n - 1;
        if (lostbits)
            lostbits = 1;
        else if (k > 0)
            lostbits = any_on(b,k);
        if (x[k>>kshift] & 1 << (k & kmask))
            lostbits |= 2;
        nbits -= n;
        rshift(b,n);
        e = emin;
    }
    if (lostbits) {
        up = 0;
        switch(rounding) {
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
            if (denorm) ;
            else if (b->wds > k
                     || ((n = nbits & kmask) != 0
                         && hi0bits(x[k-1]) < 32-n))
            {
                rshift(b,1);
                if (++e > Emax)
                    goto ovfl;
            }
        }
    }

    if (denorm)
        word0(rvp) = b->wds > 1 ? b->x[1] & ~0x100000 : 0;
    else
        word0(rvp) = (b->x[1] & ~0x100000) | ((e + 0x3ff + 52) << 20);
    word1(rvp) = b->x[0];

    Bfree(b);
}
#endif /*!NO_HEX_FP}*/

 static int
dshift(Bigint *b, int p2)
{
    int rv = hi0bits(b->x[b->wds-1]) - 4;
    if (p2 > 0)
        rv -= p2;
    return rv & kmask;
    }

 static int
quorem(Bigint *b, Bigint *S)
{
    int n;
    ULong *bx, *bxe, q, *sx, *sxe;
#ifdef ULLong
    ULLong borrow, carry, y, ys;
#else
    ULong borrow, carry, y, ys;
#ifdef Pack_32
    ULong si, z, zs;
#endif
#endif

    n = S->wds;
#ifdef DEBUG
    /*debug*/ if (b->wds > n)
    /*debug*/   Bug("oversize b in quorem");
#endif
    if (b->wds < n)
        return 0;
    sx = S->x;
    sxe = sx + --n;
    bx = b->x;
    bxe = bx + n;
    q = *bxe / (*sxe + 1);  /* ensure q <= true quotient */
#ifdef DEBUG
#ifdef NO_STRTOD_BIGCOMP
    /*debug*/ if (q > 9)
#else
    /* An oversized q is possible when quorem is called from bigcomp and */
    /* the input is near, e.g., twice the smallest denormalized number. */
    /*debug*/ if (q > 15)
#endif
    /*debug*/   Bug("oversized quotient in quorem");
#endif
    if (q) {
        borrow = 0;
        carry = 0;
        do {
#ifdef ULLong
            ys = *sx++ * (ULLong)q + carry;
            carry = ys >> 32;
            y = *bx - (ys & FFFFFFFF) - borrow;
            borrow = y >> 32 & (ULong)1;
            *bx++ = y & FFFFFFFF;
#else
#ifdef Pack_32
            si = *sx++;
            ys = (si & 0xffff) * q + carry;
            zs = (si >> 16) * q + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            z = (*bx >> 16) - (zs & 0xffff) - borrow;
            borrow = (z & 0x10000) >> 16;
            Storeinc(bx, z, y);
#else
            ys = *sx++ * q + carry;
            carry = ys >> 16;
            y = *bx - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            *bx++ = y & 0xffff;
#endif
#endif
            }
            while(sx <= sxe);
        if (!*bxe) {
            bx = b->x;
            while(--bxe > bx && !*bxe)
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
#ifdef ULLong
            ys = *sx++ + carry;
            carry = ys >> 32;
            y = *bx - (ys & FFFFFFFF) - borrow;
            borrow = y >> 32 & (ULong)1;
            *bx++ = y & FFFFFFFF;
#else
#ifdef Pack_32
            si = *sx++;
            ys = (si & 0xffff) + carry;
            zs = (si >> 16) + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            z = (*bx >> 16) - (zs & 0xffff) - borrow;
            borrow = (z & 0x10000) >> 16;
            Storeinc(bx, z, y);
#else
            ys = *sx++ + carry;
            carry = ys >> 16;
            y = *bx - (ys & 0xffff) - borrow;
            borrow = (y & 0x10000) >> 16;
            *bx++ = y & 0xffff;
#endif
#endif
            }
            while(sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if (!*bxe) {
            while(--bxe > bx && !*bxe)
                --n;
            b->wds = n;
            }
        }
    return q;
    }

#if defined(Avoid_Underflow) || !defined(NO_STRTOD_BIGCOMP) /*{*/
 static double
sulp(U *x, BCinfo *bc)
{
    U u;
    double rv;
    int i;

    rv = ulp(x);
    if (!bc->scale || (i = 2*P + 1 - ((word0(x) & Exp_mask) >> Exp_shift)) <= 0)
        return rv; /* Is there an example where i <= 0 ? */
    word0(&u) = Exp_1 + (i << Exp_shift);
    word1(&u) = 0;
    return rv * u.d;
    }
#endif /*}*/

#ifndef NO_STRTOD_BIGCOMP
 static void
bigcomp(U *rv, const char *s0, BCinfo *bc)
{
    Bigint *b, *d;
    int b2, bbits, d2, dd, dig, dsign, i, j, nd, nd0, p2, p5, speccase;

    dsign = bc->dsign;
    nd = bc->nd;
    nd0 = bc->nd0;
    p5 = nd + bc->e0 - 1;
    speccase = 0;
#ifndef Sudden_Underflow
    if (rv->d == 0.) {  /* special case: value near underflow-to-zero */
                /* threshold was rounded to zero */
        b = i2b(1);
        p2 = Emin - P + 1;
        bbits = 1;
#ifdef Avoid_Underflow
        word0(rv) = (P+2) << Exp_shift;
#else
        word1(rv) = 1;
#endif
        i = 0;
#ifdef Honor_FLT_ROUNDS
        if (bc->rounding == 1)
#endif
            {
            speccase = 1;
            --p2;
            dsign = 0;
            goto have_i;
            }
        }
    else
#endif
        b = d2b(rv, &p2, &bbits);
#ifdef Avoid_Underflow
    p2 -= bc->scale;
#endif
    /* floor(log2(rv)) == bbits - 1 + p2 */
    /* Check for denormal case. */
    i = P - bbits;
    if (i > (j = P - Emin - 1 + p2)) {
#ifdef Sudden_Underflow
        Bfree(b);
        b = i2b(1);
        p2 = Emin;
        i = P - 1;
#ifdef Avoid_Underflow
        word0(rv) = (1 + bc->scale) << Exp_shift;
#else
        word0(rv) = Exp_msk1;
#endif
        word1(rv) = 0;
#else
        i = j;
#endif
        }
#ifdef Honor_FLT_ROUNDS
    if (bc->rounding != 1) {
        if (i > 0)
            b = lshift(b, i);
        if (dsign)
            b = increment(b);
        }
    else
#endif
        {
        b = lshift(b, ++i);
        b->x[0] |= 1;
        }
#ifndef Sudden_Underflow
 have_i:
#endif
    p2 -= p5 + i;
    d = i2b(1);
    /* Arrange for convenient computation of quotients:
    * shift left if necessary so divisor has 4 leading 0 bits.
    */
    if (p5 > 0)
        d = pow5mult(d, p5);
    else if (p5 < 0)
        b = pow5mult(b, -p5);
    if (p2 > 0) {
        b2 = p2;
        d2 = 0;
        }
    else {
        b2 = 0;
        d2 = -p2;
        }
    i = dshift(d, d2);
    if ((b2 += i) > 0)
        b = lshift(b, b2);
    if ((d2 += i) > 0)
        d = lshift(d, d2);

    /* Now b/d = exactly half-way between the two floating-point values */
    /* on either side of the input string.  Compute first digit of b/d. */

    if (!(dig = quorem(b,d))) {
        b = multadd(b, 10, 0);  /* very unlikely */
        dig = quorem(b,d);
        }

    /* Compare b/d with s0 */

    for(i = 0; i < nd0; ) {
        if ((dd = s0[i++] - '0' - dig))
            goto ret;
        if (!b->x[0] && b->wds == 1) {
            if (i < nd)
                dd = 1;
            goto ret;
            }
        b = multadd(b, 10, 0);
        dig = quorem(b,d);
        }
    for(j = bc->dp1; i++ < nd;) {
        if ((dd = s0[j++] - '0' - dig))
            goto ret;
        if (!b->x[0] && b->wds == 1) {
            if (i < nd)
                dd = 1;
            goto ret;
            }
        b = multadd(b, 10, 0);
        dig = quorem(b,d);
        }
    if (dig > 0 || b->x[0] || b->wds > 1)
        dd = -1;
 ret:
    Bfree(b);
    Bfree(d);
#ifdef Honor_FLT_ROUNDS
    if (bc->rounding != 1) {
        if (dd < 0) {
            if (bc->rounding == 0) {
                if (!dsign)
                    goto retlow1;
                }
            else if (dsign)
                goto rethi1;
            }
        else if (dd > 0) {
            if (bc->rounding == 0) {
                if (dsign)
                    goto rethi1;
                goto ret1;
                }
            if (!dsign)
                goto rethi1;
            dval(rv) += 2.*sulp(rv,bc);
            }
        else {
            bc->inexact = 0;
            if (dsign)
                goto rethi1;
            }
        }
    else
#endif
    if (speccase) {
        if (dd <= 0)
            rv->d = 0.;
        }
    else if (dd < 0) {
        if (!dsign) /* does not happen for round-near */
retlow1:
            dval(rv) -= sulp(rv,bc);
        }
    else if (dd > 0) {
        if (dsign) {
 rethi1:
            dval(rv) += sulp(rv,bc);
            }
        }
    else {
        /* Exact half-way case:  apply round-even rule. */
        if ((j = ((word0(rv) & Exp_mask) >> Exp_shift) - bc->scale) <= 0) {
            i = 1 - j;
            if (i <= 31) {
                if (word1(rv) & (0x1 << i))
                    goto odd;
                }
            else if (word0(rv) & (0x1 << (i-32)))
                goto odd;
            }
        else if (word1(rv) & 1) {
 odd:
            if (dsign)
                goto rethi1;
            goto retlow1;
            }
        }

#ifdef Honor_FLT_ROUNDS
 ret1:
#endif
    return;
    }
#endif /* NO_STRTOD_BIGCOMP */

f64 Nst_strtod(const i8 *s00, i8 **se)
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
#ifdef USE_BF96
    ULLong bhi, blo, brv, t00, t01, t02, t10, t11, terv, tg, tlo, yz;
    const BF96 *p10;
    int bexact, erv;
#endif
#ifdef Avoid_Underflow
    ULong Lsb, Lsb1;
#endif
#ifdef SET_INEXACT
    int oldinexact;
#endif
#ifndef NO_STRTOD_BIGCOMP
    int req_bigcomp = 0;
#endif
#ifdef MULTIPLE_THREADS
    ThInfo *TI = 0;
#endif
#ifdef Honor_FLT_ROUNDS /*{*/
#ifdef Trust_FLT_ROUNDS /*{{ only define this if FLT_ROUNDS really works! */
    bc.rounding = Flt_Rounds;
#else /*}{*/
    bc.rounding = 1;
    switch(fegetround()) {
     case FE_TOWARDZERO:    bc.rounding = 0; break;
     case FE_UPWARD:    bc.rounding = 2; break;
     case FE_DOWNWARD:  bc.rounding = 3;
     }
#endif /*}}*/
#endif /*}*/
#ifdef USE_LOCALE
    const char *s2;
#endif

    sign = nz0 = nz1 = nz = bc.dplen = bc.uflchk = 0;
    dval(&rv) = 0.;
    for(s = s00;;s++) switch(*s) {
        case '-':
            sign = 1;
            /* no break */
        case '+':
            if (*++s)
                goto break2;
            /* no break */
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
#ifndef NO_HEX_FP /*{*/
        switch(s[1]) {
        case 'x':
        case 'X':
#ifdef Honor_FLT_ROUNDS
            gethex(&s, &rv, bc.rounding, sign);
#else
            gethex(&s, &rv, 1, sign);
#endif
            goto ret;
        }
#endif /*}*/
        nz0 = 1;
        while(*++s == '0') ;
        if (!*s)
            goto ret;
        }
    s0 = s;
    nd = nf = 0;
#ifdef USE_BF96
    yz = 0;
    for(; (c = *s) >= '0' && c <= '9'; nd++, s++)
        if (nd < 19)
            yz = 10*yz + c - '0';
#else
    y = z = 0;
    for(; (c = *s) >= '0' && c <= '9'; nd++, s++)
        if (nd < 9)
            y = 10*y + c - '0';
        else if (nd < DBL_DIG + 2)
            z = 10*z + c - '0';
#endif
    nd0 = nd;
    bc.dp0 = bc.dp1 = s - s0;
    for(s1 = s; s1 > s0 && *--s1 == '0'; )
        ++nz1;
#ifdef USE_LOCALE
    s1 = localeconv()->decimal_point;
    if (c == *s1) {
        c = '.';
        if (*++s1) {
            s2 = s;
            for(;;) {
                if (*++s2 != *s1) {
                    c = 0;
                    break;
                    }
                if (!*++s1) {
                    s = s2;
                    break;
                    }
                }
            }
        }
#endif
    if (c == '.') {
        c = *++s;
        bc.dp1 = s - s0;
        bc.dplen = bc.dp1 - bc.dp0;
        if (!nd) {
            for(; c == '0'; c = *++s)
                nz++;
            if (c > '0' && c <= '9') {
                bc.dp0 = s0 - s;
                bc.dp1 = bc.dp0 + bc.dplen;
                s0 = s;
                nf += nz;
                nz = 0;
                goto have_dig;
                }
            goto dig_done;
            }
        for(; c >= '0' && c <= '9'; c = *++s) {
 have_dig:
            nz++;
            if (c -= '0') {
                nf += nz;
                i = 1;
#ifdef USE_BF96
                for(; i < nz; ++i) {
                    if (++nd <= 19)
                        yz *= 10;
                    }
                if (++nd <= 19)
                    yz = 10*yz + c;
#else
                for(; i < nz; ++i) {
                    if (nd++ < 9)
                        y *= 10;
                    else if (nd <= DBL_DIG + 2)
                        z *= 10;
                    }
                if (nd++ < 9)
                    y = 10*y + c;
                else if (nd <= DBL_DIG + 2)
                    z = 10*z + c;
#endif
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
        switch(c = *++s) {
            case '-':
                esign = 1;
            case '+':
                c = *++s;
            }
        if (c >= '0' && c <= '9') {
            while(c == '0')
                c = *++s;
            if (c > '0' && c <= '9') {
                L = c - '0';
                s1 = s;
                while((c = *++s) >= '0' && c <= '9')
                    L = 10*L + c - '0';
                if (s - s1 > 8 || L > 19999)
                    /* Avoid confusion from exponents
                * so large that e might overflow.
                */
                    e = 19999; /* safe for 16 bit ints */
                else
                    e = (int)L;
                if (esign)
                    e = -e;
                }
            else
                e = 0;
            }
        else
            s = s00;
        }
    if (!nd) {
        if (!nz && !nz0) {
#ifdef INFNAN_CHECK /*{*/
            /* Check for Nan and Infinity */
            if (!bc.dplen)
          switch(c) {
           case 'i':
           case 'I':
                if (match(&s,"nf")) {
                    --s;
                    if (!match(&s,"inity"))
                        ++s;
                    word0(&rv) = 0x7ff00000;
                    word1(&rv) = 0;
                    goto ret;
                    }
                break;
           case 'n':
           case 'N':
                if (match(&s, "an")) {
                    word0(&rv) = NAN_WORD0;
                    word1(&rv) = NAN_WORD1;
#ifndef No_Hex_NaN
                    if (*s == '(') /*)*/
                        hexnan(&rv, &s);
#endif
                    goto ret;
                    }
           }
#endif /*} INFNAN_CHECK */
 ret0:
            s = s00;
            sign = 0;
            }
        goto ret;
        }
    bc.e0 = e1 = e -= nf;

    /* Now we have nd0 digits, starting at s0, followed by a
    * decimal point, followed by nd-nd0 digits.  The number we're
    * after is the integer represented by those digits times
    * 10**e */

    if (!nd0)
        nd0 = nd;
#ifndef USE_BF96
    k = nd < DBL_DIG + 2 ? nd : DBL_DIG + 2;
    dval(&rv) = y;
    if (k > 9) {
#ifdef SET_INEXACT
        if (k > DBL_DIG)
            oldinexact = get_inexact();
#endif
        dval(&rv) = tens[k - 9] * dval(&rv) + z;
        }
#endif
    bd0 = 0;
    if (nd <= DBL_DIG
#ifndef RND_PRODQUOT
#ifndef Honor_FLT_ROUNDS
        && Flt_Rounds == 1
#endif
#endif
            ) {
#ifdef USE_BF96
        dval(&rv) = yz;
#endif
        if (!e)
            goto ret;
#ifndef ROUND_BIASED_without_Round_Up
        if (e > 0) {
            if (e <= Ten_pmax) {
#ifdef SET_INEXACT
                bc.inexact = 0;
                oldinexact = 1;
#endif
#ifdef VAX
                goto vax_ovfl_check;
#else
#ifdef Honor_FLT_ROUNDS
                /* round correctly FLT_ROUNDS = 2 or 3 */
                if (sign) {
                    rv.d = -rv.d;
                    sign = 0;
                    }
#endif
                /* rv = */ rounded_product(dval(&rv), tens[e]);
                goto ret;
#endif
                }
            i = DBL_DIG - nd;
            if (e <= Ten_pmax + i) {
                /* A fancier test would sometimes let us do
             * this for larger i values.
             */
#ifdef SET_INEXACT
                bc.inexact = 0;
                oldinexact = 1;
#endif
#ifdef Honor_FLT_ROUNDS
                /* round correctly FLT_ROUNDS = 2 or 3 */
                if (sign) {
                    rv.d = -rv.d;
                    sign = 0;
                    }
#endif
                e -= i;
                dval(&rv) *= tens[i];
#ifdef VAX
                /* VAX exponent range is so narrow we must
             * worry about overflow here...
             */
 vax_ovfl_check:
                word0(&rv) -= P*Exp_msk1;
                /* rv = */ rounded_product(dval(&rv), tens[e]);
                if ((word0(&rv) & Exp_mask)
             > Exp_msk1*(DBL_MAX_EXP+Bias-1-P))
                    goto ovfl;
                word0(&rv) += P*Exp_msk1;
#else
                /* rv = */ rounded_product(dval(&rv), tens[e]);
#endif
                goto ret;
                }
            }
#ifndef Inaccurate_Divide
        else if (e >= -Ten_pmax) {
#ifdef SET_INEXACT
                bc.inexact = 0;
                oldinexact = 1;
#endif
#ifdef Honor_FLT_ROUNDS
            /* round correctly FLT_ROUNDS = 2 or 3 */
            if (sign) {
                rv.d = -rv.d;
                sign = 0;
                }
#endif
            /* rv = */ rounded_quotient(dval(&rv), tens[-e]);
            goto ret;
            }
#endif
#endif /* ROUND_BIASED_without_Round_Up */
        }
#ifdef USE_BF96
    k = nd < 19 ? nd : 19;
#endif
    e1 += nd - k;   /* scale factor = 10^e1 */

#ifdef IEEE_Arith
#ifdef SET_INEXACT
    bc.inexact = 1;
#ifndef USE_BF96
    if (k <= DBL_DIG)
#endif
        oldinexact = get_inexact();
#endif
#ifdef Honor_FLT_ROUNDS
    if (bc.rounding >= 2) {
        if (sign)
            bc.rounding = bc.rounding == 2 ? 0 : 2;
        else
            if (bc.rounding != 2)
                bc.rounding = 0;
        }
#endif
#endif /*IEEE_Arith*/

#ifdef USE_BF96 /*{*/
    Debug(++dtoa_stats[0]);
    i = e1 + 342;
    if (i < 0)
        goto undfl;
    if (i > 650)
        goto ovfl;
    p10 = &pten[i];
    brv = yz;
    /* shift brv left, with i =  number of bits shifted */
    i = 0;
    if (!(brv & 0xffffffff00000000ull)) {
        i = 32;
        brv <<= 32;
        }
    if (!(brv & 0xffff000000000000ull)) {
        i += 16;
        brv <<= 16;
        }
    if (!(brv & 0xff00000000000000ull)) {
        i += 8;
        brv <<= 8;
        }
    if (!(brv & 0xf000000000000000ull)) {
        i += 4;
        brv <<= 4;
        }
    if (!(brv & 0xc000000000000000ull)) {
        i += 2;
        brv <<= 2;
        }
    if (!(brv & 0x8000000000000000ull)) {
        i += 1;
        brv <<= 1;
        }
    erv = (64 + 0x3fe) + p10->e - i;
    if (erv <= 0 && nd > 19)
        goto many_digits; /* denormal: may need to look at all digits */
    bhi = brv >> 32;
    blo = brv & 0xffffffffull;
    /* Unsigned 32-bit ints lie in [0,2^32-1] and */
    /* unsigned 64-bit ints lie in [0, 2^64-1].  The product of two unsigned */
    /* 32-bit ints is <= 2^64 - 2*2^32-1 + 1 = 2^64 - 1 - 2*(2^32 - 1), so */
    /* we can add two unsigned 32-bit ints to the product of two such ints, */
    /* and 64 bits suffice to contain the result. */
    t01 = bhi * p10->b1;
    t10 = blo * p10->b0 + (t01 & 0xffffffffull);
    t00 = bhi * p10->b0 + (t01 >> 32) + (t10 >> 32);
    if (t00 & 0x8000000000000000ull) {
        if ((t00 & 0x3ff) && (~t00 & 0x3fe)) { /* unambiguous result? */
            if (nd > 19 && ((t00 + (1<<i) + 2) & 0x400) ^ (t00 & 0x400))
                goto many_digits;
            if (erv <= 0)
                goto denormal;
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 0: goto noround;
           case 2: goto roundup;
           }
#endif
            if (t00 & 0x400 && t00 & 0xbff)
                goto roundup;
            goto noround;
            }
        }
    else {
        if ((t00 & 0x1ff) && (~t00 & 0x1fe)) { /* unambiguous result? */
            if (nd > 19 && ((t00 + (1<<i) + 2) & 0x200) ^ (t00 & 0x200))
                goto many_digits;
            if (erv <= 1)
                goto denormal1;
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 0: goto noround1;
           case 2: goto roundup1;
           }
#endif
            if (t00 & 0x200)
                goto roundup1;
            goto noround1;
            }
        }
    /* 3 multiplies did not suffice; try a 96-bit approximation */
    Debug(++dtoa_stats[1]);
    t02 = bhi * p10->b2;
    t11 = blo * p10->b1 + (t02 & 0xffffffffull);
    bexact = 1;
    if (e1 < 0 || e1 > 41 || (t10 | t11) & 0xffffffffull || nd > 19)
        bexact = 0;
    tlo = (t10 & 0xffffffffull) + (t02 >> 32) + (t11 >> 32);
    if (!bexact && (tlo + 0x10) >> 32 > tlo >> 32)
        goto many_digits;
    t00 += tlo >> 32;
    if (t00 & 0x8000000000000000ull) {
        if (erv <= 0) { /* denormal result */
            if (nd >= 20 || !((tlo & 0xfffffff0) | (t00 & 0x3ff)))
                goto many_digits;
 denormal:
            if (erv <= -52) {
#ifdef Honor_FLT_ROUNDS
                switch(bc.rounding) {
              case 0: goto undfl;
              case 2: goto tiniest;
              }
#endif
                if (erv < -52 || !(t00 & 0x7fffffffffffffffull))
                    goto undfl;
                goto tiniest;
                }
            tg = 1ull << (11 - erv);
            t00 &= ~(tg - 1); /* clear low bits */
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 0: goto noround_den;
           case 2: goto roundup_den;
           }
#endif
            if (t00 & tg) {
#ifdef Honor_FLT_ROUNDS
 roundup_den:
#endif
                t00 += tg << 1;
                if (!(t00 & 0x8000000000000000ull)) {
                    if (++erv > 0)
                        goto smallest_normal;
                    t00 = 0x8000000000000000ull;
                    }
                }
#ifdef Honor_FLT_ROUNDS
 noround_den:
#endif
            LLval(&rv) = t00 >> (12 - erv);
            Set_errno(ERANGE);
            goto ret;
            }
        if (bexact) {
#ifdef SET_INEXACT
            if (!(t00 & 0x7ff) && !(tlo & 0xffffffffull)) {
                bc.inexact = 0;
                goto noround;
                }
#endif
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 2:
                if (t00 & 0x7ff)
                    goto roundup;
           case 0: goto noround;
           }
#endif
            if (t00 & 0x400 && (tlo & 0xffffffff) | (t00 & 0xbff))
                goto roundup;
            goto noround;
            }
        if ((tlo & 0xfffffff0) | (t00 & 0x3ff)
       && (nd <= 19 ||  ((t00 + (1ull << i)) & 0xfffffffffffffc00ull)
                == (t00 & 0xfffffffffffffc00ull))) {
            /* Unambiguous result. */
            /* If nd > 19, then incrementing the 19th digit */
            /* does not affect rv. */
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 0: goto noround;
           case 2: goto roundup;
           }
#endif
            if (t00 & 0x400) { /* round up */
 roundup:
                t00 += 0x800;
                if (!(t00 & 0x8000000000000000ull)) {
                    /* rounded up to a power of 2 */
                    if (erv >= 0x7fe)
                        goto ovfl;
                    terv = erv + 1;
                    LLval(&rv) = terv << 52;
                    goto ret;
                    }
                }
 noround:
            if (erv >= 0x7ff)
                goto ovfl;
            terv = erv;
            LLval(&rv) = (terv << 52) | ((t00 & 0x7ffffffffffff800ull) >> 11);
            goto ret;
            }
        }
    else {
        if (erv <= 1) { /* denormal result */
            if (nd >= 20 || !((tlo & 0xfffffff0) | (t00 & 0x1ff)))
                goto many_digits;
 denormal1:
            if (erv <= -51) {
#ifdef Honor_FLT_ROUNDS
                switch(bc.rounding) {
              case 0: goto undfl;
              case 2: goto tiniest;
              }
#endif
                if (erv < -51 || !(t00 & 0x3fffffffffffffffull))
                    goto undfl;
 tiniest:
                LLval(&rv) = 1;
                Set_errno(ERANGE);
                goto ret;
                }
            tg = 1ull << (11 - erv);
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 0: goto noround1_den;
           case 2: goto roundup1_den;
           }
#endif
            if (t00 & tg) {
#ifdef Honor_FLT_ROUNDS
 roundup1_den:
#endif
                if (0x8000000000000000ull & (t00 += (tg<<1)) && erv == 1) {

 smallest_normal:
                    LLval(&rv) = 0x0010000000000000ull;
                    goto ret;
                    }
                }
#ifdef Honor_FLT_ROUNDS
 noround1_den:
#endif
            if (erv <= -52)
                goto undfl;
            LLval(&rv) = t00 >> (12 - erv);
            Set_errno(ERANGE);
            goto ret;
            }
        if (bexact) {
#ifdef SET_INEXACT
            if (!(t00 & 0x3ff) && !(tlo & 0xffffffffull)) {
                bc.inexact = 0;
                goto noround1;
                }
#endif
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 2:
                if (t00 & 0x3ff)
                    goto roundup1;
           case 0: goto noround1;
           }
#endif
            if (t00 & 0x200 && (t00 & 0x5ff || tlo))
                goto roundup1;
            goto noround1;
            }
        if ((tlo & 0xfffffff0) | (t00 & 0x1ff)
       && (nd <= 19 ||  ((t00 + (1ull << i)) & 0x7ffffffffffffe00ull)
                == (t00 & 0x7ffffffffffffe00ull))) {
            /* Unambiguous result. */
#ifdef Honor_FLT_ROUNDS
            switch(bc.rounding) {
           case 0: goto noround1;
           case 2: goto roundup1;
           }
#endif
            if (t00 & 0x200) { /* round up */
 roundup1:
                t00 += 0x400;
                if (!(t00 & 0x4000000000000000ull)) {
                    /* rounded up to a power of 2 */
                    if (erv >= 0x7ff)
                        goto ovfl;
                    terv = erv;
                    LLval(&rv) = terv << 52;
                    goto ret;
                    }
                }
 noround1:
            if (erv >= 0x800)
                goto ovfl;
            terv = erv - 1;
            LLval(&rv) = (terv << 52) | ((t00 & 0x3ffffffffffffc00ull) >> 10);
            goto ret;
            }
        }
 many_digits:
    Debug(++dtoa_stats[2]);
    if (nd > 17) {
        if (nd > 18) {
            yz /= 100;
            e1 += 2;
            }
        else {
            yz /= 10;
            e1 += 1;
            }
        y = yz / 100000000;
        }
    else if (nd > 9) {
        i = nd - 9;
        y = (yz >> i) / pfive[i-1];
        }
    else
        y = yz;
    dval(&rv) = yz;
#endif /*}*/

#ifdef IEEE_Arith
#ifdef Avoid_Underflow
    bc.scale = 0;
#endif
#endif /*IEEE_Arith*/

    /* Get starting approximation = rv * 10**e1 */

    if (e1 > 0) {
        if ((i = e1 & 15))
            dval(&rv) *= tens[i];
        if (e1 &= ~15) {
            if (e1 > DBL_MAX_10_EXP) {
 ovfl:
                /* Can't trust HUGE_VAL */
#ifdef IEEE_Arith
#ifdef Honor_FLT_ROUNDS
                switch(bc.rounding) {
              case 0: /* toward 0 */
              case 3: /* toward -infinity */
                    word0(&rv) = Big0;
                    word1(&rv) = Big1;
                    break;
              default:
                    word0(&rv) = Exp_mask;
                    word1(&rv) = 0;
              }
#else /*Honor_FLT_ROUNDS*/
                word0(&rv) = Exp_mask;
                word1(&rv) = 0;
#endif /*Honor_FLT_ROUNDS*/
#ifdef SET_INEXACT
                /* set overflow bit */
                dval(&rv0) = 1e300;
                dval(&rv0) *= dval(&rv0);
#endif
#else /*IEEE_Arith*/
                word0(&rv) = Big0;
                word1(&rv) = Big1;
#endif /*IEEE_Arith*/
 range_err:
                if (bd0) {
                    Bfree(bb);
                    Bfree(bd);
                    Bfree(bs);
                    Bfree(bd0);
                    Bfree(delta);
                    }
                Set_errno(ERANGE);
                goto ret;
                }
            e1 >>= 4;
            for(j = 0; e1 > 1; j++, e1 >>= 1)
                if (e1 & 1)
                    dval(&rv) *= bigtens[j];
        /* The last multiplication could overflow. */
            word0(&rv) -= P*Exp_msk1;
            dval(&rv) *= bigtens[j];
            if ((z = word0(&rv) & Exp_mask)
          > Exp_msk1*(DBL_MAX_EXP+Bias-P))
                goto ovfl;
            if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
                /* set to largest number */
                /* (Can't trust DBL_MAX) */
                word0(&rv) = Big0;
                word1(&rv) = Big1;
                }
            else
                word0(&rv) += P*Exp_msk1;
            }
        }
    else if (e1 < 0) {
        e1 = -e1;
        if ((i = e1 & 15))
            dval(&rv) /= tens[i];
        if (e1 >>= 4) {
            if (e1 >= 1 << n_bigtens)
                goto undfl;
#ifdef Avoid_Underflow
            if (e1 & Scale_Bit)
                bc.scale = 2*P;
            for(j = 0; e1 > 0; j++, e1 >>= 1)
                if (e1 & 1)
                    dval(&rv) *= tinytens[j];
            if (bc.scale && (j = 2*P + 1 - ((word0(&rv) & Exp_mask)
                        >> Exp_shift)) > 0) {
                /* scaled rv is denormal; clear j low bits */
                if (j >= 32) {
                    if (j > 54)
                        goto undfl;
                    word1(&rv) = 0;
                    if (j >= 53)
                word0(&rv) = (P+2)*Exp_msk1;
                    else
                word0(&rv) &= 0xffffffff << (j-32);
                    }
                else
                    word1(&rv) &= 0xffffffff << j;
                }
#else
            for(j = 0; e1 > 1; j++, e1 >>= 1)
                if (e1 & 1)
                    dval(&rv) *= tinytens[j];
            /* The last multiplication could underflow. */
            dval(&rv0) = dval(&rv);
            dval(&rv) *= tinytens[j];
            if (!dval(&rv)) {
                dval(&rv) = 2.*dval(&rv0);
                dval(&rv) *= tinytens[j];
#endif
                if (!dval(&rv)) {
 undfl:
                    dval(&rv) = 0.;
#ifdef Honor_FLT_ROUNDS
                    if (bc.rounding == 2)
                        word1(&rv) = 1;
#endif
                    goto range_err;
                    }
#ifndef Avoid_Underflow
                word0(&rv) = Tiny0;
                word1(&rv) = Tiny1;
                /* The refinement below will clean
             * this approximation up.
             */
                }
#endif
            }
        }

    /* Now the hard part -- adjusting rv to the correct value.*/

    /* Put digits into bd: true value = bd * 10^e */

    bc.nd = nd - nz1;
#ifndef NO_STRTOD_BIGCOMP
    bc.nd0 = nd0;   /* Only needed if nd > strtod_diglim, but done here */
            /* to silence an erroneous warning about bc.nd0 */
            /* possibly not being initialized. */
    if (nd > strtod_diglim) {
        /* ASSERT(strtod_diglim >= 18); 18 == one more than the */
        /* minimum number of decimal digits to distinguish double values */
        /* in IEEE arithmetic. */
        i = j = 18;
        if (i > nd0)
            j += bc.dplen;
        for(;;) {
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
        if (nd < 9) { /* must recompute y */
            y = 0;
            for(i = 0; i < nd0; ++i)
                y = 10*y + s0[i] - '0';
            for(j = bc.dp1; i < nd; ++i)
                y = 10*y + s0[j++] - '0';
            }
        }
#endif
    bd0 = s2b(s0, nd0, nd, y, bc.dplen);

    for(;;) {
        bd = Balloc(bd0->k);
        Bcopy(bd, bd0);
        bb = d2b(&rv, &bbe, &bbbits);   /* rv = bb * 2^bbe */
        bs = i2b(1);

        if (e >= 0) {
            bb2 = bb5 = 0;
            bd2 = bd5 = e;
            }
        else {
            bb2 = bb5 = -e;
            bd2 = bd5 = 0;
            }
        if (bbe >= 0)
            bb2 += bbe;
        else
            bd2 -= bbe;
        bs2 = bb2;
#ifdef Honor_FLT_ROUNDS
        if (bc.rounding != 1)
            bs2++;
#endif
#ifdef Avoid_Underflow
        Lsb = LSB;
        Lsb1 = 0;
        j = bbe - bc.scale;
        i = j + bbbits - 1; /* logb(rv) */
        j = P + 1 - bbbits;
        if (i < Emin) { /* denormal */
            i = Emin - i;
            j -= i;
            if (i < 32)
                Lsb <<= i;
            else if (i < 52)
                Lsb1 = Lsb << (i-32);
            else
                Lsb1 = Exp_mask;
            }
#else /*Avoid_Underflow*/
#ifdef Sudden_Underflow
#ifdef IBM
        j = 1 + 4*P - 3 - bbbits + ((bbe + bbbits - 1) & 3);
#else
        j = P + 1 - bbbits;
#endif
#else /*Sudden_Underflow*/
        j = bbe;
        i = j + bbbits - 1; /* logb(rv) */
        if (i < Emin)   /* denormal */
            j += P - Emin;
        else
            j = P + 1 - bbbits;
#endif /*Sudden_Underflow*/
#endif /*Avoid_Underflow*/
        bb2 += j;
        bd2 += j;
#ifdef Avoid_Underflow
        bd2 += bc.scale;
#endif
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
#ifndef NO_STRTOD_BIGCOMP /*{*/
        if (bc.nd > nd && i <= 0) {
            if (bc.dsign) {
                /* Must use bigcomp(). */
                req_bigcomp = 1;
                break;
                }
#ifdef Honor_FLT_ROUNDS
            if (bc.rounding != 1) {
                if (i < 0) {
                    req_bigcomp = 1;
                    break;
                    }
                }
            else
#endif
                i = -1; /* Discarded digits make delta smaller. */
            }
#endif /*}*/
#ifdef Honor_FLT_ROUNDS /*{*/
        if (bc.rounding != 1) {
            if (i < 0) {
                /* Error is less than an ulp */
                if (!delta->x[0] && delta->wds <= 1) {
                    /* exact */
#ifdef SET_INEXACT
                    bc.inexact = 0;
#endif
                    break;
                    }
                if (bc.rounding) {
                    if (bc.dsign) {
                        adj.d = 1.;
                        goto apply_adj;
                        }
                    }
                else if (!bc.dsign) {
                    adj.d = -1.;
                    if (!word1(&rv)
                && !(word0(&rv) & Frac_mask)) {
                        y = word0(&rv) & Exp_mask;
#ifdef Avoid_Underflow
                        if (!bc.scale || y > 2*P*Exp_msk1)
#else
                        if (y)
#endif
                    {
                    delta = lshift(delta,Log2P);
                    if (cmp(delta, bs) <= 0)
                            adj.d = -0.5;
                    }
                        }
 apply_adj:
#ifdef Avoid_Underflow /*{*/
                    if (bc.scale && (y = word0(&rv) & Exp_mask)
                        <= 2*P*Exp_msk1)
                 word0(&adj) += (2*P+1)*Exp_msk1 - y;
#else
#ifdef Sudden_Underflow
                    if ((word0(&rv) & Exp_mask) <=
                            P*Exp_msk1) {
                        word0(&rv) += P*Exp_msk1;
                        dval(&rv) += adj.d*ulp(dval(&rv));
                        word0(&rv) -= P*Exp_msk1;
                        }
                    else
#endif /*Sudden_Underflow*/
#endif /*Avoid_Underflow}*/
                    dval(&rv) += adj.d*ulp(&rv);
                    }
                break;
                }
            adj.d = ratio(delta, bs);
            if (adj.d < 1.)
                adj.d = 1.;
            if (adj.d <= 0x7ffffffe) {
                /* adj = rounding ? ceil(adj) : floor(adj); */
                y = adj.d;
                if (y != adj.d) {
                    if (!((bc.rounding>>1) ^ bc.dsign))
                        y++;
                    adj.d = y;
                    }
                }
#ifdef Avoid_Underflow /*{*/
            if (bc.scale && (y = word0(&rv) & Exp_mask) <= 2*P*Exp_msk1)
                word0(&adj) += (2*P+1)*Exp_msk1 - y;
#else
#ifdef Sudden_Underflow
            if ((word0(&rv) & Exp_mask) <= P*Exp_msk1) {
                word0(&rv) += P*Exp_msk1;
                adj.d *= ulp(dval(&rv));
                if (bc.dsign)
                    dval(&rv) += adj.d;
                else
                    dval(&rv) -= adj.d;
                word0(&rv) -= P*Exp_msk1;
                goto cont;
                }
#endif /*Sudden_Underflow*/
#endif /*Avoid_Underflow}*/
            adj.d *= ulp(&rv);
            if (bc.dsign) {
                if (word0(&rv) == Big0 && word1(&rv) == Big1)
                    goto ovfl;
                dval(&rv) += adj.d;
                }
            else
                dval(&rv) -= adj.d;
            goto cont;
            }
#endif /*}Honor_FLT_ROUNDS*/

        if (i < 0) {
            /* Error is less than half an ulp -- check for
          * special case of mantissa a power of two.
          */
            if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask
#ifdef IEEE_Arith /*{*/
#ifdef Avoid_Underflow
          || (word0(&rv) & Exp_mask) <= (2*P+1)*Exp_msk1
#else
          || (word0(&rv) & Exp_mask) <= Exp_msk1
#endif
#endif /*}*/
                ) {
#ifdef SET_INEXACT
                if (!delta->x[0] && delta->wds <= 1)
                    bc.inexact = 0;
#endif
                break;
                }
            if (!delta->x[0] && delta->wds <= 1) {
                /* exact result */
#ifdef SET_INEXACT
                bc.inexact = 0;
#endif
                break;
                }
            delta = lshift(delta,Log2P);
            if (cmp(delta, bs) > 0)
                goto drop_down;
            break;
            }
        if (i == 0) {
            /* exactly half-way between */
            if (bc.dsign) {
                if ((word0(&rv) & Bndry_mask1) == Bndry_mask1
             &&  word1(&rv) == (
#ifdef Avoid_Underflow
            (bc.scale && (y = word0(&rv) & Exp_mask) <= 2*P*Exp_msk1)
        ? (0xffffffff & (0xffffffff << (2*P+1-(y>>Exp_shift)))) :
#endif
                     0xffffffff)) {
                    /*boundary case -- increment exponent*/
                    if (word0(&rv) == Big0 && word1(&rv) == Big1)
                        goto ovfl;
                    word0(&rv) = (word0(&rv) & Exp_mask)
                        + Exp_msk1
#ifdef IBM
                        | Exp_msk1 >> 4
#endif
                        ;
                    word1(&rv) = 0;
#ifdef Avoid_Underflow
                    bc.dsign = 0;
#endif
                    break;
                    }
                }
            else if (!(word0(&rv) & Bndry_mask) && !word1(&rv)) {
 drop_down:
                /* boundary case -- decrement exponent */
#ifdef Sudden_Underflow /*{{*/
                L = word0(&rv) & Exp_mask;
#ifdef IBM
                if (L <  Exp_msk1)
#else
#ifdef Avoid_Underflow
                if (L <= (bc.scale ? (2*P+1)*Exp_msk1 : Exp_msk1))
#else
                if (L <= Exp_msk1)
#endif /*Avoid_Underflow*/
#endif /*IBM*/
                    {
                    if (bc.nd >nd) {
                        bc.uflchk = 1;
                        break;
                        }
                    goto undfl;
                    }
                L -= Exp_msk1;
#else /*Sudden_Underflow}{*/
#ifdef Avoid_Underflow
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
#endif /*Avoid_Underflow*/
                L = (word0(&rv) & Exp_mask) - Exp_msk1;
#endif /*Sudden_Underflow}}*/
                word0(&rv) = L | Bndry_mask1;
                word1(&rv) = 0xffffffff;
#ifdef IBM
                goto cont;
#else
#ifndef NO_STRTOD_BIGCOMP
                if (bc.nd > nd)
                    goto cont;
#endif
                break;
#endif
                }
#ifndef ROUND_BIASED
#ifdef Avoid_Underflow
            if (Lsb1) {
                if (!(word0(&rv) & Lsb1))
                    break;
                }
            else if (!(word1(&rv) & Lsb))
                break;
#else
            if (!(word1(&rv) & LSB))
                break;
#endif
#endif
            if (bc.dsign)
#ifdef Avoid_Underflow
                dval(&rv) += sulp(&rv, &bc);
#else
                dval(&rv) += ulp(&rv);
#endif
#ifndef ROUND_BIASED
            else {
#ifdef Avoid_Underflow
                dval(&rv) -= sulp(&rv, &bc);
#else
                dval(&rv) -= ulp(&rv);
#endif
#ifndef Sudden_Underflow
                if (!dval(&rv)) {
                    if (bc.nd >nd) {
                        bc.uflchk = 1;
                        break;
                        }
                    goto undfl;
                    }
#endif
                }
#ifdef Avoid_Underflow
            bc.dsign = 1 - bc.dsign;
#endif
#endif
            break;
            }
        if ((aadj = ratio(delta, bs)) <= 2.) {
            if (bc.dsign)
                aadj = aadj1 = 1.;
            else if (word1(&rv) || word0(&rv) & Bndry_mask) {
#ifndef Sudden_Underflow
                if (word1(&rv) == Tiny1 && !word0(&rv)) {
                    if (bc.nd >nd) {
                        bc.uflchk = 1;
                        break;
                        }
                    goto undfl;
                    }
#endif
                aadj = 1.;
                aadj1 = -1.;
                }
            else {
                /* special case -- power of FLT_RADIX to be */
                /* rounded down... */

                if (aadj < 2./FLT_RADIX)
                    aadj = 1./FLT_RADIX;
                else
                    aadj *= 0.5;
                aadj1 = -aadj;
                }
            }
        else {
            aadj *= 0.5;
            aadj1 = bc.dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
            switch(bc.rounding) {
                case 2: /* towards +infinity */
                    aadj1 -= 0.5;
                    break;
                case 0: /* towards 0 */
                case 3: /* towards -infinity */
                    aadj1 += 0.5;
                }
#else
            if (Flt_Rounds == 0)
                aadj1 += 0.5;
#endif /*Check_FLT_ROUNDS*/
            }
        y = word0(&rv) & Exp_mask;

        /* Check for overflow */

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
                }
            else
                word0(&rv) += P*Exp_msk1;
            }
        else {
#ifdef Avoid_Underflow
            if (bc.scale && y <= 2*P*Exp_msk1) {
                if (aadj <= 0x7fffffff) {
                    if ((z = aadj) <= 0)
                        z = 1;
                    aadj = z;
                    aadj1 = bc.dsign ? aadj : -aadj;
                    }
                dval(&aadj2) = aadj1;
                word0(&aadj2) += (2*P+1)*Exp_msk1 - y;
                aadj1 = dval(&aadj2);
                adj.d = aadj1 * ulp(&rv);
                dval(&rv) += adj.d;
                if (rv.d == 0.)
#ifdef NO_STRTOD_BIGCOMP
                    goto undfl;
#else
                    {
                    req_bigcomp = 1;
                    break;
                    }
#endif
                }
            else {
                adj.d = aadj1 * ulp(&rv);
                dval(&rv) += adj.d;
                }
#else
#ifdef Sudden_Underflow
            if ((word0(&rv) & Exp_mask) <= P*Exp_msk1) {
                dval(&rv0) = dval(&rv);
                word0(&rv) += P*Exp_msk1;
                adj.d = aadj1 * ulp(&rv);
                dval(&rv) += adj.d;
#ifdef IBM
                if ((word0(&rv) & Exp_mask) <  P*Exp_msk1)
#else
                if ((word0(&rv) & Exp_mask) <= P*Exp_msk1)
#endif
                    {
                    if (word0(&rv0) == Tiny0
                && word1(&rv0) == Tiny1) {
                        if (bc.nd >nd) {
                            bc.uflchk = 1;
                            break;
                            }
                        goto undfl;
                        }
                    word0(&rv) = Tiny0;
                    word1(&rv) = Tiny1;
                    goto cont;
                    }
                else
                    word0(&rv) -= P*Exp_msk1;
                }
            else {
                adj.d = aadj1 * ulp(&rv);
                dval(&rv) += adj.d;
                }
#else /*Sudden_Underflow*/
            /* Compute adj so that the IEEE rounding rules will
          * correctly round rv + adj in some half-way cases.
          * If rv * ulp(rv) is denormalized (i.e.,
          * y <= (P-1)*Exp_msk1), we must adjust aadj to avoid
          * trouble from bits lost to denormalization;
          * example: 1.2e-307 .
          */
            if (y <= (P-1)*Exp_msk1 && aadj > 1.) {
                aadj1 = (double)(int)(aadj + 0.5);
                if (!bc.dsign)
                    aadj1 = -aadj1;
                }
            adj.d = aadj1 * ulp(&rv);
            dval(&rv) += adj.d;
#endif /*Sudden_Underflow*/
#endif /*Avoid_Underflow*/
            }
        z = word0(&rv) & Exp_mask;
#ifndef SET_INEXACT
        if (bc.nd == nd) {
#ifdef Avoid_Underflow
        if (!bc.scale)
#endif
        if (y == z) {
            /* Can we stop now? */
            L = (Long)aadj;
            aadj -= L;
            /* The tolerances below are conservative. */
            if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask) {
                if (aadj < .4999999 || aadj > .5000001)
                    break;
                }
            else if (aadj < .4999999/FLT_RADIX)
                break;
            }
        }
#endif
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
#ifndef NO_STRTOD_BIGCOMP
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
#endif
#ifdef Avoid_Underflow
    if (bc.scale) {
        word0(&rv0) = Exp_1 - 2*P*Exp_msk1;
        word1(&rv0) = 0;
        dval(&rv) *= dval(&rv0);
        }
#endif /* Avoid_Underflow */
 ret:
#ifdef SET_INEXACT
    if (bc.inexact) {
        if (!(word0(&rv) & Exp_mask)) {
            /* set underflow and inexact bits */
            dval(&rv0) = 1e-300;
            dval(&rv0) *= dval(&rv0);
            }
        else if (!oldinexact) {
            word0(&rv0) = Exp_1 + (70 << Exp_shift);
            word1(&rv0) = 0;
            dval(&rv0) += 1.;
            }
        }
    else if (!oldinexact)
        clear_inexact();
#endif
    if (se)
        *se = (char *)s;
    return sign ? -dval(&rv) : dval(&rv);
    }

 static char *
rv_alloc(int i)
{
    int j, k, *r;

    j = sizeof(ULong);
    for(k = 0;
        (int)(sizeof(Bigint) - sizeof(ULong) - sizeof(int)) + j <= i;
        j <<= 1)
            k++;
    r = (int*)Balloc(k);
    *r = k;
    return (char *)(r+1);
    }

 static char *
nrv_alloc(const char *s, char *s0, size_t s0len, char **rve, int n)
{
    char *rv, *t;

    if (!s0)
        s0 = rv_alloc(n);
    else if ((long long)s0len <= n) {
        rv = 0;
        t = rv + n;
        goto rve_chk;
        }
    t = rv = s0;
    while((*t = *s++))
        ++t;
 rve_chk:
    if (rve)
        *rve = t;
    return rv;
    }

/* freedtoa(s) must be used to free values s returned by dtoa.  It should be
 * used in all cases.
 */

 void
Nst_freedtoa(i8 *s)
{
#ifdef MULTIPLE_THREADS
    ThInfo *TI = 0;
#endif
    Bigint *b = (Bigint *)((int *)s - 1);
    b->maxwds = 1 << (b->k = *(int*)b);
    Bfree(b);
}

/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 112-126].
 *
 * Modifications:
 *  1. Rather than iterating, we use a simple numeric overestimate
 *      to determine k = floor(log10(d)).  We scale relevant
 *      quantities using O(log2(k)) rather than O(k) multiplications.
 *  2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *      try to generate digits strictly left to right.  Instead, we
 *      compute with fewer bits and propagate the carry if necessary
 *      when rounding the final digit up.  This is often faster.
 *  3. Under the assumption that input will be rounded nearest,
 *      mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *      That is, we allow equality in stopping tests when the
 *      round-nearest rule will give the same floating-point value
 *      as would satisfaction of the stopping test with strict
 *      inequality.
 *  4. We remove common factors of powers of 2 from relevant
 *      quantities.
 *  5. When converting floating-point integers less than 1e16,
 *      we use floating-point arithmetic rather than resorting
 *      to multiple-precision integers.
 *  6. When asked to produce fewer than 15 digits, we first try
 *      to get by with floating-point arithmetic; we resort to
 *      multiple-precision integer arithmetic only if we cannot
 *      guarantee that the floating-point calculation has given
 *      the correctly rounded result.  For k requested digits and
 *      "uniformly" distributed input, the probability is
 *      something like 10^(k-15) that we must resort to the Long
 *      calculation.
 */

 char *
dtoa_r(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve, char *buf, size_t blen)
{
 /* Arguments ndigits, decpt, sign are similar to those
    of ecvt and fcvt; trailing zeros are suppressed from
    the returned string.  If not null, *rve is set to point
    to the end of the return value.  If d is +-Infinity or NaN,
    then *decpt is set to 9999.

    mode:
        0 ==> shortest string that yields d when read in
            and rounded to nearest.
        1 ==> like 0, but with Steele & White stopping rule;
            e.g. with IEEE P754 arithmetic , mode 0 gives
            1e23 whereas mode 1 gives 9.999999999999999e22.
        2 ==> max(1,ndigits) significant digits.  This gives a
            return value similar to that of ecvt, except
            that trailing zeros are suppressed.
        3 ==> through ndigits past the decimal point.  This
            gives a return value similar to that from fcvt,
            except that trailing zeros are suppressed, and
            ndigits can be negative.
        4,5 ==> similar to 2 and 3, respectively, but (in
            round-nearest mode) with the tests of mode 0 to
            possibly return a shorter string that rounds to d.
            With IEEE arithmetic and compilation with
            -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
            as modes 2 and 3 when FLT_ROUNDS != 1.
        6-9 ==> Debugging modes similar to mode - 4:  don't try
            fast floating-point estimate (if applicable).

        Values of mode other than 0-9 are treated as mode 0.

    When not NULL, buf is an output buffer of length blen, which must
    be large enough to accommodate suppressed trailing zeros and a trailing
    null byte.  If blen is too small, rv = NULL is returned, in which case
    if rve is not NULL, a subsequent call with blen >= (*rve - rv) + 1
    should succeed in returning buf.

    When buf is NULL, sufficient space is allocated for the return value,
    which, when done using, the caller should pass to freedtoa().

    USE_BF is automatically defined when neither NO_LONG_LONG nor NO_BF96
    is defined.
    */

#ifdef MULTIPLE_THREADS
    ThInfo *TI = 0;
#endif
    int bbits, b2, b5, be, dig, i, ilim, ilim1,
        j, j1, k, leftright, m2, m5, s2, s5, spec_case;
#if !defined(Sudden_Underflow) || defined(USE_BF96)
    int denorm;
#endif
    Bigint *b, *b1, *delta, *mlo = NULL, *mhi, *S;
    U u;
    char *s;
#ifdef SET_INEXACT
    int inexact, oldinexact;
#endif
#ifdef USE_BF96 /*{{*/
    BF96 *p10;
    ULLong dbhi, dbits, dblo, den, hb, rb, rblo, res, res0, res3, reslo, sres,
        sulp, tv0, tv1, tv2, tv3, ulp, ulplo, ulpmask, ures, ureslo, zb;
    int eulp, k1, n2, ulpadj, ulpshift;
#else /*}{*/
#ifndef Sudden_Underflow
    ULong x;
#endif
    Long L;
    U d2, eps;
    double ds;
    int ieps, ilim0, k0, k_check, try_quick;
#ifndef No_leftright
#ifdef IEEE_Arith
    U eps1;
#endif
#endif
#endif /*}}*/
#ifdef Honor_FLT_ROUNDS /*{*/
    int Rounding;
#ifdef Trust_FLT_ROUNDS /*{{ only define this if FLT_ROUNDS really works! */
    Rounding = Flt_Rounds;
#else /*}{*/
    Rounding = 1;
    switch(fegetround()) {
     case FE_TOWARDZERO:    Rounding = 0; break;
     case FE_UPWARD:    Rounding = 2; break;
     case FE_DOWNWARD:  Rounding = 3;
     }
#endif /*}}*/
#endif /*}*/

    u.d = dd;
    if (word0(&u) & Sign_bit) {
        /* set sign for everything, including 0's and NaNs */
        *sign = 1;
        word0(&u) &= ~Sign_bit; /* clear sign bit */
        }
    else
        *sign = 0;

#if defined(IEEE_Arith) + defined(VAX)
#ifdef IEEE_Arith
    if ((word0(&u) & Exp_mask) == Exp_mask)
#else
    if (word0(&u)  == 0x8000)
#endif
        {
        /* Infinity or NaN */
        *decpt = 9999;
#ifdef IEEE_Arith
        if (!word1(&u) && !(word0(&u) & 0xfffff))
            return nrv_alloc("Infinity", buf, blen, rve, 8);
#endif
        return nrv_alloc("NaN", buf, blen, rve, 3);
        }
#endif
#ifdef IBM
    dval(&u) += 0; /* normalize */
#endif
    if (!dval(&u)) {
        *decpt = 1;
        return nrv_alloc("0", buf, blen, rve, 1);
        }

#ifdef SET_INEXACT
#ifndef USE_BF96
    try_quick =
#endif
    oldinexact = get_inexact();
    inexact = 1;
#endif
#ifdef Honor_FLT_ROUNDS
    if (Rounding >= 2) {
        if (*sign)
            Rounding = Rounding == 2 ? 0 : 2;
        else
            if (Rounding != 2)
                Rounding = 0;
        }
#endif
#ifdef USE_BF96 /*{{*/
    dbits = (u.LL & 0xfffffffffffffull) << 11;  /* fraction bits */
    if ((be = u.LL >> 52)) /* biased exponent; nonzero ==> normal */ {
        dbits |= 0x8000000000000000ull;
        denorm = ulpadj = 0;
        }
    else {
        denorm = 1;
        ulpadj = be + 1;
        dbits <<= 1;
        if (!(dbits & 0xffffffff00000000ull)) {
            dbits <<= 32;
            be -= 32;
            }
        if (!(dbits & 0xffff000000000000ull)) {
            dbits <<= 16;
            be -= 16;
            }
        if (!(dbits & 0xff00000000000000ull)) {
            dbits <<= 8;
            be -= 8;
            }
        if (!(dbits & 0xf000000000000000ull)) {
            dbits <<= 4;
            be -= 4;
            }
        if (!(dbits & 0xc000000000000000ull)) {
            dbits <<= 2;
            be -= 2;
            }
        if (!(dbits & 0x8000000000000000ull)) {
            dbits <<= 1;
            be -= 1;
            }
        assert(be >= -51);
        ulpadj -= be;
        }
    j = Lhint[be + 51];
    p10 = &pten[j];
    dbhi = dbits >> 32;
    dblo = dbits & 0xffffffffull;
    i = be - 0x3fe;
    if (i < p10->e
    || (i == p10->e && (dbhi < p10->b0 || (dbhi == p10->b0 && dblo < p10->b1))))
        --j;
    k = j - 342;

    /* now 10^k <= dd < 10^(k+1) */

#else /*}{*/

    b = d2b(&u, &be, &bbits);
#ifdef Sudden_Underflow
    i = (int)(word0(&u) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
#else
    if ((i = (int)(word0(&u) >> Exp_shift1 & (Exp_mask>>Exp_shift1)))) {
#endif
        dval(&d2) = dval(&u);
        word0(&d2) &= Frac_mask1;
        word0(&d2) |= Exp_11;
#ifdef IBM
        if (j = 11 - hi0bits(word0(&d2) & Frac_mask))
            dval(&d2) /= 1 << j;
#endif

        /* log(x)   ~=~ log(1.5) + (x-1.5)/1.5
       * log10(x)    =  log(x) / log(10)
       *        ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
       * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
       *
       * This suggests computing an approximation k to log10(d) by
       *
       * k = (i - Bias)*0.301029995663981
       *    + ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
       *
       * We want k to be too large rather than too small.
       * The error in the first-order Taylor series approximation
       * is in our favor, so we just round up the constant enough
       * to compensate for any error in the multiplication of
       * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
       * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
       * adding 1e-13 to the constant term more than suffices.
       * Hence we adjust the constant term to 0.1760912590558.
       * (We could get a more accurate k by invoking log10,
       *  but this is probably not worthwhile.)
       */

        i -= Bias;
#ifdef IBM
        i <<= 2;
        i += j;
#endif
#ifndef Sudden_Underflow
        denorm = 0;
        }
    else {
        /* d is denormalized */

        i = bbits + be + (Bias + (P-1) - 1);
        x = i > 32  ? word0(&u) << (64 - i) | word1(&u) >> (i - 32)
             : word1(&u) << (32 - i);
        dval(&d2) = x;
        word0(&d2) -= 31*Exp_msk1; /* adjust exponent */
        i -= (Bias + (P-1) - 1) + 1;
        denorm = 1;
        }
#endif
    ds = (dval(&d2)-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
    k = (int)ds;
    if (ds < 0. && ds != k)
        k--;    /* want k = floor(ds) */
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
        }
    else {
        b2 = -j;
        s2 = 0;
        }
    if (k >= 0) {
        b5 = 0;
        s5 = k;
        s2 += k;
        }
    else {
        b2 -= k;
        b5 = -k;
        s5 = 0;
        }
#endif /*}}*/
    if (mode < 0 || mode > 9)
        mode = 0;

#ifndef USE_BF96
#ifndef SET_INEXACT
#ifdef Check_FLT_ROUNDS
    try_quick = Rounding == 1;
#endif
#endif /*SET_INEXACT*/
#endif

    if (mode > 5) {
        mode -= 4;
#ifndef USE_BF96
        try_quick = 0;
#endif
        }
    leftright = 1;
    ilim = ilim1 = -1;  /* Values for cases 0 and 1; done here to */
                /* silence erroneous "gcc -Wall" warning. */
    switch(mode) {
        case 0:
        case 1:
            i = 18;
            ndigits = 0;
            break;
        case 2:
            leftright = 0;
            /* no break */
        case 4:
            if (ndigits <= 0)
                ndigits = 1;
            ilim = ilim1 = i = ndigits;
            break;
        case 3:
            leftright = 0;
            /* no break */
        case 5:
            i = ndigits + k + 1;
            ilim = i;
            ilim1 = i - 1;
            if (i <= 0)
                i = 1;
        }
    if (!buf) {
        buf = rv_alloc(i);
        blen = sizeof(Bigint) + ((1 << ((int*)buf)[-1]) - 1)*sizeof(ULong) - sizeof(int);
        }
    else if ((long long)blen <= i) {
        buf = 0;
        if (rve)
            *rve = buf + i;
        return buf;
        }
    s = buf;

    /* Check for special case that d is a normalized power of 2. */

    spec_case = 0;
    if (mode < 2 || (leftright
#ifdef Honor_FLT_ROUNDS
            && Rounding == 1
#endif
                )) {
        if (!word1(&u) && !(word0(&u) & Bndry_mask)
#ifndef Sudden_Underflow
       && word0(&u) & (Exp_mask & ~Exp_msk1)
#endif
                ) {
            /* The special case */
            spec_case = 1;
            }
        }

#ifdef USE_BF96 /*{*/
    b = 0;
    if (ilim < 0 && (mode == 3 || mode == 5)) {
        S = mhi = 0;
        goto no_digits;
        }
    i = 1;
    j = 52 + 0x3ff - be;
    ulpshift = 0;
    ulplo = 0;
    /* Can we do an exact computation with 64-bit integer arithmetic? */
    if (k < 0) {
        if (k < -25)
            goto toobig;
        res = dbits >> 11;
        n2 = pfivebits[k1 = -(k + 1)] + 53;
        j1 = j;
        if (n2 > 61) {
            ulpshift = n2 - 61;
            if (res & (ulpmask = (1ull << ulpshift) - 1))
                goto toobig;
            j -= ulpshift;
            res >>= ulpshift;
            }
        /* Yes. */
        res *= ulp = pfive[k1];
        if (ulpshift) {
            ulplo = ulp;
            ulp >>= ulpshift;
            }
        j += k;
        if (ilim == 0) {
            S = mhi = 0;
            if (res > (5ull << j))
                goto one_digit;
            goto no_digits;
            }
        goto no_div;
        }
    if (ilim == 0 && j + k >= 0) {
        S = mhi = 0;
        if ((dbits >> 11) > (pfive[k-1] << j))
            goto one_digit;
        goto no_digits;
        }
    if (k <= dtoa_divmax && j + k >= 0) {
        /* Another "yes" case -- we will use exact integer arithmetic. */
 use_exact:
        Debug(++dtoa_stats[3]);
        res = dbits >> 11;  /* residual */
        ulp = 1;
        if (k <= 0)
            goto no_div;
        j1 = j + k + 1;
        den = pfive[k-i] << (j1 - i);
        for(;;) {
            dig = res / den;
            *s++ = '0' + dig;
            if (!(res -= dig*den)) {
#ifdef SET_INEXACT
                inexact = 0;
                oldinexact = 1;
#endif
                goto retc;
                }
            if (ilim < 0) {
                ures = den - res;
                if (2*res <= ulp
                && (spec_case ? 4*res <= ulp : (2*res < ulp || dig & 1)))
                    goto ulp_reached;
                if (2*ures < ulp)
                    goto Roundup;
                }
            else if (i == ilim) {
                switch(Rounding) {
              case 0: goto retc;
              case 2: goto Roundup;
              }
                ures = 2*res;
                if (ures > den
                || (ures == den && dig & 1)
                || (spec_case && res <= ulp && 2*res >= ulp))
                    goto Roundup;
                goto retc;
                }
            if (j1 < ++i) {
                res *= 10;
                ulp *= 10;
                }
            else {
                if (i > k)
                    break;
                den = pfive[k-i] << (j1 - i);
                }
            }
 no_div:
        for(;;) {
            dig = den = res >> j;
            *s++ = '0' + dig;
            if (!(res -= den << j)) {
#ifdef SET_INEXACT
                inexact = 0;
                oldinexact = 1;
#endif
                goto retc;
                }
            if (ilim < 0) {
                ures = (1ull << j) - res;
                if (2*res <= ulp
                && (spec_case ? 4*res <= ulp : (2*res < ulp || dig & 1))) {
 ulp_reached:
                    if (ures < res
                    || (ures == res && dig & 1))
                        goto Roundup;
                    goto retc;
                    }
                if (2*ures < ulp)
                    goto Roundup;
                }
            --j;
            if (i == ilim) {
#ifdef Honor_FLT_ROUNDS
                switch(Rounding) {
              case 0: goto retc;
              case 2: goto Roundup;
              }
#endif
                hb = 1ull << j;
                if (res & hb && (dig & 1 || res & (hb-1)))
                    goto Roundup;
                if (spec_case && res <= ulp && 2*res >= ulp) {
 Roundup:
                    while(*--s == '9')
                        if (s == buf) {
                            ++k;
                            *s++ = '1';
                            goto ret1;
                            }
                    ++*s++;
                    goto ret1;
                    }
                goto retc;
                }
            ++i;
            res *= 5;
            if (ulpshift) {
                ulplo = 5*(ulplo & ulpmask);
                ulp = 5*ulp + (ulplo >> ulpshift);
                }
            else
                ulp *= 5;
            }
        }
 toobig:
    if (ilim > 28)
        goto Fast_failed1;
    /* Scale by 10^-k */
    p10 = &pten[342-k];
    tv0 = p10->b2 * dblo; /* rarely matters, but does, e.g., for 9.862818194192001e18 */
    tv1 = p10->b1 * dblo + (tv0 >> 32);
    tv2 = p10->b2 * dbhi + (tv1 & 0xffffffffull);
    tv3 = p10->b0 * dblo + (tv1>>32) + (tv2>>32);
    res3 = p10->b1 * dbhi + (tv3 & 0xffffffffull);
    res = p10->b0 * dbhi + (tv3>>32) + (res3>>32);
    be += p10->e - 0x3fe;
    eulp = j1 = be - 54 + ulpadj;
    if (!(res & 0x8000000000000000ull)) {
        --be;
        res3 <<= 1;
        res = (res << 1) | ((res3 & 0x100000000ull) >> 32);
        }
    res0 = res; /* save for Fast_failed */
#if !defined(SET_INEXACT) && !defined(NO_DTOA_64) /*{*/
    if (ilim > 19)
        goto Fast_failed;
    Debug(++dtoa_stats[4]);
    assert(be >= 0 && be <= 4); /* be = 0 is rare, but possible, e.g., for 1e20 */
    res >>= 4 - be;
    ulp = p10->b0;  /* ulp */
    ulp = (ulp << 29) | (p10->b1 >> 3);
    /* scaled ulp = ulp * 2^(eulp - 60) */
    /* We maintain 61 bits of the scaled ulp. */
    if (ilim == 0) {
        if (!(res & 0x7fffffffffffffeull)
       || !((~res) & 0x7fffffffffffffeull))
            goto Fast_failed1;
        S = mhi = 0;
        if (res >= 0x5000000000000000ull)
            goto one_digit;
        goto no_digits;
        }
    rb = 1; /* upper bound on rounding error */
    for(;;++i) {
        dig = res >> 60;
        *s++ = '0' + dig;
        res &= 0xfffffffffffffffull;
        if (ilim < 0) {
            ures = 0x1000000000000000ull - res;
            if (eulp > 0) {
                assert(eulp <= 4);
                sulp = ulp << (eulp - 1);
                if (res <= ures) {
                    if (res + rb > ures - rb)
                        goto Fast_failed;
                    if (res < sulp)
                        goto retc;
                    }
                else {
                    if (res - rb <= ures + rb)
                        goto Fast_failed;
                    if (ures < sulp)
                        goto Roundup;
                    }
                }
            else {
                zb = -(1ull << (eulp + 63));
                if (!(zb & res)) {
                    sres = res << (1 - eulp);
                    if (sres < ulp && (!spec_case || 2*sres < ulp)) {
                        if ((res+rb) << (1 - eulp) >= ulp)
                            goto Fast_failed;
                        if (ures < res) {
                            if (ures + rb >= res - rb)
                                goto Fast_failed;
                            goto Roundup;
                            }
                        if (ures - rb < res + rb)
                            goto Fast_failed;
                        goto retc;
                        }
                    }
                if (!(zb & ures) && ures << -eulp < ulp) {
                    if (ures << (1 - eulp) < ulp)
                        goto  Roundup;
                    goto Fast_failed;
                    }
                }
            }
        else if (i == ilim) {
            ures = 0x1000000000000000ull - res;
            if (ures < res) {
                if (ures <= rb || res - rb <= ures + rb) {
                    if (j + k >= 0 && k >= 0 && k <= 27)
                        goto use_exact1;
                    goto Fast_failed;
                    }
#ifdef Honor_FLT_ROUNDS
                if (Rounding == 0)
                    goto retc;
#endif
                goto Roundup;
                }
            if (res <= rb || ures - rb <= res + rb) {
                if (j + k >= 0 && k >= 0 && k <= 27) {
 use_exact1:
                    s = buf;
                    i = 1;
                    goto use_exact;
                    }
                goto Fast_failed;
                }
#ifdef Honor_FLT_ROUNDS
            if (Rounding == 2)
                goto Roundup;
#endif
            goto retc;
            }
        rb *= 10;
        if (rb >= 0x1000000000000000ull)
            goto Fast_failed;
        res *= 10;
        ulp *= 5;
        if (ulp & 0x8000000000000000ull) {
            eulp += 4;
            ulp >>= 3;
            }
        else {
            eulp += 3;
            ulp >>= 2;
            }
        }
#endif /*}*/
#ifndef NO_BF96
 Fast_failed:
#endif
    Debug(++dtoa_stats[5]);
    s = buf;
    i = 4 - be;
    res = res0 >> i;
    reslo = 0xffffffffull & res3;
    if (i)
        reslo = (res0 << (64 - i)) >> 32 | (reslo >> i);
    rb = 0;
    rblo = 4; /* roundoff bound */
    ulp = p10->b0;  /* ulp */
    ulp = (ulp << 29) | (p10->b1 >> 3);
    eulp = j1;
    for(i = 1;;++i) {
        dig = res >> 60;
        *s++ = '0' + dig;
        res &= 0xfffffffffffffffull;
#ifdef SET_INEXACT
        if (!res && !reslo) {
            if (!(res3 & 0xffffffffull)) {
                inexact = 0;
                oldinexact = 1;
                }
            goto retc;
            }
#endif
        if (ilim < 0) {
            ures = 0x1000000000000000ull - res;
            ureslo = 0;
            if (reslo) {
                ureslo = 0x100000000ull - reslo;
                --ures;
                }
            if (eulp > 0) {
                assert(eulp <= 4);
                sulp = (ulp << (eulp - 1)) - rb;
                if (res <= ures) {
                    if (res < sulp) {
                        if (res+rb < ures-rb)
                            goto retc;
                        }
                    }
                else if (ures < sulp) {
                    if (res-rb > ures+rb)
                        goto Roundup;
                    }
                goto Fast_failed1;
                }
            else {
                zb = -(1ull << (eulp + 60));
                if (!(zb & (res + rb))) {
                    sres = (res - rb) << (1 - eulp);
                    if (sres < ulp && (!spec_case || 2*sres < ulp)) {
                        sres = res << (1 - eulp);
                        if ((j = eulp + 31) > 0)
                            sres += (rblo + reslo) >> j;
                        else
                            sres += (rblo + reslo) << -j;
                        if (sres + (rb << (1 - eulp)) >= ulp)
                            goto Fast_failed1;
                        if (sres >= ulp)
                            goto more96;
                        if (ures < res
                        || (ures == res && ureslo < reslo)) {
                            if (ures + rb >= res - rb)
                                goto Fast_failed1;
                            goto Roundup;
                            }
                        if (ures - rb <= res + rb)
                            goto Fast_failed1;
                        goto retc;
                        }
                    }
                if (!(zb & ures) && (ures-rb) << (1 - eulp) < ulp) {
                    if ((ures + rb) << (1 - eulp) < ulp)
                        goto Roundup;
                    goto Fast_failed1;
                    }
                }
            }
        else if (i == ilim) {
            ures = 0x1000000000000000ull - res;
            sres = ureslo = 0;
            if (reslo) {
                ureslo = 0x100000000ull - reslo;
                --ures;
                sres = (reslo + rblo) >> 31;
                }
            sres += 2*rb;
            if (ures <= res) {
                if (ures <=sres || res - ures <= sres)
                    goto Fast_failed1;
#ifdef Honor_FLT_ROUNDS
                if (Rounding == 0)
                    goto retc;
#endif
                goto Roundup;
                }
            if (res <= sres || ures - res <= sres)
                goto Fast_failed1;
#ifdef Honor_FLT_ROUNDS
            if (Rounding == 2)
                goto Roundup;
#endif
            goto retc;
            }
 more96:
        rblo *= 10;
        rb = 10*rb + (rblo >> 32);
        rblo &= 0xffffffffull;
        if (rb >= 0x1000000000000000ull)
            goto Fast_failed1;
        reslo *= 10;
        res = 10*res + (reslo >> 32);
        reslo &= 0xffffffffull;
        ulp *= 5;
        if (ulp & 0x8000000000000000ull) {
            eulp += 4;
            ulp >>= 3;
            }
        else {
            eulp += 3;
            ulp >>= 2;
            }
        }
 Fast_failed1:
    Debug(++dtoa_stats[6]);
    S = mhi = mlo = 0;
#ifdef USE_BF96
    b = d2b(&u, &be, &bbits);
#endif
    s = buf;
    i = (int)(word0(&u) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
    i -= Bias;
    if (ulpadj)
        i -= ulpadj - 1;
    j = bbits - i - 1;
    if (j >= 0) {
        b2 = 0;
        s2 = j;
        }
    else {
        b2 = -j;
        s2 = 0;
        }
    if (k >= 0) {
        b5 = 0;
        s5 = k;
        s2 += k;
        }
    else {
        b2 -= k;
        b5 = -k;
        s5 = 0;
        }
#endif /*}*/

#ifdef Honor_FLT_ROUNDS
    if (mode > 1 && Rounding != 1)
        leftright = 0;
#endif

#ifndef USE_BF96 /*{*/
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
            for(; j; j >>= 1, i++)
                if (j & 1) {
                    ieps++;
                    ds *= bigtens[i];
                    }
            dval(&u) /= ds;
            }
        else if (j1 > 0) {
            dval(&u) *= tens[j1 & 0xf];
            for(j = j1 >> 4; j; j >>= 1, i++)
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
#ifndef No_leftright
        if (leftright) {
            /* Use Steele & White method of only
          * generating digits needed.
          */
            dval(&eps) = 0.5/tens[ilim-1] - dval(&eps);
#ifdef IEEE_Arith
            if (j1 >= 307) {
                eps1.d = 1.01e256; /* 1.01 allows roundoff in the next few lines */
                word0(&eps1) -= Exp_msk1 * (Bias+P-1);
                dval(&eps1) *= tens[j1 & 0xf];
                for(i = 0, j = (j1-256) >> 4; j; j >>= 1, i++)
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
#endif
            for(i = 0;;) {
                L = dval(&u);
                dval(&u) -= L;
                *s++ = '0' + (int)L;
                if (1. - dval(&u) < dval(&eps))
                    goto bump_up;
                if (dval(&u) < dval(&eps))
                    goto retc;
                if (++i >= ilim)
                    break;
                dval(&eps) *= 10.;
                dval(&u) *= 10.;
                }
            }
        else {
#endif
            /* Generate ilim digits, then fix them up. */
            dval(&eps) *= tens[ilim-1];
            for(i = 1;; i++, dval(&u) *= 10.) {
                L = (Long)(dval(&u));
                if (!(dval(&u) -= L))
                    ilim = i;
                *s++ = '0' + (int)L;
                if (i == ilim) {
                    if (dval(&u) > 0.5 + dval(&eps))
                        goto bump_up;
                    else if (dval(&u) < 0.5 - dval(&eps))
                        goto retc;
                    break;
                    }
                }
#ifndef No_leftright
            }
#endif
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
        for(i = 1;; i++, dval(&u) *= 10.) {
            L = (Long)(dval(&u) / ds);
            dval(&u) -= L*ds;
#ifdef Check_FLT_ROUNDS
            /* If FLT_ROUNDS == 2, L will usually be high by 1 */
            if (dval(&u) < 0) {
                L--;
                dval(&u) += ds;
                }
#endif
            *s++ = '0' + (int)L;
            if (!dval(&u)) {
#ifdef SET_INEXACT
                inexact = 0;
#endif
                break;
                }
            if (i == ilim) {
#ifdef Honor_FLT_ROUNDS
                if (mode > 1)
                switch(Rounding) {
              case 0: goto retc;
              case 2: goto bump_up;
              }
#endif
                dval(&u) += dval(&u);
#ifdef ROUND_BIASED
                if (dval(&u) >= ds)
#else
                if (dval(&u) > ds || (dval(&u) == ds && L & 1))
#endif
                    {
 bump_up:
                    while(*--s == '9')
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

#endif /*}*/
    m2 = b2;
    m5 = b5;
    mhi = mlo = 0;
    if (leftright) {
        i =
#ifndef Sudden_Underflow
            denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
#ifdef IBM
            1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
            1 + P - bbits;
#endif
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
            }
        else
            b = pow5mult(b, b5);
        }
    S = i2b(1);
    if (s5 > 0)
        S = pow5mult(S, s5);

    if (spec_case) {
        b2 += Log2P;
        s2 += Log2P;
        }

    /* Arrange for convenient computation of quotients:
    * shift left if necessary so divisor has 4 leading 0 bits.
    *
    * Perhaps we should just compute leading 28 bits of S once
    * and for all and pass them and a shift to quorem, so it
    * can do shifts and ors to compute the numerator for q.
    */
    i = dshift(S, s2);
    b2 += i;
    m2 += i;
    s2 += i;
    if (b2 > 0)
        b = lshift(b, b2);
    if (s2 > 0)
        S = lshift(S, s2);
#ifndef USE_BF96
    if (k_check) {
        if (cmp(b,S) < 0) {
            k--;
            b = multadd(b, 10, 0);  /* we botched the k estimate */
            if (leftright)
                mhi = multadd(mhi, 10, 0);
            ilim = ilim1;
            }
        }
#endif
    if (ilim <= 0 && (mode == 3 || mode == 5)) {
        if (ilim < 0 || cmp(b,S = multadd(S,5,0)) <= 0) {
            /* no digits, fcvt style */
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

        /* Compute mlo -- check for special case
       * that d is a normalized power of 2.
       */

        mlo = mhi;
        if (spec_case) {
            mhi = Balloc(mhi->k);
            Bcopy(mhi, mlo);
            mhi = lshift(mhi, Log2P);
            }

        for(i = 1;;i++) {
            dig = quorem(b,S) + '0';
            /* Do we yet have the shortest decimal string
          * that will round to d?
          */
            j = cmp(b, mlo);
            delta = diff(S, mhi);
            j1 = delta->sign ? 1 : cmp(b, delta);
            Bfree(delta);
#ifndef ROUND_BIASED
            if (j1 == 0 && mode != 1 && !(word1(&u) & 1)
#ifdef Honor_FLT_ROUNDS
                && (mode <= 1 || Rounding >= 1)
#endif
                           ) {
                if (dig == '9')
                    goto round_9_up;
                if (j > 0)
                    dig++;
#ifdef SET_INEXACT
                else if (!b->x[0] && b->wds <= 1)
                    inexact = 0;
#endif
                *s++ = dig;
                goto ret;
                }
#endif
            if (j < 0 || (j == 0 && mode != 1
#ifndef ROUND_BIASED
                            && !(word1(&u) & 1)
#endif
                    )) {
                if (!b->x[0] && b->wds <= 1) {
#ifdef SET_INEXACT
                    inexact = 0;
#endif
                    goto accept_dig;
                    }
#ifdef Honor_FLT_ROUNDS
                if (mode > 1)
             switch(Rounding) {
              case 0: goto accept_dig;
              case 2: goto keep_dig;
              }
#endif /*Honor_FLT_ROUNDS*/
                if (j1 > 0) {
                    b = lshift(b, 1);
                    j1 = cmp(b, S);
#ifdef ROUND_BIASED
                    if (j1 >= 0 /*)*/
#else
                    if ((j1 > 0 || (j1 == 0 && dig & 1))
#endif
                    && dig++ == '9')
                        goto round_9_up;
                    }
 accept_dig:
                *s++ = dig;
                goto ret;
                }
            if (j1 > 0) {
#ifdef Honor_FLT_ROUNDS
                if (!Rounding && mode > 1)
                    goto accept_dig;
#endif
                if (dig == '9') { /* possible if i == 1 */
 round_9_up:
                    *s++ = '9';
                    goto roundoff;
                    }
                *s++ = dig + 1;
                goto ret;
                }
#ifdef Honor_FLT_ROUNDS
 keep_dig:
#endif
            *s++ = dig;
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
        }
    else
        for(i = 1;; i++) {
            dig = quorem(b,S) + '0';
            *s++ = dig;
            if (!b->x[0] && b->wds <= 1) {
#ifdef SET_INEXACT
                inexact = 0;
#endif
                goto ret;
                }
            if (i >= ilim)
                break;
            b = multadd(b, 10, 0);
            }

    /* Round off last digit */

#ifdef Honor_FLT_ROUNDS
    if (mode > 1)
        switch(Rounding) {
        case 0: goto ret;
        case 2: goto roundoff;
        }
#endif
    b = lshift(b, 1);
    j = cmp(b, S);
#ifdef ROUND_BIASED
    if (j >= 0)
#else
    if (j > 0 || (j == 0 && dig & 1))
#endif
        {
 roundoff:
        while(*--s == '9')
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
    while(s > buf && s[-1] == '0')
        --s;
 ret1:
    if (b)
        Bfree(b);
    *s = 0;
    *decpt = k + 1;
    if (rve)
        *rve = s;
#ifdef SET_INEXACT
    if (inexact) {
        if (!oldinexact) {
            word0(&u) = Exp_1 + (70 << Exp_shift);
            word1(&u) = 0;
            dval(&u) += 1.;
            }
        }
    else if (!oldinexact)
        clear_inexact();
#endif
    return buf;
    }

i8 *Nst_dtoa(f64 dd, int mode, int ndigits, int *decpt, int *sign, i8 **rve)
{
    /*  Sufficient space is allocated to the return value
        to hold the suppressed trailing zeros.
        See dtoa_r() above for details on the other arguments.
    */
    return dtoa_r(dd, mode, ndigits, decpt, sign, rve, 0, 0);
}
