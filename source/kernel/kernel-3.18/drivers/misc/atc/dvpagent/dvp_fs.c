
/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2008 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
*  RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file dvp_fs.cpp
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* -------
*
*
*------------------------------------------------------------------------------
*
*******************************************************************************/
#include "agent.h"
#include "dvp_fs.h"
#include "mm_debug.h"

#define FSLOG  (0)

static  struct DFileSysIO  g_cFSIO;

#define DVP_FS_TOTAL_CNT_SA      (g_cFSIO.m_dwBaseAddr)
#define DVP_FS_TOTAL_CNT_SIZE    (64)
#define DVP_FS_TOTAL_CNT_EA      (DVP_FS_TOTAL_CNT_SA + DVP_FS_TOTAL_CNT_SIZE)


#define DVP_FS_COUT_DIR_SA       (DVP_FS_TOTAL_CNT_SA)
#define DVP_FS_COUT_DIR_SIZE     (2)
#define DVP_FS_COUT_DIR_EA       \
    (DVP_FS_COUT_DIR_SA + DVP_FS_COUT_DIR_SIZE)

#define DVP_FS_COUT_VIDEO_SA     (DVP_FS_COUT_DIR_EA)
#define DVP_FS_COUT_VIDEO_SIZE   (2)
#define DVP_FS_COUT_VIDEO_EA     (DVP_FS_COUT_VIDEO_SA + DVP_FS_COUT_VIDEO_SIZE)

#define DVP_FS_COUT_AUDIO_SA     (DVP_FS_COUT_VIDEO_EA)
#define DVP_FS_COUT_AUDIO_SIZE   (2)
#define DVP_FS_COUT_AUDIO_EA     (DVP_FS_COUT_AUDIO_SA + DVP_FS_COUT_AUDIO_SIZE)

#define DVP_FS_COUT_PIC_SA       (DVP_FS_COUT_AUDIO_EA)
#define DVP_FS_COUT_PIC_SIZE     (2)
#define DVP_FS_COUT_PIC_EA       \
    (DVP_FS_COUT_PIC_SA + DVP_FS_COUT_PIC_SIZE)

#define DVP_FS_FILTER_VIDEO_SA   (DVP_FS_COUT_PIC_EA)
#define DVP_FS_FILTER_VIDEO_SIZE (4)
#define DVP_FS_FILTER_VIDEO_EA   \
    (DVP_FS_FILTER_VIDEO_SA + DVP_FS_FILTER_VIDEO_SIZE)

#define DVP_FS_FILTER_AUDIO_SA   (DVP_FS_FILTER_VIDEO_EA)
#define DVP_FS_FILTER_AUDIO_SIZE (4)
#define DVP_FS_FILTER_AUDIO_EA   \
    (DVP_FS_FILTER_AUDIO_SA + DVP_FS_FILTER_AUDIO_SIZE)

#define DVP_FS_FILTER_PIC_SA     (DVP_FS_FILTER_AUDIO_EA)
#define DVP_FS_FILTER_PIC_SIZE   (4)
#define DVP_FS_FILTER_PIC_EA     (DVP_FS_FILTER_PIC_SA + DVP_FS_FILTER_PIC_SIZE)

#define DVP_FS_SBLIST_SA         (DVP_FS_FILTER_PIC_EA)
#define DVP_FS_SBLIST_SIZE       (4)
#define DVP_FS_SBLIST_EA         (DVP_FS_SBLIST_SA + DVP_FS_SBLIST_SIZE)


#define DVP_FS_WORKING_DIRS_SA   (DVP_FS_TOTAL_CNT_EA)

u32 MapDVPAddr(u32 dwAddr)
{
    u32 dwDvpAddr = GetDvpMemBaseAddr();
    pr_debug("[dvp][drv] MapDVPAddr dwAddr-0x3000000: 0x%08x, dwDvpAddr: 0x%08x",
        dwAddr - 0x03000000, dwDvpAddr);
    return changePhyToVirtualAddress(dwDvpAddr + (dwAddr - 0x03000000));
}

