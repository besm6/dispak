/*
 * BESM-6 arithmetic instructions.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING" for more details.
 */
#include <math.h>
#include "defs.h"

double
get_real (alureg_t word)
{
	int exponent = (word.l >> 17) - 64;
	int64_t mantissa = ((int64_t) word.l << 24 | word.r) << (64 - 48 + 7);
        return ldexp(mantissa, exponent - 63);
}

#define ABS(x) ((x) < 0 ? -x : x)
#define BESM_TO_INT64(from,to) {\
	to = from.mr | (int64_t)from.ml << 24;\
        if (from.ml & 0x10000) to |= -1ll << 40;\
}

void NEGATE(alureg_t* pR) { 
    alureg_t R = *pR;
          
    int was_neg = NEGATIVE(R); 
    if (was_neg) 
        (R).ml |= 0x20000; 
    (R).mr = (~(R).mr & 0xffffff) + 1; 
    (R).ml = (~(R).ml + ((R).mr >> 24)) & 0x3ffff; 
    (R).mr &= 0xffffff; 
    if (!was_neg && NEGATIVE(R))
        (R).ml |= 0x20000;
    *pR = R;
}

int
add()
{
	alureg_t        a1, a2;
	int             diff, neg;

	diff = acc.o - enreg.o;
	if (diff < 0) {
		diff = -diff;
		a1 = acc;
		a2 = enreg;
	} else {
		a1 = enreg;
		a2 = acc;
	}
	// operands may be pre-negated, look at the second sign bit
	neg = a1.ml & 0x20000;
	if (!diff)
		/*
		accex.o = accex.ml = accex.mr = 0;
		*/      ;
	else if (diff <= 16) {
		int rdiff = 16 - diff;
		/*
		accex.mr = 0;
		*/      ;
		rnd_rq = (accex.ml = (a1.mr << rdiff) & 0xffff) != 0;
		a1.mr = ((a1.mr >> diff) | (a1.ml << (rdiff + 8))) & 0xffffff;
		a1.ml = ((a1.ml >> diff) |
			(neg ? ~0 << rdiff : 0)) & 0x3ffff;
	} else if (diff <= 40) {
		diff -= 16;
		rnd_rq = (accex.mr = (a1.mr << (24 - diff)) & 0xffffff) != 0;
/* было                rnd_rq |= (accex.ml = (((a1.mr & 0xff0000) >> diff) |   */
		rnd_rq |= (accex.ml = ((a1.mr >> diff) |
			(a1.ml << (24 - diff))) & 0xffff) != 0;
		a1.mr = ((((a1.mr >> 16) | (a1.ml << 8)) >> diff) |
				(neg ? (~0l << (24 - diff)) : 0)) & 0xffffff;
		a1.ml = neg ? 0x3ffff : 0;
	} else if (diff <= 56) {
		int rdiff = 16 - (diff -= 40);
		rnd_rq = a1.ml || a1.mr;
		accex.mr = ((a1.mr >> diff) | (a1.ml << (rdiff + 8))) & 0xffffff;
		accex.ml = ((a1.ml >> diff) |
			(neg ? ~0 << rdiff : 0)) & 0xffff;
		if (neg) {
			a1.mr = 0xffffff;
			a1.ml = 0x3ffff;
		} else
			a1.ml = a1.mr = 0;
	} else if (diff <= 80) {
		diff -= 56;
		rnd_rq = a1.ml || a1.mr;
		accex.mr = ((((a1.mr >> 16) | (a1.ml << 8)) >> diff) |
				(neg ? (~0l << (24 - diff)) : 0)) & 0xffffff;
		accex.ml = neg ? 0x3ffff : 0;
		if (neg) {
			a1.mr = 0xffffff;
			accex.ml = a1.ml = 0x3ffff;
		} else
			accex.ml = a1.ml = a1.mr = 0;
	} else {
		rnd_rq = a1.ml || a1.mr;
		if (neg) {
			accex.ml = 0xffff;
			a1.mr = accex.mr = 0xffffff;
			a1.ml = 0x3ffff;
		} else
			accex.ml = accex.mr = a1.ml = a1.mr = 0;
	}
	acc.o = a2.o;
	acc.mr = a1.mr + a2.mr;
	acc.ml = a1.ml + a2.ml + (acc.mr >> 24);
	acc.mr &= 0xffffff;
	return E_SUCCESS;
}

