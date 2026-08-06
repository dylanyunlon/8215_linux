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

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include "ddr_includes.h"

#ifdef CC_MTK_PRELOADER
#if 0
static void _DdrSetDramRefresh(void)
{
    UINT32 u4Clk = TCMGET_DDR_CLK();
    UINT32 u4MemClk;
    UINT32 u4RefreshCount;

    u4MemClk = u4Clk/2;

    // 78: tREFI = 7.8 us. 39: tREFI = 3.9 us.
    // Set 50: tREFI = 5.0 us.
    u4RefreshCount = ((u4MemClk / 1000000) * 50) / 160;       

    // Full frequency mode when DDRI, SDR and frequency < 400Mhz.
    u4RefreshCount = u4RefreshCount / 2;

    u4RefreshCount &= 0xFF;
    
    // Set REFCNT by tREFI and enable ADVREFEN.
    //DRAM_WRITE32(0x08, 0x200 |u4RefreshCount);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(0x008),0x200 |u4RefreshCount);
}
//#endif

UINT32 _DDR_CheckSize(BOOL fgChBActive)
{  
    register UINT32 u4DramSize = 0x4000000;   // 64 Mbytes.
    register UINT32 u4CheckAddr1;
    register UINT32 u4CheckAddr2;  
    register UINT32 u4BaseAddr;      
    register UINT32 u4Val1;
    register UINT32 u4MaxAddr = 0x40000000;  // 1 Gbytes.

	BOOL fgSizingDone = FALSE;

    if (fgChBActive)
    {
        u4BaseAddr = INIT_DRAM_B_CHB_BASE;
    }
    else
    {
        u4BaseAddr = INIT_DRAM_B_BASE;
    }

    while ((!fgSizingDone) && (u4DramSize < u4MaxAddr))
    {
        u4CheckAddr1 = u4BaseAddr + (u4DramSize*1) - 4;
        *((volatile UINT32 *)u4CheckAddr1) = 0x12345678;
        u4CheckAddr2 = u4BaseAddr + (u4DramSize*2) - 4;
        *((volatile UINT32 *)u4CheckAddr2) = 0x55AACC33;
        
        u4Val1 = *((volatile UINT32 *)u4CheckAddr1);
        
        if (u4Val1 == 0x12345678)
        {
            u4DramSize *= 2;
        }
        else            
        {
            fgSizingDone = TRUE;
        }
    }
	
#if defined(CC_FORCE_DRAM768M) || defined(CC_FORCE_DRAM1G)
	if (!IS_DRAM_CHANNELB_ACTIVE())
		u4DramSize = 0x20000000;	// CHannel A 512M
	else
        #ifdef 	CC_FORCE_DRAM768M	
		u4DramSize = 0x10000000;	//Channel B 256M
        #else
		u4DramSize = 0x20000000;	//Channel B 512M
        #endif
#endif

    return u4DramSize;
}
#endif


//----------------------------------------------------------------------------
/* DDR_SetDramController() to set DRAM controller parameter
 *  @param u4Clk    Data rate divided by 2
 */
//----------------------------------------------------------------------------
void DDR_SetDramController()
{    
#if defined(DRAM_DEBUG) || defined(DRAM_SLT_DEBUG)
    // User's chance to modify the default settings at TCM_DRAM_SETUP.
    //CLK_QueryDramSetting((UINT8 *)"scan clock-cmd/addr phase relation:", DRAM_CHANGE_CLK_PI_SETTING);
#endif /* #if defined(DRAM_DEBUG) || defined(DRAM_SLT_DEBUG) */
	DramcInit();
	DramcConfig();
	//_DdrSetDramRefresh();
}

#else /* #ifdef CC_MTK_PRELOADER */
//-----------------------------------------------------------------------------
/** DDR_SetAgentPriority() Set dram group 1 agent priorities
 *  @param pu1Priorities   Priorities of aud, dmx, fci, vbi, osd, pscan,
 *                         b2r, cpu, scpos
 *  @retval none.
 */
