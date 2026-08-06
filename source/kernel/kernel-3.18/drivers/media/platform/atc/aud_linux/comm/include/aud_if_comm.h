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




#ifndef _AUD_IF_COMM_H_
#define _AUD_IF_COMM_H_

#include "aud_comm_macros.h"
#include "aud_comm_datatype.h"
#include "aud_comm_ver.h"
#include "aud_comm_reg_rw.h"

#include "aud_comm_os.h"
#include "aud_comm_tbl.h"
#include "aud_comm_misc.h"
#include "aud_comm_file.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


extern void AudCommTest_Cmd(u32 u4Type, u32 u4Param1, u32 u4Param2);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //_AUD_IF_COMM_H_
