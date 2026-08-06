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

#ifndef __DRV_IR_H__
#define __DRV_IR_H__


#define IR_FAIL             (-1)
#define IR_SUCC             (0)

/* #include "u_irrc_btn_def.h" */
#include "drv_config.h"
#include "chip_ver.h"


#if 1 /* DRV_SUPPORT_MHL_RX */
typedef int (*IRRX_SNED_MHL_KEY)(UINT32 keycode);
extern IRRX_SNED_MHL_KEY   p_irrx_mhl_fun;
#endif

/******************************************************************************/
#ifndef KEY_GROUP_DIGIT

#define KEY_GROUP_DIGIT         ((UINT32) 0x00010000)
#define KEY_GROUP_ALPHA         ((UINT32) 0x00020000)
#define KEY_GROUP_CURSOR        ((UINT32) 0x00030000)
#define KEY_GROUP_SEL_CTRL      ((UINT32) 0x00040000)
#define KEY_GROUP_PRG_CTRL      ((UINT32) 0x00050000)
#define KEY_GROUP_AUD_CTRL      ((UINT32) 0x00060000)
#define KEY_GROUP_TTX_CTRL      ((UINT32) 0x00070000)
#define KEY_GROUP_FCT_CTRL      ((UINT32) 0x00080000)
#define KEY_GROUP_STRM_CTRL     ((UINT32) 0x00090000)
#define KEY_GROUP_DVD_CTRL      ((UINT32) 0x000a0000)
#define KEY_GROUP_USER_DEF      ((UINT32) 0x000b0000)
#define KEY_GROUP_MAX           ((UINT32) 0x000c0000)

#define BTN_DIGIT_0             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '0')))
#define BTN_DIGIT_1             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '1')))
#define BTN_DIGIT_2             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '2')))
#define BTN_DIGIT_3             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '3')))
#define BTN_DIGIT_4             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '4')))
#define BTN_DIGIT_5             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '5')))
#define BTN_DIGIT_6             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '6')))
#define BTN_DIGIT_7             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '7')))
#define BTN_DIGIT_8             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '8')))
#define BTN_DIGIT_9             ((UINT32)(KEY_GROUP_DIGIT | ((UINT32) '9')))
#define BTN_DIGIT_PLUS_5        ((UINT32)(KEY_GROUP_DIGIT | 0x0000f000))
#define BTN_DIGIT_PLUS_10       ((UINT32)(KEY_GROUP_DIGIT | 0x0000f001))
#define BTN_DIGIT_PLUS_20       ((UINT32)(KEY_GROUP_DIGIT | 0x0000f002))
#define BTN_DIGIT_PLUS_100      ((UINT32)(KEY_GROUP_DIGIT | 0x0000f003))
#define BTN_DIGIT_DOT           ((UINT32)(KEY_GROUP_DIGIT | 0x0000f004))
#if CONFIG_DRV_CUSTOM_KLG
#define BTN_DASH                ((UINT32)(KEY_GROUP_DIGIT | 0x0000f005))
#endif

#define BTN_CURSOR_LEFT         ((UINT32)(KEY_GROUP_CURSOR | 0x0000f000))
#define BTN_CURSOR_RIGHT        ((UINT32)(KEY_GROUP_CURSOR | 0x0000f001))
#define BTN_CURSOR_UP           ((UINT32)(KEY_GROUP_CURSOR | 0x0000f002))
#define BTN_CURSOR_DOWN         ((UINT32)(KEY_GROUP_CURSOR | 0x0000f003))

#define BTN_SELECT              ((UINT32)(KEY_GROUP_SEL_CTRL | 0x0000f000))
#define BTN_EXIT                ((UINT32)(KEY_GROUP_SEL_CTRL | 0x0000f001))
#define BTN_CLEAR               ((UINT32)(KEY_GROUP_SEL_CTRL | 0x0000f002))

