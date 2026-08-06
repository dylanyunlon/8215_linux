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
#ifndef __DRV_IF_EADEV_H_
#define __DRV_IF_EADEV_H_

#include "u_common.h"

#include "drv_if.h"
#include "drv_if_rc.h"
#include "drv_if_ftr.h"
#include "drv_if_syncctrl.h"
#include "drv_if_vdp.h"
#include "drv_comp_id.h"
#include "x_aud_ext.h"

#include "drv_im.h"
#include "x_rm_dev_types.h"

typedef enum
{
    EADEV_DEC_AFD_STD = 0,
    EADEV_DEC_AFD_MULTI,
    EADEV_DEC_PROLOGIC,
    EADEV_DEC_PLII_MOVIE,
    EADEV_DEC_PLII_MUSIC,
    EADEV_DEC_PLII_X_MOVIE,
    EADEV_DEC_PLII_X_MUSIC,
    EADEV_DEC_NE06_CINEMA,
    EADEV_DEC_NE06_MUISC,
    EADEV_DEC_LINK,
    EADEV_DEC_2CH_STEREO,
    EADEV_DEC_HP_2CH,
    EADEV_DEC_SWAP_THRU,
    EADEV_DEC_SWAP_F2S,
    EADEV_DEC_SWAP_F2CW,
    EADEV_DEC_SWAP_FULL,
    EADEV_DEC_SWAP_F2SB,
    
}DRV_EADEV_DECODE_MODE;


typedef enum
{
    EADEV_SOUND_AUTO = 0,
    EADEV_SOUND_MUSIC,
    EADEV_SOUND_MOVIE,
    EADEV_SOUND_SPORTS,
    EADEV_SOUND_NEWS,
    EADEV_SOUND_GAME,
    EADEV_SOUND_OMNI_DIR,
    
}DRV_EADEV_SOUND_MODE;


/// This interface represents IIDEADev2IFcon interface.
/// This interface contains EADev2IFcon functions.
typedef struct _IEADev2IFcon
{
  /// Set sound effect setting to IFcon
  /// \param fgOn [IN] Ture: Sound effect on. False: Sound effect off.
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSoundEffect)(BOOL fgOn, BOOL * pfgValid);

  /// Set speaker surround back connnection setting to IFcon
  /// \param fgConnectOn [IN] Ture: Exist. False: None.
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerConnect_SB)(BOOL fgConnectOn, BOOL * pfgValid);

  /// Set speaker surround connnection setting to IFcon
  /// \param fgConnectOn [IN] Ture: Exist. False: None.
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerConnect_S)(BOOL fgConnectOn, BOOL * pfgValid);

  /// Set speaker center connnection setting to IFcon
  /// \param fgConnectOn [IN] Ture: Exist. False: None.
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerConnect_C)(BOOL fgConnectOn, BOOL * pfgValid);

  /// Set speaker front left distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_FL)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker front right distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_FR)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker center distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_C)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker sround left distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SL)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker sround right distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SR)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker sround back left distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SBL)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker sround back right distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SBR)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker subwoofer distance setting to IFcon
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SW)(UINT16 u2dist, BOOL * pfgValid);

  /// Set speaker front left level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_FL)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker front right level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_FR)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker centerlevel setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_C)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker surround left level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SL)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker surround right level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SR)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker surround back left level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SBL)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker surround back right level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SBR)(INT8 ilevel, BOOL * pfgValid);

  /// Set speaker subwoffer level setting to IFcon
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SW)(INT8 ilevel, BOOL * pfgValid);

  /// Notify audio information to IFcon
  /// \param t_aud_info [IN] audio information structure, see x_aud_ext.h for detail
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconNotifyAudioInfo)(AUDIO_INFORMATION_T* pt_aud_info);

  /// Notify audio output channel status to IFcon
  /// \param t_aud_output [IN] audio output channel information structure, see x_aud_ext.h for detail
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconNotifyOutputStatus)(AUDIO_OUTPUT_STATUS_T* pt_aud_output);

  
  /// Notify audio MCK status to IFcon
  /// \param b_stable [IN] TRUE: stable; FALSE: unstable
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconNotifyMCKStatus)(BOOL b_stable);

  /// Notify audio Post Decode information to IFcon
  /// \param e_post_dec [IN] post decode state
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconNotifyPostDecInfo)(AUD_DEC_POST_DECODE_INFO_T e_post_dec);

  /// Set decode mode to IFcon
  /// \param e_dec_mode [IN] Current decode mode
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetDecodeMode)(DRV_EADEV_DECODE_MODE e_dec_mode, BOOL* pfgValid);

  /// Set sound mode to IFcon
  /// \param e_sud_mode [IN] Current sound mode
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSoundMode)(DRV_EADEV_SOUND_MODE e_sud_mode, BOOL* pfgValid);

  /// Set dynamic bass on/off setting to IFcon
  /// \param fgOn [IN] dynamic bass on or off
  /// \param pfgValid [OUT] Audio driver need to set this setting or not
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetDynamicBass)(BOOL fgOn, BOOL* pfgValid);
  
} IEADev2IFcon;

/// This interface represents IIDEADev2Aud interface.
/// This interface contains EADev2Aud functions.
typedef struct _IEADev2Aud
{
  /// Set sound effect setting to Audio Driver
  /// \param fgOn [IN] Ture: Sound effect on. False: Sound effect off.
  /// \return This function returns negative value if failed.
  INT32 (*pi4IAudetSoundEffect)(BOOL fgOn);

  /// Set speaker surround back connnection setting to Audio Driver
  /// \param fgConnectOn [IN] Ture: Exist. False: None.
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerConnect_SB)(BOOL fgConnectOn);

  /// Set speaker surround connnection setting to Audio Driver
  /// \param fgConnectOn [IN] Ture: Exist. False: None.
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerConnect_S)(BOOL fgConnectOn);

  /// Set speaker center connnection setting to Audio Driver
  /// \param fgConnectOn [IN] Ture: Exist. False: None.
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerConnect_C)(BOOL fgConnectOn);

  /// Set speaker front left distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_FL)(UINT16 u2dist);

  /// Set speaker front right distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_FR)(UINT16 u2dist);

  /// Set speaker center distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_C)(UINT16 u2dist);

  /// Set speaker sround left distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SL)(UINT16 u2dist);

  /// Set speaker sround right distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SR)(UINT16 u2dist);

  /// Set speaker sround back left distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SBL)(UINT16 u2dist);

  /// Set speaker sround back right distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SBR)(UINT16 u2dist);

  /// Set speaker subwoofer distance setting to Audio Driver
  /// \param u2dist [IN] The speaker distance value (1cm step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerDist_SW)(UINT16 u2dist);

  /// Set speaker front left level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_FL)(INT8 ilevel);

  /// Set speaker front right level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_FR)(INT8 ilevel);

  /// Set speaker centerlevel setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_C)(INT8 ilevel);

  /// Set speaker surround left level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SL)(INT8 ilevel);

  /// Set speaker surround right level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SR)(INT8 ilevel);

  /// Set speaker surround back left level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SBL)(INT8 ilevel);

  /// Set speaker surround back right level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SBR)(INT8 ilevel);

  /// Set speaker subwoffer level setting to Audio Driver
  /// \param ilevel [IN] The speaker level value (0.5dB step)
  /// \return This function returns negative value if failed.
  INT32 (*pi4IFconSetSpeakerLvl_SW)(INT8 ilevel);
  
} IEADev2Aud;


#endif //__DRV_IF_EADEV_H_
