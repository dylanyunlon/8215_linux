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
#include "boot.h"

#ifdef CC_MTK_PRELOADER
#define fcMEMCTLPLL_PWD_ISSUE
//#define fcMEMCTLPLL_MUX_ISSUE --> [JC] Need to double check this definition
//                                               --> [QW] mt8563 have fixed
#ifdef FOR_MT3363
#define fcSET_MC_CLK_AS_PLL        0 //dram clock source select    //zhishang 3363 change 1->0
#else
#define fcSET_MC_CLK_AS_PLL        1
#endif
#define fcSET_AXI_CLK_USE_CLK_DRAM 1 //[important]!!! must set, otherwise BIM can't access dram

//-------------------------------------------------------------------------
/** DdrPhyInit
 *  DDR PHY Initialization.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
#define DRAMK_SIMULATION 0 //For FPGA no DDRPHY register
void DdrPhyInit(void)
{
    U16 u2frequency1, u2real_freq;
    U32 u4frequency_hex;
    U8 fgvcocal_cplt, ucvco_state, ucvco_state_assign, ucPiDelay = 0x3d;
    U32 u4value;
    U32 u4clock;
    //U8 ucloop_count = 0;
    U32 u4mempll_prediv_hex = 0;
    U32 u4mempll_predivider[3] = {1, 2, 4}; //caution: u4mempll_predivider = 2^u4mempll_prediv_hex;
    U32 u4ssc_prd, u4ssc_delta;
    U32 u4Permillage = 0;
    U32 u4Frequency = 0;
    U32 ucloop_count=0;
	U32 u4feed_div_int=0,u4feed_div_fraction=0,u4real_freq_fraction=0;
#if DRAMK_SIMULATION
    U32 regddrphy_a40 = 0;
    U32 regddrphy_0bc = 0x00AA0000;
    U32 regddrphy_a4c = 0x00000005;
    U32 regddrphy_2a0 = 0x0000A000;
    U32 regddrphy_2a8 = 0x8600200C;
    U32 regddrphy_0c0 = 0x00000008;
    U32 regddrphy_0c8 = 0x08068600;
    U32 regddrphy_2b4 = 0x00000806;
#endif

#if SUPPORT_8BIT
    mcSHOW_DBG_MSG2("Support 4*8bit DDR\n");
#endif

    u4clock = (TCMGET_DDR_CLK()/BASE_DDR_CLK)/2;
    u2frequency1 = u4clock / 2;
    u4frequency_hex = (U32) (u2frequency1/(XTAL_MHZ/u4mempll_predivider[u4mempll_prediv_hex]));

#if !(SUPPORT_MEMPLL_FRACT)
    // ceil operation
    if ((u4frequency_hex*(XTAL_MHZ/u4mempll_predivider[u4mempll_prediv_hex])) != u2frequency1)
    {
        u4frequency_hex = u4frequency_hex + 1;
    }
#endif

    u4frequency_hex = u4frequency_hex - 1;

    //  bit8 to 0  we just need [7:0]
    u4frequency_hex =u4frequency_hex&0x000000ff;
    u2real_freq = (U16) ((u4frequency_hex+1)*(XTAL_MHZ/u4mempll_predivider[u4mempll_prediv_hex])*2);
#if SUPPORT_MEMPLL_FRACT
	//MEMPLL fractional setting..
    u4feed_div_int = (u4clock/2)/(XTAL_MHZ); 
    u4feed_div_fraction = (u4clock/2 - u4feed_div_int*XTAL_MHZ)*16/XTAL_MHZ; 
	mcSHOW_DBG_MSG2("u4feed_div_int =%d, u4feed_div_fraction =%d\n", u4feed_div_int, u4feed_div_fraction);

	u4real_freq_fraction = (U32)u2real_freq+((U32)u2frequency1-(u4frequency_hex + 1)*XTAL_MHZ)/u4mempll_predivider[u4mempll_prediv_hex]*2;
	mcSHOW_DBG_MSG2("Real frequency from PLL is %d MHz\n", u4real_freq_fraction);	
	UNUSED(u4real_freq_fraction);
#else
    mcSHOW_DBG_MSG2("Real frequency from PLL is %d MHz\n", u2real_freq);
    LOG(1,"DDR3, data rate: %d MHz. \n", u2real_freq*2);
	
    UNUSED(u2real_freq);
#endif

    /*[20130828]QW: As Zhijie's advice, set mc clock source as  pll for normal use */
#if fcSET_MC_CLK_AS_PLL
    /*CKGEN_REG 60[4:2] = 0 */
    u4value = ucDram_Register_Read(0x70000060);
    u4value = u4value & (~(0x07L<<2));
    ucDram_Register_Write(0x70000060, u4value);

    /*DDRPHY_REG a40[16]=1, then a40[16]=0*/
#if DRAMK_SIMULATION
    regddrphy_a40 |= (0x01L<<16);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL), regddrphy_a40);

    regddrphy_a40 &= ~(0x01L<<16);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL), regddrphy_a40);
#else

    //R_CTLCLKMUX_CHG: Toggle to 1 to change clock source of the glitch-free clock mux (MT5399 E2 version)
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL));
    mcSET_BIT(u4value, 16);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL), u4value);
    mcDELAY_us(1);
    mcCLR_BIT(u4value, 16);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL), u4value);

    mcDELAY_us(100);
#endif
#endif

    //PHY initial settings (based on DR simulation settings, from Justin Chan)
    //-A1 part
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0), 0x00000000);
    //--2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG4), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG5), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG6), 0x00000000);
    //--2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG9), 0x00000000);
    //--2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG12), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG13), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG14), 0x00000008);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15), 0x00000008);
#ifdef DDR_CHA_4BIT_SWAP
    //RG_*_RX_BYTE_SWAP=1・b1
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16), 0x0000006e);
    //RG_*_RX_DQSI_SEL=16・hf0f0
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), 0xF0F00100);
#else
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16), 0x0000002e);
    //--modified by benson 1018, change DQS sel setting [31:16]
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), 0xFF000100);
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18), 0x8000e008|(DEFAULT_TX_VREF<<4));
    //--2012/10/12, Alcuary, SCAN_IN_BUF off [31:16] for power saving
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG20), 0x00600000|(SR_VALUE_CLK<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG21), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG22), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG23), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG24), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG25), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG26), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG27), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG28), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG29), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG30), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG31), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG32), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG33), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG34), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG35), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG36), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG37), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG38), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG39), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG40), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG41), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG42), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG43), 0x00200000|(SR_VALUE<<16));
    //--RG_TX_ARCS_CSBEN, [22]=1 for differential CS
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG44), 0x00600000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG45), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG46), 0x00200000|(SR_VALUE<<16));
    //RG_TX_CMDA_EN=0 for power saving, SP, 2012/10/29
    //ucDram_Register_Write(mcSET_PHY_REG_ADDR(0x0bc), 0x00600000|(SR_VALUE<<16));
    //After Zhijie double check, must be RG_TX_CMDA_EN = 1, otherwise no cmd singal outpt, 2013/10/16
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG47), 0x00600000|(SR_VALUE<<16)|(1<<15));
#if DRAMK_SIMULATION
    regddrphy_0bc = 0x00600000|(SR_VALUE<<16)|(1<<15);
#endif
    //--PLL registers will be set later
    //...0x10c4~0x10cc
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG48), 0x00000002);
#if DRAMK_SIMULATION
    regddrphy_0c0 = 0x00000002;
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), (DEFAULT_PI_A1_DQA<<24)|(DEFAULT_PI_A1_DQB<<8));
    //only for channel A, clk pi delay 14*T/64
    // 3363 DC balance ON, default = 0x00, check by waveform
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG53), ucPiDelay << 24);    //--RG_MEMPHYPLL_A1_TEST_EN (10D8H[4]) may be set later?, use default values here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG54), 0x00230000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG55), 0x00000000);
    //--RG_PHYPLL_A1_TOP_REV (10E0H[31:16]) may be set later?, use default values here
    //--set OCD / ODT default values to 45ohm / 120ohm
    //--[31:28] active cap off for power consumtion, 2012/10/12, from SP
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG56), 0xf0001717);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG57), 0x0000b8b8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG58), 0x00000300);
    
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1), 0x00500000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), 0x01400000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), 0x00000140|(SR_VALUE));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10), 0x00500000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11), 0x00000050|(SR_VALUE));
#if 0
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
    {
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1), 0x00400000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), 0x01500000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), 0x00000150|(SR_VALUE));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10), 0x00400000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11), 0x00000040|(SR_VALUE));
    }
    else
    {
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1), 0x00500000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), 0x01400000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), 0x00000140|(SR_VALUE));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10), 0x00500000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11), 0x00000050|(SR_VALUE));
    }
#endif

#if 0
    //-B2 part
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0), 0x00000000);
    //--2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG2), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG4), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG5), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG6), 0x00000000);
    //--2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG9), 0x00000000);
    //--2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG12), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG13), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG14), 0x00000008);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15), 0x00000008);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16), 0x0000002e);
    //--modified by benson 1018, change DQS sel setting [31:16]
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), 0xff000100);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG18), 0x8000e008|(DEFAULT_TX_VREF<<4));
    //--2012/10/12, Alcuary, SCAN_IN_BUF off [31:16]
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG20), 0x00600000|(SR_VALUE_CLK<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG21), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG22), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG23), 0x0020000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG24), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG25), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG26), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG27), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG28), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG29), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG30), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG31), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG32), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG33), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG34), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG35), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG36), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG37), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG38), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG39), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG40), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG41), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG42), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG43), 0x00200000|(SR_VALUE<<16));

    //RG_TX_BRCS_CSBEN, [22]=1 for differential CS
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG44), 0x00600000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG45), 0x00200000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG46), 0x00200000|(SR_VALUE<<16));
    //RG_TX_CMDB_EN=0 for power saving, SP, 2012/10/29
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG47), 0x00600000|(SR_VALUE<<16));
    //PLL registers will be set later (14c4~14cc)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG48), 0x00000002);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG52), (DEFAULT_PI_B2_DQA<<24)|(DEFAULT_PI_B2_DQB<<8));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG53), 0x00000000);
    //RG_MEMPHYPLL_B2_TEST_EN (14D8H[4]) may be set later, use default values here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG54), 0x00230000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG55), 0x00000000);
    //RG_PHYPLL_B2_TOP_REV (14E0H[31:16]) may be set later, use default values here
    //set OCD / ODT default values to 45ohm / 120ohm
    //[31:28] active cap off for power consumtion, 2012/10/12, from SP
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG56), 0xf0001717);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG57), 0x0000b8b8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG58), 0x00000300);

    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
    {
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1), 0x00400000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG5), 0x01500000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), 0x00000150|(SR_VALUE));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG10), 0x00400000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG11), 0x00000040|(SR_VALUE));
    }
    else
    {
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1), 0x00500000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG5), 0x01400000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), 0x00000140|(SR_VALUE));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG10), 0x00500000|(SR_VALUE<<16));
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG11), 0x00000050|(SR_VALUE));
    }
#endif

    //AB part
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG0), 0x00000000);
    //2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG2), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG4), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG5), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG6), 0x00000000);
    //2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG9), 0x00000000);
    //2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG12), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG13), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG14), 0x00000008);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15), 0x00000008);
#ifdef DDR_CHA_4BIT_SWAP
    //RG_*_RX_BYTE_SWAP=1・b1
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16), 0x0000006e);
    //RG_*_RX_DQSI_SEL=16・hf0f0
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), 0xf0f00100);
#else
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16), 0x0000002e);
    //modified by benson 1018, change DQS sel setting
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), 0xff000100);
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG18), 0x8000e008|(DEFAULT_TX_VREF<<4));
    //2012/10/12, Alcuary, SCAN_IN_BUF off [31:16]
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20), 0x00600000|(SR_VALUE_CLK<<16));

#if 0
	//2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG22), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG23), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG24), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG25), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG26), 0x00000000);
    //2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG29), 0x00000000);
    //2012/10/03, for 2133MHz
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG32), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG33), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG34), 0x00000008);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35), 0x00000008);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36), 0x0000002e);
    //modified by benson 1018, change DQS sel setting
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), 0xff000100);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38), 0x8000e008|(DEFAULT_TX_VREF<<4));
    //2012/10/12, Alcuary, SCAN_IN_BUF off [31:16]
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39), 0x00000000);
#endif

    //RG_MEMPLL_* (12A0H[15:0]), will be set later, use default values here
    //2012/10/2, MEMPLL to 2X mode
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40), 0x0060a050|(SR_VALUE_CLK<<16));
#if DRAMK_SIMULATION
    regddrphy_2a0 =  0x0060a050|(SR_VALUE_CLK<<16);
#endif
    //PLL registers will be set later (12a4~12c4)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47), 0x00008000);

    //RG_MEMPHYPLL_AB_*_DL (12C8H[20:16], 12C8H[28:24]), may be set later, use default values here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), DEFAULT_PI_A2_DQA<<8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), (ucPiDelay << 8) | (DEFAULT_PI_A2_DQB<<24));
    //RG_MEMPHYPLL_AB_TEST_EN (12D0[12]), may be set later, use default values here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), 0x23000000);
    //RG_PHYPLL_A2_TOP_REV (12D4[31:16]), may be set later, use default values here
    //[31:30] active cap off for power consumtion, 2012/10/12, from SP
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), 0xc0000000|(DEFAULT_PI_B1_DQA<<8));
#if 0 
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), DEFAULT_PI_B1_DQB<<24);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG55), 0x23000000);
#endif
    //PLL registers will be set later, RG_RSTB18V & RG_DMSUS18V?? (ignored from ACD/SP)
    //[31:30] active cap off for power consumtion, 2012/10/12, from SP
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG56), 0xc0000600);


		// A2
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1), 0x00500000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7), 0x01400000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8), 0x00000140|(SR_VALUE));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG10), 0x00500000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG11), 0x00000050|(SR_VALUE));
		// B1
