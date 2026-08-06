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

#ifndef X_DRAM_H
#define X_DRAM_H

#include "x_typedef.h"
#include "../drv_cust/ac8317_m1v1_v00.h"

// DRAM data read/write macros
#define INIT_DRAM_B_BASE            0x80000000
#define INIT_DRAM_B_CHB_BASE    	0xc0000000

// timeout for TE2: (CMP_CPT_POLLING_PERIOD X MAX_CMP_CPT_WAIT_LOOP) 
// complete flag, need to double check with DE
#define CMP_CPT_POLLING_PERIOD  2  // 1 
#define MAX_CMP_CPT_WAIT_LOOP  100  // max loop 100

#define DEFAULT_TEST2_0_CAL 0xaa550000   // patter 1 and 0
#if SUPPORT_2GB_SIZE
#define DEFAULT_TEST2_1_CAL 0x40000000  
#else
#define DEFAULT_TEST2_1_CAL 0x30000000   // base address for test engine when we do calibration, [30:28] is row mask, A15 masked 
#endif
#define DEFAULT_TEST2_2_CAL 0x00000400   // offset address for test engine when we  do calibraion

// suspend/resume
#define DRAMC_BACKUP_REG_NUM		15// number of backup registers in DRAMC when suspend
#define DRAMRB_BACKUP_REG_NUM		8
#define PHY_BACKUP_REG_NUM			70   // number of backup registers in DDRPHY when suspend
#define CHA_CHB_COMMON_REG_NUM		5	//cha&chb common register when suspend

#define   REG_RW_CONF     0x0
#define   REG_RW_PRI0     0x10
#define   REG_RW_PRI1     0x14
#define   REG_RW_AGTIM0   0x20
#define   REG_RW_AGTIM1   0x24
#define   REG_RW_STARVE1  0x34
#define   REG_RW_STARVE2  0x38
#define   REG_RW_STARVE6  0x48


//xtal frequency
#define XTAL_MHZ	    27   //Crystal is 27MHz

/***********************************************************************/
/*          Public Types                                               */
/***********************************************************************/
#if 0
typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned long   U32;
typedef unsigned int    UINT;

typedef signed char     S8;
typedef signed short    S16;
typedef signed long     S32;
typedef signed int      SINT;
#endif


typedef U32             DWORD;  //32 bits

typedef U8              Data8;
typedef U16             Data16;
typedef U32             Data32;

typedef U8              *PU8;
typedef U16             *PU16;
typedef U32             *PU32;
typedef S8              *PS8;
typedef S16             *PS16;

/***********************************************************************/
/*              Defines                                                */
/***********************************************************************/
#define ENABLE  1
#define DISABLE 0

typedef enum
{
    DRAM_OK = 0, // OK
    DRAM_FAIL    // FAIL
} DRAM_STATUS_T; // DRAM status type

typedef enum
{
	DRAM_CALIBRATION_PASS = 0,
	DRAM_CALIBRATION_FAIL,
	DRAM_CALIBRATION_WINDOW_SIZE_TOO_SMALL,
	DRAM_CALIBRATION_WINDOW_SIZE_TOO_BIG
} DRAM_CALIBRATION_STATUS;

typedef enum
{
    TE_OP_WRITE_READ_CHECK = 0,
    TE_OP_READ_CHECK
} DRAM_TE_OP_T;

typedef enum
{
    DLINE_0 = 0,
    DLINE_1,
    DLINE_TOGGLE    
} PLL_PHASE_CAL_STATUS_T;


////////////////////////////

typedef struct _RXDQS_PERBIT_DLY_T
{
    S8 first_dqdly_pass;
    S8 last_dqdly_pass;
    S8 first_dqsdly_pass;
    S8 last_dqsdly_pass;
    U8 best_dqdly;
    U8 best_dqsdly;
} RXDQS_PERBIT_DLY_T;

typedef struct _TXDQS_PERBIT_DLY_T
{
    S8 first_dqdly_l_pass;
    S8 last_dqdly_l_pass;
    S8 first_dqdly_r_pass;
    S8 last_dqdly_r_pass;
    S8 pi_dqdly_ok_center;
} TXDQS_PERBIT_DLY_T;

/************************ Bit Process *************************/
#define mcBITL(b)               (1L << (b))
#define mcBIT(b)                (1L << (b))
#define mcMASK(w)               (mcBIT(w) - 1)
#define mcMASKS(w, b)           (mcMASK(w) << (b))
#define mcFIELD(val, msk, pos)  (((val) << (pos)) & (msk))