/**
 *  FS::commit_change
 *
 *  DVP Ext name is ASCII, if file name is UNICODE,
 *    Translate Ext name to UNICODE
 *
 */
static void ParserDVPFileName(u8 *pbDest, u32 bDesLen,
            const u8 *pbSrc, u32 bSrcLen, u8 bIsUnicode)
{
    u32 u4SrcExtStart = 0;
    u32 u4ExtNameLen = 0;
    u32 u4DesExtStart = 0;

    if (bSrcLen > bDesLen)
        bSrcLen = bDesLen;
    memset((VOID *)pbDest, 0, bDesLen);

    if (!bIsUnicode) {
        memcpy((void *)pbDest, pbSrc, bSrcLen);
        return;
    }
    /*Covent Ext Name to Unicode String */
    /*Get Ext Name length */
    for (u4SrcExtStart = bSrcLen - 1, u4ExtNameLen = 0;
        u4SrcExtStart > 0;
        u4SrcExtStart--, u4ExtNameLen++) {
        if (pbSrc[u4SrcExtStart] == '.') {
            u4ExtNameLen++;
            /*Check Des Buffer Length, if is not enough,
              cut down main file name */
            u4DesExtStart = u4SrcExtStart;
            if ((u4ExtNameLen * 2) > (bDesLen - u4SrcExtStart)) {
                if ((u4ExtNameLen * 2) > (bDesLen)) {
                    u4ExtNameLen = 0;
                    break;
                } else {
                    u4DesExtStart =
                        bDesLen -
                        (u4ExtNameLen * 2) - 1;
                }
            }

            /* adjust remain name length */
            bSrcLen = u4DesExtStart;

            /* copy ext name */
            while (u4ExtNameLen--) {
                pbDest[u4DesExtStart] = '\0';
                u4DesExtStart++;
                pbDest[u4DesExtStart] =
                    pbSrc[u4SrcExtStart];
                u4DesExtStart++;
                u4SrcExtStart++;
            }
            break;
        }
    }

    /*Copy File Name, excpet Ext Name if Jolietlevel is not 0  */
    memcpy((void *)pbDest, pbSrc, bSrcLen);

    return;
}

static struct DFileTable *DFileTableMaxLenInit(const u32 u4MaxLen)
{
    u32 u4MaxTabL = u4MaxLen;
    struct DFileTable *pFileTable = NULL;