#if 0 //fcFOR_ONE_CHANNEL_DESIGN
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG21), 0x00000050|(SR_VALUE));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27), 0x01400000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28), 0x00000140|(SR_VALUE));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG30), 0x00500000|(SR_VALUE<<16));
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG31), 0x00000050|(SR_VALUE));
#endif

    // ch-A, ???
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG60), 0x80806500);
#if 0  //zhishang 3363 should mark this
    // ch-B
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG60), 0x80806500);
#endif

    //PHY Wrapper part
    //RG_DDRPHY_RESETB (1A4CH[31]), will be set later, use default values here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), 0x03440055);
#if DRAMK_SIMULATION
    regddrphy_a4c = 0x03440055;
#endif
    //to solve 1st data DQS voltage too high issue (from Chaowei), 20121115
    //reg_dly_ODT(0x1a50[28:27])=00
#ifdef DDR_CONFIG_CSD
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_CMD_SELPH), 0x08002801);   //Write ODT Delay 0.5T by waveform
#else
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_CMD_SELPH), 0x08002800);   //Write ODT Delay 0.5T by waveform
#endif
#if SUPPORT_AYSMMETRIC
    mcSHOW_DBG_MSG2("Using AYSMMETRIC Function! \n");
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_CMD_SELPH));
    mcSET_FIELD(u4value, 1, 0x00000001, 0);  //CSD 1
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_CMD_SELPH), u4value);
	u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16));
    mcSET_FIELD(u4value, 1, 0x00000020, 5);  //CSD 1
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16), u4value);
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_CMD_SELPH_B), 0x01100051);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ANA_TST_OUT), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(0xa5c), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(0xa60), 0x00000000);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(0xa64), 0x00000000);
    //R_DMSYNCRST (1B44H[6]), will be set later, use default values here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), 0x00080300);

    //For MT5399, the default value is 32・h0000_f220 to disable PHY input buffer.
    //Enable it here
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_POWERDOWN), 0x00000110);
#if 0 
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_POWERDOWN_B), 0x00000110);
#endif
    //wait 1us, 1us here
    mcDELAY_us(1);

    //TX serializer RSTB, RG_TX_CMDA_RSTB (10BCH[14]) toggle from 0 to 1 (default is 0)
#if DRAMK_SIMULATION
    mcSET_BIT(regddrphy_0bc, 14);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG47), regddrphy_0bc);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG47));
    mcSET_BIT(u4value, 14);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG47), u4value);
#endif

#if 0 //zhishang 3363 should mark this
    //TX serializer RSTB, RG_TX_CMDB_RSTB (14BCH[14]) toggle from 0 to 1 (default is 0)
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG47));
    mcSET_BIT(u4value, 14);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG47), u4value);
#endif
    //DDRPHY clock engine mute, RG_DDRPHY_RESETB (1A4CH[31]) can be released (0->1)
#if DRAMK_SIMULATION
    mcSET_BIT(regddrphy_a4c, 31);
    //mcSET_FIELD(u4value, 0x0, 0x03000000, 24);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), regddrphy_a4c);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET));
    mcSET_BIT(u4value, 31);
    //mcSET_FIELD(u4value, 0x0, 0x03000000, 24);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), u4value);
#endif
    //wait 1us
    mcDELAY_us(1);

    //u4value = ucDram_Register_Read(0xf000d210);
    //mcCLR_BIT(u4value, 6);
    //ucDram_Register_Write(0xf000d210, u4value);

    //MEMPLL initialization sequence (refer to A60807 verification plan)

#ifdef  fcMEMCTLPLL_PWD_ISSUE
    // Move MEMCTLPLL init here; due to MEMPLL bias current controlled by MEMCTLPLL PWD
    //RG_MEMCTLPLL_AUTOK_LOAD (12B4H[0]) set to 0
    //RG_MEMCTLPLL_AUTOK_VCO (12B4[1]) set to 0
    //MEMCTLPLL setting (BW)
    //RG_MEMCTLPLL_DIVEN (12B4H[30:28])
    //RG_MEMCTLPLL_BC     (12B4H[20:19]=11)
    //RG_MEMCTLPLL_BIC    (12B4H[18:16]=010)
    //RG_MEMCTLPLL_BIR    (12B4H[27:24]=0010)
    //RG_MEMCTLPLL_BP     (12B4H[15:12]=0001)
    //RG_MEMCTLPLL_BR     (12B4H[23:21]=100)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG45), 0x029a1880);
#if DRAMK_SIMULATION
    regddrphy_2b4 = 0x029a1880;
#endif

    //delay 1us
    mcDELAY_us(1);

    // move to below
    //RG_MEMCTLPLL_PWD (12B0H[15]) from 1 to 0
    //u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(0x2b0));
    //mcCLR_BIT(u4value, 15);
    //ucDram_Register_Write(mcSET_PHY_REG_ADDR(0x2b0), u4value);

    //delay 100us, 1ms here
    //mcDELAY_us(100);
#endif

    //T1:
    //RG_MEMPLL_BIAS_PWD             (12A8H[25] = 1 -> 0)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), 0x8400200c);
#if DRAMK_SIMULATION
    regddrphy_2a8 = 0x8400200c;
#endif
    //RG_MEMPLL_AUTOK_LOAD           (12A4H[0]  = 0 -> 1)
    //recommend by ACD/SP, 20120810
    //RG_MEMPLL_BC     (12A4H[20:19]=11)
    //RG_MEMPLL_BIC    (12A4H[18:16]=010)
    //RG_MEMPLL_BIR    (12A4H[27:24]=0010)
    //RG_MEMPLL_BP     (12A4H[15:12]=0001)
    //RG_MEMPLL_BR     (12A4H[23:21]=100)
    //PLL setting...
    //RG_MEMPLL_PREDIV (12A0H[1:0] = 00)
    //RG_MEMPLL_POSDIV (12A0H[5:4] = 00)
    //RG_MEMPLL_FBSEL  (12A0H[7:6] = 00)
    //RG_MEMPLL_FBDIV  (12A0H[14:8])
#if DRAMK_SIMULATION
    mcSET_FIELD(regddrphy_2a0, u4frequency_hex, 0x00007f00, 8);
    mcSET_FIELD(regddrphy_2a0, u4mempll_prediv_hex, 0x00000003, 0);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40), regddrphy_2a0);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40));
    mcSET_FIELD(u4value, u4frequency_hex, 0x00007f00, 8);
    mcSET_FIELD(u4value, u4mempll_prediv_hex, 0x00000003, 0);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40), u4value);
#endif
    //20121005, from SP
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG41), 0x0d997883);

#if SUPPORT_MEMPLL_FRACT
	//RG_MEMPLL_REV (2A8H[23:20])
	//set this value as MEMPLL fraction
	u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
	mcSET_FIELD(u4value, u4feed_div_fraction, 0x00f00000, 20);  
	ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);
	mcSHOW_DBG_MSG2("DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42 u4value 0x%x Register 0x%x\n",u4value, ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42)));
#endif
    //DDS setting...
    //RG_MEMPLL_PCW_NCPO (12A8H[14:8])
    //set this value as MEMPLL FBDIV

#if DRAMK_SIMULATION
    mcSET_FIELD(regddrphy_2a8, u4frequency_hex, 0x00007f00, 8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), regddrphy_2a8);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
    mcSET_FIELD(u4value, u4frequency_hex, 0x00007f00, 8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);
#endif
    //Adjust SSC amplitude and frequency
    //DELTA1 (0x12B0[23:16])
    //DELTA (0x12B0[31:24])
    //PRD (0x12AC[15:0])
    //SSC_REV (0x12AC[23:16])
    //from A60806
    //DDS SSC setting
    //RG_SSC_PRD= (Fout/Fmod/2)=(24M/30K/2)=400
    //RG_SSC_DELTA=(Fin/Fout)*2A/(Fout/Fmod/2)*2^18
    //                       =(408M/24M)*0.5%*2/400*2^18=112
    /* Get Spectrum Permillage. */
    u4Permillage = DRVCUST_InitGet(eDmpllSpectrumPermillage);
    u4Frequency = DRVCUST_InitGet(eDmpllSpectrumFrequency);

    if ((DRVCUST_InitGet(eDmpllSpectrumPermillage) != 0) &&
            (DRVCUST_InitGet(eDmpllSpectrumFrequency) != 0))
    {
        u4ssc_prd = (((XTAL_MHZ/u4mempll_predivider[u4mempll_prediv_hex])*1000)/u4Frequency)/2;
        u4ssc_delta = (u4frequency_hex+1)*u4Permillage*2*262144/(10000*u4ssc_prd);
    }
    else
    {
        u4ssc_prd = 400;
        u4ssc_delta = 112;
    }
	//Down spread:RG_PHASE_JNI_1 = 1 and RG_SSC_DETAL1=RG_SSC_DELTA 
	//Center spread:RG_PHASE_JNI_1 = 1 and RG_SSC_DETAL1=RG_SSC_DELTA/2
	//up spread:RG_PHASE_JNI_1 = 0 and RG_SSC_DETAL1=RG_SSC_DELTA
#ifdef fcMEMCTLPLL_PWD_ISSUE
    u4value = 0x00000000|((u4ssc_delta)<<16)|(u4ssc_delta<<24); //Down spread
#else
    u4value = 0x00008000|((u4ssc_delta)<<16)|(u4ssc_delta<<24);
#endif

    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG44), u4value);
    u4value = 0x10ff0000|(u4ssc_prd);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG43), u4value);

    //delay 10us, 1ms here
    mcDELAY_us(10);

    //T2
    //RG_MEMPLL_PWD (12A0H[15]) from 1 to 0
#if DRAMK_SIMULATION
    mcCLR_BIT(regddrphy_2a0, 15);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40), regddrphy_2a0);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40));
    mcCLR_BIT(u4value, 15);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40), u4value);
#endif

    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0));
    while (mcCHK_BIT1(u4value, 25) == 0)
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0));
        mcDELAY_us(CMP_CPT_POLLING_PERIOD);
        ucloop_count++;
        if (ucloop_count > 10000)
        {
            mcSHOW_ERROR_CHIP_DisplayString("MEMPLL VCOCAL cplt flag polling timeout.\n");
			
            break;
        }
    }

    //check RGS_MEMPLL_VCOCAL_CPLT (12E8H[25]) & RGS_MEMPLL_VCO_STATE (12E8H[31:26])
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0));
    fgvcocal_cplt = (U8) mcGET_FIELD(u4value, 0x02000000, 25);
    ucvco_state = (U8) mcGET_FIELD(u4value, 0xfc000000, 26);
    mcSHOW_DBG_MSG2("MEMPLL VCOCAL CPLT FLAG = %d; VCO STATE = %d\n", fgvcocal_cplt, ucvco_state);

	//vMtkTraceDWRD(u4ssc_prd);
    //T3
    //RG_MEMPLL_BIAS_RST (12A8H[26]) from 1 to 0 (after VCO band K cplt)
#if DRAMK_SIMULATION
    mcCLR_BIT(regddrphy_2a8, 26);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), regddrphy_2a8);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
    mcCLR_BIT(u4value, 26);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);
#endif

    //MEMPLL ready -->

    //MEMPHYPLL initialization sequence (refer to A60807 verification plan)
    //T0
    //RG_MEMPHYPLL_A1_BIAS_PWD (10C8H[9]) from 1 to 0
    //RG_MEMPHYPLL_B2_BIAS_PWD (14C8H[9]) from 1 to 0
    //RG_MEMPHYPLL_AB_BIAS_PWD (12C4H[25]) from 1 to 0
    //RG_PI_A1_EN (10D8H[17]) from 0 to 1, has set @ PHY initial settings
    //RG_PI_A2_EN (12D0H[25]) from 0 to 1, has set @ PHY initial settings
    //RG_PI_B1_EN (12DCH[25]) from 0 to 1, has set @ PHY initial settings
    //RG_PI_B2_EN (14D8H[17]) from 0 to 1, has set @ PHY initial settings

    //T1
    //RG_MEMPHYPLL_A1_AUTOK_LOAD (10C8[16]) from 0 to 1
    //RG_MEMPHYPLL_B2_AUTOK_LOAD (14C8[16]) from 0 to 1
    //RG_MEMPHYPLL_AB_AUTOK_LOAD (12C0[0]) from 0 to 1
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG50), 0x18838400);
#if DRAMK_SIMULATION
    regddrphy_0c8 = 0x18838400;
#endif
#if 0   //zhishang 3363 should mark this
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG50), 0x18838400);
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG48), 0x029a1883);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG49), 0x84000001);


    //PLL settings (BW...)
    //recommend by ACD/SP, 20120810
    //RG_MEMPHYPLL_A1_BC     (10C4H[5:4]=11)
    //RG_MEMPHYPLL_A1_BIC    (10C4H[3:1]=010)
    //RG_MEMPHYPLL_A1_BIR    (10C4H[12:9]=0010)
    //RG_MEMPHYPLL_A1_BP     (10C8H[31:28]=0001)
    //RG_MEMPHYPLL_A1_BR     (10C4H[8:6]=100)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG49), 0x00000534);
    //10c8H has set above

