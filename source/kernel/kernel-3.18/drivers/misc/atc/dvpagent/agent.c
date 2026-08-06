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
* file agent.c
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
*
*******************************************************************************/
#include "agent.h"
#include "aud_ioctrl.h"
#include "dvp_cmd.h"
#include "agent_drv.h"
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <ceddk.h>
#include "chip_ver.h"
#include "dvp_fsIo.h"
#include "metazone.h"
#include "dvp_mod.h"
#include <stdarg.h>

#include <mach/pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <linux/fs.h>
#include <semaphore.h>
#include <mutex.h>
#include <asm/io.h>
#include <errno.h>

static bool DVPHost_Init(void);
static s32 ReadThread(void *pArg);

#define DVP_SEMA_NAME          "DVP_SEMA"
#define MAX_DVP_FILEITEM_NUM   (32)
#define OK 0

//#undef    MEMRSV_PHY_TO_VIRT
//#define MEMRSV_PHY_TO_VIRT(x) (x)

static u16          szMP3Path[MAX_PATH + 1];
static u16          szMP3RootDir[MAX_PATH + 1];
static u32          g_u4TrkLen = (u32)-1;
//static HANDLE_T     *g_hReadThread;  /*read DVP info */
static struct task_struct *dvpThread;
static u32          dwRipDataAddr;
//static u32            g_dwReadThreadID;
static bool         g_fgMediaExist;
//HANDLE                g_critEvtLock;
struct semaphore    *dvpsem;
struct mutex        dvplock;

static bool         g_fgReadThread;
static bool         g_fgDeviceEnable;

#define DVP_READ_STACK_SIZE       16384
#define DVP_READ_PRIORITY         197      /* when priority is 23 */
#define INVALID_HANDLE_VALUE      ((HANDLE)-1)
#define DRM_PACKED_ALLOCATION_BYTES         (49) /* Divx 3.1HT */

extern struct resvd_mem_info dvp_share_rsv_mem;
extern struct resvd_mem_info dsp_share_rsv_mem;

struct DVPINFO_T {
    s32         fgIsDiscInsert;
    u16         szCurrentDir[MAX_PATH + 1];
    u16         szCurrentFilePath[MAX_PATH + 1];
    u16         u2TotalFileCount;
    s16         i2StartIdx;
    s8          iStart;
    s8          iEnd;
    s32         *pi4RipTrkLbaLen;
    u8          uCSType;
    s32         fgPT110Log;
    s32         fg8032Log;
    u32         dwFileInfoDramBaseAddress;
    u32         u4CodePage;
    struct DVP_FILEITEM_INFO_T    rFileItems[MAX_DVP_FILEITEM_NUM];
};

struct DVP_TIMECMD_T {
    struct AP2DVPPACKET_T  rCmd;
    u32                    u4Time;
};

static u8    *puLyricBuf;
static u32   u4LyricLen;
static u8    *puId3PicBuf;
static u32   u4Id3PicLen;

#define MAX_DVP_TIMECMD_NUM      5
static struct DVP_TIMECMD_T *g_prTimeCmds[MAX_DVP_TIMECMD_NUM];
static struct DVPINFO_T g_rDVPInfo;

#define DVP_VMALLOC(X)  dvp_vmalloc(X, __func__, __LINE__)

void *dvp_vmalloc(u32 u4Size, const s8 *pFunName, u32 u4Line)
{
    if (u4Size == 0) {
        pr_debug("[dvp][drv] [malloc] u4Size is zero, fun:%s, line:%d\r\n",
            pFunName, u4Line);
        return NULL;
    }
    return vmalloc(u4Size);
}

u32 GetDvpMemBaseAddr(void)
{
    return (*(volatile u32 *)(IO_BASE_VA + DVP_MEM_BANK_REG)) << 24;
}

u32 changePhyToVirtualAddress(u32 address)
{
    return (((u32)(dvp_share_rsv_mem.virt_addr)) + (address - dvp_share_rsv_mem.base));
}

u32 getDvpBaseVirtualAddress(void)
{
    return (u32)(dvp_share_rsv_mem.virt_addr);
}

static u16 SpecMemCopy(u8 *puData, u16 u2DataLen, u8  *szStr)
{
    u16   i = 0;
    u16   j = 0;

    memcpy(szStr, (const void *)puData, u2DataLen);
    while (i < u2DataLen) {
        if ((0 == szStr[i]) && (0 == szStr[i+1])) {
            j = i;
            while (j < u2DataLen) {
                szStr[i] = 0;
                j++;
            }
            break;
        }
        i++;
    }
    return i;
}

u16 BigEndiaDataUcs2ToUtf8(u8 *puData, u16 u2DataLen, u8 *szStr)
{
    u16  i = 0;
    u16  j = 0;
    u32 u2Ucs2code = 0;

    while ((i + 1) < u2DataLen) {
        if ((0 == puData[i]) && (0 == puData[i+1]))
            break;

        u2Ucs2code = puData[i];
        u2Ucs2code = (u2Ucs2code << 8) | puData[i+1];

        if (u2Ucs2code < 0x80) {
            szStr[j] = u2Ucs2code&0x7F;
            j++;
        } else if (u2Ucs2code < 0x800) {
            szStr[j] = 0xC0 | ((u2Ucs2code>>6)&0x3F);
            j++;
            szStr[j] = 0x80 | (u2Ucs2code&0x1F);
            j++;
        } else if (u2Ucs2code < 0x10000) {
            szStr[j] = 0xE0 | ((u2Ucs2code>>12)&0x0F);
            j++;
            szStr[j] = 0x80 | ((u2Ucs2code>>6)&0x3F);
            j++;
            szStr[j] = 0x80 | (u2Ucs2code&0x3F);
            j++;
        }
        i += 2;
    }
    return j;
}

u16 BigEndiaDataUcs4ToUtf8(u8 *puData, u16 u2DataLen, u8 *szStr)
{
    u16  i = 0;
    u16  j = 0;
    u32 u2Ucs2code = 0;

    while ((i+4) < u2DataLen) {
        if ((0 == puData[i+2]) && (0 == puData[i+3]))
            break;

        u2Ucs2code = puData[i+2];
        u2Ucs2code = (u2Ucs2code << 8) | puData[i+3];

        if (u2Ucs2code < 0x80) {
            szStr[j] = u2Ucs2code&0x7F;
            j++;
        } else if (u2Ucs2code < 0x800) {
            szStr[j] = 0xC0 | ((u2Ucs2code>>6)&0x3F);
            j++;
            szStr[j] = 0x80 | (u2Ucs2code&0x1F);
            j++;
        } else if (u2Ucs2code < 0x10000) {
            szStr[j] = 0xE0 | ((u2Ucs2code>>12)&0x0F);
            j++;
            szStr[j] = 0x80 | ((u2Ucs2code>>6)&0x3F);
            j++;
            szStr[j] = 0x80 | (u2Ucs2code&0x3F);
            j++;
        }

        i += 4;
    }
    return j;
}

/* 0 --other(UTF8,UTF16....), 1---ucs2 , 2---ucs4(utf32)   */
u16 OtherCodeToUtf8(u8 *puData, u16 u2DataLen, u8 *szStr)
{
    u16 u2StrLen;
    u16 u2CodeType = 0;

    if (u2DataLen >= 3) {
        if (0 == puData[0]) {
            if (0 == puData[1]) {
                pr_debug("[dvp][drv] current puData[0] is %d, puData[1] is %d\r\n",
                    puData[0], puData[1]);
                u2CodeType = 2; /* ucs4  or UTF32 */
            } else {
                 u2CodeType = 1; /* UCS2  */
            }
        } else {
            if ((puData[0] < 0x80) && (puData[1] < 0x80)) {
                u2CodeType = 0; /*UTF8  */
            } else {
                if ((((puData[0]&(0xc0)) == 0x80) &&
                    ((puData[1]&(0xe0)) == 0xc0)) ||
                (((puData[0]&(0xc0)) == 0x80) &&
                ((puData[1]&(0xc0)) == 0x80) &&
                ((puData[2]&(0xe0)) == 0xc0))) {
                    u2CodeType = 0;
                } else {
                    u2CodeType = 1;
                }
            }
            u2CodeType = 0;
        }
    } else {
    u2CodeType = 0;
    }
    switch (u2CodeType) {
    case 1:
        u2StrLen = BigEndiaDataUcs2ToUtf8(puData, u2DataLen, szStr);
        break;

    case 2:
        u2StrLen = BigEndiaDataUcs4ToUtf8(puData, u2DataLen, szStr);
        break;

    case 0:

    default:
        u2StrLen = SpecMemCopy(puData, u2DataLen, szStr);
        break;
    }
    return u2StrLen;
}

