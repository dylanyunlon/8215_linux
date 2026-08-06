/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef _NFI_H
#define _NFI_H

#include "x_hal_ic.h"
#include "x_bim.h"

#define _SUPPORT_WP_

#define FDM_BYTES  6
#define FDM_ECC_BYTES 6
#define SECTOR_BYTES 512
/* Data Structures */
typedef struct _NFIType
{
   //UINT16   IOInterface;     /* IO_8BITS or IO_16BITS */
   UINT16   pageSize;        /* 512, 2048 */
   UINT16   spareSize;
   UINT16   addressCycle;   
   UINT16   pageShift;
} NFI_MENU;

typedef struct _NFIAccess
{
   UINT32  readAddr1;
   UINT32  readAddr2;
} NFI_ACCESS;

typedef struct _BOOTLHeader_
{
   char ID1[12];
   char version[4];
   UINT32 length;
   UINT32 startAddr;
   UINT32 checksum;
   char ID2[8];
   NFI_MENU  NFIinfo;
   UINT16 pagesPerBlock;   
   UINT16  totalBlocks;
   UINT16  blockShift;
   UINT16  linkAddr[6];   
   UINT16  lastBlock;
} BOOTL_HEADER;

#define NFI_CNFG                          ((volatile UINT16 *)(NFI_BASE+0x00))
    #define AHB_MODE        0x0001
    #define READ_MODE      0x0002
    #define OP_IDLE            0x0000
    #define OP_READ           0x1000
    #define OP_READ_ID_ST   0x2000
    #define OP_PROGRAM    0x3000
    #define OP_ERASE         0x4000
    #define OP_RESET         0x5000
    #define OP_CUSTOME    0x6000
    #define HW_ECC_EN      0x0100
    #define AUTO_FMT_EN  0x0200
  
#define NFI_PAGEFMT             ((volatile UINT16 *)(NFI_BASE+0x04))
    #define NAND_IO_8BITS      0x0000
    #define NAND_IO_16BITS     0x0008
    #define NAND_PSIZE(x)               (((UINT32) x & 0x03) << 0)
    #define NAND_PSIZE_528B             0
    #define NAND_PSIZE_2112B            1
    #define NAND_PSIZE_4096B       2
    #define FDM_NUM(x)                (((UINT32) x &0x0F) << 8)
    #define FDM_ECC_NUM(x)                (((UINT32) x &0x0F) << 12)
    #define SPARE_16  0x0000
    #define SPARE_26  0x0010
    #define SPARE_27  0x0020
    
#define NFI_CON                    ((volatile UINT16 *)(NFI_BASE+0x08))
    #define FIFO_FLUSH              ((UINT32) 1 << 0)
    #define NFI_RST             ((UINT32) 1 << 1)
    #define BURST_RD            ((UINT32) 1 << 8)
    #define BURST_WR            ((UINT32) 1 << 9)
    #define SINGLE_RD           ((UINT32) 1 << 4)
    #define NOB_DWORD                   0x0080
    #define NOB_WORD                    0x0040
    #define SEC_NUM(x)                (((UINT32) x &0x0F) << 12)
    
#define NFI_ACCCON                     ((volatile UINT32 *)(NFI_BASE+0x0C))
    #define LCD2NAND(x)   (((UINT32)x& 0xF) << 28)
    #define PRECS(x)     (((UINT32) x & 0x3f) << 22)
    #define C2R(x)          (((UINT32) x & 0x3f) << 16)
    #define W2R(x)          (((UINT32) x & 0x0f) << 12)
    #define WH(x)           (((UINT32) x & 0x0f) <<  8)
    #define WST(x)          (((UINT32) x & 0x0f) <<  4)
    #define RLT(x)          (((UINT32) x & 0x0f) <<  0)

#define NFI_INTR_EN                         ((volatile UINT16 *)(NFI_BASE+0x10))
   #define AHB_DONE_EN              0x40
   #define WR_DONE_EN               0x02
   #define RD_DONE_EN                0x01
   #define RESET_DONE_EN        0x04
   
#define NFI_INTR               ((volatile UINT16 *)(NFI_BASE+0x14))
  #define AHB_DONE                     0x40
  #define WR_DONE                      0x02
  #define RD_DONE                       0x01
  #define RESET_DONE                0x04
  
#define NFI_CMD                            ((volatile UINT16 *)(NFI_BASE+0x20))
    #define NAND_CMD_READ1_00           0x00
    #define NAND_CMD_READ1_01           0x01
    #define NAND_CMD_PROG_PAGE          0x10    /* WRITE 2 */
    #define NAND_CMD_READ_2K        0x30    // only for 2KB  page-size
    #define NAND_CMD_READ2              0x50
    #define NAND_CMD_ERASE1_BLK         0x60
    #define NAND_CMD_STATUS             0x70
    #define NAND_CMD_INPUT_PAGE         0x80    /* WRITE 1 */
    #define NAND_CMD_READ_ID            0x90
    #define NAND_CMD_ERASE2_BLK         0xD0
    #define NAND_CMD_RESET              0xFF

