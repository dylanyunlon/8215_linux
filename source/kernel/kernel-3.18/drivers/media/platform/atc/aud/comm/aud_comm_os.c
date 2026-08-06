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
*[File]                 aud_comm_os.c
*[Author]               tongfa.luo@autochips.com
*[Description]
*
*[Copyright]
*
******************************************************************************/

#include "aud_oal.h"
#include "aud_comm_os.h"
#include <linux/interrupt.h>
#include "aud_if.h"

//================================================//
  #define CodeSight_AudOS_IRQ
//================================================//

void AudOS_IRQ_Enable(u32 u4Vect)
{
    if (!BIM_EnableIrq(u4Vect))
    {
      COMMLOG_ERR(T("Failed to Enable IRQ(%d).\r\n"), (s32)u4Vect);
    }
}


void AudOS_IRQ_Disable(u32 u4Vect)
{
    if (!BIM_DisableIrq(u4Vect))
    {
        COMMLOG_ERR(T("Failed to Disable IRQ(%d).\r\n"), (s32)u4Vect);
    }
}


void AudOS_IRQ_Clear(u32 u4Vect)
{
    BIM_ClearIrq(u4Vect);
}


//================================================//
  #define CodeSight_AudOS_ISR
//================================================//

bool AudOS_ISR_Reg(u32 u4Vect, x_os_isr_fct pfnIsr)
{
    bool fgRet = TRUE;
    x_os_isr_fct pfnOldIsr;
    char dev_name[16] = {0};

    snprintf(dev_name, 16, "ISR_Aud0x%x", u4Vect);

    AudOS_IRQ_Disable(u4Vect);
    if (OSR_OK != request_irq(u4Vect, pfnIsr, 0, dev_name, NULL))
    {
        fgRet = FALSE;
        COMMLOG_ERR(T("Failed to Register IRQ(%d).\r\n"), (s32)u4Vect);
    }

    return (fgRet);
}


bool AudOS_ISR_UnReg(u32 u4Vect)
{
    bool fgRet = TRUE;
    x_os_isr_fct pfnOldIsr;

    AudOS_IRQ_Disable(u4Vect);

    free_irq((unsigned int)u4Vect, NULL);

    return (fgRet);
}


//================================================//
  #define CodeSight_AudOS_Memory
//================================================//

u32 AudOS_Memory_Alloc(u32 u4Size, u32 u4Alignment, u32 *pu4PhyAddr)
{
    u32 u4VirAddr = 0;
    #ifndef __linux__
    u4VirAddr = (u32)AllocPhysMem(u4Size, PAGE_READWRITE, u4Alignment, 0, (u32 *)(pu4PhyAddr));
    #else
    //u4VirAddr = (u32)x_mem_alloc_ret_phy_addr(u4Size, pu4PhyAddr);
    u4VirAddr = (u32)kmalloc(u4Size, GFP_KERNEL);
    *pu4PhyAddr = __pa(u4VirAddr);
    #endif
    if (0 == u4VirAddr)
    {
        COMMLOG_ERR(T("Failed to Malloc Physical Memory, Size(%d)!\r\n"), (s32)u4Size);
        AUD_ASSERT(u4VirAddr);
        return (0);
    }
    return (u4VirAddr);
}


void AudOS_Memory_Free(u32 *pu4VirAddr)
{
    if (*pu4VirAddr)
    {
        #ifndef __linux__
        FreePhysMem(*pu4VirAddr);
        #else
        kfree((void *)(*pu4VirAddr));
        #endif
        *pu4VirAddr = 0;
    }
}


//================================================//
  #define CodeSight_AudOS_Regkey
//================================================//
#ifndef __linux__
static u32 OS_Regkey_Get(DRV_TYPE_E eDrv, s8 *szName, u8 *pbValue, u32 u4DataSize)
{
    u32 u4Ret = 0;
    #ifndef __linux__
    HKEY hKey = NULL;
    #endif

    #ifndef __linux__
    RegOpenKeyEx(HKEY_LOCAL_MACHINE , REGKEY_PATH(eDrv), 0 , KEY_ALL_ACCESS , &hKey);

    if (hKey)
    {
        u32 u4DataType;
        if (ERROR_SUCCESS ==
            RegQueryValueEx(hKey, szName, 0, &u4DataType, (PUCHAR)pbValue, &u4DataSize))
        {
            u4Ret = u4DataSize;
        }
        RegCloseKey(hKey);
    }
    #else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (u4Ret);
}