bool DVPAgent_Init(void)
{
    s8  i;
    pr_info("[dvp][drv] DVPAgent_Init. \r\n");
    if (!DVPHost_Init()) {
        pr_err("[dvp][drv] DVPHost_Init fail[%s %s %d]\r\n", FILE_ONLY, __func__, __LINE__);


        return 0;
    }

    g_rDVPInfo.fgIsDiscInsert = FALSE;
    g_rDVPInfo.szCurrentDir[0] = _T('\0');
    g_rDVPInfo.szCurrentFilePath[0] = _T('\0');
    g_rDVPInfo.u2TotalFileCount = 0;
    g_rDVPInfo.i2StartIdx = -1;
    g_rDVPInfo.iStart = 0;
    g_rDVPInfo.iEnd = -1;
    g_rDVPInfo.pi4RipTrkLbaLen = NULL;
    for (i = 0; i < MAX_DVP_FILEITEM_NUM; i++)
        memset(&g_rDVPInfo.rFileItems[i], 0,
            sizeof(struct DVP_FILEITEM_INFO_T));
    g_rDVPInfo.uCSType = CODEC_UNICODE;
    g_rDVPInfo.fgPT110Log = FALSE;
    g_rDVPInfo.fg8032Log = FALSE;
    g_rDVPInfo.dwFileInfoDramBaseAddress = 0;
    g_rDVPInfo.u4CodePage = 936;
    g_fgMediaExist = FALSE;   /* for no EMI hardware protect*/
    memset(szMP3Path, 0, sizeof(u16)*(MAX_PATH + 1));
    for (i = 0; i < MAX_DVP_TIMECMD_NUM; i++)
        g_prTimeCmds[i] = NULL;

    if (!g_fgReadThread) {
        g_fgReadThread = TRUE;
        dvpThread = kthread_create_on_node(ReadThread, NULL, NUMA_NO_NODE, "DVP Read Thread");
        if (IS_ERR(dvpThread)) {
            pr_err("[dvp][drv] Create thread fail! [%s %s %d]\r\n", FILE_ONLY, __func__, __LINE__);
            dvpThread = NULL;
            g_fgReadThread = FALSE;
            return FALSE;
        }
        wake_up_process(dvpThread);
    }

    pr_info("[dvp][drv] DVPAgent_Init function is OK. \r\n");
    return TRUE;
}

void DVPAgent_Deinit(void)
{
    pr_info("[dvp][drv] DVPAgent_Deinit function in OK.\r\n");

    g_fgReadThread = FALSE;
    DVPHost_Deinit();

    if (dvpThread) {
        kthread_stop(dvpThread);
        dvpThread = NULL;
    }

    if (g_rDVPInfo.pi4RipTrkLbaLen) {
        pr_debug("[dvp][drv] DVPAgent_Deinit vfree\r\n");
        vfree(g_rDVPInfo.pi4RipTrkLbaLen);
        g_rDVPInfo.pi4RipTrkLbaLen = NULL;
    }

    if (puLyricBuf) {
        vfree(puLyricBuf);
        puLyricBuf = NULL;
        u4LyricLen = 0;
    }

    if (puId3PicBuf) {
        vfree(puId3PicBuf);
        puId3PicBuf = NULL;
        u4Id3PicLen = 0;
    }
}

