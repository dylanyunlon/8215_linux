/********************************************************************************************
*     LEGAL DISCLAIMER
*
*     (Header of AutoChips Software/Firmware Release or Documentation)
*
*     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AutoChips SOFTWARE") RECEIVED
*     FROM AutoChips AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
*     ONLY. AutoChips EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
*     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
*     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AutoChips PROVIDE ANY WARRANTY
*     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
*     INCORPORATED IN, OR SUPPLIED WITH THE AutoChips SOFTWARE, AND BUYER AGREES TO LOOK
*     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AutoChips SHALL ALSO
*     NOT BE RESPONSIBLE FOR ANY AutoChips SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
*     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AutoChips'S ENTIRE AND CUMULATIVE LIABILITY WITH
*     RESPECT TO THE AutoChips SOFTWARE RELEASED HEREUNDER WILL BE, AT AutoChips'S OPTION,
*     TO REVISE OR REPLACE THE AutoChips SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
*     FEES OR SERVICE CHARGE PAID BY BUYER TO AutoChips FOR SUCH AutoChips SOFTWARE AT ISSUE.
*
*     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
*     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
************************************************************************************************/

/******************************************************************************
*[File]			apcm_ioctrl.cpp
*[Author]		atc6013
*[Description]
*
******************************************************************************/

#include "apcm_ioctrl.h"


#define LOG_TAG		"[ioctrl]"


void ioctrl_cli_srv(u32 type, u32 arg1, u32 arg2, u32 arg3, const s8 **pfilename)
{
	PR_D("[cli_srv] type(0x%x) arg(%d, %d, %d) \n", type, arg1, arg2, arg3);

	switch(type)
	{
	case APCM_IOCTRL_MIX_CH:
		outhw_set_output_ch(arg1, arg2);
		break;

	case APCM_IOCTRL_PRIMARY_MIC:
		mic_set_primary_idx(arg1);
		break;

	case APCM_IOCTRL_SET_MIC_HW_GAIN:
		mic_set_hw_gain(arg1);
		break;

	case APCM_IOCTRL_SET_OUTHW_GAIN:
		arg3 = (arg2 << 16) + arg2;
		outhw_set_gain(arg1, arg3);
		break;

	case APCM_IOCTRL_BT_ENHANCE:
		btcall_ul_enhance_enable(arg1);
		break;

	case APCM_IOCTRL_BT_DUMP_ENABLE:
		txrx_dump_enable(arg1);
		break;

	case APCM_IOCTRL_BT_DBG_FLAG:
		btcall_rx_dbg_enable(arg1);
		break;

	case APCM_IOCTRL_VER:
		log_show_version(arg1);
		break;

	case APCM_IOCTRL_LOG:
		log_set_level(arg1);
		break;

	case APCM_IOCTRL_DUMP_OUT_FILE:
		outhw_dump_enable(arg1);
		break;

	case APCM_IOCTRL_DUMP_MIC_REG:
		mic_dump_regs();
		break;

	case APCM_IOCTRL_DUMP_TXRX_REG:
		txrx_dump_regs();
		break;

	case APCM_IOCTRL_DUMP_ASRC_REG:
		asrc_dump_regs();
		break;

	case APCM_IOCTRL_LOG_BUF:
		if (arg1 && arg2) {
			log_vir_buf(arg1, arg2);
		}
		break;

	case APCM_IOCTRL_UT_COMM:
		utcomm_test(arg1);
		break;

	case APCM_IOCTRL_UT_OUTHW:
		uthw_out_test();
		break;

	case APCM_IOCTRL_UT_MIC:
		uthw_mic_test(arg1);
		break;

	case APCM_IOCTRL_UT_TXRX:
		uthw_txrx_test(arg1);
		break;

	case APCM_IOCTRL_UT_ASRC:
		uthw_asrc_test(arg1, arg2, arg3);
		break;

	case APCM_IOCTRL_UT_MAO:
		uthw_mao_test((bool)arg1);
		break;

	case APCM_IOCTRL_UT_SPH:
		uthw_sph_test();
		break;

	default:
		break;
	}

}


