/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef BT_LIB_H
#define BT_LIB_H

typedef signed short		Word16;
typedef signed long			Word32;
typedef unsigned short		uWord16;
typedef unsigned long		uWord32;

#define AEC_NDC_PARAM_NUM		48U
#define COMMOM_PARAM_NUM        12U
#define DMNR_PARAM_NUM			96U
#define AEC_COM_RX				22U
#define AEC_COM_TX				22U
#define DMNR_PARAM_NUM_16K		76U
#define COMPEN_FILTER_16K		270U

/* Please Follow this control Structure for AEC API Process control */
typedef struct {
	uWord32 enhance_pars[AEC_NDC_PARAM_NUM];
	uWord32 error_flag;
} SPH_ENH_08K_ctrl_struct;

typedef SPH_ENH_08K_ctrl_struct SPH_PARAM_T;
typedef struct {
	Word16 dmnrParm[DMNR_PARAM_NUM];
} DMNR_PARAM_T;

typedef struct {
	uWord16 enhance_pars[AEC_COM_RX];
} AEC_COM_RX_struct;

typedef struct {
	uWord16 enhance_pars[AEC_COM_TX];
} AEC_COM_TX_struct;


#define AEC_SAMPLE_PER_FRAME	  160U

#define IN
#define OUT

#define AEC_RESULT_SUCESS		  0U
#define AEC_RESULT_FAILED		  0x80000000U

#if defined(__cplusplus)
extern "C" {
#endif

/* Get required memory for AEC Process */
signed long   ENH_08K_API_Get_Memory(SPH_ENH_08K_ctrl_struct *Sph_Enh_ctrl);
/* After memory allocation by main function, re-assigned to AEC Process */
signed short ENH_08K_API_Alloc(SPH_ENH_08K_ctrl_struct *Sph_Enh_ctrl, Word32 *aec_mem_ptr);
/* Initialize AEC Process */
signed short ENH_08K_API_Init_AEC(SPH_ENH_08K_ctrl_struct *Sph_Enh_ctrl, Word16 *aec_com_rx, Word16 *aec_com_tx);
/* Free AEC Process */
signed short ENH_08K_API_Free_AEC(void);
/* Initialize ABF Process */
signed short ENH_08K_API_Init_ABF(SPH_ENH_08K_ctrl_struct *Sph_Enh_ctrl_parms, Word16 *ABF_cal_data);
/* AEC Up-Link API. ne_sp[160] => near-end speech input/output(final up-link); fe_sp[160] => far-end speech input */
void ENH_08K_API_Run_Aec_UL(SPH_ENH_08K_ctrl_struct *Sph_Enh_ctrl, Word16 *ne_sp , Word16 *fe_sp, Word16 *ne2_sp);
/* AEC Down-link API. fe_sp[160] => far-end speech input/output(final downlink) */
void ENH_08K_API_Run_Aec_DL(SPH_ENH_08K_ctrl_struct *Sph_Enh_ctrl, Word16 *fe_sp);

Word32 ENH_08K_API_Set_FIR(Word16 *TX, Word16 *RX);


/* Initialize */
void NDC_08K_Com_Init(unsigned long *Pars);
void NDC_08K_UL_Init(void);
void NDC_08K_DL_Init(void);

/* NDC main function */
void NDC_08K_UL_MAIN(Word16 *ne_sp);
void NDC_08K_DL_MAIN(Word16 *fe_sp);


void ENH_08K_API_AGC_Init(Word32 MAX_AGC_GAIN);
void ENH_08K_API_AGC_1(Word16 *input1);
void ENH_08K_API_AGC_2(Word16 *input1, Word16 *input2);

void ENH_08K_API_Get_AGC_Gain(Word16 *input1, Word16 *input2);
/* Gain values will be write to address of gain1 and gain 2
 Call and dump AGC gain only in debug mode, not necessary for MP*/

/* ================================================ */
void ENH_08K_API_PLC(short *pswSource);
void ENH_08K_API_Init_PLC(void);


/*16k sample rate interface */
#define WB_VOIP		2U

typedef struct {
	uWord32 enhance_pars[AEC_NDC_PARAM_NUM];
	uWord32 common_pars[COMMOM_PARAM_NUM];
	Word32 App_table;
	Word32 Fea_Cfg_table;
	Word32 MIC_DG;
	Word32 sample_rate;
	Word32 frame_rate;
	Word32 MMI_ctrl;
	Word32 RCV_DG;		/* for VoIP, 0xE3D, downlink PGA cost-down */
	Word16 DMNR_cal_data[DMNR_PARAM_NUM_16K];
	Word16 Compen_filter[COMPEN_FILTER_16K];
	Word16 PCM_buffer[1920];
	//Word16 EPL_buffer[4160];
	Word16 EPL_buffer[4800];
	Word32 Device_mode;
	Word32 MMI_MIC_GAIN;
	Word32 Near_end_vad;

	Word32 *SCH_mem; /* caster to (SCH_mem_struct*) in every alloc function  */

#ifdef MTK_Simulator

	Word16 ne_IIR_temp[160];
	Word16 DBG_buffer[960];
	Word16 CommLog_Buffer_last[320];

	/* only used in mtk simulator  */
	char ini_file_name[100];
	char input_filename[100];
	char input_filename2[100]; /* abf */
	char input_dl_filename[100];
	char downlink_filename[100];
	char uplink_filename1[100];
	char uplink_filename2[100];
	char AGC_filename1[100];	/* abf */
	char AGC_filename2[100];	/* abf */

	uWord32  data_length;
	FILE *fp_in;
	FILE *fp_in2;
	FILE *fp_dl_in;
	FILE *fp_dl;
	FILE *fp_out2;
	FILE *fp_out;
	FILE *fp_debug_00;
	FILE *fp_debug_01;
	FILE *fp_debug_02;
	FILE *fp_debug_03;
	FILE *fp_debug_04;
	FILE *fp_debug_05;
	FILE *fp_debug_06;
	FILE *fp_debug_07;
	FILE *fp_debug_08;
	FILE *fp_debug_09;
	FILE *fp_debug_10;
	FILE *fp_debug_11;

	FILE *fp_epl;
	FILE *fp_epl2txt;

	/* 48K EPL */
	FILE *UL1_Pre_48k;
	FILE *UL2_Pre_48k;
	FILE *UL1_Pos_48k;
	FILE *UL2_Pos_48k;

	/*16K EPL */
	FILE *AGC1_Pre;
	FILE *AGC2_Pre;
	FILE *DMNR1_Pre;
	FILE *DMNR2_Pre;
	FILE *UL_NDC_Pre;
	FILE *UL_NDC_Pos;
	FILE *DL_NDC_Pre;
	FILE *DL_VCE_Pre;
	FILE *DL_VCE_Pos;
	FILE *UL2_Pos; /* AEC delay buffer*/
	FILE *AUX1;
	FILE *AUX2;

#endif

} SPH_ENH_ctrl_struct;

Word32	ENH_API_Get_Memory(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
Word16	ENH_API_Alloc(SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word32 *mem_ptr);
Word16	ENH_API_Rst(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
void ENH_API_Process(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
Word16	ENH_API_Free(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);


#if defined(__cplusplus)
}
#endif

#endif