static u32 OS_Regkey_Set(DRV_TYPE_E eDrv, s8 *szName, u32 u4DataType, u8 *pbValue, u32 u4DataSize)
{
    u32 u4Ret = 0;
    #ifndef __linux__
    HKEY hKey = NULL;
    #endif

    #ifndef __linux__
    RegOpenKeyEx(HKEY_LOCAL_MACHINE , REGKEY_PATH(eDrv), 0 , KEY_ALL_ACCESS , &hKey);

    if (hKey)
    {
        if (ERROR_SUCCESS ==
            RegSetValueEx(hKey, szName, 0, u4DataType, (PUCHAR)pbValue, u4DataSize))
        {
            u4Ret = u4DataSize;
        }
        RegCloseKey(hKey);
    }
    #else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (u4Ret);
}
#endif

bool AudOS_Regkey_GetDword(DRV_TYPE_E eDrv, s8 *szName, u32 *pu4Value, u32 u4Default)
{
    bool fgRet = TRUE;

    #ifndef __linux__
    if (!OS_Regkey_Get(eDrv, szName, (u8 *)pu4Value, sizeof(u32)))
    {
        *pu4Value = u4Default;
        fgRet = FALSE;
    }
    #else
    *pu4Value = u4Default;
    //COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (fgRet);
}


bool AudOS_Regkey_SetDword(DRV_TYPE_E eDrv, s8 *szName, u32 u4Value)
{
    bool fgRet = TRUE;

    #ifndef __linux__
    if (!OS_Regkey_Set(eDrv, szName, REG_u32, (u8 *)(&u4Value), sizeof(u32)))
    {
        fgRet = FALSE;
    }
    #else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (fgRet);
}


bool AudOS_Regkey_GetString(DRV_TYPE_E eDrv, s8 *szName, s8 *szValue, s8 *szDefault)
{
    bool fgRet = TRUE;

    #ifndef __linux__
    if (!OS_Regkey_Get(eDrv, szName, (u8 *)szValue, 200))
    {
        wcscpy(szValue, szDefault);
        fgRet = FALSE;
    }
    #else
    {
        s8 *Dest, *Src;
        Src = (s8*)szDefault;
        Dest = (s8*)szValue;
        while (*Src)
        {
            *Dest++ = *Src++;
        }
    }
    #endif
    return (fgRet);
}


bool AudOS_Regkey_SetString(DRV_TYPE_E eDrv, s8 *szName, s8 *szValue)
{
    bool fgRet = TRUE;

    #ifndef __linux__
    u32 u4Size = (wcslen(szValue) + 1) << 1;
    if (!OS_Regkey_Set(eDrv, szName, REG_SZ, (u8 *)szValue, u4Size))
    {
        fgRet = FALSE;
    }
    #else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (fgRet);
}


bool AudOS_Regkey_GetBinary(DRV_TYPE_E eDrv, s8 *szName, u8 *szValue, u32 u4Size)
{
    bool fgRet = TRUE;

    #ifndef __linux__
    if (!OS_Regkey_Get(eDrv, szName, (u8 *)szValue, u4Size))
    {
        fgRet = FALSE;
    }
    #else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (fgRet);
}


bool AudOS_Regkey_SetBinary(DRV_TYPE_E eDrv, s8 *szName, u8 *szValue, u32 u4Size)
{
    bool fgRet = TRUE;

    #ifndef __linux__
    if (!OS_Regkey_Set(eDrv, szName, REG_BINARY, (u8 *)szValue, u4Size))
    {
        fgRet = FALSE;
    }
    #else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
    #endif

    return (fgRet);
}

//================================================//
  #define CodeSight_AudOS_File
//================================================//


void* AudOS_File_Open(void* hFile, bool fgRead, s8 *szFileName)
{
    if (hFile != INVALID_HANDLE_VALUE)
    {
        COMMLOG_ERR(T("[OS]Open File Fail: hFile is not inalid handle!\r\n"));
    }
    else
    {
        s8 szFullName[AUD_FILE_MAX_PATH] = {0};
        u32 i = 0;

        for (i = 0; i < (AUD_FILE_MAX_PATH - 1) && szFileName[i]; i++)
        {
            szFullName[i] = (s8)szFileName[i];
        }
        szFullName[i] = 0;

        if (fgRead) {
            hFile = (void *)filp_open(szFullName, O_RDONLY | O_CREAT, 0);
        } else {
            hFile = (void *)filp_open(szFullName, O_RDWR | O_CREAT, 0);
        }
        COMMLOG_ERR_DBG(hFile == INVALID_HANDLE_VALUE, T("[OS] Open file %s !   fgRead(%d)\r\n"),
            szFullName, (s32)fgRead);
    }

    return (hFile);
}