static bool DVPHost_Init(void)
{
    pr_info("[dvp][drv] DVPHost_Init function Enter.\r\n");
    if (!DVPComHW_Init()) {
        pr_err("[dvp][drv] DVPHost_Init fail [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return false;
    }

    mutex_init(&dvplock);
    DVPPFs_Init();
    return true;
}

void DVPHost_Deinit(void)
{
    DVPPFs_DeInit();
    DVPComHW_Deinit();
    mutex_destroy(&dvplock);
    pr_info("[dvp][drv] DVPHost_Deinit function in OK.\n");
}

u32 DVPHost_Read(u8 *puData, u32 u4MaxLen, bool *fgFromPT)
{
    u32 rc;
    pr_debug("[dvp][drv] DVPHost_Read function in OK.");
    rc = DVPComHW_ReceiveData(puData, u4MaxLen, fgFromPT);
    return (u32)rc;
}

u32 DVPHost_Write(u8 *puData, u32 u4DataLen)
{
    s32     rc = 0;
    pr_debug("[dvp][drv] DVPHost_Write function in OK.");
    rc = DVPComHW_SendDataPT110(puData, u4DataLen);
    return rc;
}

u32 DVPHost_WritePT110(u8 *puData, u32 u4DataLen)
{
    s32      rc = 0;
    u8       i = 0;
    u8 uBuf[MAX_AP2DVP_LEN];
    u8 bCheckSum = 0;

    if (puData != NULL) {
        struct AP2DVPPACKET_T *pAP2PT = (struct AP2DVPPACKET_T *)puData;
        pr_debug("[dvp][drv] DVPHost_WritePT110, uInstance:");
        pr_debug("0x%x cmd:0x%x uParam1:0x%x uParam2:0x%x uParam3:0x%x uParam4:0x%x\r\n",
            pAP2PT->uInstance,
            pAP2PT->uCmd,
            pAP2PT->uParam1,
            pAP2PT->uParam2,
            pAP2PT->uParam3,
            pAP2PT->uParam4);
    } else {
        pr_debug("[dvp][drv] DVPHost_WritePT110 puData is NULL!\r\n");
    }

    ASSERT(u4DataLen < MAX_AP2DVP_LEN-1);
    if (u4DataLen > MAX_AP2DVP_LEN-2) {
        pr_debug("[dvp][drv] DVPHost_WritePT110 u4DataLen Error.\r\n");
        return 0;
    }
    memset(uBuf, 0, sizeof(u8) * MAX_AP2DVP_LEN);
    pr_debug("[dvp][drv] AP2PT110: FE ");
    uBuf[0] = 0xFE;
    bCheckSum ^= uBuf[0];
    for (i = 0; i < u4DataLen; i++) {
        uBuf[i+1] = puData[i];
        bCheckSum ^= uBuf[i+1];
        pr_debug("%02X ", uBuf[i+1]);
    }
    uBuf[i+1] = bCheckSum;
    pr_debug("%02X \r\n", bCheckSum);
    rc = DVPComHW_SendDataPT110(uBuf, u4DataLen + 2);
    return rc;
}

u32 DVPHost_Write8032(u8 *puData, u32 u4DataLen)
{
    s32     rc = 0;
    u8      i = 0;
    u8      bCheckSum = 0;
    u8      uBuf[MAX_AP2DVP_LEN];

    if (puData != NULL) {
        struct AP2DVPPACKET_T *prPacket =
            (struct AP2DVPPACKET_T *)puData;
        pr_debug("[dvp][drv] DVPHost_Write8032, uInstance:");
        pr_debug("0x%x cmd:0x%x uParam1:0x%x uParam2:0x%x uParam3:0x%x uParam4:0x%x\r\n",
            prPacket->uInstance,
            prPacket->uCmd,
            prPacket->uParam1,
            prPacket->uParam2,
            prPacket->uParam3,
            prPacket->uParam4);
        if (AP_CMD_CHG_DEVICE == prPacket->uCmd)
            g_fgDeviceEnable = (prPacket->uParam1 == 0);
    } else {
        pr_debug("[dvp][drv] DVPHost_Write8032 puData is NULL!\r\n");
    }

    ASSERT(u4DataLen < MAX_AP2DVP_LEN-1);
    if (u4DataLen > MAX_AP2DVP_LEN-2) {
        pr_debug("[dvp][drv] DVPHost_Write8032 u4DataLen Error. \r\n");
        return 0;
    }
    memset(uBuf, 0, sizeof(u8)*MAX_AP2DVP_LEN);
    pr_debug("[dvp][drv] AP28032: FE ");
    uBuf[0] = 0xFE;
    bCheckSum ^= 0xFE;
    for (i = 0; i < u4DataLen; i++) {
        uBuf[i+1] = puData[i];
        bCheckSum ^= puData[i];
        pr_debug("%02X ", uBuf[i+1]);
    }
    uBuf[i+1] = bCheckSum;
    pr_debug("%02X \r\n", bCheckSum);
    rc = DVPComHW_SendData8032(uBuf, u4DataLen+2);

    return rc;
}

void DVPHost_ChangeDeviceIdle(void)
{
    if (g_fgDeviceEnable) {
        struct AP2DVPPACKET_T rPacket;
        pr_debug("[dvp][drv] +++++++++ TO ChangeDeviceIdle +++++++++++\r\n");
        memset(&rPacket, sizeof(rPacket), 0);
        rPacket.uInstance = 1;
        rPacket.uCmd = AP_CMD_CHG_DEVICE;
        rPacket.uParam1 = 0xFF;
        DVPHost_Write8032((u8 *)&rPacket,
            sizeof(struct AP2DVPPACKET_T));
    }
}

static s32 ReadThread(void *pArg)
{
    u8            uDVPData[MAX_DVPDATA_LEN];
    u32           u4DataLen = 0;
    bool          fgFromPT = FALSE;
    pr_debug("[dvp][drv] ReadThread function in OK.");

    while (g_fgReadThread) {
        struct DVP2APPACKET_T   rPacket;

        set_current_state(TASK_UNINTERRUPTIBLE);
        if(kthread_should_stop())
            break;

        memset(uDVPData, 0, sizeof(u8) * MAX_DVPDATA_LEN);
        u4DataLen = DVPHost_Read(uDVPData, MAX_DVPDATA_LEN, &fgFromPT);

        if (u4DataLen == 0)
            continue;

        memset(&rPacket, 0, sizeof(struct DVP2APPACKET_T));

        rPacket.auData[0] = rPacket.auData[1] = 0;
        InputProtocolData(uDVPData, u4DataLen);
        while (ParseNextPacket(&rPacket)) {
            bool   fgSendToQ = TRUE;
            u32    u4Param1, u4Param2;
            u4Param1 = u4Param2 = 0;
            if (fgFromPT) {
                pr_debug("[dvp][drv][cmd]PT110 CMD:0x%x\n",
                    rPacket.uCmd);
                switch (rPacket.uCmd) {
                case PT2AP_RIP_DATA_READY:
                    DVPAgent_getRipAddress(&rPacket);
                    break;

                default:
                    pr_debug("[dvp][drv] From PT110: DVP_CMD_DEFAULT\r\n");
                    break;
            }
        } else {
            switch (rPacket.uCmd) {
            case DVP_CMD_ACK:
                mutex_lock(&dvplock);
                pr_debug("[dvp][drv] Receive DVP ACK Command: %02x %02x\r\n",
                    rPacket.auData[0], rPacket.auData[1]);
                mutex_unlock(&dvplock);
                break;

            case DVP_CMD_PBC_ADO_ID3_TXT:
                pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_ID3_TXT\n");
                fgSendToQ = DVPAgent_parseAudioID3TXT(&rPacket);
                break;

            case DVP_CMD_PBC_ADO_ID3_PIC:
                pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_ID3_PIC\r\n");
                fgSendToQ = DVPAgent_parseAudioID3PIC(&rPacket);
                break;

            case DVP_CMD_PBC_ADO_LYRICS:
                pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_LYRICS\r\n");
                fgSendToQ = DVPAgent_parseLyrics(&rPacket);
                break;

            case DVP_CMD_FL_CODEC:
                pr_debug("[dvp][drv] DVP_CMD_FL_CODEC: %d\r\n",
                    g_rDVPInfo.uCSType);
                mutex_lock(&dvplock);
                g_rDVPInfo.uCSType = rPacket.auData[0];
                mutex_unlock(&dvplock);
                DVPFs_SetFLCodec(g_rDVPInfo.uCSType);
                break;

            case DVP_CMD_FL_TOTAL_COUNT:
                pr_debug("[dvp][drv] DVP_CMD_FL_TOTAL_COUNT\r\n");
                u4Param1 = (rPacket.auData[0] << 8) |
                    rPacket.auData[1];
                mutex_lock(&dvplock);
                g_rDVPInfo.i2StartIdx = -1;
                g_rDVPInfo.iStart = 0;
                g_rDVPInfo.iEnd = -1;
                g_rDVPInfo.u2TotalFileCount = u4Param1;
                if (u4Param1 && !g_rDVPInfo.pi4RipTrkLbaLen)
                    g_rDVPInfo.pi4RipTrkLbaLen =
                    (s32 *)DVP_VMALLOC(u4Param1 * 32);
                pr_debug("[dvp][drv] TOTAL_COUNT: %d\r\n",
                    u4Param1);
                mutex_unlock(&dvplock);
                break;

            case DVP_CMD_SAVE_ITEM_ADDRESS:
                g_rDVPInfo.dwFileInfoDramBaseAddress =
                    getDvpBaseVirtualAddress();
                g_rDVPInfo.dwFileInfoDramBaseAddress +=
                    (rPacket.auData[0] << 24)|
                    (rPacket.auData[1] << 16) |
                    (rPacket.auData[2] << 8) |
                    (rPacket.auData[3]);
                pr_debug("[dvp][drv] DVP_CMD_SAVE_ITEM_ADDRESS: Addr = 0X%08X \r\n",
                    g_rDVPInfo.dwFileInfoDramBaseAddress);
                break;

            case DVP_CMD_CREATE_FSTAB_OK:
                pr_debug("[dvp][drv] DVP_CMD_CREATE_FSTAB_OK\r\n");
                {
                    u32 dwAddr = 0;
                    u32 dwFilter = 0;
                    dwAddr = (rPacket.auData[0] << 24) |
                        (rPacket.auData[1] << 16) |
                        (rPacket.auData[2] << 8) |
                        (rPacket.auData[3]);
                    dwFilter = (rPacket.auData[4] << 24) |
                        (rPacket.auData[5] << 16) |
                        (rPacket.auData[6] << 8) |
                        (rPacket.auData[7]);
                    DVPFs_SetFsValid(dwAddr, dwFilter);

                    pr_debug("[dvp][drv] Addr = 0X%08X , dwFilter = 0X%08X\r\n",
                        dwAddr, dwFilter);
                }
                break;

            case DVP_CMD_SAVE_FILE_TYPE:
                pr_debug("[dvp][drv] DVP_CMD_SAVE_FILE_TYPE\r\n");
                {
                    enum E_FILE_FILTER eFilter = 0;
                    u16              u2WorkDir = 0;

                    eFilter = rPacket.auData[0];
                    pr_debug("[dvp][drv] DVP_CMD_SAVE_FILE_TYPE: eFilter = %d\r\n",
                        eFilter);
                    if (FILE_FILTER_ALL == eFilter) {
                        u2WorkDir =
                        ((rPacket.auData[1] << 8) |
                        (rPacket.auData[2]));
                        DVPFs_SetWorkDir(u2WorkDir);
                        pr_debug("[dvp][drv] TYPE: WorkDir = %d\r\n",
                            u2WorkDir);
                    }
                    DVPFs_SetFilter(eFilter);
                }
                break;

            case DVP_CMD_INVALID_FSTAB:
                pr_debug("[dvp][drv] DVP_CMD_INVALID_FSTAB\r\n");
                DVPFs_SetFsInvalid();
                break;

            case DVP_CMD_FL_ITEM:
                pr_debug("[dvp][drv] DVP_CMD_FL_ITEM\n");
                fgSendToQ = DVPAgent_parseItemInfo(&rPacket);
                break;

            case DVP_CMD_PBC_FILE_PATH: /* have not break*/
                pr_debug("[dvp][drv] DVP_CMD_PBC_FILE_PATH\n");

            case DVP_CMD_PBC_ADO_FILE_PATH: /* add for audio files*/
                pr_debug("[dvp][drv] DVP_CMD_PBC_AUDIO_FILE_PATH");
                fgSendToQ = DVPAgent_filePath(&rPacket);
                break;

            case DVP_CMD_FL_FILE_PATH:
                pr_debug("[dvp][drv][PATH]DVP_CMD_FL_FILE_PATH\r\n");
                fgSendToQ = DVPAgent_CurrentfilePath(&rPacket);
                break;

            case DVP_CMD_RIP_TRK_LBA_LEN:
                fgSendToQ = DVPAgent_riptrklbalen(&rPacket);
                break;

            case DVP_CMD_DRM_SAVE_DATA:  /*DVIX 3.1*/
                pr_debug("[dvp][drv] DVP_CMD_DRM_SAVE_DATA\r\n");
                fgSendToQ = FALSE;
                DVPAgent_SaveDrmData();
                break;

            default:
                pr_debug("[dvp][drv] DVP_CMD_DEFAULT\r\n");
                break;
            }
        }
        if (fgSendToQ)
            DVPAgent_SendtoMainIoctl(fgFromPT, &rPacket);
        }
    }
    pr_debug("[dvp][drv] exit ReadThread success.");
    //x_thread_exit();
    return 0;
}

void DVPAgent_SendtoMainIoctl(bool fgFromPT, struct DVP2APPACKET_T *rPacket)
{
    struct MSG_STRUCT_T  Msg;
    u32                  mNum = 0;
    u32                  mPr = 0;
    Msg.u4Len = sizeof(struct DVP2APPACKET_T);
    Msg.fgFromPT = fgFromPT;
    memcpy(Msg.uDVPData, (const void *)(rPacket),
        sizeof(struct DVP2APPACKET_T));

    mutex_lock(&dvplock);
    for (mNum = 0; mNum < MAX_DVPAGENT_INS_CNT; mNum++) {
        if (_arDVPAgentInsTable[mNum] &&
            (0 != _arDVPAgentInsTable[mNum]->hMsgQ)) {
            s32 ret = x_msg_q_send(_arDVPAgentInsTable[mNum]->hMsgQ,
                &Msg, sizeof(struct MSG_STRUCT_T), 1);
            if (ret != OSR_OK) {
                pr_debug("[dvp][drv] x_msg_q_send FAIL: %d\r\n",
                    ret);
                pr_debug("[dvp][drv] MSG2APP: uInst = %02X, uCmd = %02X, uLenH = %02X, uLenL = %02X\r\n",
                    rPacket->uInstance, rPacket->uCmd,
                    rPacket->uLenH, rPacket->uLenL);
                pr_debug("[dvp][drv] [=== FATAL ERROR ===]Data: ");
                for (mPr = 0; mPr < 9; mPr++)
                    pr_debug("%02X ", rPacket->auData[mPr]);
                pr_debug("\r\n");
            }
        }
    }
    mutex_unlock(&dvplock);
}

void DVPAgent_SaveDrmData(void)
{
    u8   ShareDrmMemory[DRM_PACKED_ALLOCATION_BYTES];
    u32  u4Idx = MZ_DRM_INFO_IDX_START;
    u32  DivxShareMemAddr = changePhyToVirtualAddress(GetDvpMemBaseAddr() +
        SI_DRM_MEMORY_ADDR);

    memset(ShareDrmMemory, 0,
        sizeof(u8) * DRM_PACKED_ALLOCATION_BYTES);
    memcpy(ShareDrmMemory, (const void *)DivxShareMemAddr,
        sizeof(u8)*DRM_PACKED_ALLOCATION_BYTES);
    mutex_lock(&dvplock);

#ifdef MM_SUPPORT_DIVXHT31
    if (MZ_SUCCESS == MetaZone_WriteBinary(u4Idx,
        (u8 *)ShareDrmMemory, sizeof(u8)*DRM_PACKED_ALLOCATION_BYTES))
        MetaZone_Flush(TRUE);
    else
        pr_debug("[dvp][drv] Write memory failed\r\n");
#endif

    mutex_unlock(&dvplock);
}

bool DVPAgent_parseAudioID3TXT(struct DVP2APPACKET_T *rPacket)
{
    u16  u2DataLen = 0;
    u32  dwId3TextAddr = getDvpBaseVirtualAddress();
    u32  dwId3TextAddrOfst = 0;

    dwId3TextAddrOfst = (rPacket->auData[2] << 24) |
        (rPacket->auData[3] << 16) |
        (rPacket->auData[4] << 8) | (rPacket->auData[5]);

    if ((dwId3TextAddrOfst >= dvp_share_rsv_mem.size) ||
        (0xFF == rPacket->auData[5])) {
        pr_debug("[dvp][drv] cmd: 0x%x, Invalid dwId3TextAddrOfst: 0x%08x",
            rPacket->uCmd, dwId3TextAddrOfst);
        return false;
    }
    dwId3TextAddr += dwId3TextAddrOfst;
    u2DataLen = (rPacket->auData[6] << 8) | (rPacket->auData[7]);

    pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_ID3_TXT Addr2 0X%08X, Len %d\r\n",
        dwId3TextAddr, u2DataLen);

    rPacket->uLenH = rPacket->auData[6];
    rPacket->uLenL = rPacket->auData[7];

    if (u2DataLen > MAX_DVP_SEND_DATA-2) {
        u2DataLen = MAX_DVP_SEND_DATA-2;
        memcpy(&(rPacket->auData[2]), (const void *)dwId3TextAddr,
            u2DataLen);
    } else {
        memcpy(&(rPacket->auData[2]), (const void *)dwId3TextAddr,
            u2DataLen);
    }
    return true;
}

