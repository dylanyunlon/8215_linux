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


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
#ifdef DRAM_TX_EYE_SCAN
U32 u4tx_eye_scan_buf[16][49];
#endif

#if 0 //mtk40739 temp mark
U8 ucswap_table[2][32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 , 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
						  0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13, 14, 15, 16, 17, 18, 19, 24, 25, 26, 27, 20, 21, 22, 23, 28, 29, 30, 31};
#endif





#ifdef DDR_CHA_4BIT_SWAP
U32 DramcDQ4bitSwap(U32 value)
{
	U32 dq7to4, dq11to8, dq23to20, dq27to24;
	U32 result;

	//value[7:4]
	dq7to4 = (value & (0xf<<4))>>4;
	//value[11:8]
	dq11to8 = (value & (0xf<<8))>>8;
	//value[23:20]
	dq23to20 = (value & (0xf<<20))>>20;
	//value[27:24]
	dq27to24 = (value & (0xf<<24))>>24;

	result = value;
	mcSET_FIELD(result, dq7to4, 0x00000f00, 8);
	mcSET_FIELD(result, dq11to8, 0x000000f0, 4);
	mcSET_FIELD(result, dq23to20, 0x0f000000, 24);
	mcSET_FIELD(result, dq27to24, 0x00f00000, 20);

	return result;
}
#endif
//-------------------------------------------------------------------------
/** DramcCheckPllDline
 *  Check PLL DLINE compare value with confidence count.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param u4addr           (U32): register address to be read.
 *  @param ucbit_pos        (U8):  bit position of DLINE.
 *  @retval status          (PLL_PHASE_CAL_STATUS_T): DLINE_0 or DLINE_1 or DLINE_TOGGLE 
 */
//-------------------------------------------------------------------------
#ifdef DRAM_Pll_PHASE_CAL
PLL_PHASE_CAL_STATUS_T DramcCheckPllDline(U32 u4addr, U8 ucbit_pos)
{
    U8 ii, ucconf_count;
    U32 u4value, u4mask;
    PLL_PHASE_CAL_STATUS_T dline_status;
    
    u4mask = (U32)1<<ucbit_pos;
    ucconf_count = 0;
    for (ii=0; ii<PLL_PHASE_CAL_CONF_COUNT; ii++)
    {
        u4value = ucDram_Register_Read(u4addr);
        u4value = mcGET_FIELD(u4value, u4mask, ucbit_pos);
        ucconf_count = ucconf_count + (U8) u4value;   
		//mcSHOW_DBG_MSG2("\nii = %d,ucconf_count = %d\n",ii,ucconf_count);
    }    

    if (ucconf_count == 0)
    {
        dline_status = DLINE_0;        
    }
    else if (ucconf_count == PLL_PHASE_CAL_CONF_COUNT)
    {
        dline_status = DLINE_1;  
    }
    else
    {
        dline_status = DLINE_TOGGLE;  
    }

    mcSHOW_DBG_MSG2("DLINE=%d , count=%2d @ confidence=%2d\n", dline_status, ucconf_count, PLL_PHASE_CAL_CONF_COUNT);

    return dline_status;
}

//-------------------------------------------------------------------------
/** DramcPllPhaseCal
 *  start PLL Phase Calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
void DramcPllPhaseCal(void)
{
    U8 ii;
    U32 u4value;
    PLL_PHASE_CAL_STATUS_T dline_status1, dline_status2;

    //===============================================
    //A1
    //set REF and FBK delay to 0 at initial
    mcSHOW_DBG_MSG2("\n========== A1 PLL ==========\n");
    //mcSHOW_DBG_VAL_FOR_SIMULATION("\n============== A1 PLL ==============\n");
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG51));
    u4value = u4value & 0xffffe0e0;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG51), u4value);

    //check DLINE at delay 0, if 0, incr FBK, else, incr REF
    mcSHOW_DBG_MSG2("delay tap= 0    ");	
	
    dline_status1 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_A1_CFG0), 23);
    
    if (dline_status1 == DLINE_0)
    {
        mcSHOW_DBG_MSG2("A1 PLL need to scan FBK delay\n");		
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG51));
            mcSET_FIELD(u4value, ii, 0x0000001f, 0);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG51), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);
  					
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_A1_CFG0), 23);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_1 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("A1 PLL calibration done! Final delay is %2d\n", ii);
				
                break;
            }
        }
    }
    else if (dline_status1 == DLINE_1)
    {
        mcSHOW_DBG_MSG2("A1 PLL need to scan REF delay\n");
		
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG51));
            mcSET_FIELD(u4value, ii, 0x00001f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG51), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);

			
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_A1_CFG0), 23);            
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_0 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("A1 PLL calibration done!Final delay is %2d\n", ii);

				
                break;
            }
        }
    }
    else   // dline_status1 == DLINE_TOGGLE
    {
        mcSHOW_DBG_MSG2("A1 PLL no need to scan FBK or REF delay\n");		
    }


    //===============================================
    //AB
    //set REF and FBK delay to 0 at initial
    mcSHOW_DBG_MSG2("\n========== AB PLL =========\n");
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
    u4value = u4value & 0xe0e0ffff;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

    //check DLINE at delay 0, if 0, incr FBK, else, incr REF
    mcSHOW_DBG_MSG2("delay tap= 0    ");
    dline_status1 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0), 5);
    
    if (dline_status1 == DLINE_0)
    {
        mcSHOW_DBG_MSG2("AB PLL need to scan FBK delay\n");
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, ii, 0x001f0000, 16);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0), 5);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_1 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("AB PLL calibration done!Final delay is %2d\n", ii);
                break;
            }
        }
    }
    else if (dline_status1 == DLINE_1)
    {
        mcSHOW_DBG_MSG2("AB PLL need to scan REF delay\n");
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, ii, 0x1f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0), 5);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_0 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("AB PLL calibration done!Final delay is %2d\n", ii);
                break;
            }
        }
    }
    else   // dline_status1 == DLINE_TOGGLE
    {
        mcSHOW_DBG_MSG2("AB PLL no need to scan FBK or REG delay\n");
    }
    
    //===============================================
    //B2
    //set REF and FBK delay to 0 at initial
#if 0     
    mcSHOW_DBG_MSG2("\n============== B2 PLL ==============\n");
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG51));
    u4value = u4value & 0xffffe0e0;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG51), u4value);

    //check DLINE at delay 0, if 0, incr FBK, else, incr REF
    mcSHOW_DBG_MSG2("delay tap= 0    ");
    dline_status1 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_B2_CFG0), 23);
    
    if (dline_status1 == DLINE_0)
    {
        mcSHOW_DBG_MSG2("B2 PLL need to scan FBK delay\n");
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG51));
            mcSET_FIELD(u4value, ii, 0x0000001f, 0);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG51), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_B2_CFG0), 23);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_1 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("B2 PLL calibration done!Final delay is %2d\n", ii);
                break;
            }
        }
    }
    else if (dline_status1 == DLINE_1)
    {
        mcSHOW_DBG_MSG2("B2 PLL need to scan REF delay\n");
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG51));
            mcSET_FIELD(u4value, ii, 0x00001f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG51), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_CMD_DQ2BYTE_B2_CFG0), 23);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_0 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("B2 PLL calibration done!Final delay is %2d\n", ii);
                break;
            }
        }
    }
    else   // dline_status1 == DLINE_TOGGLE
    {
        mcSHOW_DBG_MSG2("B2 PLL no need to scan FBK or REG delay\n");
    }
#endif

    //===============================================
    //CTL
    //set REF and FBK delay to 0 at initial
    mcSHOW_DBG_MSG2("\n======== CTL PLL ========\n");

    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46));
    u4value = u4value & 0xffffffe0;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46), u4value);

    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47));
    u4value = u4value & 0xe0ffffff;
    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47), u4value);

    //check DLINE at delay 0, if 0, incr FBK, else, incr REF
    mcSHOW_DBG_MSG2("delay tap= 0    ");	
    dline_status1 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0), 15);
    
    if (dline_status1 == DLINE_0)
    {
        mcSHOW_DBG_MSG2("CTL PLL need to scan FBK delay\n");	
		
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47));
            mcSET_FIELD(u4value, ii, 0x1f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG47), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);		
			
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0), 15);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_1 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("CTL PLL calibration done!Final delay is %2d\n", ii);		
  			
                break;
            }
        }
    }
    else if (dline_status1 == DLINE_1)
    {
        mcSHOW_DBG_MSG2("CTL PLL need to scan REF delay\n");	
		
        for (ii=1; ii<32; ii++)
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46));
            mcSET_FIELD(u4value, ii, 0x0000001f, 0);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG46), u4value);

            mcSHOW_DBG_MSG2("delay tap=%2d    ", ii);	
 			
            dline_status2 = DramcCheckPllDline(mcSET_PHY_REG_ADDR(DDRPHY_RGS_INT_MEM_DQ2BYTE_AB_CFG0), 15);
            if (dline_status2 != dline_status1)           	
            {
                // status change: DLINE_0 or DLINE_TOGGLE, break
                mcSHOW_DBG_MSG2("CTL PLL calibration done!Final delay is %2d\n", ii);		
                 break;
            }
        }
    }
    else   // dline_status1 == DLINE_TOGGLE
    {
        mcSHOW_DBG_MSG2("CTL PLL no need to scan FBK or REG delay\n");		
		
    }
	
    //log example
    /*
============== A1 PLL ==============
delay tap= 0    DLINE=0 , count= 0 @ confidence=16
A1 PLL need to scan FBK delay
delay tap= 1    DLINE=0 , count= 0 @ confidence=16
delay tap= 2    DLINE=0 , count= 0 @ confidence=16
delay tap= 3    DLINE=1 , count=16 @ confidence=16
A1 PLL calibration done! Final delay is  3

============== AB PLL ==============
delay tap= 0    DLINE=0 , count= 0 @ confidence=16
AB PLL need to scan FBK delay
delay tap= 1    DLINE=0 , count= 0 @ confidence=16
delay tap= 2    DLINE=0 , count= 0 @ confidence=16
delay tap= 3    DLINE=1 , count=16 @ confidence=16
AB PLL calibration done!Final delay is  3

============== B2 PLL ==============
delay tap= 0    DLINE=0 , count= 0 @ confidence=16
B2 PLL need to scan FBK delay
delay tap= 1    DLINE=0 , count= 0 @ confidence=16
delay tap= 2    DLINE=0 , count= 0 @ confidence=16
delay tap= 3    DLINE=1 , count=16 @ confidence=16
B2 PLL calibration done!Final delay is  3

============== CTL PLL ==============
delay tap= 0    DLINE=0 , count= 0 @ confidence=16
CTL PLL need to scan FBK delay
delay tap= 1    DLINE=0 , count= 0 @ confidence=16
delay tap= 2    DLINE=0 , count= 0 @ confidence=16
delay tap= 3    DLINE=1 , count=16 @ confidence=16
CTL PLL calibration done!Final delay is  3
   */
}
#endif


#ifdef DRAM_IMPEDANCE_CAL
//-------------------------------------------------------------------------
/** DramcImpedanceCalTxOcd
 *  start TX OCD impedance calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
void DramcImpedanceCalTxOcd(void)
{
    U8 fgcal_out;
    S8 p_drv, n_drv;
    U8 uccode_map[32] = {0x00, 0x01, 0x08, 0x10, 0x20, 0x40, 0x80, 0x12, 0x24, 0x48, \
 	                                 0x13, 0x19, 0x46, 0x89, 0x17, 0x1e, 0x4d, 0x35, 0x93, 0x4f, \
 	                                 0x8f, 0x3d, 0xb8, 0x73, 0xb6, 0xe9, 0xb7, 0xeb, 0xf3, 0xef, \
 	                                 0xfe, 0xff};
    U32 u4PHY_BASE_ADDR, u4value; 
    U32 u4prv_phy_register_0e0, u4prv_phy_register_0e8;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0);
    }
#if 0 
    else
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0);
    } 
#endif

    //backup registers
    u4prv_phy_register_0e8 = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe8);
    u4prv_phy_register_0e0 = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);

    //start calibrate DRVP
    //enable calibration, RG_TX_IMPx_OCD_CAL_EN (0x1?E8[31])
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe8);
    mcSET_BIT(u4value, 31);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe8, u4value);

    //select DRVP, RG_TX_IMPA_OCD_PUCMP_EN (0x1?E8[30]=1)
    mcSET_BIT(u4value, 30);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe8, u4value);

    //0x10e0[31]=1
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);
    mcSET_BIT(u4value, 31);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4value);

    for (p_drv=0; p_drv<32; p_drv++)
    {
        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe4);
        mcSET_FIELD(u4value, uccode_map[(U8)p_drv], 0x000000ff, 0);
        ucDram_Register_Write(u4PHY_BASE_ADDR|0xe4, u4value);

        mcDELAY_us(1);

        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xf4);
        fgcal_out = (U8) mcGET_FIELD(u4value, 0x00100000, 20);
        mcSHOW_DBG_MSG("OCD DRVP=%2d | 8-bit code=%2x    CALOUT=%d\n", p_drv, uccode_map[p_drv], fgcal_out);

        if (fgcal_out == 0)
        {
            mcSHOW_DBG_MSG("OCD DRVP calibration OK! DRVP=%2d\n", p_drv);
            break;
        }
    }
    if (p_drv==32)
    {
        mcSHOW_ERROR_CHIP_DisplayString("No valid OCD DRVP!!\n");
        mcSHOW_ERROR_CHIP_DisplayString("Skip OCD DRVN calibration!!\n");
        goto IMP_CAL_TX_OCD_END;
    }

    //start calibrate DRVN
    //select DRVN, RG_TX_IMPA_OCD_PUCMP_EN (0x1?E8[30]=0)
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe8);
    mcCLR_BIT(u4value, 30);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe8, u4value);
    
    for (n_drv=31;n_drv>=0; n_drv--)
    {
        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe4);
        mcSET_FIELD(u4value, uccode_map[(U8)n_drv], 0x0000ff00, 8);
        ucDram_Register_Write(u4PHY_BASE_ADDR|0xe4, u4value);

        mcDELAY_us(1);

        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xf4);
        fgcal_out = (U8) mcGET_FIELD(u4value, 0x00100000, 20);
        mcSHOW_DBG_MSG("OCD DRVN=%2d | 8-bit code=%2x    CALOUT=%d\n", n_drv, uccode_map[n_drv], fgcal_out);

        if (fgcal_out == 0)
        {
            mcSHOW_DBG_MSG("OCD DRVN calibration OK! DRVN=%2d\n", n_drv);
            break;
        }
    }
    if (n_drv < 0)
    {
        mcSHOW_ERROR_CHIP_DisplayString("No valid OCD DRVN!!\n");        
        goto IMP_CAL_TX_OCD_END;
    } 	
 	
IMP_CAL_TX_OCD_END:
    //recover registers
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe8, u4prv_phy_register_0e8);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4prv_phy_register_0e0);
   
    // log example
    /*
OCD DRVP= 0 | 8-bit code= 0    CALOUT=1
OCD DRVP= 1 | 8-bit code= 1    CALOUT=1
OCD DRVP= 2 | 8-bit code= 8    CALOUT=1
OCD DRVP= 3 | 8-bit code=10    CALOUT=1
OCD DRVP= 4 | 8-bit code=20    CALOUT=1
OCD DRVP= 5 | 8-bit code=40    CALOUT=1
OCD DRVP= 6 | 8-bit code=80    CALOUT=1
OCD DRVP= 7 | 8-bit code=12    CALOUT=1
OCD DRVP= 8 | 8-bit code=24    CALOUT=1
OCD DRVP= 9 | 8-bit code=48    CALOUT=1
OCD DRVP=10 | 8-bit code=13    CALOUT=1
OCD DRVP=11 | 8-bit code=19    CALOUT=1
OCD DRVP=12 | 8-bit code=46    CALOUT=1
OCD DRVP=13 | 8-bit code=89    CALOUT=1
OCD DRVP=14 | 8-bit code=17    CALOUT=1
OCD DRVP=15 | 8-bit code=1e    CALOUT=1
OCD DRVP=16 | 8-bit code=4d    CALOUT=1
OCD DRVP=17 | 8-bit code=35    CALOUT=0
OCD DRVP calibration OK! DRVP=17
OCD DRVN=31 | 8-bit code=ff    CALOUT=1
OCD DRVN=30 | 8-bit code=fe    CALOUT=1
OCD DRVN=29 | 8-bit code=ef    CALOUT=1
OCD DRVN=28 | 8-bit code=f3    CALOUT=1
OCD DRVN=27 | 8-bit code=eb    CALOUT=1
OCD DRVN=26 | 8-bit code=b7    CALOUT=1
OCD DRVN=25 | 8-bit code=e9    CALOUT=1
OCD DRVN=24 | 8-bit code=b6    CALOUT=1
OCD DRVN=23 | 8-bit code=73    CALOUT=1
OCD DRVN=22 | 8-bit code=b8    CALOUT=1
OCD DRVN=21 | 8-bit code=3d    CALOUT=1
OCD DRVN=20 | 8-bit code=8f    CALOUT=1
OCD DRVN=19 | 8-bit code=4f    CALOUT=1
OCD DRVN=18 | 8-bit code=93    CALOUT=1
OCD DRVN=17 | 8-bit code=35    CALOUT=1
OCD DRVN=16 | 8-bit code=4d    CALOUT=1
OCD DRVN=15 | 8-bit code=1e    CALOUT=1
OCD DRVN=14 | 8-bit code=17    CALOUT=0
OCD DRVN calibration OK! DRVN=14
   */
}

//-------------------------------------------------------------------------
/** DramcImpedanceCalRxOdt
 *  start RX DDT impedance calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
void DramcImpedanceCalRxOdt(void)
{
    U8 fgcal_out;
    S8 p_drv = 0;
	S8 n_drv = 0;
    U8 uccode_map[32] = {0x00, 0x01, 0x08, 0x10, 0x20, 0x40, 0x80, 0x12, 0x24, 0x48, \
 	                                 0x13, 0x19, 0x46, 0x89, 0x17, 0x1e, 0x4d, 0x35, 0x93, 0x4f, \
 	                                 0x8f, 0x3d, 0xb8, 0x73, 0xb6, 0xe9, 0xb7, 0xeb, 0xf3, 0xef, \
 	                                 0xfe, 0xff};
    U32 u4PHY_BASE_ADDR, u4value; 
    U32 u4prv_phy_register_0e0, u4prv_phy_register_0e4;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
	  {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0);
    }
#if 0
    else
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0);
    }
 
#endif

    //backup registers
    u4prv_phy_register_0e4 = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe4);
    u4prv_phy_register_0e0 = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);

    //start calibrate DRVP
    //enable calibration, RG_TX_IMPx_ODT_CAL_EN (0x1?E4[31])
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe4);
    mcSET_BIT(u4value, 31);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe4, u4value);

    //select DRVP, RG_TX_IMPA_ODT_PUCMP_EN (0x1?E4[30]=1)
    mcSET_BIT(u4value, 30);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe4, u4value);

    //0x10e0[31]=1
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);
    mcSET_BIT(u4value, 31);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4value);

    for (p_drv=0; p_drv<32; p_drv++)
    {
        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);
        mcSET_FIELD(u4value, uccode_map[(U8)p_drv], 0x000000ff, 0);
        ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4value);

        mcDELAY_us(1);

        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xf4);
        fgcal_out = (U8) mcGET_FIELD(u4value, 0x00200000, 21);
        mcSHOW_DBG_MSG("ODT DRVP=%2d | 8-bit code=%2x    CALOUT=%d\n", p_drv, uccode_map[p_drv], fgcal_out);

        if (fgcal_out == 0)
        {
            mcSHOW_DBG_MSG("ODT DRVP calibration OK! DRVP=%2d\n", p_drv);
            break;
        }
    }
    if (p_drv==32)
    {
        mcSHOW_ERROR_CHIP_DisplayString("No valid ODT DRVP!!\n");
        mcSHOW_ERROR_CHIP_DisplayString("Skip ODT DRVN calibration!!\n");
        goto IMP_CAL_RX_ODT_END;
    }

    //start calibrate DRVN
    //select DRVN, RG_TX_IMPA_ODT_PUCMP_EN (0x1?E4[30]=0)
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe4);
    mcCLR_BIT(u4value, 30);
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe4, u4value);
    
    for (n_drv=31;n_drv>=0; n_drv--)
    {
        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);
        mcSET_FIELD(u4value, uccode_map[(U8)n_drv], 0x0000ff00, 8);
        ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4value);

        mcDELAY_us(1);

        u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xf4);
        fgcal_out = (U8) mcGET_FIELD(u4value, 0x00200000, 21);
        mcSHOW_DBG_MSG("ODT DRVN=%2d | 8-bit code=%2x    CALOUT=%d\n", n_drv, uccode_map[n_drv], fgcal_out);

        if (fgcal_out == 0)
        {
            mcSHOW_DBG_MSG("ODT DRVN calibration OK! DRVN=%2d\n", n_drv);
            break;
        }
    }
    if (n_drv < 0)
    {
        mcSHOW_ERROR_CHIP_DisplayString("No valid ODT DRVN!!\n");        
        goto IMP_CAL_RX_ODT_END;
    } 	
 	
IMP_CAL_RX_ODT_END:
    //recover registers
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe4, u4prv_phy_register_0e4);
    // 0xe0 duplicate, need to apply drv_p & drv_n below if apply
    ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4prv_phy_register_0e0);
    
	// check if p_drv and n_drv valid?!
	if (p_drv == 32)
	{
	    p_drv = 31;
	    n_drv = 31;
	}
	else if (n_drv < 0)
	{
	    n_drv = 0;
	}

	u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);
	mcSET_FIELD(u4value, uccode_map[(U8)p_drv], 0x000000ff, 0);
	mcSET_FIELD(u4value, uccode_map[(U8)n_drv], 0x0000ff00, 8);
	ucDram_Register_Write(u4PHY_BASE_ADDR|0xe0, u4value);
 
    // log example
    /*
ODT DRVP= 0 | 8-bit code= 0    CALOUT=1
ODT DRVP= 1 | 8-bit code= 1    CALOUT=1
ODT DRVP= 2 | 8-bit code= 8    CALOUT=1
ODT DRVP= 3 | 8-bit code=10    CALOUT=1
ODT DRVP= 4 | 8-bit code=20    CALOUT=1
ODT DRVP= 5 | 8-bit code=40    CALOUT=1
ODT DRVP= 6 | 8-bit code=80    CALOUT=1
ODT DRVP= 7 | 8-bit code=12    CALOUT=1
ODT DRVP= 8 | 8-bit code=24    CALOUT=1
ODT DRVP= 9 | 8-bit code=48    CALOUT=1
ODT DRVP=10 | 8-bit code=13    CALOUT=1
ODT DRVP=11 | 8-bit code=19    CALOUT=1
ODT DRVP=12 | 8-bit code=46    CALOUT=1
ODT DRVP=13 | 8-bit code=89    CALOUT=1
ODT DRVP=14 | 8-bit code=17    CALOUT=0
ODT DRVP calibration OK! DRVP=14
ODT DRVN=31 | 8-bit code=ff    CALOUT=1
ODT DRVN=30 | 8-bit code=fe    CALOUT=1
ODT DRVN=29 | 8-bit code=ef    CALOUT=1
ODT DRVN=28 | 8-bit code=f3    CALOUT=1
ODT DRVN=27 | 8-bit code=eb    CALOUT=1
ODT DRVN=26 | 8-bit code=b7    CALOUT=1
ODT DRVN=25 | 8-bit code=e9    CALOUT=1
ODT DRVN=24 | 8-bit code=b6    CALOUT=1
ODT DRVN=23 | 8-bit code=73    CALOUT=1
ODT DRVN=22 | 8-bit code=b8    CALOUT=1
ODT DRVN=21 | 8-bit code=3d    CALOUT=1
ODT DRVN=20 | 8-bit code=8f    CALOUT=1
ODT DRVN=19 | 8-bit code=4f    CALOUT=1
ODT DRVN=18 | 8-bit code=93    CALOUT=1
ODT DRVN=17 | 8-bit code=35    CALOUT=1
ODT DRVN=16 | 8-bit code=4d    CALOUT=1
ODT DRVN=15 | 8-bit code=1e    CALOUT=1
ODT DRVN=14 | 8-bit code=17    CALOUT=1
ODT DRVN=13 | 8-bit code=89    CALOUT=1
ODT DRVN=12 | 8-bit code=46    CALOUT=0
ODT DRVN calibration OK! DRVN=12
   */
}

