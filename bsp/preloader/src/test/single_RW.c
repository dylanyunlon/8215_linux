#include "targetConfig.h"
#include "preloader_common.h"
#include "chip_test.h"







void Area_SingleRW(UINT32 AreaAddr,UINT32 AreaSize,UINT32 GoldenAddr,UINT32 BitMaskAddr,UINT32 flag,UINT32 *u4Buf1,UINT32 *u4Buf2,UINT32 *u4Buf3)
{
	
	unsigned char *pU8,*pGU8;
	unsigned short *pU16,*pGU16;
	unsigned long *pU32,*pGU32;
	unsigned long seed = 10;
	unsigned long offset = 0;
	unsigned long u4PtnCnt;
	unsigned long tmpoffset;

	unsigned long *pGoldenAddr,*pBitMaskAddr;
	
	
	unsigned long i,j,u4BufSize,loop,u4BufSizeMark;
	

	pGoldenAddr = (unsigned long *)GoldenAddr;
	pBitMaskAddr = (unsigned long *)BitMaskAddr;
	
	u4PtnCnt = sizeof(rand_pattern) / sizeof(rand_pattern[0]);
	u4BufSize = MAX_BUF_SIZE;
	j = rand() % u4PtnCnt;
	if (u4BufSize == 0)
	{
		u4BufSize = 1;
	}

	pU8 = (unsigned char *)AreaAddr;
	pU16 = (unsigned short *)AreaAddr;
	pU32 = (unsigned long *)AreaAddr;


	for (i = 0; i < u4BufSize; i++, j++)
	{
		j = j % u4PtnCnt;
		u4Buf1[i] = rand_pattern[j];
	}
	
	if(pGoldenAddr == 0)
	{

		pGU8 = (unsigned char *)u4Buf1;
		pGU16 = (unsigned short *)u4Buf1;
		pGU32 = (unsigned long *)u4Buf1;
						
	}
	else
	{
		if(pBitMaskAddr != 0)
		{
			pGU8 = (unsigned char *)u4Buf3;
			pGU16 = (unsigned short *)u4Buf3;
			pGU32 = (unsigned long *)u4Buf3;
		}
		else
		{
			pGU8 = (unsigned char *)pGoldenAddr;
			pGU16 = (unsigned short *)pGoldenAddr;
			pGU32 = (unsigned long *)pGoldenAddr;
		}
	}
	
	

	
	Printf("*****************start single rw test from 0x%x,length 0x%x, golden 0x%x***************\n",AreaAddr,AreaSize,pGU8);	
	if(flag & SINGLE_RW_U8_TEST)
	{
		if(pGoldenAddr == 0)
		{
			u4BufSizeMark = (u4BufSize << 2) - 1;
			Printf("start u8 Continuous writing ....\n");		
			for(offset =0;offset < AreaSize;offset++)
			{


				pU8[offset] = pGU8[offset&u4BufSizeMark];
			}
		}
		else
		{
		
			if(pBitMaskAddr != 0)
			{
				u4BufSizeMark = (u4BufSize << 2) - 1;
				Printf("start u8 Continuous writing ....\n");
				for(offset =0;offset < AreaSize;offset++)
				{
					tmpoffset = (offset >> 2)&(u4BufSize-1);
					u4Buf3[tmpoffset] = (u4Buf1[tmpoffset] & pBitMaskAddr[offset >> 2])|(pGoldenAddr[offset >>2]&(~pBitMaskAddr[offset >>2]));
					pU8[offset] = pGU8[offset&u4BufSizeMark];
				}
			}
			else
			{
				u4BufSizeMark = AreaSize - 1;
			}
		}

	
		Printf("start u8 Continuous reading ....\n");
		for(offset =0;offset < AreaSize;offset++)
		{
			if(pBitMaskAddr != 0)
			{
				tmpoffset = (offset >>2)&(u4BufSize-1);
				u4Buf3[tmpoffset] = (u4Buf1[tmpoffset] & pBitMaskAddr[offset >>2])|(pGoldenAddr[offset >>2]&(~pBitMaskAddr[offset >>2]));	
			}
			if(pU8[offset]!=pGU8[offset&u4BufSizeMark])
			{
		  	Printf("[U8 read]diff at 0x%x (golden:%d,value:%d)\n",offset,pGU8[offset&u4BufSizeMark],pU8[offset]);
		  	break;
			}
		}
		if(offset == AreaSize)
		{
			Printf("[U8] Continuous RW  ok!\n");
		}
		else
		{
			Printf("[U8] Continuous RW  fail!\n");
			while(1);
		}
	}

	if(flag & SINGLE_RW_U16_TEST)
	{
		if(pGoldenAddr == 0)
		{
			u4BufSizeMark = (u4BufSize << 1) - 1;
			Printf("start u16 continuous writing ....\n");		
			for(offset =0;offset < (AreaSize>>1);offset++)
			{
				pU16[offset] = pGU16[offset&u4BufSizeMark];
			}
		}
		else
		{
			if(pBitMaskAddr != 0)
			{
				u4BufSizeMark = (u4BufSize << 1) - 1;
				Printf("start u16 continuous writing ....\n");
				for(offset =0;offset < (AreaSize>>1);offset++)
				{
					tmpoffset = (offset >> 1)&(u4BufSize-1);
					u4Buf3[tmpoffset] = (u4Buf1[tmpoffset] & pBitMaskAddr[offset >>1])|(pGoldenAddr[offset >>1]&(~pBitMaskAddr[offset >>1]));
					pU16[offset] = pGU16[offset&u4BufSizeMark];
				}
			}
			else
			{
				u4BufSizeMark = (AreaSize>>1) - 1;	
			}
		}
	

	
		Printf("start u16 continuous reading ....\n");
		for(offset =0;offset < (AreaSize>>1);offset++)
		{
			if(pBitMaskAddr != 0)
			{
				tmpoffset = (offset >>1)&(u4BufSize-1);
				u4Buf3[tmpoffset] = (u4Buf1[tmpoffset] & pBitMaskAddr[offset >>1])|(pGoldenAddr[offset >>1]&(~pBitMaskAddr[offset >>1]));	
			}
			if(pU16[offset]!=pGU16[offset&u4BufSizeMark])
			{
			  Printf("[U16 read]diff at 0x%x (golden:%d,value:%d)\n",offset,pGU8[offset&u4BufSizeMark],pU8[offset]);
			  break;
			}
		}

		if(offset == (AreaSize>>1))
		{
			Printf("[U16] Continuous RW  ok!\n");
		}
		else
		{
			Printf("[U16] Continuous RW  fail!\n");
			while(1);
		}

	}

	if(flag & SINGLE_RW_U32_TEST)
	{
		if(pGoldenAddr == 0)
		{
			u4BufSizeMark = u4BufSize - 1;
			Printf("start u32 continuous writing ....\n");		
			for(offset =0;offset < (AreaSize>>2);offset++)
			{
				pU32[offset] = pGU32[offset&u4BufSizeMark];
			}
		}
		else
		{
			if(pBitMaskAddr != 0)
			{
				u4BufSizeMark = (u4BufSize << 0) - 1;
				Printf("start u32 continuous writing ....\n");
				for(offset =0;offset < (AreaSize>>2);offset++)
				{
					tmpoffset = (offset >> 0)&(u4BufSize-1);
					u4Buf3[tmpoffset] = (u4Buf1[tmpoffset] & pBitMaskAddr[offset >>0])|(pGoldenAddr[offset >>0]&(~pBitMaskAddr[offset >>0]));
					pU32[offset] = pGU32[offset&u4BufSizeMark];
				}
			}
			else
			{
				u4BufSizeMark = (AreaSize>>2) - 1;
			}
		}
	
		Printf("start u32 continuous reading ....\n");
		for(offset =0;offset < (AreaSize>>2);offset++)
		{
			if(pBitMaskAddr != 0)
			{
				tmpoffset = (offset >>0)&(u4BufSize-1);
				u4Buf3[tmpoffset] = (u4Buf1[tmpoffset] & pBitMaskAddr[offset >>0])|(pGoldenAddr[offset >>0]&(~pBitMaskAddr[offset >>0]));	
			}
			if(pU32[offset]!=pGU32[offset&u4BufSizeMark])
			{
			  Printf("[U32 read]diff at 0x%x (golden:%d,value:%d)\n",pU32+offset,pGU32[offset&u4BufSizeMark],pU32[offset]);
			  break;
			}
		}

		if(offset == (AreaSize>>2))
		{
			Printf("[U32] Continuous RW  ok!\n");
		}
		else
		{
			Printf("[U32] Continuous RW  fail!\n");
			while(1);
		}
		
	}
	Printf("start random reading  ....\n");
		
		

	if(flag & SINGLE_RW_U8_TEST)
	{	
		Printf("start u8 random writing ....\n");		
		for(offset =0;offset < MAX_BUF_SIZE;offset++)
		{
			u4Buf1[offset] = rand()% AreaSize;
			//Printf("[U8] random write to 0x%x\n",u4Buf1[offset]);
			if(pGoldenAddr == 0)
			{
				pU8[u4Buf1[offset]] = (u4Buf1[offset]&0xFF);
			}
			else
			{
				if(pBitMaskAddr != 0)
				{
					
					u4Buf3[offset] = (u4Buf1[offset] & pBitMaskAddr[u4Buf1[offset] >>2])|(pGoldenAddr[u4Buf1[offset] >>2]&(~pBitMaskAddr[u4Buf1[offset] >>2]));
					pU8[u4Buf1[offset]] = (u4Buf3[offset]&0xFF);
				}
			}
		}
	
		Printf("start u8 random reading ....\n");
		for(offset =0;offset < MAX_BUF_SIZE;offset++)
		{
			if(pGoldenAddr == 0)
			{
				if(pU8[u4Buf1[offset]]!=(u4Buf1[offset]&0xFF))
				{
			  		Printf("[U8 read]diff at 0x%x (golden:%d,value:%d)\n",pU8+u4Buf1[offset],(u4Buf1[offset]&0xFF),pU8[u4Buf1[offset]]);
			  		break;
				}
			}
			else
			{
				if(pBitMaskAddr != 0)
				{
					if(pU8[u4Buf1[offset]]!=(u4Buf3[offset]&0xFF))
					{
			  			Printf("[U8 read]diff at 0x%x (golden:%d,value:%d)\n",pU8+u4Buf1[offset],(u4Buf3[offset]&0xFF),pU8[u4Buf1[offset]]);
			  			break;
					}
				}
				else
				{
					if(pU8[u4Buf1[offset]]!=pGU8[u4Buf1[offset]])
					{
			  			Printf("[U8 read]diff at 0x%x (golden:%d,value:%d)\n",pU8+u4Buf1[offset],pGU8[u4Buf1[offset]],pU8[u4Buf1[offset]]);
			  			break;
					}
				}
			}
		}
		if(offset == MAX_BUF_SIZE)
		{
			Printf("[U8] Rondom RW ok!\n");
		}
		else
		{
			Printf("[U8] Rondom RW  fail!\n");
			while(1);
		}
		
	}
	if(flag & SINGLE_RW_U16_TEST)
	{
		Printf("start u16 random writing ....\n");		
		for(offset =0;offset < MAX_BUF_SIZE;offset++)
		{
			u4Buf1[offset] = rand()% (AreaSize>>1);
			//Printf("[U16] random write to 0x%x\n",u4Buf1[offset]);
			if(pGoldenAddr == 0)
			{
				pU16[u4Buf1[offset]] = (u4Buf1[offset]&0xFFFF);
			}
			else
			{
				if(pBitMaskAddr != 0)
				{
					
					u4Buf3[offset] = (u4Buf1[offset] & pBitMaskAddr[u4Buf1[offset] >>1])|(pGoldenAddr[u4Buf1[offset] >>1]&(~pBitMaskAddr[u4Buf1[offset] >>1]));
					pU16[u4Buf1[offset]] = (u4Buf3[offset]&0xFFFF);
				}
			}
		}
		
		Printf("start u16 random reading ....\n");
		for(offset =0;offset < MAX_BUF_SIZE;offset++)
		{
			if(pGoldenAddr == 0)
			{
				if(pU16[u4Buf1[offset]]!=(u4Buf1[offset]&0xFFFF))
				{
			  		Printf("[U16 read]diff at 0x%x (golden:%d,value:%d)\n",pU16+u4Buf1[offset],(u4Buf1[offset]&0xFFFF),pU16[u4Buf1[offset]]);
			  		break;
				}
			}
			else
			{
				if(pBitMaskAddr != 0)
				{
					if(pU16[u4Buf1[offset]]!=(u4Buf3[offset]&0xFFFF))
					{
			  			Printf("[U16 read]diff at 0x%x (golden:%d,value:%d)\n",pU16+u4Buf1[offset],(u4Buf3[offset]&0xFFFF),pU16[u4Buf1[offset]]);
			  			break;
					}
				}
				else
				{
					if(pU16[u4Buf1[offset]]!=pGU16[u4Buf1[offset]])
					{
			  			Printf("[U16 read]diff at 0x%x (golden:%d,value:%d)\n",pU16+u4Buf1[offset],pGU16[u4Buf1[offset]],pU16[u4Buf1[offset]]);
			  			break;
					}
				}
			}
		}
		if(offset == MAX_BUF_SIZE)
		{
			Printf("[U16] Rondom RW ok!\n");
		}
		else
		{
			Printf("[U16] Rondom RW  fail!\n");
			while(1);
		}
	}
	if(flag & SINGLE_RW_U32_TEST)
	{

		Printf("start u32 random writing ....\n");		
		for(offset =0;offset < MAX_BUF_SIZE;offset++)
		{
			u4Buf1[offset] = rand()% (AreaSize>>2);
			//Printf("[U32] random write to 0x%x\n",u4Buf1[offset]);
			if(pGoldenAddr == 0)
			{
				pU32[u4Buf1[offset]] = u4Buf1[offset];
			}
			else
			{
				if(pBitMaskAddr != 0)
				{
					
					u4Buf3[offset] = (u4Buf1[offset] & pBitMaskAddr[u4Buf1[offset] >>0])|(pGoldenAddr[u4Buf1[offset] >>0]&(~pBitMaskAddr[u4Buf1[offset] >>0]));
					pU32[u4Buf1[offset]] = (u4Buf3[offset]&0xFFFFFFFF);
				}
			}
		}
		
		Printf("start u32 random reading ....\n");
		for(offset =0;offset < MAX_BUF_SIZE;offset++)
		{
			if(pGoldenAddr == 0)
			{
				if(pU32[u4Buf1[offset]]!=u4Buf1[offset])
				{
			  		Printf("[U32 read]diff at 0x%x (golden:%d,value:%d)\n",pU32+u4Buf1[offset],u4Buf1[offset],pU32[u4Buf1[offset]]);
			  		break;
				}
			}
			else
			{
				if(pBitMaskAddr != 0)
				{
					if(pU32[u4Buf1[offset]]!=(u4Buf3[offset]&0xFFFFFFFF))
					{
			  			Printf("[U32 read]diff at 0x%x (golden:%d,value:%d)\n",pU32+u4Buf1[offset],(u4Buf3[offset]&0xFFFFFFFF),pU32[u4Buf1[offset]]);
			  			break;
					}
				}
				else
				{
					if(pU32[u4Buf1[offset]]!=pGU32[u4Buf1[offset]])
					{
			  			Printf("[U32 read]diff at 0x%x (golden:%d,value:%d)\n",pU32+u4Buf1[offset],pGU32[u4Buf1[offset]],pU32[u4Buf1[offset]]);
			  			break;
					}
				}
			}
		}
		if(offset == MAX_BUF_SIZE)
		{
			Printf("[U32] Rondom RW ok!\n");
		}		
		else
		{
			Printf("[U32] Rondom RW  fail!\n");
			while(1);
		}
	}
}