#define BTN_PRG_UP              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f000))
#define BTN_PRG_DOWN            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f001))
#define BTN_PREV_PRG            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f002))
#define BTN_CH_LIST             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f003))
#define BTN_FAV_CH              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f004))
#define BTN_FAV_CH_UP           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f005))
#define BTN_FAV_CH_DOWN         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f006))
#define BTN_FAVORITE            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f007))
#define BTN_PIP                 ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f008))
#define BTN_PIP_CH_UP           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f009))
#define BTN_PIP_CH_DOWN         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f00a))
#define BTN_PIP_SIZE            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f00b))
#define BTN_PIP_POS             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f00c))
#define BTN_PIP_INPUT_SRC       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f00d))
#define BTN_POP                 ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f00e))
#define BTN_PIP_POP             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f00f))
#define BTN_SWAP                ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f010))
#define BTN_FREEZE              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f011))
#define BTN_ZOOM                ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f012))
#define BTN_ZOOM_IN             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f013))
#define BTN_ZOOM_OUT            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f014))
#define BTN_ASPECT              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f015))
#define BTN_P_EFFECT            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f016))
#define BTN_PRG_INFO            ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f017))
#define BTN_PRG_DETAIL          ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f018))
#define BTN_CC                  ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f019))
#define BTN_ADD_ERASE           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f01a))
#define BTN_SLEEP               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f01b))
#define BTN_TIMER               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f01c))
#define BTN_AUTO_SYNC           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f01d))
#define BTN_INPUT_SRC           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f01e))
#define BTN_TV                  ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f01f))
#define BTN_TV_ANA              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f020))
#define BTN_TV_DIG              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f021))
#define BTN_TUNER_TER_ANA       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f022))
#define BTN_TUNER_CAB_ANA       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f023))
#define BTN_TUNER_SAT_ANA       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f024))
#define BTN_TUNER_TER_DIG       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f025))
#define BTN_TUNER_CAB_DIG       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f026))
#define BTN_TUNER_SAT_DIG       ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f027))
#define BTN_COMPOSITE           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f028))
#define BTN_COMPOSITE_1         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f029))
#define BTN_COMPOSITE_2         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f02a))
#define BTN_COMPOSITE_3         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f02b))
#define BTN_COMPOSITE_4         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f02c))
#define BTN_S_VIDEO             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f02d))
#define BTN_S_VIDEO_1           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f02e))
#define BTN_S_VIDEO_2           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f02f))
#define BTN_S_VIDEO_3           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f030))
#define BTN_S_VIDEO_4           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f031))
#define BTN_COMPONENT           ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f032))
#define BTN_COMPONENT_1         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f033))
#define BTN_COMPONENT_2         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f034))
#define BTN_COMPONENT_3         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f035))
#define BTN_COMPONENT_4         ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f036))
#define BTN_VGA                 ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f037))
#define BTN_DVI                 ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f038))
#define BTN_DVI_1               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f039))
#define BTN_DVI_2               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f03a))
#define BTN_DVI_3               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f03b))
#define BTN_DVI_4               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f03c))
#define BTN_HDMI                ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f03d))
#define BTN_HDMI_1              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f03e))
#define BTN_HDMI_2              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f03f))
#define BTN_HDMI_3              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f040))
#define BTN_HDMI_4              ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f041))
#define BTN_SCART               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f042))
#define BTN_SCART_1             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f043))
#define BTN_SCART_2             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f044))
#define BTN_SCART_3             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f045))
#define BTN_SCART_4             ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f046))
#define BTN_DVD                 ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f047))
#define BTN_ILINK               ((UINT32)(KEY_GROUP_PRG_CTRL | 0x0000f048))