//-------------------------------------------------------------------------
/** DramcImpedanceCalApply
 *  TX/RX OCD/ODT impedance calibration and apply to driving registers.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
void DramcImpedanceCalApply(void)
{
    U32 u4PHY_BASE_ADDR, u4value; 
    U8 uctx_ocd_drvp, uctx_ocd_drvn, ucrx_odt_drvp, ucrx_odt_drvn;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG0);
    }
#if 0 
    else
    {
        u4PHY_BASE_ADDR = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG0);
    } 
#endif

    DramcImpedanceCalTxOcd();
    DramcImpedanceCalRxOdt();

    //read TX OCD P/N DRV
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe4);
    uctx_ocd_drvp = (U8) mcGET_FIELD(u4value, 0x000000ff, 0);
    uctx_ocd_drvn = (U8) mcGET_FIELD(u4value, 0x0000ff00, 8);

    //read RX ODT P/N DRV
    u4value = ucDram_Register_Read(u4PHY_BASE_ADDR|0xe0);
    ucrx_odt_drvp = (U8) mcGET_FIELD(u4value, 0x000000ff, 0);
    ucrx_odt_drvn = (U8) mcGET_FIELD(u4value, 0x0000ff00, 8);
    mcSHOW_DBG_MSG2("ocd_drvp:0x%x ocd_drvn:0x%x odt_drvp:0x%x odt_drvn:0x%x\n", uctx_ocd_drvp, uctx_ocd_drvn, ucrx_odt_drvp, ucrx_odt_drvn);

    DramcDqDriving(uctx_ocd_drvp, uctx_ocd_drvn, ucrx_odt_drvp, ucrx_odt_drvn);
    DramcDqsDriving(uctx_ocd_drvp, uctx_ocd_drvn, ucrx_odt_drvp, ucrx_odt_drvn);
    DramcDqmDriving(uctx_ocd_drvp, uctx_ocd_drvn, ucrx_odt_drvp, ucrx_odt_drvn);
    // CLK driving is different from others (based on waveform result), may need offset?!
    DramcClkDriving(uctx_ocd_drvp, uctx_ocd_drvn);
#if SUPPORT_8BIT
	uctx_ocd_drvp = 0xf3;
	uctx_ocd_drvn = 0xf3;
#endif
    DramcCaDriving(uctx_ocd_drvp, uctx_ocd_drvn);

}
#endif

#ifdef DRAM_WRITE_LEVELING_CAL
//-------------------------------------------------------------------------
/** DramcWriteLeveling
 *  start Write Leveling Calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
void DramcWriteLeveling(void)
{
    U32 u4value, u4temp, u4dq_o1;
    U32 u4prv_register_1dc, u4prv_register_044, u4prv_register_0e4, u4prv_register_07c, u4prv_register_0d8, u4prv_register_340;
    U8 ucdqs_final_delay[DQS_NUMBER], ucsample_status[DQS_NUMBER], ucdq_o1_perbyte[DQS_NUMBER], uc_count[DQS_NUMBER];
    U8 ii, byte_i, ucdq_o1_perbyte_pre[DQS_NUMBER];
	//U8 ucsample_count;

   
    // backup mode settings
    u4prv_register_1dc = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DRAMC_PD_CTRL));
    u4prv_register_044 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3));
    u4prv_register_0e4 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4));
    u4prv_register_07c = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL));
    u4prv_register_0d8 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY));
    u4prv_register_340 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV));

	//write leveling mode initialization
    //disable auto refresh: REFCNT_FR_CLK = 0 (0x1dc[23:16]), ADVREFEN = 0 (0x44[30])
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DRAMC_PD_CTRL));
    mcCLR_MASK(u4value, MASK_DRAMC_PD_CTRL_REFCNT_FR_CLK);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DRAMC_PD_CTRL), u4value);

    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3));
    mcCLR_BIT(u4value, POS_TEST2_3_ADVREFEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4value);

    //Make CKE fixed at 1 (Put this before issuing MRS): CKEFIXON = 1 (0xe4[2])
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4));
    mcSET_BIT(u4value, POS_PADCTL4_CKEFIXON);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4value);

    //Enable Write ODT: WOEN = 1 (0x7c[3])
    //may no need to set here, initial value
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL));
    mcSET_BIT(u4value, POS_DDR2CTL_WOEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4value);

    //ODT, DQIEN fixed at 1; FIXODT = 1 (0xd8[23]), FIXDQIEN = 1111 (0xd8[15:12])
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY));
    mcSET_BIT(u4value, POS_MCKDLY_FIXODT);
    mcSET_MASK(u4value, MASK_MCKDLY_FIXDQIEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY), u4value);

    //Enable SMT_EN: chA-> 0x104c[31:16] / 0x124c[31:16]; chB-> 0x129c[31:16] / 0x144c[31:16]
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19));
        mcSET_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19));
        mcSET_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19), u4value);
    }
#if 0 
    else
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39));
        mcSET_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19));
        mcSET_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19), u4value);
    }
 
#endif

    //Set {R_DQS_B3_G R_DQS_B2_G R_DQS_B1_G R_DQS_B0_G}=~DA_TX_CMDACLK_D*X : 0x340[4:1]
    //Enable Write leveling: 0x340[0]
    u4temp = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET));
    u4temp = mcGET_FIELD(u4temp, 0x0000000f, 0);
    u4temp = (~u4temp)&0xf;

    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV));
    mcSET_FIELD(u4value, u4temp, MASK_WRLEV_DQS_Bx_G, POS_WRLEV_DQS_Bx_G);
    mcSET_BIT(u4value, POS_WRLEV_WRITE_LEVEL_EN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV), u4value);

    mcDELAY_us(1);

    //issue MR1 to enable write leveling
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), 0x00002084);
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD));
    mcSET_BIT(u4value, POS_SPCMD_MRWEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), u4value);
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_SPCMD_MRWEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), u4value);

    //wait 10 XTAL cycles after issuing MR1, 1ms here
    mcDELAY_us(1);

    //Proceed write leveling...
    //Initilize sw parameters
    for (ii=0; ii < (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); ii++)
    {
        ucsample_status[ii] = 0;
        ucdqs_final_delay[ii] = 0;
        ucdq_o1_perbyte_pre[ii] = 0;
		uc_count[ii] =0;
    }
    //used for early break
    //ucsample_count = 0;

    mcSHOW_DBG_MSG2("=================================\n");
    mcSHOW_DBG_MSG2("\n   dramc_write_leveling_swcal\n");
    mcSHOW_DBG_MSG2("=================================\n");


#if 1
    mcSHOW_DBG_MSG("delay  byte0  byte1  byte2  byte3\n");
#else
    if (TCMGET_DATA_WIDTH() == DATA_WIDTH_8BIT)
    {
        mcSHOW_DBG_MSG2("delay  byte0\n");		
    }
    else
    {
        mcSHOW_DBG_MSG2("delay  byte0  byte1\n");	
    }
#endif
    mcSHOW_DBG_MSG("----------------------\n");



    for (ii=0; ii<128; ii = ii + 2)
    {
    #if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
	#endif
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
            mcSET_FIELD(u4value, ii, 0x0000007f, 0);
            mcSET_FIELD(u4value, ii, 0x007f0000, 16);        
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);

            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, ii, 0x0000007f, 0);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
            mcSET_FIELD(u4value, ii, 0x007f0000, 16);        
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);
        }
	#if 0 
        else
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
            mcSET_FIELD(u4value, ii, 0x0000007f, 0);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);

            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
            mcSET_FIELD(u4value, ii, 0x007f0000, 16);        
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);

            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
            mcSET_FIELD(u4value, ii, 0x0000007f, 0);
            mcSET_FIELD(u4value, ii, 0x007f0000, 16);        
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);
        }
     
    #endif

        //wait for PI settling (100 XTAL cycles), 1ms here
        //Can be ignored by only 1-step tuning (need to delay if large step) by SP
        //#after 1
        mcDELAY_us(10);
		
        //Trigger DQS pulse, R_DQS_WLEV: 0x340[8] from 1 to 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV));
        mcSET_BIT(u4value, POS_WRLEV_DQS_WLEV);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV), u4value);
        mcCLR_BIT(u4value, POS_WRLEV_DQS_WLEV);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV), u4value);

        //wait 10 XTAL cycles, 1ms here
        mcDELAY_us(1);

        //Read DQ_O1 from register, 0x380 for chA, 0x384 for chB
    #if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
	#endif
        {
            //u4dq_o1 = ucDram_Register_Read(0xF0007380);
			     u4dq_o1 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_O1));
        }
	#if 0 
        else
        {
            //u4dq_o1 = ucDram_Register_Read(0xF000f384);
			     u4dq_o1 = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_O1_B));
        }
    
    #endif
		
#ifdef DDR_CHA_4BIT_SWAP
        u4dq_o1 = DramcDQ4bitSwap(u4dq_o1);
#endif

#if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
        {
#ifdef DDR_CHA_4BIT_SWAP
            //DQS0: DQ3, DQS1: DQ13, DQS2: DQ17. DQS3: DQ28
            ucdq_o1_perbyte[0] = (U8)((u4dq_o1>>3)&0x00000001);
            ucdq_o1_perbyte[1] = (U8)((u4dq_o1>>13)&0x00000001);
            ucdq_o1_perbyte[2] = (U8)((u4dq_o1>>17)&0x00000001);
            ucdq_o1_perbyte[3] = (U8)((u4dq_o1>>28)&0x00000001);
#else
            //DQS0: DQ3, DQS1: DQ15, DQS2: DQ17. DQS3: DQ29
            ucdq_o1_perbyte[0] = (U8)((u4dq_o1>>3)&0x00000001);
            ucdq_o1_perbyte[1] = (U8)((u4dq_o1>>15)&0x00000001);
            ucdq_o1_perbyte[2] = (U8)((u4dq_o1>>17)&0x00000001);
            ucdq_o1_perbyte[3] = (U8)((u4dq_o1>>29)&0x00000001);
#endif
        }
#if 0
        else
        {
            //DQS0: DQ3, DQS1: DQ13, DQS2: DQ18. DQS3: DQ28
            ucdq_o1_perbyte[0] = (U8)((u4dq_o1>>1)&0x00000001);
            ucdq_o1_perbyte[1] = (U8)((u4dq_o1>>15)&0x00000001);
            ucdq_o1_perbyte[2] = (U8)((u4dq_o1>>17)&0x00000001);
            ucdq_o1_perbyte[3] = (U8)((u4dq_o1>>28)&0x00000001);
        }
 
#endif

    #if 1
        mcSHOW_DBG_MSG("%3d %02x %02x %02x %02x\n", ii, u4dq_o1&0x000000ff, (u4dq_o1&0x0000ff00)>>8, (u4dq_o1&0x00ff0000)>>16, (u4dq_o1&0xff000000)>>24);
    #else
        if (TCMGET_DATA_WIDTH() == DATA_WIDTH_8BIT)
        {
            mcSHOW_DBG_MSG("%3d %02x\n", ii, u4dq_o1&0x000000ff);
			mcSHOW_DBG_VAL_FOR_SIMULATION(ii);		
       }
        else
        {
            mcSHOW_DBG_MSG("%3d %02x %02x\n", ii, u4dq_o1&0x000000ff, (u4dq_o1&0x0000ff00)>>8);
	
        }
    #endif

        //sample from 0 to 1
        for (byte_i = 0; byte_i < (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER);  byte_i++)
        {
        #if 1
            if ((ucsample_status[byte_i]==0) && (ucdq_o1_perbyte[byte_i]==1))
            {
                ucsample_status[byte_i] = 0;//1-->1
#if 0
                uc_count[byte_i]++;
                if(uc_count[byte_i] > 24)
				{
				    ucsample_status[byte_i] = 3;//0-->1
				    ucdqs_final_delay[byte_i] = 0;
					
				}
#endif
                //used for early break
                //ucsample_count++;
                //record delay value
                //ucdqs_final_delay[byte_i] = 0;
            }
            else if ((ucsample_status[byte_i]==0) && (ucdq_o1_perbyte[byte_i]==0) && (ucdq_o1_perbyte_pre[byte_i] == 0))
            {
                ucsample_status[byte_i] = 1;//1-->0
            }
            else if ((ucsample_status[byte_i]==1) && (ucdq_o1_perbyte[byte_i]==1) && (ucdq_o1_perbyte_pre[byte_i] == 1))
            {
                ucsample_status[byte_i] = 3;//0-->1
                //used for early break
                //ucsample_count++;
                //record delay value
                ucdqs_final_delay[byte_i] = ii - 2;
            }
            else if((ucsample_status[byte_i]==1) && (ucdq_o1_perbyte[byte_i]==0) && (ucdq_o1_perbyte_pre[byte_i] == 0))
            {
                ucsample_status[byte_i] = 2;//0-->0
            }
            else if((ucsample_status[byte_i]==2) && (ucdq_o1_perbyte[byte_i]==1) && (ucdq_o1_perbyte_pre[byte_i] == 1))
            {
                ucsample_status[byte_i] = 3;//0-->1
                //used for early break
                //ucsample_count++;
                //record delay value
                ucdqs_final_delay[byte_i] = ii - 2;
            }
            
            if(ucdqs_final_delay[byte_i]>40)
            {
                ucdqs_final_delay[byte_i] = 0;
            }
            ucdq_o1_perbyte_pre[byte_i] = ucdq_o1_perbyte[byte_i];
        #else
            if ((ucsample_status[byte_i]==0) && (ucdq_o1_perbyte[byte_i]==1))
            {
                ucsample_status[byte_i] = 2;
                //used for early break
                //ucsample_count++;
                //record delay value
                ucdqs_final_delay[byte_i] = 0;
            }
            else if ((ucsample_status[byte_i]==0) && (ucdq_o1_perbyte[byte_i]==0))
            {
                ucsample_status[byte_i] = 1;
            }
            else if ((ucsample_status[byte_i]==1) && (ucdq_o1_perbyte[byte_i]==1))
            {
                ucsample_status[byte_i] = 2;
                //used for early break
                //ucsample_count++;
                //record delay value
                ucdqs_final_delay[byte_i] = ii;
            }
        #endif
        }
        //early break, may be marked for debug use
        //if (ucsample_count==(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER))
        //{
        //    break;
        //}        
    }

    mcDELAY_us(1);	

    //mcSHOW_DBG_MSG("pass bytecount = %d\n", ucsample_count);
    mcSHOW_DBG_MSG2("byte_i    status    best delay\n");
#if SUPPORT_8BIT
    u4temp = (ucdqs_final_delay[0] + ucdqs_final_delay[1]) / 2;
    ucdqs_final_delay[0] = u4temp;
    ucdqs_final_delay[1] = u4temp;
    u4temp = (ucdqs_final_delay[2] + ucdqs_final_delay[3]) / 2;
    ucdqs_final_delay[2] = u4temp;
    ucdqs_final_delay[3] = u4temp;
#endif
    for (byte_i = 0; byte_i < (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER);  byte_i++)
    {
        mcSHOW_DBG_MSG2("%d    %d    %d\n", byte_i, ucsample_status[byte_i], ucdqs_final_delay[byte_i]);


	}    
    mcSHOW_DBG_MSG("======================\n");	

    // write leveling done, mode settings recovery if necessary
    // recover MR1, refer to initial value (with dynamic ODT: disable RTT_Nom off, 2013/1/3)

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MRS), DEFAULT_MR1_VALUE);

    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD));
    mcSET_BIT(u4value, POS_SPCMD_MRWEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), u4value);
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_SPCMD_MRWEN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_SPCMD), u4value);

    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_WLEV), u4prv_register_340);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DRAMC_PD_CTRL), u4prv_register_1dc);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4prv_register_044);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PADCTL4), u4prv_register_0e4);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), u4prv_register_07c);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY), u4prv_register_0d8);

    
    //Disable SMT_EN: chA-> 0x104c[31:16] / 0x124c[31:16]; chB-> 0x129c[31:16] / 0x144c[31:16]
    //for power saving
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19));
        mcCLR_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19));
        mcCLR_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19), u4value);
    }
#if 0
    else
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39));
        mcCLR_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19));
        mcCLR_MASK(u4value, 0xffff0000);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19), u4value);
    }
 
#endif

    // set best delay value
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
        mcSET_FIELD(u4value, ucdqs_final_delay[1], 0x0000007f, 0);
        mcSET_FIELD(u4value, ucdqs_final_delay[0], 0x007f0000, 16);        
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
        mcSET_FIELD(u4value, ucdqs_final_delay[2], 0x0000007f, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
        mcSET_FIELD(u4value, ucdqs_final_delay[3], 0x007f0000, 16);        
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);
    }
#if 0
    else
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
        mcSET_FIELD(u4value, ucdqs_final_delay[0], 0x0000007f, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
        mcSET_FIELD(u4value, ucdqs_final_delay[1], 0x007f0000, 16);        
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
        mcSET_FIELD(u4value, ucdqs_final_delay[3], 0x0000007f, 0);
        mcSET_FIELD(u4value, ucdqs_final_delay[2], 0x007f0000, 16);        
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);
    }
 
#endif

    // log example
/*
===================================================================

                dramc_write_leveling_swcal
                apply=1 channel=2(2:cha, 3:chb)
===================================================================
delay  byte0  byte1  byte2  byte3
-----------------------------
  0    0    0    0    1
  1    0    0    0    1
  2    0    0    1    1
  3    0    0    1    1
  4    0    0    1    1
  5    0    0    1    1
  6    0    0    1    1
  7    0    0    1    1
  8    0    0    1    1
  9    0    0    1    1
 10    0    0    1    1
 11    1    1    1    1
pass bytecount = 4
byte_i    status    best delay
0         2         11
1         2         11
2         2         2
3         2         0
*/
}
#endif