#define NFI_ADDRNOB                 ((volatile UINT16 *)(NFI_BASE+0x30))
  #define COL_ADDR_NOB(x)     (((UINT32) x & 0x07) << 0)
  #define ROW_ADDR_NOB(x)     (((UINT32) x & 0x07) << 4)

#define NFI_COLADDR                 ((volatile UINT32 *)(NFI_BASE+0x34))

#define NFI_ROWADDR                     ((volatile UINT32 *)(NFI_BASE+0x38))

#define NFI_DATAW                 ((volatile UINT32 *)(NFI_BASE+0x50))

#define NFI_DATAR                 ((volatile UINT32 *)(NFI_BASE+0x54))

#define NFI_STA                     ((volatile UINT32 *)(NFI_BASE+0x60))
    #define DATAW                       0x08
    #define BUSY                           0x0100
    #define NAND_STATUS_BUSY            ((UINT32) 1 << 8)
    #define NAND_STATUS_DTWR            ((UINT32) 1 << 3)
    #define NAND_STATUS_DTRD            ((UINT32) 1 << 2)
    #define NAND_STATUS_ADDR            ((UINT32) 1 << 1)
    #define NAND_STATUS_CMD             ((UINT32) 1 << 0)

    #define STATUS_CMD         0x1
    #define DATAW                       0x08
    #define BUSY                           0x0100
    
#define NFI_FIFOSTA                ((volatile UINT16 *)(NFI_BASE+0x64))
//  #define WB_FULL         ((UINT32) 1 << 7)
//  #define RB_EMPTY            ((UINT32) 1 << 6)
    #define WR_FULL         ((UINT32) 1 << 15)
    #define WR_EMPTY            ((UINT32) 1 << 14)
    #define RD_FULL         ((UINT32) 1 << 7)
    #define RD_EMPTY            ((UINT32) 1 << 6)

#define NFI_ADDRCNTR   ((volatile UINT16 *)(NFI_BASE+0x0070))

#define NFI_STRADDR    ((volatile UINT32 *)(NFI_BASE+0x0080))

#define NFI_BYTELEN     ((volatile UINT16 *)(NFI_BASE+0x0084))

#define NAND_ECCBLK0_ADDR           ((volatile UINT16 *)(NFI_BASE+0x50))
#define NAND_ECCBLK1_ADDR       ((volatile UINT16 *)(NFI_BASE+0x54))
#define NAND_ECCBLK2_ADDR           ((volatile UINT16 *)(NFI_BASE+0x58))
#define NAND_ECCBLK3_ADDR           ((volatile UINT16 *)(NFI_BASE+0x5c))
#define NAND_ECCBLK0_DATA           ((volatile UINT32 *)(NFI_BASE+0x60))
#define NAND_ECCBLK1_DATA           ((volatile UINT32 *)(NFI_BASE+0x64))
#define NAND_ECCBLK2_DATA           ((volatile UINT32 *)(NFI_BASE+0x68))
#define NAND_ECCBLK3_DATA           ((volatile UINT32 *)(NFI_BASE+0x6c))
#define NAND_ECC_PARITY0            ((volatile UINT16 *)(NFI_BASE+0x80))
#define NAND_ECC_PARITY1            ((volatile UINT16 *)(NFI_BASE+0x84))
#define NAND_ECC_PARITY2            ((volatile UINT16 *)(NFI_BASE+0x88))
#define NAND_ECC_PARITY3            ((volatile UINT16 *)(NFI_BASE+0x8c))
#define NAND_ECC_PARITY4            ((volatile UINT16 *)(NFI_BASE+0x90))
#define NAND_ECC_PARITY5            ((volatile UINT16 *)(NFI_BASE+0x94))
#define NAND_ECC_PARITY6            ((volatile UINT16 *)(NFI_BASE+0x98))
#define NAND_ECC_PARITY7            ((volatile UINT16 *)(NFI_BASE+0x9c))

#define NFI_FDM0L       ((volatile UINT32 *)(NFI_BASE+0xA0))
#define NFI_FDM0M      ((volatile UINT32 *)(NFI_BASE+0xA4))

#define NFI_ECC_RDY   ((volatile UINT32 *)(NFI_BASE+0xFFC))
      #define WAIT_RDY_MASK         0x1000

#define NFIECC_BASE        (IO_BASE + 0x1E800)

#define NFIECC_ENCCON      ((volatile UINT16 *)(NFIECC_BASE+0x0000))
    #define ENC_EN      0x01
    
