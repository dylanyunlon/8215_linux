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


#ifndef SMC_DRV_H
#define SMC_DRV_H

#include "x_typedef.h"

#ifdef SMC_DEBUG
	#define __STR2WSTR(str) L##str
	#define _STR2WSTR(str) __STR2WSTR(str)
	#define __FUNCTIONW__ _STR2WSTR(__FUNCTION__)
	
    #define SMC_FUNC_ENTRY              RETAILMSG(1, (TEXT("[SMC] ++++++ %s Enter ++++++\r\n"), __FUNCTIONW__))
    #define SMC_FUNC_EXIT               RETAILMSG(1, (TEXT("[SMC] ------ %s Exit ------\r\n"), __FUNCTIONW__))
#else

    #define SMC_FUNC_ENTRY
    #define SMC_FUNC_EXIT
#endif  // FUNC_PROFILE

#if 0
#define SMC_LOG_ERR(arg)			RETAILMSG(1, arg)
#define SMC_LOG_WRN(arg)			RETAILMSG(1, arg)
#define SMC_LOG_INFO(arg)			RETAILMSG(1, arg)
#define SMC_LOG_DBG(arg)			RETAILMSG(1, arg)
#define SMC_LOG_DUMP(arg)			RETAILMSG(0, arg)
#else
#define SMC_LOG_ERR(arg)
#define SMC_LOG_WRN(arg)
#define SMC_LOG_INFO(arg)
#define SMC_LOG_DBG(arg)
#define SMC_LOG_DUMP(arg)
#endif


typedef struct _SMC_CMD_SEND_T {
    UINT8 block_type;      /* telling the type of the block( info,receive,supervisory ) */
    UINT8 ins_code;        /* instruction code */
    UINT8 ns;              /* send sequence number */
} SMC_CMD_SEND_T;

typedef  struct _SMC_CMD_RESPONSE_T {
    UINT8          protocol_number;                /* Version of IC Card */
    UINT8          unit_length;                    /* Length of payload */
    UINT16         card_instruction;               /* Instruction from Smart Card */
    UINT16         return_code;                    /* Process result of Smart card */
} SMC_CMD_RESPONSE_T;

typedef enum {
    SMC_CARD_PRESENT,                  /* Card Present                       */
    SMC_CARD_ABSENT,                   /* Card Absent                        */
    SMC_CARD_INIT_FAIL,                  /* Card Invalid                       */
    SMC_CARD_ACTIVATED,                  /* Card Present                       */    
    SMC_CARD_DEACTIVATED,                  /* Card Present                       */        
    SMC_CARD_ERROR                     /* Card invalid                         */
} SMC_CARD_STATUS_T;

typedef struct {
    UINT16 ecm_pid;
    UINT8  odd_key[8];
    UINT8  even_key[8];
    UINT8  smc_slot_nbr;
    UINT32 session_id;
    UINT8  card_id[6];
} SMC_BCAS_SCRAMBLE_KEYS_T;

typedef struct {
    UINT8 maker_identification;            /* Maker identification               */
    UINT8 version;                         /* Version                            */
    UINT8 id_identification;               /* Identification                     */
    UINT8 group_id[6];          /* Group id                           */
    UINT16 check_code;                     /* Check code                         */
} SMC_BCAS_GROUP_ID_T;

typedef struct {
    UINT8 maker_identification;            /* Maker identification               */
    UINT8 version;                         /* Version                            */
    UINT8 id_identification;               /* Identification                     */
    UINT8 ind_card_id[6];    /* Individual Card id                 */
    UINT16 check_code;                     /* Check code                         */
} SMC_BCAS_IND_CARD_ID_T;

typedef enum {
    GENERAL_CARD,                       /* General Card                       */
    PREPAID_CARD                        /* Prepaid Card                       */
} SMC_BCAS_CARD_TYPE_T;