#ifdef DRAM_MiockJmeter
//-------------------------------------------------------------------------
/** DramcMiockJmeter
 *  start MIOCK jitter meter.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param block_no         (U8): block 0 or 1.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
void DramcMiockJmeter(void)
{
    U8 ucsearch_state, ucdqs_dly, fgcurrent_value, fginitial_value, ucstart_period, ucend_period, ucdqs_predly;
    U32 u4value, u4addr_array[4], u4sample_cnt, u4ones_cnt, u4frequency_hex;
    U16 u2real_freq, u2real_period, u2delay_cell_ps;
    U32 u4mempll_prediv_hex, u2frequency1,u4real_freq_fraction; 
    U32 u4mempll_predivider[3] = {1, 2, 4};
    U32 u4flag = 0;
    
    fginitial_value = 0;
    ucstart_period = 0;
    ucend_period = 0;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18);
        u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19);
    }
#if 0
    else
    {
        u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38);
        u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39);
    }
#endif

    u4addr_array[2] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_TOGGLE_CNT);
    u4addr_array[3] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQS_ERR_CNT);

    //Enable DQ eye scan
    //RG_??_RX_EYE_SCAN_EN
    //RG_??_RX_VREF_EN 
    //RG_??_RX_VREF_OP_EN 
    //RG_??_RX_SMT_EN
    u4value = ucDram_Register_Read(u4addr_array[0]);
    u4value = u4value | 0x0000e000;
    ucDram_Register_Write(u4addr_array[0], u4value);

    u4value = ucDram_Register_Read(u4addr_array[1]);
    u4value = u4value | 0xffff0000;
    ucDram_Register_Write(u4addr_array[1], u4value);

    //Enable MIOCK jitter meter mode (RG_??_RX_DQS_MIOCK_SEL=1, RG_RX_MIOCK_JIT_EN=1)
    //RG_??_RX_DQS_MIOCK_SEL=1
    u4value = ucDram_Register_Read(u4addr_array[0]);
    u4value = u4value | 0x00001000;
    ucDram_Register_Write(u4addr_array[0], u4value);

    //RG_RX_MIOCK_JIT_EN=1
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
    mcSET_BIT(u4value, POS_DCBLN_RX_MIOCK_JIT_EN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

    ucsearch_state = 0;

    //Disable DQ eye scan (b'1), for counter clear
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
    mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

    for (ucdqs_dly=0; ucdqs_dly<128; ucdqs_dly++)
    {
        if(u4flag)
        {
            ucdqs_dly = ucdqs_predly;
        }
        //Set DQS delay (RG_??_RX_DQS_EYE_DLY)
        u4value = ucDram_Register_Read(u4addr_array[0]);
        mcSET_FIELD(u4value, ucdqs_dly, 0x007f0000, 16);
        ucDram_Register_Write(u4addr_array[0], u4value);

        //Reset eye scan counters (reg_sw_rst): 1 to 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcSET_BIT(u4value, POS_DCBLN_REG_SW_RST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
        mcCLR_BIT(u4value, POS_DCBLN_REG_SW_RST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        //Enable DQ eye scan (b'1)
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcSET_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        // 2ns/sample, here we delay 1ms about 500 samples
        mcDELAY_us(1000);

        //Disable DQ eye scan (b'1), for counter latch
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

		//mcDELAY_us(1000);

        //Read the counter values from registers (toggle_cnt*, dqs_err_cnt*)
        u4sample_cnt = ucDram_Register_Read(u4addr_array[2]);
        u4ones_cnt = ucDram_Register_Read(u4addr_array[3]);

        mcSHOW_DBG_MSG("%3d | %8x --%8x \n", ucdqs_dly, u4sample_cnt, u4ones_cnt); 

        if(u4ones_cnt == 0 && u4sample_cnt == 0)
        {
            u4flag++;
            if(u4flag == 1)
            {
                ucdqs_predly = ucdqs_dly;
                continue;
            }
        }
        u4flag = 0;
        //change to boolean value
        if (u4ones_cnt < (u4sample_cnt/2))
        {
            fgcurrent_value = 0;
        }
        else
        {
            fgcurrent_value = 1;
        }
        
        if (ucsearch_state==0)
        {
            //record initial value at the beginning
            fginitial_value = fgcurrent_value;            
            ucsearch_state = 1;
        }
        else if (ucsearch_state==1)
        {
            // check if change value
            if (fgcurrent_value != fginitial_value)
            {
                // start of the period
                fginitial_value = fgcurrent_value;
                ucstart_period = ucdqs_dly;
                ucsearch_state = 2;
            }
        }
        else if (ucsearch_state==2)
        {
            // check if change value
            if (fgcurrent_value != fginitial_value)
            {
                fginitial_value = fgcurrent_value;
                ucsearch_state = 3;
            }
        }
        else if (ucsearch_state==3)
        {
            // check if change value
            if (fgcurrent_value != fginitial_value)
            {
                // end of the period, break the loop
                ucend_period = ucdqs_dly;
                ucsearch_state = 4;
                break;
            }
        }
        else
        {
            //nothing
        }
    }

    //Calculate 1 delay cell = ? ps
    // 1T = ? delay cell
	TCMSET_DLYCELL_PERT(ucend_period-ucstart_period);
    // 1T = ? ps
    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG40));
    u4frequency_hex = mcGET_FIELD(u4value, 0x00007f00, 8);
	u4mempll_prediv_hex = mcGET_FIELD(u4value, 0x00000003, 0);
    u2real_freq = (U16) ((u4frequency_hex+1)*(XTAL_MHZ/u4mempll_predivider[u4mempll_prediv_hex])*2);
#if SUPPORT_MEMPLL_FRACT
    u2frequency1 = (TCMGET_DDR_CLK()/BASE_DDR_CLK)/2/ 2;   
    u4real_freq_fraction = (U32)u2real_freq+((U32)u2frequency1-(u4frequency_hex + 1)*XTAL_MHZ)/u4mempll_predivider[u4mempll_prediv_hex]*2;
	u2real_freq          =  u4real_freq_fraction;
#endif
    u2real_period = (U16) ((U32)1000000/(U32)u2real_freq);
    //calculate delay cell time
    u2delay_cell_ps = u2real_period / TCMGET_DLYCELL_PERT;

    //RG_RX_MIOCK_JIT_EN=0
    //u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), &u4value);
    //mcCLR_BIT(u4value, POS_DCBLN_RX_MIOCK_JIT_EN);
    //ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

    mcSHOW_DBG_MSG2("========================\n");
    mcSHOW_DBG_MSG2("    MIOCK jitter meter \n");
    mcSHOW_DBG_MSG2("========================\n");
    mcSHOW_DBG_MSG2("1T = (%d-%d) = %d delay cells\n", ucend_period, ucstart_period, TCMGET_DLYCELL_PERT);
    mcSHOW_DBG_MSG("Clock frequency = %d MHz, Clock period = %d ps, 1 delay cell = %d ps\n", u2real_freq, u2real_period, u2delay_cell_ps);

	UNUSED(u2delay_cell_ps);
 	
    // log example
    /*
  0 |   d91b60 --       0
  1 |   d91b71 --       0
  2 |   e018e4 --       0
  3 |   d9228c --       0
  4 |   d90d08 --       0
  5 |   d91434 --       0
  6 |   d9568b --       0
  7 |   e030cc --       0
  8 |   d91bf9 --       0
  9 |   d8dfad --       0
 10 |   e0197d --       0
 11 |   d92f2a --       0
 12 |   d91bf9 --  cc0f01
 13 |   d902df --  d902df
 14 |   d91a2e --  d91a2e
 15 |   e01ac0 --  e01ac0
 16 |   d92e80 --  d92e80
 17 |   d908d9 --  d908d9
 18 |   e01aaf --  e01aaf
 19 |   d90a0b --  d90a0b
 20 |   d91f07 --  d91f07
 21 |   d91302 --  d91302
 22 |   d92d4e --  d92d4e
 23 |   e01be1 --  e01be1
 24 |   d8d7e8 --  d8d7e8
 25 |   d92468 --  d92468
 26 |   e021db --  e021db
 27 |   d94cfb --  d94cfb
 28 |   d9291f --  d9291f
 29 |   d94701 --  d94701
 30 |   d913ac --  d913ac
 31 |   e01be1 --  e01be1
 32 |   d91d2b --  d91d2b
 33 |   d90c6f --  d90c6f
 34 |   e01262 --  e01262
 35 |   d923be --    6bdc
 36 |   d91b60 --       0
 37 |   d92fb2 --       0
 38 |   d92c1c --       0
 39 |   dff34e --       0
 40 |   d90c6f --       0
 41 |   d91269 --       0
 42 |   e03891 --       0
 43 |   d92e80 --       0
 44 |   d92de7 --       0
 45 |   d8ca18 --       0
 46 |   d923be --       0
 47 |   dff937 --       0
 48 |   d90d19 --       0
 49 |   d92457 --       0
 50 |   e014b5 --       0
 51 |   d92cb5 --       0
 52 |   d92e80 --       0
 53 |   e015f8 --       0
 54 |   d927ed --       0
 55 |   e012ea --       0
 56 |   d91bf9 --       0
 57 |   d92e80 --       0
 58 |   e01e56 --       0
 59 |   d9291f --       0
 60 |   d91c92 --    8fc5
 61 |   e00a8c --  e00a8c
========================================================================
    MIOCK jitter meter - channel=2(2:cha, 3:chb), block_0
========================================================================
1T = (61-12) = 49 delay cells
Clock frequency = 918 MHz, Clock period = 1089 ps, 1 delay cell = 22 ps
   */
}
#endif

#ifdef DRAM_GATING_SCAN
//-------------------------------------------------------------------------
/** DramcRxdqsGatingCal (v2 version)
 *  start the dqsien software calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  bl_type         (U8): Burst length type. 4 or 8 (4 is just used for lpddr2 ,if bl_type is 4 ,we will change BL to 4 to do this calibraion ,write back to 8 when exit).
 *  @param  test2_0         (U32): 16bits,set pattern1 [31:24] and set pattern0 [23:16].
 *  @param  test2_1         (U32): 28bits,base address[27:0].
 *  @param  test2_2         (U32): 28bits,offset address[27:0].
 *  @param  gold_counter    (U32): the dqs counter value base on test2_1 and test2_2 you give.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @param  test_pattern    (DRAM_TEST_PATTERN_T): used test pattern. 0: ISI, 1: AUDIO, 2: TESTPAT4, 3: TESTPAT4_3
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcRxdqsGatingCal(void)
{
    U8 ucmin, ucmax;
    U32 u4value, u4all_result_R, u4all_result_F;
    U8 ucpass_begin[DQS_NUMBER], ucpass_count[DQS_NUMBER];
    U8 ucmin_coarse_tune2T[DQS_NUMBER]={0}, ucmin_coarse_tune0p5T[DQS_NUMBER]={0}, ucmin_fine_tune[DQS_NUMBER]={0};
    U8 ucpass_count_1[DQS_NUMBER]={0}, ucmin_coarse_tune2T_1[DQS_NUMBER]={0}, ucmin_coarse_tune0p5T_1[DQS_NUMBER]={0}, ucmin_fine_tune_1[DQS_NUMBER]={0};
    U8 dqs_i, ucRX_DLY_DQSIENSTB_LOOP, ucdly_coarse_2T, ucdly_coarse_0p5T, ucdly_fine_xT;
    U8 ucdqs_result_R, ucdqs_result_F, ucerr_cnt, uctmp_offset, uctmp_value;
    U8 ucbest_fine_tune[DQS_NUMBER], ucbest_coarse_tune0p5T[DQS_NUMBER], ucbest_coarse_tune2T[DQS_NUMBER];
    U8 ucfinal_fine_tune[DQS_NUMBER], ucfinal_coarse_tune0p5T[DQS_NUMBER], ucfinal_coarse_tune2T;
    U16 u2one_hot_dly;
    int u4Avg[DQS_NUMBER],u4Min[DQS_NUMBER],u4Max[DQS_NUMBER];
    U32 test2_0 = 0xaa550000, test2_1 = DEFAULT_TEST2_1_CAL, test2_2 = 0x00000406;
    U8 fgfail = DRAM_CALIBRATION_PASS;
	
    //power up dqs when in idle state when gating calibration
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        //RG_A1_TX_ARDQS0_R75KP=0; 0x5801c[21]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7));
        mcCLR_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), u4value);
        //RG_A1_TX_ARDQS1_R75KP=0; 0x58020[5]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8));
        mcCLR_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), u4value);
        //RG_A2_TX_ARDQS2_R75KP=0; 0x5821c[21]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7));
        mcCLR_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7), u4value);
        //RG_A2_TX_ARDQS3_R75KP=0; 0x58220[5]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8));
        mcCLR_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8), u4value);		
    }
#if 0
    else
    {
        //RG_B1_TX_BRDQS0_R75KP=0; 0x5826c[21]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27));
        mcCLR_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27), u4value);
        //RG_B1_TX_BRDQS1_R75KP=0; 0x58270[5]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28));
        mcCLR_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28), u4value);
        //RG_B2_TX_BRDQS2_R75KP=0; 0x5841c[21]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7));
        mcCLR_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7), u4value);
        //RG_B2_TX_BRDQS3_R75KP=0; 0x58420[5]=0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8));
        mcCLR_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), u4value);		
    }
 
#endif

    // begin 
    // enable RG_RX_*_DLY_DQSIENSTB_*_SWEN
    // Use JMeter to find delay cell measurement, ?T
    // transfer all coarse/fine tune steps into the same unit (say, ?T)
    // scan all possible range and find pass window and middle value per byte
    // since coarse_2T is per channel, select the min value between 4 bytes, use coarse_0.5T and fine_xT to get the best value
    // disable RG_RX_*_DLY_DQSIENSTB_*_SWEN (no need for A60807 since there is no HW gating)

    //enable RG_RX_*_DLY_DQSIENSTB_*_SWEN (switch to SW fine-tuning)
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        //RG_A1_RX_DLY_DQSIENSTB_0_SWEN: 0x103c[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15), u4value);
        //RG_A1_RX_DLY_DQSIENSTB_1_SWEN: 0x1040[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16), u4value);
        //RG_A2_RX_DLY_DQSIENSTB_2_SWEN: 0x123c[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15), u4value);
        //RG_A2_RX_DLY_DQSIENSTB_3_SWEN: 0x1240[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16), u4value);		
    }
#if 0
    else
    {
        //RG_B1_RX_DLY_DQSIENSTB_0_SWEN: 0x128c[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35), u4value);
        //RG_B1_RX_DLY_DQSIENSTB_1_SWEN: 0x1290[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36), u4value);
        //RG_B2_RX_DLY_DQSIENSTB_2_SWEN: 0x143c[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15), u4value);
        //RG_B2_RX_DLY_DQSIENSTB_3_SWEN: 0x1440[23] = 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16));
        mcSET_BIT(u4value, 23);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16), u4value);		
    }
 
#endif

    //Initialize variables
    for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
    {
        ucpass_begin[dqs_i] = 0;
        ucpass_count[dqs_i] = 0;
    }

    //coarse tune delay is (reg_DLY_DQSIENSTB_*)x2 + (RG_**_RX_DLY_DQSIENSTB_*)x0.5  DRAM clock cycle.
    // loop number of 2T and 0.5T coarse tune are defined @ pi_api.h
    //fine tune delay, depends on JMeter result
    ucRX_DLY_DQSIENSTB_LOOP = TCMGET_DLYCELL_PERT / 2;

    mcSHOW_DBG_MSG("===================================================\n");
    mcSHOW_DBG_MSG("x = dqs result \ny = coarse_2T  coarse_0.5T  finetune\n");
    mcSHOW_DBG_MSG("---------------------------------------------------\n"); 

#if 1
    mcSHOW_DBG_MSG("y  |  dqs0f   dqs0r   dqs1f   dqs1r   dqs2f   dqs2r   dqs3f   dqs3r\n");
#else
    if (TCMGET_DATA_WIDTH() == DATA_WIDTH_8BIT)
    {
        mcSHOW_DBG_MSG2("y  |  dqs0f   dqs0r\n");	
    }
    else
    {
        mcSHOW_DBG_MSG2("y  |  dqs0f   dqs0r   dqs1f   dqs1r\n");
    }
#endif
    mcSHOW_DBG_MSG("--------------------------------------------------\n");

    
    /*[Bring up] scan all gating steps*/
    for (ucdly_coarse_2T=0; ucdly_coarse_2T<=DLY_DQSIENSTB_LOOP; ucdly_coarse_2T++)
    //for (ucdly_coarse_2T=4; ucdly_coarse_2T<=DLY_DQSIENSTB_LOOP; ucdly_coarse_2T++)
    {
    #if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
	#endif
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET));
            mcSET_FIELD(u4value, ucdly_coarse_2T, 0x00000700, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), u4value);	
        }
	#if 0
        else
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET));
            mcSET_FIELD(u4value, ucdly_coarse_2T, 0x00007000, 12);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), u4value);	
        }
     
    #endif

        for (ucdly_coarse_0p5T=1; ucdly_coarse_0p5T<=RX_DQS_CTL_LOOP; ucdly_coarse_0p5T++)
        {
            u2one_hot_dly = (U16) 1<<(ucdly_coarse_0p5T-1);

        #if 0
            if (!IS_DRAM_CHANNELB_ACTIVE())
		#endif
            {
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG14));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG14), u4value);

                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15), u4value);

                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG14));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG14), u4value);

                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15), u4value);
            }
        #if 0
            else
            {
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG34));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG34), u4value);

                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35), u4value);

                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG14));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG14), u4value);

                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15));
                mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15), u4value);
            }
         
        #endif
        
            for (ucdly_fine_xT=0; ucdly_fine_xT<=ucRX_DLY_DQSIENSTB_LOOP; ucdly_fine_xT++)
            {
            #if 0
                if (!IS_DRAM_CHANNELB_ACTIVE())
            #endif
                {
                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16), u4value);
                }
			#if 0 
                else
                {
                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16));
                    mcSET_FIELD(u4value, ucdly_fine_xT, 0x003f0000, 16);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16), u4value);
                }
             
            #endif

                //ok we set a coarse/fine tune value already
                //reset the read counters in both DRAMC and DDRPHY (R_DMPHYRST: 0x0f0[28])
                //enable test engine
                //record the counter value

                //reset phy R_DMPHYRST: 0xf0[28] 
                // 0x0f0[28] = 1 -> 0
                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
                mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
                //delay 10ns, 1ms here
                mcDELAY_us(1);
                mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

                // read data counter reset
                // 0x0f4[25] = 1 -> 0
                u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
                mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
                //delay 10ns, 1ms here
                mcDELAY_us(1);                
                mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
                ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

            #if 0
                if(!IS_DRAM_CHANNELB_ACTIVE())
			      #endif
                {
                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17));
                    mcCLR_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);
                    mcDELAY_us(1);
                    mcSET_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17));
                    mcCLR_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
                    mcDELAY_us(1);
                    mcSET_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
                }
           #if 0 
                else
                {
                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37));
                    mcCLR_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);
                    mcDELAY_us(1);
                    mcSET_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);

                    u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17));
                    mcCLR_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
                    mcDELAY_us(1);
                    mcSET_BIT(u4value, 0);
                    ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
                }
            
            #endif
                
                // enable TE2, audio pattern
                DramcEngine2(TE_OP_READ_CHECK, test2_0, test2_1, test2_2);

                u4all_result_R = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_STBENERR_R));
                u4all_result_F = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_STBENERR_F));

#ifdef DDR_CHA_4BIT_SWAP
                u4all_result_R = DramcDQ4bitSwap(u4all_result_R);
                u4all_result_F = DramcDQ4bitSwap(u4all_result_F);
#endif				
                //mcSHOW_DBG_MSG(("--------------------------------\n"));
                //mcSHOW_DBG_MSG(("%2d  %2d  %2d | %8x  %8x\n", ucdly_coarse_2T, ucdly_coarse_0p5T, ucdly_fine_xT, u4all_result_R, u4all_result_F));

            #if 1
                mcSHOW_DBG_MSG("%2d  %2d  %2d  |  %2x  %2x  %2x  %2x  %2x  %2x  %2x  %2x  \n", ucdly_coarse_2T, ucdly_coarse_0p5T, ucdly_fine_xT, \
                                                     (u4all_result_F)&0xff,         (u4all_result_R)&0xff,         (u4all_result_F>>8)&0xff,   (u4all_result_R>>8)&0xff, \
                                                     (u4all_result_F>>16)&0xff, (u4all_result_R>>16)&0xff, (u4all_result_F>>24)&0xff, (u4all_result_R>>24)&0xff);
            #else
                if (TCMGET_DATA_WIDTH() == DATA_WIDTH_8BIT)
                {
                    mcSHOW_DBG_MSG("%2d  %2d  %2d  |  %2x  %2x  \n", ucdly_coarse_2T, ucdly_coarse_0p5T, ucdly_fine_xT, (u4all_result_F)&0xff, (u4all_result_R)&0xff);


				}
                else
                {
                    mcSHOW_DBG_MSG("%2d  %2d  %2d  |  %2x  %2x  %2x  %2x  \n", ucdly_coarse_2T, ucdly_coarse_0p5T, ucdly_fine_xT, \
                                                     (u4all_result_F)&0xff, (u4all_result_R)&0xff, (u4all_result_F>>8)&0xff, (u4all_result_R>>8)&0xff);
	

				}
            #endif

                //find gating window pass range per DQS separatelysd
                for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
                {
                    //get dqs error result
                    ucdqs_result_R = (U8) ((u4all_result_R>>(8*dqs_i))&0x000000ff);
                    ucdqs_result_F = (U8) ((u4all_result_F>>(8*dqs_i))&0x000000ff);

                    //if current tap is pass 
                    if ((ucdqs_result_R==0) && (ucdqs_result_F==0))
                    {
                        if (ucpass_begin[dqs_i]==0)
                        {
                            //no pass tap before , so it is the begining of pass range
                            ucpass_begin[dqs_i] = 1;
                            ucpass_count_1[dqs_i] = 0;
                            ucmin_coarse_tune2T_1[dqs_i] = ucdly_coarse_2T;
                            ucmin_coarse_tune0p5T_1[dqs_i] = ucdly_coarse_0p5T;
                            ucmin_fine_tune_1[dqs_i] = ucdly_fine_xT;                            
                        }

                        if (ucpass_begin[dqs_i]==1)
                        {
                            //incr pass tap number
                            ucpass_count_1[dqs_i]++;
                        }
                    }
                    else
                    {
                        if (ucpass_begin[dqs_i]==1)
                        {
                            //at the end of pass range
                            ucpass_begin[dqs_i] = 0;

                            //save the max range settings, to avoid glitch
                            if (ucpass_count_1[dqs_i] > ucpass_count[dqs_i])
                            {
                                ucmin_coarse_tune2T[dqs_i] = ucmin_coarse_tune2T_1[dqs_i];
                                ucmin_coarse_tune0p5T[dqs_i] = ucmin_coarse_tune0p5T_1[dqs_i];
                                ucmin_fine_tune[dqs_i] = ucmin_fine_tune_1[dqs_i];
                                ucpass_count[dqs_i] = ucpass_count_1[dqs_i];
                            }
                        }
                    }
                }                                 
            }
        }
    }
    
    //reset phy R_DMPHYRST: 0xf0[28] 
    // 0x0f0[28] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
    mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

    // read data counter reset
    // 0x0f4[25] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
    mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);                
    mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

#if 0
    if(!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
    }
#if 0
    else
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
    }
 
