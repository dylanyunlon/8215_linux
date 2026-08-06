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

#ifndef _HDCP2X_DRV_IF_H_
#define _HDCP2X_DRV_IF_H_

#include "windef.h"
#include "windev.h"

/*-----------------------------------------------------------------------------
                    macros, typedefs, enums
 ----------------------------------------------------------------------------*/
#define HDCP2X_OK                ((int)    0)
#define HDCP2X_FAILED            ((int)   -1)
#define HDCP2X_INV_HANDLE        ((int)   -3)
#define HDCP2X_INV_ARG           ((int)   -4)


// HDCP2 key
#define HDCP2X_KEYLEN_ENC 862
    
#define HDCP2X_KEYLEN_RXID (5)             // Receiver ID 40 bit Low Secure Storage
#define HDCP2X_KEYLEN_PUBLIC (128 + 3)     // Device Public Key 1024 + 24 bit Low Secure Storage
#define HDCP2X_KEYLEN_RESERVED (2)         // RESERVED
#define HDCP2X_KEYLEN_ROOTSIGN (384)       // Root Signature 3072 bit Low Secure Storage
    
#define HDCP2X_KEYLEN_EKPUB_KM 128
#define HDCP2X_KEYLEN_EKS 16
#define HDCP2X_KEYLEN_RTX 8
#define HDCP2X_KEYLEN_RRX 8
#define HDCP2X_KEYLEN_RRN 8
#define HDCP2X_KEYLEN_HPRIME 32
#define HDCP2X_KEYLEN_LPRIME 32
    
    
typedef struct
{
    unsigned char au1EncKey[HDCP2X_KEYLEN_ENC];
}HDCP2X_KEYENC_T;

typedef struct
{
    unsigned char au1RxID[HDCP2X_KEYLEN_RXID];
    unsigned char au1Public[HDCP2X_KEYLEN_PUBLIC];
    unsigned char au1Reserved[HDCP2X_KEYLEN_RESERVED];
    unsigned char au1RootSign[HDCP2X_KEYLEN_ROOTSIGN];
}HDCP2X_KEYCERT_T;

typedef struct
{
    unsigned char au1Ekpubkm[HDCP2X_KEYLEN_EKPUB_KM];
}HDCP2X_EKPUBKM_T;

typedef struct
{
    unsigned char au1rTx[HDCP2X_KEYLEN_RTX];
}HDCP2X_RTX_T;

typedef struct
{
    unsigned char au1rRx[HDCP2X_KEYLEN_RRX];
}HDCP2X_RRX_T;

typedef struct
{
    unsigned char au1rHprime[HDCP2X_KEYLEN_HPRIME];
    unsigned char u1Repeater;
    unsigned char au1rTx[HDCP2X_KEYLEN_RTX];

    unsigned char  rx_version;
    unsigned char  rx_cap_mask[2];
    unsigned char  tx_version;
    unsigned char  tx_cap_mask[2];
    
}HDCP2X_CALHPRIME_T;

typedef struct
{
    unsigned char au1rLprime[HDCP2X_KEYLEN_HPRIME];
    unsigned char au1rRn[HDCP2X_KEYLEN_RRN];
    unsigned char au1rRx[HDCP2X_KEYLEN_RRX];
}HDCP2X_CALLPRIME_T;

typedef struct
{
    unsigned char au1Eks[HDCP2X_KEYLEN_EKS];
    unsigned char au1Rtx[HDCP2X_KEYLEN_RTX];
    unsigned char au1Rrx[HDCP2X_KEYLEN_RRX];
    unsigned char au1Rn[HDCP2X_KEYLEN_RRN];
}HDCP2X_EKS_T;

typedef struct
{
    int ai4Arg[2];
    int i4Ret;
} HDCP2X_IOCTL_2ARG_T;

typedef struct
{
    int ai4Arg[4];
    int i4Ret;
} HDCP2X_IOCTL_4ARG_T;

typedef struct
{
    int ai4Arg[6];
    int i4Ret;
} HDCP2X_IOCTL_6ARG_T;


//IOCTL

#define CTL_HDCP_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)


#define IOCTL_HDCP2X_CMD_SET_ENC_KEY \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x500, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_GETCERTINFO \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x501, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_DECRYPT_RSAESOAEP \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x502, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_KDKEYDERIVATION \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x503, METHOD_BUFFERED, FILE_ANY_ACCESS)   // 0x2200c

#define IOCTL_HDCP2X_CMD_COMPUTE_HPRIME \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x504, METHOD_BUFFERED, FILE_ANY_ACCESS)   // 0x22010

#define IOCTL_HDCP2X_CMD_COMPUTE_LPRIME \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x505, METHOD_BUFFERED, FILE_ANY_ACCESS)   // 0x22014

#define IOCTL_HDCP2X_CMD_COMPUTE_KH \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x506, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_ENCRYPT_KM \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x507, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_DECRYPT_KM \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x508, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_DECRYPT_EKS \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x509, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_DECRYPT_PES \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x50A, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_GET_KSXORLC128 \
	CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x50B, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
#define IOCTL_HDCP2X_CMD_HDCP2_2_KDKEYDERIVATION \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x50C, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_HDCP2_2_COMPUTE_HPRIME \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x50D, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_HDCP2_2_COMPUTE_LPRIME \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x50E, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HDCP2X_CMD_HDCP2_2_DECRYPT_EKS \
    CTL_HDCP_CODE(FILE_DEVICE_UNKNOWN, 0x50F, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif //_X_CIPHSV_H_