#if 0
    //recommend by ACD/SP, 20120810
    //RG_MEMPHYPLL_B2_BC     (14C4H[5:4]=11)
    //RG_MEMPHYPLL_B2_BIC    (14C4H[3:1]=010)
    //RG_MEMPHYPLL_B2_BIR    (14C4H[12:9]=0010)
    //RG_MEMPHYPLL_B2_BP     (14C8H[31:28]=0001)
    //RG_MEMPHYPLL_B2_BR     (14C4H[8:6]=100)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG49), 0x00000534);
    //14c8H has set above
#endif
    //recommend by ACD/SP, 20120810
    //RG_MEMPHYPLL_AB_BC     (12C0H[20:19]=11)
    //RG_MEMPHYPLL_AB_BIC    (12C0H[18:16]=010)
    //RG_MEMPHYPLL_AB_BIR    (12C0H[27:24]=0010)
    //RG_MEMPHYPLL_AB_BP     (12C0H[15:12]=0001)
    //RG_MEMPHYPLL_AB_BR     (12C0H[23:21]=100)
    //has set above
    


    //delay 1us, 1ms here
    mcDELAY_us(1);

    //T2
    //(RG_MEMPHYPLL_A1_EN? not found) RG_MEMPHYPLL_A1_PWD (10C0H[1]) from 1 to 0
    //(RG_MEMPHYPLL_B2_EN? not found) RG_MEMPHYPLL_B2_PWD (14C0H[1]) from 1 to 0
    //(RG_MEMPHYPLL_AB_EN? not found) RG_MEMPHYPLL_AB_PWD (12BCH[15]) from 1 to 0
#if DRAMK_SIMULATION
    mcCLR_BIT(regddrphy_0c0, 1);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG48), regddrphy_0c0);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG48));
    mcCLR_BIT(u4value, 1);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG48), u4value);
#endif
#if 0
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG48));
    mcCLR_BIT(u4value, 1);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG48), u4value);
#endif
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47));
    mcCLR_BIT(u4value, 15);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47), u4value);


    //delay 100us, 1ms here
    mcDELAY_us(100);

    //check RGS_MEMPHYPLL_A1_VCOCAL_CPLT (10ecH[25]) & RGS_MEMPHYPLL_A1_VCO_STATE (10ecH[31:26])
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_A1_CFG0));
    fgvcocal_cplt = (U8) mcGET_FIELD(u4value, 0x02000000, 25);
    ucvco_state_assign = (U8) mcGET_FIELD(u4value, 0xfc000000, 26);
    mcSHOW_DBG_MSG2("MEMPHYPLL A1 VCOCAL CPLT FLAG = %d; VCO STATE = %d\n", fgvcocal_cplt, ucvco_state_assign);


#if 0
    //check RGS_MEMPHYPLL_B2_VCOCAL_CPLT (14ecH[25]) & RGS_MEMPHYPLL_B2_VCO_STATE (14ecH[31:26])
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_B2_CFG0));
    fgvcocal_cplt = (U8) mcGET_FIELD(u4value, 0x02000000, 25);
    ucvco_state = (U8) mcGET_FIELD(u4value, 0xfc000000, 26);
    mcSHOW_DBG_MSG("MEMPHYPLL B2 VCOCAL CPLT FLAG = %d; VCO STATE = %d\n", fgvcocal_cplt, ucvco_state);
#endif
    //check RGS_MEMPHYPLL_AB_VCOCAL_CPLT (12E8H[7]) & RGS_MEMPHYPLL_AB_VCO_STATE (12E8H[13:8])
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0));
    fgvcocal_cplt = (U8) mcGET_FIELD(u4value, 0x00000080, 7);
    ucvco_state_assign = (U8) mcGET_FIELD(u4value, 0x00003f00, 8);
    mcSHOW_DBG_MSG2("MEMPHYPLL AB VCOCAL CPLT FLAG = %d; VCO STATE = %d\n", fgvcocal_cplt, ucvco_state_assign);


    //T4
    //RG_MEMPHYPLL_A1_BIAS_RST (10C8H[10]) from 1 to 0
    //RG_MEMPHYPLL_B2_BIAS_RST (14C8H[10]) from 1 to 0
    //RG_MEMPHYPLL_AB_BIAS_RST (12C4H[26]) from 1 to 0 (suggestion from ACD/SP, after VCO band K cplt)
    //RG_MEMPHYPLL_A1_AUTOK_VCO (10C8H[17]) from 1 to 0 (not to do here from SP)
    //RG_MEMPHYPLL_B2_AUTOK_VCO (14C8H[17]) from 1 to 0 (not to do here from SP)
    //RG_MEMPHYPLL_AB_AUTOK_VCO (12C0H[1]) from 1 to 0 (not to do here from SP)
    //BIAS_RST 1->0

#if DRAMK_SIMULATION
    mcCLR_BIT(regddrphy_0c8, 10);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG50), regddrphy_0c8);
#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG50));
    mcCLR_BIT(u4value, 10);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG50), u4value);
#endif

#if 0
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG50));
    mcCLR_BIT(u4value, 10);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG50), u4value);
#endif
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG49));
    mcCLR_BIT(u4value, 26);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG49), u4value);


    //PHY Clock ready -->

#ifdef fcMEMCTLPLL_MUX_ISSUE
    // Special Note: Before change CPU AXI from XTAL to MEMCTLPLL, neet to
    // enable MEMCTLPLL first. No external path for MEMCTLPLL in XTAL mode
    // switch to internal path for MT5399
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46));
    mcCLR_BIT(u4value, 8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46), u4value);

    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG44));
    mcSET_FIELD(u4value, 0x01, 0x00007f00, 8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG44), u4value);
#endif

#ifndef fcMEMCTLPLL_PWD_ISSUE
    //MEMCTLPLL initialization sequence (refer to A60807 verification plan)
    //delay 1us, no need to delay, believe it is enough for MEMPLL
    //T1
    //RG_MEMCTLPLL_AUTOK_LOAD (12B4H[0]) from 0 to 1
    //MEMCTLPLL setting (BW)
    //RG_MEMCTLPLL_DIVEN (12B4H[30:28])
    //RG_MEMCTLPLL_BC     (12B4H[20:19]=11)
    //RG_MEMCTLPLL_BIC    (12B4H[18:16]=010)
    //RG_MEMCTLPLL_BIR    (12B4H[27:24]=0010)
    //RG_MEMCTLPLL_BP     (12B4H[15:12]=0001)
    //RG_MEMCTLPLL_BR     (12B4H[23:21]=100)
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG45), 0x029a1883);

    //delay 1us, 1ms here
    mcDELAY_us(1);

    //T2
    //RG_MEMCTLPLL_PWD (12B0H[15]) from 1 to 0
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG44));
    mcCLR_BIT(u4value, 15);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG44), u4value);

    //delay 100us, 1ms here
    mcDELAY_us(100);

    //T3

    //T4
    //RG_MEMCTLPLL_AUTOK_VCO (12B4H[1]) from 1 to 0
#else
    //Assign MEMPHYPLL_AB (for MT5863, it is A1) VCO state to MEMCTLPLL
#if DRAMK_SIMULATION
    mcSET_FIELD(regddrphy_2b4, ucvco_state_assign, 0x000000fc, 2);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG45), regddrphy_2b4);

#else
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG45));
    mcSET_FIELD(u4value, ucvco_state_assign, 0x000000fc, 2);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG45), u4value);
#endif
    mcDELAY_us(100);
#endif

    //check RGS_MEMCTLPLL_VCOCAL_CPLT (12E8H[17]) & RGS_MEMCTLPLL_VCO_STATE (12E8H[23:18])
    //u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46));
    //zhishang 3363 should modify   
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0));
	
    fgvcocal_cplt = (U8) mcGET_FIELD(u4value, 0x00020000, 17);
    ucvco_state = (U8) mcGET_FIELD(u4value, 0x00fc0000, 18);
    mcSHOW_DBG_MSG2("MEMCTLPLL VCOCAL CPLT FLAG = %d; VCO STATE = %d\n", fgvcocal_cplt, ucvco_state);


    //MEMCTLPLL Ready -->

    mcDELAY_us(100);

#ifdef fcMEMCTLPLL_MUX_ISSUE
    //change cpu axi clock source from XTAL to MEMCTLPLL
    //NOTE: this is CKGEN TOP register. Need to modify based on projects.
#if 0
    u4value = ucDram_Register_Read(0xf000d210);
    --> [JC] Need to update...
    mcCLR_BIT(u4value, 6);
    ucDram_Register_Write(0xf000d210, u4value);
    --> [JC] Need to update...
#else
    u4value = ucDram_Register_Read(0x700080cc);
    mcSET_BIT(u4value, 0);
    ucDram_Register_Write(0x700080cc, u4value);
#endif

    //R_CTLCLKMUX_CHG: Toggle to 1 to change clock source of the glitch-free clock mux (MT5399 E2 version)
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL));
    mcSET_BIT(u4value, 16);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL), u4value);
    mcDELAY_us(1);
    mcCLR_BIT(u4value, 16);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_JMETER_CTRL), u4value);

    mcDELAY_us(100);

    //change internal loop to external loop
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46));
    mcSET_BIT(u4value, 8);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46), u4value);

    mcDELAY_us(100);
#endif

#if SUPPORT_SSC  
    //enable ssc
#if(!SUPPORT_MEMPLL_FRACT)
    if (IS_DDR_DMSSON())  // if supprt   use SUPPORT_MEMPLL_FRACT
#endif
    {
        mcSHOW_DBG_MSG2("Support SSC \r\n");
        //Enable SSC flow
        //Disable all first
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
        u4value = u4value & 0xffff7f0f;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG43));
        mcCLR_BIT(u4value, 29);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG43), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG41));
        mcCLR_BIT(u4value, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG41), u4value);

        //RG_MEMPLL_DDS_PWDB (12A8H[15]) from 0 to 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
        mcSET_BIT(u4value, 15);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);

        mcDELAY_us(100);

        //RG_MEMPLL_DDS_RSTB (12A8H[6]) from 0 to 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
        mcSET_BIT(u4value, 6);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);

        mcDELAY_us(100);

        //RG_MEMPLL_PCW_NCPO_CHG (12A8H[5]) from 0 to 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
        mcSET_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);

        mcDELAY_us(100);

        //RG_MEMPLL_FIFO_START_MAN (12A8H[4]) from 0 to 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
        mcSET_BIT(u4value, 4);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);

        mcDELAY_us(100);

        //RG_MEMPLL_NCPO_EN (12A8H[7]) from 0 to 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42));
        mcSET_BIT(u4value, 7);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG42), u4value);

        mcDELAY_us(100);

        //RG_MEMPLL_SSC_EN (12ACH[29]) from 0 to 1 (DDS SSC enable). 20us after RG_MEMPLL_BIAS_RST from FNPLL AN (It is enough here)
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG43));
        mcSET_BIT(u4value, 29);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG43), u4value);

        mcDELAY_us(100);

        //RG_MEMPLL_DDSEN (12A4H[8]) from 0 to 1 (DDS Feedback Enable)
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG41));
        mcSET_BIT(u4value, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG41), u4value);
    }
#endif

    mcDELAY_us(100);

    //RX RESET for channel A & B, R_DMPHYRST (*0F0H[28]) to 1 for at least 100ns and then set it back to 0
    //Async FIFO RESET, R_DMSYNCRST (1B44H[6]) to 1 for at least 100ns and then set it back to 0
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), 0x10000000);
#if 0
    ucDram_Register_Write(DRAM_CHB_BASE | 0x0f0, 0x10000000);
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), 0x00080340);
    //delay 100ns, 1ms here
    mcDELAY_us(1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), 0x00000000);
#if 0
    ucDram_Register_Write(DRAM_CHB_BASE | 0x0f0, 0x00000000);
#endif
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), 0x00080300);

    UNUSED(fgvcocal_cplt);
    UNUSED(ucvco_state);

    // log example
    /*
    Real frequency from PLL is 918 MHz
    MEMPLL VCOCAL CPLT FLAG = 1; VCO STATE = 13
    MEMPHYPLL A1 VCOCAL CPLT FLAG = 1; VCO STATE = 11
    MEMPHYPLL B2 VCOCAL CPLT FLAG = 1; VCO STATE = 11
    MEMPHYPLL AB VCOCAL CPLT FLAG = 1; VCO STATE = 11
    MEMCTLPLL VCOCAL CPLT FLAG = 1; VCO STATE = 10
    */

#if fcSET_AXI_CLK_USE_CLK_DRAM  //zhishang 3363 here may should modify
    /*[important]!!! change axi_clk to clk_dram after mempll enable, otherwise can't BIM access dram */
#ifdef FOR_MT3363
    mcDELAY_us(1);
    u4value = ucDram_Register_Read(0xf0000010);
    mcSET_BIT(u4value, 27);
    ucDram_Register_Write(0xf0000010, u4value);
    mcDELAY_us(1);
#else
    mcDELAY_us(1);
    u4value = ucDram_Register_Read(0x700080CC);
    mcSET_BIT(u4value, 0);
    ucDram_Register_Write(0x700080CC, u4value);
    mcDELAY_us(1);
#endif
#endif

}