bool DVPAgent_parseAudioID3PIC(struct DVP2APPACKET_T *rPacket)
{
    u32  u4DataLen;
    u32  dwId3PicAddr = getDvpBaseVirtualAddress();
    u32  dwId3PicAddrOfst = 0;

    dwId3PicAddrOfst = (rPacket->auData[2] << 24) |
        (rPacket->auData[3] << 16) |
        (rPacket->auData[4] << 8) | (rPacket->auData[5]);

    if (dwId3PicAddrOfst >= dvp_share_rsv_mem.size) {
        pr_debug("[dvp][drv] cmd: 0x%x, Invalid dwId3PicAddrOfst: 0x%08x",
            rPacket->uCmd, dwId3PicAddrOfst);
        return false;
    }
    dwId3PicAddr += dwId3PicAddrOfst;
    u4DataLen = (rPacket->auData[6] << 8) | (rPacket->auData[7])
        | (rPacket->auData[0] << 24) | (rPacket->auData[1] << 16);

    pr_debug("[dvp][drv] Data: %02X %02X %02X %02X %02X %02X %02X %02X %02X \r\n",
        rPacket->auData[0], rPacket->auData[1], rPacket->auData[2],
        rPacket->auData[3], rPacket->auData[4], rPacket->auData[5],
        rPacket->auData[6], rPacket->auData[7], rPacket->auData[8]);

    pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_ID3_PIC: Addr = 0X%08X , Len = %x\r\n",
        dwId3PicAddr, u4DataLen);

    rPacket->uLenH = rPacket->auData[6];
    rPacket->uLenL = rPacket->auData[7];

    mutex_lock(&dvplock);
    if (!puId3PicBuf) {
        puId3PicBuf = (u8 *)DVP_VMALLOC(u4DataLen * sizeof(u8));
    } else {
        vfree(puLyricBuf);
        puId3PicBuf = (u8 *)DVP_VMALLOC(u4DataLen * sizeof(u8));
    }

    if (!puId3PicBuf) {
        pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_ID3_PIC malloc puId3PicBuf fail\r\n");
        mutex_unlock(&dvplock);
        return true;
    }

    u4Id3PicLen = (u32)u4DataLen;
    memcpy(puId3PicBuf, (const void *)dwId3PicAddr, u4DataLen);
    mutex_unlock(&dvplock);
    return true;
}

