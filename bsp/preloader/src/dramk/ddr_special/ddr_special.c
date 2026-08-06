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
/** DramcConfig
 *  DRAM configuration.
 *  @param p                Pointer of context created by DramcCtxCreate.
 *  @param data_width      (DRAM_DATA_WIDTH_T): data width
 *  @param dram_mode        (DRAM_DRAM_MODE_T): 1x or 2x mode
 *  @retval status          (DRAM_STATUS_T): DRAM_OK or DRAM_FAIL
 */
//-------------------------------------------------------------------------
void DramcConfig(void)
{
// This function may be modified by different design. Need to review with DE
    /*
         mode                 R_DM64BITEN,R_DMFREQDIV2,R_DM16BITFULL  R_DMBL4
         LPDDR2-1066 32bit 2x      1,           1,          0            1
         DDR3-1600    16bit 2x      0,           1,          0            1
         DDR3-1600    32bit 2x      1,           1,          0            1
    */
    //DDR3 SBS pinmux should be set to 00 (reg0xd8[31:30])
	U32 u4value, u4Temp;
#if 0
	  UINT32 u4BLDramPCB = 0;
    u4BLDramPCB = BIM_READ32(REG_RW_GPRDW5); // load dram config
	  u4BLDramPCB = DDR_GET_VERSION_PCB_CONFIG(u4BLDramPCB);
    ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY), 0x00100900);
#endif
#if 0
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
		ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_MCKDLY), 0x00100900);
	}

    //0x4[9:8]  control the number of the column address,
    //need set to differrnt value when use different DRAM,
    //current we set 2`b10, mean have 10 bit column address
    //A60806: 0x004=0xf07406c3
    //[15] CKE function enabling (0: disable power down function, CKE will keep high 1: enable power down function, CKE will go down when idle)
    //[5] ?
    //b15 set to 0 by A60807 (not concern power issue). May validate set to 1 case. b5 TBD
    //2012/11/06, for suspend / resume
    if (IS_DDR_SUSPENDSTATE())
    {
        //in suspend state, self refresh mode
//zhishang 3363 here should as below

	     if (TCMGET_DATA_WIDTH()==DATA_WIDTH_32BIT)
	     { // 32 bits
		       ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), 0xf47486c1);
	     }
	     else
	     { // 16 bits
		       ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), 0xf47486c0);
	     }

    }
    else
    {

	   //zhishang 3363 here should as below
	    if (TCMGET_DATA_WIDTH()==DATA_WIDTH_32BIT)
	    { // 32 bits
		      ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), 0xf07486c1);
	    }
	    else
	    { // 16 bits
		       ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_CONF1), 0xf07486c0);
	    }

    }

    //change 0x7c[7:4] from 9 to a for A channel DDR3
    //A60806: chA 0x07c=0xd48733a1, chb/c 0x07c=0xd4873391
    //[30:28] Write latency = WLAT + 1/WLAT + 3 when FDIV2 = 0/1
    //[26:24] Read ODT timing control for DDR2 (000: For CL3 001: For CL4 and CL5 010: for CL6 and CL7)
    //[6:4] Internal read data timing control
    //[3] Write ODT enabling
    //[2] Read ODT enabling
    //[30:28] depend on data rate. Excel table for AC timing calculation provided by Justin
    //[26:24] depend on data rate. Excel table for AC timing calculation provided by Justin
    //[6:4] may depend on PCB. Calibration is required
    //[3] A60807 R/W ODT are enabled
    //2012/10/03, for 2133MHz, DATLAT e -> [6:4] = 110, tR2W [15:12] = 0100
    if (TCMGET_DDR_CLK() > CLK_1700MHZ)
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), 0xe28743ed);
    else
        ucDram_Register_Write(mcSET_DRAMC_REG_ADDR(DRAMC_REG_DDR2CTL), 0xe18733cd);  //Read ODT Delay 1T By Waveform

    //A60807 verification plan has no this setting? default is 0x00000000
    //[12] CS0 is also applied to CS1
    //A60807 no CS1 for fly by. No need to set for A60807
    //#proc_write 0x2000${DRAMC_ADDR}1ec 0x00001000

    //set default driving here
    //OCD: 60ohm, ODT: 120ohm, CLK OCD: 45ohm
    DramcDqDriving(DEFAULT_OCDP_DRIVING, DEFAULT_OCDN_DRIVING, DEFAULT_ODTP_DRIVING, DEFAULT_ODTN_DRIVING);
    DramcDqsDriving(DEFAULT_OCDP_DRIVING, DEFAULT_OCDN_DRIVING, DEFAULT_ODTP_DRIVING, DEFAULT_ODTN_DRIVING);
    DramcDqmDriving(DEFAULT_OCDP_DRIVING, DEFAULT_OCDN_DRIVING, DEFAULT_ODTP_DRIVING, DEFAULT_ODTN_DRIVING);
    DramcClkDriving(DEFAULT_OCDP_DRIVING_CLK, DEFAULT_OCDN_DRIVING_CLK);
    DramcCaDriving(DEFAULT_OCDP_DRIVING, DEFAULT_OCDN_DRIVING);
    //3363 chengsheng simulation suggest @zhishang 20140716
    //reset AGENT REQ first