#define mcSET_MASK(a, b)        ((a) |= (b))
#define mcCLR_MASK(a, b)        ((a) &= (~(b)))
#define mcCHK_MASK(a, b)        ((a) & (b))
//#define mcSET_BIT(a, b)         mcSET_MASK(a, mcBIT(b))
#define mcSET_BIT(a, b)         ((a) |= ((U32)1L<<(b)))
//#define mcCLR_BIT(a, b)         mcCLR_MASK(a, mcBIT(b))
#define mcCLR_BIT(a, b)         ((a) &= (~((U32)1L<<(b))))
#define mcCHK_BIT1(a, b)        ((a) & mcBIT(b))
#define mcCHK_BITM(a, b, m)     (((a) >> (b)) & (m))
#define mcCHK_BITS(a, b, w)     mcCHK_BITM(a, b, mcMASK(w))
//#define mcTEST_BIT(a, b)        mcCHK_BITM(a, b, 1)
#define mcTEST_BIT(a, b)        mcCHK_BIT1(a, b)
#define mcCHG_BIT(a, b)         ((a) ^= mcBIT(b))

#define mcSET_FIELD0(var, val, msk, pos)    mcSET_MASK(var, mcFIELD(val, msk, pos))

#define mcSET_FIELD(var, value, mask, pos)  \
{                                           \
    mcCLR_MASK(var, mask);                  \
    mcSET_FIELD0(var, value, mask, pos);    \
}

#define mcGET_FIELD(var, mask, pos)     (((var) & (mask)) >> (pos))

//mtk40739 modify this for 3363 code refine
//#define mcSET_DRAMC_REG_ADDR(offset)    ((IS_DRAM_CHANNELB_ACTIVE() ? DRAM_DRAMC_CHB_BASE : DRAM_DRAMC_BASE) | (offset))
#define mcSET_DRAMC_REG_ADDR(offset)     (DRAM_DRAMC_BASE | (offset))

#define mcSET_PHY_REG_ADDR(offset)    	((DRAM_DDRPHY_BASE) | (offset))

//mtk40739 modify this for 3363 code refine
//#define mcSET_ARBITER_ADDR(offset)		((IS_DRAM_CHANNELB_ACTIVE() ? DRAM_DMARB_CHB_BASE : DRAM_DMARB_BASE) | (offset))
#define mcSET_ARBITER_ADDR(offset)       (DRAM_DMARB_BASE  | (offset))


// DRAM controller register read/write macros
#define ucDram_Register_Read(reg)			HAL_READ32(reg)
#if 1
#define ucDram_Register_Write(reg, value)	HAL_WRITE32(reg, value)
#else
//For DE run simulation
#define ucDram_Register_Write(reg, value)	do{\
												HAL_WRITE32(reg, value); \
												Printf("RISCWrite(32'h%08X, 32'h%08X);\n", reg&(~0x70000000), value);\
											}while(0)


#endif
/***********************************************************************/

#if !defined(CC_MTK_PRELOADER) && !defined(CC_MTK_LOADER)
typedef struct _DRAM_CFG_T
{
    UINT8       ui1_ssc_range;
    UINT8       ui1_ssc_modulation;
    UINT8       ui1_clk_driving;
    UINT8       ui1_clk_delay;
    UINT8       ui1_cmd_driving;
    UINT8       ui1_cmd_delay;
    UINT8       ui1_wdqs_driving;
    UINT8       ui1_wdqs0_delay;
    UINT8       ui1_wdqs1_delay;
    UINT8       ui1_wdqs2_delay;    
    UINT8       ui1_wdqs3_delay;        
    UINT8       ui1_wdq_driving;
    UINT8       ui1_wdq0_delay;    
    UINT8       ui1_wdq1_delay;    
    UINT8       ui1_wdq2_delay; 
    UINT8       ui1_wdq3_delay;        
} DRAM_CFG_T;
#endif /* !defined(CC_MTK_PRELOADER) && !defined(CC_MTK_LOADER) */

// DDR related functions.
extern CHAR * DDR_DramTypeString(UINT32 u4Type);
extern UINT32 DDR_IsDqsFail(UINT32 u4DQSth, UINT32 u4DQSVal, UINT32 u4DQSType, UINT32 u4ChipNum);
extern UINT32 DDR_CalibrateDqs(void);
extern UINT32 DDR_CalibrateOutDqs(void);
extern void DDR_SetDramController(void);
extern void DDR_SetAgentPriority(const UINT8 *pu1Prio);
extern void DDR_SetBurstLen(UINT32 u4TimeSlot1, UINT32 u4TimeSlot2);
extern void DDR_SetArbiterTime(UINT8 u1Group, UINT8 u1Time);
extern UINT32 DRAMC_str_save_registers(UINT32 *u4DRAMCTemp, UINT32 u4Size);
extern void DRAMC_str_restore_registers(UINT32 *u4DRAMCTemp);
//extern void DRAMC_Exit_Suspend(UINT32 *u4DRAMCTemp);
extern void DMARC_TCM_suspend(void);
extern void DMARC_TCM_resume(void);
extern void DMARC_TCM_partial_resume(void);
extern U32 DramcEngine2(DRAM_TE_OP_T wr, U32 test2_0, U32 test2_1, U32 test2_2);

