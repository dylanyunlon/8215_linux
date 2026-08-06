/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/** @file fci_type.h
 *  All types related to FCI are declared in this file.
 */

#ifndef SD_TYPE_H
#define SD_TYPE_H

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------



/*
 * Error code.
 */

#define S_OK							0
#define E_CMD_TIMEOUT                   1
#define E_NO_CARD                       2
#define E_SD_R3_OCR_BUSY                3
#define E_INVALID_CARD                  5
#define E_INVALID_BLOCK_SIZE           	6 	/* over 2048 or not multiple of 4 */
#define E_DATA_CRC_ERROR                7
#define E_CMD_RSP_CRC_ERR               9
#define E_DATA_TIMEOUT                  10
#define E_STATUS                        11
#define E_INVALID_ARG                   12	/* Joshua add */
#define E_SD_APPCMD_FAILED              13
#define E_RESPBUSY                      14	/* Joshua add */
#define E_NO_RESPONSE                   15
#define E_SD_NOT_SUPPORT_WIDE_BUS       16
#define E_UNKNOWN_CARD                  20
#define E_MS_CMD_CRC_ERROR             	21
#define E_MS_SET_RW_REG_ADRS         	22
#define E_CARD_IS_LOCKED                60
#define E_ALLOC_BUFFER_ERROR            61
#define E_CARD_READ_ERROR               62
#define E_CARD_WRITE_ERROR              63 
#define E_FAIL                          0xFF

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

#endif // SD_TYPE_H
