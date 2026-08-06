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


#ifndef _DRV_UART_H_
#define _DRV_UART_H_

#include "x_typedef.h"
//#include "drv_dbg.h"
#include "x_dbg_drv.h"
//#include "x_drv_cb.h"
#include "x_handle.h"
//#include "x_hal_ic.h"
#include "drv_config.h"


// *********************************************************************
// UART General Define
// *********************************************************************

//#define UART_MODEM

#define UART_PORT_NUM 3

#define UART_PORT_0  0
#define UART_PORT_1  1
#define UART_PORT_2  2

#define UART_SUCC	 ((INT32)1)
#define UART_FAIL	 ((INT32)-1)

//only for linux build
#if (CONFIG_DRV_LINUX)    
#define  UART_SUPPORT_FILE_OP 1
#else
#define  UART_SUPPORT_FILE_OP 0
#endif

#ifndef _X_RS_232_H_

/* Flags used during registration */
#define RS_232_FLAG_REC_DATA_BUFFER  ((UINT32) 0x01000000)
#define RS_232_FLAG_XMT_DATA_BUFFER  ((UINT32) 0x02000000)
#define RS_232_FLAG_CTRL_LINE        ((UINT32) 0x04000000)
#define RS_232_FLAG_CTRL_LINE_NFY    ((UINT32) 0x08000000)


/* Control line flags */
#define RS_232_CTRL_LINE_CTS   ((UINT8) 0x01)
#define RS_232_CTRL_LINE_RTS   ((UINT8) 0x02)
#define RS_232_CTRL_LINE_DCD   ((UINT8) 0x04)
#define RS_232_CTRL_LINE_DSR   ((UINT8) 0x08)
#define RS_232_CTRL_LINE_DTR   ((UINT8) 0x10)
#define RS_232_CTRL_LINE_RI    ((UINT8) 0x20)


/* Data length */
typedef enum
{
    RS_232_DATA_LEN_4 = 0,
    RS_232_DATA_LEN_5,
    RS_232_DATA_LEN_6,
    RS_232_DATA_LEN_7,
    RS_232_DATA_LEN_8,
    RS_232_DATA_LEN_9
}   RS_232_DATA_LEN_T;

/* Parity */
typedef enum
{
    RS_232_PARITY_NONE = 0,
    RS_232_PARITY_EVEN,
    RS_232_PARITY_ODD,
    RS_232_PARITY_MARK,
    RS_232_PARITY_SPACE
}   RS_232_PARITY_T;

/* Data speed */
typedef enum
{
    RS_232_SPEED_75 = 0,
    RS_232_SPEED_110,
    RS_232_SPEED_134,
    RS_232_SPEED_150,
    RS_232_SPEED_300,
    RS_232_SPEED_600,
    RS_232_SPEED_1200,
    RS_232_SPEED_1800,
    RS_232_SPEED_2400,
    RS_232_SPEED_4800,
    RS_232_SPEED_9600,
    RS_232_SPEED_14400,
    RS_232_SPEED_19200,
    RS_232_SPEED_28800,
    RS_232_SPEED_38400,
    RS_232_SPEED_57600,
    RS_232_SPEED_115200,
    RS_232_SPEED_128000,
    RS_232_SPEED_230400,
    RS_232_SPEED_460800,
    RS_232_SPEED_921600
}   RS_232_SPEED_T;

/* Stop bit */
typedef enum
{
    RS_232_STOP_BIT_1 = 0,
    RS_232_STOP_BIT_1_5,
    RS_232_STOP_BIT_2
}   RS_232_STOP_BIT_T;


/* Capabilities */
#if 0
#define RS_232_CAP_DATA_LEN_4  (((UINT32) 1) << RS_232_DATA_LEN_4)
#define RS_232_CAP_DATA_LEN_5  (((UINT32) 1) << RS_232_DATA_LEN_5)
#define RS_232_CAP_DATA_LEN_6  (((UINT32) 1) << RS_232_DATA_LEN_6)
#define RS_232_CAP_DATA_LEN_7  (((UINT32) 1) << RS_232_DATA_LEN_7)
#define RS_232_CAP_DATA_LEN_8  (((UINT32) 1) << RS_232_DATA_LEN_8)
#define RS_232_CAP_DATA_LEN_9  (((UINT32) 1) << RS_232_DATA_LEN_9)

#define RS_232_CAP_PARITY_NONE   (((UINT32) 1) << RS_232_PARITY_NONE)
#define RS_232_CAP_PARITY_EVEN   (((UINT32) 1) << RS_232_PARITY_EVEN)
#define RS_232_CAP_PARITY_ODD    (((UINT32) 1) << RS_232_PARITY_ODD)
#define RS_232_CAP_PARITY_MARK   (((UINT32) 1) << RS_232_PARITY_MARK)
#define RS_232_CAP_PARITY_SPACE  (((UINT32) 1) << RS_232_PARITY_SPACE)

