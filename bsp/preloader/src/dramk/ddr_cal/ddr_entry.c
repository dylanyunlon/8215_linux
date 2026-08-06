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

#include "ddr_includes.h"
#include "x_pdwnc.h"


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


#if 0
void ddr_init_dramc_dram(void)
{   
    // Init channel A DRAM.
    TCMSET_CHANNELA_ACTIVE();
    DDR_SetDramController();

#if 0
    // Init channel B DRAM.
    if (IS_DRAM_CHANNELB_SUPPORT())
    {
        mcSHOW_DBG_MSG("Channel B DRAM enable.\n");
		
        TCMSET_CHANNELB_ACTIVE();        
        DDR_SetDramController();
    }
#endif    
}
#endif


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


UINT32 _DDR_CheckSize(BOOL fgChBActive)
{  
    register UINT32 u4DramSize = 0x4000000;   // 64 Mbytes.
    register UINT32 u4CheckAddr1;
    register UINT32 u4CheckAddr2;  
    register UINT32 u4BaseAddr;      
    register UINT32 u4Val1, u4value = 0;
#if SUPPORT_2GB_SIZE
    register UINT32 u4MaxAddr = 0x80000000;  // 2 Gbytes.
#else
    register UINT32 u4MaxAddr = 0x40000000;  // 1 Gbytes.
#endif

	BOOL fgSizingDone = FALSE;

    if (fgChBActive)
    {
        u4BaseAddr = INIT_DRAM_B_CHB_BASE;
    }
    else
    {
#if SUPPORT_2GB_SIZE
        u4BaseAddr  = 0;
#else
        u4BaseAddr = INIT_DRAM_B_BASE;
#endif
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
#if  SUPPORT_AYSMMETRIC
            if((u4Val1&0x0000FFFF) == 0x00005678)
            {          
                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
                mcSET_FIELD(u4value, 1, 0x00000002, 1);
                mcSET_FIELD(u4value, 1, 0x00000004, 2); 
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value); 

                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0));
                mcSET_FIELD(u4value, 0, 0x00001000, 12);
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0), u4value); //CS0 is also applied to CS1
                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0));

                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
                if(u4DramSize == 0x20000000)
                {		    
                    mcSET_FIELD(u4value, 1, 0x00000010, 4);  // 0:1G+512M 1:512M+256
                    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);    
                }
                else if(u4DramSize == 0x40000000)
                {
                    mcSET_FIELD(u4value, 0, 0x00000010, 4);  // 0:1G+512M 1:512M+256
                    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);
                }

                if((u4DramSize == 0x20000000)|| (u4DramSize == 0x40000000))
                {
                    u4CheckAddr1 = u4BaseAddr + (u4DramSize*1) - 4;
                    *((volatile UINT32 *)u4CheckAddr1) = 0x1133EEFF; 
                    u4CheckAddr2 = u4BaseAddr + u4DramSize + (u4DramSize>>1) - 4;
                    *((volatile UINT32 *)u4CheckAddr2) = 0x87654321;
                    u4Val1 = *((volatile UINT32 *)u4CheckAddr1);
                    if (u4Val1 == 0x1133EEFF)
                    {
                        u4DramSize = u4DramSize + (u4DramSize >>1);
                    }
                }

                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RKCFG));
                mcSET_FIELD(u4value, 1, 0x00000002, 1);
                mcSET_FIELD(u4value, 1, 0x00000010, 4);
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RKCFG), u4value);

             }
             
#endif 
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

		//bin yang add: save u4DramSzie to 0xF0008104
		
		BIM_WRITE32(REG_RW_GPRDW1, u4DramSize);
    return u4DramSize;
}


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

#ifdef DRAM_CALI_FAIL_RESET
void _reset1()
{
	/*
	 * use powerdown watch dog to reset system
	 */
	UINT32 u4Test;

	Printf("Preloader Reboot is working now.\n");

	PDWNC_WRITE32(0x164, 0x24000164);

	//PDWNC_WRITE32(REG_RW_WDT, 0xff000000);
	PDWNC_WRITE32(REG_RW_WDT, 0xFFFF0000);

	//for(u4Test = 0; u4Test < 10000; u4Test++)
	//{

	//}
	PDWNC_WRITE32(REG_RW_WDTSET, 1);
	while(1);

}
#endif

static UINT32 _ddr_do_calibration(void)
{
	UINT32 u4Ret = 0;

#ifdef DRAM_RX_EYE_SCAN
	U8 dq_no=0;
#endif

#ifdef DRAM_Pll_PHASE_CAL
	DramcPllPhaseCal();
#endif

#ifdef DRAM_IMPEDANCE_CAL
	DramcImpedanceCalApply();
#endif

#ifdef DRAM_WRITE_LEVELING_CAL
	DramcWriteLeveling();
#endif

#ifdef DRAM_MiockJmeter	
	DramcMiockJmeter();
#endif

#ifdef DRAM_GATING_SCAN
	u4Ret = DramcRxdqsGatingCal();
#endif

#ifdef DRAM_RX_ODT_SCAN
	DramcRxOdtScan();
#endif

#ifdef DRAM_RX_WINDOW_PERBIT_CAL
	u4Ret |= DramcRxWindowPerbitCal();
#endif

#ifdef DRAM_RX_DATLAT_CAL
	if(DramcRxdatlatCal(0)== DRAM_FAIL)
    {
#ifdef DRAM_CALI_FAIL_RESET
        _reset1();
#endif
    }
#endif

#ifdef DRAM_TX_OCD_DRV_SCAN
	DramcTxOcdDrvScan();
#endif

#ifdef DRAM_TX_VREF_CAL
	DramcTxVrefCal();
#endif

#ifdef DRAM_TX_WINDOW_PERBIT_CAL
	u4Ret |= DramcTxWindowPerbitCal();
#endif

#ifdef DRAM_RX_EYE_SCAN
	for(dq_no =0;dq_no<32;dq_no++)
		DramcRxEyeScan(dq_no);
#endif

#ifdef DRAM_TX_EYE_SCAN
	DramcTxEyeScan();
#endif

    return u4Ret;
}


