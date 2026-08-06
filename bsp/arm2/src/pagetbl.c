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


#include "mmu.h"
#include "mem_operation.h"
#include "generated/atc_project.h"

/**********************************************************************

Memory mapping of MT33XX ARM2

00000000 - 00100000   -   -          0M -      1M        1M    DRAM-B not allowed to access
00100000 - 3FFFFFFF     C   B         1M - 1024M   1023M    DRAM-B
40000000 - 403FFFFF     -   -      1024M - 1028M      4M    - (IO)
40400000 - 4FFFFFFF     -   -      1028M - 1280M    252M    -
50000000 - 503FFFFF     -   -      1280M - 1284M      4M    - (IO Extension)
50400000 - 5FFFFFFF     -   -      1284M - 1536M    252M    -
60000000 - 60FFFFFF     -   -      1536M - 1552M     16M    PBI-A (Flash)
61000000 - 6FFFFFFF     -   -      1552M - 1792M    240M    -
70000000 - 703FFFFF     -   -      1792M - 1796M      4M    IO
70400000 - 7FFFFFFF     -   -      1796M - 2048M    252M    -
80000000 - 80FFFFFF     C   B      2048M - 2064M     16M    PBI-B (Flash)
81000000 - 8FFFFFFF     -   -      2064M - 2304M    240M    -
90000000 - BFFFFFFF     -   -      2304M - 3072M    768M    -
C0000000 - FFFFFFFF     -   -      3072M - 4096M   1024M    DRAM-A

**********************************************************************/

/*----------------------------------------------------------------------------
 * CreatePageTable() Create page table
 *  @param pagetable[in] - Address of page table, shall be aligned to 16K boundary
 *---------------------------------------------------------------------------*/
		
#if 0