    pFileTable = (struct DFileTable *)vmalloc(sizeof(struct DFileTable));
    if (NULL == pFileTable) {
        pr_err("[dvp][drv] DFileTableMaxLenInit fail vmalloc DFileTable FAIL:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return NULL;
    }

    pFileTable->m_u4TableLen = 0;
    pFileTable->m_u4MaxLen = 0;
    pFileTable->m_pItemTable = NULL;

    if (u4MaxTabL > MAX_FILE_NUM)
        u4MaxTabL = MAX_FILE_NUM;

    if (u4MaxTabL > 0) {
        pFileTable->m_pItemTable = (struct FileItem *)vmalloc  \
            (u4MaxTabL * sizeof(struct FileItem));
        if (NULL == pFileTable->m_pItemTable) {
            pr_err("[dvp][drv] DFileTableMaxLenInit fail vmalloc m_pItemTable FAIL:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            vfree(pFileTable);
            pFileTable = NULL;
            return NULL;
        }
        memset(pFileTable->m_pItemTable, 0,
            u4MaxTabL * sizeof(struct FileItem));
        pFileTable->m_u4MaxLen = u4MaxTabL;
    }
    return pFileTable;
}

static void DFileTableDeInit(struct DFileTable *pFileTable)
{
    if (NULL != pFileTable) {
        if (NULL != pFileTable->m_pItemTable) {
            vfree(pFileTable->m_pItemTable);
            pFileTable->m_pItemTable = NULL;
            pFileTable->m_u4TableLen = 0;
            pFileTable->m_u4MaxLen = 0;
        }
        vfree(pFileTable);
        pFileTable = NULL;
    }
}

static bool DFileTableReadFileInfo(struct DFileTable *pFileTable,
    struct DVPFsFileItem *pFileItem, const u32 u4ReadIndex)
{
    u32 dwAddr = 0;
    struct FS_FILE_INS *pFileInf = NULL;
    if ((NULL == pFileTable) ||
        (NULL == pFileTable->m_pItemTable) ||
        (NULL == pFileItem) ||
        (u4ReadIndex >= pFileTable->m_u4TableLen)) {
        return false;
    }

    dwAddr = pFileTable->m_pItemTable[u4ReadIndex].u4FileAddr;
    pFileInf = (struct FS_FILE_INS *)dwAddr;

    pr_debug("[dvp][drv] DFileTableReadFileInfo: pFileInf = 0X%08X  len = %d\r\n",
        (u32)pFileInf, pFileInf->bNLen);

    pFileItem->uFType = pFileInf->eFType;
    pFileItem->uCodeType = (pFileInf->pbName[0] == 0x10) ? \
        CODEC_UNICODE : CODEC_OTHER;
    pr_debug("[dvp][drv] DFileTableReadFileInfo: pFileItem->uFType = %d \r\n",
        pFileItem->uFType);

    ParserDVPFileName(pFileItem->szName, FS_RAM_CODE_MAX_FILENAME_BUF_SIZE,
            pFileInf->pbName, pFileInf->bNLen,
            (pFileItem->uCodeType == CODEC_UNICODE) ? TRUE : FALSE);
    return TRUE;
}

static bool DFileTableAddFileInfo(struct DFileTable *pFileTable,
    struct FileItem FileInfo)
{
    if ((NULL == pFileTable) ||
        (NULL == pFileTable->m_pItemTable)) {
        return FALSE;
    }

    if (pFileTable->m_u4TableLen >= pFileTable->m_u4MaxLen)
        return FALSE;

    pFileTable->m_pItemTable[pFileTable->m_u4TableLen].u4FileAddr =
        FileInfo.u4FileAddr;
    pFileTable->m_pItemTable[pFileTable->m_u4TableLen].uCodeType =
        FileInfo.uCodeType;

    pFileTable->m_u4TableLen++;

    pr_debug("[dvp][drv] DFileTableAddFileInfo: m_eType = %d m_u4MaxLen = %d \r\n",
        pFileTable->m_eType, pFileTable->m_u4MaxLen);

    return TRUE;
}

static bool DFileTableSetFileType(struct DFileTable *pFileTable,
    enum E_FILE_FILTER eType)
{
    pFileTable->m_eType = eType;
    return TRUE;
}


void DFileSysIOInit(void)
{
    g_cFSIO.m_pAudioTable = NULL;
    g_cFSIO.m_pVideoTable = NULL;
    g_cFSIO.m_pPicTable = NULL;
    g_cFSIO.m_dwBaseAddr = 0;
    g_cFSIO.m_dwDVPSupportFile = 0;
    g_cFSIO.m_dwVideoFilter = 0;
    g_cFSIO.m_dwAudioFilter = 0;
    g_cFSIO.m_dwPicFilter = 0;
    g_cFSIO.m_u2WorkDir = 0;
    g_cFSIO.m_bValid = FALSE;
    g_cFSIO.m_u2Codec = CODEC_UNICODE;
    g_cFSIO.m_eFilter = FILE_FILTER_ALL;
}

void DFileSysIODeInit(void)
{
    DeInitTable();
    g_cFSIO.m_dwBaseAddr = 0;
    g_cFSIO.m_dwDVPSupportFile = 0;
    g_cFSIO.m_dwVideoFilter = 0;
    g_cFSIO.m_dwAudioFilter = 0;
    g_cFSIO.m_dwPicFilter = 0;
    g_cFSIO.m_u2WorkDir = 0;
    g_cFSIO.m_bValid = FALSE;
    g_cFSIO.m_u2Codec = CODEC_UNICODE;
    g_cFSIO.m_eFilter = FILE_FILTER_ALL;
}

bool ReadFileInfo(struct DVPFsFileItem *pFileItem, u32 u4ReadIndex)
{
    bool bRet = FALSE;
    if (g_cFSIO.m_bValid == FALSE) {
        pr_err("[dvp][drv] ReadFileInfo Valid Flase:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    pr_debug("[dvp][drv] ReadFileInfo u4ReadIndex: %d, m_eFilter = %d\r\n",
        u4ReadIndex, g_cFSIO.m_eFilter);

    switch (g_cFSIO.m_eFilter) {
    case FILE_FILTER_ALL:
        bRet = ReadAllFileInfo(pFileItem, u4ReadIndex);
        break;

    case FILE_FILTER_AUDIO:
        if (NULL != g_cFSIO.m_pAudioTable)
            bRet = DFileTableReadFileInfo(g_cFSIO.m_pAudioTable,
            pFileItem, u4ReadIndex);
        break;

    case FILE_FILTER_VIDEO:
        if (NULL != g_cFSIO.m_pVideoTable)
            bRet = DFileTableReadFileInfo(g_cFSIO.m_pVideoTable,
            pFileItem, u4ReadIndex);
        break;

    case FILE_FILTER_PIC:
        if (NULL != g_cFSIO.m_pPicTable)
            bRet = DFileTableReadFileInfo(g_cFSIO.m_pPicTable,
            pFileItem, u4ReadIndex);
        break;

    default:
        bRet = FALSE;
        break;

    }

    return bRet;
}


bool ReadAllFileInfo(struct DVPFsFileItem *pFileItem, u32 u4ReadIndex)
{
    bool bRet = FALSE;
    u32 dwFileNameAddr = 0;
    u16 u4Cout = *((u16 *)DVP_FS_COUT_DIR_SA);
    struct FS_DIR *pFsDir = (struct FS_DIR *)(DVP_FS_WORKING_DIRS_SA);

    u32 u4I = 0;
    u32 u4CurIdx = 0;
    struct FS_FILE_INS *pFileInf = NULL;
    /*FileItem rFileInfo = {0}; */

    if (0 == g_cFSIO.m_dwBaseAddr || NULL == pFileItem)
        return FALSE;
    u4ReadIndex = u4ReadIndex + 2; /*ignore "." and ".." */

    pr_debug("[dvp][drv] ReadAllFileInfo m_u4WorkDir: %d\r\n",
        g_cFSIO.m_u2WorkDir);

    if (g_cFSIO.m_u2WorkDir >= u4Cout || u4ReadIndex >=
        pFsDir[g_cFSIO.m_u2WorkDir].rFileList.wCnt)
        return FALSE;

    pFileInf = (struct FS_FILE_INS *)MapDVPAddr  \
        ((u32)(pFsDir[g_cFSIO.m_u2WorkDir].rFileList.prList));

    u4CurIdx = 1;

    for (u4I = 2; u4I < pFsDir[g_cFSIO.m_u2WorkDir].rFileList.wCnt; u4I++) {
        if (pFileInf[u4I].eFType == FS_FTYPE_DIR &&
            IsVaildDir(pFileInf[u4I].dwLBA)) {
            u4CurIdx++;

            if (u4CurIdx == u4ReadIndex) {
                memcpy(pFileItem->szName, pFileInf[u4I].pbName,
                    pFileInf[u4I].bNLen);
                pFileItem->szName[pFileInf[u4I].bNLen] = '\0';
                pFileItem->uFType = pFileInf[u4I].eFType;
                pFileItem->uCodeType =
                    (pFileItem->szName[0] == 0x10) \
                    ? CODEC_UNICODE : CODEC_OTHER;
                return TRUE;
            }
        }
    }

    for (u4I = 2; u4I < pFsDir[g_cFSIO.m_u2WorkDir].rFileList.wCnt; u4I++) {
        if (IsSupportFile(pFileInf[u4I].eFType)) {
            if (pFileInf[u4I].eFType == FS_FTYPE_DIR)
                continue;
            u4CurIdx++;

            if (u4CurIdx == u4ReadIndex) {
                pFileItem->uFType = pFileInf[u4I].eFType;
                pFileItem->uCodeType =
                    (pFileInf[u4I].pbName[0] == 0x10) ?  \
                    CODEC_UNICODE : CODEC_OTHER;
                ParserDVPFileName(pFileItem->szName,
                    FS_RAM_CODE_MAX_FILENAME_BUF_SIZE,
                    pFileInf[u4I].pbName,
                    pFileInf[u4I].bNLen,
                    (pFileItem->uCodeType ==
                    CODEC_UNICODE) ? TRUE : FALSE);
                return TRUE;
            }
        }
    }

    pr_debug("[dvp][drv] ReadAllFileInfo: pFileInf = 0X%08X \r\n",
        (u32)pFileInf);

    pFileItem->uFType = pFileInf[u4ReadIndex].eFType;
    dwFileNameAddr = (u32)(pFileInf[u4ReadIndex].pbName);

    memcpy(pFileItem->szName, (BYTE *)dwFileNameAddr,
        FS_RAM_CODE_MAX_FILENAME_BUF_SIZE);
    pFileItem->uCodeType =
        (pFileItem->szName[0] == 0x10) ? CODEC_UNICODE : CODEC_OTHER;
    pr_debug("[dvp][drv] ReadAllFileInfo pFileItem->szName %s\r\n",
        pFileItem->szName);

    bRet = TRUE;

    return bRet;
}

bool SetFsValid(u32 dwFsAddr, u32 dwFilter)
{
    bool bRet = FALSE;

    g_cFSIO.m_dwBaseAddr = MapDVPAddr(dwFsAddr);
    g_cFSIO.m_dwDVPSupportFile = dwFilter;

    pr_debug("[dvp][drv] SetFsValid m_dwBaseAddr = 0X%08X, m_dwDVPSupportFile = 0X%08X \r\n",
        g_cFSIO.m_dwBaseAddr, g_cFSIO.m_dwDVPSupportFile);

    g_cFSIO.m_dwVideoFilter = *((u32 *)DVP_FS_FILTER_VIDEO_SA);
    g_cFSIO.m_dwAudioFilter = *((u32 *)DVP_FS_FILTER_AUDIO_SA);
    g_cFSIO.m_dwPicFilter   = *((u32 *)DVP_FS_FILTER_PIC_SA);

    pr_debug("[dvp][drv] SetFsValid m_dwVideoFilter = 0X%08X \r\n",
        g_cFSIO.m_dwVideoFilter);
    pr_debug("[dvp][drv] SetFsValid: m_dwAudioFilter = 0X%08X \r\n",
        g_cFSIO.m_dwAudioFilter);
    pr_debug("[dvp][drv] SetFsValid: m_dwPicFilter = 0X%08X \r\n",
        g_cFSIO.m_dwPicFilter);

    bRet = InitTable();
    g_cFSIO.m_bValid = bRet;

    return bRet;
}

bool SetFsInvalid(void)
{
    bool bRet = FALSE;

    g_cFSIO.m_bValid = FALSE;

    g_cFSIO.m_dwBaseAddr = 0;

    bRet = DeInitTable();

    return bRet;
}

bool InitTable(void)
{
    struct FS_SB  *pFsSb  = NULL;
    u16 u4I = 0, u4J = 0;
    struct FS_FILE_INS *pFileInf = NULL;
    struct FileItem rFileInfo = {0};
    u8 mFileType = 0;

    u16 u4CoutDir = *((u16 *)DVP_FS_COUT_DIR_SA);
    u16 u4CoutVideo = *((u16 *)(DVP_FS_COUT_VIDEO_SA));
    u16 u4CoutAudio = *((u16 *)(DVP_FS_COUT_AUDIO_SA));
    u16 u4CoutPic = *((u16 *)(DVP_FS_COUT_PIC_SA));
    struct FS_DIR *pFsDir = (struct FS_DIR *)(DVP_FS_WORKING_DIRS_SA);
    u32 dwFsSbAddr = *((u32 *)(DVP_FS_SBLIST_SA));

    if (0 == g_cFSIO.m_dwBaseAddr) {
        pr_err("[dvp][drv] InitTable fail  m_dwBaseAddr is 0 :[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    pFsSb = (struct FS_SB *)MapDVPAddr(dwFsSbAddr);

    pr_debug("[dvp][drv] DVP_FS_SBLIST_SA = 0X%08X \r\n", dwFsSbAddr);
    pr_debug("[dvp][drv] InitTable u4CoutDir= %d, u4CoutVideo = %d, u4CoutAudio = %d, u4CoutPic = %d \r\n",
        u4CoutDir, u4CoutVideo, u4CoutAudio, u4CoutPic);

    if (NULL != g_cFSIO.m_pAudioTable ||
        NULL != g_cFSIO.m_pVideoTable ||
        NULL != g_cFSIO.m_pPicTable) {
        pr_err("[dvp][drv] before init have to DeInitTable NULL:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        DeInitTable();
    }

    g_cFSIO.m_pAudioTable = DFileTableMaxLenInit(u4CoutAudio);
    g_cFSIO.m_pVideoTable = DFileTableMaxLenInit(u4CoutVideo);
    g_cFSIO.m_pPicTable = DFileTableMaxLenInit(u4CoutPic);

    if ((NULL == g_cFSIO.m_pAudioTable) ||
       (NULL == g_cFSIO.m_pVideoTable) ||
       (NULL == g_cFSIO.m_pPicTable)) {
        DeInitTable();
        pr_debug("[dvp][drv] InitTable FAIL\r\n");
        return FALSE;
    }

    DFileTableSetFileType(g_cFSIO.m_pAudioTable, FILE_FILTER_AUDIO);
    DFileTableSetFileType(g_cFSIO.m_pVideoTable, FILE_FILTER_VIDEO);
    DFileTableSetFileType(g_cFSIO.m_pPicTable, FILE_FILTER_PIC);

    for (u4I = 0; u4I < u4CoutDir; u4I++) {
        pr_debug("[dvp][drv] InitTable: pFsDir[u4I].rFileList.wCnt = %d \r\n",
            pFsDir[u4I].rFileList.wCnt);
        rFileInfo.uCodeType =
            (pFsSb[u4I].wExt > 0 ? CODEC_UNICODE  : CODEC_OTHER);
        for (u4J = 0; u4J < pFsDir[u4I].rFileList.wCnt; u4J++) {
            pFileInf =
                (struct FS_FILE_INS *)MapDVPAddr  \
                ((u32)(pFsDir[u4I].rFileList.prList));
            rFileInfo.u4FileAddr = (u32)(&(pFileInf[u4J]));

            pr_debug("[dvp][drv] InitTable: pFileInf[%d].eFType %d \r\n",
                u4J, pFileInf[u4J].eFType);

            pr_debug("[dvp][drv] ReadAllFileInfo: pFileInf[%d].u4FileAddr = 0X%08X \r\n",
                u4J, rFileInfo.u4FileAddr);

            pr_debug("[dvp][drv] ReadAllFileInfo: pFileInf = 0X%08X \r\n",
                (u32)(&pFileInf[u4J]));

            mFileType = pFileInf[u4J].eFType;
            if (IsSupportFile(mFileType)) {
                if (FFillsKept(mFileType,
                    g_cFSIO.m_dwPicFilter))
                    DFileTableAddFileInfo(
                        g_cFSIO.m_pPicTable,
                        rFileInfo);
                else if (FFillsKept(mFileType,
                    g_cFSIO.m_dwVideoFilter))
                    DFileTableAddFileInfo(
                    g_cFSIO.m_pVideoTable,
                        rFileInfo);
                else if (FFillsKept(mFileType,
                    g_cFSIO.m_dwAudioFilter))
                    DFileTableAddFileInfo(
                        g_cFSIO.m_pAudioTable,
                        rFileInfo);
            } else {
                pr_debug("[dvp][drv] InitTable: Nor Support pFileInf[%d].eFType %d \r\n",
                    u4J, pFileInf[u4J].eFType);
            }

        }

    }

    return TRUE;
}


bool DeInitTable(void)
{
    pr_debug("[dvp][drv] DeInitTable\r\n");

    if (NULL != g_cFSIO.m_pAudioTable) {
        DFileTableDeInit(g_cFSIO.m_pAudioTable);
        g_cFSIO.m_pAudioTable = NULL;
    }

    if (NULL != g_cFSIO.m_pVideoTable) {
        DFileTableDeInit(g_cFSIO.m_pVideoTable);
        g_cFSIO.m_pVideoTable = NULL;
    }

    if (NULL != g_cFSIO.m_pPicTable) {
        DFileTableDeInit(g_cFSIO.m_pPicTable);
        g_cFSIO.m_pPicTable = NULL;
    }

    return TRUE;

}


bool IsSupportFile(BYTE bFilter)
{
    u32 dwMask = 0;

    dwMask = 0x00000001 << bFilter;
    if (g_cFSIO.m_dwDVPSupportFile & dwMask)
        return TRUE;
    else
        return FALSE;
}

bool FFillsKept(BYTE bFType, u32 dwFilter)
{
    u32 dwMask = 0;

    /*1. check global file filter first */
    if (!IsSupportFile(bFType))
        return FALSE;

    /*2. check with the given filter factor */
    dwMask = 0x00000001 << bFType;
    if (dwFilter & dwMask)
        return TRUE;

    return FALSE;
}


bool SetFilter(enum E_FILE_FILTER eFilter)
{
    g_cFSIO.m_eFilter = eFilter;

    pr_debug("[dvp][drv] SetFilter eFilter: %d\r\n", eFilter);

    return TRUE;
}


bool SetWorkDir(u16 u2DirIndex)
{
    g_cFSIO.m_u2WorkDir = u2DirIndex;

    pr_debug("[dvp][drv] SetWorkDir m_u2WorkDir: %d\r\n", u2DirIndex);

    return TRUE;
}

bool GetFsValid(void)
{
    return g_cFSIO.m_bValid;
}


u32 GetWorkDir(void)
{
    return g_cFSIO.m_u2WorkDir;
}

enum E_FILE_FILTER GetFilter(void)
{
    return g_cFSIO.m_eFilter;
}

bool SetFLCodec(u8 u2Codec)
{
    g_cFSIO.m_u2Codec = u2Codec;

    pr_debug("[dvp][drv] SetFLCodec DVP_CMD_FL_CODEC %s\r\n",
        (g_cFSIO.m_u2Codec == CODEC_UNICODE) ? ("Unicode") : ("Local"));
    return TRUE;
}

bool IsVaildDir(const u32 dwLBA)
{
    bool bRet = FALSE;
    struct FS_SB *prSbList = NULL;
    u16 i2I = 0;
    u16 u4Cout = *((u16 *)DVP_FS_COUT_DIR_SA);
    u32 dwAddr = *((u32 *)(DVP_FS_SBLIST_SA));

    prSbList = (struct FS_SB *)MapDVPAddr(dwAddr);

    for (i2I = g_cFSIO.m_u2WorkDir; i2I < u4Cout; i2I++) {
        if (dwLBA == prSbList[i2I].dwLba) {
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}

u32 DVPFs_GetPathByIndex(u16 u2Index, s8 *pPathBuf, u32 u4BufSize)
{
    u32 u4LeftBufsize = 0;
    u16 u2DirNameLen = 0;
    s8 wStr[MAX_PATH + 1] = {0};
    s8 wPathStr[MAX_PATH + 1] = {0};
    u32 dwAddr = *((u32 *)(DVP_FS_SBLIST_SA));
    struct FS_SB *prSbList = NULL;
    u8 *pu1Buf = NULL;
    u8 u1DirBufLen = 0;
    u8 u1Codec = CODEC_UNICODE;
    u32 u4Idx = 0;
    u32 u4StrLen = 0;

    u4LeftBufsize = (u4BufSize > MAX_PATH) ? (MAX_PATH) : (u4BufSize);

    prSbList = (struct FS_SB *)MapDVPAddr(dwAddr);

    if (u4LeftBufsize > 0) {
        u4LeftBufsize--;
        wPathStr[u4LeftBufsize] = '\\';
    }

    while (u2Index != 0 && u4LeftBufsize > 0) {
        pu1Buf = (u8 *)MapDVPAddr((u32)(prSbList[u2Index].pbId));

        u2Index = prSbList[u2Index].wParent;

        if (NULL != pu1Buf) {
            u1DirBufLen = pu1Buf[0];
            u1Codec = pu1Buf[1];

            u1Codec =
                (u1Codec == 0x10) ? CODEC_UNICODE : CODEC_OTHER;

            if (CODEC_UNICODE == u1Codec)
                u1DirBufLen = u1DirBufLen - (u1DirBufLen % 2);
            #if 1
            pr_debug("[dvp][drv][PATH] u1Codec:%d u1DirBufLen:%d\r\n",
                u1Codec, u1DirBufLen);

            pr_debug("\r\n[dvp][drv][PATH] ++++++++++++++++ dump s ++++++++++++++++\r\n");
            pr_debug("0x ");
            for (u4Idx = 0; u4Idx < u1DirBufLen; u4Idx++) {
                pr_debug("%2x ", *(pu1Buf + 2 + u4Idx));

                if (((u4Idx + 1) % 4) == 0)
                    pr_debug(" ");
                if (((u4Idx + 1) % 16) == 0)
                    pr_debug("\r\n");
            }
            pr_debug("\r\n");
            pr_debug("[dvp][drv][PATH] ++++++++++++++++ dump e ++++++++++++++++\r\n");
            #endif

            if (CODEC_OTHER == u1Codec) {
                u2DirNameLen =
                    OtherCodeToUtf8((pu1Buf + 2),
                    u1DirBufLen, wStr);
                u2DirNameLen--;
            } else {
                u2DirNameLen =
                    BigEndiaDataUcs2ToUtf8((pu1Buf + 2),
                    u1DirBufLen, wStr);
            }

            while (u2DirNameLen && u4LeftBufsize > 0) {
                u2DirNameLen--;
                u4LeftBufsize--;
                wPathStr[u4LeftBufsize] = wStr[u2DirNameLen];
            }

            if (u4LeftBufsize > 0) {
                u4LeftBufsize--;
                wPathStr[u4LeftBufsize] = '\\';
            }

        }
    }

    u4StrLen = 0;
    for (u4Idx = 0; u4Idx < u4BufSize && u4LeftBufsize < u4BufSize;
        u4Idx++, u4LeftBufsize++) {
        pPathBuf[u4Idx] = wPathStr[u4LeftBufsize];
        u4StrLen++;
    }

    pPathBuf[u4Idx] = '\0';
    u4StrLen++;

    #if 1
    pr_debug("[dvp][drv][PATH] u4StrLen:%d \r\n", u4StrLen);

    pr_debug("\r\n[dvp][drv][PATH] ++++++++++++++++ dump res s ++++++++++++++++\r\n");
    pr_debug("0x ");
    for (u4Idx = 0; u4Idx < u4StrLen; u4Idx++) {
        pr_debug("%2x ", *(pPathBuf + u4Idx));

        if (((u4Idx + 1) % 4) == 0)
            pr_debug(" ");
        if (((u4Idx + 1) % 16) == 0)
            pr_debug("\r\n");
    }
    pr_debug("\r\n");
    pr_debug("[dvp][drv][PATH] ++++++++++++++++ dump res e ++++++++++++++++\r\n");
    #endif
    return u4StrLen;
}


