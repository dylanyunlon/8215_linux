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

#ifndef _VDEC_DEFINE_H_
#define _VDEC_DEFINE_H


#include "mm_errcode.h"


/*************************************************************************
*       
*     These public error code apply for MSDKC
*
*************************************************************************/

#define RET_VDEC_OK                                RET_MSDKC_OK   
//Alloc memory fail
#define RET_VDEC_OUTOFMEMORY                       RET_MSDKC_OUTOFMEMORY
//HW unsupported
#define RET_VDEC_FILE_UNSUPPORT                    MAKE_ERR_CODE(MOD_ERRCODE_VDEC, 1)
//HW decode timeout
#define RET_VDEC_DECODE_TIMEOUT                    MAKE_ERR_CODE(MOD_ERRCODE_VDEC, 2)

#define RET_VDEC_NO_RESOURCE                       MAKE_ERR_CODE(MOD_ERRCODE_VDEC, 3)


/****************************state code************************/
//frame error - bitstream error, frame unsupport, no ref pic......
#define RET_VDEC_STATE_FRAME_ERROR                 MAKE_STATE_CODE(MOD_ERRCODE_VDEC, 1)


#endif
