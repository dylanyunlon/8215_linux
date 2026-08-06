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

//============================================================================
// Include files
//============================================================================

#include "targetConfig.h"
#include "preloader_common.h"
#include "chip_test.h"


extern void BurstR(UINT32 u4Addr, UINT32 *pBuf, UINT32 u4Cnt);
extern void BurstW(UINT32 u4Addr, UINT32 *pBuf, UINT32 u4Cnt);
extern UINT32 ReadARMCycleCounter(void);



#define HALREAD32(_reg_)           (*((volatile UINT32*)(_reg_)))
#define HAL_WRITE32(_reg_, _val_)   (*((volatile UINT32*)(_reg_)) = (_val_))

// *********************************************************************
// Function    : void Main(UINT32 u4HeapStart, UINT32 u4HeapSize)
// Description : User Main entry point
// Parameter   : u4HeapStart, u4HeapSize
// Return      : None
// *********************************************************************




void Area_BurstRW(UINT32 AreaAddr,UINT32 AreaSize,UINT32 GoldenAddr,UINT32 BitMaskAddr,UINT32 *u4Buf1,UINT32 *u4Buf2,UINT32 *u4Buf3)
{
    UINT32 i, j, u4PtnCnt, u4BufSize;
    UINT32 u4Loop, u4BaseAddr, u4Size;
    UINT32 randomStart,randomLength,randomtestn;

	UINT32 *p1,*p2,*p3,*p4;

	
	
	u4PtnCnt = sizeof(rand_pattern) / sizeof(rand_pattern[0]);
	u4BufSize = rand() % MAX_BUF_SIZE;
	j = rand() % u4PtnCnt;
	if (u4BufSize == 0)
	{
		u4BufSize = 1;
	}


	for (i = 0; i < u4BufSize; i++, j++)
	{
		j = j % u4PtnCnt;
		u4Buf1[i] = rand_pattern[j];
	}

	
	u4BaseAddr = AreaAddr;
	u4Size = AreaSize;
	u4Loop = u4Size / (u4BufSize << 2);
	if(GoldenAddr == 0)
	{
		p1 = u4Buf1;
		
	}
	else
	{
		if(BitMaskAddr != 0)
		{
			p3 = (UINT32 *)BitMaskAddr;
			p4 = (UINT32 *)GoldenAddr;
			p1 = u4Buf3;
		}
		else
		{
			p1 = (UINT32 *)GoldenAddr;
		}
	}

	
	p2 = u4Buf2;
	
	Printf("*****************start burst rw test from 0x%x,length 0x%x, golden 0x%x***************\n",AreaAddr,AreaSize,p1);	
	
	for (i = 0; i < u4Loop; i++)
	{
		if(GoldenAddr == 0)
		{
			
			BurstW(u4BaseAddr + i * u4BufSize * 4, p1, u4BufSize);
		}
		else
		{
			if(BitMaskAddr != 0)
			{
				for (j = 0; j < u4BufSize; j++)
				{
					p1[j] = (u4Buf1[j] & p3[j])| (p4[j]&(~p3[j]));
				}
				BurstW(u4BaseAddr + i * u4BufSize * 4, p1, u4BufSize);
			}
		}
		
		BurstR(u4BaseAddr + i * u4BufSize * 4, p2, u4BufSize);

		
	
		for (j = 0; j < u4BufSize; j++)
		{
			if (p1[j] != p2[j])
			{
				Printf("************** Burst R/W failed at 0x%x (G:0x%x != V:0x%x)**************\n",u4BaseAddr + i * u4BufSize * 4+(j<<2),p1[j],p2[j]);
				while(1);
			}
		}
	
		Printf("[Burst R/W] addr=0x%08X, size=0x%X Bytes\n", u4BaseAddr + i * u4BufSize * 4, u4BufSize*4);
		if(GoldenAddr > 0)
		{
			if(BitMaskAddr != 0)
			{
				p3 += u4BufSize;
				p4 += u4BufSize;
			}
			else
			{
				p1 += u4BufSize;
			}
		}
	}
	
	u4Loop = (u4Size >> 2) % u4BufSize;
	if(GoldenAddr == 0)
	{
		p1 = u4Buf1;
	}

	if (u4Loop != 0)
	{
		if(GoldenAddr == 0)
		{
			BurstW(u4BaseAddr + i * u4BufSize * 4, p1, u4Loop);
		}
		else
		{
			if(BitMaskAddr != 0)
			{
				for (j = 0; j < u4Loop; j++)
				{
					p1[j] = (u4Buf1[j] & p3[j])| (p4[j]&(~p3[j]));
				}
				BurstW(u4BaseAddr + i * u4BufSize * 4, p1, u4Loop);
			}
		}

		BurstR(u4BaseAddr + i * u4BufSize * 4, p2, u4Loop);
	
		for (j = 0; j < u4Loop; j++)
		{
			if (p1[j] != p2[j])
			{
				Printf("************** Burst R/W failed at 0x%x (G:0x%x != V:0x%x)**************\n",u4BaseAddr + i * u4BufSize * 4+(j<<2),p1[j],p2[j]);
				while(1);
			}
		}
	
		Printf("[Burst R/W] addr=0x%08X, size=0x%X Bytes\n", u4BaseAddr + i * u4BufSize * 4, u4Loop*4);
		if(GoldenAddr > 0)
		{
			p1 += u4Loop;
		}
	}

	Printf("[Burst random test .......]\n");
	randomtestn = 50;

	while(randomtestn --)
	{
		randomLength = rand()%u4BufSize;
		randomStart = rand()%(u4Size >> 2);

		if(((u4Size>>2)-randomStart) > randomLength)
		{
			
		}
		else
		{
			randomLength = ((u4Size>>2)-randomStart);
		}
		if(GoldenAddr == 0)
		{
			BurstW(u4BaseAddr + (randomStart <<2), p1, randomLength);
		}
		else
		{
			if(BitMaskAddr != 0)
			{
				p3 = (UINT32 *)(BitMaskAddr+(randomStart <<2));
				p4 = (UINT32 *)(GoldenAddr+(randomStart <<2));
				
				for (j = 0; j < randomLength; j++)
				{
					p1[j] = (u4Buf1[j] & p3[j])| (p4[j]&(~p3[j]));
				}
				BurstW(u4BaseAddr + (randomStart <<2), p1, randomLength);
			}
			else
			{
				p1 = (UINT32 *)(GoldenAddr+(randomStart <<2));
			}
		}
		BurstR(u4BaseAddr + (randomStart <<2), p2, randomLength);
	
		for (j = 0; j < randomLength; j++)
		{
			if (p1[j] != p2[j])
			{
				Printf("************** Random Burst R/W failed at 0x%x (G:0x%x != V:0x%x)**************\n",u4BaseAddr + (randomStart <<2)+(j<<2),p1[j],p2[j]);
				while(1);
			}
		}
		Printf("Random Burst R/W start at 0x%x,length=0x%x**************\n",u4BaseAddr + (randomStart <<2),randomLength);
	}

	
	

}





