/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2013 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file AtcDisplaySettings.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
* $Revision: #3 $
* $Modtime:$
* $Log:$
*
*******************************************************************************/
#ifndef _ATCDISPLAYSETTINGS_H_
#define _ATCDISPLAYSETTINGS_H_

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Tcon cmd */
#define DISPLAY_SET_LVDS_SSC                    (0x00020001)
#define DISPLAY_GET_DITHER                      (0x00020002)
#define DISPLAY_SET_DITHER_DISABLE              (0x00020003)
#define DISPLAY_SET_DITHER                      (0x00020004)
#define DISPLAY_SET_CONTRAST                   	(0x00020005)
#define DISPLAY_SET_BRIGNTNESS               	(0x00020006)
#define DISPLAY_SET_SATURATION                	(0x00020007)
#define DISPLAY_SET_BKL_INTENSITY           	(0x00020008)
#define DISPLAY_SET_HUE                         (0x00020009)
#define DISPLAY_SET_YGAIN				       	(0x0002001A)
#define DISPLAY_SET_UGAIN				       	(0x0002001B)
#define DISPLAY_SET_VGAIN				       	(0x0002001C)
#define DISPLAY_SET_GAMMA				       	(0x0002001D)

#define DISPLAY_GET_CONTRAST                   	(0x0002001E)
#define DISPLAY_GET_BRIGNTNESS               	(0x0002001F)
#define DISPLAY_GET_SATURATION                	(0x00020020)
#define DISPLAY_GET_BKL_INTENSITY           	(0x00020021)
#define DISPLAY_GET_HUE                         (0x00020022)
#define DISPLAY_GET_YGAIN				       	(0x00020023)
#define DISPLAY_GET_UGAIN				       	(0x00020024)
#define DISPLAY_GET_VGAIN				       	(0x00020025)
#define DISPLAY_GET_GAMMA				       	(0x00020026)
#define DISPLAY_SET_BKL_SHUTDOWN				(0x00020027)
#define DISPLAY_GET_ROTATE                      (0x00020030)
/* vcp cmd */

/**
**  description-->set lvds ssc.
**  input
**  'dir' ssc direction: 0:center, 1: down, 2: up
**  'freq' ssc freqency(the unit is Hz, 15~30KHZ)
**  'range' ssc range(10~500 means 0.1%~5%)
**  return-->0 for success, -1 for failed.
**/
int SetLvdsSsc(unsigned int dir, unsigned int freq, unsigned int range);

int GetDitherLevel(void);

int SetDitherDisable(void);

int SetDitherLevel(int level);


/****************************************
**Display settings' API begin
****************************************/

/**
**  description-->set brightness level.
**  input-->'level' brightness level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetBrightnessLevel(int level);

/**
**  description-->set contrast level.
**  input-->'level' contrast level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetContrastLevel(int level);

/**
**  description-->set backlight level.
**  input-->'level' backlight level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetBackLightLevel(int level);

/**
**  description-->set bkl module to be shut down.
**  input-->'fgShutdown' 1:shut down, other value:nothing to be done.
**  return-->0 for success, -1 for failed.
**/
int SetBklShutDown(int fgShutdown);

/**
**  description-->set HUE level.
**  input-->'level' HUE level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetHueLevel(int level);

/**
**  description-->set saturation level.
**  input-->'level' saturation level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 100.
**/
int SetSaturationLevel(int level);

/**
**  description-->set ygain level.
**  input-->'level' ygain level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 0x1ff.
**/
int SetYGainLevel(int level);

/**
**  description-->set ugain level.
**  input-->'level' ugain level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 0x1ff.
**/
int SetUGainLevel(int level);

/**
**  description-->set vgain level.
**  input-->'level' vgain level to be set.
**  return-->0 for success, -1 for failed.
**  note-->'level' set to between 0 and 0x1ff.
**/
int SetVGainLevel(int level);

/**
**  description-->set gamma table.
**  input-->'GamTbl' gamma table to be set.
**  return-->0 for success, -1 for failed.
**  note-->'GamTbl' array address,the size of this array is 64.
**/
int SetGammaTblLevel(unsigned char* GamTbl);