//-----------------------------------------------------------------------------
void DDR_SetAgentPriority(const UINT8 *pu1Prio)
{
    UINT64 u8Priority = 0;
    INT32 i;
    UINT8 u1Prio;

    ASSERT(pu1Prio != NULL);

    for (i = 0; i < 16; i++)
    {
        u1Prio = pu1Prio[i];
        // '1' ~ '9' ==> Pri 1 ~ 9
        if ((u1Prio <= (UINT8)'9') && (u1Prio >= (UINT8)'0'))
        {
            u1Prio -= (UINT8)'0';
        }
        // 'A' ==> Pri 10
        else if ((u1Prio == (UINT8)'A') || (u1Prio == (UINT8)'a'))
        {
            u1Prio = 10;
        }
        // 'B' ==> Pri 11
        else if ((u1Prio == (UINT8)'B') || (u1Prio == (UINT8)'b'))
        {
            u1Prio = 11;
        }
        // 'C' ==> Pri 12
        else if ((u1Prio == (UINT8)'C') || (u1Prio == (UINT8)'c'))
        {
            u1Prio = 12;
        }
        else if ((u1Prio == (UINT8)'D') || (u1Prio == (UINT8)'d'))
        {
            u1Prio = 13;
        }
        else if ((u1Prio == (UINT8)'E') || (u1Prio == (UINT8)'e'))
        {
            u1Prio = 14;
        }
        else if ((u1Prio == (UINT8)'F') || (u1Prio == (UINT8)'f'))
        {
            u1Prio = 15;
        }
    //    else if (i == 9)    // backward compatible, for only 9 agents
    //    {
   //         u1Prio = 9;
    //    }
        else
        {
            ASSERT(0);
            return;
        }

        u8Priority |= (((UINT64)u1Prio) << (4 * i));
    }

    // Set agent 0 ~ 7 of group 1 static priority.
    IO_WRITE32(DRAM_DMARB_CHB_BASE, 0x10, (UINT32)(u8Priority & 0xffffffff));
    IO_WRITE32(DRAM_DMARB_BASE, 0x10, (UINT32)(u8Priority & 0xffffffff));

    // Set agent 8 ~ 15 of group 1 static priority.
    IO_WRITE32(DRAM_DMARB_CHB_BASE, 0x14, (UINT32)((u8Priority>>32) & 0xffffffff));
    IO_WRITE32(DRAM_DMARB_BASE, 0x14, (UINT32)((u8Priority>>32) & 0xffffffff));


    // Set dynamic priority control.
    // dynamic priority on NR: 0x20007128[13:0] <= 14'h2031
    // DRAM_WRITE32(0x128, (DRAM_READ32(0x128) & ~(0x3fffU << 0)) | (0x2031U << 0));
    // dynamic priority on Scalar: 0x2000712c[29:16] <= 14'h2031
    // DRAM_WRITE32(0x12c, (DRAM_READ32(0x12c) & ~(0x3fffU << 16)) | (0x2031U << 16));
    // dynamic priority on VDEC: 0x20007130[13:0] <= 14'h2031
    // DRAM_WRITE32(0x130, (DRAM_READ32(0x130) & ~(0x3fffU << 0)) | (0x2031U << 0));
    // Enable dynamic control.
    //DRAM_WRITE32(0x150, (DRAM_READ32(0x150) | 0x10000));
}

//-----------------------------------------------------------------------------
/** DDR_SetBurstLen() Set dram group 1 agent burst length
 *  @param u4BurstLen1     Burst length of agent 0 ~ 7
 *  @param u4BurstLen2     Burst length of agent 8 ~ 16
 *  @retval none.
 */
//-----------------------------------------------------------------------------
void DDR_SetBurstLen(UINT32 u4BurstLen1, UINT32 u4BurstLen2)
{
    IO_WRITE32(DRAM_DMARB_BASE, 0x20, u4BurstLen1);
    IO_WRITE32(DRAM_DMARB_BASE, 0x24, u4BurstLen2);
}

//-----------------------------------------------------------------------------
/** DDR_SetArbitorTime() Set dram group arbiter time
 *  @param u1Group         Dram agent group (1,2,3)
 *  @param u1Time          Arbitor time (0 ~ 15)
 *  @retval none.
 */
//-----------------------------------------------------------------------------
void DDR_SetArbiterTime(UINT8 u1Group, UINT8 u1Time)
{
    UINT32 u4RegVal;

    if (u1Time > 15)
    {
        u1Time = 15;
    }

    u4RegVal = IO_READ32(DRAM_DMARB_BASE, 0x60);
    switch(u1Group)
    {
    case 1:
        u4RegVal &= 0xff0fffff;
        u4RegVal |= (((UINT32)u1Time) << 20);
        break;
    case 2:
        u4RegVal &= 0xf0ffffff;
        u4RegVal |= (((UINT32)u1Time) << 24);
        break;
    case 3:
        u4RegVal &= 0x0fffffff;
        u4RegVal |= (((UINT32)u1Time) << 28);
        break;
    default:
        return;
    }
    IO_WRITE32(DRAM_DMARB_BASE, 0x60,u4RegVal);
}
#endif
//================================================================================
// Load dram config from adv_boot_config_data
//================================================================================