#define BTN_VOL_UP              ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f000))
#define BTN_VOL_DOWN            ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f001))
#define BTN_MUTE                ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f002))
#define BTN_MTS                 ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f003))
#define BTN_AUDIO               ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f004))
#define BTN_MTS_AUDIO           ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f005))
#define BTN_PIP_AUDIO           ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f006))
#define BTN_S_EFFECT            ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f007))
#define BTN_SPK_LEV             ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f008))
#define BTN_OPTICAL             ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f009))
#define BTN_MIC_UP              ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f00a))
#define BTN_MIC_DOWN            ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f00b))
#define BTN_FP_VOL_UP           ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f00c))
#define BTN_FP_VOL_DOWN         ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f00d))
#define BTN_OPTICAL_CEC         ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f00e))
#define BTN_AUX_CEC             ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f00f))
#define BTN_AM                  ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f010))
#define BTN_FM                  ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f011))
#define BTN_AUDIO_SYNC          ((UINT32)(KEY_GROUP_AUD_CTRL | 0x0000f012))


#define BTN_TTX                 ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f000))
#define BTN_INDEX               ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f001))
#define BTN_REVEAL              ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f002))
#define BTN_MIX                 ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f003))
#define BTN_SUBPAGE             ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f004))
#define BTN_SIZE                ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f005))
#define BTN_WIDE_MODE           ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f006))
#if CONFIG_DRV_CUSTOM_KLG
#define BTN_FLASHBACK           ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f007))
#define BTN_CAPTION             ((UINT32)(KEY_GROUP_TTX_CTRL | 0x0000f008))
#endif

#define BTN_POWER               ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f000))
#define BTN_MENU                ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f001))
#define BTN_EPG                 ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f002))
#define BTN_MEM_CARD            ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f003))
#define BTN_TEXT                ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f004))
#define BTN_POWER_ON            ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f005))
#define BTN_POWER_OFF           ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f006))
#define BTN_MODE                ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f009))
#if CONFIG_DRV_CUSTOM_JDZ
#define BTN_CONNECTED           ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f007))
#define BTN_NETFLIX             ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f008))
#define BTN_SETTING         ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f00a))   /*TSB 2013 */
#define BTN_EXIT_TO_HOME    ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f00b))   /*TSB 2013 */
#endif
#if (CONFIG_DRV_DENON_SUPPORT && CONFIG_DRV_CUSTOM_CDN)
#define BTN_YOUTOBE               ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f00b))
#define BTN_NETFLIX                ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f00c))
#define BTN_VUDU                    ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f00d))
/* #define BTN_HULU                    ((UINT32)(KEY_GROUP_FCT_CTRL | 0x0000f00a)) */
#endif

#define BTN_PLAY                ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f000))
#define BTN_PAUSE               ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f001))
#define BTN_STOP                ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f002))
#define BTN_RESUME              ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f003))
#define BTN_RECORD              ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f004))
#define BTN_NEXT                ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f005))
#define BTN_PREV                ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f006))
#define BTN_FF                  ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f007))
#define BTN_FR                  ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f008))
#define BTN_SF                  ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f009))
#define BTN_SR                  ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f00a))
#define BTN_STEPF               ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f00b))
#define BTN_STEPR               ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f00c))
#define BTN_PLAY_PAUSE          ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f00d))
#define BTN_PAUSE_STEP          ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f00e))
#define BTN_STOP_RESUME         ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f00f))
#define BTN_PLAY_ENTER          ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f010))
#define BTN_FP_NEXT             ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f011))
#define BTN_FP_PREV             ((UINT32)(KEY_GROUP_STRM_CTRL | 0x0000f012))



#define BTN_EJECT               ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f000))
#define BTN_TITLE_MENU          ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f001))
#define BTN_ROOT_MENU           ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f002))
#define BTN_GOTO                ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f003))
#define BTN_MARK                ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f004))
#define BTN_DIGEST              ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f005))
#define BTN_PROGRAM             ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f006))
#define BTN_VRMT                ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f007))
#define BTN_PBC                 ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f008))
#define BTN_REPEAT              ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f009))
#define BTN_REPEAT_A            ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f00a))
#define BTN_REPEAT_B            ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f00b))
#define BTN_REPEAT_A_B          ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f00c))
#define BTN_SUB_TITLE           ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f00d))
#define BTN_ANGLE               ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f00e))
#define BTN_DISPLAY             ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f00f))
#define BTN_RANDOM              ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f010))
#define BTN_PAL_NTSC            ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f011))
#define BTN_SURROUND            ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f012))
#define BTN_EQUALIZER           ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f013))
#define BTN_HOME_DLIST          ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f014))
#define BTN_NEXT_DLIST          ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f015))
#define BTN_PREV_DLIST          ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f016))
#define BTN_KARAOKE             ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f017))
#define BTN_AUD_KEY_PLUS        ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f018))
#define BTN_AUD_KEY_MINUS       ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f019))
#define BTN_AUD_KEY_RESUME      ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f01a))
#define BTN_ECHO_PLUS           ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f01b))
#define BTN_ECHO_MINUS          ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f01c))
#define BTN_VOCAL_ASSIST        ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f01d))
#define BTN_MENU_PBC            ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f01e))
#define BTN_TITLE_PBC           ((UINT32)(KEY_GROUP_DVD_CTRL | 0x0000f01f))

