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
*[File]                     log.c
*[Author]                   atc6013
*[Description]
*
******************************************************************************/
#include "apcm_log.h"

#define LOG_TAG         "[Log]"

u32 g_log = 0x3F;

u32 g_file_idx = 0;

u32 g_hibernate = false;


void log_set_level(u32 level)
{
	PR_I("[Set Level] From 0x%x to 0x%x. \n", g_log, level);
	g_log = level;
}


void log_show_version(u32 level)
{
	switch(level)
	{
	case 0:
		PR_I("[Show Version] [%s]:%d.%d_%d \n",
			APCM_MOD_NAME, APCM_VER_MAIN, APCM_VER_MINOR, APCM_VER_REV);
		break;

	default:
		PR_I("[Show Version] [%s]:%d.%d_%d   [%s](%s)\r\n",
			APCM_MOD_NAME, APCM_VER_MAIN, APCM_VER_MINOR, APCM_VER_REV,
			APCM_VER_BRANCH, APCM_VER_DATE);
		break;
	}
}


void log_vir_buf(void *sadr, u32 size)
{
	u32 i = 0;
	u32 *pdata = (u32 *)sadr;

	PR_D("[log_vir_buf] 0x%p, %d \n", sadr, size);
	for (i = 0; i < size; i += 16)
	{
		PR_D("0x%p: 0x%x, 0x%x, 0x%x, 0x%x \n",
			pdata, *pdata, *(pdata + 1), *(pdata + 2), *(pdata + 3));
		pdata += 4;
	}
	PR_D("[log_vir_buf] ============================ \n");
}