void ResetDemuxLocalArbiter()
{
	U32 u4value = 0;

    #if 1
    /*disable agent0 agent1*/
    u4value = ucDram_Register_Read(0xF0052004); 
	mcCLR_BIT(u4value, 24);
	mcCLR_BIT(u4value, 25);
	mcCLR_BIT(u4value, 26);
	mcCLR_BIT(u4value, 27);
	mcCLR_BIT(u4value, 28);
	mcCLR_BIT(u4value, 29);
	mcCLR_BIT(u4value, 30);
	mcCLR_BIT(u4value, 31);
	ucDram_Register_Write(0xF0052004, u4value);	
	if (DEBUG_LOG == 1)
	    Printf("\nStep 1: Disable 0x52004 bit 24-31\n");

	u4value = ucDram_Register_Read(0xF0052104);
	mcCLR_BIT(u4value, 24);
	mcCLR_BIT(u4value, 25);
	mcCLR_BIT(u4value, 26);
	mcCLR_BIT(u4value, 27);
	mcCLR_BIT(u4value, 28);
	mcCLR_BIT(u4value, 29);
	mcCLR_BIT(u4value, 30);
	mcCLR_BIT(u4value, 31);
	ucDram_Register_Write(0xF0052104, u4value);
	if (DEBUG_LOG == 1)
	    Printf("\nStep 2: Disable 0x52104 bit 24-31\n");
	#endif
	
	/*Loacal arbiter reset*/   	
    u4value = ucDram_Register_Read(0xF00000BC);
    mcCLR_BIT(u4value, 7);
    ucDram_Register_Write(0xF00000BC, u4value);       
    if (DEBUG_LOG == 1)
	    Printf("\nStep 3: Reset Demux logic 0xBC[7]=0\n");
   	
    u4value = ucDram_Register_Read(0xF00000A0);
    /*Set 7 - 11 bit*/
    mcSET_BIT(u4value, 7);
    mcSET_BIT(u4value, 8);
    mcSET_BIT(u4value, 9);
    mcSET_BIT(u4value, 10);
    mcSET_BIT(u4value, 11);
    ucDram_Register_Write(0xF00000A0, u4value);
    mcDELAY_us(1000);
    if (DEBUG_LOG == 1)
	    Printf("\nStep 4: Disable Clock logic 0xA0[7-11]=0\n");
    
    u4value = ucDram_Register_Read(0xF00000BC); 
    mcSET_BIT(u4value, 7);  
    ucDram_Register_Write(0xF00000BC, u4value);
    if (DEBUG_LOG == 1)
	    Printf("\nStep 5: Reset Demux logic 0xBC[7]=1\n");

	u4value = ucDram_Register_Read(0xF0006000);
	mcSET_BIT(u4value, 31);
	ucDram_Register_Write(0xF0006000, u4value);

	u4value = ucDram_Register_Read(0xF00070E4);
	mcSET_BIT(u4value, 2);
	ucDram_Register_Write(0xF00070E4, u4value);

	u4value = ucDram_Register_Read(0xF00000A0);
	mcSET_BIT(u4value, 1);
	ucDram_Register_Write(0xF00000A0, u4value);

	u4value = ucDram_Register_Read(0xF00000BC);
	mcSET_BIT(u4value, 1);
	ucDram_Register_Write(0xF00000BC, u4value);
	mcDELAY_us(10);	

	u4value = ucDram_Register_Read(0xF00070E4);
    mcCLR_BIT(u4value, 2);
    ucDram_Register_Write(0xF00070E4, u4value);

    mcDELAY_us(10);	

	u4value = ucDram_Register_Read(0xF00000A0);
	mcCLR_BIT(u4value, 1);
    ucDram_Register_Write(0xF00000A0, u4value);

	u4value = ucDram_Register_Read(0xF00000BC);
	mcCLR_BIT(u4value, 1);
	ucDram_Register_Write(0xF00000BC, u4value);
	
    u4value = ucDram_Register_Read(0xF0007008);
	mcSET_BIT(u4value, 4);
	ucDram_Register_Write(0xF0007008, u4value);

	u4value = ucDram_Register_Read(0xF0007004);
	mcSET_BIT(u4value, 26);
	ucDram_Register_Write(0xF0007004, u4value);

	while ((ucDram_Register_Read(0xF00073B8) & 0x10000) != 0x10000);
	mcDELAY_us(10);
}


//-------------------------------------------------------------------------
/** DramcInit
 *  DRAMC Initialization.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------

void DramcInit(void)
{
    UINT32 u4BLDramPCB = 0, u4value = 0;

    //ResetDemuxLocalArbiter();
    
    u4BLDramPCB = BIM_READ32(REG_RW_GPRDW5); // load dram config
    u4BLDramPCB = DDR_GET_VERSION_PCB_CONFIG(u4BLDramPCB);


    
    // This function is implemented based on DE's bring up flow for DRAMC
 #if SUPPORT_AYSMMETRIC
     u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0));
      mcSET_FIELD(u4value, 1, 0x00001000, 12);
     ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0), u4value); //CS0 is also applied to CS1
     u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0));  
#endif

    // DDR3 in channel A
    //========dramc_init============
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), 0x0000110d);

    //DDR3 SBS pinmux should be set to 00 (reg0xd8[31:30])

	//mcSHOW_DBG_MSG("u4BLDramPCB = %d\n",u4BLDramPCB);

#if 0      //mtk40739 this may change for project
	if (u4BLDramPCB == DRAM_PCB_VERSION_MT8659P1V1 || 
		u4BLDramPCB == DRAM_PCB_VERSION_MT8507P1V1 ||
		u4BLDramPCB == DRAM_PCB_VERSION_MT8639P1V1 || 
		u4BLDramPCB == DRAM_PCB_VERSION_MT8639P2V1)
	{
		ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY), 0xC0100900); //for 13*13
	}
	else
#endif		
	{
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY), 0x00100900); //for 15*15
	}
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), 0x000000b2);
    // GDDR3RST must keep HIGH > 500us (1ms here)
    mcDELAY_us(500);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CLK1DELAY), 0x00000001);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_IOCTL), 0x00000000);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSIEN), 0x80000000);

    //A60806: 0x0dc=0x83000000
    //[23:12] DQS1 input range control, 1 hot encoding
    //[11:0]  DQS0 input range control, 1 hot encoding
    //gating window coarse tune default value
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSCTL0), 0x83008008);

    //A60806: 0x0e0=0x12000000
    //[23:12] DQS3 input range control, 1 hot encoding
    //[11:0]  DQS2 input range control, 1 hot encoding
    //[26:24] DQS input range control by M_CK
    //gating window coarse tune default value
    //by KS
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSCTL1), 0x10008008);

    //A60806: 0x0f0=0x80000000
    //[31] DQ 4-bit multiplex for DDR3
    //no 4-bit swap for A60807
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
#ifdef DDR_CHA_4BIT_SWAP
        //R_DMDQ4BMUX=1・b1
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), 0x80000000);
        //R_DMDQMSWAP=1・b1(MT5399 only)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), 0x01000000);
#else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), 0x00000000);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), 0x01000000);
#endif
#ifdef DDR_CONFIG_CSD
        //CS0 is also applied to CS1
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0));
        mcSET_FIELD(u4value, 1, 0x00001000, 12);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PERFCTL0), u4value);
#endif
    }
#if 0
    else
    {
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), 0x00000000);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), 0x01000000);
    }
#endif

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ARBCTL0), 0x00000080);

    //A60806: reg_duty_free_sel_cmd (not set)
    //A60807: DRAM CLOCK ENABLE CONTROL, register change
    //Useless for A60807, no connection from DRAMC to DDRPHY
    //#proc_write 0x2000${DRAMC_ADDR}130 0x30000000

    //A60806: dly_sel_mux2to1 (not set)
    //A60807: INPUT DQS GATING CONTROL, register change
    //Useless for A60807
    //#proc_write 0x2000${DRAMC_ADDR}124 0x80000033

    //A60806: 0x094=0x80000000
    //DQS INPUT RANGE FINE TUNER
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSIEN), 0x40404040);

    //A60806: chA, 0x1c0=0x0f000000 (default=0x00000000)
    //DQS CAL CONTROL 0
    //[31] DQS strobe calibration enable
    //[28:24] RA output delay chain setting for bit14
    //[15] DQS strobe calibration high-limit enable
    //[14:8] DQS strobe calibration high-limit value
    //[7] DQS strobe calibration low-limit enable
    //[6:0] DQS strobe calibration low-limit value
    //Useless for A60807?
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQSCAL0), 0x8000c8b8);

    //new register, Write Leveling
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV), 0x00000000);

    //new register, DQ/DQM/DQS selph
    //2012/10/03, for 2133MHz
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_SELPH), 0x36dc07c0);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_SELPH), 0x349b07c0);

    //new register, (blank)
    //DC balance by benson
    if (IS_DDR_DCBALANCEON())
    {
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), 0x00003c02);
    }
    else
    {
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), 0x00000c02);
    }
    //caustion, delay 100us to charge the caps after enable dc-balance
    mcDELAY_us(100);

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DLLCONF), 0xf1200f01);

    //A60806: 0x1e0=0x88000000
    //[31] DRAM address decode
    //[30] Select IO O1 as output
    //[29] DDR mode for A[14] pin (LPDDR2 DDR command rate)
    //[26] Fast IO output enable
    //For A60807, bit 31 is 1, bit 30 is useless, bit 29 is useless, bit 26 is useless
    //Will set below
    //#proc_write 0x2000${DRAMC_ADDR}1e0 0x6c000000

    //A60806: 0x110=0b051111
    //[27] ?
    //[26:24] Rank address selection
    //[20] ?
    //[19:18] cross rank timing W2W
    //[11] ?
    //[7] Per-bank refresh enable for LPDDR2?
    //[2:0] Multi-rank mode support? Set to non-zero for multi-rank
    //Useless for A60807
    //#proc_write 0x2000${DRAMC_ADDR}110 0x00111990

    //A60806: 0x158=0x0ff00ff0
    //not in register map
    //may be 4-bit swap. TBD
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_158), 0x00000000);

    //========dram_init_start==========
    //[15] CKE function enabling
    //[10] BL4
    //[0] DM64BITEN
    //[15] -> When set to MRS, make sure that we don't toggle CKE.
    //2012/11/06, for suspend / resume
    if (IS_DDR_SUSPENDSTATE())     //zhishang 3363 here may be should modify
    {
		//in suspend state, self refresh mode		 
		  ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), 0xf47402c0); 	  

    }
    else
    {
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), 0xf07402c0);
    }

    //[2] CKE always ON
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), 0x000000b6);

    //set mode registers, delay is to wait 50 XTAL cycles, 1ms here
    //MR2
    //A60806: 4018
    //[5:3] CWL (CAS Write Latency)
    //depend on data rate
    //2012/10/03, for 2133MHz, CWL=101 (10)
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), DEFAULT_MR2_VALUE);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00004018);

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000001);
    mcDELAY_us(1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000000);

    //MR3
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00006000);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000001);
    mcDELAY_us(1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000000);

    //MR1
    //60ohm, 2012/10/02, by KS
    //for dynamic ODT: RTT_Nom = OFF, MR1[9,6,2]=000, 2013/1/3
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), DEFAULT_MR1_VALUE);

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000001);
    mcDELAY_us(1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000000);

    //MR0
    //A60806: 0d71
    //[11:9] WR
    //[6:4] CAS Latency
    //[2] CL
    //depend on data rate
    //only for 2400MHz, CL = 0101 (14)
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)    //zhishang 3363 here may should modify
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00000f15);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00000d71);

    //ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00000f25);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000001);
    mcDELAY_us(1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000000);

    //Enable ZQ calibration (A10=1 ZQCL, A10=0 ZQCS)
    //0x088[10] represents A10
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00000400);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000010);
    mcDELAY_us(1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000000);

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00001100);

    //[2] CKE control by HW
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), 0x000000b2);

    //A60806: 0x1e0=0x88000000
    //[31] DRAM address decode
    //[30] Select IO O1 as output
    //[29] DDR mode for A[14] pin (LPDDR2 DDR command rate)
    //[26] Fast IO output enable
    //For A60807, bit 31 is 0, bit 30 is useless, bit 29 is useless, bit 26 is useless
    //2012/09/27, for test chip, bit 31 is 0 for TE/TA/UART; for SoC it is 1
    //2012/10/03, the same as A60806, for TA&UART b'31=1; for TE b'31=0
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), 0x88000000);

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x0000ffff);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000020);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), 0x00000000);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DRAMC_PD_CTRL), 0x10622842);

    //For A60807, disable power down function, CKE will keep high (b15)
    //So we don't enable b15 here. Ignored. b10 & b0 are set in dramc_config()
    //#proc_write 0x2000${DRAMC_ADDR}004 0xf07486e3

    //========dram_init_end============
    //A60806: 0xff000000
    //[31:28] CS1 signal output delay
    //[27:24] DRAM clock signal output delay
    //based on simulation
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL1), 0x00000000);

    //A60806: 0x55fc47eb
    //Timing settings
    //See register map for detail
    //depend on data rate. Excel table for AC timing calculation provided by Justin
    //2012/10/03, for 2133MHz
#if 0
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)  //zhishang 3363 here may should modify
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM0), 0x66dd48cc);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM0), 0x44bc48ab);
#endif
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM0), 0x55dc489b);

    //A60806: 0x28820400
    //[19:16] tRFC Timing setting
    //[3:0] Test loop number of test agent2
    //depend on data rate. Excel table for AC timing calculation provided by Justin
    //2012/10/03, for 2133MHz
#if 0 
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), 0x28800401);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), 0x288A0401);
#endif
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), 0x28800401);

    //A60806: 0x00000c20
    //[15:8] tRFCPB Timing setting
    //[7:4] tRFC Timing setting for bit 7 ~ 4
    //depend on data rate. Excel table for AC timing calculation provided by Justin
    //2012/10/03, for 2133MHz
#if 0
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), 0x00000690);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), 0x00000670);
#endif

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_ACTIM1), 0x00000660);
    //A60806: 0x03047960
    //[26:24] Refresh threshold value for promoting refresh request to high-priority
    //[7:0] Refresh period = (REFCNT * 16) DRAMC clock cycles
    //depend on capacity/size of DRAM. Refresh period spec.
    //2012/10/03, for 2133MHz
#if 0 
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), 0x000479d0);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), 0x000479b0);
#endif
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), 0x00047958); // update DDR tREFI

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL2), 0x00000000);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL7), 0xedcb000f);

    //A60806: 0x27010000
    //[30:28] tXP Timing setting
    //depend on data rate. Excel table for AC timing calculation provided by Justin
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MISCTL0), 0x37010000);

    //
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_RKCFG), 0x00051100);

    //enable scramble bit[28] and wdatkey=bit{[31:30], [27:26], [23:22],[19:18]}=0x56
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_OCDK), 0x54480000);


    //========DMARB_init_start===========
    // This register depends on projects
    //OK. DRAM arbiter
    //CHSEL=0x5, channel A 1Gbytes
#if 0 
    if (!IS_DRAM_CHANNELB_ACTIVE())   //zhishang 3363 here may should modify
#endif
    {
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), 0xe0501f00);
    }
#if 0
    else
    {
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), 0xc050ef00);
    }
#endif


    //Wait until tZQinit (max(512nCK,640ns)) is satisfied
    mcDELAY_us(1);
}

//-------------------------------------------------------------------------
/** DramcDqDriving
 *  DRAMC DQ driving settings.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param ocd_pvalue       (U8): OCD P value
 *  @param ocd_nvalue       (U8): OCD N value
 *  @param odt_pvalue       (U8): ODT P value
 *  @param odt_nvalue       (U8): ODT N value
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcDqDriving(U8 ocd_pvalue, U8 ocd_nvalue, U8 odt_pvalue, U8 odt_nvalue)
{
    U32 u4value;

#if 0 //zhishang 3363 here may should modify
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        //A1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0), u4value);

        //A2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG0));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG0), u4value);
    }
#if 0 
    else
    {
        //B1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG21));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG21), u4value);

        //B2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0), u4value);
    }

#endif
}

//-------------------------------------------------------------------------
/** DramcDqsDriving
 *  DRAMC DQS driving settings.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param ocd_pvalue       (U8): OCD P value
 *  @param ocd_nvalue       (U8): OCD N value
 *  @param odt_pvalue       (U8): ODT P value
 *  @param odt_nvalue       (U8): ODT N value
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcDqsDriving(U8 ocd_pvalue, U8 ocd_nvalue, U8 odt_pvalue, U8 odt_nvalue)
{
    U32 u4value;

#if 0  //zhishang 3363 here may should modify
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
	{ 
        //A1, DQS0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG6));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG6), u4value);

        //A1, DQS1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), u4value);

        //A2, DQS2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG6));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG6), u4value);

        //A2, DQS3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8), u4value);

    }
#if 0 
    else
    {
        //B1, DQS0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG26));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG26), u4value);

        //B1, DQS1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28), u4value);

        //B2, DQS2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG6));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG6), u4value);

        //B2, DQS3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), u4value);
    }
#endif
}

//-------------------------------------------------------------------------
/** DramcDqmDriving
 *  DRAMC DQM driving settings.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param ocd_pvalue       (U8): OCD P value
 *  @param ocd_nvalue       (U8): OCD N value
 *  @param odt_pvalue       (U8): ODT P value
 *  @param odt_nvalue       (U8): ODT N value
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcDqmDriving(U8 ocd_pvalue, U8 ocd_nvalue, U8 odt_pvalue, U8 odt_nvalue)
{
    U32 u4value;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
	{
        //A1, DQM0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG9));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG9), u4value);

        //A1, DQM1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11), u4value);

        //A2, DQM2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG9));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG9), u4value);

        //A2, DQM3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG10));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG10), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG11));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG11), u4value);
    }
#if 0 
    else
    {
        //B1, DQM0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG29));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG29), u4value);

        //B1, DQM1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG30));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG30), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG31));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG31), u4value);

        //B2, DQM2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG9));
        mcSET_FIELD(u4value, odt_pvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_pvalue, 0x0000ff00, 8);
        mcSET_FIELD(u4value, odt_nvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG9), u4value);

        //B2, DQM3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG10));
        mcSET_FIELD(u4value, odt_nvalue, 0x000000ff, 0);
        mcSET_FIELD(u4value, ocd_nvalue, 0x0000ff00, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG10), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG11));
        mcSET_FIELD(u4value, odt_pvalue, 0x00ff0000, 16);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG11), u4value);
    }

#endif
}

//-------------------------------------------------------------------------
/** DramcClkDriving
 *  DRAMC CLK driving settings.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param ocd_pvalue       (U8): OCD P value
 *  @param ocd_nvalue       (U8): OCD N value
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcClkDriving(U8 ocd_pvalue, U8 ocd_nvalue)
{
    U32 u4value;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
	{
        //A1, CLK
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19));
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG20));
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG20), u4value);

        //CMDACLK
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG46));
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG46), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG47));
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG47), u4value);

        //A2, CLK
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19));
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20));
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20), u4value);
    }
#if 0 
    else
    {
        //B1, CLK
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39));
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40));
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40), u4value);

        //B2, CLK
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19));
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG20));
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG20), u4value);

        //CMDBCLK
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG46));
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG46), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG47));
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG47), u4value);
    }

#endif
}

//-------------------------------------------------------------------------
/** DramcCaDriving
 *  DRAMC CA driving settings.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param ocd_pvalue       (U8): OCD P value
 *  @param ocd_nvalue       (U8): OCD N value
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcCaDriving(U8 ocd_pvalue, U8 ocd_nvalue)
{
    U8 i;
    U32 u4value, u4addr1, u4addr2;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4addr1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG20);
        u4addr2 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG21);
    }
#if 0 
    else
    {
        u4addr1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG20);
        u4addr2 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG21);
    }
#endif

    // ?RA0~14, ?RCKE, ?RBA0~2, ?RA15, ?RCAS, ?RRAS, ?RODT, ?RCS, ?RRESET, ?RWE
    for (i=0; i<=25; i++)
    {
        u4value = ucDram_Register_Read(u4addr1);
        mcSET_FIELD(u4value, ocd_nvalue, 0x000000ff, 0);
        ucDram_Register_Write(u4addr1, u4value);

        u4value = ucDram_Register_Read(u4addr2);
        mcSET_FIELD(u4value, ocd_pvalue, 0xff000000, 24);
        ucDram_Register_Write(u4addr2, u4value);

        u4addr1 +=4;
        u4addr2 +=4;
    }

#ifdef DDR_CONFIG_CSD
    u4addr1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG60);
    u4value = ucDram_Register_Read(u4addr1);
    mcSET_FIELD(u4value, ocd_nvalue, 0xff000000, 24);
    mcSET_FIELD(u4value, ocd_pvalue, 0x00ff0000, 16);
	mcSHOW_DBG_MSG2("CSD driving:%x\r\n", u4value);
    ucDram_Register_Write(u4addr1, u4value);
#endif
}


//-------------------------------------------------------------------------
/** DramcEngine1
 *  start the self test engine inside dramc to test dram w/r.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  test2_0         (U32): 16bits,set pattern1 [31:24] and set pattern0 [23:16].
 *  @param  test2_1         (U32): 28bits,base address[27:0].
 *  @param  test2_2         (U32): 28bits,offset address[27:0].
 *  @param  loopforever     (S16):  0 read\write one time ,then exit
 *                                 >0 enable eingie1, after "loopforever" second ,write log and exit
 *                                 -1 loop forever to read\write, every "period" seconds ,check result ,only when we find error,write log and exit
 *                                 -2 loop forever to read\write, every "period" seconds ,write log ,only when we find error,write log and exit
 *                                 -3 just enable loop forever ,then exit
 *  @param period           (U8):  it is valid only when loopforever <0; period should greater than 0
 *  @retval status          (U32): return the value of DM_CMP_ERR  ,0  is ok ,others mean  error
 */