#define BTN_RED                 ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f000))
#define BTN_GREEN               ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f001))
#define BTN_YELLOW              ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f002))
#define BTN_BLUE                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f003))
#define BTN_FACTORY_MODE_1      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f004))
#define BTN_FACTORY_MODE_2      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f005))
#define BTN_FACTORY_MODE_3      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f006))
#define BTN_FACTORY_MODE_4      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f007))
#define BTN_FACTORY_MODE_5      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f008))
#define BTN_FACTORY_MODE_6      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f009))
#define BTN_FACTORY_MODE_7      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f00a))
#define BTN_FACTORY_MODE_8      ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f00b))
#define BTN_FUNCTION_1          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f00c))
#define BTN_FUNCTION_2          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f00d))
#define BTN_FUNCTION_3          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f00e))
#define BTN_FUNCTION_4          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f00f))
#define BTN_FUNCTION_5          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f010))
#define BTN_FUNCTION_6          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f011))
#define BTN_FUNCTION_7          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f012))
#define BTN_FUNCTION_8          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f013))
#define BTN_FUNCTION_9          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f014))
#define BTN_FUNCTION_10         ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f015))
#define BTN_FUNCTION_11         ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f016))
#define BTN_FUNCTION_12         ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f017))
#define BTN_CUSTOM_1            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f018))
#define BTN_CUSTOM_2            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f019))
#define BTN_CUSTOM_3            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f01a))
#define BTN_CUSTOM_4            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f01b))
#define BTN_CUSTOM_5            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f01c))
#define BTN_CUSTOM_6            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f01d))
#define BTN_CUSTOM_7            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f01e))
#define BTN_CUSTOM_8            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f01f))
#define BTN_CUSTOM_9            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f020))
#define BTN_CUSTOM_10           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f021))
#define BTN_CUSTOM_11           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f022))
#define BTN_CUSTOM_12           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f023))
#if CONFIG_SUPPORT_SER_MODE /* yuancof */
#define BTN_SERMODE_EJECT        ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f024))
#endif
#define BTN_FP_PLAY              ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f025))

#define BTN_MP1                  ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f026))
#define BTN_MP2                  ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f027))
#if CONFIG_SUPPORT_BBK_DRIVER

#define BTN_VR_SELECT                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f028))
#define BTN_VR_VD                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f029))
#define BTN_VR_CN                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02a))
#define BTN_VR_FF                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02b))
#define BTN_VR_YT                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02c))
#define BTN_VR_RS                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02d))
#define BTN_VR_PD                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02e))
#define BTN_VR_PA                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02f))
#define BTN_VR_NF                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f030))
#endif