#endif

    //check if there is no pass taps for each DQS
    ucerr_cnt = 0;
    for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
    {
        if (ucpass_count[dqs_i]==0)
        {
            mcSHOW_ERROR_CHIP_DisplayString("error, no pass taps in DQS_");
            mcSHOW_ERROR_CHIP_DisplayInteger(dqs_i);
            mcSHOW_ERROR_CHIP_DisplayString(" !!!\n");
            ucerr_cnt++;
			

        }
    }

    if (ucerr_cnt != 0)
    {
		return DRAM_FAIL;
		
    }

    //find center of each byte
    for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
    {
        //note: confirm SoC can use "/" and "%" operations
        uctmp_offset = (ucpass_count[dqs_i]-1)/2;
        uctmp_value = ucmin_fine_tune[dqs_i]+uctmp_offset;
        ucbest_fine_tune[dqs_i] = (U8)uctmp_value%(U8)(ucRX_DLY_DQSIENSTB_LOOP+1);

        uctmp_offset = (U8)uctmp_value / (U8)(ucRX_DLY_DQSIENSTB_LOOP+1);
        //coase tune 0.5T : 1~4 -> 0~3 first, after modulo will recover
        uctmp_value = (ucmin_coarse_tune0p5T[dqs_i]-1)+uctmp_offset;
        ucbest_coarse_tune0p5T[dqs_i] = (uctmp_value%RX_DQS_CTL_LOOP)+1;

        uctmp_offset = uctmp_value/RX_DQS_CTL_LOOP;
        ucbest_coarse_tune2T[dqs_i] = ucmin_coarse_tune2T[dqs_i]+uctmp_offset;
        // may no need to check, it is impossible to exceed if pass_count is correct
        //ucbest_coarse_tune2T[dqs_i] = ((ucbest_coarse_tune2T[dqs_i] > (DLY_DQSIENSTB_MAX)) ? (DLY_DQSIENSTB_MAX) : ucbest_coarse_tune2T[dqs_i]);
    }

    //check if coarse tune 2T the same per channel
    //find min and max
    ucmin = 255;
    ucmax = 0;
    for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
    {
        if (ucbest_coarse_tune2T[dqs_i]<ucmin)
        {
            ucmin = ucbest_coarse_tune2T[dqs_i];
        }

        if (ucbest_coarse_tune2T[dqs_i]>ucmax)
        {
            ucmax = ucbest_coarse_tune2T[dqs_i];
        }
    }

    if (ucmin == ucmax)
    {
        //all DQS are the same
        mcSHOW_DBG_MSG("Best Coarse Tune for 2T are the same...Done!!\n");	
        ucfinal_coarse_tune2T = ucmin;
        for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
        {
            ucfinal_coarse_tune0p5T[dqs_i] = ucbest_coarse_tune0p5T[dqs_i];
            ucfinal_fine_tune[dqs_i] = ucbest_fine_tune[dqs_i];
        }
    }
    else
    {
        //different values between Coarse tune 2T (per channel setting)
        //set final coarse tune 2T to min 
        ucfinal_coarse_tune2T = ucmin;
        for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
        {
            if (ucbest_coarse_tune2T[dqs_i] != ucfinal_coarse_tune2T)
            {
                // 1 x coarse_tune2T = 4 x coarse_tune0p5T
                ucfinal_coarse_tune0p5T[dqs_i] = ucbest_coarse_tune0p5T[dqs_i] + RX_DQS_CTL_LOOP*(ucbest_coarse_tune2T[dqs_i]-ucfinal_coarse_tune2T);
                if (ucfinal_coarse_tune0p5T[dqs_i]>RX_DQS_CTL_MAX)
                {
                    //Exceed max taps of coarse_tune0p5T
                    // 1 x fine_tune = ? x coarse_tune0p5T (need to get from JMeter, assume fine_tune is 0.1T here)
                    ucfinal_fine_tune[dqs_i] = ucbest_fine_tune[dqs_i]+ucRX_DLY_DQSIENSTB_LOOP*(ucfinal_coarse_tune0p5T[dqs_i]-RX_DQS_CTL_MAX);
                    ucfinal_fine_tune[dqs_i] = ((ucfinal_fine_tune[dqs_i] > (RX_DLY_DQSIENSTB_MAX)) ? (RX_DLY_DQSIENSTB_MAX) : ucfinal_fine_tune[dqs_i]);
                    ucfinal_coarse_tune0p5T[dqs_i] = RX_DQS_CTL_MAX;
                }
                else
                {
                    ucfinal_fine_tune[dqs_i] = ucbest_fine_tune[dqs_i];
                }
            }
            else
            {
                ucfinal_coarse_tune0p5T[dqs_i] = ucbest_coarse_tune0p5T[dqs_i];
                ucfinal_fine_tune[dqs_i] = ucbest_fine_tune[dqs_i];
            }
        }
    }

    //switch to hw calibration RG_RX_*_DLY_DQSIENSTB_*_SWEN (no need for A60807)
    //if {[string compare -nocase $channel chA]==0} {
    //    RG_A1_RX_DLY_DQSIENSTB_0_SWEN: 0x103c[23] = 0
    //    RG_A1_RX_DLY_DQSIENSTB_1_SWEN: 0x1040[23] = 0
    //    RG_A2_RX_DLY_DQSIENSTB_2_SWEN: 0x123c[23] = 0
    //    RG_A2_RX_DLY_DQSIENSTB_3_SWEN: 0x1240[23] = 0
    //} elseif {[string compare -nocase $channel chB]==0} {
    //     RG_B1_RX_DLY_DQSIENSTB_0_SWEN: 0x128c[23] = 0
    //     RG_B1_RX_DLY_DQSIENSTB_1_SWEN: 0x1290[23] = 0
    //     RG_B2_RX_DLY_DQSIENSTB_2_SWEN: 0x143c[23] = 0
    //     RG_B2_RX_DLY_DQSIENSTB_3_SWEN: 0x1440[23] = 0
    //}
    for (dqs_i=0; dqs_i<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); dqs_i++)
    {
	    
	   u4Min[dqs_i] = 0 - (ucpass_count[dqs_i] - 1) / 2;
	   u4Max[dqs_i] = (ucpass_count[dqs_i] - 1) - (ucpass_count[dqs_i] - 1) / 2;

	   mcSHOW_DBG_MSG("Byte %d : Gating(%2d ~ %2d), Size=%d.\n", 	   		   
	   	dqs_i, u4Min[dqs_i], u4Max[dqs_i], ucpass_count[dqs_i]);

	   
    }

    mcSHOW_DBG_MSG("===============================================\n");
    mcSHOW_DBG_MSG("    dqs input gating window, final delay value\n)");
    mcSHOW_DBG_MSG("================================================\n");
    mcSHOW_DBG_MSG("final Coarse 2T = %d\n", ucfinal_coarse_tune2T);
    mcSHOW_DBG_MSG("final DQS0 Coarse 0.5T = %d\n", ucfinal_coarse_tune0p5T[0]);
    mcSHOW_DBG_MSG("final DQS1 Coarse 0.5T = %d\n", ucfinal_coarse_tune0p5T[1]);
	


#if 0 
	if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        mcSHOW_DBG_MSG("final DQS1 Coarse 0.5T = %d\n", ucfinal_coarse_tune0p5T[1]);
		

    }
#endif

#if 1
    mcSHOW_DBG_MSG("final DQS2 Coarse 0.5T = %d\n", ucfinal_coarse_tune0p5T[2]);
    mcSHOW_DBG_MSG("final DQS3 Coarse 0.5T = %d\n", ucfinal_coarse_tune0p5T[3]);


#endif
    mcSHOW_DBG_MSG("_________________________________\n");
    mcSHOW_DBG_MSG("final DQS0 fine tune = %d\n", ucfinal_fine_tune[0]);
	mcSHOW_DBG_MSG("final DQS1 fine tune = %d\n", ucfinal_fine_tune[1]);
    mcSHOW_DBG_MSG("_________________________________\n");

	#if 0 
    if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        mcSHOW_DBG_MSG("final DQS1 fine tune = %d\n", ucfinal_fine_tune[1]);
		

		
    }
	#endif
#if 1
    mcSHOW_DBG_MSG("final DQS2 fine tune = %d\n", ucfinal_fine_tune[2]);
    mcSHOW_DBG_MSG("final DQS3 fine tune = %d\n", ucfinal_fine_tune[3]);

#endif

    mcSHOW_DBG_MSG2("===============================================\n");
	
    mcSHOW_DBG_MSG2("\n    dqs input gating window, best delay value\n");
    mcSHOW_DBG_MSG2("===============================================\n"); 
    mcSHOW_DBG_MSG2("best DQS0 Coarse 2T = %d\n", ucbest_coarse_tune2T[0]);
	mcSHOW_DBG_MSG2("best DQS1 Coarse 2T = %d\n", ucbest_coarse_tune2T[1]);

#if 0
	if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        mcSHOW_DBG_MSG2("best DQS1 Coarse 2T = %d\n", ucbest_coarse_tune2T[1]);
		

    }
#endif
#if 1
    mcSHOW_DBG_MSG2("best DQS2 Coarse 2T = %d\n", ucbest_coarse_tune2T[2]);
    mcSHOW_DBG_MSG2("best DQS3 Coarse 2T = %d\n", ucbest_coarse_tune2T[3]);


#endif
    mcSHOW_DBG_MSG2("_________________________________\n");
    mcSHOW_DBG_MSG2("best DQS0 Coarse 0.5T = %d\n", ucbest_coarse_tune0p5T[0]);
	mcSHOW_DBG_MSG2("best DQS1 Coarse 0.5T = %d\n", ucbest_coarse_tune0p5T[1]);

#if 0	
    if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        mcSHOW_DBG_MSG2("best DQS1 Coarse 0.5T = %d\n", ucbest_coarse_tune0p5T[1]);
		

    }
#endif	
#if 1
    mcSHOW_DBG_MSG2("best DQS2 Coarse 0.5T = %d\n", ucbest_coarse_tune0p5T[2]);
    mcSHOW_DBG_MSG2("best DQS3 Coarse 0.5T = %d\n", ucbest_coarse_tune0p5T[3]);

#endif
    mcSHOW_DBG_MSG2("_________________________________\n");
    mcSHOW_DBG_MSG2("best DQS0 fine tune = %d\n", ucbest_fine_tune[0]);
	mcSHOW_DBG_MSG2("best DQS1 fine tune = %d\n", ucbest_fine_tune[1]);
/*


	
    if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        mcSHOW_DBG_MSG2("best DQS1 fine tune = %d\n", ucbest_fine_tune[1]);	
    }
    */
#if 1
    mcSHOW_DBG_MSG2("best DQS2 fine tune = %d\n", ucbest_fine_tune[2]);
    mcSHOW_DBG_MSG2("best DQS3 fine tune = %d\n", ucbest_fine_tune[3]);

	
#endif
    mcSHOW_DBG_MSG2("=================================\n");
	
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        //set final coarse tune 2T delay value
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET));
        mcSET_FIELD(u4value, ucfinal_coarse_tune2T, 0x00000700, 8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), u4value);	

        //set final coarse tune 0.5T delay value
        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[0]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG14));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG14), u4value);
       #if 0
        if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
      #endif
		{
            u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[1]-1);
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15));
            mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15), u4value);
        }

    #if 1
        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[2]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG14));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG14), u4value);

        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[3]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15), u4value);
    #endif
        //set final fine tune delay value
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15));
        mcSET_FIELD(u4value, ucfinal_fine_tune[0], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG15), u4value);
        #if 0
        if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
        #endif 
		{
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16));
            mcSET_FIELD(u4value, ucfinal_fine_tune[1], 0x003f0000, 16);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG16), u4value);
        }

    #if 1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15));
        mcSET_FIELD(u4value, ucfinal_fine_tune[2], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG15), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16));
        mcSET_FIELD(u4value, ucfinal_fine_tune[3], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG16), u4value);
    #endif
    }
#if 0
    else
    {
        //set best coarse tune 2T delay value
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET));
        mcSET_FIELD(u4value, ucfinal_coarse_tune2T, 0x00007000, 12);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_WRAP_RESET), u4value);	

        //set final coarse tune 0.5T delay value
        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[0]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG34));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG34), u4value);

        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[1]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35), u4value);

        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[2]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG14));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG14), u4value);

        u2one_hot_dly = (U16) 1<<(ucfinal_coarse_tune0p5T[3]-1);
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15));
        mcSET_FIELD(u4value, u2one_hot_dly, 0x00000fff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15), u4value);

        //set final fine tune delay value
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35));
        mcSET_FIELD(u4value, ucfinal_fine_tune[0], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG35), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36));
        mcSET_FIELD(u4value, ucfinal_fine_tune[1], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG36), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15));
        mcSET_FIELD(u4value, ucfinal_fine_tune[2], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG15), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16));
        mcSET_FIELD(u4value, ucfinal_fine_tune[3], 0x003f0000, 16);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG16), u4value);
    }
#endif

#if 0
    //power down dqs when in idle state
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {
        //RG_A1_TX_ARDQS0_R75KP=1; 0x5801c[21]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7));
        mcSET_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), u4value);
        //RG_A1_TX_ARDQS1_R75KP=1; 0x58020[5]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8));
        mcSET_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), u4value);
        //RG_A2_TX_ARDQS2_R75KP=1; 0x5821c[21]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7));
        mcSET_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7), u4value);
        //RG_A2_TX_ARDQS3_R75KP=1; 0x58220[5]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8));
        mcSET_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8), u4value);		
    }
#if 0
    else
    {
        //RG_B1_TX_BRDQS0_R75KP=1; 0x5826c[21]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27));
        mcSET_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27), u4value);
        //RG_B1_TX_BRDQS1_R75KP=1; 0x58270[5]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28));
        mcSET_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28), u4value);
        //RG_B2_TX_BRDQS2_R75KP=1; 0x5841c[21]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7));
        mcSET_BIT(u4value, 21);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7), u4value);
        //RG_B2_TX_BRDQS3_R75KP=1; 0x58420[5]=1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8));
        mcSET_BIT(u4value, 5);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), u4value);		
    }

#endif
	
    //temp solution, dummy read/write before reset
    DramcEngine2(TE_OP_READ_CHECK, test2_0, test2_1, test2_2);

    //Reset after scan to avoid error gating counter due to DQS glitch
    //reset phy R_DMPHYRST: 0xf0[28] 
    // 0x0f0[28] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
    mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

    // read data counter reset
    // 0x0f4[25] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
    mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);                
    mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

#if 0
    if(!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
    }
#if 0
    else
    {
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17));
        mcCLR_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
        mcDELAY_us(1);
        mcSET_BIT(u4value, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
    }

#endif

    UNUSED(u4Avg);
    UNUSED(u4Min);
    UNUSED(u4Max);
    
    return DRAM_OK;

    // log example
    /*
======================================================================
x = dqs result
y = coarse_2T  coarse_0.5T  finetune
----------------------------------------------------------------------
y  |  dqs0f   dqs0r   dqs1f   dqs1r   dqs2f   dqs2r   dqs3f   dqs3r
----------------------------------------------------------------------
 6   1   0  |  ff  ff   0   0  ff  ff  ff  ff
 6   1   1  |  ff  ff   0   0  ff  ff  ff  ff
 6   1   2  |  ff  ff   0   0  ff  ff  ff  ff
 6   1   3  |  ff  ff   0   0  ff  ff  ff  ff
 6   1   4  |  ff  ff   0   0  ff  ff  7f  7f
 6   1   5  |  ff  ff   0   0  ff  ff  fe  fe
 6   1   6  |  ff  ff  c7  c7  ff  ff  ff  ff
 6   1   7  |  ff  ff  ff  ff  7f  7f  ff  ff
 6   1   8  |  ff  ff  ff  ff  ff  ff  ff  ff
 6   1   9  |  ff  ff  ff  ff  ff  ff  ff  ff
 6   1  10  |  ff  ff  ff  ff  ff  ff  ff  ff
 6   1  11  |  ff  ff  ff  ff   0   0   0   0
 6   1  12  |  ff  ff  ff  ff   0   0   0   0
 6   1  13  |  ff  ff  ff  ff   0   0   0   0
 6   1  14  |  ff  ff  ff  ff   0   0   0   0
 6   1  15  |  ff  ff  ff  ff   0   0   0   0
 6   1  16  |  ff  ff  ff  ff   0   0   0   0
 6   1  17  |  ff  ff  ff  ff   0   0   0   0
 6   1  18  |  ff  ff  df  df   0   0   0   0
 6   1  19  |  ff  ff  ff  ff   0   0   0   0
 6   1  20  |  ff  ff  ff  ff   0   0   0   0
 6   1  21  |  14  14  ff  ff   0   0   0   0
 6   1  22  |  ff  ff  ff  ff   0   0   0   0
 6   1  23  |  ff  ff  ff  ff   0   0   0   0
 6   1  24  |  ff  ff  ff  ff   0   0   0   0
 6   2   0  |   0   0   0   0   0   0   0   0
 6   2   1  |   0   0   0   0   0   0   0   0
 6   2   2  |   0   0   0   0   0   0   0   0
 6   2   3  |   0   0   0   0   0   0   0   0
 6   2   4  |   0   0   0   0   0   0   0   0
 6   2   5  |   0   0   0   0   0   0   0   0
 6   2   6  |   0   0   0   0   0   0   0   0
 6   2   7  |   0   0   0   0   0   0   0   0
 6   2   8  |   0   0   0   0   0   0   0   0
 6   2   9  |   0   0   0   0   0   0   0   0
 6   2  10  |   0   0   0   0   0   0   0   0
 6   2  11  |   0   0   0   0   0   0   0   0
 6   2  12  |   0   0   0   0   0   0   0   0
 6   2  13  |   0   0   0   0   0   0   0   0
 6   2  14  |   0   0   0   0   0   0   0   0
 6   2  15  |   0   0   0   0   0   0   0   0
 6   2  16  |   0   0   0   0   0   0   0   0
 6   2  17  |   0   0   0   0   0   0   0   0
 6   2  18  |   0   0   0   0   0   0   0   0
 6   2  19  |   0   0   0   0   0   0   0   0
 6   2  20  |   0   0   0   0   0   0   0   0
 6   2  21  |   0   0   0   0   0   0   0   0
 6   2  22  |   0   0   0   0   0   0   0   0
 6   2  23  |   0   0   0   0   0   0   0   0
 6   2  24  |   0   0   0   0   0   0   0   0
 6   3   0  |   0   0   0   0   0   0   0   0
 6   3   1  |   0   0   0   0   0   0   0   0
 6   3   2  |   0   0   0   0   0   0   0   0
 6   3   3  |   0   0   0   0   0   0   0   0
 6   3   4  |   0   0   0   0   0   0   0   0
 6   3   5  |   0   0   0   0   0   0   0   0
 6   3   6  |   0   0   0   0   0   0   0   0
 6   3   7  |   0   0   0   0   0   0   0   0
 6   3   8  |   0   0   0   0   0   0   0   0
 6   3   9  |   0   0   0   0   0   0   0   0
 6   3  10  |   0   0   0   0   0   0   0   0
 6   3  11  |   0   0   0   0  ff  ff   0   0
 6   3  12  |   0   0   0   0  ff  ff  ff  ff
 6   3  13  |   0   0   0   0  ff  ff  ff  ff
 6   3  14  |   0   0   0   0  ff  ff  ff  ff
 6   3  15  |   0   0   0   0  ff  ff  ff  ff
 6   3  16  |   0   0   0   0  ff  ff  ff  ff
 6   3  17  |   0   0   0   0  ff  ff  ff  ff
 6   3  18  |   0   0   0   0  ff  ff  ff  ff
 6   3  19  |   0   0   0   0  ff  ff  ff  ff
 6   3  20  |   0   0   0   0  ff  ff  ff  ff
 6   3  21  |   0   0   0   0  ff  ff  ff  ff
 6   3  22  |   0   0   0   0  ff  ff  ff  ff
 6   3  23  |   0   0   0   0  ff  ff  ff  ff
 6   3  24  |   0   0  ff  ff  ff  ff  ff  ff
 6   4   0  |  ff  ff  ff  ff  ff  ff  ff  ff
 6   4   1  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   2  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   3  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   4  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   5  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   6  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   7  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   8  |  ff  ff  ff  ff  ff  ff   0   0
 6   4   9  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  10  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  11  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  12  |  ff  ff  ff  ff  ff  ff  ff  ff
 6   4  13  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  14  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  15  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  16  |  ff  ff  ff  ff  ff  ff   0   0
 6   4  17  |  fb  fb  ff  ff  ff  ff   0   0
 6   4  18  |  c1  c1  ff  ff  ff  ff   0   0
 6   4  19  |  80   0  ff  ff  ff  ff   0   0
 6   4  20  |  80   0  ff  ff  ff  ff   0   0
 6   4  21  |  80   0  ff  ff  ff  ff   0   0
 6   4  22  |  80   0  ff  ff  ff  ff   0   0
 6   4  23  |  80   0  ff  ff  ff  ff   0   0
 6   4  24  |  80   0  ff  ff  ff  ff   0   0
Best Coarse Tune for 2T are the same...Done!!
======================================================================

    dqs input gating widnow, final delay value
    apply =1
======================================================================
bl_type: 1 test2_1: 1426063360 test2_2: -1442840573 counter: 20202020
final Coarse 2T = 6
final DQS0 Coarse 0.5T = 2
final DQS1 Coarse 0.5T = 2
final DQS2 Coarse 0.5T = 2
final DQS3 Coarse 0.5T = 2
__________________________________________________
final DQS0 fine tune = 24
final DQS1 fine tune = 24
final DQS2 fine tune = 10
final DQS3 fine tune = 11
======================================================================

    dqs input gating widnow, best delay value
======================================================================
best DQS0 Coarse 2T = 6
best DQS1 Coarse 2T = 6
best DQS2 Coarse 2T = 6
best DQS3 Coarse 2T = 6
__________________________________________________
best DQS0 Coarse 0.5T = 2
best DQS1 Coarse 0.5T = 2
best DQS2 Coarse 0.5T = 2
best DQS3 Coarse 0.5T = 2
__________________________________________________
best DQS0 fine tune = 24
best DQS1 fine tune = 24
best DQS2 fine tune = 10
best DQS3 fine tune = 11
======================================================================
   */
}
#endif