void CreatePageTable(UINT32* pagetable)
{
    UINT32 i;//, ca, pa;

    
    // Note
    // 1. Unused entries must be reserved.
    // 2. All accessible regions are set to domain 0
    
    memset( pagetable, 0, sizeof(UINT32) * 4096 );
	  
    // Map address 0x00000000 to 0x01000000   16M
    for(i = 0; i < 16; i++)
    {
      pagetable[i] = SECTION_DESC(
  		i << 20, 
  		EXECUTABLE, 
  		APX_NO_LIMIT, 
  		AP_USR_NO_LIMIT, 
  		TEX_STD, 
  		CACHE_WRITEBACK, 
  		0, 
  		0, 
  		0, 
  		NONSEC, 
  		DOMAIN00);
    }
	
    // Map the non-cachable & non-bufferable memory space 0x70000000 -- 703FFFFF  4M IO

    for(i=1792; i<1796; i++)  //0x700 - 0x704
    {
        pagetable[i] = SECTION_DESC(
				i << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

  // Map the non-cachable & non-bufferable memory space 0x70000000 -- 703FFFFF  4M IO   -> 0xA0000000
	for(i=0XA00; i<0XD00; i++)
    {
        pagetable[i] = SECTION_DESC(
				(0X700 + 0XA00 - i) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

  // Map address 0x00000000 to 0x08000000   128M  -> 0xAC000000 - 0xB4000000
	for(i=0xAC0; i<0XB40; i++)
    {
        pagetable[i] = SECTION_DESC(
				(i-0XAC0 ) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }


	
    // Map the non-cachable & non-bufferable memory space 0x00000000 -- 0x08000000 128M -> 0xC0000000 -0xC8000000
     	
    for(i=3072; i<3200; i++) //C00 -> C80
    {
        pagetable[i] = SECTION_DESC(
				(i-3072) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

	
    // Map the non-cachable & non-bufferable memory space 0x07000000 -- 0x08000000----->0xB3000000
	 for(i=0XB30; i<0XB3A; i++)
    {
        pagetable[i] = SECTION_DESC(
				(0X70+i - 0XB30) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

	
    return;
}

#else

extern UINT32 g_u4MemorySize;

#if defined(CONFIG_ATC_PLATFORM_ac83xx)
void CreatePageTable(UINT32* pagetable, UINT32 dramsize)
{
    UINT32 i;//, ca, pa;
    
    dramsize = dramsize >> 20;

    
    // Note
    // 1. Unused entries must be reserved.
    // 2. All accessible regions are set to domain 0
    
    memset( pagetable, 0, sizeof(UINT32) * 4096 );
	  
    // Map address 0x00000000 to 0x01000000   16M
    for(i = 0; i < 16; i++)
    {
      pagetable[i] = SECTION_DESC(
  		i << 20, 
  		EXECUTABLE, 
  		APX_NO_LIMIT, 
  		AP_USR_NO_LIMIT, 
  		TEX_STD, 
  		CACHE_WRITEBACK, 
  		0, 
  		0, 
  		0, 
  		NONSEC, 
  		DOMAIN00);
    }
	
    // Map the non-cachable & non-bufferable memory space 0x70000000 -- 703FFFFF  4M IO

    for(i=0xF00; i<0xF04; i++)  //0x700 - 0x704
    {
        pagetable[i] = SECTION_DESC(
				i << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

  // Map the non-cachable & non-bufferable memory space 0x70000000 -- 703FFFFF  16M IO   -> 0xA0000000
	///////////////for(i=0XA00; i<0XA10; i++)
	for(i=0XFD0; i<0XFE0; i++)
    {
        pagetable[i] = SECTION_DESC(
				(0XF00 + i - 0XFD0) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

  // Map address 0x00000000 to 0x08000000   128M  -> 0xAC000000 - 0xB4000000   OR
  // Map address 0x00000000 to 0x10000000   256M  -> 0xAC000000 - 0xBC000000
	//for(i=0xAC0; i<0XBC0; i++)
	
	for(i=0x700; i<0X700 + dramsize; i++)
    {
        pagetable[i] = SECTION_DESC(
				(i-0X700 ) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }


#if 0	
    // Map the non-cachable & non-bufferable memory space 0x00000000 -- 0x10000000 256M -> 0xC0000000 -0xD0000000
    for(i=0xC00; i<0xF00; i++) //C00 -> C80
    {
        pagetable[i] = SECTION_DESC(
				(i-0xC00) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }
#endif
    return;
}
#elif defined(CONFIG_ATC_PLATFORM_ac823x)
void CreatePageTable(UINT32* pagetable, UINT32 dramsize)
{
    UINT32 i;//, ca, pa;
    
    dramsize = dramsize >> 20;
    
    // Note
    // 1. Unused entries must be reserved.
    // 2. All accessible regions are set to domain 0
    
    memset( pagetable, 0, sizeof(UINT32) * 4096 );
	  
    // Map address 0x00000000 to 0x01000000   16M
    for(i = 0; i < 0x4; i++)
    {
      pagetable[i] = SECTION_DESC(
  		i << 20, 
  		EXECUTABLE, 
  		APX_NO_LIMIT, 
  		AP_USR_NO_LIMIT, 
  		TEX_STD, 
  		CACHE_WRITEBACK, 
  		0, 
  		0, 
  		0, 
  		NONSEC, 
  		DOMAIN00);
    }
    for(i = 0x4; i < 0x68; i++)
    {
      pagetable[i] = SECTION_DESC(
  		i << 20, 
  		EXECUTABLE, 
  		APX_NO_LIMIT, 
  		AP_USR_NO_LIMIT, 
  		TEX_STD, 
		NOCACHE_ORDERED, 
  		0, 
  		0, 
  		0, 
  		NONSEC, 
  		DOMAIN00);
    }
    for(i = 0x68; i < 0x100; i++)
    {
      pagetable[i] = SECTION_DESC(
  		i << 20, 
  		EXECUTABLE, 
  		APX_NO_LIMIT, 
  		AP_USR_NO_LIMIT, 
  		TEX_STD, 
  		CACHE_WRITEBACK, 
  		0, 
  		0, 
  		0, 
  		NONSEC, 
  		DOMAIN00);
    }
    // Map the non-cachable & non-bufferable memory space 0x70000000 -- 703FFFFF  4M IO

    for(i=0x100; i<0x104; i++)  //0x700 - 0x704
    {
        pagetable[i] = SECTION_DESC(
				i << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
				NOCACHE_ORDERED, 
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }

  // Map the non-cachable & non-bufferable memory space 0x70000000 -- 703FFFFF  16M IO   -> 0xA0000000
	///////////////for(i=0XA00; i<0XA10; i++)
    for(i=0X104; i<0X400; i++)
    {
        pagetable[i] = SECTION_DESC(
				(i) << 20, 
				EXECUTABLE, 
				APX_NO_LIMIT, 
				AP_USR_NO_LIMIT, 
				TEX_STD, 
  				CACHE_WRITEBACK,
				0, 
				0, 
				0, 
				NONSEC, 
				DOMAIN00);
    }
    return;
}
#else
# error "Must define platform macro"
#endif

#endif