bool DVPAgent_parseLyrics(struct DVP2APPACKET_T *rPacket)
{
    u16  u2DataLen;
    u32  dwLyricsAddr = getDvpBaseVirtualAddress();
    u32  dwLyricsAddrOfst = 0;

    dwLyricsAddrOfst = (rPacket->auData[0] << 24) |
        (rPacket->auData[1] << 16) |
        (rPacket->auData[2] << 8) | (rPacket->auData[3]);

    if (dwLyricsAddrOfst >= dvp_share_rsv_mem.size) {
        pr_debug("[dvp][drv] cmd: 0x%x, Invalid dwLyricsAddrOfst: 0x%08x",
            rPacket->uCmd, dwLyricsAddrOfst);
        return false;
    }
    dwLyricsAddr += dwLyricsAddrOfst;
    u2DataLen = (rPacket->auData[4] << 8) | (rPacket->auData[5]);

    pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_LYRICS: Addr = 0X%08X , Len = %d\r\n",
        dwLyricsAddr, u2DataLen);

    rPacket->uLenH = rPacket->auData[4];
    rPacket->uLenL = rPacket->auData[5];
    mutex_lock(&dvplock);
    if (!puLyricBuf) {
        puLyricBuf = (u8 *)DVP_VMALLOC(u2DataLen * sizeof(u8));
    } else {
        vfree(puLyricBuf);
        puLyricBuf = (u8 *)DVP_VMALLOC(u2DataLen * sizeof(u8));
    }
    if (!puLyricBuf) {
        pr_debug("[dvp][drv] DVP_CMD_PBC_ADO_LYRICS DVP_VMALLOC puLyricBuf fail\r\n");
        mutex_unlock(&dvplock);
        return true;
    }
    u4LyricLen = (u32)u2DataLen;
    memcpy(puLyricBuf, (const void *)dwLyricsAddr, u2DataLen);
    memcpy(rPacket->auData, (const void *)dwLyricsAddr, MAX_DVP_SEND_DATA);
    mutex_unlock(&dvplock);
    return true;
}

bool DVPAgent_parseItemInfo(struct DVP2APPACKET_T *rPacket)
{
    u16  u2DataLen;
    u32  dwFilePathAddr = getDvpBaseVirtualAddress();
    u32  dwFilePathAddrOfst = 0;

    dwFilePathAddrOfst = (rPacket->auData[3] << 24) |
        (rPacket->auData[4] << 16) |
        (rPacket->auData[5] << 8) | (rPacket->auData[6]);
    if (dwFilePathAddrOfst >= dvp_share_rsv_mem.size) {
        pr_debug("[dvp][drv] cmd: 0x%x, Invalid dwFilePathAddrOfst: 0x%08x",
            rPacket->uCmd, dwFilePathAddrOfst);
        return false;
    }
    dwFilePathAddr += dwFilePathAddrOfst;
    u2DataLen = (rPacket->auData[7] << 8) | (rPacket->auData[8]);

    pr_debug("[dvp][drv] DVP_CMD_FL_ITEM Addr 0X%08X, Len = %d\r\n",
        dwFilePathAddr, u2DataLen);

    u2DataLen += 3;
    rPacket->uLenH = (u8)(u2DataLen >> 8);
    rPacket->uLenL = (u8)u2DataLen;
    u2DataLen -= 3;

    if (u2DataLen > MAX_DVP_SEND_DATA-3) {
        u2DataLen = MAX_DVP_SEND_DATA-3;
        memcpy(&(rPacket->auData[3]), (const void *)dwFilePathAddr,
            u2DataLen);
    } else {
        memcpy(&(rPacket->auData[3]), (const void *)dwFilePathAddr,
            u2DataLen);
    }
    mutex_lock(&dvplock);
    return false;
}

bool DVPAgent_filePath(struct DVP2APPACKET_T *rPacket)
{
    u16  u2DataLen;
    u32  dwFilePathAddr = getDvpBaseVirtualAddress();
    u32  dwFilePathAddrOfst = 0;

    dwFilePathAddrOfst = (rPacket->auData[0] << 24) |
        (rPacket->auData[1] << 16) |
        (rPacket->auData[2] << 8) | (rPacket->auData[3]);

    if (dwFilePathAddrOfst >= dvp_share_rsv_mem.size) {
        pr_debug("[dvp][drv] cmd: 0x%x, Invalid AddrOfst: 0x%08x",
            rPacket->uCmd, dwFilePathAddrOfst);
        return false;
    }
    dwFilePathAddr += dwFilePathAddrOfst;
    u2DataLen = (rPacket->auData[4] << 8) | (rPacket->auData[5]);

    pr_debug("[dvp][drv] DVP_CMD_PBC_FILE_PATH: Addr = 0X%08X, Len = %d\r\n",
        dwFilePathAddr, u2DataLen);

    rPacket->uLenH = rPacket->auData[4];
    rPacket->uLenL = rPacket->auData[5];

    if (u2DataLen > MAX_DVP_SEND_DATA) {
        u2DataLen = MAX_DVP_SEND_DATA;
        memcpy(rPacket->auData, (const void *)dwFilePathAddr,
            u2DataLen);
    } else {
        memcpy(rPacket->auData, (const void *)dwFilePathAddr,
            u2DataLen);
    }

    mutex_lock(&dvplock);
    if (u2DataLen > MAX_PATH)
        u2DataLen = MAX_PATH;

    memset(g_rDVPInfo.szCurrentFilePath, 0,
        sizeof(g_rDVPInfo.szCurrentFilePath));
    if (CODEC_UNICODE == g_rDVPInfo.uCSType) {
        BigEndiaDataUcs2ToUtf8(rPacket->auData, u2DataLen,
            (u8 *)(g_rDVPInfo.szCurrentFilePath));
    } else {
        OtherCodeToUtf8(rPacket->auData, u2DataLen,
            (u8 *)(g_rDVPInfo.szCurrentFilePath));
    }
    pr_debug("[dvp][drv] g_rDVPInfo.szCurrentFilePath: %s\r\n",
        (u8 *)g_rDVPInfo.szCurrentFilePath);
    mutex_unlock(&dvplock);
    return true;
}

bool DVPAgent_CurrentfilePath(struct DVP2APPACKET_T *rPacket)
{
    u16 u2DirIndex = 0;
    u16 u2Len = MAX_DVP_SEND_DATA;

    u2DirIndex = rPacket->auData[1] << 8 | rPacket->auData[0];

    pr_debug("[DVR_DRV][PATH] u2DirIndex: %d \r\n", u2DirIndex);

    mutex_lock(&dvplock);
    u2Len = DVPFs_GetPathByIndex(u2DirIndex,
        (u8 *)g_rDVPInfo.szCurrentFilePath, MAX_DVP_SEND_DATA/2);
    pr_debug("[dvp][drv][PATH]:%d szCurrentFilePath: %s\r\n",
        u2Len, (u8 *)g_rDVPInfo.szCurrentFilePath);
    memcpy(rPacket->auData, (u8 *)(g_rDVPInfo.szCurrentFilePath), u2Len);
    mutex_unlock(&dvplock);
    rPacket->uLenH = (u2Len >> 8) & 0xFF;
    rPacket->uLenL = (u2Len & 0xFF);
    return TRUE;
}

void DVPAgent_getRipAddress(struct DVP2APPACKET_T *rPacket)
{
    u32 u2DataLen = 0;

    dwRipDataAddr = getDvpBaseVirtualAddress();
    pr_debug("[dvp][drv] PT2AP_RIP_DATA_READY %02X %02X %02X %02X %02X %02X %02X %02X \r\n",
        rPacket->auData[0], rPacket->auData[1], rPacket->auData[2],
        rPacket->auData[3], rPacket->auData[4], rPacket->auData[5],
        rPacket->auData[6], rPacket->auData[7]);

    dwRipDataAddr += (rPacket->auData[0] << 24) |
        (rPacket->auData[1] << 16) |
        (rPacket->auData[2] << 8) |
        (rPacket->auData[3]);
    u2DataLen = (rPacket->auData[4] << 24) | (rPacket->auData[5] << 16)
        | (rPacket->auData[6] << 8) | (rPacket->auData[7]);

    pr_debug("[dvp][drv] PT2AP_RIP_DATA_READY Addr 0X%08X, Len %d\r\n",
        dwRipDataAddr, u2DataLen);
}

bool DVPAgent_riptrklbalen(struct DVP2APPACKET_T *rPacket)
{
    mutex_lock(&dvplock);
    if (!g_rDVPInfo.pi4RipTrkLbaLen) {
        g_rDVPInfo.pi4RipTrkLbaLen = (s32 *)DVP_VMALLOC \
            (g_rDVPInfo.u2TotalFileCount * sizeof(INT32));
    } else {
        /*the first track lba lengthm */
        if (rPacket->auData[0] == 1) {
            vfree(g_rDVPInfo.pi4RipTrkLbaLen);
            g_rDVPInfo.pi4RipTrkLbaLen = (s32 *)DVP_VMALLOC \
                (g_rDVPInfo.u2TotalFileCount * sizeof(INT32));
        }
    }
    if (!g_rDVPInfo.pi4RipTrkLbaLen) {
        pr_debug("[dvp][drv] DVP_CMD_RIP_TRK_LBA_LEN: malloc pi4RipTrkLbaLen fail.\r\n");
        mutex_unlock(&dvplock);
        return false;
    }
    if ((rPacket->auData[0] > 0) && (rPacket->auData[0] <=
        g_rDVPInfo.u2TotalFileCount)) {
        g_rDVPInfo.pi4RipTrkLbaLen[rPacket->auData[0] - 1] =
            (rPacket->auData[3] << 24) | (rPacket->auData[4] << 16)
            | (rPacket->auData[1] << 8) | rPacket->auData[2];
    }
    mutex_unlock(&dvplock);
    return true;
}