#if (CONFIG_DRV_DENON_SUPPORT && CONFIG_DRV_CUSTOM_CDN)/* yuancof */
#define BTN_SERMODE_EJECT        ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f024))
#define BTN_Heat_Run             ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f025))
#define BTN_Lock_Tray            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f026))
#define BTN_Key_Input            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f027))
#define BTN_VFD_ALL_ON           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f028))
#define BTN_Test_Mode            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f029))
#define BTN_Lock_Romete          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02a))
#define BTN_Initial              ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02b))
/* #define BTN_PURE_DIRECT        BTN_FUNCTION_1 */
#define BTN_DISC_LAYER           BTN_CUSTOM_4
#define BTN_opu_input            ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02c))
#define BTN_Laser_clear          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02d))
#define BTN_PURE_DIRECT          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02e))
#define BTN_HEAT_RUN_COUNT       ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f02f))
#define  BTN_VFD_SWITCH          ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f030))
/* #if CONFIG_APP_RS232C     //20101109 kato */
#define BTN_CUSTOM_13           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f031))
#define BTN_CUSTOM_14           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f032))
#define BTN_CUSTOM_15           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f033))
#define BTN_CUSTOM_16           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f034))
#define BTN_CUSTOM_17           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f035))
#define BTN_CUSTOM_18           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f036))
#define BTN_CUSTOM_19           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f037))
#define BTN_CUSTOM_20           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f038))
#define BTN_CUSTOM_21           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f039))
#define BTN_CUSTOM_22           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f03a))
#define BTN_CUSTOM_23           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f03b))
#define BTN_CUSTOM_24           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f03c))
#define BTN_CUSTOM_25           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f03d))
#define BTN_CUSTOM_26           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f03e))
#define BTN_CUSTOM_27           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f03f))
#define BTN_CUSTOM_28           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f040))
#define BTN_CUSTOM_29           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f041))
#define BTN_CUSTOM_30           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f042))
#define BTN_CUSTOM_31           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f043))
#define BTN_CUSTOM_32           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f044))
#define BTN_CUSTOM_33           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f045))
#define BTN_CUSTOM_34           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f046))
#define BTN_CUSTOM_35           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f047))
#define BTN_CUSTOM_36           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f048))
#define BTN_CUSTOM_37           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f049))
#define BTN_CUSTOM_38           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f04A))
#define BTN_CUSTOM_39           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f04B))
#define BTN_CUSTOM_40           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f04C))
#define BTN_CUSTOM_41           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f04D))
#define BTN_CUSTOM_42           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f04E))
#define BTN_CUSTOM_43           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f04F))
#define BTN_CUSTOM_44           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f050))
#define BTN_CUSTOM_45           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f051))
#define BTN_CUSTOM_46           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f052))
#define BTN_CUSTOM_47           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f053))
#define BTN_CUSTOM_48           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f054))
#define BTN_CUSTOM_49           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f055))
#define BTN_CUSTOM_50           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f056))
#define BTN_CUSTOM_51           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f057))
#if 1/*CTK_ADD_SYSTEM_KEY */            /*ctk jack 20111220 define system key,
if system key will change, modefy follw key value */
#define BTN_YOUTOBE               ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f058))
#define BTN_NETFLIX                ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f059))
#define BTN_VUDU                    ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f05a))
#endif
#if CONFIG_DRV_CUSTOM_KLG && CONFIG_DRV_SUPPORT_MR2014
#define BTN_WIFI_READY           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f05b))
#define BTN_WIFI_DISABLE         ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f05c))
#define BTN_WIFI_SETUP           ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f05d))
#define BTN_WPS                  ((UINT32)(KEY_GROUP_USER_DEF | 0x0000f05e))

