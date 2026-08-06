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



#ifndef _AUD_MISC_TBL_H_
#define _AUD_MISC_TBL_H_

#include "aud_comm_macros.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define AUD_TBL_SINE_16BIT_SZ           128
extern u8 AUD_TBL_SINE_16BIT[AUD_TBL_SINE_16BIT_SZ];

#define AUD_TBL_SINE_24BIT_SZ           192
extern u8 AUD_TBL_SINE_24BIT[AUD_TBL_SINE_24BIT_SZ];


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //__AUD_MISC_TBL_H_

