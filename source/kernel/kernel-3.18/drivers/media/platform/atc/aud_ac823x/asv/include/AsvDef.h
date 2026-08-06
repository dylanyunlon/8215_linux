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

#ifndef _ASV_DEF_H_
#define _ASV_DEF_H_

//---------------------------------------------------------------------------
//AUD-DSP state machine
//---------------------------------------------------------------------------
//AUD-DSP state
#define ST_DSP_POWER_OFF        0x00
#define ST_DSP_INIT             0x01
#define ST_DSP_READY            0x02
#define ST_DSP_PLAYING          0x03
#define ST_DSP_PLAYOK           0x04
#define ST_DSP_STOPPING         0x05
#define ST_DSP_PAUSING          0x06
#define ST_DSP_PAUSED           0x07
#define ST_DSP_RESUMING         0x08
//AUD-DSP state trigger
#define TR_DSP_POWER_OFF        0x00
#define TR_DSP_POWER_ON         0x01
#define TR_DSP_INIT_READY       0x02
#define TR_DSP_R_PLAY           0x03
#define TR_DSP_S_PLAY_OK        0x04
#define TR_DSP_R_STOP           0x05
#define TR_DSP_S_STOP_OK        0x06
#define TR_DSP_R_PAUSE          0x07
#define TR_DSP_S_PAUSE_OK       0x08
#define TR_DSP_R_RESUME         0x09
#define TR_DSP_S_RESUME_OK      0x0A
//AUD-DSP state machine result
#define RTN_DSP_FAIL            0x00
#define RTN_DSP_SUCCESS         0x01

//---------------------------------------------------------------------------
//DSP A state machine
//---------------------------------------------------------------------------
//DSP A state
#define ST_DSP_A_POWER_OFF       0x00
#define ST_DSP_A_INIT            0x01
#define ST_DSP_A_AOUT_OFF        0x02
#define ST_DSP_A_AOUT_STARTING   0x03
#define ST_DSP_A_AOUT_ON         0x04
#define ST_DSP_A_AOUT_STOPPING   0x05
#define ST_DSP_A_DISCONNECTED    0x06
#define ST_DSP_A_CONNECTING      0x07
#define ST_DSP_A_CONNECTED       0x08
#define ST_DSP_A_DISCONNECTING   0x09
#define ST_DSP_A_STEPPING        0x0A
#define ST_DSP_A_STEPPING_TO_END 0x0B //NEW_STEP_FLOW
//DSP A state trigger
#define TR_DSP_A_POWER_OFF      0x00
#define TR_DSP_A_POWER_ON       0x01
#define TR_DSP_A_INIT_READY     0x02
#define TR_DSP_A_R_AOUT_ON      0x03  
#define TR_DSP_A_S_AOUT_STARTED 0x04    // intr from DSP A
#define TR_DSP_A_R_AOUT_OFF     0x05
#define TR_DSP_A_S_AOUT_STOPPED 0x06    // intr from DSP A
#define TR_DSP_A_R_CONNECT      0x07
#define TR_DSP_A_S_CONNECTED    0x08
#define TR_DSP_A_R_DISCONNECT   0x09
#define TR_DSP_A_S_DISCONNECTED 0x0A
#define TR_DSP_A_R_STEP         0x0B
#define TR_DSP_A_S_STEP_DONE         0x0C
#define TR_DSP_A_R_STEP_TO_END       0x0D  //NEW_STEP_FLOW
#define TR_DSP_A_S_STEP_CANCEL_DONE  0x0E  //NEW_STEP_FLOW


//DSP A state machine result
#define RTN_DSP_A_FAIL          0x00
#define RTN_DSP_A_SUCCESS       0x01