typedef struct {
    UINT8 smc_slot_nbr;                    /* Smartcard slot number              */
    UINT8 card_id[6];         /* Card id of smartcard               */
    UINT16 ca_system_id;                   /* CA System ID                       */
    UINT8 no_of_grp_id;                    /* Number of Group id                 */
    UINT8 system_key[32];         /* Card id of smartcard               */    
    UINT8 cbc_initial_vector[8];         /* Card id of smartcard               */        
    SMC_BCAS_IND_CARD_ID_T ind_id;          /* Individual Card ID                 */
    SMC_BCAS_GROUP_ID_T grp_id[6]; /* Group ID                  */
    SMC_BCAS_CARD_TYPE_T card_type;
    SMC_CARD_STATUS_T card_status;    
} SMC_BCAS_CARD_INFO_T;

#define SMCR_OK             			((INT32)  0)
#define SMCR_INV_ARG        			((INT32) -1)
#define SMCR_FAIL           			((INT32) -2)
#define SMCR_CARD_WRITE_ERROR     		((INT32) -3)
#define SMCR_CARD_READ_ERROR     		((INT32) -4)
#define SMCR_LRC_ERROR     				((INT32) -5)
#define SMCR_CARD_ERROR     			((INT32) -6)
#define SMCR_NO_CARD        			((INT32) -7)

typedef void (* PFN_SMC_HOTPLUG_CALLBACK)(UINT8 u1HotPlugStatus);

// IOCTL Definitions
#define STREAM_DEVICE_SMC					0x2000

#define DEF_IOCTL_FUNC_CODE(FuncNum)			\
	CTL_CODE(STREAM_DEVICE_SMC, FuncNum, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
#define SMC_IOCTL_RESET						DEF_IOCTL_FUNC_CODE(0x101)
#define SMC_IOCTL_ACTIVE					DEF_IOCTL_FUNC_CODE(0x102)
#define SMC_IOCTL_DEACTIVE					DEF_IOCTL_FUNC_CODE(0x103)
#define SMC_IOCTL_SET_HOTPLUG_CB			DEF_IOCTL_FUNC_CODE(0x104)
#define SMC_IOCTL_SEND_RECEIVE_APDU			DEF_IOCTL_FUNC_CODE(0x105)
#define SMC_IOCTL_REQUEST_IFS				DEF_IOCTL_FUNC_CODE(0x106)
#define SMC_IOCTL_GET_CARD_ID				DEF_IOCTL_FUNC_CODE(0x107)
#define SMC_IOCTL_INIT_SETTING				DEF_IOCTL_FUNC_CODE(0x108)
#define SMC_IOCTL_SEND_ECM					DEF_IOCTL_FUNC_CODE(0x109)

typedef struct {
	UINT8 	u1SlotNbr; 
  	UINT8*	pu1SendData;
  	UINT8 	u1SendLength;
  	UINT8*	pu1ResponseData;
  	UINT8*	pu1ResponseLength;    
} SMC_SEND_RECEIVE_APDU_T;

typedef struct {
	UINT8	u1SlotNbr;
	UINT8* 	pu1EcmData;
	UINT16  u2Length;
  	SMC_BCAS_SCRAMBLE_KEYS_T *prKeys;    
} SMC_SEND_ECM_T;

// Export Interface Functions
EXTERN_C DWORD SMC_Init(VOID *pContext); 

EXTERN_C BOOL SMC_Open(DWORD hDeviceContext, DWORD AccessMode, DWORD ShareMode );

EXTERN_C BOOL SMC_Close(DWORD hDeviceContext);

EXTERN_C BOOL SMC_IOControl(
			  					DWORD dwContext,
			  					DWORD dwIOCode,
			  					UCHAR *pInBuffer,
			  					DWORD dwInSize,
			  					UCHAR *pOutBuffer,
			  					DWORD dwOutSize,
			  					DWORD *pdwOutSize);

EXTERN_C VOID SMC_Deinit(DWORD hDeviceContext);                                                     

#endif //SMC_DRV_H