int
aax()
{
	acc.l &= enreg.l;
	acc.r &= enreg.r;
	accex = zeroword;
	return E_SUCCESS;
}

int
aex()
{
	accex = acc;
	acc.l ^= enreg.l;
	acc.r ^= enreg.r;
	return E_SUCCESS;
}

int
arx()
{
	uint            i;

	acc.r = (i = acc.r + enreg.r) & 0xffffff;
	acc.l = (i = acc.l + enreg.l + (i >> 24)) & 0xffffff;

	if (i & 0x1000000) {
		acc.r = (i = acc.r + 1) & 0xffffff;
		acc.l = acc.l + (i >> 24);
	}

	accex = zeroword;
	return E_SUCCESS;
}

int
avx()
{
	if (NEGATIVE(enreg))
		NEGATE(&acc);
	return E_SUCCESS;
}

int
aox()
{
	acc.l |= enreg.l;
	acc.r |= enreg.r;
	accex = zeroword;
	return E_SUCCESS;
}

/*
 * non-restoring division
 */
uint64_t
nrdiv (int64_t nn, int64_t dd, int * expdiff)
{
	int64_t res = 0;
	int64_t q = 1LL << 40;
	nn *= 2;
	dd *= 2;
	if (ABS(nn) >= ABS(dd))
		nn/=2, (*expdiff)++;

        if (dd == 1ll << 40)
            return nn;          /* dividing by a power of 2 */

	while (q > 1) {
		if (nn == 0)
			break;

		if (ABS(nn) < (1LL << 39)) {
			nn *= 2;	/* magic shortcut */
		} else if ((nn > 0) ^ (dd > 0)) {
			res -= q;
			nn = 2*nn+dd;
		} else {
			res += q;
			nn = 2*nn-dd;
		}
		q /= 2;
	}
	return res / 2;
}

int
b6div()
{
	int64_t         dividend, divisor, quotient;
	int expdiff;
	accex.o = accex.ml = accex.mr = 0;
	if ((enreg.ml & 0x18000) == 0 || (enreg.ml & 0x18000) == 0x18000)
		return E_ZERODIV;
	if ((acc.ml == 0) && (acc.mr == 0)) {
qzero:
		acc = zeroword;
		return E_SUCCESS;
	}

	BESM_TO_INT64(acc, dividend);
	BESM_TO_INT64(enreg, divisor);

	expdiff = acc.o - enreg.o;
	quotient = nrdiv(dividend, divisor, &expdiff);

	if (expdiff < -64)
		goto qzero;
	acc.o = (expdiff+64) & 0x7f;
        acc.ml = quotient >> 24;
        acc.mr = quotient & 0xffffff;
	if ((expdiff > 63) && !dis_exc)
		return E_OVFL;
	return E_SUCCESS;
}

int
elfun(int fun)
{
#ifdef DIV_NATIVE
	int             o;
	double          arg;

	accex.o = accex.ml = accex.mr = 0;
	UNPCK(acc);
        arg = get_real(acc);
        if ((arg < 0x1p-65 && arg >= -0x1p-65)) {
            // If the operand was out of range for normalized numbers,
            // flush to zero.
            arg = 0;
        }
	switch (fun) {
	case EF_SQRT:
		arg = sqrt(arg);
		if (isnan(arg)) {
			return E_SQRT;
		}
		break;
	case EF_SIN:
		arg = sin(arg);
		break;
	case EF_COS:
		arg = cos(arg);
		break;
	case EF_ARCTG:
		arg = atan(arg);
		break;
	case EF_ARCSIN:
		arg = asin(arg);
		if (isnan(arg)) {
			return E_ASIN;
		}
		break;
	case EF_ALOG:
		arg = log(arg);
		if (isnan(arg)) {
			return E_ALOG;
		}
		break;
	case EF_EXP:
		arg = exp(arg);
		if (isinf(arg)) {
			return E_EXP;
		}
		break;
	case EF_ENTIER:
		arg = floor(arg);
		break;
	default:
		return E_INT;
	}

        arg = frexp(arg, &o);
        if (arg == -0.5) { arg = -1.0; --o; }
        o += 64;
	if (arg == 0 || o < 0) {
		// biased exponent is negative,
		// flush to zero
		acc = zeroword;
		return E_SUCCESS;
	}
        
	acc.o = o & 0x7f;
	acc.ml = (uint64_t) (arg * 0x10000000000LL) >> 24;
	acc.mr = (uint64_t) (arg * 0x10000000000LL) & 0xFFFFFF;
	if ((o > 0x7f) && !dis_exc)
		return E_EXP;	// the only one that can overflow
	PACK(acc)
	return E_SUCCESS;

#else
	return E_UNIMP;
#endif
}