//-------------------------------------------------------------------------
/*
U32 DramcEngine1(U32 test2_0, U32 test2_1, U32 test2_2, S16 loopforever, U8 period)
{
    // This function may not need to be modified unless test engine-1 design has changed

    U8 ucengine_status;
    U8 ucnumber;
    U32 u4value, u4result = 0xffffffff;
    U8 ucloop_count = 0;

    // we get the status
    // loopforever    period    status    mean
    //     0             x         1       read\write one time ,then exit ,don't write log
    //    >0             x         2       read\write in a loop,after "loopforever" seconds ,disable it ,return the R\W status
    //    -1            >0         3       read\write in a loop,every "period" seconds ,check result ,only when we find error,write log and exit
    //    -2            >0         4       read\write in a loop,every "period" seconds ,write log ,only when we find error,write log and exit
    //    -3             x         5       just enable loop forever , then exit (so we should disable engine1 outside the function)
    if (loopforever == 0)
    {
        ucengine_status = 1;
    }
    else if (loopforever > 0)
    {
        ucengine_status = 2;
    }
    else if (loopforever == -1)
    {
        if (period > 0)
        {
            ucengine_status = 3;
        }
        else
        {
            mcSHOW_ERROR_CHIP_DisplayString(("parameter 'status' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -2)
    {
        if (period > 0)
        {
            ucengine_status = 4;
        }
        else
        {
            mcSHOW_ERROR_CHIP_DisplayString(("parameter 'status' should be equal or greater than 0\n"));
            return u4result;
        }
    }
    else if (loopforever == -3)
    {
        ucengine_status = 5;
    }
    else
    {
        mcSHOW_ERROR_CHIP_DisplayString(("wrong parameter!\n"));
        mcSHOW_ERROR_CHIP_DisplayString(("loopforever    period    status    mean \n"));
        mcSHOW_ERROR_CHIP_DisplayString(("      0                x           1         read/write one time ,then exit ,don't write log\n"));
        mcSHOW_ERROR_CHIP_DisplayString(("    >0                x           2         read/write in a loop,after [loopforever] seconds ,disable it ,return the R/W status\n"));
        mcSHOW_ERROR_CHIP_DisplayString(("    -1              >0           3         read/write in a loop,every [period] seconds ,check result ,only when we find error,write log and exit\n"));
        mcSHOW_ERROR_CHIP_DisplayString(("    -2              >0           4         read/write in a loop,every [period] seconds ,write log ,only when we find error,write log and exit\n"));
        mcSHOW_ERROR_CHIP_DisplayString(("    -3                x           5         just enable loop forever , then exit (so we should disable engine1 outside the function)\n"));
        return u4result;
    }

    // set ADRDECEN=0,address decode not by DRAMC
    //2012/10/03, the same as A60806, for TA&UART b'31=1; for TE b'31=0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2));
    mcCLR_BIT(u4value, POS_LPDDR2_ADRDECEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), u4value);

    // step
    // 1.select loop forever or not
    // 2.set pattern, base address,offset address
    // 3.enable test1
    // 4.run different code according status
    // 5.diable test1
    // 6.return DM_CMP_ERR

    if (ucengine_status == 4)
    {
        mcSHOW_DBG_MSG(("============================================\n"));
        mcSHOW_DBG_MSG(("enable test egine1 loop forever\n"));
        mcSHOW_DBG_MSG(("============================================\n"));
        ucnumber = 1;
    }

    // 1.
    if (loopforever != 0)
    {
        // enable infinite loop
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
        mcSET_BIT(u4value, POS_CONF1_TESTLP);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);
    }
    else
    {
        // disable infinite loop
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
        mcCLR_BIT(u4value, POS_CONF1_TESTLP);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);
    }
    // 2.
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_0), test2_0);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_1), test2_1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_2), test2_2);
    // 3.
    // enable test engine 1 (first write and then read)
    // disable it before enable ,DM_CMP_ERR may not be 0,because may be loopforever and don't disable it before
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcCLR_BIT(u4value, POS_CONF2_TEST1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcSET_BIT(u4value, POS_CONF2_TEST1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);
    // 4.
    if (ucengine_status == 1)
    {
        // read data compare ready check
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        // infinite loop??? check DE about the time???
        ucloop_count = 0;
        while(mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0)
        {
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
            mcDELAY_us(CMP_CPT_POLLING_PERIOD);
            ucloop_count++;
            if (ucloop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
                mcSHOW_ERR_MSG(("TESTRPT_DM_CMP_CPT polling timeout\n"));
                break;
            }
        }

        // delay 10ns after ready check from DE suggestion (1ms here)
        mcDELAY_us(1);

        // save  DM_CMP_ERR, 0 is ok ,others are fail,disable test engine 1
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);
        mcSHOW_DBG_MSG(("0x3fc = %d\n", u4value));
    }
    else if (ucengine_status == 2)
    {
        // wait "loopforever" seconds
        mcDELAY_us(loopforever*1000);
        // get result, no need to check read data compare ready???
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);
    }
    else if (ucengine_status == 3)
    {
        while(1)
        {
            // wait "period" seconds
            mcDELAY_us(period*1000);
            // get result
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
            u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);
            if (u4result == 0)
            {
                // pass, continue to check
                continue;
            }
            // some bit error
            // write log
            mcSHOW_DBG_MSG(("%d#    CMP_ERR = 0x%8x\n", ucnumber, u4result));
            break;
        }
    }
    else if (ucengine_status == 4)
    {
        while(1)
        {
            // wait "period" seconds
            mcDELAY_us(period*1000);
            // get result
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
            u4result = mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_ERR);

            // write log
            mcSHOW_DBG_MSG(("%d#    CMP_ERR = 0x%8x\n", ucnumber, u4result));

            if (u4result == 0)
            {
                // pass, continue to check
                continue;
            }
            // some bit error
            break;
        }
    }
    else if (ucengine_status == 5)
    {
        // loopforever is  enable ahead ,we just exit this function
        return 0;
    }
    else
    {
    }

    // 5. disable engine1
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcCLR_BIT(u4value, POS_CONF2_TEST1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

    // 6.
    // set ADRDECEN to 1
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2));
    mcSET_BIT(u4value, POS_LPDDR2_ADRDECEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_LPDDR2), u4value);

    UNUSED(ucnumber);

    return u4result;
}
*/
//-------------------------------------------------------------------------
/** DramcEngine2
 *  start the self test engine 2 inside dramc to test dram w/r.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  wr              (DRAM_TE_OP_T): TE operation
 *  @param  test2_0         (U32): 16bits,set pattern1 [31:24] and set pattern0 [23:16].
 *  @param  test2_1         (U32): 28bits,base address[27:0].
 *  @param  test2_2         (U32): 28bits,offset address[27:0]. (unit is 16-byte, i.e: 0x100 is 0x1000).
 *  @param  loopforever     (S16): 0    read\write one time ,then exit
 *                                >0 enable eingie2, after "loopforever" second ,write log and exit
 *                                -1 loop forever to read\write, every "period" seconds ,check result ,only when we find error,write log and exit
 *                                -2 loop forever to read\write, every "period" seconds ,write log ,only when we find error,write log and exit
 *                                -3 just enable loop forever ,then exit
 *  @param period           (U8):  it is valid only when loopforever <0; period should greater than 0
 *  @param log2loopcount    (U8): test loop number of test agent2 loop number =2^(log2loopcount) ,0 one time
 *  @retval status          (U32): return the value of DM_CMP_ERR  ,0  is ok ,others mean  error
 */