#endif
#define BTN_FIRM_VERSION                BTN_CUSTOM_1
/* #define BTN_DRMMER                      BTN_CUSTOM_2
//#define BTN_POWER                       BTN_CUSTOM_3 */
#define BTN_DISC_LAYER                  BTN_CUSTOM_4
/* #define BTN_MODE                        BTN_CUSTOM_5
#define BTN_LAYER_MEDIA               BTN_CUSTOM_6 */
#define BTN_SYSTEM_STATUS               BTN_CUSTOM_7
/*#define BTN_PAGE_PLUS                 BTN_CUSTOM_8
#define BTN_PAGE_MINUS                BTN_CUSTOM_9
#define BTN_SOURCE                      BTN_CUSTOM_10
#define BTN_PIC_ADJUST                BTN_CUSTOM_11
#define BTN_MENORY                      BTN_CUSTOM_12 */
#define BTN_UPDATE_START                BTN_CUSTOM_13
#define BTN_UPDATE_STATUS               BTN_CUSTOM_14
#define BTN_PIP_SUB_TITLE               BTN_CUSTOM_15
#define BTN_SUB_TITLE_STYLE       BTN_CUSTOM_16
#define BTN_BD_AUDIO_HD                 BTN_CUSTOM_17
#define BTN_BD_AUDIO_MIX                BTN_CUSTOM_18
#define BTN_AV_SYNC_HDMI                BTN_CUSTOM_19
#define BTN_AV_SYNC_ANALOG          BTN_CUSTOM_20
#define BTN_PIP_OFF                       BTN_CUSTOM_21
#define BTN_ASPECT_RATIO_16_9_NORMAL      BTN_CUSTOM_22
#define BTN_ASPECT_RATIO_16_9_FULL        BTN_CUSTOM_23
#define BTN_ASPECT_RATIO_4_3_PS             BTN_CUSTOM_24
#define BTN_ASPECT_RATIO_4_3_LB             BTN_CUSTOM_25
#define BTN_PROGRESSIVE_AUTO                  BTN_CUSTOM_26
#define BTN_PROGRESSIVE_VIDEO                 BTN_CUSTOM_27
#define BTN_PROGRESSIVE_FILM                  BTN_CUSTOM_28
#define BTN_ANALOG_AUDIO_OUT_7_1            BTN_CUSTOM_29
#define BTN_ANALOG_AUDIO_OUT_5_1            BTN_CUSTOM_30
#define BTN_ANALOG_AUDIO_OUT_2_0            BTN_CUSTOM_31
#define BTN_ANGLE_PLUS                          BTN_CUSTOM_32
#define BTN_AUDIO_RS232C                        BTN_CUSTOM_33
#define BTN_SEC_AUDIO_RS232C                  BTN_CUSTOM_34
#define BTN_SUBTITLE_RS232C                   BTN_CUSTOM_35
#define BTN_AUTOTRANSFER_RS232C             BTN_CUSTOM_36
#define BTN_ONETRANSFER_RS232C              BTN_CUSTOM_37
#endif

#define BTN_INVALID             ((UINT32)(KEY_GROUP_MAX | 0x0000f000))


#endif /* KEY_GROUP_DIGITS */

/***************************************************************************/
#define BTN_INPUT               BTN_INPUT_SRC
#define BTN_PICSIZE             BTN_SWAP
#define BTN_OSD                 BTN_PRG_INFO
#define BTN_FVRTCHNL            BTN_FAV_CH
#define BTN_CLOCK               BTN_EPG
#define BTN_UPDATE              BTN_MEM_CARD
#define BTN_CE                  BTN_EXIT


/***************************************************************************/
#define KEY_GROUP_NO_DEF        ((UINT32) 0xffff0000)
#define BTN_ACTCTRL             ((UINT32) (KEY_GROUP_NO_DEF | 0x00000002))
#define BTN_SMARTPIC            ((UINT32) (KEY_GROUP_NO_DEF | 0x00000003))
#define BTN_TTTV                ((UINT32) (KEY_GROUP_NO_DEF | 0x00000004))
#define BTN_PIPPOS              ((UINT32) (KEY_GROUP_NO_DEF | 0x00000009))
#define BTN_COLORSYS            ((UINT32) (KEY_GROUP_NO_DEF | 0x0000000a))
#define BTN_CAPTURE             ((UINT32) (KEY_GROUP_NO_DEF | 0x0000000d))
/* #define BTN_SLEEP               ((UINT32) (KEY_GROUP_NO_DEF | 0x00000010)) */
/* #define BTN_INDEX               ((UINT32) (KEY_GROUP_NO_DEF | 0x00000014)) */
/* #define BTN_TIMER               ((UINT32) (KEY_GROUP_NO_DEF | 0x00000016)) */
/* #define BTN_FREEZE              ((UINT32) (KEY_GROUP_NO_DEF | 0x00000019)) */
#define BTN_VCHIP               ((UINT32) (KEY_GROUP_NO_DEF | 0x0000001a))
#define BTN_SNDEFCT             ((UINT32) (KEY_GROUP_NO_DEF | 0x0000001c))


