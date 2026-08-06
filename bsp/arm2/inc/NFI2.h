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


#ifndef _NFI2_H
#define _NFI2_H

#include "x_hal_ic.h"
#include "x_bim.h"
#include "mtd_nand.h"

#define INT_WR_CLR

#define FDM2_BYTES  9
#define FDM2_ECC_BYTES 9
#define SECTOR_BYTES 512

#define  IO_8BITS   0x0000
#define  IO_16BITS  0x0001




#define NFI_base        (IO_BASE + 0x1E400)
#define NFI_CNFG                          ((volatile UINT16 *)(NFI_base+0x00))
    #define AHB_MODE        0x0001
    #define READ_MODE      0x0002
    #define SEL_SEC_512BYTE 0x0020
    #define OP_IDLE            0x0000
    #define OP_READ           0x1000
    #define OP_READ_ID_ST   0x2000
    #define OP_PROGRAM    0x3000
    #define OP_ERASE         0x4000
    #define OP_RESET         0x5000
    #define OP_CUSTOME    0x6000
    #define HW_ECC_EN      0x0100
    #define AUTO_FMT_EN  0x0200
  
#define NFI_PAGEFMT             ((volatile UINT16 *)(NFI_base+0x04))
    #define NAND_IO_8BITS      0x0000
    #define NAND_IO_16BITS     0x0008
    #define NAND_PSIZE(x)               (((UINT32) x & 0x03) << 0)
    #define NAND_PSIZE_2K_512             0
    #define NAND_PSIZE_4K_2K            1
    #define NAND_PSIZE_8K_4K       2
    #define FDM2_NUM(x)                (((UINT32) x &0x1F) << 6)
    #define FDM2_ECC_NUM(x)                (((UINT32) x &0x1F) << 11)
    #define SPARE_16  0x0000
    #define SPARE_26  0x0010
    #define SPARE_27  0x0020
    
#define NFI_CON                    ((volatile UINT16 *)(NFI_base+0x08))
    #define FIFO_FLUSH              ((UINT32) 1 << 0)
    #define NFI_RST             ((UINT32) 1 << 1)
    #define BURST_RD            ((UINT32) 1 << 8)
    #define BURST_WR            ((UINT32) 1 << 9)
    #define SINGLE_RD           ((UINT32) 1 << 4)
    #define NOB_DWORD                   0x0080
    #define NOB_WORD                    0x0040
    #define NOB_DWORD                 0x0080    
    #define SEC_NUM(x)                (((UINT32) x &0x0F) << 12)
    
#define NFI_ACCCON                     ((volatile UINT32 *)(NFI_base+0x0C))
    #define LCD2NAND(x)   (((UINT32)x& 0xF) << 28)
    #define PRECS(x)     (((UINT32) x & 0x3f) << 22)
    #define C2R(x)          (((UINT32) x & 0x3f) << 16)
    #define W2R(x)          (((UINT32) x & 0x0f) << 12)
    #define WH(x)           (((UINT32) x & 0x0f) <<  8)
    #define WST(x)          (((UINT32) x & 0x0f) <<  4)
    #define RLT(x)          (((UINT32) x & 0x0f) <<  0)

#define NFI_INTR_EN                         ((volatile UINT16 *)(NFI_base+0x10))
   #define AHB_DONE_EN              0x40
   #define WR_DONE_EN               0x02
   #define RD_DONE_EN                0x01
   #define RESET_DONE_EN        0x04
   
#define NFI_INTR               ((volatile UINT16 *)(NFI_base+0x14))
  #define AHB_DONE                     0x40
  #define WR_DONE                      0x02
  #define RD_DONE                       0x01
  #define RESET_DONE                0x04
  
#define NFI_CMD                            ((volatile UINT16 *)(NFI_base+0x20))
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


#define NFI_ADDRNOB                 ((volatile UINT16 *)(NFI_base+0x30))
  #define COL_ADDR_NOB(x)     (((UINT32) x & 0x07) << 0)
  #define ROW_ADDR_NOB(x)     (((UINT32) x & 0x07) << 4)

#define NFI_COLADDR                 ((volatile UINT32 *)(NFI_base+0x34))

#define NFI_ROWADDR                     ((volatile UINT32 *)(NFI_base+0x38))

#define NFI_DATAW                 ((volatile UINT32 *)(NFI_base+0x50))

#define NFI_DATAR                 ((volatile UINT32 *)(NFI_base+0x54))

#define NFI_STA                     ((volatile UINT32 *)(NFI_base+0x60))
#define NAND_STATUS_BUSY            ((UINT32) 1 << 8)
#define NAND_STATUS_DTWR            ((UINT32) 1 << 3)
#define NAND_STATUS_DTRD            ((UINT32) 1 << 2)
#define NAND_STATUS_ADDR            ((UINT32) 1 << 1)
#define NAND_STATUS_CMD             ((UINT32) 1 << 0)

    #define STATUS_CMD         0x1
    #define DATAW                       0x08
    #define BUSY                           0x0100
    
#define NFI_FIFOSTA                ((P_U16)(NFI_base+0x64))
//  #define WB_FULL         ((UINT32) 1 << 7)
//  #define RB_EMPTY            ((UINT32) 1 << 6)
    #define WR_FULL         ((UINT32) 1 << 15)
    #define WR_EMPTY            ((UINT32) 1 << 14)
    #define RD_FULL         ((UINT32) 1 << 7)
    #define RD_EMPTY            ((UINT32) 1 << 6)