void ddr_calibrate(void)
{   
	UINT32 u4Ret = 0;
	UINT32 u4Val;
	

	mcSHOW_DBG_MSG("DRAM Channel A Calibration.\n");
	

	TCMSET_CHANNELA_ACTIVE();
   
	u4Ret = _ddr_do_calibration();

		
	if (u4Ret == DRAM_OK)
    {
#if SUPPORT_2GB_SIZE
        ucDram_Register_Write(0xF003801c,1);
#endif
        TCMSET_CHANNELA_SIZE(_DDR_CheckSize(FALSE)/0x100000); //get channel A dram size
        mcSHOW_ERROR_CHIP_DisplayString("DRAM A Size = ");
        mcSHOW_ERROR_CHIP_DisplayInteger(TCMGET_CHANNELA_SIZE());
        mcSHOW_ERROR_CHIP_DisplayString(" Mbytes.\n");
		
		
    }
#if 0		
	if (IS_DRAM_CHANNELB_SUPPORT())
	{
		mcSHOW_DBG_MSG("DRAM Channel B Calibration.\n");

		TCMSET_CHANNELB_ACTIVE();
		
		u4Ret |= _ddr_do_calibration();
		
        if (u4Ret == DRAM_OK)
        {
            TCMSET_CHANNELB_SIZE(_DDR_CheckSize(TRUE)/0x100000);
            mcSHOW_ERROR_CHIP_DisplayString("DRAM B Size = ");       
            mcSHOW_ERROR_CHIP_DisplayInteger(TCMGET_CHANNELB_SIZE());
            mcSHOW_ERROR_CHIP_DisplayString(" Mbytes.\n");
			
        }
	
		// Back to channel A.
		TCMSET_CHANNELA_ACTIVE();
	}
#endif
			
}


#ifdef DRAM_WRITE_READ_LOOP_AFTER_CALIBRATION
void dram_test_after_cali(void)
{
    UINT32 i;
    UINT32 u4err_value;
    UINT32 test2_0 = DEFAULT_TEST2_0_CAL;
    UINT32 test2_1 = DEFAULT_TEST2_1_CAL;
    UINT32 test2_2 = DEFAULT_TEST2_2_CAL;
    
    for(i=0;(i*DEFAULT_TEST2_2_CAL*32)<(TCMGET_CHANNELA_SIZE()*1024*1024);i++)
    {
        test2_1 =  DEFAULT_TEST2_1_CAL | (i*DEFAULT_TEST2_2_CAL);
        u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);

        if(!u4err_value)
        {   
            mcSHOW_DBG_MSG("Channel A ");
            mcSHOW_ERROR_CHIP_DisplayHex(i*DEFAULT_TEST2_2_CAL*32);
            mcSHOW_DBG_MSG(" ~ ");
            mcSHOW_ERROR_CHIP_DisplayHex((i+1)*DEFAULT_TEST2_2_CAL*32-1);
            mcSHOW_DBG_MSG(" Pass!!\n");
			

        }
        else
        {
            mcSHOW_DBG_MSG("Channel A ");
            mcSHOW_ERROR_CHIP_DisplayHex(i*DEFAULT_TEST2_2_CAL*32);
            mcSHOW_DBG_MSG(" ~ ");
            mcSHOW_ERROR_CHIP_DisplayHex((i+1)*DEFAULT_TEST2_2_CAL*32-1);
            mcSHOW_DBG_MSG(" Error!!\n");
            mcSHOW_DBG_MSG("Error value is ");
            mcSHOW_ERROR_CHIP_DisplayHex(u4err_value);
            mcSHOW_DBG_MSG(".\n");

            while(1);
        }
        mcDELAY_us(1000);
    }
        
    if (IS_DRAM_CHANNELB_SUPPORT())
    {
        TCMSET_CHANNELB_ACTIVE();
        
        for(i=0;(i*DEFAULT_TEST2_2_CAL*32)<(TCMGET_CHANNELB_SIZE()*1024*1024);i++)
        {
            u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);
            if(!u4err_value)
            {
                mcSHOW_DBG_MSG("Channel B ");
                mcSHOW_ERROR_CHIP_DisplayHex(i*DEFAULT_TEST2_2_CAL*32);
                mcSHOW_DBG_MSG(" ~ ");
                mcSHOW_ERROR_CHIP_DisplayHex((i+1)*DEFAULT_TEST2_2_CAL*32-1);
                mcSHOW_DBG_MSG(" Pass!!\n");
            }
            else
            {
                mcSHOW_DBG_MSG("Channel B ");
                mcSHOW_ERROR_CHIP_DisplayHex(i*DEFAULT_TEST2_2_CAL*32);
                mcSHOW_DBG_MSG(" ~ ");
                mcSHOW_ERROR_CHIP_DisplayHex((i+1)*DEFAULT_TEST2_2_CAL*32-1);
                mcSHOW_DBG_MSG(" Error!!\n");
                mcSHOW_DBG_MSG("Error value is ");
                mcSHOW_ERROR_CHIP_DisplayHex(u4err_value);
                mcSHOW_DBG_MSG(".\n");
                while(1);
            }
            mcDELAY_us(1000);
        }
        TCMSET_CHANNELA_ACTIVE();
    }
}
#endif



