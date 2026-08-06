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


/*****************************************************************************
*  Audio Driver: Exported interfaces for dspctrl and other modules
*****************************************************************************/

#ifndef _AUD_EXT_CTRL_H_
#define _AUD_EXT_CTRL_H_

#include "x_aud_dec.h"
#include "x_aud_ext.h"

void vAudExtCtrlNotifyInfo(AUD_DEC_STATE_T eState, bool fgDecoderReady);
void vAudExtCtrlNotifyPostDecoderState(void);

#endif /* _AUD_EXT_CTRL_H_ */