/***************************************************************************/
#define BTN_NONE                ((UINT32) 0xffffffff)
#define BTN_NO_DEF              ((UINT32) 0xfffffffe)
/* modify by msz00420 07-09-10 for resolving BTN_REPEAT multi-defined in */
/* drv_ir.h and u_irrc_btn_def.h */
/*last code
#define BTN_REPEAT              ((UINT32) 0xfffffffd)
*/
#define BTN_KEY_REPEAT              ((UINT32) 0xfffffffd)
/* modify end */

#if !CONFIG_SUPPORT_LIRC
/******************************************************************************
* u4Info:
*   Bit 31~24 is the value of the sampling counter in the 3rd pulse.
*   Bit 23~16 is the value of the sampling counter in the 2nd pulse.
*   Bit 15~08 is the value of the sampling counter in the 1st pulse.
*   Bit 05~00 is the bit count of this IR command.
******************************************************************************/
#define INFO_TO_BITCNT(u4Info)      ((u4Info & IRRX_CH_BITCNT_MASK)    >> IRRX_CH_BITCNT_BITSFT)
#define INFO_TO_1STPULSE(u4Info)    ((u4Info & IRRX_CH_1ST_PULSE_MASK) >> IRRX_CH_1ST_PULSE_BITSFT)
#define INFO_TO_2NDPULSE(u4Info)    ((u4Info & IRRX_CH_2ND_PULSE_MASK) >> IRRX_CH_2ND_PULSE_BITSFT)
#define INFO_TO_3RDPULSE(u4Info)    ((u4Info & IRRX_CH_3RD_PULSE_MASK) >> IRRX_CH_3RD_PULSE_BITSFT)
typedef void (* PFN_IRRXCB_T)(UINT32 u4Info, const UINT8 *pu1Data);

#ifndef FASTLOGO_IR_SUPPORT
/******************************************************************************/
/* Power down mode functions. */
extern INT32 IRHW_Down(const INT32 *pi4Data);
extern INT32 IRHW_PKey(const INT32 *pi4Data);
extern UINT32 IRHW_PowerBitNum(const UINT32 *pu4Data);
extern UINT32 IRHW_PowerUpKey1(const UINT32 *pu4Data);
extern UINT32 IRHW_PowerUpKey2(const UINT32 *pu4Data);

/* IR IF functions. */
extern INT32 IR_Diag(void);

/* IRRX IF functions. An easy interface for mw_irrx.c */
extern INT32 IRRX_InitMtkIr(void);
extern INT32 IRRX_StopMtkIr(void);
extern INT32 IRRX_ResetMtkIr(void);
extern INT32 IRRX_SendMtkIr(UINT32 u4Key);
extern INT32 IRRX_PollMtkIr(UINT32 *pu4Key);

extern VOID  IRRX_SetVfdLockTime(UINT32 ui4_time);
extern BOOL  IRRX_VfdKeyStat(BOOL fg_keysend, BOOL fg_set);




extern INT32 IRRX_SendMtkIr_Ex(UINT32 u4Key, BOOL fgIsFpkey);
extern INT32 IRRX_PollMtkIr_Ex(UINT32 *pu4Key, UINT32 *pu4FpData);

#if CONFIG_SUPPORT_SS
extern INT32 IRRX_PollMtkIr_Micom(UINT8 *pData, UINT32 *pu4Key, UINT32 *pu4KeyStatus);
#else
extern INT32 IRRX_PollMtkIr_Micom(UINT32 *pu4Key, UINT32 *pu4FpData);
#endif
extern VOID  IRRX_Lock_Eject_Key(BOOL b_lock);

