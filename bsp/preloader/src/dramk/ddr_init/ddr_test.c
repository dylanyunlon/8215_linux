#include "ddr.h"
#include "ddr_includes.h"

#define MAX_TIME  0xE0000000

void DumpDramRegister()
{
    U16 ii;
    U32 u4addr, u4value;

    Printf("\n=================AC8317 PLL/PHY register value================================");
    for (ii=0; ii<=0x0f4; ii=ii+4)
    {
        if (ii%16==0)
        {
            Printf("\n0x%8x | ", mcSET_PHY_REG_ADDR(0x000)+ii);
        }
        u4addr = mcSET_PHY_REG_ADDR(0x000)+ii;
        u4value = ucDram_Register_Read(u4addr);
        Printf(" %08x", u4value);
    }

    for (ii=0x2a0; ii<=0x2e8; ii=ii+4)
    {
        if (ii%16==0)
        {
            Printf("\n0x%8x | ", mcSET_PHY_REG_ADDR(0x000)+ii);
        }
        u4addr = mcSET_PHY_REG_ADDR(0x000)+ii;
        u4value = ucDram_Register_Read(u4addr);
        Printf(" %08x", u4value);
    }


    Printf("\n=================AC8317 PLL Wrapper register value================================");
    for (ii=0xa40; ii<=0xbd0; ii=ii+4)
    {
        if ((ii-0xa40)%16==0)
        {
            Printf("\n0x%8x | ", mcSET_PHY_REG_ADDR(0x000)+ii);
        }
        u4addr = mcSET_PHY_REG_ADDR(0x000)+ii;
        u4value = ucDram_Register_Read(u4addr);
        Printf(" %08x", u4value);
    }

    Printf("\n=========AC8317 cha dramc register value===========================");
    for (ii=0; ii<0x400; ii=ii+4)
    {
        if (ii%16==0)
        {
            Printf("\n0x%8x | ", mcSET_DRAMC_REG_ADDR(ii));
        }
        u4addr = mcSET_DRAMC_REG_ADDR(ii);
        u4value = ucDram_Register_Read(u4addr);
        Printf(" %08x", u4value);
    }

    UNUSED(u4value);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:        DramBSelfTest

Description:     DramB Self Test

Arguments:      u4StartAddr      - [in]Start Address
u4Len               - [in] Length	

Return Value:   

-------------------------------------------------------------------*/
BOOL DramBSelfTest(U32 u4StartAddr, U32 u4Len)
{
	BOOL bRet = TRUE;
	U32 u4Tmp = 0,u4TestLoop = 0, u4TestAddr = 0, u4TestLen = DEFALUT_TESTB_LEN;

	/*1. Set Agent15 disable*/
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_CONF);
	//printf("Before DRAMB_REG_CONF 0x%x\n",u4Tmp);
	u4Tmp = u4Tmp & (~BIT_AG15_EN);
	//printf("After DRAMB_REG_CONF 0x%x\n",u4Tmp);
	DRAM_DMARB_WRITE32(DRAMB_REG_CONF, u4Tmp);   

	/*2. Set Test Start Address*/
	u4TestAddr = u4StartAddr|DEAFULT_TESTB_ADDR;
	//printf("DRAMB_REG_TEST0_0 Test Start 0x%x\n",u4TestAddr);
	DRAM_DMARB_WRITE32(DRAMB_REG_TEST0_0, u4TestAddr);

	/*3. Set Test Address Length*/
	if(u4Len > DEFALUT_TESTB_LEN)
	{
		u4TestLen = u4Len>>4;
	}
	//printf("DRAMB_REG_TEST0_1 Test Length 0x%x\n",u4TestLen);
	DRAM_DMARB_WRITE32(DRAMB_REG_TEST0_1, u4TestLen);

	/*4. Start Self Test*/
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_TEST);
	//printf("Before DRAMB_REG_TEST 0x%x\n",u4Tmp);
	u4Tmp = u4Tmp | (BIT_WAGENT_EN);
	//printf("After DRAMB_REG_TEST 0x%x\n",u4Tmp);
	DRAM_DMARB_WRITE32(DRAMB_REG_TEST, u4Tmp);

	/*5. Check Result*/
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_TEST0_RESULT);
	//printf("Result 0x%x 0x%x\n",u4Tmp, (u4Tmp&CMP_CPT0_MASK));
	mcDELAY_us(1);
	while(TRUE)
	{
		u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_TEST0_RESULT);
		//printf("Result 0x%x u4TestLoop %d\n", u4Tmp, u4TestLoop);
		mcDELAY_us(1000);
		if(((u4Tmp&CMP_CPT0_MASK) ==1)&&(((u4Tmp&(1<<DLE_CNT_OK_OFFSET))>>DLE_CNT_OK_OFFSET)== 1))
		{
			break;
		}
		else
		{
			u4TestLoop++;
			if(u4TestLoop > MAX_TIME)
			{
				printf("DRAMB Get Test Time out! \n");
				break;
			}

		}
	}

	mcDELAY_us(1000);

	//printf("Final Test Result 0x%x \n", u4Tmp);
	if(((((u4Tmp&(1<<CMP_ERR0_OFFSET))>>CMP_ERR0_OFFSET)== 0))&&(((u4Tmp&(1<<DLE_CNT_OK_OFFSET))>>DLE_CNT_OK_OFFSET)!= 0))
	{
		printf("Start Addr 0x%x and len 0x%x DRAMB Test AgentB is OK! \n",u4StartAddr,u4Len);
		bRet = TRUE;

	}
	else
	{
		printf("Final Test Result 555 0x%x \n", u4Tmp);
		printf("DRAMB_REG_TEST0_1 Test Length 0x%x\n",u4TestLen);
		printf("******Error Start Addr 0x%x and len 0x%x DRAMB Test AgentB is Failed! \n",u4StartAddr,u4Len);
		bRet = FALSE;
	}


	/*6. Set Agent15 enable*/
	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_CONF);
	//printf("Before DRAMB_REG_CONF 0x%x\n",u4Tmp);
	u4Tmp = u4Tmp |BIT_AG15_EN ;
	//printf("After DRAMB_REG_CONF 0x%x\n",u4Tmp);
	DRAM_DMARB_WRITE32(DRAMB_REG_CONF, u4Tmp);

	u4Tmp = DRAM_DMARB_READ32(DRAMB_REG_TEST);
	//printf("Before DRAMB_REG_TEST 0x%x\n",u4Tmp);
	u4Tmp = u4Tmp & (~BIT_WAGENT_EN);
	//printf("After DRAMB_REG_TEST 0x%x\n",u4Tmp);
	DRAM_DMARB_WRITE32(DRAMB_REG_TEST, u4Tmp);

	return bRet;
}