static u32 StrLenW(const u16 *wszStr)
{
    const u16 *p = wszStr - 1;
    /* exclude the terminating '\0' */
    while (*++p)
        ;
    return p - wszStr;
}

static bool CreateFileDir(u16 *pFile)
{
    u16    *p = NULL;
    u32    u4Len;
    u32    u4pLen;
    bool   fgMediaExist = FALSE;
    bool   fgRet = FALSE;
    s8     sDir[MAX_PATH + 1];
    u32    dwErr;

    if (!pFile)
        return FALSE;
    pr_debug("[dvp][drv] CreateFileDir Parameter: *pFile= %s\r\n",
        (u8 *)pFile);

    u4Len = StrLenW(pFile);
    u4pLen = 0;
    p = pFile;

    while (p != _T('\0') && (u4pLen + 1 < u4Len)) {
        if (*p == _T('\\')) {
            memset(sDir, 0, sizeof(s8)*(MAX_PATH + 1));
            memcpy(sDir, (const void *)(pFile),
                (p - pFile) * sizeof(s8));

            if (!fgMediaExist) {
                memset(szMP3RootDir, 0,
                    sizeof(u16)*(MAX_PATH + 1));
                memcpy(szMP3RootDir, (const void *)(pFile),
                    (p - pFile) * sizeof(u16));
                if (!fgRet) {
                #ifndef __linux__
                    pr_debug("[dvp][drv] Check Media Exist: err = %s(%d)!\r\n", strerrno(errno), errno);
                    if (errno == OK)
                        g_fgMediaExist = TRUE;
                    else
                        g_fgMediaExist = FALSE;
                #endif
                } else{
                    g_fgMediaExist = FALSE;
                }
                fgMediaExist = TRUE;
            }
        }
        p++;
        u4pLen++;
    }

    return TRUE;
}

bool DVPAgent_SetRipPath(struct DVP_Rip_Path *pRipPath)
{
    pr_debug("[dvp][drv]DVPAgent_SetRipPath %s, %d, %d\r\n",
        (u8 *)pRipPath->path, pRipPath->u4Len, pRipPath->u4TrkLen);

    if (pRipPath->path && (pRipPath->u4Len < MAX_PATH)) {
        memset(szMP3Path, 0, sizeof(u16) * (MAX_PATH + 1));
        memcpy(szMP3Path, (const void *)(pRipPath->path),
            sizeof(u16) * pRipPath->u4Len);

        if (pRipPath->u4Len == 0) {
            pRipPath->u4Len =
                sizeof("\\USB Hard Disk\\ripdata.mp3");
            memcpy(szMP3Path,
                (const void *)("\\USB Hard Disk\\ripdata.mp3"),
                pRipPath->u4Len);
            szMP3Path[pRipPath->u4Len] = _T('\0');
        } else {
            szMP3Path[pRipPath->u4Len] = TEXT('\0');
        }
        pr_debug("[dvp][drv] AP_CMD_RIP_PATH %s\r\n", (u8 *)szMP3Path);
        CreateFileDir(szMP3Path);
        g_u4TrkLen = pRipPath->u4TrkLen;
    }
    return TRUE;
}

s32 DVPAgent_SendCmd(u8 bInstID, struct AP2DVPCMD_T *prCmd)
{
    struct AP2DVPPACKET_T   rPacket;
    bool            fgSend2DVP = FALSE;
    u32             u4Len = 0;

    switch (prCmd->uCmd) {
    case AP_CMD_RIP_PATH:
        if (prCmd->uParam1 && (prCmd->uParam2 < MAX_PATH)) {
            memset(szMP3Path, 0,
                sizeof(u16) * (MAX_PATH + 1));
            if (szMP3Path[prCmd->uParam2-1] != _T('3') ||
                szMP3Path[prCmd->uParam2-2] != _T('p') ||
                szMP3Path[prCmd->uParam2-2] != _T('P') ||
                szMP3Path[prCmd->uParam2-3] != _T('m') ||
                szMP3Path[prCmd->uParam2-3] != _T('M')) {
                u4Len = sizeof("\\USB Hard Disk\\ripdata.mp3");
                memcpy(szMP3Path,
                (const void *)("\\USB Hard Disk\\ripdata.mp3"),
                u4Len);
                szMP3Path[u4Len] = _T('\0');
            } else {
                copy_from_user(szMP3Path,
                (const void *)(prCmd->uParam1),
                (u32)(prCmd->uParam2 * sizeof(u16)));
                szMP3Path[prCmd->uParam2] = TEXT('\0');
            }
            pr_debug("[dvp][drv] AP_CMD_RIP_PATH %s\r\n",
                (u8 *)szMP3Path);
            CreateFileDir(szMP3Path);
        }
        break;

    case AP_CMD_RIP_TRACK:
        {
            pr_debug("[dvp][drv] AP_CMD_RIP_TRACK\r\n");
            fgSend2DVP = TRUE;
        }
        break;

    case AP_CMD_RIP_STOP:
        {
            pr_debug("[dvp][drv] AP_CMD_RIP_STOP\r\n");
            fgSend2DVP = TRUE;
        }
        break;

    default:
        fgSend2DVP = TRUE;
        break;
    }

    if (fgSend2DVP) {
        rPacket.uInstance = bInstID;

        rPacket.uCmd = prCmd->uCmd;
        rPacket.uParam1 = prCmd->uParam1;
        rPacket.uParam2 = prCmd->uParam2;
        rPacket.uParam3 = prCmd->uParam3;
        rPacket.uParam4 = prCmd->uParam4;
        pr_debug("[dvp][drv] Send DVP Command: %02x %02x %02x %02x %02x %02x\n",
            rPacket.uInstance, rPacket.uCmd, rPacket.uParam1,
            rPacket.uParam2, rPacket.uParam3, rPacket.uParam4);

        DVPHost_Write8032((u8 *)&rPacket, sizeof(rPacket));
    }
    return RET_OK;
}

bool DVPAgent_CopyLyricsBuf(u8 *puData, u32 u4DataLen)
{
    if (!puData || u4DataLen == 0 || !puLyricBuf)
        return FALSE;

    mutex_lock(&dvplock);
    if (u4DataLen < u4LyricLen) {
        pr_debug("[dvp][drv] u4DataLen is less than puLyricBuf length, Lyrics is cut!\r\n");
        memcpy(puData, (const void *)(puLyricBuf), u4DataLen);
    } else {
        memcpy(puData, (const void *)(puLyricBuf), u4LyricLen);
    }

    vfree(puLyricBuf);
    puLyricBuf = NULL;
    u4LyricLen = 0;
    mutex_unlock(&dvplock);
    return TRUE;
}

bool DVPAgent_CopyId3PicBuf(u8 *puData, u32 u4DataLen)
{
    if (!puData || u4DataLen == 0 || !puId3PicBuf)
        return FALSE;

    mutex_lock(&dvplock);
    if (u4DataLen < u4Id3PicLen) {
        pr_debug("[dvp][drv] u4DataLen [%d]is less than puId3PicBuf length[%d], Id3Pic is cut!\r\n",
            u4DataLen, u4Id3PicLen);
        memcpy(puData, (const void *)(puId3PicBuf), u4DataLen);
    } else {
        memcpy(puData, (const void *)(puId3PicBuf), u4Id3PicLen);
    }
    vfree(puId3PicBuf);
    puId3PicBuf = NULL;
    u4Id3PicLen = 0;
    mutex_unlock(&dvplock);
    return TRUE;
}

bool DVPAgent_CopyRipData(s8 *puRipBuf, u32 u4Len)
{
    memcpy(puRipBuf, (LPCSTR)dwRipDataAddr, u4Len);
    return TRUE;
}