#define RS_232_CAP_SPEED_75     (((UINT32) 1) << RS_232_SPEED_75)
#define RS_232_CAP_SPEED_110    (((UINT32) 1) << RS_232_SPEED_110)
#define RS_232_CAP_SPEED_134    (((UINT32) 1) << RS_232_SPEED_134)
#define RS_232_CAP_SPEED_150    (((UINT32) 1) << RS_232_SPEED_150)
#define RS_232_CAP_SPEED_300    (((UINT32) 1) << RS_232_SPEED_300)
#define RS_232_CAP_SPEED_600    (((UINT32) 1) << RS_232_SPEED_600)
#define RS_232_CAP_SPEED_1200   (((UINT32) 1) << RS_232_SPEED_1200)
#define RS_232_CAP_SPEED_1800   (((UINT32) 1) << RS_232_SPEED_1800)
#define RS_232_CAP_SPEED_2400   (((UINT32) 1) << RS_232_SPEED_2400)
#define RS_232_CAP_SPEED_4800   (((UINT32) 1) << RS_232_SPEED_4800)
#define RS_232_CAP_SPEED_9600   (((UINT32) 1) << RS_232_SPEED_9600)
#define RS_232_CAP_SPEED_14400  (((UINT32) 1) << RS_232_SPEED_14400)
#define RS_232_CAP_SPEED_19200  (((UINT32) 1) << RS_232_SPEED_19200)
#define RS_232_CAP_SPEED_28800  (((UINT32) 1) << RS_232_SPEED_28800)
#define RS_232_CAP_SPEED_38400  (((UINT32) 1) << RS_232_SPEED_38400)
#define RS_232_CAP_SPEED_57600  (((UINT32) 1) << RS_232_SPEED_57600)
#define RS_232_CAP_SPEED_115200 (((UINT32) 1) << RS_232_SPEED_115200)
#define RS_232_CAP_SPEED_128000 (((UINT32) 1) << RS_232_SPEED_128000)
#define RS_232_CAP_SPEED_230400 (((UINT32) 1) << RS_232_SPEED_230400)
#define RS_232_CAP_SPEED_460800 (((UINT32) 1) << RS_232_SPEED_460800)
#define RS_232_CAP_SPEED_921600 (((UINT32) 1) << RS_232_SPEED_921600)

#define RS_232_CAP_STOP_BIT_1    (((UINT32) 1) << RS_232_STOP_BIT_1)
#define RS_232_CAP_STOP_BIT_1_5  (((UINT32) 1) << RS_232_STOP_BIT_1_5)
#define RS_232_CAP_STOP_BIT_2    (((UINT32) 1) << RS_232_STOP_BIT_2)

#else

#define RS_232_CAP_DATA_LEN_4  (((UINT32) 1) << ((UINT32)RS_232_DATA_LEN_4))
#define RS_232_CAP_DATA_LEN_5  (((UINT32) 1) << ((UINT32)RS_232_DATA_LEN_5))
#define RS_232_CAP_DATA_LEN_6  (((UINT32) 1) << ((UINT32)RS_232_DATA_LEN_6))
#define RS_232_CAP_DATA_LEN_7  (((UINT32) 1) << ((UINT32)RS_232_DATA_LEN_7))
#define RS_232_CAP_DATA_LEN_8  (((UINT32) 1) << ((UINT32)RS_232_DATA_LEN_8))
#define RS_232_CAP_DATA_LEN_9  (((UINT32) 1) << ((UINT32)RS_232_DATA_LEN_9))

#define RS_232_CAP_PARITY_NONE   (((UINT32) 1) << ((UINT32)RS_232_PARITY_NONE))
#define RS_232_CAP_PARITY_EVEN   (((UINT32) 1) << ((UINT32)RS_232_PARITY_EVEN))
#define RS_232_CAP_PARITY_ODD    (((UINT32) 1) << ((UINT32)RS_232_PARITY_ODD))
#define RS_232_CAP_PARITY_MARK   (((UINT32) 1) << ((UINT32)RS_232_PARITY_MARK))
#define RS_232_CAP_PARITY_SPACE  (((UINT32) 1) << ((UINT32)RS_232_PARITY_SPACE))

