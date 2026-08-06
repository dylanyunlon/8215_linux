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
/*
*
* For bt speech lib (include AEC, NDC, PLC...)
*
*/

#ifndef ___BT_LIB_H___
#define ___BT_LIB_H___

typedef signed short        Word16;
typedef signed long         Word32;
typedef unsigned short      uWord16;
typedef unsigned long       uWord32;

#define AEC_NDC_PARAM_NUM       28
#define DMNR_PARAM_NUM          96
#define AEC_COM_RX              22//word
#define AEC_COM_TX              22//word

// Please Follow this control Structure for AEC API Process control
typedef struct 
{
    uWord32 enhance_pars[AEC_NDC_PARAM_NUM];
    uWord32 error_flag;       
}SPH_ENH_ctrl_struct;

typedef SPH_ENH_ctrl_struct SPH_PARAM_T;
typedef struct 
{
    Word16 dmnrParm[DMNR_PARAM_NUM];
}DMNR_PARAM_T;

typedef struct
{
    uWord16 enhance_pars[AEC_COM_RX];
}AEC_COM_RX_struct;

typedef struct
{
    uWord16 enhance_pars[AEC_COM_TX];
}AEC_COM_TX_struct;


#define AEC_SAMPLE_PER_FRAME      160

#define IN
#define OUT

#define AEC_RESULT_SUCESS         0
#define AEC_RESULT_FAILED         0x80000000

#if defined(__cplusplus)
extern "C" {
#endif   // __cplusplus

// Get required memory for AEC Process
Word32 ENH_API_Get_Memory( SPH_ENH_ctrl_struct *Sph_Enh_ctrl );
// After memory allocation by main function, re-assigned to AEC Process
Word16 ENH_API_Alloc( SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word32 *mem_ptr);
// Initialize AEC Process
Word16 ENH_API_Init_AEC( SPH_ENH_ctrl_struct *Sph_Enh_ctrl,Word16 *aec_com_rx,Word16 *aec_com_tx);
// Free AEC Process
Word16 ENH_API_Free_AEC( void );
// Initialize ABF Process
Word16 ENH_API_Init_ABF(SPH_ENH_ctrl_struct *Sph_Enh_ctrl_parms,Word16 *ABF_cal_data);
// AEC Up-Link API. ne_sp[160] => near-end speech input/output(final up-link); fe_sp[160] => far-end speech input
void ENH_API_Run_Aec_UL( SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word16 *ne_sp , Word16 *fe_sp, Word16 *ne2_sp);
// AEC Down-link API. fe_sp[160] => far-end speech input/output(final downlink)
void ENH_API_Run_Aec_DL( SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word16 *fe_sp );



Word32 ENH_API_Set_FIR( Word16 *TX, Word16 *RX);


/* Initialize */
extern void NDC_Com_Init ( unsigned long *Pars );
extern void NDC_UL_Init ( void );
extern void NDC_DL_Init ( void );

/* NDC main function */
extern void NDC_UL_MAIN ( Word16 *ne_sp );
extern void NDC_DL_MAIN ( Word16 *fe_sp );

void ENH_API_AGC_Init( Word32 MAX_AGC_GAIN);
void ENH_API_AGC_2(Word16 *input1, Word16 *input2);

void ENH_API_Get_AGC_Gain(Word16 *gain1, Word16 *gain2);    


/* ============================================================================================= */
void ENH_API_PLC(short *pswSource);
void ENH_API_Init_PLC(void);

#if defined(__cplusplus)
}
#endif   // __cplusplus

#endif /* ___BT_LIB_H___ */