#ifdef DRAM_RX_WINDOW_PERBIT_CAL
//-------------------------------------------------------------------------
/** DramcRxWindowPerbitCal (v2 version)
 *  start the rx dqs perbit sw calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcRxWindowPerbitCal(void)
{
    U8 ii, jj;
    U32 u4value, u4err_value, u4fail_bit;
    RXDQS_PERBIT_DLY_T dqdqs_perbit_dly[DQ_DATA_WIDTH];
    U8 ucbit_first, ucbit_last;
    U8 uchold_pass_number;
    U8 ucsetup_pass_number;
    U8 ucmax_dqsdly_byte[DQS_NUMBER];
    U32 test2_0 = DEFAULT_TEST2_0_CAL;
    U32 test2_1 = DEFAULT_TEST2_1_CAL;
    U32 test2_2 = DEFAULT_TEST2_2_CAL;
    U8 fgfail = DRAM_CALIBRATION_PASS;
	
    // 1.delay DQ ,find the pass widnow (left boundary).
    // 2.delay DQS find the pass window (right boundary). 
    // 3.Find the best DQ / DQS to satify the middle value of the overall pass window per bit
    // 4.Set DQS delay to the max per byte, delay DQ to de-skew

    // 1
    // set DQS delay to 0 first
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DELDLY1), 0);

    // initialize parameters
    for (ii = 0; ii < TCMGET_DATA_WIDTH(); ii++)
    {
        dqdqs_perbit_dly[ii].first_dqdly_pass = -1;
        dqdqs_perbit_dly[ii].last_dqdly_pass = -2;
        dqdqs_perbit_dly[ii].first_dqsdly_pass = -1;
        dqdqs_perbit_dly[ii].last_dqsdly_pass = -2;
    }
    
    mcSHOW_DBG_MSG2("------------------------------------------------------\n"); 
    mcSHOW_DBG_MSG2("Start DQ delay to find pass range, DQS delay fixed to 0...\n");
    mcSHOW_DBG_MSG2("------------------------------------------------------\n"); 
    mcSHOW_DBG_MSG2("x-axis is bit #; y-axis is DQ delay (%d~%d)\n", 0, MAX_RX_DQDLY_TAPS-1);

    // delay DQ from 0 to 15 to get the setup time
    for (ii = 0; ii < MAX_RX_DQDLY_TAPS; ii++)
    {
        for (jj=0; jj<TCMGET_DATA_WIDTH(); jj=jj+4)
        {
            //every 4bit dq have the same delay register address
            u4value = ((U32) ii) + (((U32)ii)<<8) + (((U32)ii)<<16) + (((U32)ii)<<24);  
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQIDLY1+jj), u4value);            
        }

        //Reset after scan to avoid error gating counter due to DQS glitch
        //reset phy R_DMPHYRST: 0xf0[28] 
        // 0x0f0[28] = 1 -> 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
        mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
        //delay 10ns, 1ms here
        mcDELAY_us(1);
        mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

        // read data counter reset
        // 0x0f4[25] = 1 -> 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
        mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
        //delay 10ns, 1ms here
        mcDELAY_us(1);                
        mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);       


        // enable TE2, audio pattern
        u4err_value = 0;
        for (jj = 0; jj < 1; jj++)
        {
            u4err_value |= DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);
        }            
      

        // check fail bit ,0 ok ,others fail
        for (jj = 0; jj < TCMGET_DATA_WIDTH(); jj++)
        {
            u4fail_bit = u4err_value&((U32)1<<jj);

            if (u4fail_bit == 0)
            {
                if (dqdqs_perbit_dly[jj].first_dqdly_pass == -1)
                {
                    // first DQ pass delay tap
                    dqdqs_perbit_dly[jj].first_dqdly_pass = ii;
                }
                if (dqdqs_perbit_dly[jj].last_dqdly_pass == -2)
                {
                    if (ii == (MAX_RX_DQDLY_TAPS-1))
                    {
                        // pass to the last tap
                        dqdqs_perbit_dly[jj].last_dqdly_pass = ii;
                    }
                }
            }
            else
            {
                if ((dqdqs_perbit_dly[jj].first_dqdly_pass != -1)&&(dqdqs_perbit_dly[jj].last_dqdly_pass == -2))
                {
                    dqdqs_perbit_dly[jj].last_dqdly_pass = ii -1;
                }
            }           
            
            if (u4fail_bit == 0)
            {
                mcSHOW_DBG_MSG2("o");
            }
            else
            {
                mcSHOW_DBG_MSG2("x");
            }
            
        }        
        mcSHOW_DBG_MSG2("\n");
    }

    // 2
    //set dq delay to 0
    for (jj=0; jj<TCMGET_DATA_WIDTH(); jj=jj+4)
    {
        //every 4bit dq have the same delay register address
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQIDLY1+jj), 0x00000000);            
    }        
   
    mcSHOW_DBG_MSG2("------------------------------------------------------\n"); 
    mcSHOW_DBG_MSG2("Start DQS delay to find pass range, DQ delay fixed to 0...\n");
    mcSHOW_DBG_MSG2("------------------------------------------------------\n"); 
    mcSHOW_DBG_MSG2("x-axis is bit #; y-axis is DQS delay (%d~%d)\n", 1, MAX_RX_DQSDLY_TAPS-1);
	

    //dqs from 1       
    // because the tap DQdly=0 DQSdly=0 will be counted when we delay dq ,so we don't count it here
    // so we set first dqs delay to 1
    for (ii = 1; ii < MAX_RX_DQSDLY_TAPS; ii++)
    {
        // 0x18
    #if 1
        u4value = ((U32) ii) + (((U32)ii)<<8) + (((U32)ii)<<16) + (((U32)ii)<<24);
    #else
        if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
        {
            u4value = ((U32) ii) + (((U32)ii)<<8);
        }
        else
        {
            u4value = (U32) ii;
        }
    #endif
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DELDLY1), u4value);

        //Reset after scan to avoid error gating counter due to DQS glitch
        //reset phy R_DMPHYRST: 0xf0[28] 
        // 0x0f0[28] = 1 -> 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
        mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
        //delay 10ns, 1ms here
        mcDELAY_us(1);
        mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

        // read data counter reset
        // 0x0f4[25] = 1 -> 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
        mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
        //delay 10ns, 1ms here
        mcDELAY_us(1);                
        mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);     
        
        // enable TE2, audio pattern
        u4err_value = 0;
        for (jj = 0; jj < 1; jj++)
        {
            u4err_value |= DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);
        }            

        // check fail bit ,0 ok ,others fail
        for (jj = 0; jj < TCMGET_DATA_WIDTH(); jj++)
        {
            u4fail_bit = u4err_value&((U32)1<<jj);

            if (u4fail_bit == 0)
            {
                if (dqdqs_perbit_dly[jj].first_dqsdly_pass == -1)
                {
                    // first DQS pass delay tap
                    dqdqs_perbit_dly[jj].first_dqsdly_pass = ii;
                }
                if (dqdqs_perbit_dly[jj].last_dqsdly_pass == -2)
                {
                    if (ii == (MAX_RX_DQSDLY_TAPS-1))
                    {
                        // pass to the last tap
                        dqdqs_perbit_dly[jj].last_dqsdly_pass = ii;
                    }
                }
            }
            else
            {
                if ((dqdqs_perbit_dly[jj].first_dqsdly_pass != -1)&&(dqdqs_perbit_dly[jj].last_dqsdly_pass == -2))
                {
                    dqdqs_perbit_dly[jj].last_dqsdly_pass = ii -1;
                }
            }            
            
            if (u4fail_bit == 0)
            {
                mcSHOW_DBG_MSG2("o");				
            }
            else
            {
                mcSHOW_DBG_MSG2("x");
				
            }            
        }
        mcSHOW_DBG_MSG2("\n");        	
		
    }

    // 3
    mcSHOW_DBG_MSG("------------------------------------------------------\n"); 
    mcSHOW_DBG_MSG("Start calculate dq time and dqs time / \n");
    mcSHOW_DBG_MSG("Find max DQS delay per byte / Adjust DQ delay to align DQS...\n");
    mcSHOW_DBG_MSG("------------------------------------------------------\n"); 


    //As per byte, check max DQS delay in 8-bit. Except for the bit of max DQS delay, delay DQ to fulfill setup time = hold time
    for (ii = 0; ii < (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); ii++)
    {
        ucbit_first = 8*ii;
        ucbit_last = 8*ii+7;
        ucmax_dqsdly_byte[ii] = 0;
        for (jj = ucbit_first; jj <= ucbit_last; jj++)
        {
            // hold time = DQS pass taps
            uchold_pass_number = dqdqs_perbit_dly[jj].last_dqsdly_pass - dqdqs_perbit_dly[jj].first_dqsdly_pass + 1;
            // setup time = DQ pass taps
            ucsetup_pass_number = dqdqs_perbit_dly[jj].last_dqdly_pass - dqdqs_perbit_dly[jj].first_dqdly_pass + 1;

            if (uchold_pass_number > ucsetup_pass_number)
            {
                if (ucsetup_pass_number != 0)
                {
                    // like this:
                    // setup time(dq delay)     hold time(dqs delay)
                    // 15                  0 1                       15 tap
                    // xxxxxxxxxxxxxoooooooo|ooooooooooooooooooooxxxxx
                    dqdqs_perbit_dly[jj].best_dqdly = 0;
                    dqdqs_perbit_dly[jj].best_dqsdly = (uchold_pass_number - ucsetup_pass_number) / 2;

                    if (dqdqs_perbit_dly[jj].best_dqsdly > ucmax_dqsdly_byte[ii])
                    {
                        ucmax_dqsdly_byte[ii] = dqdqs_perbit_dly[jj].best_dqsdly;
                    }
                }
                else
                {
                    // like this:
                    // setup time(dq delay)     hold time(dqs delay)
                    // 15                  0 1                       15 tap
                    // xxxxxxxxxxxxxxxxxxxxx|xxxooooooooooxxxxxxxxxxxx
                    dqdqs_perbit_dly[jj].best_dqdly = 0;
                    dqdqs_perbit_dly[jj].best_dqsdly = (uchold_pass_number - ucsetup_pass_number) / 2 + dqdqs_perbit_dly[jj].first_dqsdly_pass;

                    if (dqdqs_perbit_dly[jj].best_dqsdly > ucmax_dqsdly_byte[ii])
                    {
                        ucmax_dqsdly_byte[ii] = dqdqs_perbit_dly[jj].best_dqsdly;
                    }
                }
            }
            else if (uchold_pass_number < ucsetup_pass_number)
            {
                if (uchold_pass_number != 0)
                {
                    // like this:
                    // setup time(dq delay)     hold time(dqs delay)
                    // 15                  0 1                       15 tap
                    // xxxoooooooooooooooooo|ooooooooxxxxxxxxxxxxxxxxx
                    dqdqs_perbit_dly[jj].best_dqsdly = 0;
                    dqdqs_perbit_dly[jj].best_dqdly = (ucsetup_pass_number - uchold_pass_number) / 2;                    
                }
                else
                {
                    // like this:
                    // setup time(dq delay)     hold time(dqs delay)
                    // 15                  0 1                       15 tap
                    // xxxoooooooooooooooxxx|xxxxxxxxxxxxxxxxxxxxxxxxx
                    dqdqs_perbit_dly[jj].best_dqsdly = 0;
                    dqdqs_perbit_dly[jj].best_dqdly = (ucsetup_pass_number - uchold_pass_number) / 2 + dqdqs_perbit_dly[jj].first_dqdly_pass;                    
                }
            }
            else   // hold time == setup time
            {
                if (uchold_pass_number != 0)
                {
                    // like this:
                    // setup time(dq delay)     hold time(dqs delay)
                    // 15                  0 1                       15 tap
                    // xxxxxxxxxxxxxxxoooooo|ooooooxxxxxxxxxxxxxxxxxxx
                    dqdqs_perbit_dly[jj].best_dqsdly = 0;
                    dqdqs_perbit_dly[jj].best_dqdly = 0;    
                }
                else
                {
                    // like this:
                    // setup time(dq delay)     hold time(dqs delay)
                    // 15                  0 1                       15 tap
                    // xxxxxxxxxxxxxxxxxxxxx|xxxxxxxxxxxxxxxxxxxxxxxxx
                    // mean this bit is error
                    mcSHOW_ERROR_CHIP_DisplayString("error on bit ");
					mcSHOW_ERROR_CHIP_DisplayInteger(jj);
					mcSHOW_ERROR_CHIP_DisplayString(" ,setup_time =hold_time =0!!\n ");
				
                    dqdqs_perbit_dly[jj].best_dqsdly = 0;
                    dqdqs_perbit_dly[jj].best_dqdly = 0;
                    fgfail = DRAM_CALIBRATION_FAIL;
                }
            } 
            mcSHOW_DBG_MSG("bit#%2d : dq time=%2d dqs time=%2d\n", jj, ucsetup_pass_number, uchold_pass_number);                                    


		}

        //mcSHOW_DBG_MSG("----seperate line----\n");

        // we delay DQ or DQS to let DQS sample the middle of tx pass window for all the 8 bits,
        for (jj = ucbit_first; jj <= ucbit_last; jj++)
        {
            // set DQS to max for 8-bit
            if (dqdqs_perbit_dly[jj].best_dqsdly < ucmax_dqsdly_byte[ii])
            {
                // delay DQ to compensate extra DQS delay
                dqdqs_perbit_dly[jj].best_dqdly = dqdqs_perbit_dly[jj].best_dqdly + (ucmax_dqsdly_byte[ii] - dqdqs_perbit_dly[jj].best_dqsdly);
                // max limit to 15
                dqdqs_perbit_dly[jj].best_dqdly = ((dqdqs_perbit_dly[jj].best_dqdly > 15) ? 15 : dqdqs_perbit_dly[jj].best_dqdly);
            }            
        }
    }
        
    {
        mcSHOW_DBG_MSG("===============================\n");
        mcSHOW_DBG_MSG("    dramc_rxdqs_perbit_swcal_v2\n");
        mcSHOW_DBG_MSG("===============================\n");
		

    #if 1
        mcSHOW_DBG_MSG("DQS Delay :\n DQS0 = %d DQS1 = %d DQS2 = %d DQS3 = %d\n", ucmax_dqsdly_byte[0], ucmax_dqsdly_byte[1], ucmax_dqsdly_byte[2], ucmax_dqsdly_byte[3]);
    #else
        if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
        {
            mcSHOW_DBG_MSG("DQS Delay :\n DQS0 = %d DQS1 = %d\n", ucmax_dqsdly_byte[0], ucmax_dqsdly_byte[1]);



		}
        else
        {
            mcSHOW_DBG_MSG("DQS Delay :\n DQS0 = %d \n", ucmax_dqsdly_byte[0]);


			
        }
    #endif
        mcSHOW_DBG_MSG("DQ Delay :\n");
        for (ii = 0; ii < TCMGET_DATA_WIDTH(); ii++)
        {
            mcSHOW_DBG_MSG("DQ%2d = %2d \n", ii, dqdqs_perbit_dly[ii].best_dqdly);
        }
        mcSHOW_DBG_MSG("______________________________________\n");

	}    

    // set dqs delay
#if 1
    u4value = ((U32) ucmax_dqsdly_byte[0]) + (((U32)ucmax_dqsdly_byte[1])<<8) + (((U32)ucmax_dqsdly_byte[2])<<16) + (((U32)ucmax_dqsdly_byte[3])<<24);
#else
    if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        u4value = ((U32) ucmax_dqsdly_byte[0]) + (((U32)ucmax_dqsdly_byte[1])<<8);
    }
    else
    {
        u4value = (U32) ucmax_dqsdly_byte[0];
    }
#endif
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DELDLY1), u4value);
			u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
			mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
			//delay 10ns, 1ms here
			mcDELAY_us(1);
			mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
	
			// read data counter reset
			// 0x0f4[25] = 1 -> 0
			u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
			mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
			//delay 10ns, 1ms here
			mcDELAY_us(1);				  
			mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
    // set dq delay
    for (jj=0; jj<TCMGET_DATA_WIDTH(); jj=jj+4)
    {
		//every 4bit dq have the same delay register address
		u4value = ((U32) dqdqs_perbit_dly[jj].best_dqdly) + (((U32)dqdqs_perbit_dly[jj+1].best_dqdly)<<8) + (((U32)dqdqs_perbit_dly[jj+2].best_dqdly)<<16) + (((U32)dqdqs_perbit_dly[jj+3].best_dqdly)<<24);  
		ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQIDLY1+jj), u4value);            
    }              

    // Log example
    /*
------------------------------------------------------
Start calculate dq time and dqs time /
Find max DQS delay per byte / Adjust DQ delay to align DQS...
------------------------------------------------------
bit# 0 : dq time=11 dqs time= 8
bit# 1 : dq time=11 dqs time= 8
bit# 2 : dq time=11 dqs time= 6
bit# 3 : dq time=10 dqs time= 8
bit# 4 : dq time=11 dqs time= 8
bit# 5 : dq time=10 dqs time= 8
bit# 6 : dq time=11 dqs time= 8
bit# 7 : dq time= 9 dqs time= 6
----seperate line----
bit# 8 : dq time=12 dqs time= 7
bit# 9 : dq time=10 dqs time= 8
bit#10 : dq time=11 dqs time= 8
bit#11 : dq time=10 dqs time= 8
bit#12 : dq time=11 dqs time= 8
bit#13 : dq time=11 dqs time= 8
bit#14 : dq time=11 dqs time= 8
bit#15 : dq time=12 dqs time= 8
----seperate line----
bit#16 : dq time=11 dqs time= 7
bit#17 : dq time=10 dqs time= 8
bit#18 : dq time=11 dqs time= 7
bit#19 : dq time=11 dqs time= 6
bit#20 : dq time=10 dqs time= 9
bit#21 : dq time=11 dqs time=10
bit#22 : dq time=11 dqs time=10
bit#23 : dq time= 9 dqs time= 9
----seperate line----
bit#24 : dq time=12 dqs time= 6
bit#25 : dq time=13 dqs time= 6
bit#26 : dq time=13 dqs time= 7
bit#27 : dq time=11 dqs time= 7
bit#28 : dq time=12 dqs time= 8
bit#29 : dq time=10 dqs time= 8
bit#30 : dq time=13 dqs time= 7
bit#31 : dq time=11 dqs time= 8
----seperate line----
==================================================
    dramc_rxdqs_perbit_swcal_v2
    channel=2(2:cha, 3:chb) apply = 1
==================================================
DQS Delay :
 DQS0 = 0 DQS1 = 0 DQS2 = 0 DQS3 = 0
DQ Delay :
DQ 0 =  1 DQ 1 =  1 DQ 2 =  2 DQ 3 =  1
DQ 4 =  1 DQ 5 =  1 DQ 6 =  1 DQ 7 =  1
DQ 8 =  2 DQ 9 =  1 DQ10 =  1 DQ11 =  1
DQ12 =  1 DQ13 =  1 DQ14 =  1 DQ15 =  2
DQ16 =  2 DQ17 =  1 DQ18 =  2 DQ19 =  2
DQ20 =  0 DQ21 =  0 DQ22 =  0 DQ23 =  0
DQ24 =  3 DQ25 =  3 DQ26 =  3 DQ27 =  2
DQ28 =  2 DQ29 =  1 DQ30 =  3 DQ31 =  1
_______________________________________________________________
   */

    if (fgfail == DRAM_CALIBRATION_FAIL)
    {
        mcSHOW_ERROR_CHIP_DisplayString("RX DQ/DQS calibration fail!\n");
        return DRAM_FAIL;
    }

    return DRAM_OK;
}
#endif


#ifdef DRAM_TX_WINDOW_PERBIT_CAL

#define fcTX_TEST_PATTERN 0   // 0: audio, 1: TE1, 2: XTALK
//#define fcTX_PI_OFFSET 0   // no offset
#define fcTX_PI_OFFSET -5   // based on waveform measurement
//#define fcTX_PI_OFFSET -3   // based on waveform measurement
#define DEFAULT_FIRST_DQDLY_L_PASS	-15
#define DEFAULT_LAST_DQDLY_L_PASS 	-16
#define DEFAULT_FIRST_DQDLY_R_PASS  -15
#define DEFAULT_LAST_DQDLY_R_PASS	-16

void DramcTxSetDqDelayCell(U8 *pdq_dly)
{
    U32 u4value, u4temp;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {        
        //bit 0~3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1));
        u4temp = ((U32)pdq_dly[3]) |((U32)pdq_dly[2]<<4) | ((U32)pdq_dly[1]<<8) |((U32)pdq_dly[0]<<12);
        mcSET_FIELD(u4value, u4temp, 0x0000ffff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1), u4value);
        //bit 4~11
        u4value = ((U32)pdq_dly[11]) | ((U32)pdq_dly[10]<<4) | ((U32)pdq_dly[9]<<8) |((U32)pdq_dly[8]<<12) | \
                        ((U32)pdq_dly[7]<<16) |((U32)pdq_dly[6]<<20) | ((U32)pdq_dly[5]<<24) |((U32)pdq_dly[4]<<28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2), u4value);
        //bit 12~15
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3));
        u4temp = ((U32)pdq_dly[15]<<16) |((U32)pdq_dly[14]<<20) | ((U32)pdq_dly[13]<<24) |((U32)pdq_dly[12]<<28);
        mcSET_FIELD(u4value, u4temp, 0xffff0000, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3), u4value);
        //bit 16~19
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1));
        u4temp = ((U32)pdq_dly[19]) |((U32)pdq_dly[18]<<4) | ((U32)pdq_dly[17]<<8) |((U32)pdq_dly[16]<<12);
        mcSET_FIELD(u4value, u4temp, 0x0000ffff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1), u4value);
        //bit 20~27
        u4value = ((U32)pdq_dly[27])|((U32)pdq_dly[26]<<4) | ((U32)pdq_dly[25]<<8) |((U32)pdq_dly[24]<<12) | \
                        ((U32)pdq_dly[23]<<16) |((U32)pdq_dly[22]<<20) | ((U32)pdq_dly[21]<<24) |((U32)pdq_dly[20]<<28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG2), u4value);
        //bit 28~31
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3));
        u4temp = ((U32)pdq_dly[31]<<16) |((U32)pdq_dly[30]<<20) | ((U32)pdq_dly[29]<<24) |((U32)pdq_dly[28]<<28);
        mcSET_FIELD(u4value, u4temp, 0xffff0000, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3), u4value);
    }
#if 0
    else
    {
        //bit 0~7
        u4value = ((U32)pdq_dly[7]) | ((U32)pdq_dly[6]<<4) | ((U32)pdq_dly[5]<<8) |((U32)pdq_dly[4]<<12) | \
                        ((U32)pdq_dly[3]<<16) |((U32)pdq_dly[2]<<20) | ((U32)pdq_dly[1]<<24) |((U32)pdq_dly[0]<<28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG22), u4value);
        //bit 8~15
        u4value = ((U32)pdq_dly[15]) | ((U32)pdq_dly[14]<<4) | ((U32)pdq_dly[13]<<8) |((U32)pdq_dly[12]<<12) | \
                        ((U32)pdq_dly[11]<<16) |((U32)pdq_dly[10]<<20) | ((U32)pdq_dly[9]<<24) |((U32)pdq_dly[8]<<28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG23), u4value);
        //bit 16~19
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1));
        u4temp = ((U32)pdq_dly[19]) |((U32)pdq_dly[18]<<4) | ((U32)pdq_dly[17]<<8) |((U32)pdq_dly[16]<<12);
        mcSET_FIELD(u4value, u4temp, 0x0000ffff, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1), u4value);
        //bit 20~27
        u4value = ((U32)pdq_dly[27]) | ((U32)pdq_dly[26]<<4) | ((U32)pdq_dly[25]<<8) |((U32)pdq_dly[24]<<12) | \
                        ((U32)pdq_dly[23]<<16) |((U32)pdq_dly[22]<<20) | ((U32)pdq_dly[21]<<24) |((U32)pdq_dly[20]<<28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG2), u4value);
        //bit 28~31
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3));
        u4temp = ((U32)pdq_dly[31]<<16) |((U32)pdq_dly[30]<<20) | ((U32)pdq_dly[29]<<24) |((U32)pdq_dly[28]<<28);
        mcSET_FIELD(u4value, u4temp, 0xffff0000, 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3), u4value);
    }    