/*
    DRAM configuration API for factory mode.
        res_mngr\drv\u_drv_cust.h
        mw_if\drv_cust_api.c
*/ 
#if !defined(CC_MTK_PRELOADER) && !defined(CC_MTK_LOADER)
extern BOOL DDR_SetCustCfg(DRAM_CFG_T *prDdrCfg);
extern BOOL DDR_GetCustCfg(DRAM_CFG_T *prDdrCfg);
#endif /* !defined(CC_MTK_PRELOADER) && !defined(CC_MTK_LOADER) */

#define DRAM_DRAMC_READ32(offset)                    IO_READ32(DRAM_DRAMC_BASE, offset)
#define DRAM_DRAMC_WRITE32(offset, value)            IO_WRITE32(DRAM_DRAMC_BASE, offset, (value))

#define DRAM_DMARB_READ32(offset)                    IO_READ32(DRAM_DMARB_BASE, offset)
#define DRAM_DMARB_WRITE32(offset, value)            IO_WRITE32(DRAM_DMARB_BASE, offset, (value))

#define DRAM_DDRPHY_READ32(offset)                   IO_READ32(DRAM_DDRPHY_BASE, offset)
#define DRAM_DDRPHY_WRITE32(offset, value)           IO_WRITE32(DRAM_DDRPHY_BASE, offset, (value))

#define   REG_RW_DMSUS               0xF0024038
#define   BIT_DMSUS                  0

#define DRAMB_REG_DYNPRI             (0x0000006C)
#define BIT_PRIUPDAGE_EN             (1<<0)
#define BIT_PSTWRFLS_EN              (1<<5)
#define BIT_MERFLS_EN                 (1<<4)


/*DRAM Agent Priority Setting*/
/*
*typedef struct
*{
*	UINT32 u4GroupID;
*	UINT32 u4AgtID;
*	UINT32 u4Priorty;
*	TCHAR  cAgtName[300];
*}T_AGTINFO,*P_T_AGTINFO;
*
*T_AGTINFO g_tAgtInfoTbl[]=
*{  //original config
*	{1, 0,  3,  _T("mali_reg(gfx_3d)/ demux_req1(ts_demux)/ddi_req(ts_demux)/img_resz_req(img_resz)/gfx24bpp_req(gfx24bpp)")},
*	{1, 1,  13, _T("mali_req(gfx_3d)/mgra_req(AP_2D)/jpgdec_req(jpgdec)/rle_req(rle_dec)/png_req(png_decoder)/gif_req(gif_decoder)/osd_resz_req(osd_resz)/demux_req2 (ts_demux)")},
*	{1, 2,  2,  _T("CA7")},
*	{1, 3,  14, _T("arm9_req/USB1/NFI_req/bim_local_req_req (has layer2 arbitor)/USB0_req/irt_dma/")},
*	{1, 4,  1,  _T("mphone_req/aout_req/aout2_req/iec_req/iec2_req/gps_aout_req/pcm_rx_req/pcm_tx_req")},
*	{1, 5,  9,  _T("audio_largl_0_req/audio_largl_1_req/audio2_largl_0_req/spi_mtk_dram_req/spi_moto1_dram_req/spi_moto2_dram_req")},
*	{1, 6,  15, _T("msdc_0/1/2_dram_hreqm /rfi_dram_req/au_peri_larb_2/3_req")},
*	{1, 7,  7,  _T("VDEC Pred/MC/VDEC CABAC/VDEC PP/connect to 0/VDEC VLD")},
*	{1, 8,  8,  _T("VDEC MC/Pred")},
*	{1, 9,  6,  _T("audio-vdo out/asm_rd_req/ain_dma_req/vdo_dram_req")},
*	{1, 10, 11, _T("cor_wreq/dspreq/c2req/edcw_req/screq/correq/cddec_req/pio_req")},
*	{1, 11, 12, _T("vdec /adsp/mc req/t32_ic_rd/risc_dram_req/t8032_dram_req")},
*	{1, 12, 5,  _T("VDO:VDO_F/VDO_R")},
*	{1, 13, 4,  _T("OSD:OSD1-5/OSD2_R/OSD3_R")},
*	{1, 14, 10, _T("TVD:TVD/VBI/WRITE_CHANNEL")},
*	{1, 15, 15, _T("2D/img_risize0/png/jpeg/gif/img_risize0")},
*};
*/
#define DRAMB_REG_PRI0               (0x00000010)
#define DRAMB_REG_PRI1               (0x00000014) 
#define DRAMB_REG_DYNPRI             (0x0000006C)
//#define DYNPRI0                       0x7f91E2D3
//#define DYNPRI1                       0xFA45CB68
/*
*       img_resz_req    VDO    OSD
*  ori      3            5      4
*  now      5            3      4
*/
//#define DYNPRI0                       0x7f91E2D5
//#define DYNPRI1                       0xFA43CB68
//osd priotity up ap priotity down for fix  disp screen problem
/*
*       	CA7      OSD
*  ori      2          4
*  now      4          2  
*/
#define DYNPRI0                       0x7f91E4D5
#define DYNPRI1                       0xFA23CB68
#define BIT_PRIUPDAGE_EN              (1<<0)