#define NFI_ADDRCNTR   ((UINT16 *)(NFI_base+0x0070))

#define NFI_STRADDR    ((volatile UINT32 *)(NFI_base+0x0080))

#define NFI_BYTELEN     ((volatile UINT16 *)(NFI_base+0x0084))

#define NFI_FDM0L       ((volatile UINT32 *)(NFI_base+0x200))
#define NFI_FDM0M      ((volatile UINT32 *2)(NFI_base+0x204))

#define NFI_ECC_RDY   ((volatile UINT32 *)(NFI_base+0xFFC))
      #define WAIT_RDY_MASK         0x1000

#define NFIECC_BASE        (IO_BASE + 0x1EC00)

#define NFIECC_ENCCON      ((UINT16 *)(NFIECC_BASE+0x0000))
    #define ENC_EN      0x01
    
#define NFIECC_ENCCNFG  ((UINT32 *)(NFIECC_BASE+0x0004))
    #define ENC_TNUM(x)                (((UINT32) x &0x07))
    #define ENC_NFI_MODE             0x01 << 4
    #define ENC2_MS(x)                    (((UINT32) x &0x3FFF) << 16)

#define NFIECC_ENCDIADDR      ((UINT32 *)(NFIECC_BASE+0x0008))

#define NFIECC_ENCIDLE          ((UINT16 *)(NFIECC_BASE+0x000C))
    #define ENC_IDLE                    0x01

#define NFIECC_ENCPAR0        ((UINT32 *)(NFIECC_BASE+0x0010))

#define NFIECC_DECCON      ((UINT16 *)(NFIECC_BASE+0x0100))
    #define DEC_EN      0x01

#define NFIECC_DECCNFG  ((UINT32 *)(NFIECC_BASE+0x0104))
    #define DEC_TNUM(x)                ((((UINT32) x / 2) - 2))
    #define DEC_NFI_MODE             0x01 << 4
    #define DEC_CON(x)                    (((UINT32) x &0x03) << 12)     
    #define DEC2_CS(x)                    (((UINT32) x &0x3FFF) << 16)
    #define DEC_EMPTY_EN             0x80000000

#define NFIECC_DECDIADDR      ((UINT32 *)(NFIECC_BASE+0x0108))

#define NFIECC_DECIDLE          ((UINT16 *)(NFIECC_BASE+0x010C))
    #define DEC_IDLE                    0x01

#define NFIECC_DECFER            ((volatile UINT16 *)(NFIECC_BASE+0x0110))

#define NFIECC_DECENUM            ((UINT32 *)(NFIECC_BASE+0x0150))

#define NFIECC_DECENUM2            ((UINT32 *)(NFIECC_BASE+0x0154))

#define NFIECC_DECDONE        ((UINT16 *)(NFIECC_BASE+0x0118))

#define NFIECC_DECEL0           ((UINT32 *)(NFIECC_BASE+0x160))

#define NFIECC_DECIRQEN       ((UINT16 *)(NFIECC_BASE+0x0134))
    #define DEC_IRQEN                0x01

#define NFIECC_FDMADDR        ((UINT32 *)(NFIECC_BASE+0x013C))

#define NFI_CLK_SEL       ((volatile UINT32 *)(CKGEN_BASE+0x70))

// NFI_CLK
#define PDN_NFI 0x00080000
#define NFI_CLK_144 0x00030000
#define NFI_CLK_27  0x0
#define NFI_CLK_200 0x00050000
#define NFI_CLK_234 0x00010000
#define NFI_CLK NFI_CLK_144

#define STATUS_WRITE_PROTECT            0x0
#define STATUS_READY_BUSY               0x40
#define STATUS_ERASE_SUSPEND            0x20
#define STATUS_PASS_FAIL                0x01

#define REPLICATION_NUMBER  8

typedef enum {
    ECC2_4_BITS = 4,
    ECC2_6_BITS = 6,
    ECC2_8_BITS = 8,
    ECC2_10_BITS = 10,
    ECC2_12_BITS = 12,
    ECC2_22_BITS = 22,
    ECC2_24_BITS = 24
} ECC2_Level_t;

typedef enum {
    ECC2_DEC_NONE,
    ECC2_DEC_DETECT,
    ECC2_DEC_LOCATE,
    ECC2_DEC_CORRECT
} ECC2_Decode_Type_t;

/* Prototypes */
void NFI_Init(void);
void NFI_Reset(void);
void NFI_Config(NFI_MENU *input);
STATUS_E NFI_ReadPage(UINT32 *pu4Buf, UINT32 u4PgIdx, BOOL fgEccCheck);
STATUS_E NFI_ReadSpare(UINT32 *pu4Buf, UINT32 u4PageSize, UINT32 u4AddrLen, UINT32 u4PgIdx);
INT32 NFI_ReadBlockInfo(NFI_MENU *prInput, UINT32 u4PgIdx, UINT32* p4TmpData);

extern NFI_MENU _rNFIRetrials[];
extern UINT32 _u4Config_no;
extern BOOL _fgUsingNFI;

#define NUTL_DEFAULT_TIMEOUT    0x0000FFFF
#define NFI_Wait(condition_expression, timeout)     while( (condition_expression) && (--timeout) )

#define NONCACHE(expression) (expression|0xC0000000)

#endif // _NFI_H