//-------------------------------------------------------------------------

#if 1
U32 DramcEngine2(DRAM_TE_OP_T wr, U32 test2_0, U32 test2_1, U32 test2_2)
{

    U8 ucloop_count = 0;
    U32 u4value, u4result = 0x0;

    // disable self test engine1 and self test engine2
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcCLR_MASK(u4value, MASK_CONF2_TE12_ENABLE);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

    // 1.set pattern ,base address ,offset address
    // 2.select  ISI pattern or audio pattern
    // 3.set loop number
    // 4.enable read or write
    // 5.loop to check DM_CMP_CPT
    // 6.return CMP_ERR
    // currently only implement ucengine_status = 1, others are left for future extension

    // 1
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_0), test2_0);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_1), test2_1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_2), test2_2);

    // 2 & 3
    //use audio pattern

    // set AUDINIT=0x11 AUDINC=0x0d AUDBITINV=1 AUDMODE=1
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4));
    mcSET_FIELD(u4value, 0x00000011, MASK_TEST2_4_TESTAUDINIT, POS_TEST2_4_TESTAUDINIT);
#if defined(DRAM_LOAD_BOARD) || defined(__MODEL_slt__) || defined(DRAM_WRITE_READ_LOOP_AFTER_CALIBRATION)
    mcSET_BIT(u4value, POS_TEST2_4_TEST2DISSCRAM);
#else
    mcCLR_BIT(u4value, POS_TEST2_4_TEST2DISSCRAM);
#endif
    mcSET_FIELD(u4value, 0x0000000d, MASK_TEST2_4_TESTAUDINC, POS_TEST2_4_TESTAUDINC);
    mcSET_BIT(u4value, POS_TEST2_4_TESTAUDBITINV);
    mcSET_BIT(u4value, POS_TEST2_4_TESTAUDMODE);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4value);

    // set addr 0x044 [7] to 1 ,select audio pattern
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3));
    mcSET_BIT(u4value, POS_TEST2_3_TESTAUDPAT);
    mcSET_FIELD(u4value, 0, MASK_TEST2_3_TESTCNT, POS_TEST2_3_TESTCNT);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4value);

    // 4
    if (wr == TE_OP_READ_CHECK)
    {
        // enable read, 0x008[31:29]
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
        mcSET_FIELD(u4value, (U32) 2, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

        //if audio pattern, enable read only (disable write after read), AUDMODE=0x48[15]=0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4));
        mcCLR_BIT(u4value, POS_TEST2_4_TESTAUDMODE);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4value);
    }
    else if (wr == TE_OP_WRITE_READ_CHECK)
    {
        // enable write
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
        mcSET_FIELD(u4value, (U32) 4, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

        // read data compare ready check
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        ucloop_count = 0;
        while (mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0)
        {
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
            mcDELAY_us(CMP_CPT_POLLING_PERIOD);
            ucloop_count++;
            if (ucloop_count > MAX_CMP_CPT_WAIT_LOOP)
            {
                mcSHOW_ERROR_CHIP_DisplayString("TESTRPT_DM_CMP_CPT polling timeout\n");

                break;
            }
        }

        // disable write
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
        mcSET_FIELD(u4value, (U32) 0, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

        // enable read
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
        mcSET_FIELD(u4value, (U32) 2, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);
    }

    // 5
    // read data compare ready check
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
    ucloop_count = 0;
    while (mcCHK_BIT1(u4value, POS_TESTRPT_DM_CMP_CPT) == 0)
    {
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        mcDELAY_us(CMP_CPT_POLLING_PERIOD);
        ucloop_count++;
        if (ucloop_count > MAX_CMP_CPT_WAIT_LOOP)
        {
            mcSHOW_ERROR_CHIP_DisplayString("TESTRPT_DM_CMP_CPT polling timeout\n");			
			
            break;
        }
    }

    // delay 10ns after ready check from DE suggestion (1ms here)
    mcDELAY_us(1);

    // 6
    // return CMP_ERR, 0 is ok ,others are fail,diable test2w or test2r
    // get result
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CMP_ERR));
    // or all result
    u4result |= u4value;
    // disable read
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcCLR_MASK(u4value, MASK_CONF2_TE12_ENABLE);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4value);

    if (TCMGET_DATA_WIDTH() == DATA_WIDTH_16BIT)
    {
        u4result = u4result & 0x0000ffff;
    }
 

    return u4result;
}


#ifdef DRAM_RX_DATLAT_CAL
//-------------------------------------------------------------------------
/** DramcRxdatlatCal
 *  scan the pass range of DATLAT for DDRPHY read data window.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcRxdatlatCal(U32 u4CalStartAddr)
//DRAM_STATUS_T DramcRxdatlatCal(DRAMC_CTX_T *p, U8 apply, U16 low_freq_wr)
{
    U8 ii;
    U32 u4prv_register_07c, u4prv_register_0e4;
    U32 u4value, u4err_value;
    U8 ucfirst, ucbegin, ucsum, ucbest_step;
    U32 test2_0 = DEFAULT_TEST2_0_CAL;
    U32 test2_1 = DEFAULT_TEST2_1_CAL|u4CalStartAddr;
    U32 test2_2 = DEFAULT_TEST2_2_CAL;

#ifdef CONFIG_CORE_OFF_STR
    if (IS_DDR_SUSPENDSTATE())
    {
        if(!IS_DRAM_CHANNELB_ACTIVE())	//channel trust zone 0x1000000, 16M
            test2_1 = ((((TCMGET_CHANNELA_SIZE()*0x100000)-0x1000000)>>4)-DEFAULT_TEST2_2_CAL)| 0x30000000;
        else
            test2_1 = ((((TCMGET_CHANNELB_SIZE()*0x100000)-0x1000000)>>4)-DEFAULT_TEST2_2_CAL)| 0x30000000;
    }
#endif

    mcSHOW_DBG_MSG2("=======================\n");
    mcSHOW_DBG_MSG2("    DATLAT calibration \n");
    mcSHOW_DBG_MSG2("=======================\n");
    
    // pre-save
    // 0x07c[6:4]   DATLAT bit2-bit0
    u4prv_register_07c = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL));
    // 0x0e4[4]     DALLAT bit3
    u4prv_register_0e4 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4));

    ucbest_step = ((u4prv_register_07c>>4)&0x7) | ((u4prv_register_0e4>>1)&0x8);
    mcSHOW_DBG_MSG2("DATLAT Default value = 0x%x\n", ((u4prv_register_07c>>4)&0x7) | ((u4prv_register_0e4>>1)&0x8));


    // 1.set DATLAT 0-15
    // 2.enable engine1 or engine2 
    // 3.check result  ,2-3 taps pass 
    // 4.set DATLAT 1st value when 2 taps pass ,2nd value when 3taps pass

    // Initialize
    ucfirst = 0xff;
    ucbegin = 0;
    ucsum = 0;
    u4value = u4prv_register_07c;
    mcCLR_MASK(u4value, MASK_DDR2CTL_DATLAT);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4value);

    u4value = u4prv_register_0e4;
    mcCLR_BIT(u4value, POS_PADCTL4_DATLAT3);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4value);
    
    /* [Bring up] scan all steps */
    //for (ii = 0; ii < DATLAT_TAP_NUMBER; ii++)

    for (ii = 8; ii < DATLAT_TAP_NUMBER; ii++)
    {        
        // 1
        if (ii == 8)
        {
            // bit3=1 ,bit2=bit1=bit0 =0
            u4value = u4prv_register_0e4;
            mcSET_BIT(u4value, POS_PADCTL4_DATLAT3);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4value);

            u4value = u4prv_register_07c;
            mcCLR_MASK(u4value, MASK_DDR2CTL_DATLAT);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4value);
        }
        else
        {
            u4value = u4prv_register_07c;
            mcSET_FIELD(u4value, (ii&0x7), MASK_DDR2CTL_DATLAT, POS_DDR2CTL_DTALAT);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4value);
        }

        // 2

        // enable TE2, audio pattern
        u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);
       
        
        if (u4err_value == 0)
        {
            if (ucbegin == 0)
            {
                // first tap which is pass
                ucfirst = ii;
                ucbegin = 1;
            }
            if (ucbegin == 1)
            {
                ucsum++;
            }
        }
        else
        {
            if (ucbegin == 1)
            {
                // pass range end
                ucbegin = 0xff;
            }
        }

        mcSHOW_DBG_MSG2("TAP=%2d, err_value=0x%8x, begin=%d, first=%3d, sum=%d\n", ii, u4err_value, ucbegin, ucfirst, ucsum);
	}

    // 4
    if (ucsum == 0)
    {
        mcSHOW_ERROR_CHIP_DisplayString("no DATLAT taps pass\n");
		return DRAM_FAIL;
    }    
    else if (ucsum > 1)
    {
        //if test engine2, DLE return not so density, once pass, always pass
        //so we choose the second one as the best
        ucbest_step = ucfirst + 1;
    }
    else
    {
        ucbest_step = ucfirst + (ucsum-1)/2;
    }

    mcSHOW_DBG_MSG2("first_step=%d total pass=%d best_step=%d\n", ucfirst, ucsum, ucbest_step);


    if (ucsum == 0)
    {
        mcSHOW_ERROR_CHIP_DisplayString("DATLAT calibration fail, write back to default values!\n");		

        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4prv_register_07c);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4prv_register_0e4);
    }
    else
    {
        if (ucbest_step >= 8)
        {
			u4value = u4prv_register_0e4;
			mcSET_BIT(u4value, POS_PADCTL4_DATLAT3);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4value);
        }
        else
        {
			u4value = u4prv_register_0e4;
			mcCLR_BIT(u4value, POS_PADCTL4_DATLAT3);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4value);
        }
        u4value = u4prv_register_07c;
        mcSET_FIELD(u4value, (ucbest_step&0x7), MASK_DDR2CTL_DATLAT, POS_DDR2CTL_DTALAT);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4value);
    }    

    // Log Example
/*
==============================================================
    DATLAT calibration
    channel=2(2:cha, 3:chb)    apply = 1
==============================================================
DATLAT Default value = 0xe
TAP= 0, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 1, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 2, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 3, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 4, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 5, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 6, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 7, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 8, err_value=0xffffffff, begin=0, first=255, sum=0
TAP= 9, err_value=0xffffffff, begin=0, first=255, sum=0
TAP=10, err_value=0xffffffff, begin=0, first=255, sum=0
TAP=11, err_value=0xffffffff, begin=0, first=255, sum=0
TAP=12, err_value=0x       0, begin=1, first= 12, sum=1
TAP=13, err_value=0x       0, begin=1, first= 12, sum=2
TAP=14, err_value=0x       0, begin=1, first= 12, sum=3
TAP=15, err_value=0x       0, begin=1, first= 12, sum=4
pattern=1(0: ISI, 1: AUDIO, 2: TA4, 3: TA4-3) first_step=12 total pass=4 best_step=13
*/

    return DRAM_OK;

}
#endif

#endif

#ifdef CONFIG_CORE_OFF_STR
//-------------------------------------------------------------------------
/** DramcEnterSR
 *  DRAMC issue self refresh command to DRAM
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (U8): 0: OK, others: Fail
 */
