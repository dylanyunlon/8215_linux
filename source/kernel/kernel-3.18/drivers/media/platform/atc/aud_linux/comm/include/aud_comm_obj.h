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



#ifndef _AUD_COMM_OBJ_H_
#define _AUD_COMM_OBJ_H_

#include "aud_comm_misc.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef struct 
{
    void (*SetValue)(void * pThis, u32 u4Value);  //the first argument must use void * and name m
    u32 (*GetValue)(void * pThis);

    u32 (*Delete)(void * pThis);              // the format and name must be like this
 
}AUD_OBJ_CLS_PUB, *PAUD_OBJ_CLS_PUB;                // use _CLS_T as postfix, means CLASS struct


AUD_OBJ_CLS_PUB *AudObj_New(void);


void AudObj_UnitTest_Cmd(u32 au4Params[], u32 u4ParamNum);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_COMM_OBJ_H_

