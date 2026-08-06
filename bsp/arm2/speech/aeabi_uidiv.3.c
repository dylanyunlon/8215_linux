/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#define ULL     unsigned long long
#define UINT64_C(c)     (c ## ULL)
#define DBL_MANT_DIG    53

typedef unsigned int size_t;

//typedef     int         si_int;  
//typedef     unsigned    su_int;  
//typedef     long long   di_int; 
//typedef unsigned long long du_int;  
typedef union 
{
    long long all;  
    struct 
    { 
        unsigned low;
        int high;  
    }s;  
}dwords; 

typedef union
{
    unsigned long long all;  
    struct 
    {  
        unsigned low;
        unsigned high;  
    }s;  
}udwords;

typedef union
{
    udwords u;
    double  f;
}double_bits;
    
#if 0
int __aeabi_idivmod(int a, int b)
{
    const int bits_in_word_m1 = (int)(sizeof(int) * 8) - 1;
    
    int s_a = a >> bits_in_word_m1;
    int s_b = b >> bits_in_word_m1;
    a = (a ^ s_a) - s_a;
    b = (b ^ s_b) - s_b;
    s_a ^= s_b;
    
    return (__aeabi_uidiv(a, b) ^ s_a) - s_a;
}
#endif
int  __aeabi_ulcmp(unsigned long long a, unsigned long long b)
{
    udwords x;
    udwords y;
    
    x.all = a;
    y.all = b;
    if (x.s.high < y.s.high)
    {
        return 0;
    }
    if (x.s.high > y.s.high)
    {
        return 2;
    }
    if (x.s.low < y.s.low)
    {
        return 0;
    }
    if (x.s.low > y.s.low)
    {
        return 2;
    }

    return 1;   
}
/*
double __aeabi_ul2d(unsigned long long a)
{    
    const unsigned N = sizeof(unsigned long long) * 8;
    double_bits fb;
    
    if (a == 0)  
    {
        return 0.0;  
    } 
        
    int sd = N - __builtin_clzll(a);
    int e = sd - 1;
    
    if (sd > DBL_MANT_DIG)  
    {  
        switch (sd)  
        {  
        case DBL_MANT_DIG + 1:  
            a <<= 1;  
            break;  
    
        case DBL_MANT_DIG + 2:  
            break;  
    
        default:  
            a = (a >> (sd - (DBL_MANT_DIG + 2)))| 
                ((a & ((unsigned long long)(-1) >> ((N + DBL_MANT_DIG + 2) - sd))) != 0);         
        }
    
        a |= (a & 4) != 0;
        ++a;
        a >>= 2;
        if (a & ((unsigned long long)1 << DBL_MANT_DIG)) 
        {  
            a >>= 1;  
            ++e;  
        }  
    }  
    else 
    {  
        a <<= (DBL_MANT_DIG - sd);  
    }
     
    fb.u.s.high = ((e + 1023) << 20) | ((unsigned long long)(a >> 32) & 0x000FFFFF);
    fb.u.s.low = (unsigned long long)a;
  
    return fb.f;  
}
*/
unsigned long long __aeabi_d2ulz(double a)
{ 
    udwords r;
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.s.high & 0x80000000))
    {
        return 0; 
    }
    r.s.high = (fb.u.s.high & 0x000FFFFF) | 0x00100000;
    r.s.low = fb.u.s.low;
    if (e > 52)
    {
        r.all <<= (e - 52); 
    }
    else 
    {
        r.all >>= (52 - e);  
    }
    
    return r.all;
}

long long __aeabi_lasr(long long a, int b)
{   
    const int bits_in_word = (int)(sizeof(int) * 8);     
    dwords input;    
    dwords result;
    
    input.all = a;    
    if (b & bits_in_word)
    {    
        result.s.high = input.s.high >> (bits_in_word - 1);     
        result.s.low = input.s.high >> (b - bits_in_word);      
    }      
    else    
    {      
        if (b == 0)
        {
            return a;
        }
        result.s.high  = input.s.high >> b;     
        result.s.low = (input.s.high << (bits_in_word - b)) | (input.s.low >> b);    
    }  
    
    return result.all; 
}


typedef unsigned char       bool;
typedef unsigned long long  rep_t;
#define significandBits     52
#define typeWidth           (sizeof(rep_t) * 8)  
#define exponentBits        (typeWidth - significandBits - 1)  
#define maxExponent         ((1 << exponentBits) - 1)  
#define exponentBias        (maxExponent >> 1)  
#define implicitBit         (UINT64_C(1) << significandBits)  
#define significandMask     (implicitBit - 1U)  
#define signBit             (UINT64_C(1) << (significandBits + exponentBits))  
#define absMask             (signBit - 1U)  
#define exponentMask        (absMask ^ significandMask)  
#define infRep              exponentMask  
#define quietBit            (implicitBit >> 1)  
#define qnanRep             (exponentMask | quietBit) 


static rep_t toRep(double x)
{  
    const union 
    { 
        double  f;
        rep_t   i;
    }rep = {.f = x};
        
    return rep.i;
}

static double fromRep(rep_t x)
{  
    const union
    {
        double  f;
        rep_t   i;
    }rep = {.i = x};  

    return rep.f;
} 