void Write2Mem(UINT32 u4StartMemAddr, UINT32 u4Len)
{
	UINT32 *pu4MemAddr = (UINT32 *)u4StartMemAddr;
	UINT32 u4tstPatten = 0;
        while(pu4MemAddr < (UINT32 *)(u4StartMemAddr + u4Len))
	{
		*pu4MemAddr = u4tstPatten++;
		pu4MemAddr++;
	}
	Printf("WriteMem :Last tstPatten 0x%x\n",(u4tstPatten - 1));
}

UINT32 selftest0(void)
{
	UINT32 data;
	UINT32 res;

	/* selftest0 */
	ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), 0xC0601F00);
	ucDram_Register_Write(mcSET_ARBITER_ADDR(0x100), 0x0);
	ucDram_Register_Write(mcSET_ARBITER_ADDR(0x104), 0x40000000);

	ucDram_Register_Write(mcSET_ARBITER_ADDR(0x118), 0x1400110D);
	data = ucDram_Register_Read(mcSET_ARBITER_ADDR(0x140));

	while ((data & 0x1) == 0) {
		mcDELAY_us(1000);
		data = ucDram_Register_Read(mcSET_ARBITER_ADDR(0x140));
	}

	if (data == 0x101)
		res = 1;
	else
		res = 0;

	ucDram_Register_Write(mcSET_ARBITER_ADDR(0x118), 0x0400110D);
	ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), 0xE0601F00);

	return res;
}

