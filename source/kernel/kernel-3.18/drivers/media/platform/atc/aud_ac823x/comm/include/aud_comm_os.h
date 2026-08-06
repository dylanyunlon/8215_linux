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

/******************************************************************************
*[File]                     aud_comm_os.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*        
*
******************************************************************************/
#ifndef _AUD_COMM_OS_H_
#define _AUD_COMM_OS_H_

#include "x_os.h"
#include "x_bim.h"
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/sched.h>


#include "aud_comm_log.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define AUD_REGKEY_PATH                     L"Drivers\\BuiltIn\\AUDDEC" 
#define WAV_REGKEY_PATH                     L"Drivers\\BuiltIn\\WAVEDRV" 

#define REGKEY_PATH(idx)                    (idx) ? L"Drivers\\BuiltIn\\WAVEDRV" : L"Drivers\\BuiltIn\\AUDDEC"

#define AUDOS_MEMCPY                      memcpy

typedef enum
{
    AUD_DRV = 0,
    WAV_DRV = 1
}DRV_TYPE_E;

//============================================================//

typedef struct
{
    uintptr_t u4PhyAddr;
    uintptr_t u4VirAddr;
    u32 u4MemSize;
    u32 u4UsedSize;   
}AUD_BACKUP_BUF_T, *PAUD_BACKUP_BUF_T;


//===========================================================//

void AudOS_IRQ_Enable(u32 u4Vect);
void AudOS_IRQ_Disable(u32 u4Vect);
void AudOS_IRQ_Clear(u32 u4Vect);

//===========================================================//

bool AudOS_ISR_Reg(u32 u4Vect, x_os_isr_fct pfnIsr);
bool AudOS_ISR_UnReg(u32 u4Vect);

//===========================================================//

uintptr_t AudOS_Memory_Alloc(u32 u4Size, u32 u4Alignment, uintptr_t *pu4PhyAddr);
void AudOS_Memory_Free(uintptr_t *pu4VirAddr);

//===========================================================//

bool AudOS_Regkey_GetDword(DRV_TYPE_E eDrv, s8 *szName, u32 *pu4Value, u32 u4Default);
bool AudOS_Regkey_SetDword(DRV_TYPE_E eDrv, s8 *szName, u32 u4Value);

bool AudOS_Regkey_GetString(DRV_TYPE_E eDrv, s8 *szName, s8 *szValue, s8 *szDefault);
bool AudOS_Regkey_SetString(DRV_TYPE_E eDrv, s8 *szName, s8 *szValue);

bool AudOS_Regkey_GetBinary(DRV_TYPE_E eDrv, s8 *szName, u8 *szValue, u32 u4Size);
bool AudOS_Regkey_SetBinary(DRV_TYPE_E eDrv, s8 *szName, u8 *szValue, u32 u4Size);

//===========================================================//

void * AudOS_File_Open(void * hFile, bool fgRead, s8 *szFileName);
void * AudOS_File_Close(void * hFile);
u32 AudOS_File_Read(void * hFile, void * lpData, u32 u4Size);
u32 AudOS_File_Write(void * hFile, void * lpData, u32 u4Size);

//===========================================================//

//===========================================================//

void AudOS_BkBuf_Free(PAUD_BACKUP_BUF_T prBkBuf);
void AudOS_BkBuf_Alloc(PAUD_BACKUP_BUF_T prBkBuf, u32 u4Size);
void AudOS_BkBuf_Reset(PAUD_BACKUP_BUF_T prBkBuf);
uintptr_t AudOS_BkBuf_Request(PAUD_BACKUP_BUF_T prBkBuf, u32 u4Size);

//===========================================================//

void AudOS_FlushDCacheRange(uintptr_t u4DstAddr, u32 u4Size);

s32 to_sched_priority(u8 ui1_priority);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //_AUD_COMM_OS_H_

