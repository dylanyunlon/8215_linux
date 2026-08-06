#include <linux/uaccess.h>
#include <errno.h>

#include "dvp_cmd.h"
#include "dvp_protocol.h"
#include "agent_drv.h"
#include "agent.h"

struct WCH_BUF_IN {
    u32 u4VirYAddr;
    u32 u4VirCAddr;
    u32 u4PhyYAddr;
    u32 u4PhyCAddr;
};

struct WCH_BUF_IN bufin;
extern HANDLE  g_critEvtLock;
extern struct mutex dvplock;

#define WriteRegAP(addr, data)  ((*(volatile u32 *)(addr)) = (u32)(data))
#define ReadRegAP(addr)         (*(volatile u32 *)(addr))
#define IO_BASE_PT110           (IO_BASE_VA + 0X1F008)

struct DVPAGENT_INST_T *_arDVPAgentInsTable[MAX_DVPAGENT_INS_CNT];

bool DVP_Init(void)
{
    u32 i = 0;
    pr_info("[dvp][drv] DVP_Init\r\n");

    for (i = 0; i < MAX_DVPAGENT_INS_CNT; i++)
        _arDVPAgentInsTable[i] = NULL;

    if (!DVPAgent_Init()) {
        pr_err("[dvp][drv] DVPAgent_Init fail [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }
    return TRUE;
}


bool DVP_Deinit(void)
{
    u32 i = 0;
    pr_info("[dvp][drv] DVP_Deinit enter\r\n");
    mutex_lock(&dvplock);
    for (i = 0; i < MAX_DVPAGENT_INS_CNT; i++) {
        if (NULL == _arDVPAgentInsTable[i])
            continue;

        if (0 != _arDVPAgentInsTable[i]->hMsgQ) {
            VERIFY(x_msg_q_delete(_arDVPAgentInsTable[i]->hMsgQ)
                == OSR_OK);
            _arDVPAgentInsTable[i]->hMsgQ = 0;
        }

        vfree(_arDVPAgentInsTable[i]);
        _arDVPAgentInsTable[i] = NULL;
    }
    mutex_unlock(&dvplock);
    DVPAgent_Deinit();
    return TRUE;
}


u32 DVP_Open(u32 hDeviceContext, u32 AccessCode, u32 ShareMode)
{
    u32 i = 0;
    pr_info("[dvp][drv] DVP_Open enter.\r\n");

    #ifdef MM_SUPPORT_DIVXHT31
    loadDrmData();
    #endif

    mutex_lock(&dvplock);
    for (i = 0; i < MAX_DVPAGENT_INS_CNT; i++) {
        if (!_arDVPAgentInsTable[i]) {
            _arDVPAgentInsTable[i] =
                (struct DVPAGENT_INST_T *)vmalloc(sizeof(struct DVPAGENT_INST_T));
            if (!_arDVPAgentInsTable[i]) {
                pr_err("[dvp][drv] vmalloc _arDVPAgentInsTable[%d] error.[file = %s function = %s lineNo = %d]\r\n", i, FILE_ONLY, __func__, __LINE__);
                return FALSE;
            }
            memset(_arDVPAgentInsTable[i], 0,
                sizeof(struct DVPAGENT_INST_T));
            _arDVPAgentInsTable[i]->dwID = i;
            sprintf((s8 *)_arDVPAgentInsTable[i]->szMsgQName,
                "DVPAgentMsgQ%c", i+_T('0'));
            pr_info("[dvp][drv] DVPAgentMsgQ name is %s\r\n",
                (s8 *)_arDVPAgentInsTable[i]->szMsgQName);
            break;
        }
    }
    mutex_unlock(&dvplock);

    if (MAX_DVPAGENT_INS_CNT == i) {
        pr_err("[dvp][drv] MAX_DVPAGENT_INS_CNT [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    pr_info("[dvp][drv] DVP_Open Success.\r\n");

    return (u32)_arDVPAgentInsTable[i];
}


bool DVP_Close(u32 hOpenContext)
{
    struct DVPAGENT_INST_T *pIns = NULL;
    u32                     u4InstId = 0;

    pr_info("[dvp][drv][mod] DVP_Close ++++++++++++++ entry!\r\n");

    pIns = (struct DVPAGENT_INST_T *)hOpenContext;
    if (NULL == pIns) {
        pr_err("[dvp][drv] DVP_Close: hOpenContext is NULL! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return TRUE;
    }
    DVPHost_ChangeDeviceIdle();

    pr_debug("[dvp][drv][mod] DVP_Close ++++++++++++++ 2\r\n");

    mutex_lock(&dvplock);
    if (0 != pIns->hMsgQ) {
        if (x_msg_q_delete(pIns->hMsgQ) == OSR_OK)
            pr_debug("[dvp][drv][mod] DVP_Close ++++++++++++++ x_msg_q_delete(pIns->hMsgQ) success\r\n");
        else
            pr_debug("[dvp][drv][mod] DVP_Close ++++++++++++++ x_msg_q_delete(pIns->hMsgQ) fail\r\n");
        pIns->hMsgQ = 0;
    } else {
        pr_debug("[dvp][drv][mod] DVP_Close ++++++++++++++ pIns->hMsgQ is zero!\r\n");
    }

    u4InstId = pIns->dwID;
    if (NULL != _arDVPAgentInsTable[u4InstId]) {
        vfree(_arDVPAgentInsTable[u4InstId]);
        _arDVPAgentInsTable[u4InstId] = NULL;
    }
    mutex_unlock(&dvplock);
    return TRUE;
}

static void DVP2APMix(u8 fgMix)
{
    u32 dwApReg = 0;
    if (2 == fgMix) {
        pr_debug("[dvp][drv] DVP2APMIX Before REG[0X7014] = 0X%08X, REG[1F000] = 0X%08X\r\n",
            ReadRegAP(IO_BASE_VA + 0X7104),
            ReadRegAP(IO_BASE_VA + 0X1F000));

        dwApReg = ReadRegAP(IO_BASE_VA + 0X7104);
        dwApReg = dwApReg & 0XFFFF00FF;
        dwApReg = dwApReg | 0X0000BA00;
        WriteRegAP(IO_BASE_VA + 0X7104, dwApReg);
        dwApReg = 0;
        dwApReg = ReadRegAP(IO_BASE_VA + 0X1F000);
        dwApReg |= 0X00000082;
        WriteRegAP(IO_BASE_VA + 0X1F000, dwApReg);

        pr_debug("[dvp][drv] DVP2APMIX REG[0X7014] = 0X%08X, REG[1F000] = 0X%08X\r\n",
            ReadRegAP(IO_BASE_VA + 0X7104),
            ReadRegAP(IO_BASE_VA + 0X1F000));
    } else if (1 == fgMix) {
        dwApReg = ReadRegAP(IO_BASE_VA + 0X7104);
        dwApReg = dwApReg & 0XF0FF00FF;
        dwApReg = dwApReg | 0X0A00CB00;
        WriteRegAP(IO_BASE_VA + 0X7104, dwApReg);
    } else {
        dwApReg = ReadRegAP(IO_BASE_VA + 0X1F000);
        dwApReg &= 0XFFFFFF7D;
        WriteRegAP(IO_BASE_VA + 0X1F000, dwApReg);

        pr_debug("[dvp][drv] DVP2APMIX REG[0X7014] = 0X%08X, REG[1F000] = 0X%08X\r\n",
            ReadRegAP(IO_BASE_VA + 0X7104),
            ReadRegAP(IO_BASE_VA + 0X1F000));
    }
}

bool DVP_IOGetEvent(char *pBufOut, u32 dwLenOut, u32 *pdwActualOut,
    struct DVPAGENT_INST_T *pIns)
{
    u32    dwErr = 0;
    u32    i = 0;
    struct MSG_STRUCT_T pMsg;
    struct Dvp_Data_Len *pDvpDataLen = NULL;
    u16    u2MsgIdx = 0;
    SIZE_T zMsgSize = sizeof(struct MSG_STRUCT_T);
    u32    u4InstId = 0;
    s32    i4Ret = 0;
    pr_debug("[dvp][drv] DVPAGENT_IOCTL_GETEVENT\n");
    if (!pBufOut || (dwLenOut <= 0)) {
        pr_err("[dvp][drv] Get Event Param Err! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }
    if (!pIns) {
        if (pdwActualOut)
            *pdwActualOut = 0;
        pr_err("[dvp][drv] Get Event Param Err! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }
    u4InstId = pIns->dwID;

    memset(&pMsg, 0, sizeof(struct MSG_STRUCT_T));
    i4Ret = x_msg_q_receive_timeout(&u2MsgIdx,
        (void *)(&pMsg), &zMsgSize, &(pIns->hMsgQ), 1, 200);
    if (i4Ret == OSR_OK) {
        if (pMsg.u4Len) {
            pr_debug("[dvp][drv] GetEvent dwlenin %d:", dwLenOut);
            for (i = 0; i < 10; i++)
                pr_debug("[dvp][drv] %02X", pMsg.uDVPData[i]);
            pr_debug("\n");
            if (pMsg.u4Len > dwLenOut)
                copy_to_user(pBufOut, pMsg.uDVPData, dwLenOut);
            else
                copy_to_user(pBufOut, pMsg.uDVPData, pMsg.u4Len);
            if (pdwActualOut) {
                pDvpDataLen =
                    (struct Dvp_Data_Len *)pdwActualOut;
                pDvpDataLen->dwDataLen = pMsg.u4Len;
                pDvpDataLen->fgFromPT = pMsg.fgFromPT;
            }
        } else {
            if (pdwActualOut)
                *pdwActualOut = 0;
        }

    } else if (i4Ret == OSR_TIMEOUT) {
    #ifndef __linux__
        pr_debug("[dvp][drv]DVP_IOGetEvent: err = %s(%d)!\r\n", strerrno(errno), errno);
    #endif
        return FALSE;
    } else {
        #ifndef __linux__
            pr_debug("[dvp][drv]DVP_IOGetEvent : err = %s(%d)!\r\n", strerrno(errno), errno);
        #endif
        return FALSE;
    }
    return TRUE;
}

u32 DVPHost_avswitch(u8 *puData)
{
    u8 av = 0;
    u8 fr = 0;
    u8 oc = 0;

    if (puData != NULL) {
        struct AP2DVPPACKET_T *prPacket =
            (struct AP2DVPPACKET_T *)puData;
        av = prPacket->uParam1;
        fr = prPacket->uParam2;
        oc = prPacket->uParam3;
        pr_debug("[dvp][drv] av: %d, fr: %d, oc: %d\n", av, fr, oc);
        if (AP_CMD_AV_SWITCH  == prPacket->uCmd) {
            if (av == 1) {
                if (fr == 1 && oc == 1)
                    dvp_open_audio(true);
                else if (fr == 1 && oc == 2)
                    dvp_close_audio(true);
                else if (fr == 2 && oc == 1)
                    dvp_open_audio(false);
                else if (fr == 2 && oc == 2)
                    dvp_close_audio(false);
            } else if (av == 2) {
                if (fr == 1 && oc == 1)
                    ;//dvp_open_video();
                else if (fr == 1 && oc == 2)
                    ;//dvp_close_video();
            }
        }

    } else {
        pr_info("[dvp][drv] DVPHost_Write8032 puData is NULL!\n");
    }
}

/*static BOOL fgSend = FALSE; */
bool DVP_IOControl(u32 hOpenContext, u32 dwCode, u8 *pBufIn, u32 dwLenIn,
        s8 *pBufOut, u32 dwLenOut, u32 *pdwActualOut)
{
    u32 hr = 0;
    struct DVPAGENT_INST_T *pIns = (struct DVPAGENT_INST_T *)hOpenContext;
    u8     *inbuffer = NULL;
    u8     *outbuffer = NULL;
    bool   bRet = FALSE;

    if (dwCode != DVPAGENT_IOCTL_WRITEDATA2DRAM
        && dwCode !=  DVPAGENT_IOCTL_READDATAFROMDRAM) {
        if (pBufIn && dwLenIn > 0) {
            inbuffer = (BYTE *)vmalloc(dwLenIn);
            if (inbuffer == NULL) {
                pr_err("[dvp][drv]vmalloc inbuffer err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                return FALSE;
            }
        }

        if (pBufOut && dwLenOut > 0) {
            outbuffer = (BYTE *)vmalloc(dwLenOut);
            if (outbuffer == NULL) {
                if (inbuffer) {
                    vfree(inbuffer);
                    inbuffer = NULL;
                }
                pr_err("[dvp][drv]vmalloc outbuffer err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                return FALSE;
            }
        }

        if (!access_ok(VERIFY_WRITE, (void __user *)pBufIn, dwLenIn)) {
            pr_err("[dvp][drv]access_ok pBufIn err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            goto ERROR;
        }
        copy_from_user(inbuffer, pBufIn, dwLenIn);
    }

    pr_debug("[dvp][drv] ioctrl dwcode = %d\n", dwCode);

    switch (dwCode) {
    case DVPAGENT_IOCTL_SENDCMD:
        pr_debug("[dvp][drv] DVPAGENT_IOCTL_SENDCMD\n");
        if (!inbuffer)
            break;

        hr = DVPAgent_SendCmd(1, (struct AP2DVPCMD_T *)inbuffer);
        if (hr != RET_OK) {
            *outbuffer = (u8)hr;
            goto ERROR;
        }
        break;

    case DVPAGENT_IOCTL_CREATEMSGQUEUE:
        {
            u32 u4InstId = 0;
            u32 i4RetTmp = 0;
            if (NULL == pIns) {
                pr_err("[dvp][drv] DVPAGENT_IOCTL_CREATEMSGQUEUE: hOpenContext is NULL! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                goto ERROR;
            }
            u4InstId = pIns->dwID;

            mutex_lock(&dvplock);
            i4RetTmp = x_msg_q_create(
                &(_arDVPAgentInsTable[u4InstId]->hMsgQ),
                (s8 *)_arDVPAgentInsTable[u4InstId]->szMsgQName,
                sizeof(struct MSG_STRUCT_T), 50);
            mutex_unlock(&dvplock);
            if (OSR_EXIST == i4RetTmp) {
                DVPHost_ChangeDeviceIdle();
                pr_debug("[dvp][drv][mod] DVPAGENT_IOCTL_CREATEMSGQUEUE ++++++++++++++ x_msg_q_create(pIns->hMsgQ) OSR_EXIST ++++++++++++++++++\r\n");
                return TRUE;
            } else if (i4RetTmp != OSR_OK) {
                pr_debug("[dvp][drv][mod] DVPAGENT_IOCTL_CREATEMSGQUEUE ++++++++++++++ x_msg_q_create(pIns->hMsgQ) fail\r\n");
                return FALSE;
            }
            pr_info("[dvp][drv][mod] DVPAGENT_IOCTL_CREATEMSGQUEUE ++++++++++++++ x_msg_q_create(pIns->hMsgQ) success\r\n");
        }
        break;

    case DVPAGENT_IOCTL_GETEVENT:
        {
            u32    dwErr = 0;
            u32    i = 0;
            struct MSG_STRUCT_T pMsg;
            struct Dvp_Data_Len *pDvpDataLen = NULL;
            u16    u2MsgIdx = 0;
            SIZE_T zMsgSize = sizeof(struct MSG_STRUCT_T);
            u32    u4InstId = 0;
            s32    i4Ret = 0;
            u32    ret      = 0;

            pr_debug("[dvp][drv] DVPAGENT_IOCTL_GETEVENT\n");

            if (!outbuffer || (dwLenOut <= 0)) {
                pr_err("[dvp][drv] Get Event Param Err! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                goto ERROR;
            }
            if (!pIns) {
                if (pdwActualOut) {
                    if (!access_ok(VERIFY_WRITE, (void __user *)pdwActualOut, sizeof(u32))) {
                        pr_err("access_ok pdwActualOut err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                        goto ERROR;
                    }
                    copy_to_user(pdwActualOut, &ret, sizeof(u32));
                }
                pr_err("[dvp][drv] Get Event Param Err! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                goto ERROR;
            }
            u4InstId = pIns->dwID;

            memset(&pMsg, 0, sizeof(struct MSG_STRUCT_T));
            i4Ret = x_msg_q_receive_timeout(&u2MsgIdx, (void *)(&pMsg), &zMsgSize,
                &(pIns->hMsgQ), 1, 200);
            if (i4Ret == OSR_OK) {
                if (pMsg.u4Len)
                {
                    pr_debug("[dvp][drv] GetEvent dwlenin %d:", dwLenOut);
                    for (i=0; i<10; i++) {
                        pr_debug(" %02X", pMsg.uDVPData[i]);
                    }
                    pr_debug("\n");
                    if (pMsg.u4Len > dwLenOut) {
                        memcpy(outbuffer, pMsg.uDVPData, dwLenOut);
                    } else {
                        memcpy(outbuffer, pMsg.uDVPData, pMsg.u4Len);
                    }
                    if (pdwActualOut) {
                        pDvpDataLen = (struct Dvp_Data_Len *)pdwActualOut;

                        if (!access_ok(VERIFY_WRITE, (void __user *)pDvpDataLen, sizeof(struct Dvp_Data_Len))) {
                            pr_debug("access_ok pDvpDataLen err\n");
                            goto ERROR;
                        }
                        copy_to_user(&(pDvpDataLen->dwDataLen), (&(pMsg.u4Len)), sizeof(u32));
                        copy_to_user(&(pDvpDataLen->fgFromPT), (&(pMsg.fgFromPT)), sizeof(bool));
                    }
                }
                else
                {
                    if (pdwActualOut)
                    {
                        if (!access_ok(VERIFY_WRITE, (void __user *)pdwActualOut, sizeof(u32))) {
                            pr_err("[dvp][drv]access_ok pdwActualOut err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                            goto ERROR;
                        }
                        copy_to_user(pdwActualOut, &ret, sizeof(u32));
                    }
                }

            }
            else if (i4Ret == OSR_TIMEOUT) {
            #ifndef __linux__
                pr_debug("[dvp][drv]DVP_IOControl: err = %s(%d)!\r\n", strerrno(errno), errno);
            #endif
                goto ERROR;
            }
            else
            {
            #ifndef __linux__
                pr_debug("[dvp][drv]DVP_IOControl: err = %s(%d)!\r\n", strerrno(errno), errno);
            #endif
                goto ERROR;
            }
        }
        break;

    case DVPAGENT_IOCTL_WRITE:
        pr_debug("[dvp][drv] DVPAGENT_IOCTL_WRITE\n");
        {
             DVPHost_Write8032(inbuffer, dwLenIn);
        }
        break;

    case DVPAGENT_IOCTL_WRITEPT110:
        pr_debug("[dvp][drv] DVPAGENT_IOCTL_WRITEPT110\n");
        {
            struct AP2DVPPACKET_T *pAP2PT = NULL;
            if (!inbuffer) {
                pr_err("[dvp][drv] DVPAGENT_IOCTL_WRITEPT110, pBufIn is NULL! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                break;
            }

            pAP2PT = (struct AP2DVPPACKET_T *)inbuffer;

            if (pAP2PT && (AP2PT_AP_MIX2DVP == pAP2PT->uCmd)) {
                if (pAP2PT->uParam1 == 0) {
                    WriteRegAP(IO_BASE_PT110,
                        (ReadRegAP(IO_BASE_PT110) |
                        (1 << 8)));
                    pr_debug("[dvp][drv] 0X1F008 %08X\r\n",
                        ReadRegAP(IO_BASE_PT110));
                } else {
                    WriteRegAP(IO_BASE_PT110,
                        (ReadRegAP(IO_BASE_PT110) &
                        (0XFFFFFEFF)));
                    pr_debug("[dvp][drv] 0X1F008 %08X\r\n",
                        ReadRegAP(IO_BASE_PT110));
                }
            }
            DVPHost_WritePT110(inbuffer, dwLenIn);
        }
        break;

    case DVPAGENT_IOCTL_DVP2APMIX:
        {
            u8 fgMix = 0;
            pr_debug("[dvp][drv] DVPAGENT_IOCTL_DVP2APMIX\n");
            if (!inbuffer)
                break;
            fgMix = (u8)*inbuffer;
            DVP2APMix(fgMix);
        }
        break;

    case DVPAGENT_IOCTL_QUERYID:
        pr_debug("[dvp][drv] DVPAGENT_IOCTL_QUERYID");
        *(u8 *)outbuffer = 1;
        break;

    case DVPAGENT_IOCTL_SETRIPPATH:
        if (inbuffer) {
            struct DVP_Rip_Path *pRipPath =
                (struct DVP_Rip_Path *)inbuffer;
            DVPAgent_SetRipPath(pRipPath);
        } else {
            goto ERROR;
        }
        break;

    case DVPAGENT_IOCTL_COPYLYRICSBUF:
        if (outbuffer)
            DVPAgent_CopyLyricsBuf(outbuffer, dwLenOut);
        else
            goto ERROR;
        break;

    case DVPAGENT_IOCTL_COPYID3PICBUF:
        if (outbuffer)
            DVPAgent_CopyId3PicBuf(outbuffer, dwLenOut);
        else
            goto ERROR;
        break;

    case DVPAGENT_IOCTL_COPYRIPDATABUF:
        if (!outbuffer || dwLenOut <= 0)
            break;
        DVPAgent_CopyRipData(outbuffer, dwLenOut);
        break;

    case DVPAGENT_IOCTL_SETDVPLOG:
        if (inbuffer)
            DVPAgent_SetDVPLog((struct DVP_LOG_SET *)inbuffer);
        else
            goto ERROR;
        break;

    case DVPAGENT_IOCTL_WRITEDATA2DRAM:
        if (pBufIn)
            DVPAgent_WriteData2Dram((struct DVP_Dram_Data *)pBufIn,
                dwLenIn);
        else
            goto ERROR;
        break;

    case DVPAGENT_IOCTL_READDATAFROMDRAM:
        if (pBufOut)
            DVPAgent_ReadDataFromDram(
            (struct DVP_Dram_Data *)pBufOut, dwLenOut);
        else
            goto ERROR;
        break;

    case DVPAGENT_IOCTL_READFLINFOFROMDRAM:
        if (outbuffer  && inbuffer) {
            pr_debug("[dvp][drv] DVP_IOControl 0x112 starting\r\n");
            DVPAgent_ReadFileInfoFromFsIO((
                struct DVP_FILEITEM_INFO_T *)outbuffer, dwLenOut,
                *(u32 *)inbuffer);
        } else {
            pr_err("[dvp][drv] DVP_IOControl 0x112 starting failure for pBufOut && pBufIn is NULL [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            goto ERROR;
        }
        break;

    case DVPAGENT_IOCTL_SETDVPCODEPAGE:
        if (inbuffer)
            DVPAgent_SetCodePage(*((u32 *)inbuffer));
        else
            goto ERROR;
        break;

    case DVPAGENT_IOCTL_AVSWITCH:
        DVPHost_avswitch(inbuffer);
        break;

    case DVPAGENT_IOCTL_SET_SPRCTRUM:
        dvp_informAudioSampleRate(inbuffer);
        break;

    case DVPAGENT_IOCTL_GET_SPRCTRUM:
        dvp_getAudioSpectrum(outbuffer, dwLenOut);
        break;

    case DVPAGENT_IOCTL_VIDEOINFO:
        {
            u8 ocvideo = (u8)*inbuffer;
            pr_info("[dvp][drv] openVideo: %d\n", ocvideo);

            if (ocvideo == 1)
                dvp_open_video((WCH_BUFF_INFO_T *)outbuffer, dwLenOut);
            else if (ocvideo == 0)
                dvp_close_video();
        }
        break;

    case DVPAGENT_IOCTL_GETINDEX:
        dvp_getWcDVpIndex((void *)outbuffer, dwLenOut);
        break;

    default:
        pr_debug("[dvp][drv] DVPAGENT_IOCTL_DEFAULT");
        goto ERROR;
    }

    bRet = TRUE;

    if (dwCode != DVPAGENT_IOCTL_WRITEDATA2DRAM &&
        dwCode !=  DVPAGENT_IOCTL_READDATAFROMDRAM) {
        if (bRet && outbuffer != NULL) {
            if (!access_ok(VERIFY_WRITE, (void __user *)pBufOut, dwLenOut)) {
                pr_err("[dvp][drv]access_ok pBufIn err [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
                goto ERROR;
            }
            copy_to_user(pBufOut, outbuffer, dwLenOut);
        }
    }

ERROR:
    if (inbuffer != NULL) {
        vfree(inbuffer);
        inbuffer = NULL;
    }
    if (outbuffer != NULL) {
        vfree(outbuffer);
        outbuffer = NULL;
    }
    return bRet;
}