void * AudOS_File_Close(void * hFile)
{
    if (INVALID_HANDLE_VALUE != hFile)
    {
        filp_close((struct file *)hFile, NULL);
        hFile = INVALID_HANDLE_VALUE;
    }

    return (hFile);
}


u32 AudOS_File_Read(void * hFile, void * lpData, u32 u4Size)
{
#ifndef __linux__
    if ((INVALID_HANDLE_VALUE == hFile) ||
        !ReadFile(hFile, (u8 *)lpData, u4Size, &u4Size, NULL))
    {
        COMMLOG_ERR(T("Read File Fail. hFile(0x%x) Size(%d)!\r\n"), hFile, u4Size);
        u4Size = 0;
    }
#else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
#endif
    return (u4Size);
}


u32 AudOS_File_Write(void * hFile, void * lpData, u32 u4Size)
{
#ifndef __linux__
    if ((INVALID_HANDLE_VALUE == hFile) ||
        !WriteFile(hFile, (u8 *)lpData, u4Size, &u4Size, NULL))
    {
        COMMLOG_ERR(T("Write File Fail. hFile(0x%x) Size(%d)!\r\n"), hFile, u4Size);
        u4Size = 0;
    }
#else
    // TODO:
    COMMLOG_ERR(T("To be complete for Linux. File(%s) Line(%d)!\r\n"), __FILE__, __LINE__);
#endif
    return u4Size;
}

//================================================//
  #define CodeSight_OS_BackupBuff
//================================================//

void AudOS_BkBuf_Free(PAUD_BACKUP_BUF_T prBkBuf)
{
    if (prBkBuf->u4VirAddr)
    {
        COMMLOG_INFO(T("Free BkBuf (0x%x, 0x%x, 0x%x) \n"),
                (u32)(prBkBuf->u4VirAddr), (u32)(prBkBuf->u4PhyAddr), (u32)(prBkBuf->u4MemSize));
        AudOS_Memory_Free(&prBkBuf->u4VirAddr);
    }
}


void AudOS_BkBuf_Alloc(PAUD_BACKUP_BUF_T prBkBuf, u32 u4Size)
{
    if (u4Size)
    {
        if (prBkBuf->u4VirAddr) {
            AudOS_BkBuf_Free(prBkBuf);
        }
        prBkBuf->u4MemSize = u4Size;
        prBkBuf->u4VirAddr = AudOS_Memory_Alloc(u4Size, 0xF, &prBkBuf->u4PhyAddr);
        prBkBuf->u4UsedSize = 0;

        COMMLOG_INFO(T("Alloc BkBuf: BkBuf(0x%x, 0x%x, 0x%x) \n"),
            (u32)(prBkBuf->u4VirAddr), (u32)(prBkBuf->u4PhyAddr), (u32)(prBkBuf->u4MemSize));
    }
}


void AudOS_BkBuf_Reset(PAUD_BACKUP_BUF_T prBkBuf)
{
    COMMLOG_INFO(T("Reset BkBuf: (0x%x, 0x%x, 0x%x) \n"),
            (u32)(prBkBuf->u4VirAddr), (u32)(prBkBuf->u4PhyAddr), (u32)(prBkBuf->u4MemSize));
    prBkBuf->u4VirAddr = 0;
}


u32 AudOS_BkBuf_Request(PAUD_BACKUP_BUF_T prBkBuf, u32 u4Size)
{
    u32 u4VirAddr = 0;

    if (prBkBuf->u4UsedSize + u4Size > prBkBuf->u4MemSize) {
        COMMLOG_INFO(T("No free backup memory!!! (0x%x + 0x%x > 0x%x)\n"),
            (u32)(prBkBuf->u4UsedSize), (u32)u4Size, (u32)(prBkBuf->u4MemSize));
    } else {
        u4VirAddr = prBkBuf->u4VirAddr + prBkBuf->u4UsedSize;
        prBkBuf->u4UsedSize += u4Size;
    }

    return (u4VirAddr);
}


void AudOS_FlushDCacheRange(u32 u4DstAddr, u32 u4Size)
{
#ifndef __linux__
#if CONFIG_DRV_LINUX
    BSP_FlushDCacheRange(u4Dst1, u4Size);
#endif
#endif
}

s32 to_sched_priority(u8 ui1_priority)
{
	s32 sched_priority;

	sched_priority = 100 - (s32)ui1_priority * 100 / 256;
	if (sched_priority < 1)
		sched_priority = 1;
	if (sched_priority > 99)
		sched_priority = 99;
	return sched_priority;
}