void vDDR_LoadBLClock(void)
{
	UINT32 u4Tmp;
    UINT32 u4DramClk;
	
#if 0 //mtk40739 temp mark 
	UINT32 u4BLDramCfg;

	u4BLDramCfg = BIM_READ32(REG_RW_GPRDW5); // Load DRAM config from GPR
	//LOG(1,"\nBL Config: %x\n", u4BLDramCfg);
	u4Tmp = 0;
	u4Tmp = DDR_GET_CLOCK_CONFIG(u4BLDramCfg);
#else
    // fix dram clock @ bring up
    u4Tmp = 6; // for 1512MHz (DDR3-1600)
    //u4Tmp = 1; // for 972MHz (DDR3-1066)
    //u4Tmp = 0; // for 864MHz
#endif	

    //default : 864M
    //1: 864 + 108 * 1 = 972
    //2: 864 + 108 * 2 = 1080
    //3: 864 + 108 * 3 = 1188
    //4: 864 + 108 * 4 = 1296
    //5: 864 + 108 * 5 = 1404
    //6: 864 + 108 * 6 = 1512
    //7: 864 + 108 * 7 = 1620
    //8: 864 + 108 * 8 = 1728
    //9: 864 + 108 * 9 = 1836
    //10: 864 + 108 * 10 = 1944
	//LOG(1,"\nu4Tmp: %x\n", u4Tmp); //mtk40739 add
#if SUPPORT_MEMPLL_FRACT
    u4DramClk = 1600;
#else
	u4DramClk =((DEFAULT_DDR_CLOCK / BASE_DDR_CLK) + (XTAL_MHZ*4*u4Tmp));	
#endif
	//LOG(1,"\nu4DramClk: %d\n", u4DramClk); //mtk40739 add

	#if 0
	switch(u4Tmp)
	{
	case 1:
	case 2:
	case 3:
		u4DramClk =((DEFAULT_DDR_CLOCK / BASE_DDR_CLK) + (XTAL_MHZ*2*u4Tmp));
		break;
	case 11:
	case 12:
	case 13:
		u4DramClk = (DEFAULT_DDR_CLOCK / BASE_DDR_CLK) - (XTAL_MHZ*2*(u4Tmp-10));
		break;
	default:
        u4DramClk = (DEFAULT_DDR_CLOCK / BASE_DDR_CLK);
		break;
	}
	#endif
    TCM_DRAM_FLAGS = TCM_DRAM_FLAGS & (~DRAM_CLOCK_MASK);
    TCM_DRAM_FLAGS |= (u4DramClk & DRAM_CLOCK_MASK);
}


#if 1
/*dram resizing */
#define REG_RW_RKCFG                        0x0020      //RANK CONFIGURATION
#define RW_RKCFG_CHSEL_OFFSET               20          //001: address > 64MB, then channel B 010: address > 128MB, then channel B 011: address > 256MB, then channel B 100: address > 512MB, then channel B others: channel A
#define RW_RKCFG_CHSEL_MASK                 0x00700000  //dual channel selection
#define RW_RKCFG_CHSEL_64M                  0x1         //If DRAM > 64M, Channel 2, Else Channel 1
#define RW_RKCFG_CHSEL_128M                 0x2         //If DRAM > 128M, Channel 2, Else Channel 1
#define RW_RKCFG_CHSEL_256M                 0x3         //If DRAM > 256M, Channel 2, Else Channel 1
#define RW_RKCFG_CHSEL_512M                 0x4         //If DRAM > 512M, Channel 2, Else Channel 1

#define CONFIG_DRAM_SIZE_AUTO_MODEL         0           // Auto detect channel and size
#define CONFIG_SINGLE_DRAM_SIZE_256_MODEL   10
#define CONFIG_SINGLE_DRAM_SIZE_384_MODEL   11
#define CONFIG_SINGLE_DRAM_SIZE_512_MODEL   12
#define CONFIG_SINGLE_DRAM_SIZE_768_MODEL   13
#define CONFIG_SINGLE_DRAM_SIZE_128_MODEL   20

#define DRAM_SIZE_64M               		0x04000000
#define DRAM_SIZE_128M               		0x08000000
#define DRAM_SIZE_256M               		0x10000000
#define DRAM_SIZE_384M               		0x18000000
#define DRAM_SIZE_512M               		0x20000000
#define DRAM_SIZE_768M               		0x30000000
#define DRAM_SIZE_1024M              		0x40000000

static void _DDR_CheckBLCfg(UINT32 u4Ch1DramSize, UINT32 u4Ch2DramSize)

{
    UINT32 u4RealDramSize;
    UINT32 u4CfgDramSize;
    
    u4RealDramSize = TCMGET_CHANNELA_SIZE();
    u4CfgDramSize = u4Ch1DramSize + u4Ch2DramSize;
    u4CfgDramSize = u4CfgDramSize>>20;

    if(u4CfgDramSize > u4RealDramSize)
	{

		LOG(1, "DDR config error!\n");
	#if 1
		LOG(1, "DDR PHY: %d MB\n", u4RealDramSize);
        LOG(1, "DDR CFG: %d MB\n", u4CfgDramSize);
	#endif
		while(1);
	} 
}

#if 0 