void Burst_RW_Test(UINT32 cpu)
{
    //Set to transparent mode  
	UINT32 u4Seed;
	volatile UINT32 u4Val;
	UINT32 loop;
    ConfigPerformanceMonitorControlReg(0x405);



    Printf("User Main Main_BurstRW...\n");

    u4Seed = ReadARMCycleCounter();
    u4Seed ^= ReadARMCycleCounter();
    srand(u4Seed);
    Printf("Seed = 0x%08X\n", u4Seed);

    // GPR enalbe
    u4Val = HALREAD32(0xF000804C);
    u4Val |= 0x00000002;
    HAL_WRITE32(0xF000804C, u4Val);

    loop = TEST_LOOP;

    while (loop--)
    {
		//Area_BurstRW(MCUSYS_BASE_ADDR,MCUSYS_SIZE,MCUSYS_GOLDEN_ADDR,MCUSYS_BITMASK_ADDR);
		if(cpu==0)
		{
			Area_BurstRW(CPU0_TEST_SRAM_BASE_ADDR,CPU0_TEST_SRAM_SIZE,0,0,testBuf1,testBuf2,testBuf3);
			Area_BurstRW(CPU0_TEST_ROM_BASE_ADDR,CPU0_TEST_ROM_SIZE,CPU0_TEST_ROM_GOLDEN_ADDR,0,testBuf1,testBuf2,testBuf3);
			Area_BurstRW(CPU0_TEST_GPR_BASE_ADDR,CPU0_TEST_GPR_SIZE,0,0,testBuf1,testBuf2,testBuf3);
			Area_BurstRW(CPU0_TEST_DRAMB_BASE_ADDR,CPU0_TEST_DRAM_SIZE,0,0,testBuf1,testBuf2,testBuf3);
			Area_BurstRW(CPU0_TEST_FLASHA_BASE_ADDR,CPU0_TEST_FLASHA_SIZE,CPU0_TEST_FLASHA_GOLDEN_ADDR,0,testBuf1,testBuf2,testBuf3);
		
		}
		else if(cpu == 1)
		{
			Area_BurstRW(CPU1_TEST_SRAM_BASE_ADDR,CPU1_TEST_SRAM_SIZE,0,0,testBuf4,testBuf5,testBuf6);
			Area_BurstRW(CPU1_TEST_ROM_BASE_ADDR,CPU1_TEST_ROM_SIZE,CPU1_TEST_ROM_GOLDEN_ADDR,0,testBuf4,testBuf5,testBuf6);
			Area_BurstRW(CPU1_TEST_GPR_BASE_ADDR,CPU1_TEST_GPR_SIZE,0,0,testBuf4,testBuf5,testBuf6);
			Area_BurstRW(CPU1_TEST_DRAMB_BASE_ADDR,CPU1_TEST_DRAM_SIZE,0,0,testBuf4,testBuf5,testBuf6);
			Area_BurstRW(CPU1_TEST_FLASHA_BASE_ADDR,CPU1_TEST_FLASHA_SIZE,CPU1_TEST_FLASHA_GOLDEN_ADDR,0,testBuf4,testBuf5,testBuf6);
		}
		//

	
		//
		
        Printf("=============== Complete %d loop =============\n",loop); 
    }

}


DECLARE_TEST_ITEM("Burst_RW_TEST",Burst_RW_Test)