#define NFIECC_ENCCNFG  ((volatile UINT32 *)(NFIECC_BASE+0x0004))
    #define ENC_TNUM(x)                (((UINT32) x &0x07))
    #define ENC_NFI_MODE             0x01 << 4
    #define ENC_MS(x)                    (((UINT32) x &0x1FFF) << 16)

#define NFIECC_ENCDIADDR      ((volatile UINT32 *)(NFIECC_BASE+0x0008))

#define NFIECC_ENCIDLE          ((volatile UINT16 *)(NFIECC_BASE+0x000C))
    #define ENC_IDLE                    0x01

#define NFIECC_ENCPAR0        ((volatile UINT32 *)(NFIECC_BASE+0x0010))

#define NFIECC_DECCON      ((volatile UINT16 *)(NFIECC_BASE+0x0100))
    #define DEC_EN      0x01

#define NFIECC_DECCNFG  ((volatile UINT32 *)(NFIECC_BASE+0x0104))
    #define DEC_TNUM(x)                ((((UINT32) x / 2) - 2))
    #define DEC_NFI_MODE             0x01 << 4
    #define DEC_CON(x)                    (((UINT32) x &0x03) << 12)     
    #define DEC_CS(x)                    (((UINT32) x &0x1FFF) << 16)
    #define DEC_EMPTY_EN             0x80000000

#define NFIECC_DECDIADDR      ((volatile UINT32 *)(NFIECC_BASE+0x0108))

#define NFIECC_DECIDLE          ((volatile UINT16 *)(NFIECC_BASE+0x010C))
    #define DEC_IDLE                    0x01

#define NFIECC_DECFER            ((volatile UINT16 *)(NFIECC_BASE+0x0110))

#define NFIECC_DECENUM            ((volatile UINT32 *)(NFIECC_BASE+0x0114))

#define NFIECC_DECDONE        ((volatile UINT16 *)(NFIECC_BASE+0x0118))

#define NFIECC_DECEL0           ((volatile UINT32 *)(NFIECC_BASE+0x11C))

#define NFIECC_DECIRQEN       ((volatile UINT16 *)(NFIECC_BASE+0x0134))
    #define DEC_IRQEN                0x01

#define NFIECC_FDMADDR        ((volatile UINT32 *)(NFIECC_BASE+0x013C))

#define NFI_ECC_RDY   ((volatile UINT32 *)(NFI_BASE+0xFFC))
      #define WAIT_RDY_MASK         0x1000

#define NFI_CLK_SEL       ((volatile UINT32 *)(CKGEN_BASE+0x68))

// NFI_CLK
#define NFI_CLK_27  0x0
#define NFI_CLK_162 0x50
#define NFI_CLK NFI_CLK_162
#define PDN_NFI 0x02000000

#define STATUS_WRITE_PROTECT            0x0
#define STATUS_READY_BUSY               0x40
#define STATUS_ERASE_SUSPEND            0x20
#define STATUS_PASS_FAIL                0x01

#define REPLICATION_NUMBER  8

typedef enum {
    ECC_4_BITS = 4,
    ECC_6_BITS = 6,
    ECC_8_BITS = 8,
    ECC_10_BITS = 10,
    ECC_12_BITS = 12
} ECC_Level_t;

typedef enum {
    ECC_DEC_NONE,
    ECC_DEC_DETECT,
    ECC_DEC_LOCATE,
    ECC_DEC_CORRECT
} ECC_Decode_Type_t;

typedef enum {
     S_DONE = 0
    ,S_TIMEOUT
    ,S_IN_PROGRESS  
    ,S_ECC_UNCORRECT_ERR
    ,S_ECC_CORRECTABLE_ERR  
    ,S_UNKNOWN_ERR
}STATUS_E;

/* Prototypes */
void NFI_Init(void);
//void NFI_Config(NFI_MENU *input);
STATUS_E NFI_ReadPage(UINT32 *pu4Buf, UINT32 u4PgIdx, BOOL fgEccCheck);
STATUS_E NFI_ReadSpare(NFI_MENU *pConfig, UINT32* pu4Data, UINT32 u4PgIdx);
//INT32 NFI_ReadBlockInfo(NFI_MENU *prInput, UINT32 u4PgIdx, UINT32* p4TmpData);

extern const NFI_MENU _rNFIRetrials[];
extern UINT32 _u4Config_no;

#define NUTL_DEFAULT_TIMEOUT    0x0000FFFF
#define NFI_Wait(condition_expression, timeout)     while( (condition_expression) && (--timeout) )
#define NFI_LOCK_SEMA() \
    BIM_GETHWSemaphore(HSMPHE_NAND,0)

#define NFI_UNLOCK_SEMA() \
    BIM_ReleaseHWSemaphore(HSMPHE_NAND)

#define NONCACHE(expression) (expression|0xC0000000)

#endif // _NFI_H