#define RS_232_CAP_SPEED_75     (((UINT32) 1) << ((UINT32)RS_232_SPEED_75))
#define RS_232_CAP_SPEED_110    (((UINT32) 1) << ((UINT32)RS_232_SPEED_110))
#define RS_232_CAP_SPEED_134    (((UINT32) 1) << ((UINT32)RS_232_SPEED_134))
#define RS_232_CAP_SPEED_150    (((UINT32) 1) << ((UINT32)RS_232_SPEED_150))
#define RS_232_CAP_SPEED_300    (((UINT32) 1) << ((UINT32)RS_232_SPEED_300))
#define RS_232_CAP_SPEED_600    (((UINT32) 1) << ((UINT32)RS_232_SPEED_600))
#define RS_232_CAP_SPEED_1200   (((UINT32) 1) << ((UINT32)RS_232_SPEED_1200))
#define RS_232_CAP_SPEED_1800   (((UINT32) 1) << ((UINT32)RS_232_SPEED_1800))
#define RS_232_CAP_SPEED_2400   (((UINT32) 1) << ((UINT32)RS_232_SPEED_2400))
#define RS_232_CAP_SPEED_4800   (((UINT32) 1) << ((UINT32)RS_232_SPEED_4800))
#define RS_232_CAP_SPEED_9600   (((UINT32) 1) << ((UINT32)RS_232_SPEED_9600))
#define RS_232_CAP_SPEED_14400   (((UINT32) 1) << ((UINT32)RS_232_SPEED_14400))
#define RS_232_CAP_SPEED_19200   (((UINT32) 1) << ((UINT32)RS_232_SPEED_19200))
#define RS_232_CAP_SPEED_28800   (((UINT32) 1) << ((UINT32)RS_232_SPEED_28800))
#define RS_232_CAP_SPEED_38400   (((UINT32) 1) << ((UINT32)RS_232_SPEED_38400))
#define RS_232_CAP_SPEED_57600   (((UINT32) 1) << ((UINT32)RS_232_SPEED_57600))
#define RS_232_CAP_SPEED_115200 (((UINT32) 1) << ((UINT32)RS_232_SPEED_115200))
#define RS_232_CAP_SPEED_128000 (((UINT32) 1) << ((UINT32)RS_232_SPEED_128000))
#define RS_232_CAP_SPEED_230400 (((UINT32) 1) << ((UINT32)RS_232_SPEED_230400))
#define RS_232_CAP_SPEED_460800 (((UINT32) 1) << ((UINT32)RS_232_SPEED_460800))
#define RS_232_CAP_SPEED_921600 (((UINT32) 1) << ((UINT32)RS_232_SPEED_921600))

#define RS_232_CAP_STOP_BIT_1    (((UINT32) 1) << ((UINT32)RS_232_STOP_BIT_1))
#define RS_232_CAP_STOP_BIT_1_5  (((UINT32) 1) << ((UINT32)RS_232_STOP_BIT_1_5))
#define RS_232_CAP_STOP_BIT_2    (((UINT32) 1) << ((UINT32)RS_232_STOP_BIT_2))

#endif

/* Capability structure */
typedef struct _RS_232_CAPABILITY_INFO_T
{
    UINT32  ui4_speed;
    UINT32  ui4_data_len;
    UINT32  ui4_parity;
    UINT32  ui4_stop_bit;
}   RS_232_CAPABILITY_INFO_T;


/* Notify condition */
typedef enum
{
    RS_232_COND_REC_DATA = 0,
    RS_232_COND_REC_BUFFER,
    RS_232_COND_REC_FRAME_ERROR,
    RS_232_COND_REC_OVERFLOW,
    RS_232_COND_XMT,
    RS_232_COND_XMT_EMPTY,
    RS_232_COND_CTRL_CHG
}   RS_232_COND_T;


/* Data xfer structure */
typedef struct _RS_232_MULTI_DATA_INFO_T
{
    UINT32  z_data_len;
    
    UINT8*  pui1_data;
}   RS_232_MULTI_DATA_INFO_T;


#if UNIFORM_DRV_CALLBACK

typedef struct _RS_232_NFY_FCT_DATA_T
{
     RS_232_COND_T  e_nfy_cond;
     UINT32         ui4_data;
} RS_232_NFY_FCT_DATA_T;

#else

/* Notify function */
typedef VOID (*x_rs_232_nfy_fct) (VOID*          pv_nfy_tag,
                                  RS_232_COND_T  e_nfy_cond,
                                  UINT32         ui4_data);
/* Notify setting structure */
typedef struct _RS_232_NFY_INFO_T
{
    VOID*  pv_tag;
    
    x_rs_232_nfy_fct  pf_rs_232_nfy;

}   RS_232_NFY_INFO_T;

#endif


/* Setup structure */
typedef struct _RS_232_SETUP_INFO_T
{
    RS_232_SPEED_T     e_speed;
    RS_232_DATA_LEN_T  e_data_len;
    RS_232_PARITY_T    e_parity;
    RS_232_STOP_BIT_T  e_stop_bit;
}   RS_232_SETUP_INFO_T;

