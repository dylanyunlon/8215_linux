
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
*[File]			apcm_log.h
*[Author]		atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_LOG_H__
#define __APCM_LOG_H__

#include "apcm_config.h"

extern u32 g_log;
extern u32 g_file_idx;
extern u32 g_hibernate;

#define GET_FILE_IDX 		++g_file_idx

#define PR_E(sFmt, ...)		if (g_log & (1 << 0)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);
#define PR_W(sFmt, ...)		if (g_log & (1 << 1)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);
#define PR_I(sFmt, ...)		if (g_log & (1 << 2)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);
#define PR_D(sFmt, ...)		if (g_log & (1 << 3)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);

#define PR_D1(sFmt, ...)	if (g_log & (1 << 4)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);
#define PR_D2(sFmt, ...)	if (g_log & (1 << 5)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);
#define PR_D3(sFmt, ...)	if (g_log & (1 << 6)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);
#define PR_D4(sFmt, ...)	if (g_log & (1 << 7)) pr_info("[APCM]"LOG_TAG sFmt, ##__VA_ARGS__);

//===============================================//

void log_set_level(u32 level);
void log_show_version(u32 level);

void log_vir_buf(void *sadr, u32 size);


#endif // #ifndef __APCM_LOG_H__
