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




#ifndef _AUD_IF_HW_H_
#define _AUD_IF_HW_H_

#include "aud_if_comm.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct 
{
    u32 (*Setup)(void * pThis, void * pvParams);
    u32 (*GetStatus)(void * pThis);
    
    u32 (*Start)(void * pThis, u32 u4Params);
    u32 (*Stop)(void * pThis, u32 u4Params);

    u32 (*CfgUpd)(void * pThis, void * pvParams);

    u32 (*GetBuf)(void * pThis, PAUD_DATA_BUF_T prBuf);
    u32 (*GetPoint)(void * pThis);
    u32 (*SetPoint)(void * pThis, u32 u4Point);
   
}AUD_HW_IF_T, *PAUD_HW_IF_T;


typedef struct
{
    u32 (*Setup)(void * pThis, void * pvParams);
    u32 (*GetStatus)(void * pThis);
    
    u32 (*Start)(void * pThis, u32 u4Params);
    u32 (*Stop)(void * pThis, u32 u4Params);

    u32 (*CfgUpg)(void * pThis, void * pvParams);

    u32 (*GetIBuf)(void * pThis, PAUD_DATA_BUF_T prBuf);
    u32 (*GetIRP)(void * pThis);
    u32 (*SetIWP)(void * pThis, u32 u4WP);
    
    u32 (*GetOBuf)(void * pThis, PAUD_DATA_BUF_T prBuf);
    u32 (*GetOWP)(void * pThis);    
    u32 (*SetORP)(void * pThis, u32 u4RP); 
    
}AUD_HW_IO_IF_T, *PAUD_HW_IO_IF_T;

#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_IF_HW_H_