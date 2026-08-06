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
*[File]                     apcm_config.h
*[Author]                   atc6013
*[Description]
*
******************************************************************************/

#ifndef __APCM_CONFIG_H__
#define __APCM_CONFIG_H__

#include "apcm_define.h"

// for customer to define their config

#define MIC_HW_GAIN			40U // 0 ~ 63

#define EXTERNAL_MIC_EN			0   // 1: enable, 0: disable
#define EXTERNAL_MIC_FS			48000
/* External Mic pimmux select
	1: PINMUX_I2SMICIN_GROUP1,	//i2s_in1_d        i2s_in1_bck       i2s_in1_mclk      i2s_in1_lrck
	2: PINMUX_I2SMICIN_GROUP2,	//ain0_r           ain0_l            ain1_r            ain1_l
	3: PINMUX_I2SMICIN_GROUP3,	//ain2_r           ain2_l            ain3_r            ain3_l
	4: PINMUX_I2SMICIN_GROUP4,	//demod_rst        ts_d5             ts_d6             ts_d7
	5: PINMUX_I2SMICIN_GROUP5,	//vb4              vb5               vb6               vb7
	6: PINMUX_I2SMICIN_GROUP6,	//lvds_ao1p        lvds_ao1n         lvds_ao0p         lvds_ao0n
	7: PINMUX_I2SMICIN_GROUP7,	//ts_d1            ts_d2             ts_d3             ts_d4
*/
#define EXTERNAL_MIC_I2S_PIN		1
#define EXTERNAL_MIC_SRC_BIT_NUM	24

#endif // #ifndef __APCM_CONFIG_H__
