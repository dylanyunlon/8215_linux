#include "dvpcomhw.h"
#include <linux/fs.h>

static HANDLE ghReceiveEvent8032;
static HANDLE ghReceiveEventPT110;
static HANDLE ghTimeOutEvent;
static HANDLE ghQuitThreadEvent;

#define SEND_CMD_TIMEOUT    (8000)

#define VBIM_READ32(offset)           IO_READ32(BIM_UCV_BASE, offset)
#define VBIM_WRITE32(offset, value)   IO_WRITE32(BIM_UCV_BASE, offset, (value))

bool _fg8032SetParmeterGroup0(u32 u4P1, u32 u4P2, u32 u4P3, u32 u4P4)
{
    VBIM_WRITE32(AP_8032_WSTA1, u4P2);
    VBIM_WRITE32(AP_8032_WSTA2, u4P3);
    VBIM_WRITE32(AP_8032_WSTA3, u4P4);
    VBIM_WRITE32(AP_8032_WSTA0, u4P1); /*must be last*/
    return TRUE;
}

bool _fg8032GetParmeterGroup0(u32 *pu4P1, u32 *pu4P2, u32 *pu4P3, u32 *pu4P4)
{
    *pu4P1 = VBIM_READ32(AP_8032_RSTA0);
    *pu4P2 = VBIM_READ32(AP_8032_RSTA1);
    *pu4P3 = VBIM_READ32(AP_8032_RSTA2);
    *pu4P4 = VBIM_READ32(AP_8032_RSTA3);
    return TRUE;
}

bool _fgPT110SetParmeterGroup0(u32 u4P1, u32 u4P2, u32 u4P3, u32 u4P4)
{
    VBIM_WRITE32(AP_PT110_WSTA1, u4P2);
    VBIM_WRITE32(AP_PT110_WSTA2, u4P3);
    VBIM_WRITE32(AP_PT110_WSTA3, u4P4);
    VBIM_WRITE32(AP_PT110_WSTA0, u4P1); /*must be last*/
    return TRUE;
}

bool _fgPT110GetParmeterGroup0(u32 *pu4P1, u32 *pu4P2, u32 *pu4P3, u32 *pu4P4)
{
    *pu4P1 = VBIM_READ32(AP_PT110_RSTA0);
    *pu4P2 = VBIM_READ32(AP_PT110_RSTA1);
    *pu4P3 = VBIM_READ32(AP_PT110_RSTA2);
    *pu4P4 = VBIM_READ32(AP_PT110_RSTA3);
    return TRUE;
}

void INTTo8032(void)
{
    pr_debug("[dvp][drv] fgINTTo8032 AP -> 8032");
    VBIM_WRITE32(AP_DVD_INT, AP2UP_INT0|AP2UP_BSY0);
}

void INTToPT110(void)
{
    pr_debug("[dvp][drv] fgINTToPT110 AP -> PT110");
    VBIM_WRITE32(AP_DVD_INT, AP2PT_INT0|AP2PT_BSY0);
}

static irqreturn_t DVP8032_InterruptThread(s32 u2VectorId, void *dev_id)
{
    x_event_set(ghReceiveEvent8032);
    ac83xx_mask_ack_bim_irq(u2VectorId);

    return IRQ_HANDLED;
}

static irqreturn_t DVPPT110_InterruptThread(s32 u2VectorId, void *dev_id)
{
    x_event_set(ghReceiveEventPT110);
    ac83xx_mask_ack_bim_irq(u2VectorId);

    return IRQ_HANDLED;
}