#endif
}

void DramcTxSetDqmDelayCell(U8 *pdqm_dly)
{
    U32 u4value;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {  
        //DQM0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10));
        mcSET_FIELD(u4value, pdqm_dly[0], 0xf0000000, 28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG10), u4value);
        //DQM1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11));
        mcSET_FIELD(u4value, pdqm_dly[1], 0x0000f000, 12);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG11), u4value);
        //DQM2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG10));
        mcSET_FIELD(u4value, pdqm_dly[2], 0xf0000000, 28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG10), u4value);
        //DQM3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG11));
        mcSET_FIELD(u4value, pdqm_dly[3], 0x0000f000, 12);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG11), u4value);
    }
#if 0
    else
    {
        //DQM0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG30));
        mcSET_FIELD(u4value, pdqm_dly[0], 0xf0000000, 28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG30), u4value);
        //DQM1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG31));
        mcSET_FIELD(u4value, pdqm_dly[1], 0x0000f000, 12);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG31), u4value);
        //DQM2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG10));
        mcSET_FIELD(u4value, pdqm_dly[2], 0xf0000000, 28);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG10), u4value);
        //DQM3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG11));
        mcSET_FIELD(u4value, pdqm_dly[3], 0x0000f000, 12);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG11), u4value);
    }

#endif
}

#if fcTX_TEST_PATTERN == 2
U32 DramcXtalkEngine(void)
{
U32 u4Val; 
U32 u4Err = 0xffffffff;

    //u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3));
    //mcSET_FIELD0(u4Val, 0xf, 0x0000000f, 0);
    //mcCLR_BIT(u4Val,7);
    //ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_3), u4Val);

    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4));
    mcSET_BIT(u4Val,16);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TEST2_4), u4Val);

    // enable				
    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcSET_FIELD(u4Val, (U32) 4, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4Val);

    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
    while(mcCHK_BIT1(u4Val, POS_TESTRPT_DM_CMP_CPT) == 0)
    {
        u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        mcDELAY_us(1);
    }

    // disable write
    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcSET_FIELD(u4Val, (U32) 0, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4Val);

    // enable read
    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcSET_FIELD(u4Val, (U32) 2, MASK_CONF2_TE12_ENABLE, POS_CONF2_TEST1);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4Val);

    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
    while(mcCHK_BIT1(u4Val, POS_TESTRPT_DM_CMP_CPT) == 0)
    {
        u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_TESTRPT));
        mcDELAY_us(1);
    }

    mcDELAY_us(1);

    u4Err = 0;
    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CMP_ERR));
    u4Err |= u4Val;
#if 0
#ifdef fcFOR_16BIT_DESIGN
    if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
    {
        u4Err = u4Err & 0x0000ffff;
    }
    else
    {
        u4Err = u4Err & 0x000000ff;
    }
#endif
#endif

    //mcSHOW_DBG_MSG("XtalkErr=%8x\n", u4Err);

    // disable read
    u4Val = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2));
    mcCLR_MASK(u4Val, MASK_CONF2_TE12_ENABLE);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF2), u4Val);   

    return u4Err;
					
}
#endif

//-------------------------------------------------------------------------
/** DramcTxWindowPerbitCal (v3)
 *  TX DQS per bit SW calibration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param  apply           (U8): 0 don't apply the register we set  1 apply the register we set ,default don't apply.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcTxWindowPerbitCal(void)
{
    U8 ucpass_bit, iwrap, jj, ucindex;
    S8 ii,s1pi_dq_delay[DQS_NUMBER];
    U32 u4value, u4err_value = 0xffffffff, u4fail_bit;
    TXDQS_PERBIT_DLY_T dqdqs_perbit_dly[DQ_DATA_WIDTH];   
    S16 s2sum_dly;
    S8 s1temp1, s1temp2;
    U32 test2_0 = DEFAULT_TEST2_0_CAL;
    U32 test2_1 = DEFAULT_TEST2_1_CAL;
    U32 test2_2 = DEFAULT_TEST2_2_CAL;
    U16 u2win_size;
    U8  ucdly_temp[DQ_DATA_WIDTH];
    S8 s1min_pi_dqdly_center[DQS_NUMBER];
#ifdef DRAM_TX_PERBIT_DQM_DESKEW
    S8 dqm_dly_r[DQS_NUMBER], dqm_dly_l[DQS_NUMBER];
    U8 ucwin_size[DQS_NUMBER], dqm_dly_done[DQS_NUMBER], ucdly_dqdqm_offset[DQS_NUMBER], ucfail_cnt;
#endif    	
	
    //A.set RX DQ/DQS in the middle of the pass region from read DQ/DQS calibration
    //B.Fix DQS (RG_PI_**_PBYTE*) at degree from write leveling. 
    //   Move DQ (per byte) gradually from 90 to -45 degree to find the left boundary
    //   Move DQ (per byte) gradually from 90 to 225 degree to find the right boundary
    //C.For each DQ delay in step B, start engine test
    //D.After engine test, read per bit results from registers.
    //E.Set RG_PI_**_DQ* to lie in the average of the middle of the pass region in the same byte
    
    // set DQ's delay cell to 0
    for (jj=0; jj<TCMGET_DATA_WIDTH(); jj++)
    {
        ucdly_temp[jj] = 0;        
    }
    // set to registers
    DramcTxSetDqDelayCell(ucdly_temp); 

    // set DQM delay cell to 0
    for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
            ucdly_temp[jj] = 0;              
    }    
    // set to registers
    DramcTxSetDqmDelayCell(ucdly_temp);

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {        
        //-----------------------------------------------
        //set per byte DQS delay to 0x0: RG_*_TX_*RDQS*_DLY
        //A1 DQS0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), u4value);

        //A1 DQS1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), u4value);

        //A2 DQS2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7), u4value);

        //A2 DQS3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8), u4value);        
    }
#if 0
    else
    {        
        //-----------------------------------------------
        //set per byte DQS delay to 0x0: RG_*_TX_*RDQS*_DLY
        //B1 DQS0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27), u4value);

        //B1 DQS1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28), u4value);

        //B2 DQS2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7), u4value);

        //B2 DQS3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), u4value);      
    }

#endif

    // initialize parameters
    for (ii = 0; ii < TCMGET_DATA_WIDTH(); ii++)
    {
        dqdqs_perbit_dly[(U8)ii].first_dqdly_l_pass = DEFAULT_FIRST_DQDLY_L_PASS;
        dqdqs_perbit_dly[(U8)ii].last_dqdly_l_pass = DEFAULT_LAST_DQDLY_L_PASS;
        dqdqs_perbit_dly[(U8)ii].first_dqdly_r_pass = DEFAULT_FIRST_DQDLY_R_PASS;
        dqdqs_perbit_dly[(U8)ii].last_dqdly_r_pass = DEFAULT_LAST_DQDLY_R_PASS;
    }

    ucpass_bit = 0;

    //Move DQ (total 0x00~0x3F) down from 0x10 (default: 90 degree) to 0x38 (-45 degree) to find the left boundary
    for (ii = 16; ii >= -8; ii--)
    {
        // double check if compiler have "%" operation
        //iwrap = ii % 64;
        if (ii >= 0)
        {
            iwrap = ii;
        }
        else
        {
            iwrap = ii + 64;
        }

    #if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
    #endif
		{
            //A1 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);

            //A2 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

            //A2 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);           		
        }
	#if 0
        else
        {
            //B1 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);
            
            //B1 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);  

            //B2 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);        		
        }
    
    #endif
        
#if fcTX_TEST_PATTERN == 0   // audio
        // enable TE2, audio pattern
        u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2); 
#elif fcTX_TEST_PATTERN == 1   // TE1
        // test engine1 offset must be 0x7ff
        u4err_value = DramcEngine1(test2_0, test2_1, 0x000007ff, 0, 0);
        //mcSHOW_DBG_MSG("PI DQ delay: %4d    err_value: %8x\n", ii, u4err_value);
        u4temp = u4err_value & 0x00004000;
        if (u4temp == 0)
        {
            u4err_value = 0;
        }
        else
        {
            u4err_value = 0xffffffff;
        } 
#else   // XTALK
        u4err_value = DramcXtalkEngine();
        //mcSHOW_DBG_MSG("Using XTALK pattern to calibrate TX window. %8x\n", u4err_value);
#endif

        // check fail bit ,0 ok ,others fail
        for (jj = 0; jj < TCMGET_DATA_WIDTH(); jj++)
        {
            u4fail_bit = u4err_value&((U32)1<<jj);

            if (u4fail_bit == 0)
            {
                if (dqdqs_perbit_dly[jj].first_dqdly_l_pass == DEFAULT_FIRST_DQDLY_L_PASS)
                {
                    // first DQ pass delay tap
                    dqdqs_perbit_dly[jj].first_dqdly_l_pass = ii;
                }
                if (dqdqs_perbit_dly[jj].last_dqdly_l_pass == DEFAULT_LAST_DQDLY_L_PASS)
                {
                    if (ii == (-8))
                    {
                        // pass to the last tap
                        dqdqs_perbit_dly[jj].last_dqdly_l_pass = ii;
                    }
                }
            }
            else
            {
                if ((dqdqs_perbit_dly[jj].first_dqdly_l_pass != DEFAULT_FIRST_DQDLY_L_PASS)&&(dqdqs_perbit_dly[jj].last_dqdly_l_pass == DEFAULT_LAST_DQDLY_L_PASS))
                {
                    dqdqs_perbit_dly[jj].last_dqdly_l_pass = ii + 1;
                    //may early break for applictions
                    ucpass_bit++;
                }
            }            
        }
        //early break if every bit is ok to find boundary
        if (ucpass_bit == TCMGET_DATA_WIDTH())
        {	
            mcSHOW_DBG_MSG("TX calibration finding left boundary early break. PI DQ delay=0x%2x\n", iwrap);


			break;
        }
    }
//
    ucpass_bit = 0;
    //Move DQ (total 0x00~0x3F) up from 0x10 (default: 90 degree) to 0x28 (225 degree) to find the right boundary
    for (ii = 16; ii <= 40; ii++)
    {
    #if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
	#endif		
        {
            //A1 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
            mcSET_FIELD(u4value, ii, 0x00003f00, 8);
            mcSET_FIELD(u4value, ii, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);

            //A2 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, ii, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

            //A2 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
            mcSET_FIELD(u4value, ii, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);           		
        }
	#if 0
        else
        {
            //B1 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
            mcSET_FIELD(u4value, ii, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);
            
            //B1 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
            mcSET_FIELD(u4value, ii, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);  

            //B2 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
            mcSET_FIELD(u4value, ii, 0x00003f00, 8);
            mcSET_FIELD(u4value, ii, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);        		
        }
   
    #endif

#if fcTX_TEST_PATTERN == 0   // audio
        // enable TE2, audio pattern
        u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);                        
#elif fcTX_TEST_PATTERN == 1   // TE1
        // test engine1 offset must be 0x7ff
        u4err_value = DramcEngine1(test2_0, test2_1, 0x000007ff, 0, 0);
        //mcSHOW_DBG_MSG("PI DQ delay: %4d    err_value: %8x\n", ii, u4err_value);
        u4temp = u4err_value & 0x00004000;
        if (u4temp == 0)
        {
            u4err_value = 0;
        }
        else
        {
            u4err_value = 0xffffffff;
        } 
#else   // XTALK
        u4err_value = DramcXtalkEngine();
        //mcSHOW_DBG_MSG("Using XTALK pattern to calibrate TX window. %8x\n", u4err_value);
#endif
        // check fail bit ,0 ok ,others fail
        for (jj = 0; jj < TCMGET_DATA_WIDTH(); jj++)
        {
            u4fail_bit = u4err_value&((U32)1<<jj);

            if (u4fail_bit == 0)
            {
                if (dqdqs_perbit_dly[jj].first_dqdly_r_pass == DEFAULT_FIRST_DQDLY_R_PASS)
                {
                    // first DQ pass delay tap
                    dqdqs_perbit_dly[jj].first_dqdly_r_pass = ii;
                }
                if (dqdqs_perbit_dly[jj].last_dqdly_r_pass == DEFAULT_LAST_DQDLY_R_PASS)
                {
                    if (ii == 40)
                    {
                        // pass to the last tap
                        dqdqs_perbit_dly[jj].last_dqdly_r_pass = ii;
                    }
                }
            }
            else
            {
                if ((dqdqs_perbit_dly[jj].first_dqdly_r_pass != DEFAULT_FIRST_DQDLY_R_PASS)&&(dqdqs_perbit_dly[jj].last_dqdly_r_pass == DEFAULT_LAST_DQDLY_R_PASS))
                {
                    dqdqs_perbit_dly[jj].last_dqdly_r_pass = ii - 1;
                    //may early break for applictions
                    ucpass_bit++;
                }
            }            
        }
        //early break if every bit is ok to find boundary
        if (ucpass_bit == TCMGET_DATA_WIDTH())
        {
            mcSHOW_DBG_MSG("TX calibration finding right boundary early break. PI DQ delay=0x%2x\n", ii);

			break;
        }
    }

    //Calculate the center of DQ pass window
    //average the center delay
    for (ii=0; ii<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); ii++)
    {
        s2sum_dly = 0;
        u2win_size = 0;
        s1min_pi_dqdly_center[ii] = 127; // init to max number
        for (jj=0; jj<DQS_BIT_NUMBER; jj++)
        {
            ucindex = ii * DQS_BIT_NUMBER + jj;
            dqdqs_perbit_dly[ucindex].pi_dqdly_ok_center = (dqdqs_perbit_dly[ucindex].last_dqdly_l_pass + dqdqs_perbit_dly[ucindex].last_dqdly_r_pass) / 2;
            s2sum_dly = s2sum_dly + dqdqs_perbit_dly[ucindex].pi_dqdly_ok_center;
            u2win_size = u2win_size + (dqdqs_perbit_dly[ucindex].last_dqdly_r_pass - dqdqs_perbit_dly[ucindex].last_dqdly_l_pass + 1);
            //mcSHOW_DBG_MSG("jj=%d, win_size=%d, %d, %d\n", jj, u2win_size, dqdqs_perbit_dly[ucindex].last_dqdly_r_pass, dqdqs_perbit_dly[ucindex].last_dqdly_l_pass);

            // find min center
            if (dqdqs_perbit_dly[ucindex].pi_dqdly_ok_center < s1min_pi_dqdly_center[ii])
            {
                s1min_pi_dqdly_center[ii] = dqdqs_perbit_dly[ucindex].pi_dqdly_ok_center;
            }            
        }
        // check if SoC compiler support "/" operation?
        // 'round' operation
        s1temp1 = (S8) (s2sum_dly/DQS_BIT_NUMBER);
        s1temp2 = s1temp1+1;
        if ((s2sum_dly-s1temp1*DQS_BIT_NUMBER) > (s1temp2*DQS_BIT_NUMBER-s2sum_dly))
        {
            s1pi_dq_delay[(U8)ii] = s1temp2;
        }
        else
        {
            s1pi_dq_delay[(U8)ii] = s1temp1;
        }

        //wrap to write to registers
        // check if SoC compiler support "%" operation?
        //s1pi_dq_delay[ii] = s1pi_dq_delay[ii]%64;
        if (s1pi_dq_delay[(U8)ii] < 0)
        {
            s1pi_dq_delay[(U8)ii] = s1pi_dq_delay[(U8)ii] + 64;
        }
		
#ifdef DRAM_TX_PERBIT_DQM_DESKEW
        // calculate average window size
        ucwin_size[ii] = (U8) (u2win_size/DQS_BIT_NUMBER);
        //mcSHOW_DBG_MSG("ii=%d, win_size=%d\n", ii, ucwin_size[ii]);
#endif
    }

	
    mcSHOW_DBG_MSG("================================================\n");
    mcSHOW_DBG_MSG("    TX DQS perbit delay software calibration v3 \n"); 
    mcSHOW_DBG_MSG("================================================\n");
    mcSHOW_DBG_MSG("PI DQ (per byte) window\nx=pass dq delay value (min~max)center \ny=0-7bit DQ of every group\n");


#ifdef DRAM_TX_PERBIT_DQ_DESKEW
    mcSHOW_DBG_MSG("input delay (per bit de-skew):\n Byte0 = %d\n Byte1 = %d\n Byte2 = %d\n Byte3 = %d\n", s1min_pi_dqdly_center[0], s1min_pi_dqdly_center[1], s1min_pi_dqdly_center[2], s1min_pi_dqdly_center[3]);
#else 
    mcSHOW_DBG_MSG2("Output delay (w/o per bit de-skew):\nByte0 = %d Byte1 = %d Byte2 = %d Byte3 = %d\n", s1pi_dq_delay[0], s1pi_dq_delay[1], s1pi_dq_delay[2], s1pi_dq_delay[3]);

#endif
    mcSHOW_DBG_MSG("TX PI OFFSET : %d\n", fcTX_PI_OFFSET);
    mcSHOW_DBG_MSG("=================================================\n");
    mcSHOW_DBG_MSG("bit window center \n");	

    for (ii = 0; ii < TCMGET_DATA_WIDTH(); ii++)
    {
        mcSHOW_DBG_MSG("%2d   (%2d~%2d) %2d\n", \
            	ii, dqdqs_perbit_dly[ii].last_dqdly_l_pass, dqdqs_perbit_dly[ii].last_dqdly_r_pass, dqdqs_perbit_dly[ii].pi_dqdly_ok_center);
   
	}
	
    mcSHOW_DBG_MSG("\n===============================================\n");

#ifdef DRAM_TX_PERBIT_DQM_DESKEW
    //============================================================
    // DQM calibration
    // 1. Delay all DQ's using delay cell by average win_size/2 to make DQ's behind to DQM
    // transfer PI to delay cell
    for (ii=0; ii<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); ii++)
    {
        //mcSHOW_DBG_MSG("1. ii=%d, win_size=%d\n", ii, ucwin_size[ii]);
        ucdly_dqdqm_offset[ii] = (U8) ((TCMGET_DLYCELL_PERT*(U16)(ucwin_size[ii]/2))/64);
        if (ucdly_dqdqm_offset[ii] > 15)
        {
            ucdly_dqdqm_offset[ii] = 15;
        }
        mcSHOW_DBG_MSG("DQS:%d, win_size=%2d, dqdqm delay cell offset=%2d\n", ii, ucwin_size[ii], ucdly_dqdqm_offset[ii]);

			
        ucdly_temp[DQS_BIT_NUMBER*ii] = ucdly_dqdqm_offset[ii];
			
        for (jj=1; jj < DQS_BIT_NUMBER; jj++)
        {
            ucdly_temp[DQS_BIT_NUMBER*ii+jj] = ucdly_temp[DQS_BIT_NUMBER*ii];
        }
    }
    //set to registers
    // DQ delay cell
    DramcTxSetDqDelayCell(ucdly_temp);
	
    for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
        ucdly_temp[jj] = 0; 	   
    }
    DramcTxSetDqmDelayCell(ucdly_temp);
	
    // 2. Adjust DQ/DQM PI to find DQM right boundary
    // for early break
    ucfail_cnt = 0;
    for (jj = 0; jj < (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
        dqm_dly_done[jj] = 0;
    }
		
    for (ii = -8; ii <= 32; ii++)
    {
        // double check if compiler have "%" operation
        //iwrap = ii % 64;
        if (ii >= 0)
        {
            iwrap = ii;
        }
        else
        {
            iwrap = ii + 64;
        }

#if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
#endif			
        {
            //A1 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);
	
            //A2 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);
	
            //A2 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);					
        }
#if 0
        else
        {
            //B1 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);
				
            //B1 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);	
	
            //B2 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);				
        }

#endif
        
#if fcTX_TEST_PATTERN == 0   // audio
        // enable TE2, audio pattern
        u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2); 
#elif fcTX_TEST_PATTERN == 1   // TE1
        // test engine1 offset must be 0x7ff
        u4err_value = DramcEngine1(test2_0, test2_1, 0x000007ff, 0, 0);
        u4temp = u4err_value & 0x00004000;
        if (u4temp == 0)
        {
            u4err_value = 0;
        }
        else
        {
            u4err_value = 0xffffffff;
        }
#else   // XTALK
        u4err_value = DramcXtalkEngine();
        //mcSHOW_DBG_MSG("Using XTALK pattern to calibrate TX window. %8x\n", u4err_value);
#endif        
        mcSHOW_DBG_MSG2("PI DQ delay: %4d	 err_value: %8x\n", ii, u4err_value);

			
        for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
        {
            u4fail_bit = u4err_value&((U32)0xff<<(jj*DQS_BIT_NUMBER));
	
            if ((u4fail_bit == 0) && (dqm_dly_done[jj] == 0))
            {
                dqm_dly_r[jj] = ii;
                dqm_dly_done[jj] = 1;
                ucfail_cnt++;
            }
        }
	
        if (ucfail_cnt == (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER))
        {
            // early break
            break;
        }		 
    }
	
    if (ucfail_cnt == (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER))
    {
    #if 1
        mcSHOW_DBG_MSG("DQM delay right boundary: %3d : %3d : %3d : %3d\n", dqm_dly_r[0], dqm_dly_r[1],dqm_dly_r[2],dqm_dly_r[3]);
    #else
        if (TCMGET_DATA_WIDTH() == DATA_WIDTH_16BIT)
        {
            mcSHOW_DBG_MSG("DQM delay right boundary: %3d : %3d\n", dqm_dly_r[0], dqm_dly_r[1]);
		

        }
        else
        {
            mcSHOW_DBG_MSG("DQM delay right boundary: %3d : %3d\n", dqm_dly_r[0]);
        }
    #endif
    }
    else
    {
        mcSHOW_ERROR_CHIP_DisplayString("error: cannot find right boundary of DQM\n");
		
    }
	
    // 3. set DQ's delay cell to 0; set DQM using delay cell by average win_size/2 to make DQM behind to DQ's
    for (jj=0; jj<TCMGET_DATA_WIDTH(); jj++)
    {
        ucdly_temp[jj] = 0;
    }
    DramcTxSetDqDelayCell(ucdly_temp);
	
    for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
        ucdly_temp[jj] = ucdly_dqdqm_offset[jj];		
    }
    DramcTxSetDqmDelayCell(ucdly_temp);
	
    // 4. Adjust DQ/DQM PI to find DQM left boundary
    // for early break
    ucfail_cnt = 0;
    for (jj = 0; jj < (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
        dqm_dly_done[jj] = 0;
    }
		
    for (ii = 40; ii >= 0; ii--)
    {
        // double check if compiler have "%" operation
        //iwrap = ii % 64;
        if (ii >= 0)
        {
            iwrap = ii;
        }
        else
        {
            iwrap = ii + 64;
        }

#if 0
        if (!IS_DRAM_CHANNELB_ACTIVE())
#endif			
        {
            //A1 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);
	
            //A2 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);
	
            //A2 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);					
        }
#if 0
        else
        {
            //B1 byte A
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);
				
            //B1 byte B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);	
	
            //B2 byte A&B
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
            mcSET_FIELD(u4value, iwrap, 0x00003f00, 8);
            mcSET_FIELD(u4value, iwrap, 0x3f000000, 24);
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);				
        }

#endif
        
#if fcTX_TEST_PATTERN == 0   // audio
        // enable TE2, audio pattern
        u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2); 
#elif fcTX_TEST_PATTERN == 1   // TE1
        // test engine1 offset must be 0x7ff
        u4err_value = DramcEngine1(test2_0, test2_1, 0x000007ff, 0, 0);
        u4temp = u4err_value & 0x00004000;
        if (u4temp == 0)
        {
            u4err_value = 0;
        }
        else
        {
            u4err_value = 0xffffffff;
        }	   
#else   // XTALK
        u4err_value = DramcXtalkEngine();
        //mcSHOW_DBG_MSG("Using XTALK pattern to calibrate TX window. %8x\n", u4err_value);
#endif        
        mcSHOW_DBG_MSG2("PI DQ delay: %4d	 err_value: %8x\n", ii, u4err_value);
		
        for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
        {
            u4fail_bit = u4err_value&((U32)0xff<<(jj*DQS_BIT_NUMBER));
	
            if ((u4fail_bit == 0) && (dqm_dly_done[jj] == 0))
            {
                // add offset to compensate delay cell
                dqm_dly_l[jj] = ii + (ucwin_size[jj]/2);
                dqm_dly_done[jj] = 1;
                ucfail_cnt++;
            }
        }
	
        if (ucfail_cnt == (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER))
        {
            // early break
            break;
        }		 
    }
	
    if (ucfail_cnt == (TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER))
    {
    #if 1
        mcSHOW_DBG_MSG("DQM delay left boundary: %3d : %3d : %3d : %3d\n", dqm_dly_l[0], dqm_dly_l[1],dqm_dly_l[2],dqm_dly_l[3]);
        mcSHOW_DBG_MSG2("DQM offset: %3d : %3d : %3d : %3d\n", ucwin_size[0]/2, ucwin_size[1]/2, ucwin_size[2]/2, ucwin_size[3]/2);
    #else
        if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
        {
            mcSHOW_DBG_MSG("DQM delay left boundary: %3d : %3d\n", dqm_dly_l[0], dqm_dly_l[1]);
            mcSHOW_DBG_MSG2("DQM offset: %3d : %3d\n", ucwin_size[0]/2, ucwin_size[1]/2);
			

        }
        else
        {
            mcSHOW_DBG_MSG("DQM delay left boundary: %3d : %3d\n", dqm_dly_l[0]);
            mcSHOW_DBG_MSG2("DQM offset: %3d : %3d\n", ucwin_size[0]/2);
        }
    #endif
    }
    else
    {
        mcSHOW_ERROR_CHIP_DisplayString("error: cannot find left boundary of DQM\n");
    }
	
    // 4. DQ/DQM deskew
    // DQM de-skew
    for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
        s1temp1 = ((dqm_dly_l[jj]+dqm_dly_r[jj])/2);
#ifdef DRAM_TX_PERBIT_DQ_DESKEW
        s1temp2 = s1min_pi_dqdly_center[jj];
#else
        s1temp2 = s1pi_dq_delay[jj];
#endif
        // not sure min or average center to compare?!
        if (s1temp1 < s1temp2)
        {
        ucdly_temp[jj] = 0;
        }
        else
        {
            ucdly_temp[jj] = s1temp1 - s1temp2;
        }
        mcSHOW_DBG_MSG("DQM:%d de-skew: PI -> %d; ", jj, ucdly_temp[jj]);
		
        // change PI delay to delay cells
        ucdly_temp[jj] = (U8) ((TCMGET_DLYCELL_PERT*(U16)ucdly_temp[jj])/64) + 1; // add 1 for favor more delay of DQM
        mcSHOW_DBG_MSG("Delay cell -> %d\n", ucdly_temp[jj]);
		
    }
    // set to registers
    DramcTxSetDqmDelayCell(ucdly_temp);    		
    //============================================================
#else
    for (jj=0; jj<(TCMGET_DATA_WIDTH()/DQS_BIT_NUMBER); jj++)
    {
        ucdly_temp[jj] = 0;  
        mcSHOW_DBG_MSG("DQM:%d w/o de-skew: delay cell -> %d\n", jj, ucdly_temp[jj]);
		
   }	 
    // set to registers
    DramcTxSetDqmDelayCell(ucdly_temp);  
#endif
	
    //============================================================
#ifdef DRAM_TX_PERBIT_DQ_DESKEW
    // DQ de-skew
    for (jj=0; jj<TCMGET_DATA_WIDTH(); jj++)
    {
        ucindex = jj /DQS_BIT_NUMBER;
        ucdly_temp[jj] = dqdqs_perbit_dly[jj].pi_dqdly_ok_center - s1min_pi_dqdly_center[ucindex];
        mcSHOW_DBG_MSG("DQ:%2d de-skew: PI -> %d; ", jj, ucdly_temp[jj]);
        // change PI delay to delay cells
        ucdly_temp[jj] = (U8) ((TCMGET_DLYCELL_PERT*(U16)ucdly_temp[jj])/64);
        mcSHOW_DBG_MSG("Delay cell -> %d\n", ucdly_temp[jj]);
    }
    // set to registers
    DramcTxSetDqDelayCell(ucdly_temp); 
#else
    for (jj=0; jj<TCMGET_DATA_WIDTH(); jj++)
    {
        ucdly_temp[jj] = 0;
        mcSHOW_DBG_MSG("DQ:%2d w/o de-skew: delay cell -> %d\n", jj, ucdly_temp[jj]);
		
    }
    // set to registers
    DramcTxSetDqDelayCell(ucdly_temp); 
#endif
    //============================================================	
    //set to registers
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {
        //PI DQ (per byte)
        //A1 byte A&B
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
#ifndef DRAM_TX_PERBIT_DQ_DESKEW // if TX per bit de-skew, PI set to min
        mcSET_FIELD(u4value, s1pi_dq_delay[1]+fcTX_PI_OFFSET, 0x00003f00, 8);
        mcSET_FIELD(u4value, s1pi_dq_delay[0]+fcTX_PI_OFFSET, 0x3f000000, 24);
#else
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[1]+fcTX_PI_OFFSET, 0x00003f00, 8);
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[0]+fcTX_PI_OFFSET, 0x3f000000, 24);
#endif
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);
    
        //A2 byte A
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
#ifndef DRAM_TX_PERBIT_DQ_DESKEW // if TX per bit de-skew, PI set to min
        mcSET_FIELD(u4value, s1pi_dq_delay[2]+fcTX_PI_OFFSET, 0x00003f00, 8);
#else
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[2]+fcTX_PI_OFFSET, 0x00003f00, 8);
#endif
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);
    
        //A2 byte B
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
#ifndef DRAM_TX_PERBIT_DQ_DESKEW // if TX per bit de-skew, PI set to min
        mcSET_FIELD(u4value, s1pi_dq_delay[3]+fcTX_PI_OFFSET, 0x3f000000, 24);
#else
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[3]+fcTX_PI_OFFSET, 0x3f000000, 24);
#endif
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);    
    }
#if 0
    else
    {
        //PI DQ (per byte)
        //B1 byte A
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
#ifndef DRAM_TX_PERBIT_DQ_DESKEW // if TX per bit de-skew, PI set to min
        mcSET_FIELD(u4value, s1pi_dq_delay[0]+fcTX_PI_OFFSET, 0x00003f00, 8);
#else
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[0]+fcTX_PI_OFFSET, 0x00003f00, 8);
#endif    
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);
    
        //B1 byte B
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
#ifndef DRAM_TX_PERBIT_DQ_DESKEW // if TX per bit de-skew, PI set to min
        mcSET_FIELD(u4value, s1pi_dq_delay[1]+fcTX_PI_OFFSET, 0x3f000000, 24);
#else
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[1]+fcTX_PI_OFFSET, 0x3f000000, 24);
#endif
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);    
    
        //B2 byte A&B
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
#ifndef DRAM_TX_PERBIT_DQ_DESKEW // if TX per bit de-skew, PI set to min
        mcSET_FIELD(u4value, s1pi_dq_delay[3]+fcTX_PI_OFFSET, 0x00003f00, 8);
        mcSET_FIELD(u4value, s1pi_dq_delay[2]+fcTX_PI_OFFSET, 0x3f000000, 24);
#else
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[3]+fcTX_PI_OFFSET, 0x00003f00, 8);
        mcSET_FIELD(u4value, s1min_pi_dqdly_center[2]+fcTX_PI_OFFSET, 0x3f000000, 24);
#endif
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);     
    }    

#endif    
	
    return DRAM_OK;

    // log example
    /*
TX calibration finding left boundary early break. PI DQ delay=0x3e
TX calibration finding right boundary early break. PI DQ delay=0x1c
==================================================================
    TX DQS perbit delay software calibration v3
    channel=2(2:cha, 3:chb)  apply = 1
==================================================================
PI DQ (per byte) window
x=pass dq delay value (min~max)center
y=0-7bit DQ of every group
input delay:Byte0 = 13 Byte1 = 13 Byte2 = 12 Byte3 = 13
==================================================================
bit    Byte0    bit    Byte1    bit    Byte2    bit    Byte3
 0   ( 1~26)13,  8   ( 3~26)14, 16   ( 2~27)14, 24   ( 3~26)14
 1   ( 2~26)14,  9   ( 1~24)12, 17   ( 0~25)12, 25   ( 4~26)15
 2   ( 3~25)14, 10   ( 2~26)14, 18   ( 2~25)13, 26   ( 3~27)15
 3   ( 2~24)13, 11   ( 1~23)12, 19   ( 3~25)14, 27   ( 1~23)12
 4   ( 3~26)14, 12   ( 2~26)14, 20   ( 0~24)12, 28   ( 1~25)13
 5   ( 3~25)14, 13   ( 2~25)13, 21   (-1~25)12, 29   ( 2~24)13
 6   ( 2~26)14, 14   ( 2~24)13, 22   (-1~26)12, 30   ( 3~27)15
 7   ( 1~25)13, 15   ( 2~26)14, 23   (-1~22)10, 31   ( 2~26)14

==================================================================
   */
}
#endif