extern INT32 IRRX_QuerySetRepeat(const INT32 *pi4Data);
extern UINT32 IRRX_QuerySetGroupId(const UINT32 *pu4Data);
extern UINT32 IRRX_QuerySet1stPulse(const UINT32 *pu4Data);
extern UINT32 IRRX_QuerySet2ndPulse(const UINT32 *pu4Data);
extern UINT32 IRRX_QuerySet3rdPulse(const UINT32 *pu4Data);
extern UINT32 IRRX_QuerySetRepeatTime(const UINT32 *pu4Data);
extern void IRRX_SetMiscInit(BOOL fgMiscInit);
extern void IRRX_SetEnable(BOOL fgEnable);
#if CONFIG_SUPPORT_DIVERSITY
#if 1/*SUPPORT_2K12_IR_CHILD_LOCK */
extern void IRRX_SetEnable_ChildLock(BOOL fgEnable);
extern BOOL IRRX_GetEnable_ChildLock(void);
#endif
#endif

/* HWIR RX external functions declare. */
extern void IRHW_RxRdConf(INT32 *pi4Config, INT32 *pi4SaPeriod, INT32 *pi4Threshold);
extern void IRHW_RxWrConf(INT32 i4Config, INT32 i4SaPeriod, INT32 i4Threshold);
extern INT32 IRHW_RxInit(INT32 i4Config, INT32 i4SaPeriod, INT32 i4Threshold);
extern INT32 IRHW_RxStop(void);
extern INT32 IRHW_RxSetCallback(PFN_IRRXCB_T pfnCallback, PFN_IRRXCB_T *ppfnOld);
extern INT32 i4IrHWUninit(void);
#if (!CONFIG_DRV_LINUX)
extern INT32 i4IrUninit(void);
#else
extern INT32 i4Ir_Uninit(UINT32 u4Case);
#endif
extern INT32 IR_Status_WD(void);
#if CONFIG_DRV_DENON_SUPPORT
extern void IRHW_FastejectKeyNotify(BOOL fgPowerOff);
#elif CONFIG_DRV_CUSTOM_JSN
extern void HW_FastejectKeyNotify(void);
#else
extern void IRHW_FastejectKeyNotify(void);
#endif
extern void IRHW_SetEnable(BOOL fgEnable);

/******************************************************************************/
/* IRTX IF functions. */

/* HWIR TX external functions declare. */
extern void IRHW_TxRdConf(INT32 *pi4Config, INT32 *pi4TPeriod, INT32 *pi4Modulation);
extern void IRHW_TxWrConf(INT32 i4Config, INT32 i4TPeriod, INT32 i4Modulation);
extern void IRHW_TxSendData(const INT32 *pi4DataArray, INT32 i4BitNum);

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520)
extern INT32 IRRX_RegResetIrKey(UINT32 u4ResetIr); /* return IR_SUCC: OK, IR_FAIL: NG */
extern UINT32 IRRX_GetRegResetIrKey(void);

#define RESET_MODE_NONE 0     /* no reset, while loop */
#define RESET_MODE_AUTO 1     /* Automatically Reset */
#define RESET_MODE_IR   2     /* IR Power Key Reset */

extern void IRRX_SetSysHaltResetMode(UINT32 u4Mode);
extern UINT32 IRRX_GetSysHaltResetMode(void);
extern void IRHW_PowerDown(void);
extern UINT32 BIM_WatchCounter(void);
extern void BIM_WatchDog(UINT32 u4Val);
#elif (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8530)
extern void IRHW_PDwn(void);
extern void IRHW_LoadPDwnCode(void);
extern INT32 _IRHW_WAKE_UP_ENABLE(UINT8 u1IRIdx, UINT32 u4KeyCodeM, UINT32 u4KeyCodeL);
extern INT32 _IRHW_POWER_DOWN_ENABLE(UINT8 u1IRIdx, UINT32 u4KeyCodeM, UINT32 u4KeyCodeL);
#endif
#endif
#endif

#endif /* __DRV_IR_H__ */

