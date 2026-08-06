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
 


#ifndef _DMX_EVENT_H_
#define _DMX_EVENT_H_

#include "x_typedef.h"
#include "dmx_define.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {
#endif

#if DMX_SUPPORT_3RD_INSTS

#define SPT_EVT_NAME_PBBUF_0			_T("SPT_EVT_PBBUF_0")
#define SPT_EVT_NAME_PSROFF_0			_T("SPT_EVT_PSROFF_0")
#define SPT_EVT_NAME_RSP_0				_T("SPT_EVT_RSP_0")
#define SPT_EVT_NAME_RSPTXOFF_0		_T("SPT_EVT_RSPTXOFF_0")

#define SPT_EVT_NAME_PBBUF_1			_T("SPT_EVT_PBBUF_1")
#define SPT_EVT_NAME_PSROFF_1			_T("SPT_EVT_PSROFF_1")
#define SPT_EVT_NAME_RSP_1				_T("SPT_EVT_RSP_1")
#define SPT_EVT_NAME_RSPTXOFF_1		_T("SPT_EVT_RSPTXOFF_1")

#define SPT_EVT_NAME_PBBUF_2			_T("SPT_EVT_PBBUF_2")
#define SPT_EVT_NAME_PSROFF_2			_T("SPT_EVT_PSROFF_2")
#define SPT_EVT_NAME_RSP_2				_T("SPT_EVT_RSP_2")
#define SPT_EVT_NAME_RSPTXOFF_2		_T("SPT_EVT_RSPTXOFF_2")

#define SPT_EVT_NAME_PCR					_T("SPT_EVT_PCR")

#define SPT_EVT_NAME_MPG_INFO			_T("SPT_EVT_MPG_INFO")

#else				/* DMX_SUPPORT_3RD_INSTS */

#define SPT_EVT_NAME_PBBUF				_T("SPT_EVT_PBBUF")
#define SPT_EVT_NAME_PSROFF				_T("SPT_EVT_PSROFF")
#define SPT_EVT_NAME_MPG_INFO			_T("SPT_EVT_MPG_INFO")

#define SPT_EVT_NAME_PBBUF_SP			_T("SPT_EVT_PBBUF_SP")
#define SPT_EVT_NAME_PSROFF_SP		_T("SPT_EVT_PSROFF_SP")

#define SPT_EVT_NAME_PCR					_T("SPT_EVT_PCR")

#define SPT_EVT_NAME_RSP					_T("SPT_EVT_RSP")
#define SPT_EVT_NAME_RSP_SP				_T("SPT_EVT_RSP_SP")

#define SPT_EVT_NAME_RSPTXOFF			_T("SPT_EVT_RSPTXOFF")
#define SPT_EVT_NAME_RSPTXOFF_SP	_T("SPT_EVT_RSPTXOFF_SP")

#endif				/* DMX_SUPPORT_3RD_INSTS */

#ifdef __cplusplus
}
#endif
#endif				/* #ifndef _DMX_EVENT_H_ */