#ifdef DRAM_RX_EYE_SCAN
//-------------------------------------------------------------------------
/** DramcRxEyeScan
 *  start the rx dq eye scan.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param dq_no            (U8): 0~31.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcRxEyeScan(U8 dq_no)
{
    U8 ucidx_dqs, ucidx_dq, ucdqs_dly;
    S8 s1vref, s1dq_dly;
    U32 u4value, u4err_value = 0xffffffff, u4addr_array[4], u4sample_cnt, u4error_cnt;
    U32 ii;

#if 1
    if (dq_no <= 15)
    {
        if (!IS_DRAM_CHANNELB_ACTIVE())
        {
            u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18);
            u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19);
        }
        else
        {
            u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38);
            u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39);
        }

        u4addr_array[2] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_TOGGLE_CNT);
        u4addr_array[3] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_ERR_CNT);
        ucidx_dqs =  dq_no / 8;
    }
    else
    {
        if (!IS_DRAM_CHANNELB_ACTIVE())
        {
            u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG18);
            u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19);
        }
        else
        {
            u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG18);
            u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19);
        }

        u4addr_array[2] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_TOGGLE_CNT_2);
        u4addr_array[3] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_ERR_CNT_2);
        ucidx_dqs =  (dq_no-16) / 8;
    }
#else
    u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18);
    u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19);
    u4addr_array[2] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_TOGGLE_CNT);
    u4addr_array[3] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQ_ERR_CNT);
    ucidx_dqs =  dq_no / 8;
#endif
    // check if SoC platform has "%" operation?!
    ucidx_dq =  dq_no % 16;
    	
    //Enable DQ eye scan
    //RG_??_RX_EYE_SCAN_EN
    //RG_??_RX_VREF_EN 
    //RG_??_RX_VREF_OP_EN 
    //RG_??_RX_SMT_EN
    u4value = ucDram_Register_Read(u4addr_array[0]);
    u4value = u4value | 0x0000e000;
    ucDram_Register_Write(u4addr_array[0], u4value);

    u4value = ucDram_Register_Read(u4addr_array[1]);
    u4value = u4value | 0xffff0000;
    ucDram_Register_Write(u4addr_array[1], u4value);

    //Disable MIOCK jitter meter mode (RG_??_RX_DQS_MIOCK_SEL=0, RG_RX_MIOCK_JIT_EN=0)
    //RG_??_RX_DQS_MIOCK_SEL=0
    u4value = ucDram_Register_Read(u4addr_array[0]);
    u4value = u4value & 0xffffefff;
    ucDram_Register_Write(u4addr_array[0], u4value);

    //RG_RX_MIOCK_JIT_EN=0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
    mcCLR_BIT(u4value, POS_DCBLN_RX_MIOCK_JIT_EN);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

    //select DQ to be scanned (0~15)
    //DDRPHY: RG_??_RX_DQ_EYE_SEL
    u4value = ucDram_Register_Read(u4addr_array[0]);
    mcSET_FIELD(u4value, ucidx_dq, 0x0f000000, 24)
    ucDram_Register_Write(u4addr_array[0], u4value); 
    //DRAMC: RG_RX_DQ_EYE_SEL (0~15)
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
    mcSET_FIELD(u4value, ucidx_dq, 0x000000f0, 4)
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value); 
 	
    //Select DQS to be scanned (0 or 1)
    //RG_??_RX_DQS_EYE_SEL
    u4value = ucDram_Register_Read(u4addr_array[0]);
    mcSET_FIELD(u4value, ucidx_dqs, 0x00800000, 23);
    ucDram_Register_Write(u4addr_array[0], u4value); 	

    mcSHOW_DBG_MSG("===============================\n");
    mcSHOW_DBG_MSG("    DQ RX eye scan (dq_%d)\n", dq_no);
    mcSHOW_DBG_MSG("===============================\n");


    for (s1vref=15; s1vref>=0; s1vref--)
    {
        //Set Vref voltage
        u4value = ucDram_Register_Read(u4addr_array[0]);
        mcSET_FIELD(u4value, s1vref, 0x0000000f, 0)
        ucDram_Register_Write(u4addr_array[0], u4value); 

        //Wait for Vref settles down, 1ms is enough
        mcDELAY_us(10);

        //Set DQS delay (RG_??_RX_DQS_EYE_DLY) to 0
        u4value = ucDram_Register_Read(u4addr_array[0]);
        mcSET_FIELD(u4value, 0, 0x007f0000, 16)
        ucDram_Register_Write(u4addr_array[0], u4value);  		
 		
        //Disable DQ eye scan (b'1), for counter clear
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        for (s1dq_dly=15; s1dq_dly>=0; s1dq_dly--)
        {
            //Set DQ delay (RG_??_RX_DQ_EYE_DLY)
            u4value = ucDram_Register_Read(u4addr_array[0]);
            mcSET_FIELD(u4value, s1dq_dly, 0x00000f00, 8);
            ucDram_Register_Write(u4addr_array[0], u4value);

            //Reset eye scan counters (reg_sw_rst): 1 to 0
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
            mcSET_BIT(u4value, POS_DCBLN_REG_SW_RST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
            mcCLR_BIT(u4value, POS_DCBLN_REG_SW_RST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

            //Enable DQ eye scan (b'1)
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
            mcSET_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

            // enable TE2, audio pattern
            u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, 0xaa550000, DEFAULT_TEST2_1_CAL, 0x00000100);                        

            //Disable DQ eye scan (b'1), for counter latch
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
            mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

            //Read the counter values from registers (toggle_cnt*, dqs_err_cnt*); *_cnt is for (DQS0,1) and *_cnt_2 is for (DQS2,3)
            u4sample_cnt = ucDram_Register_Read(u4addr_array[2]);
            u4error_cnt = ucDram_Register_Read(u4addr_array[3]);

            mcSHOW_DBG_MSG("%4d,", u4error_cnt);   
			
        }

        //Set DQ delay (RG_??_RX_DQ_EYE_DLY) to 0
        u4value = ucDram_Register_Read(u4addr_array[0]);
        mcSET_FIELD(u4value, 0, 0x00000f00, 8);
        ucDram_Register_Write(u4addr_array[0], u4value);

        //Disable DQ eye scan (b'1), for counter clear
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        //for eye scan, dqs only need to scan 0~31
        for (ucdqs_dly=0; ucdqs_dly<32; ucdqs_dly++)
        {
            //Set DQS delay (RG_??_RX_DQS_EYE_DLY)
            u4value = ucDram_Register_Read(u4addr_array[0]);
            mcSET_FIELD(u4value, ucdqs_dly, 0x007f0000, 16);
            ucDram_Register_Write(u4addr_array[0], u4value);

            //Reset eye scan counters (reg_sw_rst): 1 to 0
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
            mcSET_BIT(u4value, POS_DCBLN_REG_SW_RST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
            mcCLR_BIT(u4value, POS_DCBLN_REG_SW_RST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

            //Enable DQ eye scan (b'1)
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
            mcSET_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

            //reset phy R_DMPHYRST: 0xf0[28] 
            // 0x0f0[28] = 1 -> 0
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
            mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
            //delay 10ns, 1ms here
            mcDELAY_us(1);
            mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

            // read data counter reset
            // 0x0f4[25] = 1 -> 0
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
            mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
            //delay 10ns, 1ms here
            mcDELAY_us(1);                
            mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

            // enable TE2, audio pattern
            u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, 0xaa550000, DEFAULT_TEST2_1_CAL, 0x00000100);

            //Disable DQ eye scan (b'1), for counter latch
            u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
            mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
            ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

            //Read the counter values from registers (toggle_cnt*, dqs_err_cnt*); *_cnt is for (DQS0,1) and *_cnt_2 is for (DQS2,3)
            u4sample_cnt = ucDram_Register_Read(u4addr_array[2]);
            u4error_cnt = ucDram_Register_Read(u4addr_array[3]);

            mcSHOW_DBG_MSG("%4d,", u4error_cnt);   
			

        }
            mcSHOW_DBG_MSG("\n");		

		
    }

    //reset phy R_DMPHYRST: 0xf0[28] 
    // 0x0f0[28] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1));
    mcSET_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);
    mcCLR_BIT(u4value, POS_PHYCTL1_PHYRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_PHYCTL1), u4value);

    // read data counter reset
    // 0x0f4[25] = 1 -> 0
    u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1));
    mcSET_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);
    //delay 10ns, 1ms here
    mcDELAY_us(1);                
    mcCLR_BIT(u4value, POS_GDDR3CTL1_RDATRST);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_GDDR3CTL1), u4value);

    //Set Vref voltage
    u4value = ucDram_Register_Read(u4addr_array[0]);
    mcSET_FIELD(u4value, 8, 0x0000000f, 0)
    ucDram_Register_Write(u4addr_array[0], u4value);
    //Set DQ delay (RG_??_RX_DQ_EYE_DLY)
    u4value = ucDram_Register_Read(u4addr_array[0]);
    mcSET_FIELD(u4value, 0, 0x00000f00, 8);
    ucDram_Register_Write(u4addr_array[0], u4value);
    //set dqs delay
    u4value = ucDram_Register_Read(u4addr_array[0]);
    mcSET_FIELD(u4value, 9, 0x007f0000, 16);
    ucDram_Register_Write(u4addr_array[0], u4value);

    mcSHOW_DBG_MSG("vref, dqs_delay, dq_delay: reg 0x%08x=0x%08x \n", u4addr_array[0], ucDram_Register_Read(u4addr_array[0]));


    for(ii=0;ii<10;ii++)
    {
        //Reset eye scan counters (reg_sw_rst): 1 to 0
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcSET_BIT(u4value, POS_DCBLN_REG_SW_RST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
        mcCLR_BIT(u4value, POS_DCBLN_REG_SW_RST);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        //Enable DQ eye scan (b'1)
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcSET_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        // enable TE2, audio pattern
        //u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, 0xaa550000, 0x30000000, 0x00000100);                        
        mcDELAY_us(100);  //HAL_Delay_us(100); //mtk40739 modify
        //Disable DQ eye scan (b'1), for counter latch
        u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
        mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

        //Read the counter values from registers (toggle_cnt*, dqs_err_cnt*); *_cnt is for (DQS0,1) and *_cnt_2 is for (DQS2,3)
        u4sample_cnt = ucDram_Register_Read(u4addr_array[2]);
        u4error_cnt = ucDram_Register_Read(u4addr_array[3]);

        mcSHOW_DBG_MSG("%4d,", u4error_cnt);  
		

    }

    mcSHOW_DBG_MSG("\n");


    UNUSED(u4err_value);
    UNUSED(u4sample_cnt);
    UNUSED(u4error_cnt);

    //RG_RX_MIOCK_JIT_EN=0
    //u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), &u4value);
    //mcCLR_BIT(u4value, POS_DCBLN_RX_MIOCK_JIT_EN);
    //ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);

	return DRAM_OK;
	
    // log example
    /*
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 504, 502, 500, 495, 494, 227,  61,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   3, 133, 226, 354, 378, 378, 381, 424, 480, 504, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 504, 502, 500, 500, 429, 429,  53,  18,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  73, 197, 307, 378, 378, 385, 422, 493, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 502, 500, 500, 500, 225, 228,  13,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   9, 182, 200, 378, 378, 411, 450, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 500, 500, 500, 463,  59,  56,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 102, 193, 348, 382, 436, 476, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 500, 500, 484, 343,  16,  15,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  17, 129, 268, 384, 444, 503, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 500, 500, 401, 177,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,  84, 238, 372, 467, 506, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 504, 500, 223,  38,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  15, 261, 365, 499, 506, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 496,  81,   7,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  11, 264, 372, 506, 506, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 478, 121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,  45, 241, 322, 475, 506, 506, 506, 506, 506, 506, 506, 506,
 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 506, 492, 220,  37,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  10,  80, 195, 308, 470, 506, 506, 506, 506, 506, 506, 506, 506,
 512, 512, 512, 512, 512, 512, 512, 512, 506, 506, 506, 506, 466, 324, 103,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,  41, 156, 205, 315, 494, 506, 506, 506, 506, 506, 506, 506, 506,
 512, 512, 512, 512, 512, 512, 512, 512, 512, 506, 506, 496, 409, 364, 228,  17,  13,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,  74, 187, 245, 349, 502, 506, 506, 506, 506, 506, 506, 506, 506,
 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 506, 469, 386, 373, 320,  89,  85,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   6, 135, 189, 316, 373, 506, 506, 506, 506, 506, 506, 506, 506, 506,
 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 412, 378, 378, 345, 178, 181,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   9, 188, 193, 365, 397, 506, 506, 506, 506, 506, 506, 506, 506, 506,
 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 389, 378, 378, 368, 299, 297,  37,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  38, 191, 225, 376, 446, 506, 506, 506, 506, 506, 506, 506, 506, 506,
 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 508, 435, 378, 378, 374, 331, 332, 133,  24,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,  96, 192, 264, 404, 498, 506, 506, 506, 506, 506, 506, 506, 506, 506,
   */
}
#endif

