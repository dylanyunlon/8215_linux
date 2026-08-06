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

/** @file ddr.h
 *  Dram configurations and options.
 *  dram api decalaration and calibration item select
 */

#ifndef DDR_H
#define DDR_H
#include "x_typedef.h"
#include "x_dram.h"

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------

//#define DRAM_SLT_DEBUG   /* Use for SLT and Nand boot debug DRAM loader */
#if 1//mtk40581
#define DRAM_Pll_PHASE_CAL
#define DRAM_IMPEDANCE_CAL 
#define DRAM_WRITE_LEVELING_CAL   //For tDQSS
#define DRAM_MiockJmeter          //for gating window
#define DRAM_GATING_SCAN
//#define DRAM_RX_EYE_SCAN //mtk40739 add 
//#define DRAM_TX_EYE_SCAN //mtk40739 add 
#define DRAM_RX_DATLAT_CAL
#define DRAM_RX_WINDOW_PERBIT_CAL
#define DRAM_TX_WINDOW_PERBIT_CAL
#endif

//#define DRAM_WRITE_READ_LOOP_AFTER_CALIBRATION //mtk40739


#ifndef DRAM_TX_PERBIT_DQ_DESKEW
//#define DRAM_TX_PERBIT_DQ_DESKEW
#endif
#ifdef DRAM_TX_PERBIT_DQ_DESKEW
//#define DRAM_TX_PERBIT_DQM_DESKEW
#endif
//#define DRAM_RX_ODT_SCAN
//#define DRAM_TX_VREF_CAL
//#define DRAM_TX_OCD_DRV_SCAN
//#define DRAM_RX_EYE_SCAN
//#define DRAM_TX_EYE_SCAN
//#define DRAM_LOAD_BOARD
//#define DRAM_WRITE_READ_LOOP_AFTER_CALIBRATION

//init
#define SR_VALUE 5   // slew rate
#define SR_VALUE_CLK 0xf // slew rate of CLK
//#define fcFOR_16BIT_DESIGN   // for MT1389  //zhishang 3363 should mark this
#define MT8563_16BIT_DESIGN   // for MT8563
//#define MT8563_8BIT_DESIGN   // for MT8563

#define fcFOR_3363_32BIT  //zhishang add this for 3363


//#define fcFor_8BIT_DESIGN    // for MT8563

#ifndef DEFAULT_OCDP_DRIVING
#define DEFAULT_OCDP_DRIVING 0x17
#endif 
#ifndef DEFAULT_OCDN_DRIVING
#define DEFAULT_OCDN_DRIVING 0x17
#endif
#ifndef DEFAULT_ODTP_DRIVING
#define DEFAULT_ODTP_DRIVING 0x17
#endif
#ifndef DEFAULT_ODTN_DRIVING
#define DEFAULT_ODTN_DRIVING 0x17
#endif
#ifndef DEFAULT_OCDP_DRIVING_CLK
#define DEFAULT_OCDP_DRIVING_CLK 0xb8
#endif
#ifndef DEFAULT_OCDN_DRIVING_CLK
#define DEFAULT_OCDN_DRIVING_CLK 0xb8
#endif

//tx vref define
#ifndef DEFAULT_TX_VREF
#define DEFAULT_TX_VREF 8
#endif

#ifndef DEFAULT_MR1_VALUE
#define DEFAULT_MR1_VALUE 0x00002004
#endif
#ifndef DEFAULT_MR2_VALUE
#define DEFAULT_MR2_VALUE 0x00004020
#endif
//default perbyte DQ delay
#ifndef DEFAULT_PI_A1_DQA
#define DEFAULT_PI_A1_DQA 0x10
#endif
#ifndef DEFAULT_PI_A1_DQB
#define DEFAULT_PI_A1_DQB DEFAULT_PI_A1_DQA
#endif
#ifndef DEFAULT_PI_A2_DQA
#define DEFAULT_PI_A2_DQA DEFAULT_PI_A1_DQA
#endif
#ifndef DEFAULT_PI_A2_DQB
#define DEFAULT_PI_A2_DQB DEFAULT_PI_A1_DQA
#endif
#ifndef DEFAULT_PI_B1_DQA
#define DEFAULT_PI_B1_DQA 0x10
#endif
#ifndef DEFAULT_PI_B1_DQB
#define DEFAULT_PI_B1_DQB DEFAULT_PI_B1_DQA
#endif
#ifndef DEFAULT_PI_B2_DQA
#define DEFAULT_PI_B2_DQA DEFAULT_PI_B1_DQA
#endif
#ifndef DEFAULT_PI_B2_DQB
#define DEFAULT_PI_B2_DQB DEFAULT_PI_B1_DQA
#endif

#define DEFAULT_TEST2_1_DQSIEN 0x55000000   // pattern0 and base address for test engine when we do dqs gating window
#define DEFAULT_TEST2_2_DQSIEN 0xaa000010   // pattern1 and offset address for test engine when we  do dqs gating window
#define DEFAULT_GOLD_DQSIEN    0x20202020   // gold pattern for dqsien compare
#define DEFAULT_DRAM_MODE      MODE_2X

// PLL phase calibration
#define PLL_PHASE_CAL_CONF_COUNT 16

// gating window
#define DLY_DQSIENSTB_LOOP   7   // coarse tune 2T loop
#define DLY_DQSIENSTB_MAX    7   // max is 7, may reduce for time saving
#define RX_DQS_CTL_LOOP      4    // 4*0.5T = 2T
#define RX_DQS_CTL_MAX       12    // may no need to use this
#define RX_DLY_DQSIENSTB_MAX 63   // max fine tune value