bool DVPComHW_Init(void)
{
    pr_info("[dvp][drv] DVPComHW_Init");

    /* Prepare 8032 2 AP irq list   VECTOR_UP2AP0; */
    if (request_irq(VECTOR_UP2AP0, DVP8032_InterruptThread,
            IRQF_TRIGGER_NONE, "dvp", NULL) != OSR_OK) {
        DVP_ASSERT(0);
        pr_err("[dvp][drv] Register 8032 ISR Failed! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
    }

    /* Prepare PT2AP irq list       VECTOR_PT110AP0; */
    if (request_irq(VECTOR_PT110AP0, DVPPT110_InterruptThread,
            IRQF_TRIGGER_NONE, "dvp", NULL) != OSR_OK) {
        DVP_ASSERT(0);
        pr_err("[dvp][drv] Register PT110 ISR Failed! [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
    }

    /* Create ghReceiveEvent8032 event */
    ghReceiveEvent8032 = x_event_create(0, FALSE, FALSE, TEXT("8032Event"));
    if (ghReceiveEvent8032 == NULL) {
        pr_err("[dvp][drv] DVPComHw_Init() Error creating ghReceiveEvent8032 event:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    /*  Create ghReceiveEventPT110 event */
    ghReceiveEventPT110 = x_event_create(0, FALSE, FALSE, TEXT("PT110Event"));
    if (ghReceiveEventPT110 == NULL) {
        pr_err("[dvp][drv] DVPComHw_Init() Error creating ghReceiveEventPT110 event: [file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    /* Create ghTimeOutEvent event */
    ghTimeOutEvent = x_event_create(0, FALSE, FALSE, TEXT("TimeOutEvent"));
    if (ghTimeOutEvent == NULL) {
        pr_err("[dvp][drv] DVPComHw_Init() Error creating ghTimeOutEvent event:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    /* Create ghQuitThreadEvent event */
    ghQuitThreadEvent = x_event_create(0, FALSE, FALSE,
        TEXT("QuitThreadEvent"));
    if (ghQuitThreadEvent == NULL) {
        pr_err("[dvp][drv] DVPComHw_Init() Error creating ghQuitThreadEvent event:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
        return FALSE;
    }

    pr_info("[dvp][drv] DVPComHW_Init success\n ");
    return TRUE;
}

bool DVPComHW_Deinit()
{
    pr_info("[dvp][drv] DVPComHW_Deinit\n");

    free_irq(VECTOR_UP2AP0, NULL);

    free_irq(VECTOR_PT110AP0, NULL);

    pr_debug("[dvp][drv] set ghQuitThreadEvent!\n");
    x_event_set(ghQuitThreadEvent);

    return TRUE;
}

u32 DVPComHW_SendData8032(u8 *pData, u32 dwSize)
{
    u32   tmp[4] = {0};
    u8    *pTmp;
    u32   i;
    u32   u8timeout = get_jiffies_64() + SEND_CMD_TIMEOUT;
    ASSERT(dwSize <= 16);

    pTmp = (BYTE *)tmp;
    for (i = 0; i < dwSize; i++)
        pTmp[i] = pData[i];

    while (VBIM_READ32(AP_DVD_INT) & AP2UP_BSY0) {
        if (!time_before((unsigned long)get_jiffies_64(),
            (unsigned long)u8timeout)) {
            pr_err("[dvp][drv][com]Send cmd to 8032 timeout!:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            x_event_set(ghTimeOutEvent);
            break;
        }
        Sleep(1);
    }
    _fg8032SetParmeterGroup0(*tmp, *(tmp+1), *(tmp+2), *(tmp+3));
    INTTo8032();

    return dwSize;
}


u32 DVPComHW_SendDataPT110(u8 *pData, u32 dwSize)
{
    u32   tmp[4] = {0};
    u8    *pTmp;
    u32   i;
    u32   u8timeout = get_jiffies_64() + SEND_CMD_TIMEOUT;

    ASSERT(dwSize <= 16);

    pTmp = (BYTE *)tmp;
    for (i = 0; i < dwSize; i++)
        pTmp[i] = pData[i];

    while (VBIM_READ32(AP_DVD_INT) & AP2PT_BSY0) {
        if (!time_before((unsigned long)get_jiffies_64(),
            (unsigned long)u8timeout)) {
            pr_err("[dvp][drv][com]Send cmd to pt110 timeout!:[file = %s function = %s lineNo = %d]\r\n", FILE_ONLY, __func__, __LINE__);
            x_event_set(ghTimeOutEvent);
            break;
        }
        Sleep(1);
    }
    _fgPT110SetParmeterGroup0(*tmp, *(tmp+1), *(tmp+2), *(tmp+3));
    INTToPT110();

    return dwSize;
}

u32 DVPComHW_ReceiveData(u8 *pData, u32 dwSize, bool *fgFromPT)
{
    HANDLE hEvents[4];
    u32    dwNumEvents, dwStatus;
    u32    tmp[4];
    u8    *pTmp;

    memset(tmp, 0, sizeof(DWORD)*4);
    pTmp = (BYTE *)tmp;

    hEvents[0] = ghReceiveEvent8032;
    hEvents[1] = ghReceiveEventPT110;
    hEvents[2] = ghTimeOutEvent;
    hEvents[3] = ghQuitThreadEvent;
    dwNumEvents = 4;

    dwStatus = x_event_wait_for_objects(dwNumEvents,
        hEvents, FALSE, INFINITE);
    switch (dwStatus) {
    case (WAIT_OBJECT_0 + 0):
        pr_debug("DVPComHW_ReceiveData 8032");

        _fg8032GetParmeterGroup0(tmp, tmp+1, tmp+2, tmp+3);
        memcpy(pData, pTmp, dwSize);

        /*CLR busy status 0x7000824c*/
        IO_WRITE32(IO_BASE_VA, 0X824C,
            IO_READ32(IO_BASE_VA, 0X824C) | (1 << 4));
        if (fgFromPT)
            *fgFromPT = FALSE;
    break;

    case (WAIT_OBJECT_0 + 1):
        pr_debug("[dvp][drv] DVPComHW_ReceiveData PT110\n");
        _fgPT110GetParmeterGroup0(tmp, tmp+1, tmp+2, tmp+3);
        memcpy(pData, pTmp, dwSize);

        /*CLR pt110 busy status 0x7000824c*/
        IO_WRITE32(IO_BASE_VA, 0X824C,
            IO_READ32(IO_BASE_VA, 0X824C) | (1 << 20));
        if (fgFromPT)
            *fgFromPT = TRUE;
    break;

    case (WAIT_OBJECT_0 + 2):
        pr_debug("[dvp][drv] DVPComHW_ReceiveData get timeout event\r\n");
        pTmp[0] = 0XFF;
        pTmp[1] = 0XFE;
        pTmp[2] = 0X00;
        pTmp[3] = 0X00;
        pTmp[4] = 0X01;
        pTmp[5] = 0XF4;
        pTmp[6] = 0XF4;
        memcpy(pData, pTmp, dwSize);
        if (fgFromPT)
            *fgFromPT = FALSE;
    break;

    case (WAIT_OBJECT_0 + 3):
        if (ghQuitThreadEvent)
            x_event_destroy(ghQuitThreadEvent);

        if (ghReceiveEvent8032)
            x_event_destroy(ghReceiveEvent8032);

        if (ghReceiveEventPT110)
            x_event_destroy(ghReceiveEventPT110);

        if (ghTimeOutEvent)
            x_event_destroy(ghTimeOutEvent);
        dwSize = 0;
        if (fgFromPT)
            *fgFromPT = FALSE;
        pr_debug("[dvp][drv] receive ghQuitThreadEvent success!");
    break;

    default:
        pr_debug("[dvp][drv] DVPComHW_ReceiveData ERROR");
        break;
    }

    return dwSize;
}