//-------------------------------------------------------------------------
/*
U8 DramcWriteDataToDramBeforeEnterSR(void)
{
    U32 u4err_value;
    U32 test2_0 = DEFAULT_TEST2_0_CAL;
    U32 test2_1 = DEFAULT_TEST2_1_CAL;
    U32 test2_2 = DEFAULT_TEST2_2_CAL;

    //Write pre-defined data into dram (selftest2 in write mode)
    //u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);
    u4err_value = 0;

    //Issue self-refresh command to dram
    //entry self refresh    4h [26]
    //u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
    //mcSET_BIT(u4value, POS_CONF1_SELFREF);
    //ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);

    //Wait at least 9*tREFI+tCKSRE+20*tMIOCK (or 1ms)
    mcDELAY_us(1);

    mcSHOW_DBG_MSG2("%d (2: cha, 3: chb) DRAMC enter self refresh...err_value=%8x\n", IS_DRAM_CHANNELB_ACTIVE(), u4err_value);

    UNUSED(u4err_value);

    return DRAM_OK;
}
*/
//-------------------------------------------------------------------------
/** DramcExitSR
 *  DRAMC issue exit self refresh command to DRAM
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (U8): 0: OK, others: Fail
 */
//-------------------------------------------------------------------------
U8 DramcExitSR(void)
{
    U32 u4value;
    U32 test2_0 = DEFAULT_TEST2_0_CAL;
    U32 test2_1 = DEFAULT_TEST2_1_CAL;
    U32 test2_2 = DEFAULT_TEST2_2_CAL;

    //Exit self-refresh command to dram
    //exit self refresh    4h [26]
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1));
    mcCLR_BIT(u4value, POS_CONF1_SELFREF);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), u4value);

    //Wait at least 9*tREFI+tCKSRE+20*tMIOCK (or 1ms)
    mcDELAY_us(1);

    //reset phy R_DMPHYRST: 0xf0[28]
    // 0x0f0[28] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
    mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

    //Async FIFO RESET, R_DMSYNCRST 0x1b44[6]
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO));
    mcSET_BIT(u4value, 6);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), u4value);
    //delay 100ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, 6);
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_ASYNC_FIFO), u4value);

    // read data counter reset
    // 0x0f4[25] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
    mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
	
    mcSHOW_DBG_MSG2("fcSET_AXI_CLK_USE_CLK_DRAM\n");
	
	
#if fcSET_AXI_CLK_USE_CLK_DRAM
	/*[important]!!! change axi_clk to clk_dram after mempll enable, otherwise can't BIM access dram */
	mcDELAY_us(1);
	u4value = ucDram_Register_Read(0x700080CC);
	mcSET_BIT(u4value, 0);
	ucDram_Register_Write(0x700080CC, u4value);
	mcDELAY_us(1);
#endif

    mcSHOW_DBG_MSG22("DRAM_RX_DATLAT_CAL\n");

#ifdef DRAM_RX_DATLAT_CAL
    // For MT5399, add DATLAT calibration
    // Issue from MT6589, async fifo, core power off and on, DATLAT window may have 1T shift, 2012/11/28
    DramcRxdatlatCal();
#endif

    return DRAM_OK;
}

/*
void DramcBackupReg(UINT32 *u4DRAMCTemp)
{
    U8 ii;
    U32 u4value, u4addr, u4PHY_BASE_ADDR, u4PHY_BASE_ADDR1;
    // DRAMC backup registers
    U16 gu2dramc_backup_regaddr[DRAMC_BACKUP_REG_NUM] = {DRAMC_REG_CONF1, DRAMC_REG_CONF2, DRAMC_REG_TEST2_3, DRAMC_REG_DDR2CTL, DRAMC_REG_PADCTL4, \
                                                              DRAMC_REG_DQIDLY1, DRAMC_REG_DQIDLY2, DRAMC_REG_DQIDLY3, DRAMC_REG_DQIDLY4, DRAMC_REG_DQIDLY5, \
                                                              DRAMC_REG_DQIDLY6, DRAMC_REG_DQIDLY7, DRAMC_REG_DQIDLY8};

    //Backup all the VCCK domain registers
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(0x000);
        u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(0x200);
    }
    else
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(0x400);
        u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(0x250);
    }

    //DRAMC registers
    for (ii=0; ii<DRAMC_BACKUP_REG_NUM; ii++)
    {
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(gu2dramc_backup_regaddr[ii]));
        *(u4DRAMCTemp+ii) = u4value;
    }

    //DDRPHY registers
    //---A1, B2 registers
    u4addr = u4PHY_BASE_ADDR + 0x00;
    for (ii = 0; ii <= 3; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x18;
    for (ii = 4; ii <= 14; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x4c;
    for (ii = 15; ii <= 43; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xcc;
    for (ii = 44; ii <= 46; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xe0;
    for (ii = 47; ii <= 48; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //ii=49
    u4addr = u4PHY_BASE_ADDR + 0xf0;
    u4value = ucDram_Register_Read(u4addr);
    *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;

    //---A2, B1 and PLL registers
    //A2, B1 (A2+0x50) registers
    //0x00~0x0c
    u4addr = u4PHY_BASE_ADDR1 + 0x00;
    for (ii = 50; ii <= 53; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0x18~0x40
    u4addr = u4PHY_BASE_ADDR1 + 0x18;
    for (ii = 54; ii <= 64; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0x4c~0x50
    u4addr = u4PHY_BASE_ADDR1 + 0x4c;
    for (ii = 65; ii <= 66; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0xc8~0xcc (0xd4~0xd8)
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4addr = mcSET_PHY_REG_ADDR(0x2c8);
    }
    else
    {
        u4addr = mcSET_PHY_REG_ADDR(0x2d4);
    }
    for (ii = 67; ii <= 68; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
        u4addr = u4addr + 4;
    }

    //0xd4 (0xe0)
    //ii=69
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4addr = mcSET_PHY_REG_ADDR(0x2d4);
    }
    else
    {
        u4addr = mcSET_PHY_REG_ADDR(0x2e0);
    }
    u4value = ucDram_Register_Read(u4addr);
    *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM) = u4value;
}

void DramcBackupCommonReg(UINT32 *u4DRAMCTemp)
{
    U8 ii;
    U32 u4value, u4addr;

    //PLL registers, regardless of channel
    //May execute twice for chA & chB
    u4addr = mcSET_PHY_REG_ADDR(0x2b8);
    for (ii = 0; ii <= 1; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        *(u4DRAMCTemp+ii) = u4value;
        u4addr = u4addr + 4;
     }

    ii=2;
    u4addr = mcSET_PHY_REG_ADDR(0x2c8);
    u4value = ucDram_Register_Read(u4addr);
    *(u4DRAMCTemp+ii) = u4value;

    //---DDRPHY wrapper registers
    ii=3;
    *(u4DRAMCTemp+ii) = ucDram_Register_Read(mcSET_PHY_REG_ADDR(0xa4c));

    //Arbiter chsel
    ii=4;
    *(u4DRAMCTemp+ii) = TCM_DRAM_SIZE;
}
*/
void DramcWriteBackReg(UINT32 *u4DRAMCTemp)
{
    U8 ii;
    U32 u4value, u4addr, u4PHY_BASE_ADDR, u4PHY_BASE_ADDR1;
    // DRAMC backup registers
    U16 gu2dramc_backup_regaddr[DRAMC_BACKUP_REG_NUM] = {DRAMC_REG_CONF1, DRAMC_REG_CONF2, DRAMC_REG_TEST2_3, DRAMC_REG_DDR2CTL, DRAMC_REG_PADCTL4, \
            DRAMC_REG_DQIDLY1, DRAMC_REG_DQIDLY2, DRAMC_REG_DQIDLY3, DRAMC_REG_DQIDLY4, DRAMC_REG_DQIDLY5, \
            DRAMC_REG_DQIDLY6, DRAMC_REG_DQIDLY7, DRAMC_REG_DQIDLY8
                                                        };

    //restore all the VCCK domain registers
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0);
        u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG0);
    }
    else
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0);
        u4PHY_BASE_ADDR1 = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG20);
    }

    //DRAMC registers
    for (ii=0; ii<DRAMC_BACKUP_REG_NUM; ii++)
    {
        u4value = *(u4DRAMCTemp+ii);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(gu2dramc_backup_regaddr[ii]), u4value);
    }

    //DDRPHY registers
    //---A1, B2 registers
    u4addr = u4PHY_BASE_ADDR + 0x00;
    for (ii = 0; ii <= 3; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x18;
    for (ii = 4; ii <= 14; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0x4c;
    for (ii = 15; ii <= 43; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xcc;
    for (ii = 44; ii <= 46; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    u4addr = u4PHY_BASE_ADDR + 0xe0;
    for (ii = 47; ii <= 48; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //ii=49
    u4addr = u4PHY_BASE_ADDR + 0xf0;
    u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
    ucDram_Register_Write(u4addr, u4value);

    //---A2, B1 and PLL registers
    //A2, B1 (A2+0x50) registers
    //0x00~0x0c
    u4addr = u4PHY_BASE_ADDR1 + 0x00;
    for (ii = 50; ii <= 53; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0x18~0x40
    u4addr = u4PHY_BASE_ADDR1 + 0x18;
    for (ii = 54; ii <= 64; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0x4c~0x50
    u4addr = u4PHY_BASE_ADDR1 + 0x4c;
    for (ii = 65; ii <= 66; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0xc8~0xcc (0xd4~0xd8)
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50);
    }
    else
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53);
    }
    for (ii = 67; ii <= 68; ii++)
    {
        u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    //0xd4 (0xe0)
    //ii=69
    if (!IS_DRAM_CHANNELB_ACTIVE())
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53);
    }
    else
    {
        u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG56);
    }
    u4value = *(u4DRAMCTemp+ii+DRAMC_BACKUP_REG_NUM);
    ucDram_Register_Write(u4addr, u4value);
}

void DramcWriteBackCommonReg(UINT32 *u4DRAMCTemp)
{
    U8 ii;
    U32 u4value, u4addr;
    //PLL registers, regardless of channel
    //May execute twice for chA & chB
    u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46);
    for (ii = 0; ii <= 1; ii++)
    {
        u4value = *(u4DRAMCTemp+ii);
        ucDram_Register_Write(u4addr, u4value);
        u4addr = u4addr + 4;
    }

    ii=2;
    u4addr = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50);
    u4value = *(u4DRAMCTemp+ii);
    ucDram_Register_Write(u4addr, u4value);

    //---DDRPHY wrapper registers
    ii=3;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), *(u4DRAMCTemp+ii));

    //Arbiter chsel
    ii=4;
    TCM_DRAM_SIZE = *(u4DRAMCTemp+ii);
#if defined(CC_CHA_CHB_NO_GAP)
    // Set CHSEL channel B according to channel A Mbytes.
    u4value = ucDram_Register_Read(mcSET_ARBITER_ADDR(0x0));
    u4value &= ~0x700000;
    if (TCMGET_CHANNELA_SIZE()==0x40)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x100000);//channel A 64Mbytes
    else if (TCMGET_CHANNELA_SIZE()==0x80)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x200000);//channel A 128Mbytes
    else if (TCMGET_CHANNELA_SIZE()==0x100)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x300000);//channel A 256Mbytes
    else if (TCMGET_CHANNELA_SIZE()==0x200)
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x400000);//channel A 512Mbytes
    else
        ucDram_Register_Write(mcSET_ARBITER_ADDR(0x0), u4value |0x500000);//channel A 1Gbytes
#endif
}

