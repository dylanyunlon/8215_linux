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
*[File]			apcm_misc.c
*[Author]		atc6013
*[Description]
*
******************************************************************************/

#include "apcm_misc.h"


#define LOG_TAG  "[Misc]"


//===========================================//


char APCM_TBL_SINE_16BIT[APCM_TBL_SINE_16BIT_SZ] =
{
	0x00, 0x00, 0x22, 0x03, 0x3E, 0x06, 0x4A, 0x09, 0x3E, 0x0C, 0x15, 0x0F, 0xC7, 0x11, 0x4C, 0x14,
	0xA0, 0x16, 0xBC, 0x18, 0x9B, 0x1A, 0x38, 0x1C, 0x90, 0x1D, 0x9F, 0x1E, 0x62, 0x1F, 0xD8, 0x1F,
	0xFF, 0x1F, 0xD8, 0x1F, 0x62, 0x1F, 0x9F, 0x1E, 0x90, 0x1D, 0x38, 0x1C, 0x9B, 0x1A, 0xBC, 0x18,
	0xA0, 0x16, 0x4C, 0x14, 0xC7, 0x11, 0x15, 0x0F, 0x3E, 0x0C, 0x4A, 0x09, 0x3E, 0x06, 0x22, 0x03,
	0x00, 0x00, 0xDD, 0xFC, 0xC2, 0xF9, 0xB6, 0xF6, 0xC1, 0xF3, 0xEB, 0xF0, 0x39, 0xEE, 0xB3, 0xEB,
	0x60, 0xE9, 0x44, 0xE7, 0x65, 0xE5, 0xC8, 0xE3, 0x70, 0xE2, 0x61, 0xE1, 0x9E, 0xE0, 0x28, 0xE0,
	0x00, 0xE0, 0x28, 0xE0, 0x9E, 0xE0, 0x61, 0xE1, 0x70, 0xE2, 0xC8, 0xE3, 0x65, 0xE5, 0x44, 0xE7,
	0x60, 0xE9, 0xB3, 0xEB, 0x39, 0xEE, 0xEB, 0xF0, 0xC1, 0xF3, 0xB6, 0xF6, 0xC2, 0xF9, 0xDD, 0xFC
};


u32 misc_fill_sine16_tbl(apcm_buf_t *buf)
{
	u32 copy_size = 0;
  	u32 free_size = buf_get_free_size(buf);
  	u32 tbl_size = APCM_TBL_SINE_16BIT_SZ;
  	void *sadr = APCM_TBL_SINE_16BIT;

	while (free_size > tbl_size) {
		free_size -= tbl_size;
		copy_size += tbl_size;
		buf_write_data(buf, sadr, sadr, tbl_size);
	}

	return (copy_size);
}


//=====================================================================//

AUDIO_SAMPLING_T misc_get_fs_idx(u32 fs)
{
	AUDIO_SAMPLING_T efs = FS_UNKNOWN;

	switch (fs)
	{
	case 8000:
		efs = FS_8K;
		break;

	case 16000:
		efs = FS_16K;
		break;

	case 22050:
		efs = FS_22K;
		break;

	case 24000:
		efs = FS_24K;
		break;

	case 32000:
		efs = FS_32K;
		break;

	case 44100:
		efs = FS_44K;
		break;

	case 48000:
		efs = FS_48K;
		break;

	case 64000:
		efs = FS_64K;
		break;

	case 88200:
		efs = FS_88K;
		break;

	case 96000:
		efs = FS_96K;
		break;

	case 176400:
		efs = FS_176K;
		break;

	case 192000:
		efs = FS_192K;
		break;

	default:
		efs = FS_48K;
		break;
	}

	return (efs);
}

//=====================================================================//

void misc_irq_enable(u32 vect)
{
	if (!BIM_EnableIrq(vect)) {
		PR_E("[IrqEnable] Failed ! Vect(%d).\r\n", vect);
	}
}


void misc_irq_disable(u32 vect)
{
	if (!BIM_DisableIrq(vect))  {
		PR_E("[IrqDisable] Failed! Vect(%d).\r\n", vect);
	}
}


void misc_irq_clear(u32 vect)
{
	BIM_ClearIrq(vect);
}

//=====================================================================//

bool misc_isr_reg(u32 vect, x_os_isr_fct pfn_isr)
{
	bool ret = true;
	char dev_name[20] = {0};
	snprintf(dev_name, 16, "ISR_Apcm0x%x", vect);

	misc_irq_disable(vect);
	if (OSR_OK != request_irq(vect, pfn_isr, 0, dev_name, NULL)) {
		ret = false;
		PR_E("[IsrReg] Failed! Vect(%d).\r\n", vect);
	}

	if (ret == false) {
		free_irq(vect, NULL);
		if (OSR_OK != request_irq(vect, pfn_isr, 0, dev_name, NULL)) {
			ret = false;
			PR_E("[IsrReg] Failed again! Vect(%d).\r\n", vect);
		} else {
			ret = true;
			PR_D("[IsrReg] reg again to OK. Vect(%d).\r\n", vect);
		}
	}

	return (ret);
}


bool misc_isr_unreg(u32 vect)
{
	bool ret = true;
	misc_irq_disable(vect);
	free_irq(vect, NULL);

	return (ret);
}