/**
**  description-->get brightness level.
**  input-->none.
**  return-->brightness level.
**  note-->none.
**/
int GetBrightnessLevel();

/**
**  description-->get contrast level.
**  input-->none.
**  return-->contrast level.
**  note-->none.
**/
int GetContrastLevel();

/**
**  description-->get backlight level.
**  input-->none.
**  return-->backlight level.
**  note-->none.
**/
int GetBackLightLevel();

/**
**  description-->get HUE level.
**  input-->none.
**  return-->HUE level.
**  note-->none.
**/
int GetHueLevel();

/**
**  description-->get saturation level.
**  input-->none.
**  return-->saturation level.
**  note-->none.
**/
int GetSaturationLevel();

/**
**  description-->get ygain level.
**  input-->none.
**  return-->ygain level.
**  note-->none.
**/
int GetYGainLevel();

/**
**  description-->get ugain level.
**  input-->none.
**  return-->ugain level.
**  note-->none.
**/
int GetUGainLevel();

/**
**  description-->get vgain level.
**  input-->none.
**  return-->vgain level.
**  note-->none.
**/
int GetVGainLevel();

/**
**  description-->get gamma table.
**  input-->'GamTbl' gamma table to be get.
**  return-->0 for success, -1 for failed.
**  note-->'GamTbl' array address,the size of this array is 64.
**/
int GetGammaTblLevel(unsigned char* GamTbl);

/**
**  description-->get rotate value.
**  input-->'value' rotate value to be get.
**  return-->0 for success, -1 for failed.
**  note-->'value': 0:0, 1:90, 2:180, 3:270.
**/
int GetRotateValue(unsigned char *value);


/****************************************
**Display settings' API end
****************************************/

/****************************************
**color processing settings' API begin
****************************************/
/**
**  description-->set cp on.
**  input-->none.
**  return-->0 for success, -1 for failed.
**  note-->none.
**/ /*
int SetVcpOn();
*/
/**
**  description-->set cp off.
**  input-->none.
**  return-->0 for success, -1 for failed.
**  note-->none.
**/ /*
int SetVcpOff();
*/
/**
**  description-->set vcp hue level.
**  input-->'hue_t' hue struct to be set.
**  return-->0 for success, -1 for failed.
**  note-->'hue_t.i4hue' set to between 0 and 0x3f.
**/ /*
int SetVcpHUELevel(vcp_hue_paras hue_t);
*/
/**
**  description-->set vcp yuv level.
**  input-->'yuvgain_t' yuv struct to be set.
**  return-->0 for success, -1 for failed.
**  note-->'yuvgain_t.i4YGain(yuvgain_t.i4UGain,yuvgain_t.i4VGain)' set to between 0 and 0x1ff.
**/ /*
int SetVcpYUVGainLevel(vcp_yuv_paras yuvgain_t);
*/
/**
**  description-->set vcp contrast/brightness/saturation level.
**  input-->'cbs_t' cbs struct to be set.
**  return-->0 for success, -1 for failed.
**  note-->'cbs_t.i4Contr(cbs_t.i4Brit,cbs_t.i4Satr)' set to between 0 and 0xff.
**/ /*
int SetVcpContrBritSatrLevel(vcp_cbs_paras cbs_t);
*/
/**
**  description-->get vcp hue level.
**  input-->none.
**  return-->return value is hue level.
**  note-->none.
**/ /*
int GetVcpHUELevel();
*/
/**
**  description-->get vcp hue level.
**  input-->'hue_t' will get the register value.
**  return-->0 for success, -1 for failed.
**  note-->.none
**/ /*
int GetVcpYUVGainLevel(vcp_yuv_paras *yuvgain_t);
*/
/**
**  description-->get vcp hue level.
**  input-->'cbs_t' will get the register value.
**  return-->0 for success, -1 for failed.
**  note-->.none
**/ /*
int GetVcpContrBritSatrLevel(vcp_cbs_paras *cbs_t);  */
/****************************************
**color processing settings' API end
****************************************/

/****************************************
**other module settings' API begin
****************************************/


/****************************************
**other module settings' API end
****************************************/

#ifdef __cplusplus
}
#endif

#endif