#endif /* _X_RS_232_H_ */

typedef void (*PF_MODEM_NOTIFY_T) (void);

// *********************************************************************
// API list
// *********************************************************************

extern void UART_SwUart1PollPutChars(UINT8 * paData, UINT8 ucNumberToWrite);
extern void UART_SwUart2PollPutChars(UINT8 * paData, UINT8 ucNumberToWrite);
extern void UART_SwStopUartThread(void);
extern UINT32 UART_SwGetRcvDataLen(UINT8 u1Port);
extern INT32 UART_SwInit(void);
extern INT32 UART_Init (void);
extern INT32 UART_Uninit(UINT32 u4Case);
extern INT32 UART_SetParameter (UINT8 u1Port, const RS_232_SETUP_INFO_T * rSetupInfo);
extern INT32 UART_GetParameter (UINT8 u1Port, RS_232_SETUP_INFO_T * rSetupInfo);
extern INT32 UART_FlushBuffer (UINT8 u1Port);
extern INT32 UART_GetControlLine (UINT8 u1Port, UINT8 * ucGet);
extern UINT32 UART_GetRcvDataLen (UINT8 u1Port);
extern void UART_GetCapability (UINT8 u1Port, RS_232_CAPABILITY_INFO_T * prCapability);
extern UINT32 UART_Read(UINT8 u1Port, UINT8 * pBuffer, UINT32 u4NumToRead, BOOL fgBlockedMode);
extern UINT32 UART_Write(UINT8 u1Port, const UINT8* pData, UINT32 u4NumberToWrite, BOOL fgBlockedMode);
extern UINT32 UART_GetTxFreeLen (UINT8 u1Port);
extern void UART_SetControlLine (UINT8 u1Port, UINT8 ucSet);
#if UNIFORM_DRV_CALLBACK
extern void UART_SetNotifyFuc(UINT8 u1Port, const DRV_CB_REG_INFO_T* prRs232NfyInfo);
#else
extern void UART_SetNotifyFuc(UINT8 u1Port, const RS_232_NFY_INFO_T* prRs232NfyInfo);
#endif

extern BOOL UART_CheckMagicChar(UINT8 ucChar);
extern INT32 UART_InputSwitch(void);
extern void UART_SetDebugPortFactory(BOOL fgSet);
extern UINT32 UART_ReadDataInIsr(UINT8 u1Port, UINT8 * pBuffer, UINT32 u4NumToRead);
extern INT32 DBG_FactorySetCallback(BOOL fgEnable, const PF_DBG_INPUT_T pfInputCb);
extern VOID UART_PurgeRxClean(UINT8 u1Port);
extern VOID UART_PurgeTxClean(UINT8 u1Port);
extern void UART_FlushTxSwFifo(UINT8 u1Port);
extern UINT32 UART_WriteByte(UINT8 u1Port, const UINT8 u1Data);


#ifdef DEBUG
#ifdef EXT_DBG_DEV
extern INT32 DBG_SetCallback(BOOL fgEnable, PF_DBG_INPUT_T pfInputCb);
#endif
#endif

#ifdef UART_MODEM

extern INT32 UART_ModemCtl(UINT8 u1Port, BOOL fgRTS, BOOL fgDTR);
extern INT32 UART_SetModemStatusChgNotify(PF_MODEM_NOTIFY_T pfModemNotify);
extern INT32 UART_ModemCmd(UCHAR ucCmd);
extern INT32 UART_WriteModem(const CHAR * pucData, UINT32 u4Len);
extern BOOL UART_ModemInit(void);

#endif

#if  UART_SUPPORT_FILE_OP

//#ifndef HANDLE_T
//typedef UINT32  HANDLE_T;
//#endif

extern UINT32 x_uart_fopen (
        const CHAR *ps_path, 
        HANDLE_T  *ph_file );

extern UINT32 x_uart_fclose (
        HANDLE_T  h_file );

extern UINT32 x_uart_fread (
        HANDLE_T        h_file,
        VOID            *pv_data,
        UINT32          ui4_count,
        UINT32          *pui4_read);

extern UINT32 x_uart_fwrite (
        HANDLE_T        h_file,
        const VOID      *pv_data,
        UINT32          ui4_count,
        UINT32          *pui4_write);
extern INT32 x_uart_feof (
        HANDLE_T        h_file,
        BOOL            *pb_eof);

extern UINT32 x_uart_fGetlength(
        HANDLE_T        h_file,
        UINT32          *pb_Length);

#endif

#endif  // _DRV_UART_H_