static int rep_clz(rep_t a)
{
    if(a & UINT64_C(0xffffffff00000000))
    {        
        return __builtin_clz(a >> 32);
    }
    else
    {        
        return 32 + __builtin_clz(a & UINT64_C(0xffffffff));
    }
}

static int normalize(rep_t *significand)
{  
    const int shift = rep_clz(*significand) - rep_clz(implicitBit);  
    *significand <<= shift;  

    return 1 - shift;
} 
/*
double __aeabi_dadd(double a, double b)
{
    rep_t aRep = toRep(a);
    rep_t bRep = toRep(b);
    const rep_t aAbs = aRep & absMask;
    const rep_t bAbs = bRep & absMask;
    
    if ((aAbs - 1U >= infRep - 1U) || (bAbs - 1U >= infRep - 1U))
    { 
        if (aAbs > infRep)
        {
            return fromRep(toRep(a) | quietBit);
        }
        
        if (bAbs > infRep) 
        {
            return fromRep(toRep(b) | quietBit);  
        }
        
        if (aAbs == infRep) 

        {   
            if ((toRep(a) ^ toRep(b)) == signBit) 
            {
                return fromRep(qnanRep);  
            } 
            else 
            {
                return a; 
            }
        } 
         
        if (bAbs == infRep) 
        {
            return b;  
        }
        
        if (!aAbs) 
        {   
            if (!bAbs) 
            {
                return fromRep(toRep(a) & toRep(b));  
            }
            else 
            {
                return b;  
            }  
        }
        
        if (!bAbs)
        {
            return a;
        }
    }  
    
    if (bAbs > aAbs)
    {  
        const rep_t temp = aRep;  
        aRep = bRep;  
        bRep = temp;  
    }  
    
    int aExponent = aRep >> significandBits & maxExponent;  
    int bExponent = bRep >> significandBits & maxExponent;  
    rep_t aSignificand = aRep & significandMask;  
    rep_t bSignificand = bRep & significandMask;  
    
    if (aExponent == 0) 
    {
        aExponent = normalize(&aSignificand);  
    }
    
    if (bExponent == 0) 
    {
        bExponent = normalize(&bSignificand);  
    }
   
    const rep_t resultSign = aRep & signBit;  
    const bool subtraction = (aRep ^ bRep) & signBit;  
    
    aSignificand = (aSignificand | implicitBit) << 3;  
    bSignificand = (bSignificand | implicitBit) << 3;  
   
    const int align = aExponent - bExponent; 
    if (align)
    {
        if (align < typeWidth)
        {  
            const bool sticky = bSignificand << (typeWidth - align);  
            bSignificand = bSignificand >> align | sticky;  
        }
        else
        {  
            bSignificand = 1;
        }  
    }
    
    if (subtraction)
    {
        aSignificand -= bSignificand;
        if (aSignificand == 0)
        {
            return fromRep(0);  
        }
        
        if (aSignificand < implicitBit << 3)
        {  
            const int shift = rep_clz(aSignificand) - rep_clz(implicitBit << 3);  
            aSignificand <<= shift;  
            aExponent -= shift;  
        }  
    }  
    else
    {  
        aSignificand += bSignificand;  
        
        if (aSignificand & implicitBit << 4)
        {  
            const bool sticky = aSignificand & 1;
            aSignificand = aSignificand >> 1 | sticky;
            aExponent += 1;
        }  
    } 
    
    if (aExponent >= maxExponent)
    {
        return fromRep(infRep | resultSign);  
    }
    
    if (aExponent <= 0) 
    {   
        const int shift = 1 - aExponent;  
        const bool sticky = aSignificand << (typeWidth - shift);  
        aSignificand = aSignificand >> shift | sticky;  
        aExponent = 0;  
    }  
    
    const int roundGuardSticky = aSignificand & 0x7;  
    rep_t result = aSignificand >> 3 & significandMask;  
    result |= (rep_t)aExponent << significandBits;  
    result |= resultSign;  
    
    if (roundGuardSticky > 0x4) 
    {
        result++;  
    }
    
    if (roundGuardSticky == 0x4)
    {
        result += result & 1;  
    }
    
    return fromRep(result); 
}
*/
void __aeabi_memcpy4(void *dest, const void *src, size_t n) 
{    
    memcpy(dest, src, n);
}

void __aeabi_memcpy(void *dest, const void *src, size_t n) 
{    
    memcpy(dest, src, n);
}

void * memmove(void * dest, const void *src, size_t count)
{
	char *tmp, *s;

	if (dest <= src) 
    {
		tmp = (char *)dest;
		s = (char *)src;
		while (count--)
        {      
			*tmp++ = *s++;
	    }
    }
	else 
    {
		tmp = (char *)dest + count;
		s = (char *)src + count;
		while (count--)
        {      
			*--tmp = *--s;
        }
    }
    
	return dest;
}

void __aeabi_memmove4(void *dest, const void *src, size_t n) 
{    
    memmove(dest, src, n);
}

void __aeabi_memmove(void *dest, const void *src, size_t n) 
{    
    memmove(dest, src, n);
}

void __aeabi_memclr(void *dest, size_t n) 
{    
    memset(dest, n, 0);
}

void __aeabi_memclr4(void *dest, size_t n) 
{    
    memset(dest, n, 0);
}


//strong_alias(__aeabi_memclr, __aeabi_memclr4)