#ifdef DRAM_TX_EYE_SCAN
//-------------------------------------------------------------------------
/** DramcTxEyeScan
 *  Start the tx dq eye scan. (all 32-bit one time)
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcTxEyeScan(void)
{
    U8 ucdq_dly_wrap, ucdq_no, fgerr_dq;
    S8 s1dq_dly, s1vref;
    U32 u4value, u4err_value = 0xffffffff;
    U32 u4prev_phyreg_0d0, u4prev_phyreg_004, u4prev_phyreg_008, u4prev_phyreg_00c, u4prev_phyreg_048;
#if 1
    U32 u4prev_phyreg_204, u4prev_phyreg_208, u4prev_phyreg_20c, u4prev_phyreg_248, u4prev_phyreg_2c8;
    U32 u4prev_phyreg_2d4, u4prev_phyreg_2d8, u4prev_phyreg_4d0, u4prev_phyreg_258, u4prev_phyreg_2cc;
    U32 u4prev_phyreg_25c, u4prev_phyreg_404, u4prev_phyreg_408, u4prev_phyreg_40c, u4prev_phyreg_298, u4prev_phyreg_448;
    U32 u4addr_array[2];
#else
    U32 u4addr_array[1];
#endif
    U32 test2_0 = DEFAULT_TEST2_0_CAL;
    U32 test2_1 = DEFAULT_TEST2_1_CAL;
    U32 test2_2 = DEFAULT_TEST2_2_CAL;

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {
        u4prev_phyreg_0d0 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
        u4prev_phyreg_2c8 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
        u4prev_phyreg_2cc = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
        u4prev_phyreg_004 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1));
        u4prev_phyreg_008 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2));
        u4prev_phyreg_00c = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3));
        u4prev_phyreg_204 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1));
        u4prev_phyreg_208 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG2));
        u4prev_phyreg_20c = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3));     
        u4prev_phyreg_048 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18));
        u4prev_phyreg_248 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG18));
    }
#if 0
    else
    {
        u4prev_phyreg_2d4 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
        u4prev_phyreg_2d8 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
        u4prev_phyreg_4d0 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
        u4prev_phyreg_258 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG22));
        u4prev_phyreg_25c = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG23));
        u4prev_phyreg_404 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1));
        u4prev_phyreg_408 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG2));
        u4prev_phyreg_40c = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3));     
        u4prev_phyreg_298 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38)); 
        u4prev_phyreg_448 = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG18)); 
    }
   
#endif
    

    //A.set RX DQ/DQS in the middle of the pass region from read DQ/DQS calibration
    //B.Fix DQS (RG_PI_**_PBYTE*) at degree from write leveling. 
    //   Move DQ (per byte) gradually from 90 to -45 degree to find the left boundary
    //   Move DQ (per byte) gradually from 90 to 225 degree to find the right boundary
    //C.For each DQ delay in step B, start engine test
    //D.After engine test, read per bit results from registers.
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif
    {
        //set per bit DQ delay to 0x0: RG_*_TX_*RDQ*_DLY
        //A1 bit 0~15
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1));
        u4value = u4value & 0xffff0000;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1), u4value);

        if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
        {
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2), 0);
        }
        else
        {
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2));
            u4value = u4value & 0x0000ffff;
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2), u4value);
        }

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3));
        u4value = u4value & 0x0000ffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3), u4value);

    #if 1
        //A2 bit 16~31
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1));
        u4value = u4value & 0xffff0000;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1), u4value);

        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG2), 0);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3));
        u4value = u4value & 0x0000ffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3), u4value);
    #endif
        //-----------------------------------------------
        //set per byte DQS delay to 0x0: RG_*_TX_*RDQS*_DLY
        //A1 DQS0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG7), u4value);

        //if (TCMGET_DATA_WIDTH()==DATA_WIDTH_16BIT)
        {
            //A1 DQS1
            u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8));
            u4value = u4value & 0xffff0fff;
            ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG8), u4value);
        }

    #if 1
        //A2 DQS2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG7), u4value);

        //A2 DQS3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG8), u4value);        
    #endif
    
        // assign address array
        u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18);
    #if 1
        u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG18);
    #endif

        //MUX settings for VREF_GEN_SCAN out
        //RG_*_RX_VREF_OUT_SCAN_SEL=1
        //RG_*_RX_VREF_OUT_SEL=1
        //0x1044[2], [9] for A1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17));
        u4value = u4value | 0x00000204;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);
    #if 1
        //0x1244[2], [9] for A2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17));
        u4value = u4value | 0x00000204;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
    #endif        
    }
#if 0
    else
    {
        //set per bit DQ delay to 0x0: RG_*_TX_*RDQ*_DLY
        //B1 bit 0~15
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG22), 0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG23), 0);

        //B2 bit 16~31
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1));
        u4value = u4value & 0xffff0000;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1), u4value);

        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG2), 0);

        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3));
        u4value = u4value & 0x0000ffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3), u4value);
        //-----------------------------------------------
        //set per byte DQS delay to 0x0: RG_*_TX_*RDQS*_DLY
        //B1 DQS0
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG27), u4value);

        //B1 DQS1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG28), u4value);

        //B2 DQS2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7));
        u4value = u4value & 0x0fffffff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG7), u4value);

        //B2 DQS3
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8));
        u4value = u4value & 0xffff0fff;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG8), u4value);      

        // assign address array
        u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38);
        u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG18);

        //MUX settings for VREF_GEN_SCAN out
        //RG_*_RX_VREF_OUT_SCAN_SEL=1
        //RG_*_RX_VREF_OUT_SEL=1
        //0x1294[2], [9] for B1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37));
        u4value = u4value | 0x00000204;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);
        //0x1444[2], [9] for B2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17));
        u4value = u4value | 0x00000204;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
    }
#endif

    for (s1vref=0; s1vref<16; s1vref++)
    {
        //Set Vref voltage
        u4value = ucDram_Register_Read(u4addr_array[0]);
        mcSET_FIELD(u4value, s1vref, 0x0000000f, 0)
        ucDram_Register_Write(u4addr_array[0], u4value); 

    #if 1
        u4value = ucDram_Register_Read(u4addr_array[1]);
        mcSET_FIELD(u4value, s1vref, 0x0000000f, 0)
        ucDram_Register_Write(u4addr_array[1], u4value); 
    #endif

        //Wait for Vref settles down, 1ms is enough
        mcDELAY_us(10);

        //Move DQ (per byte) gradually from -45 to 225 degree (RG_PI_**_DQ*=-8~40 => 0x38 to 0x28)
        for (s1dq_dly=-8; s1dq_dly<=40; s1dq_dly++)
        {
            // double check if compiler have "%" operation
            //iwrap = ii % 64;
            if (s1dq_dly >= 0)
            {
                ucdq_dly_wrap = (U8) s1dq_dly;
            }
            else
            {
                ucdq_dly_wrap = (U8) (s1dq_dly + 64);
            }

         #if 0
            if (!IS_DRAM_CHANNELB_ACTIVE())
		     #endif		
            {
                //A1 byte A&B
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52));
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x00003f00, 8);
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x3f000000, 24);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4value);

                //A2 byte A
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50));
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x00003f00, 8);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4value);

                //A2 byte B
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51));
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x3f000000, 24);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4value);           		
            }
		#if 0
            else
            {
                //B1 byte A
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53));
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x00003f00, 8);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4value);
            
                //B1 byte B
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54));
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x3f000000, 24);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4value);  

                //B2 byte A&B
                u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52));
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x00003f00, 8);
                mcSET_FIELD(u4value, ucdq_dly_wrap, 0x3f000000, 24);
                ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4value);        		
            }
        
        #endif

            // enable TE2, audio pattern
            u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);                        

            // array index cannot be negative
            // for log
            u4tx_eye_scan_buf[s1vref][(s1dq_dly+8)] = u4err_value;
        }        
    }

    //MUX settings for Vref out recovery
    //RG_*_RX_VREF_OUT_SCAN_SEL=0
    //RG_*_RX_VREF_OUT_SEL=0
#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {
        //0x1044[2], [9] for A1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17));
        u4value = u4value & 0xfffffdfb;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG17), u4value);
        //0x1244[2], [9] for A2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17));
        u4value = u4value & 0xfffffdfb;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG17), u4value);
    }
#if 0
    else
    {
        //0x1294[2], [9] for B1
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37));
        u4value = u4value & 0xfffffdfb;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG37), u4value);
        //0x1444[2], [9] for B2
        u4value = ucDram_Register_Read(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17));
        u4value = u4value & 0xfffffdfb;
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG17), u4value);
    }

#endif
    
    // write log
    for (ucdq_no=0; ucdq_no<TCMGET_DATA_WIDTH(); ucdq_no++)
    {
        mcSHOW_DBG_MSG("--DQ%d TX eye scan\n", ucdq_no);

	


        for (s1vref=15; s1vref>=0; s1vref--)
        {
            for (s1dq_dly=-8; s1dq_dly<=40; s1dq_dly++)
            {
                u4err_value = u4tx_eye_scan_buf[s1vref][(s1dq_dly+8)];
                u4value = u4err_value&(1<<ucdq_no);
                if (u4value == 0)
                {
                    fgerr_dq = 0;
                }
                else
                {
                    fgerr_dq = 1;
                }
                mcSHOW_DBG_MSG("%1d,", fgerr_dq);
				
	
            }
            mcSHOW_DBG_MSG("\n");
			
        }
        mcSHOW_DBG_MSG("\n");
        mcSHOW_DBG_MSG("\n");
		
    } 	

#if 0
    if (!IS_DRAM_CHANNELB_ACTIVE())
#endif		
    {
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG52), u4prev_phyreg_0d0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG50), u4prev_phyreg_2c8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG51), u4prev_phyreg_2cc);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG1), u4prev_phyreg_004);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG2), u4prev_phyreg_008);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG3), u4prev_phyreg_00c);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG1), u4prev_phyreg_204);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG2), u4prev_phyreg_208);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG3), u4prev_phyreg_20c);     
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18), u4prev_phyreg_048);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG18), u4prev_phyreg_248);
    }
#if 0
    else
    {
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG53), u4prev_phyreg_2d4);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG54), u4prev_phyreg_2d8);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG52), u4prev_phyreg_4d0);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG22), u4prev_phyreg_258);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG23), u4prev_phyreg_25c);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG1), u4prev_phyreg_404);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG2), u4prev_phyreg_408);
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG3), u4prev_phyreg_40c);     
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38), u4prev_phyreg_298); 
        ucDram_Register_Write(mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG18), u4prev_phyreg_448); 
    } 

        
#endif

    UNUSED(fgerr_dq);
    
    return DRAM_OK;

    // log example
    /*
--DQ0 TX eye scan
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,


--DQ1 TX eye scan
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
...
...
...
--DQ30 TX eye scan
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,


--DQ31 TX eye scan
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   */
}
#endif

#ifdef DRAM_DQS_JMETER
//-------------------------------------------------------------------------
/** DramcDqsJmeter
 *  start DQS jitter meter. Eye scan block is built by 2-byte unit
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param dqs_no           (U8): 0~3.
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL 
 */
//-------------------------------------------------------------------------
DRAM_STATUS_T DramcDqsJmeter(U8 dqs_no)
{
		U8 ucidx_dqs, ucdqs_dly;
		U32 u4value, u4err_value = 0xffffffff, u4addr_array[4], u4sample_cnt, u4ones_cnt;
		U32 test2_0 = DEFAULT_TEST2_0_CAL;
		U32 test2_1 = DEFAULT_TEST2_1_CAL;
		U32 test2_2 = DEFAULT_TEST2_2_CAL;
	
		if ((dqs_no < 0) || (dqs_no > 3))
		{
			mcSHOW_ERROR_CHIP_DisplayString("DQS number should be 0~3\n");
			return DRAM_FAIL;
		}
	
		if ((dqs_no == 0)||(dqs_no == 1))
		{
    #if 0 //fcFOR_ONE_CHANNEL_DESIGN
			if (!IS_DRAM_CHANNELB_ACTIVE())
    #endif
			{
				u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG18);
				u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_A1_CFG19);
			}
    #if 0 //fcFOR_ONE_CHANNEL_DESIGN
			else
			{
				u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG38);
				u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG39);
			}
    #endif
	
			u4addr_array[2] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_TOGGLE_CNT);
			u4addr_array[3] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQS_ERR_CNT);
		}
		else
		{
    #if 0 //fcFOR_ONE_CHANNEL_DESIGN
			if (!IS_DRAM_CHANNELB_ACTIVE())
    #endif
			{
				u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG18);
				u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_MEM_DQ2BYTE_AB_CFG19);
			}
    #if 0 //fcFOR_ONE_CHANNEL_DESIGN
			else
			{
				u4addr_array[0] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG18);
				u4addr_array[1] = mcSET_PHY_REG_ADDR(DDRPHY_REG_INT_CMD_DQ2BYTE_B2_CFG19);
			}
    #endif
	
			u4addr_array[2] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_TOGGLE_CNT_2);
			u4addr_array[3] = mcSET_DRAMC_REG_ADDR(DRAMC_REG_DQS_ERR_CNT_2);
		}
	
		// check if SoC platform has "%" operation?!
		ucidx_dqs =  dqs_no % 2;
			
		//Enable DQ eye scan
		//RG_??_RX_EYE_SCAN_EN
		//RG_??_RX_VREF_EN 
		//RG_??_RX_VREF_OP_EN 
		//RG_??_RX_SMT_EN
		u4value = ucDram_Register_Read(u4addr_array[0]);
		u4value = u4value | 0x0000e000;
		ucDram_Register_Write(u4addr_array[0], u4value);
	
		u4value = ucDram_Register_Read(u4addr_array[1]);
		u4value = u4value | 0xffff0000;
		ucDram_Register_Write(u4addr_array[1], u4value);
	
		//Enable DQS jitter meter mode (RG_??_RX_DQS_MIOCK_SEL=0, RG_RX_MIOCK_JIT_EN=0)
		//RG_??_RX_DQS_MIOCK_SEL=0
		u4value = ucDram_Register_Read(u4addr_array[0]);
		u4value = u4value & 0xffffefff;
		ucDram_Register_Write(u4addr_array[0], u4value);
	
		//RG_RX_MIOCK_JIT_EN=0 (Justin, 2012/09/28)
		u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
		mcCLR_BIT(u4value, POS_DCBLN_RX_MIOCK_JIT_EN);
		ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
	
		//Select DQS to be scanned (0 or 1)
		//RG_??_RX_DQS_EYE_SEL
		u4value = ucDram_Register_Read(u4addr_array[0]);
		mcSET_FIELD(u4value, ucidx_dqs, 0x00800000, 23)
		ucDram_Register_Write(u4addr_array[0], u4value);	
	
		//Disable DQ eye scan (b'1), for counter clear
		u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
		mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
		ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
	
		mcSHOW_DBG_MSG("====================================================\n"));
		mcSHOW_DBG_MSG("	DQS jitter meter (channel=%d(2:cha, 3:chb), dqs_%d)\n", !IS_DRAM_CHANNELB_ACTIVE(), dqs_no);
		mcSHOW_DBG_MSG("====================================================\n");
	
		for (ucdqs_dly=0; ucdqs_dly<128; ucdqs_dly++)
		{
			//Set DQS delay (RG_??_RX_DQS_EYE_DLY)
			u4value = ucDram_Register_Read(u4addr_array[0]);
			mcSET_FIELD(u4value, ucdqs_dly, 0x007f0000, 16);
			ucDram_Register_Write(u4addr_array[0], u4value);
	
			//Reset eye scan counters (reg_sw_rst): 1 to 0
			u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
			mcSET_BIT(u4value, POS_DCBLN_REG_SW_RST);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
			mcCLR_BIT(u4value, POS_DCBLN_REG_SW_RST);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
	
			//Enable DQ eye scan (b'1)
			u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
			mcSET_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
	
			// enable TE2, audio pattern
			u4err_value = DramcEngine2(TE_OP_WRITE_READ_CHECK, test2_0, test2_1, test2_2);						  
	
			//Disable DQ eye scan (b'1), for counter latch
			u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN));
			mcCLR_BIT(u4value, POS_DCBLN_RX_EYE_SCAN_EN);
			ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
	
			//Read the counter values from registers (toggle_cnt*, dqs_err_cnt*); *_cnt is for (DQS0,1) and *_cnt_2 is for (DQS2,3)
			u4sample_cnt = ucDram_Register_Read(u4addr_array[2]);
			u4ones_cnt = ucDram_Register_Read(u4addr_array[3]);
	
			mcSHOW_DBG_MSG("%3d | %8x --%8x \n", ucdqs_dly, u4sample_cnt, u4ones_cnt);		   
		}
	
		//RG_RX_MIOCK_JIT_EN=0
		//u4value = ucDram_Register_Read(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), &u4value);
		//mcCLR_BIT(u4value, POS_DCBLN_RX_MIOCK_JIT_EN);
		//ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DCBLNC_EYESCAN), u4value);
	
		return DRAM_OK;
		
		// log example
		/*
	===================================================================
	
				DQS jitter meter (channel=2(2:cha, 3:chb), dqs_0)
	===================================================================
	  0 |	   800 --		0
	  1 |	   800 --		0
	  2 |	   800 --		0
	  3 |	   800 --		0
	  4 |	   800 --		0
	  5 |	   800 --		0
	  6 |	   800 --		0
	  7 |	   800 --		0
	  8 |	   800 --		0
	  9 |	   800 --		0
	 10 |	   800 --		0
	 11 |	   800 --		0
	 12 |	   800 --		0
	 13 |	   800 --		0
	 14 |	   800 --		0
	 15 |	   800 --	   2a
	 16 |	   800 --	  325
	 17 |	   800 --	  400
	 18 |	   800 --	  400
	 19 |	   800 --	  425
	 20 |	   800 --	  568
	 21 |	   800 --	  7f2
	 22 |	   800 --	  800
	 23 |	   800 --	  800
	 24 |	   800 --	  800
	 25 |	   800 --	  800
	 26 |	   800 --	  800
	 27 |	   800 --	  800
	 28 |	   800 --	  800
	 29 |	   800 --	  800
	 30 |	   800 --	  800
	 31 |	   800 --	  800
	 32 |	   800 --	  800
	 33 |	   800 --	  800
	 34 |	   800 --	  800
	 35 |	   800 --	  800
	 36 |	   800 --	  800
	 37 |	   800 --	  800
	 38 |	   800 --	  7fe
	 39 |	   800 --	  6ba
	 40 |	   800 --	  46c
	 41 |	   800 --	  400
	 42 |	   800 --	  3ff
	 43 |	   800 --	  380
	 44 |	   800 --	  109
	 45 |	   800 --		1
	 46 |	   800 --		0
	 47 |	   800 --		0
	 48 |	   800 --		0
	 49 |	   800 --		0
	 50 |	   800 --		0
	 51 |	   800 --		0
	 52 |	   800 --		0
	 53 |	   800 --		0
	 54 |	   800 --		0
	 55 |	   800 --		0
	 56 |	   800 --		0
	 57 |	   800 --		0
	 58 |	   800 --		0
	 59 |	   800 --		0
	 60 |	   800 --		0
	 61 |	   800 --		0
	 62 |	   800 --		0
	 63 |	   800 --		4
	 64 |	   800 --		4
	 65 |	   800 --	  1b6
	 66 |	   800 --	  35f
	 67 |	   800 --	  400
	 68 |	   800 --	  468
	 69 |	   800 --	  6ee
	 70 |	   800 --	  7e4
	 71 |	   800 --	  800
	 72 |	   800 --	  800
	 73 |	   800 --	  800
	 74 |	   800 --	  800
	 75 |	   800 --	  800
	 76 |	   800 --	  800
	 77 |	   800 --	  800
	 78 |	   800 --	  800
	 79 |	   800 --	  800
	 80 |	   800 --	  800
	 81 |	   800 --	  800
	 82 |	   800 --	  800
	 83 |	   800 --	  800
	 84 |	   800 --	  800
	 85 |	   800 --	  800
	 86 |	   800 --	  800
	 87 |	   800 --	  800
	 88 |	   800 --	  800
	 89 |	   800 --	  7fb
	 90 |	   800 --	  747
	 91 |	   800 --	  437
	 92 |	   800 --	  217
	 93 |	   800 --	   59
	 94 |	   800 --		2
	 95 |	   800 --		0
	 96 |	   800 --		0
	 97 |	   800 --		0
	 98 |	   800 --		0
	 99 |	   800 --		0
	100 |	   800 --		0
	101 |	   800 --		0
	102 |	   800 --		0
	103 |	   800 --		0
	104 |	   800 --		0
	105 |	   800 --		0
	106 |	   800 --		0
	107 |	   800 --		0
	108 |	   800 --		0
	109 |	   800 --		0
	110 |	   800 --	   3f
	111 |	   800 --	  27e
	112 |	   800 --	  3f2
	113 |	   800 --	  414
	114 |	   800 --	  4ad
	115 |	   800 --	  77e
	116 |	   800 --	  7ea
	117 |	   800 --	  800
	118 |	   800 --	  800
	119 |	   800 --	  800
	120 |	   800 --	  800
	121 |	   800 --	  800
	122 |	   800 --	  800
	123 |	   800 --	  800
	124 |	   800 --	  800
	125 |	   800 --	  800
	126 |	   800 --	  800
	127 |	   800 --	  800
	   */
}

#endif