void vDDR_DramReSizing(void)
{
	UINT32 u4Ch1DramSize = 0;
	UINT32 u4Ch2DramSize = 0;
	UINT32 u4BLDramCfg = 0;
    UINT32 u4Val;
    UINT32 u4RealDramSize;
    UINT32 i;
    
    u4RealDramSize = TCMGET_CHANNELA_SIZE();
    u4RealDramSize = u4RealDramSize<<20;

    u4BLDramCfg = BIM_READ32(REG_RW_GPRDW5); // load dram config
	u4BLDramCfg = DDR_GET_SIZE_CONFIG(u4BLDramCfg);
    u4Val = ucDram_Register_Read(mcSET_ARBITER_ADDR(0x00));
    mcCLR_MASK(u4Val,RW_RKCFG_CHSEL_MASK);
    /*Fix dram size single channel 256MB */
    //u4BLDramCfg = CONFIG_SINGLE_DRAM_SIZE_256_MODEL;
	//dram1/dram2 range setting
	switch(u4BLDramCfg)
	{
	case CONFIG_SINGLE_DRAM_SIZE_128_MODEL:
		u4Ch1DramSize = DRAM_SIZE_64M;
		u4Ch2DramSize = DRAM_SIZE_64M;	
		_DDR_CheckBLCfg(u4Ch1DramSize+u4Ch2DramSize, 0);
		ucDram_Register_Write(mcSET_ARBITER_ADDR(0x00), u4Val | (RW_RKCFG_CHSEL_128M << RW_RKCFG_CHSEL_OFFSET));	
		break;
	case CONFIG_SINGLE_DRAM_SIZE_256_MODEL:
		u4Ch1DramSize = DRAM_SIZE_128M;
		u4Ch2DramSize = DRAM_SIZE_128M;	
		_DDR_CheckBLCfg(u4Ch1DramSize+u4Ch2DramSize, 0);
		ucDram_Register_Write(mcSET_ARBITER_ADDR(0x00), u4Val | (RW_RKCFG_CHSEL_256M << RW_RKCFG_CHSEL_OFFSET));
		break;	

	case CONFIG_SINGLE_DRAM_SIZE_512_MODEL:
		u4Ch1DramSize = DRAM_SIZE_256M;
		u4Ch2DramSize = DRAM_SIZE_256M;	
		_DDR_CheckBLCfg(u4Ch1DramSize+u4Ch2DramSize, 0);
		ucDram_Register_Write(mcSET_ARBITER_ADDR(0x00), u4Val | (RW_RKCFG_CHSEL_512M << RW_RKCFG_CHSEL_OFFSET));
		break;

	default:
		u4Ch1DramSize = u4RealDramSize/2;
		u4Ch2DramSize = u4RealDramSize/2;

        i = 0;
        u4RealDramSize =  u4RealDramSize>>26;
        while(u4RealDramSize)
        {
            i++;
            u4RealDramSize = u4RealDramSize>>1;
        }
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x00), u4Val | (i << RW_RKCFG_CHSEL_OFFSET));
		break;
	}

    BIM_WRITE32(REG_RW_GPRDW6, u4Ch1DramSize);
    BIM_WRITE32(REG_RW_GPRDW7, u4Ch2DramSize);
}

//mtk40739 mark for refine code size
void vDDR_DramCheckPCBVersion(void)
{
	UINT32 u4BLDramPCB = 0;

    u4BLDramPCB = BIM_READ32(REG_RW_GPRDW5); // load dram config
	//LOG(1, "\nu4BLDramPCB =%X\n",u4BLDramPCB); //mtk40739 pcb unknow    
	u4BLDramPCB = DDR_GET_VERSION_PCB_CONFIG(u4BLDramPCB);
	

	switch(u4BLDramPCB)
	{
	case DRAM_PCB_VERSION_MT8563P1V1:
        LOG(1, "DDR PCB: MT8563P1V1\n");
		break;
		
	case DRAM_PCB_VERSION_MT8659P1V1:
        LOG(1, "DDR PCB: MT8659P1V1\n");
		break;	

	case DRAM_PCB_VERSION_MT1389P1V1:
        LOG(1, "DDR PCB: MT1389P1V1\n");
		break;
		
	case DRAM_PCB_VERSION_MT8507P1V1:
        LOG(1, "DDR PCB: MT8507P1V1\n");
		
		break;
	case DRAM_PCB_VERSION_MT8639P1V1:
        LOG(1, "DDR PCB: MT8507P1V1\n");
		break;	

	case DRAM_PCB_VERSION_MT8639P2V1:
        LOG(1, "DDR PCB: MT8639P2V1\n");
		break;

	default:
//        LOG(1, "DDR PCB UNKNOW =%d\n",u4BLDramPCB); //mtk40739 pcb unknow case
		break;
	}
}
#endif


#endif