//---------------------------------------------------------------------------
//DSP B Decoder state machine (including primary and secondary)
//---------------------------------------------------------------------------
//DSP B state
#define ST_DSP_B_POWER_OFF      0x00
#define ST_DSP_B_INIT           0x01
#define ST_DSP_B_READY          0x02
#define ST_DSP_B_PARSING        0x03
#define ST_DSP_B_WAIT_CFG_ACK   0x04
#define ST_DSP_B_DECODER_INIT   0x05
#define ST_DSP_B_DECODING       0x06
#define ST_DSP_B_EOS            0x07
#define ST_DSP_B_STOPPING       0x08
//DSP B state trigger
#define TR_DSP_B_POWER_OFF      0x00
#define TR_DSP_B_POWER_ON       0x01
#define TR_DSP_B_INIT_READY     0x02
#define TR_DSP_B_R_PLAY         0x03
#define TR_DSP_B_S_SEND_CFG     0x04
#define TR_DSP_B_R_RECEIVE_CFG  0x05
#define TR_DSP_B_S_DECODING_OK  0x06
#define TR_DSP_B_S_EOS          0x07
#define TR_DSP_B_R_STOP         0x08
#define TR_DSP_B_S_STOP_OK      0x09
#define TR_DSP_B_R_REPLAY       0x0A
//#define TR_DSP_B_R_RECFG        0x0B  //not allow
//DSP B state machine result
#define RTN_DSP_B_FAIL          0x00
#define RTN_DSP_B_SUCCESS       0x01

//---------------------------------------------------------------------------
//DSP Re-Encode state machine
//---------------------------------------------------------------------------
//DSP Re-Encode state
#define ST_DSP_REENC_POWER_OFF          0x00
#define ST_DSP_REENC_INIT               0x01
#define ST_DSP_REENC_STOP               0x02
#define ST_DSP_REENC_STARTING           0x03
#define ST_DSP_REENC_START              0x04
#define ST_DSP_REENC_STOPPING           0x05
#define ST_DSP_REENC_PAUSE              0x06
#define ST_DSP_REENC_PAUSING            0x07
#define ST_DSP_REENC_RESUME             0x08
#define ST_DSP_REENC_RESUMING           0x09
//DSP Re-Encode state trigger
#define TR_DSP_REENC_POWER_OFF          0x00
#define TR_DSP_REENC_POWER_ON           0x01
#define TR_DSP_REENC_INIT_READY         0x02
#define TR_DSP_REENC_R_STOP             0x03
#define TR_DSP_REENC_S_STOPPED          0x04
#define TR_DSP_REENC_R_START            0x05
#define TR_DSP_REENC_S_STARTED          0x06
#define TR_DSP_REENC_R_PAUSE            0x07
#define TR_DSP_REENC_S_PAUSED           0x08
#define TR_DSP_REENC_R_RESUME           0x09
#define TR_DSP_REENC_S_RESUMED          0x0A
//DSP Re-Encode state machine result
#define RTN_DSP_REENC_FAIL              0x00
#define RTN_DSP_REENC_SUCCESS           0x01


//---------------------------------------------------------------------------
// DSP Encoder state machine -- Water (AUD_RIPPING)
//---------------------------------------------------------------------------
// DSP Encoder state
#define ST_DSP_ENC_POWER_OFF            0x00
#define ST_DSP_ENC_INIT                 0x01
#define ST_DSP_ENC_STOP                 0x02
#define ST_DSP_ENC_STARTING             0x03
#define ST_DSP_ENC_START                0x04
#define ST_DSP_ENC_STOPPING             0x05
#define ST_DSP_ENC_FLUSH                0x06
#define ST_DSP_ENC_FLUSHING             0x07
// DSP Encoder state trigger
#define TR_DSP_ENC_POWER_OFF            0x00
#define TR_DSP_ENC_POWER_ON             0x01
#define TR_DSP_ENC_INIT_READY           0x02
#define TR_DSP_ENC_R_STOP               0x03
#define TR_DSP_ENC_S_STOPPED            0x04
#define TR_DSP_ENC_R_START              0x05
#define TR_DSP_ENC_S_STARTED            0x06
#define TR_DSP_ENC_R_FLUSH              0x07
#define TR_DSP_ENC_S_FLUSHED            0x08
// DSP Encoder state machine result
#define RTN_DSP_ENC_FAIL                0x00
#define RTN_DSP_ENC_SUCCESS             0x01

#endif