#if 0
    u4value = ucDram_Register_Read(0xF00000bc);
    mcCLR_BIT(u4value, 7);
    ucDram_Register_Write(0xF00000bc, u4value);
    u4value = ucDram_Register_Read(0xF00000bc);
    mcSET_BIT(u4value, 7);
    ucDram_Register_Write(0xF00000bc, u4value);
    mcSHOW_DBG_MSG("reset AGENT REQ\n");
#endif

	u4Temp = *(UINT32 *)0xF0052004;
	u4Temp &= ~(0x1F << 16);
	u4Temp |= 8 << 16;
	*(UINT32 *)0xF0052004 = u4Temp;
	u4Temp = *(UINT32 *)0xF0052104;
	u4Temp &= ~(0x1F << 16);
	u4Temp |= 8 << 16;
	*(UINT32 *)0xF0052104 = u4Temp;

	mcDELAY_us(1);
	mcSHOW_DBG_MSG2("0x52018:0x%x ", *(UINT32 *)0xF0052018);
	u4Temp = *(UINT32 *)0xF0052018;
	if(u4Temp & (1 << 5))    //Arbit 0 has 2 requests
	{
		//Reset local arbit 0
		*(UINT32 *)0xF0052004 |= 1 << 12;
		mcSHOW_DBG_MSG2("reset ");
		*(UINT32 *)0xF0052004 &= ~(1 << 12);
		mcSHOW_DBG_MSG2("0x52018:0x%x\n", *(UINT32 *)0xF0052018);
	}
	else if((u4Temp & 0x30) == 0)    //Arbit 0 has 1 request only
	{
		//Demux clock enable
		u4value = ucDram_Register_Read(0xF00000A0);
		//Clear 7 - 11 bit
		mcSET_FIELD(u4value, 0x0, 0x00000F80, 7);
		ucDram_Register_Write(0xF00000A0, u4value);

		//Demux reset disable
		u4value = ucDram_Register_Read(0xF00000BC); 
		mcSET_BIT(u4value, 7);
		ucDram_Register_Write(0xF00000BC, u4value);
		mcDELAY_us(1);
		
		// Trigger demux ddi dma
		*(UINT32 *)0xF0003000 = 0x208;
		*(UINT32 *)0xF0001140 = 0x47;
		*(UINT32 *)0xF0019c00 = 0x208;
		*(UINT32 *)0xF0019440 = 0;
		*(UINT32 *)0xF0019444 = 0;
		*(UINT32 *)0xF0019418 = 3;
		*(UINT32 *)0xF001941c = 0;
		*(UINT32 *)0xF0019850 = 0;
		*(UINT32 *)0xF0019854 = 0x10000;
		*(UINT32 *)0xF001985c = 0;
		*(UINT32 *)0xF0019460 = 6;
		*(UINT32 *)0xF0019864 = 0x10000;
		*(UINT32 *)0xF0019868 = 0x10000;
		*(UINT32 *)0xF001986c = 5;
		*(UINT32 *)0xF0019808 = 5;

		mcDELAY_us(10);
		mcSHOW_DBG_MSG2("->0x52018:0x%x\n", *(UINT32 *)0xF0052018);
		u4Temp = *(UINT32 *)0xF0052018;
		if(u4Temp & (1 << 5))
		{
			//Close demux ddi
			*(UINT32 *)0xF0019c00 = 0;
			//Demux reset enable
			u4value = ucDram_Register_Read(0xF00000BC);
			mcCLR_BIT(u4value, 7);
			ucDram_Register_Write(0xF00000BC, u4value);	   

			//Demux clock disable
			u4value = ucDram_Register_Read(0xF00000A0);
			//Set 7 - 11 bit
			mcSET_FIELD(u4value, 0x1F, 0x00000F80, 7);
			ucDram_Register_Write(0xF00000A0, u4value);

			//Reset local arbit 0
			mcDELAY_us(1);
			*(UINT32 *)0xF0052004 |= 1 << 12;
			mcSHOW_DBG_MSG2("reset ");
			*(UINT32 *)0xF0052004 &= ~(1 << 12);
			mcSHOW_DBG_MSG2("0x52018:0x%x\n", *(UINT32 *)0xF0052018);
			
			//Demux clock enable
			u4value = ucDram_Register_Read(0xF00000A0);
			//Clear 7 - 11 bit
			mcSET_FIELD(u4value, 0x0, 0x00000F80, 7);
			ucDram_Register_Write(0xF00000A0, u4value);

			//Demux reset disable
			u4value = ucDram_Register_Read(0xF00000BC);
			mcSET_BIT(u4value, 7);
			ucDram_Register_Write(0xF00000BC, u4value);	   

			//Enable demux sram begin
			mcDELAY_us(10);
			u4value = ucDram_Register_Read(0xF00120C0); 
			mcSET_BIT(u4value, 26);
			mcCLR_BIT(u4value, 27);
			ucDram_Register_Write(0xF00120C0, u4value);
		}
		else
		{
			//Close demux ddi
			*(UINT32 *)0xF0019c00 = 0;
			//Demux reset enable
			u4value = ucDram_Register_Read(0xF00000BC);
			mcCLR_BIT(u4value, 7);
			ucDram_Register_Write(0xF00000BC, u4value);	   

			//Demux clock disable
			u4value = ucDram_Register_Read(0xF00000A0);
			//Set 7 - 11 bit
			mcSET_FIELD(u4value, 0x1F, 0x00000F80, 7);
			ucDram_Register_Write(0xF00000A0, u4value);
			
			//Demux clock enable
			mcDELAY_us(1);
			u4value = ucDram_Register_Read(0xF00000A0);
			//Clear 7 - 11 bit
			mcSET_FIELD(u4value, 0x0, 0x00000F80, 7);
			ucDram_Register_Write(0xF00000A0, u4value);

			//Demux reset disable
			u4value = ucDram_Register_Read(0xF00000BC);
			mcSET_BIT(u4value, 7);
			ucDram_Register_Write(0xF00000BC, u4value);	   

			//Enable demux sram begin
			mcDELAY_us(10);
			u4value = ucDram_Register_Read(0xF00120C0); 
			mcSET_BIT(u4value, 26);
			mcCLR_BIT(u4value, 27);
			ucDram_Register_Write(0xF00120C0, u4value);
		}
	}
	mcSHOW_DBG_MSG2("0x52118:0x%x ", *(UINT32 *)0xF0052118);
	u4Temp = *(UINT32 *)0xF0052118;
	if(u4Temp & (1 << 5))    //Arbit 1 has 2 requests
	{
		//Reset local arbit 1
		*(UINT32 *)0xF0052104 |= 1 << 12;
		mcSHOW_DBG_MSG2("reset ");
		*(UINT32 *)0xF0052104 &= ~(1 << 12);
		mcSHOW_DBG_MSG2("0x52118:0x%x\n", *(UINT32 *)0xF0052118);
	}
	else if((u4Temp & 0x30) == 0)    //Arbit 1 has 1 request only
	{
		// Trigger gif decode
		*(UINT32 *)0xF00000A0 |= 0x8;
		*(UINT32 *)0xF00000BC |= 0x8;
		mcDELAY_us(1);
		*(UINT32 *)0xF0004C00 = 0x10;
		u4value = *(UINT32 *)0xF0004C58;
		u4value &= 0xfffffffb;
		u4value |= 0x3;
		*(UINT32 *)0xF0004C58 = u4value;
		*(UINT32 *)0xF0004C74 = 0x0;
		*(UINT32 *)0xF0004C78 = 0x100;
		*(UINT32 *)0xF0004C04 = 0x10;
		*(UINT32 *)0xF0004C08 = 0x100;
		*(volatile UINT32 *)0xF0004C00 = 0x110000;
		*(volatile UINT32 *)0xF0004C00 = 0x13;

		mcDELAY_us(10);
		mcSHOW_DBG_MSG2("->0x52118:0x%x\n", *(UINT32 *)0xF0052118);
		u4Temp = *(UINT32 *)0xF0052118;
		if(u4Temp & (1 << 5))
		{
			//Close gif decode
			*(UINT32 *)0xF00000BC &= ~0x8;
			*(UINT32 *)0xF00000A0 &= ~0x8;

			//Reset local arbit 1
			*(UINT32 *)0xF0052104 |= 1 << 12;
			mcSHOW_DBG_MSG2("reset ");
			*(UINT32 *)0xF0052104 &= ~(1 << 12);
			mcSHOW_DBG_MSG2("0x52118:0x%x\n", *(UINT32 *)0xF0052118);
		}
		else
		{
			//Close gif decode
			*(UINT32 *)0xF00000BC &= ~0x8;
			*(UINT32 *)0xF00000A0 &= ~0x8;
		}
	}
	else
	{
		mcSHOW_DBG_MSG2("\n");
	}

    u4value = ucDram_Register_Read(0xF00000a0);
    mcSET_BIT(u4value, 1);
    ucDram_Register_Write(0xF00000a0, u4value);
    u4value = ucDram_Register_Read(0xF00000bc);
    mcSET_BIT(u4value, 1);
    ucDram_Register_Write(0xF00000bc, u4value);
    mcSHOW_DBG_MSG2("set 0xA0[1]=1,0xBC[1]=1\n");

	u4Temp = *(UINT32 *)0xF0052018;
	mcSHOW_DBG_MSG2("0x52018:0x%x ", u4Temp);
	if((get_boot_type() == NORMAL_BOOT) && ((u4Temp & 0x10000) == 0) && (u4Temp & 0x10))
	{
		mcSHOW_DBG_MSG2("\n");
		_reset1();
	}
	u4Temp = *(UINT32 *)0xF0052118;
	mcSHOW_DBG_MSG2("0x52118:0x%x\n", u4Temp);
	if((get_boot_type() == NORMAL_BOOT) && ((u4Temp & 0x10000) == 0) && (u4Temp & 0x10))
	{
		_reset1();
	}
}

#endif
