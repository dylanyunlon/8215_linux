/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#include <linux/string.h>


#define ULL				unsigned long long
#define UINT64_C(c)		(c ## ULL)
#define DBL_MANT_DIG	53

typedef union {
	long long all;
	struct {
		unsigned low;
		int high;
	} s;
} dwords;

typedef union {
	unsigned long long all;
	struct {
		unsigned low;
		unsigned high;
	} s;
} udwords;

typedef union {
	udwords u;
	double	f;
} double_bits;

double __aeabi_ul2d(unsigned long long a)
{
	const unsigned N = sizeof(unsigned long long) * 8;
	double_bits fb;
	int sd = 0;
	int e = 0;

	if (a == 0) {
		return 0.0;
	}

	sd = N - __builtin_clzll(a);
	e = sd - 1;

	if (sd > DBL_MANT_DIG) {
		switch (sd) {
		case DBL_MANT_DIG + 1:
			a <<= 1;
			break;

		case DBL_MANT_DIG + 2:
			break;

		default:
			a = (a >> (sd - (DBL_MANT_DIG + 2))) |
			    ((a & ((unsigned long long)(-1) >> ((N + DBL_MANT_DIG + 2) - sd))) != 0);
			break;
		}

		a |= (a & 4U) != 0U;
		++a;
		a >>= 2;

		if (a & ((unsigned long long)1 << DBL_MANT_DIG)) {
			a >>= 1;
			++e;
		}
	} else {
		a <<= (DBL_MANT_DIG - sd);
	}

	fb.u.s.high = ((e + 1023) << 20) | ((unsigned long long)(a >> 32) & 0x000FFFFF);
	fb.u.s.low = (unsigned long long)a;

	return fb.f;
}

unsigned long long __aeabi_d2ulz(double a)
{
	int e = 0;
	udwords r;
	double_bits fb;

	fb.f = a;
	e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;

	if ((e < 0) || (fb.u.s.high & 0x80000000U)) {
		return 0;
	}

	r.s.high = (fb.u.s.high & 0x000FFFFFU) | 0x00100000U;
	r.s.low = fb.u.s.low;

	if (e > 52) {
		r.all <<= (e - 52);
	} else {
		r.all >>= (52 - e);
	}

	return r.all;
}


/*__aeabi_memcpy*/
/* Copy memory like memcpy, but no return value required.  Can't alias
   to memcpy because it's not defined in the same translation
   unit.  */
void __aeabi_memcpy(void *dest, const void *src, size_t n)
{
	memcpy(dest, src, n);
}

void __aeabi_memcpy4(void *dest, const void *src, size_t n)
{
	memcpy(dest, src, n);
}


/*__aeabi_memset*/
/* Set memory like memset, but different argument order and no return
   value required.	*/
void __aeabi_memset(void *dest, size_t n, int c)
{
	memset(dest, c, n);
}

/*__aeabi_memclr*/
/* Clear memory.  Can't alias to bzero because it's not defined in the
   same translation unit.  */
void __aeabi_memclr(void *dest, size_t n)
{
	__aeabi_memset(dest, n, 0);
}

void __aeabi_memclr4(void *dest, size_t n)
{
	__aeabi_memset(dest, n, 0);
}

/*__aeabi_memmove*/
/* Copy memory like memmove, but no return value required.	Can't
   alias to memmove because it's not defined in the same translation
   unit.  */
void __aeabi_memmove(void *dest, const void *src, size_t n)
{
	memmove(dest, src, n);
}

void __aeabi_memmove4(void *dest, const void *src, size_t n)
{
	memmove(dest, src, n);
}