// defined for allocating max array size
#define DQS_BIT_NUMBER       8
//#ifndef fcFOR_16BIT_DESIGN        //zhishang 3363 here may should modify
#define DQS_NUMBER           4
#define DQ_DATA_WIDTH        32
//#else
//#define DQS_NUMBER           2
//#define DQ_DATA_WIDTH        16
//#endif


// RX DQ/DQS
#define MAX_RX_DQSDLY_TAPS   16   // 0x018, 0~63 delay tap
#define MAX_RX_DQDLY_TAPS    16      // 0x210~0x22c, 0~15 delay tap

// DATLAT
#define DATLAT_TAP_NUMBER    16   // DATLAT[3:0] = {0x0e4[4] 0x07c[6:4]}

#define CLK_1866MHZ    1866000000
#define CLK_1700MHZ    1700000000
#define CLK_1600MHZ    1600000000
#define CLK_1333MHZ    1333000000
#define CLK_1242MHZ    1242000000
#define CLK_1188MHZ    1188000000
#define CLK_1080MHZ    1080000000
#define CLK_1066MHZ    1066000000
#define CLK_972MHZ      972000000
#define CLK_810MHZ      810000000
#define CLK_800MHZ      800000000

#define DRAM_CHANGE_CLK_SETTING						1
#define DRAM_GEN_TEST_PATTERN_AFTER_CALIBRATION		2
#define DRAM_CHANGE_CLK_PI_SETTING					3

#define CC_CHA_CHB_NO_GAP

#define DATA_WIDTH_32BIT 32
#define DATA_WIDTH_16BIT  16
#define DATA_WIDTH_8BIT   8

#define DDR_GET_SIZE_CONFIG(bl_config)	        ((bl_config)& 0x000000FF)
#define DDR_GET_CLOCK_CONFIG(bl_config)	        (((bl_config)>>8) & 0x000000FF)
#define DDR_GET_BITWIDTH_CONFIG(bl_config)      (((bl_config)>>16) & 0x000000FF)
#define DDR_GET_VERSION_PCB_CONFIG(bl_config)      (((bl_config)>>24) & 0x000000FF)

#define DRAM_PCB_VERSION_MT8563P1V1          1
#define DRAM_PCB_VERSION_MT8659P1V1          2
#define DRAM_PCB_VERSION_MT1389P1V1          3
#define DRAM_PCB_VERSION_MT8507P1V1          4
#define DRAM_PCB_VERSION_MT8639P1V1          5
#define DRAM_PCB_VERSION_MT8639P2V1          6


/***********************************************************************/
/*              External declarations                                  */
/***********************************************************************/
#if 0
#if !defined(__MODEL_slt__) || defined(DRAM_SLT_DEBUG)
EXTERN void CLK_QueryDramSetting(UINT8 *szString, UINT32 u4Stage);
#else
#define CLK_QueryDramSetting(x, y)
#endif
#endif

/***********************************************************************/
/*              Public Functions                                       */
/***********************************************************************/
// basic function
void DdrPhyInit(void);
void DramcInit(void);
void DramcConfig(void);
void DramcDqDriving(U8 ocd_pvalue, U8 ocd_nvalue, U8 odt_pvalue, U8 odt_nvalue);
void DramcDqsDriving(U8 ocd_pvalue, U8 ocd_nvalue, U8 odt_pvalue, U8 odt_nvalue);
void DramcDqmDriving(U8 ocd_pvalue, U8 ocd_nvalue, U8 odt_pvalue, U8 odt_nvalue);
void DramcClkDriving(U8 ocd_pvalue, U8 ocd_nvalue);
void DramcCaDriving(U8 ocd_pvalue, U8 ocd_nvalue);
//U32 DramcEngine1(U32 test2_0, U32 test2_1, U32 test2_2, S16 loopforever, U8 period);
DRAM_STATUS_T DramcRegDump(void);


void ddr_init_dramc_dram(void);
void ddr_calibrate(void);
void dram_test_after_cali(void);
void vDDR_DramReSizing(void);
void vDDR_DramCheckPCBVersion(void);


// mandatory calibration function
void DramcPllPhaseCal(void);
void DramcImpedanceCalTxOcd(void);
void DramcImpedanceCalRxOdt(void);
void DramcImpedanceCalApply(void);
void DramcWriteLeveling(void);
void DramcMiockJmeter(void);
DRAM_STATUS_T DramcRxdqsGatingCal(void);
DRAM_STATUS_T DramcRxWindowPerbitCal(void);
DRAM_STATUS_T DramcRxdatlatCal(U32 u4CalStartAddr);
DRAM_STATUS_T DramcTxWindowPerbitCal(void);
void DramcTxVrefCal(void);
void DramcTxOcdDrvScan(void);
void DramcRxOdtScan(void);

// reference function
DRAM_STATUS_T DramcRxEyeScan(U8 dq_no);
DRAM_STATUS_T DramcTxEyeScan(void);
DRAM_STATUS_T DramcDqsJmeter(U8 dqs_no);


//Suspend 
void DDR_EnterSuspend(U32 *pu4SaveAddr);
void SaveDramBackupReg(U32 *pu4SaveAddr);
void DramcBackupReg(U32 *u4DRAMCTemp);
void DramcBackupCommonReg(U32 *u4DRAMCTemp);
void DramcEnterSR(void);

//Resume
void DDR_EnterResume(U32 *pu4SaveDramRegAddr, U32 u4CalAddStart);
void ReInitDram(U32 *pu4SaveDramRegAddr);
void DRAMC_Exit_Suspend(U32 u4CalAddStart);
void  DramcExitSR(U32 u4CalAddStart);
void DramcWriteBackReg(U32 *u4DRAMCTemp);
void DramcWriteBackCommonReg(U32 *u4DRAMCTemp);



#endif // _DDR_H