inline void mult64to128(uint64_t u, uint64_t v, uint64_t* h, uint64_t* l)
{
    uint64_t u1 = (u & 0xffffffff);
    uint64_t v1 = (v & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);
    uint64_t w1;
    
    u >>= 32;
    t = (u * v1) + k;
    k = (t & 0xffffffff);
    w1 = (t >> 32);
    
    v >>= 32;
    t = (u1 * v) + k;
    k = (t >> 32);
    
    *h = (u * v) + w1 + k;
    *l = (t << 32) + w3;
}

int
mul()
{
	uchar           neg = 0;
	alureg_t        a, b;
        uint64_t        aval, bval;
#if HAVE_INT128        
	typedef unsigned __int128   uint128_t;
        uint128_t       prod;
#else
        uint64_t        prodhi, prodlo;
#endif        
	a = acc;
	b = enreg;

	if ((!a.l && !a.r) || (!b.l && !b.r)) {
		/* multiplication by zero is zero */
		acc.l = acc.r = acc.o = acc.ml =
		accex.l = accex.r = accex.o = accex.ml = 0;
		rnd_rq = 0;
		return E_SUCCESS;
	}

	if (NEGATIVE(a)) {
		neg = 1;
		NEGATE(&a);
	}
	if (NEGATIVE(b)) {
		neg ^= 1;
		NEGATE(&b);
	}
	acc.o = a.o + b.o - 64;
        aval = a.ml;
        aval = aval << 24 | a.mr;
        bval = b.ml;
        bval = bval << 24 | b.mr;

#if HAVE_INT128
        prod = (uint128_t) aval * bval;
        if (neg) {
            prod = -prod;
        }

        accex.mr = prod & 0xFFFFFF;
        prod >>= 24;
        accex.ml = prod & 0xFFFF;
        prod >>= 16;
        acc.mr = prod & 0xFFFFFF;
        prod >>= 24;
        acc.ml = prod & 0x3FFFF;
#else
        mult64to128(aval, bval, &prodhi, &prodlo);
        if (neg) {
            prodlo = -prodlo;
            prodhi = ~prodhi + !prodlo;
        }
        accex.mr = prodlo & 0xFFFFFF;
        prodlo >>= 24;
        accex.ml = prodlo & 0xFFFF;
        prodlo >>= 16;
        acc.mr = prodlo & 0xFFFFFF;
        acc.ml = prodhi & 0x3FFFF;
        
#endif
	rnd_rq = !!(accex.ml | accex.mr);

	return E_SUCCESS;
}

int
apx()
{
	for (accex.l = accex.r = 0; enreg.r; enreg.r >>= 1, acc.r >>= 1)
		if (enreg.r & 1) {
			accex.r = ((accex.r >> 1) | (accex.l << 23)) & HALFW;
			accex.l >>= 1;
			if (acc.r & 1)
				accex.l |= 0x800000;
		}
	for (; enreg.l; enreg.l >>= 1, acc.l >>= 1)
		if (enreg.l & 1) {
			accex.r = ((accex.r >> 1) | (accex.l << 23)) & HALFW;
			accex.l >>= 1;
			if (acc.l & 1)
				accex.l |= 0x800000;
		}
	acc = accex;
	accex.l = accex.r = 0;
	return E_SUCCESS;
}

int
aux()
{
	int     i;

	accex.l = accex.r = 0;
	for (i = 0; i < 24; ++i) {
		accex.l <<= 1;
		if (enreg.l & 0x800000) {
			if (acc.l & 0x800000)
				accex.l |= 1;
			acc.l = (acc.l << 1) | (acc.r >> 23);
			acc.r = (acc.r << 1) & HALFW;
		}
		enreg.l <<= 1;
	}
	for (i = 0; i < 24; ++i) {
		accex.r <<= 1;
		if (enreg.r & 0x800000) {
			if (acc.l & 0x800000)
				accex.r |= 1;
			acc.l = (acc.l << 1);
		}
		enreg.r <<= 1;
	}
	acc.l = accex.l;
	acc.r = accex.r;
	accex.l = accex.r = 0;
	return E_SUCCESS;
}

