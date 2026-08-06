/*************************************************************************/
/*****************           ATC CONFIDENTIAL            *****************/
/*****************                                       *****************/
/*****************   Description : AC83xx trustzone      *****************/
/*****************                                       *****************/
/*****************                                       *****************/
/*****************       Company : Aucochips Inc.        *****************/
/*****************       Programmer : Emily Zhang        *****************/
/*************************************************************************/

#include "mmu.h"


/**********************************************************************

Memory mapping of ARM2

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
		

void CreatePageTable(unsigned int* pagetable)
{
    unsigned int i;//, ca, pa;
    
    
    // Note
    // 1. Unused entries must be reserved.
    // 2. All accessible regions are set to domain 0
    
    memset( pagetable, 0, sizeof(unsigned int) * 4096 );
	  
    // Map address 0x00000000 to 0x01000000   256M
    for(i = 0; i < 640; i++)
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
    // Map the non-cachable & non-bufferable memory space 0xF000 0000 -- F03FFFFF  4M IO
	
    for(i=0xf00; i<0xf04; i++)  //0x700 - 0x704
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

  // Map the non-cachable & non-bufferable memory space 0xF0000000 -- F03FFFFF  4M IO   -> 0xFD000000
	for(i=0XFD0; i<0XFD4; i++)
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


  // Map the non-cachable & non-bufferable memory space 0xF100 0000 -- F110 0000  1M  GIC   -> 0xFE000000
	for(i=0XFE0; i<0XFE1; i++)
    {
        pagetable[i] = SECTION_DESC(
				(0XF10 + i - 0XFE0) << 20, 
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

	

   // return;
}