BOOL CmpWriteMem(UINT32 u4StartMemAddr, UINT32 u4Len)
{
	BOOL bRet = TRUE;
	UINT32 *pu4MemAddr = (UINT32 *)u4StartMemAddr;
	UINT32 u4tstPatten = 0, u4Tmp = 0, u4TryTimes = 0;

	Printf("CmpMem StartAddrress 0x%x ,Length 0x%x\n",u4StartMemAddr,u4Len );
	while(pu4MemAddr < (UINT32 *)(u4StartMemAddr + u4Len))
	{
		u4TryTimes = 0;
		u4tstPatten = *pu4MemAddr;
		if(u4Tmp != u4tstPatten)
		{
			Printf("*******Error****CmpMem Error  Pattern 0x%x ,OK Value u4Tmp 0x%x\n",u4tstPatten,u4Tmp );
			while(u4TryTimes < 20)
			{
				    u4tstPatten = *pu4MemAddr;
				    if(u4Tmp != u4tstPatten)
				    {
				    	  Printf("*******Try Error****CmpMem Try Times %d Error  Pattern 0x%x ,OK Value u4Tmp 0x%x\n",u4TryTimes,u4tstPatten,u4Tmp );  
				    }
				    else
				    {
				    	   Printf("*******Try OK****CmpMem Try Times %d   Pattern 0x%x ,OK Value u4Tmp 0x%x\n",u4TryTimes,u4tstPatten,u4Tmp );  
				    	   break;
				    }
				    u4TryTimes++;  
			}
			if(u4TryTimes == 20)
			{
				    *pu4MemAddr = u4Tmp;
				    Printf(" Try Read Fail,Try write Pattern 0x%x ,OK Value u4Tmp 0x%x\n",*pu4MemAddr,u4Tmp );  
			}
			bRet = FALSE;
		}
		pu4MemAddr++;
		u4Tmp++;
	}

	if(bRet == FALSE)
	{
           Printf("\n\n***********memory test failed****************\n\n");
           DumpDramRegister();
	}
	else
	{
 
           Printf("\n\n***********memory test pass****************\n\n");
	}
	return bRet;
}

BOOL DramBAllRgnTest()
{
	BOOL bRet = TRUE;
	U32 u4Loop = 0, u4Length = 0, u4TestSize = 0x4000000;
	TCMSET_CHANNELA_SIZE(0x40000000/0x100000);
	Printf("DramBAllRgnTest start %d\n",TCMGET_CHANNELA_SIZE());
	for(u4Loop=1;(u4Loop*u4TestSize)<(TCMGET_CHANNELA_SIZE()*1024*1024);u4Loop++)
	{
		u4Length =  (u4Loop*u4TestSize);
		Printf("Dramb Self Test Length 0x%x\n",u4Length);
		if(DramBSelfTest(0,u4Length) == FALSE)
		{
			Printf("DramBSelfTest Self Test Length 0x%x Failed!XXXXXXXXX!\n",u4Length);
			bRet = FALSE;
			break;
		}

		if(DramcEngine2(TE_OP_WRITE_READ_CHECK, 0xaa550000, 0x30000000, u4Length>>4) != 0)
		{
			Printf("XXXXXXXXXXXDramcEngine2 Self Test Length 0x%x Failed!XXXXXXXXX!\n",u4Length);
			bRet = FALSE;
			break;    
		}
		else
		{
			Printf("DramcEngine2 Self Test Length 0x%x 0xaa550000 OK !!\n",u4Length);
		}

		if(DramcEngine2(TE_OP_WRITE_READ_CHECK, 0x0000aa55, 0x30000000, u4Length>>4) != 0)
		{
			Printf("XXXXXXXXXXXDramcEngine2 Self Test Length 0x%x Failed!XXXXXXXXX!\n",u4Length);
			bRet = FALSE;
			break;    
		}
		else
		{
			printf("DramcEngine2 Self Test Length 0x%x 0x0000aa55 OK!!\n",u4Length);
		}

#if 0
		Write2Mem(0, u4Length);
		if(CmpWriteMem(0, u4Length) == FALSE)
		{
			bRet = FALSE;
			printf("CmpWriteMem Self Test Length 0x%x Failed!XXXXXXXXX!\n",u4Length);
		}
#endif
	}

	return bRet;
}