int
acx()
{
	int     c = 0;
	uint    i;

	for (i = acc.l; i; i &= i - 1, c++);
	for (i = acc.r; i; i &= i - 1, c++);
	acc.r = c;
	acc.l = 0;
	return arx();
}

int
anx()
{
	uint    c;
	uint    i;
	uchar   b;

	if (acc.l) {
		i = acc.l;
		c = 1;
	} else if (acc.r) {
		i = acc.r;
		c = 25;
	} else {
		acc = enreg;
		accex.l = accex.r = 0;
		return E_SUCCESS;
	}
	if (i & 0xff0000)
		b = i >> 16;
	else if (i & 0xff00) {
		b = i >> 8;
		c += 8;
	} else {
		b = i;
		c += 16;
	}
	while (!(b & 0x80)) {
		b <<= 1;
		++c;
	}

	enreg.o = 64 + 48 - c;
	asx();

	acc.r = (i = c + enreg.r) & 0xffffff;
	acc.l = (i = enreg.l + (i >> 24)) & 0xffffff;

	if (i & 0x1000000) {
		acc.r = (i = acc.r + 1) & 0xffffff;
		acc.l = acc.l + (i >> 24);
	}

	return E_SUCCESS;
}

int
epx()
{
	acc.o += enreg.o - 64;
	return E_SUCCESS;
}

int
emx()
{
	acc.o += 64 - enreg.o;
	return E_SUCCESS;
}

int
asx()
{
	int     i, j;

	accex.l = accex.r = 0;
	if (!(i = enreg.o - 64))
		return E_SUCCESS;
	if (i > 0) {
		if (i < 24) {
			j = 24 - i;
			accex.l = (acc.r << j) & 0xffffff;
			acc.r = ((acc.r >> i) | (acc.l << j)) & 0xffffff;
			acc.l >>= i;
		} else if (i < 48) {
			i -= 24;
			j = 24 - i;
			accex.r = (acc.r << j) & 0xffffff;
			accex.l = ((acc.r >> i) | (acc.l << j)) & 0xffffff;
			acc.r = acc.l >> i;
			acc.l = 0;
		} else if (i < 72) {
			i -= 48;
			accex.r = ((acc.r >> i) | (acc.l << (24 - i))) & 0xffffff;
			accex.l = acc.l >> i;
			acc.l = acc.r = 0;
		} else if (i < 96) {
			accex.r = acc.l >> (i - 72);
			acc.l = acc.r = 0;
		} else
			acc.l = acc.r = 0;
	} else {
		if (i > -24) {
			i = -i;
			j = 24 - i;
			accex.r = acc.l >> j;
			acc.l = ((acc.l << i) | (acc.r >> j)) & 0xffffff;
			acc.r = (acc.r << i) & 0xffffff;
		} else if (i > -48) {
			i = -i - 24;
			j = 24 - i;
			accex.l = acc.l >> j;
			accex.r = ((acc.l << i) | (acc.r >> j)) & 0xffffff;
			acc.l = (acc.r << i) & 0xffffff;
			acc.r = 0;
		} else if (i > -72) {
			i = -i - 48;
			j = 24 - i;
			accex.l = ((acc.l << i) | (acc.r >> j)) & 0xffffff;
			accex.r = (acc.r << i) & 0xffffff;
			acc.l = acc.r = 0;
		} else if (i > -96) {
			accex.l = (acc.r << i) & 0xffffff;
			acc.l = acc.r = 0;
		} else
			acc.l = acc.r = 0;
	}

	return E_SUCCESS;
}

int
yta()
{
	if (G_LOG) {
		acc = accex;
		return E_SUCCESS;
	}

	acc.mr = accex.mr;
	acc.l = (accex.l & 0xffff) |
		((acc.l + (enreg.o << 17) - (64 << 17)) & 0x1fe0000);
	if (acc.l & 0x1000000) {
		acc.l &= 0xffffff;
		return E_OVFL;
	}
	return E_SUCCESS;
}

/*
 * Fetch BESM "real" value and return it as native double.
 */
double
fetch_real (int addr)
{
	alureg_t word;
	LOAD(word, addr);
        return get_real(word);
}