bool DVPAgent_SetDVPLog(struct DVP_LOG_SET *prDVPLog)
{
    if ((prDVPLog->eLogType < DVP_8032_LOG) ||
        (prDVPLog->eLogType > DVP_PT110_LOG))
        return FALSE;

    if (DVP_PT110_LOG == prDVPLog->eLogType)
        g_rDVPInfo.fgPT110Log = prDVPLog->fgDVPLog;
    else
        g_rDVPInfo.fg8032Log = prDVPLog->fgDVPLog;

    return TRUE;
}

bool DVPAgent_SetCodePage(u32 u4CodePage)
{
    mutex_lock(&dvplock);
    g_rDVPInfo.u4CodePage = u4CodePage;
    mutex_unlock(&dvplock);

    return TRUE;
}

bool DVPAgent_WriteData2Dram(struct DVP_Dram_Data *prDramData, u32 u4Size)
{
    u32 dwWriteAddress = 0;
    u32 startAddress   = 0;
    if(prDramData) {
        if (!access_ok(VERIFY_WRITE, (void __user *)prDramData, sizeof(struct DVP_Dram_Data))) {
            pr_err("[dvp][drv]access_ok prDramData err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            return FALSE;
        }
    }

    copy_from_user(&startAddress, &(prDramData->dwStartAddress), sizeof(u32));
    if (startAddress + u4Size > 0X00A00000)
        return FALSE;

    dwWriteAddress = (u32)changePhyToVirtualAddress(GetDvpMemBaseAddr() + startAddress);

    if (!access_ok(VERIFY_WRITE, (void __user *)prDramData->pvBufferData, u4Size - sizeof(u32))) {
        pr_err("[dvp][drv]access_ok prDramData->pvBufferData err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }
    copy_from_user((char *)dwWriteAddress, (const void *)(prDramData->pvBufferData),
        (u32)(u4Size - sizeof(u32)));

    return TRUE;
}

bool DVPAgent_ReadDataFromDram(struct DVP_Dram_Data *prDramData, u32 u4Size)
{
    u32  dwReadAddress = 0;
    u32  startAddress  = 0;
    if(prDramData)
    {
        if (!access_ok(VERIFY_WRITE, (void __user *)prDramData, sizeof(struct DVP_Dram_Data))) {
            pr_err("[dvp][drv]access_ok pBufIn err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            return FALSE;
        }
    }

    copy_from_user(&startAddress, &(prDramData->dwStartAddress), sizeof(u32));

    if (startAddress + u4Size > 0X00A00000)
        return FALSE;

    dwReadAddress = (u32)changePhyToVirtualAddress(GetDvpMemBaseAddr() + startAddress);
    pr_info("[dvp][drv] dwReadAddress: %x\n", dwReadAddress);
    if (!access_ok(VERIFY_WRITE, (void __user *)prDramData->pvBufferData, u4Size - sizeof(u32))) {
        pr_err("[dvp][drv]access_ok prDramData->pvBufferData err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }
    copy_to_user(prDramData->pvBufferData, (const void *)dwReadAddress,
        u4Size - sizeof(u32));

    return TRUE;
}

bool DVPAgent_ReadFileInfoFromFsIO(struct DVP_FILEITEM_INFO_T *prFileInfo,
    u32 u4Size, u32 u4ReadIndex)
{
    u32     u4Index = 0;
    u32     u4ReadNum = 0;
    struct DVPFsFileItem  rFileItem;

    if (NULL == prFileInfo)
        return FALSE;

    u4ReadNum = u4Size / sizeof(struct DVP_FILEITEM_INFO_T);

    pr_debug("[dvp][drv] DVPAgent_ReadFileInfoFromFsIO!u4ReadNum: %d, u4ReadIndex: %d \r\n",
        u4ReadNum, u4ReadIndex);

    mutex_lock(&dvplock);
    for (u4Index = 0; u4Index < u4ReadNum; u4Index++) {
        memset(&rFileItem, 0, sizeof(rFileItem));
        DVPFs_ReadFileInfo(&rFileItem, u4ReadIndex);
        prFileInfo->uType = rFileItem.uFType;
        memset(prFileInfo->szFileName, 0,
            sizeof(prFileInfo->szFileName));
        memcpy(prFileInfo->szFileName, rFileItem.szName,
            FS_RAM_CODE_MAX_FILENAME_BUF_SIZE);
        pr_debug("[dvp][drv] ReadFileInfo FileType: %d, FileName: %s \r\n",
            prFileInfo->uType, (u8 *)prFileInfo->szFileName);
        u4ReadIndex++;
        prFileInfo++;
    }
    mutex_unlock(&dvplock);
    pr_debug("[dvp][drv] DVPAgent_ReadFileInfoFromFsIO end! \r\n");

    return TRUE;
}

#ifdef MM_SUPPORT_DIVXHT31
void loadDrmData()
{
    pr_debug("[dvp][drv] loadDrmData\r\n");
    u32 divxShareMemAddr = changePhyToVirtualAddress(GetDvpMemBaseAddr() +
        SI_DRM_MEMORY_ADDR);

    if (MetaZone_ReadBinary(MZ_DRM_INFO_IDX_START, (LPCSTR)divxShareMemAddr,
        sizeof(u8)*DRM_PACKED_ALLOCATION_BYTES) == MZ_FAILURE) {
        pr_debug("[dvp][drv]Write memory failed\r\n");
    }
}
#endif

#ifdef DVP_PM_SUPPORT
#define DVP_CODE_IN_NAND
#define DVP_SERVO_PATCH1
#define DVP_SERVO_PIN_CONFIG
#define BSP_DEMO
#define DRV_MMU_ENABLE

#define ARGS_DVD_CODE_IN_NAND           (1 << 0)
#define ARGS_SERVO_PATCH1               (1 << 1)
#define ARGS_DVP_SERVO_PIN_CONFIG       (1 << 2)
#define ARGS_BSP_NODVDMEMORY            (1 << 3)
#define ARGS_BSP_HAS_CUSTOM_FOUR        (1 << 4)
#define ARGS_BSP_DEMO                   (1 << 5)
#define ARGS_BSP_OTHERS                 (1 << 6)
#define ARGS_BSP_MMU_ENABLE             (1 << 9)

#define RESERVED_DVD_BASE_VA            dvp_share_rsv_mem.base
#define RESERVED_AUDIO_BASE_VA          dsp_share_rsv_mem.base

/*Target Bin Parameters */
#define CODE_INFO_START_ADDRESS         (0x200)
#define CODE_INFO_LENGTH                (0x12)
/*8032 + risc should not exceed this size */
#define DVP_TARGET_LENGTH_LIMIT         (2 * 1024 * 1024)
/*ROMCODE Offset Address in target bin */
#define ROMCODE_IN_RISC_FLASH_OFFSET    (720 * 1024)

/*DRAMB Parameters */
#define DRAM_PARTITION_ADDR_BASE        (0x71F000L)
/*ROMCODE Offset Address to DRAMB */
#define ROMCODE_IN_DRAMB_OFFSET         (0x70000)
/*ROM CODE LENGTH */
#define ROM_CODE_LENGTH                 (260 * 1024)

#define DVP_SHAREINFO_OFFSET_RESERVED_DRAM       (0x800000)

/*DDR Paremeters */
#define DVP_CODE_OFFSET_IN_RESERVED_DRAM    (0x900000)
/*this address is specified in config.bib in CE OS   */
#define DVP_RAM_IMAGE_START_ADDR     \
    (RESERVED_DVD_BASE_VA + DVP_CODE_OFFSET_IN_RESERVED_DRAM)
#define DRAMB_START_ADDR    (RESERVED_DVD_BASE_VA + DRAM_PARTITION_ADDR_BASE)

#define ReadReg32(addr)         (*(u32 *)(addr))
#define WriteReg32(addr, data)  ((*(volatile u32 *)(addr)) = (u32)(data))

#define bHiByte(arg)            (*((u8 *)&arg + 1))
#define bLoByte(arg)            (*(u8 *)&arg)
#define wHiWord(arg)            (*((u16 *)&arg + 1))
#define wLoWord(arg)            (*(u16 *)&arg)

#define Buf_GetPos8(addr, offset)   ((u8 *)((u8 *)(addr) + (offset)))
#define Buf_GetData32(addr, offset) (*(u32 *)((u8 *)(addr) + (offset)))
#define Buf_GetData8(addr, offset)  (*(u8 *)((u8 *)(addr) + (offset)))
#define IsALIGN128K(val) (((val)&((1 << 17) - 1)) ? FALSE : TRUE)

extern BOOL DVDLoad(DWORD pArgument, DWORD DVDBase, DWORD AudioBase, DWORD ShareInfoBase);
EXPORT_SYMBOL(DVDLoad);

int printf(const char *format, ...)
{
    const int len = 2048;
    char szLog[len + 1];
    va_list vl;
    va_start(vl, format);

    if (format != NULL)
    {
        memset(szLog, 0, len + 1);
        vsnprintf(szLog, len , format, vl);
        pr_debug("[printf] %s\r\n", szLog);
    }

    va_end(vl);
    return 0;
}

bool DVPAgent_PowerOn(void)
{
    u32 dwArgument = 0;
    u32 dwData0 = 0;
    u32 dwData1 = 0;

#ifdef DVP_CODE_IN_NAND
    dwArgument |= ARGS_DVD_CODE_IN_NAND;
#endif
#ifdef DVP_SERVO_PATCH1
    dwArgument |= ARGS_SERVO_PATCH1;
#endif
#ifdef DVP_SERVO_PIN_CONFIG
    dwArgument |= ARGS_DVP_SERVO_PIN_CONFIG;
#endif
#ifndef BSP_NODVDMEMORY
    dwArgument |= ARGS_BSP_NODVDMEMORY;
#endif

#if 1     /*def BSP_DEMO */
    dwArgument |= ARGS_BSP_DEMO;
#else
    dwArgument |= ARGS_BSP_OTHERS;
#endif

#ifdef DRV_MMU_ENABLE
    dwArgument |= ARGS_BSP_MMU_ENABLE;
#endif

    dwData0 = RESERVED_DVD_BASE_VA;
    dwData1 = RESERVED_AUDIO_BASE_VA;
    /*MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm] DVPAgent_PowerOn,
        IO_BASE_VA(0x%x) dvdBufferBase:0x%x audBuffBase:0x%x\r\n"),
        IO_BASE_VA, dwData0, dwData1); */
    DVDLoad(dwArgument, dwData0, dwData1, DVP_SHAREINFO_OFFSET_RESERVED_DRAM);
    return TRUE;
}

bool DVPAgent_PowerOff(void)
{
    u32 u4Temp = 0;

    u4Temp = ReadReg32(IO_BASE_VA + 0x94);
    u4Temp &= 0xFFFFFFFC;
    WriteReg32(IO_BASE_VA + 0x94, u4Temp);
    return TRUE;
}

bool DVPImage_Config(void)
{
    u8  *pMemAddr;
    u32 u4RiscAddr, u4RiscSize, u4MainRiscAddr, u4MainRiscSize;
    u32 u4Tmp, u4LoaderSize;

    /*Step 1: Move Target BIN excpet ROMCODE to DRAM*/
    pMemAddr = (u8 *)changePhyToVirtualAddress(DVP_RAM_IMAGE_START_ADDR);

    /* Step 2: Parser DVP image information form targetbin */
    /* Get RISC fastlogo code start address */
    bHiByte(wHiWord(u4RiscAddr)) = 0;
    bLoByte(wHiWord(u4RiscAddr)) = Buf_GetData8(pMemAddr,
        CODE_INFO_START_ADDRESS + 8); /* u4CodeInfo[8]; */
    bHiByte(wLoWord(u4RiscAddr)) = Buf_GetData8(pMemAddr,
        CODE_INFO_START_ADDRESS + 7);
    bLoByte(wLoWord(u4RiscAddr)) = Buf_GetData8(pMemAddr,
        CODE_INFO_START_ADDRESS + 6);

    /* Get RISC fastlogo code length */
    bHiByte(wHiWord(u4RiscSize)) = 0;
    bLoByte(wHiWord(u4RiscSize)) = Buf_GetData8(pMemAddr,
        CODE_INFO_START_ADDRESS + 11);
    bHiByte(wLoWord(u4RiscSize)) = Buf_GetData8(pMemAddr,
        CODE_INFO_START_ADDRESS + 10);
    bLoByte(wLoWord(u4RiscSize)) = Buf_GetData8(pMemAddr,
        CODE_INFO_START_ADDRESS + 9);

    /*MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm] DVPImage_Config,
    Fastlogo info -- start address = 0x%x, loader length = 0x%x\r\n"),
        u4RiscAddr, u4RiscSize); */

    /*Get Main RISC code start address */
    /*Fast logo total length*/
    u4Tmp = Buf_GetData32(pMemAddr, u4RiscAddr + 0x28);
    u4MainRiscAddr = u4RiscAddr + u4Tmp + 8;

    /*Get Main RISC code length */
    u4MainRiscSize = Buf_GetData32(pMemAddr, u4MainRiscAddr - 8);

    /*MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm] DVPImage_Config--
        MainRISC Addr = 0x%x, MainRISC Length = 0x%x\r\n"),
        u4MainRiscAddr, u4MainRiscSize); */

    if ((u4MainRiscAddr + u4MainRiscSize) > DVP_TARGET_LENGTH_LIMIT) {
        pr_err("[dvp][drv][pm] Target Bin is too large [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    /*Step 3: Prepare Fast Logo code*/
    /*move fast logo LOADER chunk to DRAMB*/
    u4Tmp = changePhyToVirtualAddress(DRAMB_START_ADDR);
    /* MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm] DVPImage_Config--
        DramB address = 0x%x\r\n"), u4Tmp); */
    memcpy((void *)u4Tmp, Buf_GetPos8(pMemAddr, u4RiscAddr), u4RiscSize);

    /*move fast logo CODE_DRAM&DATA_DRAM chunk to DRAMA */
    u4LoaderSize = Buf_GetData32(pMemAddr, u4RiscAddr + 0x20);
    u4RiscSize = Buf_GetData32(pMemAddr, u4RiscAddr + 0x28);

    u4Tmp = changePhyToVirtualAddress(RESERVED_DVD_BASE_VA);
    /*MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm]
        DVPImage_Config--DramA address =
        0x%x, u4LoaderSize:0x%x, u4RiscSize:0x%x\r\n"),
        u4Tmp, u4LoaderSize, u4RiscSize); */
    memcpy((void *)u4Tmp, Buf_GetPos8(pMemAddr, u4RiscAddr + u4LoaderSize),
        (u4RiscSize - u4LoaderSize));

    /*Step 5: Config 8032 run in dram*/
    if (!IsALIGN128K(DVP_RAM_IMAGE_START_ADDR)) {
        pr_err("[dvp][drv][pm] DVPImage_Config----ERROR: DVP code start address must align to 128K. [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    /*MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm] DVPImage_Config--
        8032 start address = 0x%x\r\n"),
        DVP_RAM_IMAGE_START_ADDR); */

    /* 8032 code start address in dram, value = (physic offset / 128K) */
    WriteReg32((IO_BASE_VA + 0X3A0008), ((DVP_RAM_IMAGE_START_ADDR) >> 17));
    /*set to 3 means 8032 code is in dram, if 0 means in flash */
    WriteReg32((IO_BASE_VA + 0X3A0010), 0x3);

    GPIO_MultiFun_Set(PIN_2_GPIO2, DVD_T8032_UP0_SEL);
    GPIO_MultiFun_Set(PIN_3_GPIO3, DVD_T8032_UP0_SEL);

    pMemAddr = (u8 *)changePhyToVirtualAddress(DVP_RAM_IMAGE_START_ADDR);
    /*MMLOG_TRACE(LOG_MOD_DVP, TEXT("[dvp][drv][pm] DVPImage_Config--
        8032 code: %x %x %x %x %x %x %x %x\r\n"),
        pMemAddr[0], pMemAddr[1], pMemAddr[2], pMemAddr[3],
        pMemAddr[4], pMemAddr[5], pMemAddr[6], pMemAddr[7]); */

    pr_debug("[dvp][drv][pm] DVPImage_Config SUCCESS\r\n");

    return TRUE;
}

u32 DVPMemRevPhy2Virt(u32 addr)
{
    return changePhyToVirtualAddress(addr);
}
#endif /*DVP_PM_SUPPORT*/