#define  DRAMB_REG_PROT0_0          (0x00000200) 
#define  REGION_BOUND_MASK          (0x7FFFFFFC)
#define  BIT_PROTECT_EN             (1U << 31)
#define  BIT_IN_PROTECT_MODE        (0U << 30)
#define  BIT_OUT_PROTECT_MODE       (1U << 30)
#define  BIT_WEN_PROTECT            (1U << 29) 
#define  BIT_REN_PROTECT            (1U << 28)

#define  DRAMB_REG_INTR0            (0x00000280)
#define  REG_RO_INTR0               (0x00006280)
#define  BIT_INTR_INTRUDEN          (1U << 31)
#define  INTRADR_MASK               (0x7FFFFFFC)

#define  DRAMB_REG_INTRUID0         (0x000002A0)
#define  INTR_AGTID_MASK            (0x0000000F)
#define  INTR_SUBID_MASK            (0x000000F0)
#define  INTR_SUBID_SHIFT           4

#define  DRAMB_REG_INTCLR           (0x00000270)
#define  INTCLR_MASK                (0xFFFFFFFE) 


#define  DRAMB_REG_BMCYC            (0x0000008C)
#define  BMCYC_MASK                 (0xFFFFFFFF) 
          

#define DRAMB_REG_BM                (0x00000080)
#define BM_GROUP1                   1
#define BM_GROUP2                   2
#define BM_GROUP3                   3
#define BM_BMGP1_AG_MASK            (0x00001F00)
#define BM_BMGP1_AG_OFFSET           8
#define BIT_BMGP1_EN                (1U << 15)
#define BM_BMGP2_AG_MASK            (0x00070000)
#define BM_BMGP2_AG_OFFSET          16
#define BIT_BMGP2_EN                (1U << 19)
#define BM_BMGP3_AG_MASK            (0x00700000)
#define BM_BMGP3_AG_OFFSET          20
#define BIT_BMGP3_EN                (1U << 23)
#define BIT_REQCNTEN                (1U << 30)
#define BIT_PACNTEN                 (1U << 31)


#define DRAMB_REG_BM0               (0x00000090)   
#define BM_BMGP_AG_MASK             (0x00700000)
#define BM_BMGP3_AG_OFFSET           20            
#define DRAMB_REG_BM3               (0x0000009C) 

#define DRAMB_REG_CONF               (0x00000000)
#define BIT_AG15_EN                  (1 << 29)
#define BIT_ARBCPUQK_EN              (1 << 23)
#define DEAFULT_TESTB_ADDR           (0x100000)
#define DRAMB_REG_TEST0_0            (0x00000100)
#define DEFALUT_TESTB_LEN            (0x800000)
#define DRAMB_REG_TEST0_1            (0x00000104)
#define DRAMB_REG_TEST               (0x00000118)
#define BIT_WAGENT_EN                (1 << 28)
#define BIT_PSTTHD                   12

#define DRAMB_REG_TEST0_RESULT       (0x00000140)
#define CMP_CPT0_MASK                (0x00000001)  //0
#define CMP_ERR0_OFFSET               4// 4
#define DLE_CNT_OK_OFFSET             8  //8

typedef struct
{
   DWORD dwHighAddr;
   DWORD dwLowAddr;
   DWORD dwProtectAgentID;
   BOOL  bEnableRegion;
   BOOL  bIncludeRegion;
   BOOL  bWriteProctect;
   BOOL  bReadProctect;
}T_REGIONINFO, *P_T_REGIONINFO;


typedef struct
{
    DWORD dwAgtID;
    DWORD dwSubID;
    DWORD dwAddr;
}T_INTRUDEINFO,*P_T_INTRUDEINFO;





#endif /* X_DRAM_H */