//-------------------------------------------------------------------------
/** DramcSuspend
 *  Suspend DRAMC and DDRPHY
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
/*
UINT32 DRAMC_str_save_registers(UINT32 *u4DRAMCTemp, UINT32 u4Size)
{
    // enter self refresh
    TCMSET_CHANNELA_ACTIVE();
    DramcWriteDataToDramBeforeEnterSR();
    TCMSET_CHANNELB_ACTIVE();
    DramcWriteDataToDramBeforeEnterSR();

    // backup necessary registers
    TCMSET_CHANNELA_ACTIVE();
    DramcBackupReg(u4DRAMCTemp);
    TCMSET_CHANNELB_ACTIVE();
    DramcBackupReg(u4DRAMCTemp+(PHY_BACKUP_REG_NUM+DRAMC_BACKUP_REG_NUM));
    TCMSET_CHANNELA_ACTIVE();
    DramcBackupCommonReg(u4DRAMCTemp+(PHY_BACKUP_REG_NUM+DRAMC_BACKUP_REG_NUM)*2);

    // no suspend S1 state in MT5399
    TCMSET_DRAM_SUSPEND();

    // Set DMSUS33=1, for MT5399, it is GPIO control

    // RSTB33=0, for MT5399, it is GPIO control

    // Shut down VCCK, for MT5399, it is GPIO control

    // Shut down AVDD33, for MT5399, it is GPIO control

    return ((PHY_BACKUP_REG_NUM+DRAMC_BACKUP_REG_NUM)*2+CHA_CHB_COMMON_REG_NUM)*sizeof(UINT32);

}
*/
//-------------------------------------------------------------------------
/** DramcResume
 *  Suspend DRAMC and DDRPHY
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DRAMC_str_restore_registers(UINT32 *u4DRAMCTemp)
{
    // Turn on AVDD33, for MT5399, it is GPIO control

    // Turn on VCCK, for MT5399, it is GPIO control

    // RSTB33=1, for MT5399, it is GPIO control

    //Current dram state is suspend
    TCMSET_DRAM_SUSPEND();
    // re-Init...(dramc_init, dramc_config)
    extern 	void vDDR_BLConfig(void);
    vDDR_BLConfig();
    mcSHOW_DBG_MSG2("DdrPhyInit\n");

    TCMSET_CHANNELA_ACTIVE();
    DdrPhyInit();	
    DramcInit();
    DramcConfig();
    DramcWriteBackReg(u4DRAMCTemp);

    TCMSET_CHANNELA_ACTIVE();
    DramcWriteBackCommonReg(u4DRAMCTemp+(PHY_BACKUP_REG_NUM+DRAMC_BACKUP_REG_NUM)*2);
}

void DRAMC_Exit_Suspend(UINT32 *u4DRAMCTemp)
{
    //Set DMSUS33=0 (here due to less impact to DRAM), for MT5399, it is GPIO control

    // exit self refresh
    mcSHOW_DBG_MSG2("DRAMC_Exit_Suspend\n");	

    TCMSET_CHANNELA_ACTIVE();
    DramcExitSR();
    //TCMSET_CHANNELB_ACTIVE();
    //DramcExitSR();

    TCMSET_CHANNELA_ACTIVE();
    TCMSET_DRAM_NORMAL();
}
#endif//#ifdef CONFIG_CORE_OFF_STR

#else//#ifdef CC_MTK_PRELOADER
#ifdef CONFIG_CORE_OFF_STR

static UINT32* pm_save_tcm_msg= NULL;

void DMARC_TCM_suspend(void)
{
    pm_save_tcm_msg = x_mem_alloc(8 * sizeof(UINT32));//
    mcSHOW_DBG_MSG2("DMARC_TCM_suspend addr : 0x%x .\n",pm_save_tcm_msg);
	
    if (pm_save_tcm_msg!=NULL)
    {
        pm_save_tcm_msg[0] = TCM_DRAM_SIZE;
        pm_save_tcm_msg[1] = TCM_DRAM_FLAGS;
        pm_save_tcm_msg[2] = TCM_DRAM_FLAGS1;
        pm_save_tcm_msg[3] = TCM_DRAM_FLAGS2;
        pm_save_tcm_msg[4] = TCM_DRAM_FLAGS3;
        pm_save_tcm_msg[5] = TCM_DRAM_FLAGS4;
        pm_save_tcm_msg[6] = TCM_DRAM_DLYCELL_PERT;
    }
}

void DMARC_TCM_resume(void)
{
    if (pm_save_tcm_msg!=NULL)
    {
        TCM_DRAM_SIZE           = pm_save_tcm_msg[0];
        TCM_DRAM_FLAGS          = pm_save_tcm_msg[1];
        TCM_DRAM_FLAGS1         = pm_save_tcm_msg[2];
        TCM_DRAM_FLAGS2         = pm_save_tcm_msg[3];
        TCM_DRAM_FLAGS3         = pm_save_tcm_msg[4];
        TCM_DRAM_FLAGS4         = pm_save_tcm_msg[5];
        TCM_DRAM_DLYCELL_PERT   = pm_save_tcm_msg[6];
        x_mem_free(pm_save_tcm_msg);
    }
}
#endif
#endif//#ifdef CONFIG_CORE_OFF_STR

#if 1
//-------------------------------------------------------------------------
/** DramcRegDump
 *  Dump all registers (DDRPHY and DRAMC)
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcRegDump()
{
    U16 ii;
    U32 u4addr, u4value;

    mcSHOW_DBG_MSG2("\n=================mt8563 PLL/PHY register value================================");
    for (ii=0; ii<=0x0f4; ii=ii+4)
    {
        // confirm SoC platform has "%" operation
        if (ii%16==0)
        {
            mcSHOW_DBG_MSG2("\n0x%8x | ", mcSET_PHY_REG_ADDR(0x000)+ii);
        }
        u4addr = mcSET_PHY_REG_ADDR(0x000)+ii;
        u4value = ucDram_Register_Read(u4addr);
        mcSHOW_DBG_MSG2(" %08x", u4value);
    }

    for (ii=0x2a0; ii<=0x2e8; ii=ii+4)
    {
        // confirm SoC platform has "%" operation
        if (ii%16==0)
        {
            mcSHOW_DBG_MSG2("\n0x%8x | ", mcSET_PHY_REG_ADDR(0x000)+ii);
        }
        u4addr = mcSET_PHY_REG_ADDR(0x000)+ii;
        u4value = ucDram_Register_Read(u4addr);
        mcSHOW_DBG_MSG2(" %08x", u4value);
    }


    mcSHOW_DBG_MSG2("\n=================mt8563 PLL Wrapper register value================================");
    for (ii=0xa40; ii<=0xbd0; ii=ii+4)
    {
        // confirm SoC platform has "%" operation
        if ((ii-0xa40)%16==0)
        {
            mcSHOW_DBG_MSG2("\n0x%8x | ", mcSET_PHY_REG_ADDR(0x000)+ii);
        }
        u4addr = mcSET_PHY_REG_ADDR(0x000)+ii;
        u4value = ucDram_Register_Read(u4addr);
        mcSHOW_DBG_MSG2(" %08x", u4value);
    }
#if 0
    mcSHOW_DBG_MSG("\n mcSET_PHY_REG_ADDR(0xf94) | ");
    u4addr = mcSET_PHY_REG_ADDR(0xf94);
    u4value = ucDram_Register_Read(u4addr);
    mcSHOW_DBG_MSG(" %08x", u4value);

    TCMSET_CHANNELA_ACTIVE();
#endif
    mcSHOW_DBG_MSG2("\n=========mt8563 cha dramc register value===========================");
    for (ii=480; ii<0x4E0; ii=ii+4)
    {
        // confirm SoC platform has "%" operation
        if (ii%16==0)
        {
            mcSHOW_DBG_MSG2("\n0x%8x | ", mcSET_DRAMC_REG_ADDR(ii));
        }
        u4addr = mcSET_DRAMC_REG_ADDR(ii);
        u4value = ucDram_Register_Read(u4addr);
        mcSHOW_DBG_MSG2(" %08x", u4value);
    }

#if 0
    mcSHOW_DBG_MSG("\n=========mt8563 chb dramc register value===========================");

    TCMSET_CHANNELB_ACTIVE();
    for (ii=0; ii<0x400; ii=ii+4)
    {
        // confirm SoC platform has "%" operation
        if (ii%16==0)
        {
            mcSHOW_DBG_MSG("\n0x%8x | ", mcSET_DRAMC_REG_ADDR(ii));
        }
        u4addr = mcSET_DRAMC_REG_ADDR(ii);
        u4value = ucDram_Register_Read(u4addr);
        mcSHOW_DBG_MSG(" %08x", u4value);
    }

    mcSHOW_DBG_MSG("\n");

    TCMSET_CHANNELA_ACTIVE();
#endif

    UNUSED(u4value);

    return DRAM_OK;

    // log example
    /*
    =================a60807 PLL/PHY register value================================
    mcSET_PHY_REG_ADDR(0x000) |  80178017   550000        0        0        0        0
    0x20001018 |  80178017  1458017 80170145 80178017   558017 80170055
    0x20001030 |         0        0        2   900002   8e002e ff000100
    0x20001048 |  803df088       b8 b86f0080 80250080 80250080 80250080
    0x20001060 |  80250080 80250080 80250080 80250080 80250080 80250080
    0x20001078 |  80250080 80250080 80250080 80250080 80250080 80250080
    0x20001090 |  80250080 80250080 80250080 80250080 80250080 80250080
    0x200010a8 |  80250080 80250080 80650080 80250080 802500b8 b8654000
    0x200010c0 |         0      534 18838000    10003  d0b0d0b        0
    0x200010d8 |    230000        0 f0001717     b8b8      300 2e800000
    0x200010f0 |         0        0        0        0        0        0
    0x20001108 |         0        0        0        0        0        0
    0x20001120 |         0        0        0        0        0        0
    0x20001138 |         0        0        0        0        0        0
    0x20001150 |         0        0        0        0        0        0
    0x20001168 |         0        0        0        0        0        0
    0x20001180 |         0        0        0        0        0        0
    0x20001198 |         0        0        0        0        0        0
    0x200011b0 |         0        0        0        0        0        0
    0x200011c8 |         0        0        0        0        0        0
    0x200011e0 |         0        0        0        0        0        0
    0x200011f8 |         0        0 80178017   550000        0        0
    0x20001210 |         0        0 80178017  1458017 80170145 80178017
    0x20001228 |    558017 80170055        0        0        2   8a0002
    0x20001240 |    87002e ff000100 8000e088       b8 b86f8017 80170055
    0x20001258 |         0        0        0        0 80178017  1458017
    0x20001270 |  80170145 80178017   558017 80170055        0        0
    0x20001288 |         2   980002   98002e ff000100 8000e088       b8
    0x200012a0 |  b86f1050  d997883 8000100c 10ff01c2 57570000  29a1883
    0x200012b8 |  80000100  2000000  29a1883 80000001    30c02  d000000
    0x200012d0 |  23000000 c0000a0d  a0e0000 23000000 c0000600        0
    0x200012e8 |  362a0ba0        0        0        0        0        0
    0x20001300 |         0        0        0        0        0        0
    0x20001318 |         0        0        0        0        0        0
    0x20001330 |         0        0        0        0        0        0
    0x20001348 |         0        0        0        0        0        0
    0x20001360 |         0        0        0        0        0        0
    0x20001378 |         0        0        0        0        0        0
    0x20001390 |         0        0        0        0        0        0
    0x200013a8 |         0        0        0        0        0        0
    0x200013c0 |         0        0        0        0        0        0
    0x200013d8 |         0        0        0        0        0        0
    0x200013f0 |         0        0        0        0 80178017   550000
    0x20001408 |         0        0        0        0 80178017  1458017
    0x20001420 |  80170145 80178017   558017 80170055        0        0
    0x20001438 |         2   8a0002   8b002e ff000100 8000e088       b8
    0x20001450 |  b86f0080 80250080 80250080 80020080 80250080 80250080
    0x20001468 |  80250080 80250080 80250080 80250080 80250080 80250080
    0x20001480 |  80250080 80250080 80250080 80250080 80250080 80250080
    0x20001498 |  80250080 80250080 80250080 80250080 80250080 80250080
    0x200014b0 |  80650080 80250080 802500b8 b8654000        0      534
    0x200014c8 |  18838000    10003  a090b08        0   230000        0
    0x200014e0 |  f0001717     b8b8      300 2e800000        0
    =================a60807 PLL Wrapper register value================================
    0x20001a4c |  83446655     2800  1100051        0        0        0
    0x20001a64 |         0        0        0        0        0        0
    0x20001a7c |         0        0        0        0        0 b04c033e
    0x20001a94 |         0        0        0        0        0        0
    0x20001aac |         0        0        0        0        0        0
    0x20001ac4 |         0        0        0        0        0        0
    0x20001adc |         0        0        0        0        0        0
    0x20001af4 |         0        0        0 aa220000        0 aa220000
    0x20001b0c |         0 aa220000        0 aa220000        0        0
    0x20001b24 |         0        0        0        0        0        0
    0x20001b3c |         0   110000    80300        0        0        0
    0x20001b54 |         0        0     1010        0        0        0
    0x20001b6c |         0        0        0        0        0        0
    0x20001b84 |         0        0        0        0      110
    0x20001f94 |       110
    =========a60807 dramc register value===========================
    0x20002000 |  66fe49ff f07486e3    4794c        0        0        0
    0x20002018 |         0        0        0        0 f1200f01 55010000
    0x20002030 |  33000fff 55020000 33000fff 55000000 aa000400 28880480
    0x20002048 |      d10d        0        0        0        0        0
    0x20002060 |         0        0        0        0        0        0
    0x20002078 |         0 e28743dd        0        0     2004        1
    0x20002090 |         0 40404040        0        0        0        0
    0x200020a8 |         0        0        0        0 aa22aa22 aa22aa22
    0x200020c0 |         0        0        0        0        0        0
    0x200020d8 |    100900 83008008 10008008       b2        0        0
    0x200020f0 |         0  1000000 edcb000f 37010000        0        0
    0x20002108 |         0        0  b051100        0        0        0
    0x20002120 |         0 aa080088        0        0 50000000        0
    0x20002138 |         0        0        0        0        0        0
    0x20002150 |         0        0        0        0        0        0
    0x20002168 |        80        0        0        0        0        0
    0x20002180 |         0        0        0        0        0        0
    0x20002198 |         0        0        0        0        0        0
    0x200021b0 |         0        0        0        0 8000c8b8        0
    0x200021c8 |         0        0        0        0   c80000 10622842
    0x200021e0 |  88000000        0      690        0        0        0
    0x200021f8 |         0        0        0        0        0        0
    0x20002210 |   1020102  1020102  1020102  2010102  3010102        0
    0x20002228 |   2030303  1030101        0        0        0        0
    0x20002240 |         0        0        0        0        0        0
    0x20002258 |         0        0        0        0        0        0
    0x20002270 |         0        0        0        0        0        0
    0x20002288 |         0        0        0        0        0        0
    0x200022a0 |         0        0        0        0        0        0
    0x200022b8 |         0        0        0        0        0        0
    0x200022d0 |         0        0        0        0        0        0
    0x200022e8 |         0        0        0        0        0        0
    0x20002300 |         0        0        0        0        0        0
    0x20002318 |         0        0 ffffffff ffffffff        0   d92fc3
    0x20002330 |    d92fc3        0        0        0        0 46e407c0
    0x20002348 |       400        0        0        0        0        0
    0x20002360 |    d92fc3        0        0        0        0 40404040
    0x20002378 |         0        0        0        0        0        3
    0x20002390 |         3        3        3        0        0        0
    0x200023a8 |         0        0        0        0      300        0
    0x200023c0 |         0        0        0        0        0        0
    0x200023d8 |         0        0        0        0        0        0
    0x200023f0 |         0        0        0        0
    */
}
#endif