void Single_RW_Test(UINT32 cpu)
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


    loop = TEST_LOOP;

    while (loop--)
    {
		//Area_SingleRW(MCUSYS_BASE_ADDR,MCUSYS_SIZE,MCUSYS_GOLDEN_ADDR,MCUSYS_BITMASK_ADDR,SINGLE_RW_U32_TEST);
		if(cpu==0)
		{
			Area_SingleRW(CPU0_TEST_SRAM_BASE_ADDR,CPU0_TEST_SRAM_SIZE,0,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf1,testBuf2,testBuf3);
			Area_SingleRW(CPU0_TEST_ROM_BASE_ADDR,CPU0_TEST_ROM_SIZE,CPU0_TEST_ROM_GOLDEN_ADDR,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf1,testBuf2,testBuf3);
			Area_SingleRW(CPU0_TEST_GPR_BASE_ADDR,CPU0_TEST_GPR_SIZE,0,0,SINGLE_RW_U32_TEST,testBuf1,testBuf2,testBuf3);
			Area_SingleRW(CPU0_TEST_FLASHA_BASE_ADDR,CPU0_TEST_FLASHA_SIZE,CPU0_TEST_FLASHA_GOLDEN_ADDR,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf1,testBuf2,testBuf3);
			Area_SingleRW(CPU0_TEST_DRAMB_BASE_ADDR,CPU0_TEST_DRAM_SIZE,0,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf1,testBuf2,testBuf3);
		}
		else if(cpu == 1)
		{
			Area_SingleRW(CPU1_TEST_SRAM_BASE_ADDR,CPU1_TEST_SRAM_SIZE,0,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf4,testBuf5,testBuf6);
			Area_SingleRW(CPU1_TEST_ROM_BASE_ADDR,CPU1_TEST_ROM_SIZE,CPU1_TEST_ROM_GOLDEN_ADDR,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf4,testBuf5,testBuf6);
			Area_SingleRW(CPU1_TEST_GPR_BASE_ADDR,CPU1_TEST_GPR_SIZE,0,0,SINGLE_RW_U32_TEST,testBuf4,testBuf5,testBuf6);
			Area_SingleRW(CPU1_TEST_FLASHA_BASE_ADDR,CPU1_TEST_FLASHA_SIZE,CPU1_TEST_FLASHA_GOLDEN_ADDR,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf4,testBuf5,testBuf6);
			Area_SingleRW(CPU1_TEST_DRAMB_BASE_ADDR,CPU1_TEST_DRAM_SIZE,0,0,SINGLE_RW_U32_TEST|SINGLE_RW_U16_TEST|SINGLE_RW_U8_TEST,testBuf4,testBuf5,testBuf6);
		}


        Printf("=============== Complete %d loop =============\n",loop); 
    }

}



//DECLARE_TEST_ITEM("SINGLE_RW_TEST",Single_RW_Test)



