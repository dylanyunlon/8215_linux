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

#ifndef __BL_MMU_H_
#define __BL_MMU_H_

typedef enum
{
    FAULT = 0,
		COARSE_PAGE = 1,
		SECTION = 2,
		FINE_PAGE = 3
} FIRST_LEVEL_DESC;

//ARM11, new setting for MMU, TRM6.5
//The access permission bits control access to the corresponding memory region. If an
//access is made to an area of memory without the required permissions, then a
//permission fault is raised.


typedef enum
{
    APX_NO_LIMIT = 0,
		APX_NO_WRITE = 1
}APX_ATTRIBUTE;

typedef enum
{
    AP_ALL_NO_ACCESS = 0,
		AP_USR_NO_ACCESS = 1,
		AP_USR_NO_WRITE = 2,
		AP_USR_NO_LIMIT = 3,
}AP_ACCESS;

//A domain is a collection of memory regions. In compliance with the ARM Architecture
//and the TrustZone Security Extensions, the ARM1176JZ-S supports 16 Domains in the
//Secure world and 16 Domains in the Non-secure world.

typedef enum
{
    DOMAIN00 = 0,
		DOMAIN01 = 1,
		DOMAIN02 = 2,
		DOMAIN03 = 3,
		DOMAIN04 = 4,
		DOMAIN05 = 5,
		DOMAIN06 = 6,
		DOMAIN07 = 7,
		DOMAIN08 = 8,
		DOMAIN09 = 9,
		DOMAIN10 = 10,
		DOMAIN11 = 11,
		DOMAIN12 = 12,
		DOMAIN13 = 13,
		DOMAIN14 = 14,
		DOMAIN15 = 15
}AP_DOMAIN;

//CLIENT : Accesses are checked against the access permission bits in the section or page descriptor.
//MANAGER : Accesses are not checked against the access permission bits so a permission fault cannot be generated.
typedef enum
{
    DOMAIN_NO_ACCESS = 0,
		DOMAIN_CLIENT = 1,
		DOMAIN_RESERVED = 2,
		DOMAIN_MANAGER = 3
}AP_DOMAIN_ACCESS;

typedef enum
{
    TEX_STD = 0,
		TEX_SHARE_CTRL = 1,
		TEX_NO_SHARE = 2,
}TEX_ATTRIBUTE;

//These bits (C and B) indicate if the area of memory mapped by this section is
//treated as write-back cachable, write-through cachable, noncached buffered, or
//noncached nonbuffered
typedef enum
{
    NOCACHE_ORDERED = 0,
    NOCACHE_BUFFERABLE = 1,
    CACHE_WRITETHROUGH = 2,
    CACHE_WRITEBACK
}CACHE_ATTRIBUTE;


typedef enum
{
    EXECUTABLE = 0,
    NOT_EXECUTABLE = 1
}AP_EXE_ATTRIBUTE;

typedef enum
{
    SECURITY = 0,
    NONSEC = 1
}TMMU_SEC;


#define SECTION_DESC(Addr, XN, APX, AP, TEX, CB, nG, S,  P, NS, Domain)     \
                (((Addr)&0xFFF00000) | ((NS)<<19) | ((nG)<<17) | ((S)<<16)  \
                | ((APX)<<15) | ((TEX)<<12) | ((AP)<<10) | ((P)<<9)         \
                | ((Domain)<<5) | ((XN)<<4) | ((CB)<<2) | SECTION)


void Flush_Cache(unsigned int u4Start, unsigned int u4Len);

void Flush_Invalid_Cache(unsigned int u4Start, unsigned int u4Len);
#endif


