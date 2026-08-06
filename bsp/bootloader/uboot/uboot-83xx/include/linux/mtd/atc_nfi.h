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
/******************************************************************************
* [File]			atc_nfi.h
* [Version]			v1.0
* [Revision Date]	2007-08-02
* [Author]			Meng-Chang Liu, MC_Liu@mtk.com.tw, 26615, 2007-08-02
* [Description]
*	AC83XX Download Agent NFI include file
* [Copyright]
*	Copyright (C) 2007 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/
#ifndef _ATC_NFI_H
#define _ATC_NFI_H
#include <chip_ver.h>

//#define   USE_60BIT

#define INT_WR_CLR

//#define LINUX_ISR_ENABLE

#define FDM_BYTES  9
#define FDM_ECC_BYTES 9
#define MTD_NAND_DEFAULT_TIMEOUT	0x000FFFFF

#ifdef  USE_60BIT
#define NFI_BASE 0xf001e000
#else
#define NFI_BASE 0xf001e400
#endif

typedef enum {
	 CS0 = 0
	,CS1
	,CS2
	,CS3
	,CS4
	,CS5
	,CS6
	,CS7
	,CS_WITH_DECODER
	,MAX_CS = CS_WITH_DECODER
	,HW_CHIP_SELECT_END
} HW_ChipSelect_E;

/* Register definition */
#define NFI_CNFG             ((volatile u16 *)(NFI_BASE+0x0000))
#define NFI_PAGEFMT     ((volatile u32 *)(NFI_BASE+0x0004))
#define NFI_CON       ((volatile u16 *)(NFI_BASE+0x0008))
#define NFI_ACCCON      ((volatile u32 *)(NFI_BASE+0x000C))
#define NFI_INTR_EN     ((volatile u16 *)(NFI_BASE+0x0010))
#define NFI_INTR        ((volatile u16 *)(NFI_BASE+0x0014))
#define NFI_CMD         ((volatile u16 *)(NFI_BASE+0x0020))
#define NFI_ADDRNOB      ((volatile u16 *)(NFI_BASE+0x0030))
#define NFI_COLADDR       ((volatile u32 *)(NFI_BASE+0x0034))
#define NFI_ROWADDR       ((volatile u32 *)(NFI_BASE+0x0038))
#define NFI_STRDATA   ((volatile u16 *)(NFI_BASE+0x0040))
#define NFI_DATAW       ((volatile u32 *)(NFI_BASE+0x0050))
#define NFI_DATAR       ((volatile u32 *)(NFI_BASE+0x0054))
#define NFI_STA         ((volatile u32 *)(NFI_BASE+0x0060))
#define NFI_FIFOSTA     ((volatile u16 *)(NFI_BASE+0x0064))
#define NFI_ADDRCNTR   ((volatile u16 *)(NFI_BASE+0x0070))
#define NFI_STRADDR    ((volatile u32 *)(NFI_BASE+0x0080))
#define NFI_BYTELEN     ((volatile u16 *)(NFI_BASE+0x0084))
#define NFI_CSEL	      ((volatile u16 *)(NFI_BASE+0x090))

/* DMA address 16byte alignment. */
#define NFI_FDM0L		((volatile u32 *)(NFI_BASE+0x200))
#define NFI_FDM0M	   ((volatile u32 *)(NFI_BASE+0x204))
#define NFI_CLK_SEL       ((volatile u32 *)(CKGEN_BASE+0x18))
#ifdef  USE_60BIT
#define NFI_RANDOM_CFG  ((volatile u32 *)(NFI_BASE+0x280))
#endif
/*******************************************************************************
 * NFI register definition
 *******************************************************************************/
/* NFI_CNFG */
#define AHB_MODE        0x0001
#define READ_MODE      0x0002
#ifdef   USE_60BIT
#define DBYTE_EN	 	0x0010
#endif
#define SEL_SEC_512BYTE 0x0020
#define BYTE_RW		      0x0040
#define HW_ECC_EN      0x0100
#define AUTO_FMT_EN  0x0200
#define OP_IDLE            0x0000
#define OP_READ           0x1000
#define OP_READ_ID_ST   0x2000
#define OP_PROGRAM    0x3000
#define OP_ERASE         0x4000
#define OP_RESET         0x5000
#define OP_CUSTOM    0x6000

/* NFI_PAGEFMT */
#define PAGEFMT_2K_512			0x0000
#define PAGEFMT_4K_2K			0x0001
#define PAGEFMT_8K_4K         0x0002
#ifdef  USE_60BIT
#define SPARE_32_16  0x0000
#define SPARE_52_26  0x0004
#define SPARE_54_27  0x0008
#define SPARE_56_28  0x000C
#define SPARE_64_32  0x0010
#define SPARE_72_36  0x0014
#define SPARE_80_40  0x0018
#define SPARE_88_44  0x001C
#define SPARE_96_48  0x0020
#define SPARE_100_50 0x0024
#define SPARE_104_52 0x0028
#define SPARE_108_54 0x002C
#define SPARE_112_56 0x0030
#define SPARE_124_62 0x0034
#define SPARE_126_63 0x0038
#define SPARE_128_64 0x003C
#else
#define PAGEFMT_8BITS		0x0000
#define PAGEFMT_16BITS		0x0008	
#define SPARE_32_16  0x0000
#define SPARE_52_26  0x0010
#define SPARE_54_27  0x0020
#define SPARE_56_28  0X0030
#endif

#define FDM_NUM(x)                (((u32) x &0x1F) << 6)
#define FDM_ECC_NUM(x)                (((u32) x &0x1F) << 11)


/* NFI_CON */
#define FIFO_FLUSH       0x0001
#define NFI_RST             0x0002
#define SINGLE_RD          0x0010
#define NOB_BYTE             0x0020
#define NOB_WORD           0x0040
#define NOB_DWORD          0x0080
#define BURST_RD           0x0100
#define BURST_WR           0x0200
#define SEC_NUM(x)                (((u32) x &0x0F) << 12)

/* NFI_ACCCON */
#define ACCCON        (LCD2NAND| PRECS|C2R | W2R | WH | WST | RLT)

#define PDN_NFI 0x00080000
#define NFI_CLK_144 0x00030000
#define NFI_CLK_27 0x0
#define NFI_CLK_200 0x00050000
#define NFI_CLK_234 0x00010000
#define NFI_CLK NFI_CLK_144

#if NFI_CLK == NFI_CLK_27
#define RLT           0xF        // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=2
#define WST           (0xF << 4)   // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=1
#define WH            (0xF<< 8)  // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=1
#define W2R           (0xF << 12) // 0:2T, 1:4T, 2:6T, 3:8T ; recommanded value=3
#define C2R           ((u32)0x3F << 16) // C2R=111
#define PRECS       ((u32)0x0F << 22)
#define LCD2NAND      ((u32)0xF << 28)    
#else
#define RLT           0x05        // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=2
#define WST           (0x03 << 4)   // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=1
#define WH            (0x1<< 8)  // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=1
#define W2R           (0x0 << 12) // 0:2T, 1:4T, 2:6T, 3:8T ; recommanded value=3
#define C2R           ((u32)0x0 << 16) // C2R=111
#define PRECS       ((u32)0x0 << 22)
#define LCD2NAND      ((u32)0x0 << 28)
#endif

/* NFI_INTR_EN */
#define RD_DONE_EN     0x01
#define WR_DONE_EN     0x02
#define RESET_DONE_EN  0x04
#define ERASE_DONE_EN  0x08
#define BUSY_RETURN_EN     0x0010
#define ACCESS_LOCK_EN     0x0020
#define AHB_DONE_EN          0x0040

/* NFI_INTR */
#define RD_DONE    0x01
#define WR_DONE     0x02
#define RESET_DONE  0x04
#define ERASE_DONE  0x08
#define BUSY_RETURN     0x0010
#define ACCESS_LOCK     0x0020
#define AHB_DONE          0x0040

/* NFI_ADDRNOB */
#define COL_ADDR_NOB(x)   (((u32) x &0x0F) )
#define ROW_ADDR_NOB(x)   (((u32) x &0x0F) << 4 )

/* NFI_STRDATA */
#define STR_DATA    0x01

/* NFI_STA  */
#define STATUS_CMD         0x1
#define STATUS_ADDR        0x2
#define STATUS_DATAR       0x4
#define STATUS_DATAW       0x8
#define STATUS_BUSY        0x100

/* NFI_FIFOSTA */
#define RD_EMPTY_MASK      0x0040
#define RD_FULL_MASK       0x0080
#define WR_EMPTY_MASK      0x4000
#define WR_FULL_MASK      	0x8000
#define RD_REMAIN_MASK    0x001F

/* NFI_CSEL */
#define RB_CS1                0x0010  // NFI+0090[4]

/* Status register */
#define STATUS_FAIL			0x01
#define STATUS_READY		0x40
#define STATUS_WR_ALLOW		0x80
 

/*NFI_DMA*/
#define DMA_RDTRIG				((u32) 1 << 1)
#define DMA_WRTRIG				((u32) 1 << 0)

#define NFI_Wait_Ready(timeout)   while ( (*NFI_STA  & STATUS_BUSY) && (timeout--) )

/* FLASH OPERATION STATUS */
typedef enum {
	 S_DONE = 0
	,S_PGM_FAILED    //  1
	,S_ERASE_FAILED    // 2
	,S_TIMEOUT             //  3
	,S_IN_PROGRESS    //  4
	,S_CMD_ERR            //  5
	,S_BLOCK_LOCKED_ERR     //   6
	,S_BLOCK_UNSTABLE          //   7
	,S_VPP_RANGE_ERR            //   8
	,S_INVALID_BEGIN_ADDR  //   9
	,S_INVALID_RANGE             //   10
	,S_PGM_AT_ODD_ADDR      //   11
	,S_PGM_WITH_ODD_LENGTH   //  12
	,S_BUFPGM_NO_SUPPORT       //   13
	,S_UNKNOWN_ERR       //   14           
	,S_BAD_BLOCK             //    15
	,S_ECC_1BIT_CORRECT   //16
	,S_ECC_2BITS_ERR       //   17
	,S_ECC_UNCORRECT_ERR   // 18
	,S_ECC_CORRECTABLE_ERR    // 19
	,S_SPARE_CHKSUM_ERR    //  20
	,S_HW_COPYBACK_ERR     //   21
	,S_INVALID_PAGE_INDEX   //  22
	,S_NFI_NOT_SUPPORT    //   23
	,S_NFI_CS1_NOT_SUPPORT    //   24
	,S_NFI_16BITS_IO_NOT_SUPPORT    //  25
	,S_NO_GOOD_BLOCK_FOUND    //   26
	,S_SETUP_PLL_ERR               //    27
	,S_MOBILE_RAM_NOT_SUPPORT    //  28
	,S_RAM_FLOARTING          //   29
	,S_RAM_UNACCESSABLE    //  30
	,S_RAM_ERROR        //    31
	,S_DEVICE_NOT_FOUND          //  32
	,S_REACH_END_ADDR
	,S_BLOADER_IS_TOO_LARGE
	,S_SIBLEY_REWRITE_OBJ_MODE_REGION
	,S_SIBLEY_WRITE_B_HALF_IN_CTRL_MODE_REGION
	,S_SIBLEY_ILLEGAL_CMD
	,S_SIBLEY_PGM_AT_THE_SAME_REGIONS
	,S_CREATE_SEMAPHORE_FAIL
	,S_DEL_SEMAPHORE_FAIL
	,S_CREATE_EVENT_FAIL
  	,S_DEL_EVENT_FAIL	
	,S_REG_ISR_FAIL
	,S_UNREG_ISR_FAIL	
	,STATUS_END
} STATUS_E;

#define NFI_Wait(condition_expression, timeout)		while( (condition_expression) && (--timeout) )


#define NFI_EVENT_NAME 		"NFI_EVENT"

#define SECTOR_BYTES  ((*NFI_CNFG&SEL_SEC_512BYTE)?512:1024)

//extern BOOL _bUsingISR;
#define NFI_FSM_IDLE          (0x00000000)
#define NFI_FSM_RST           (0x00010000)
#define NFI_FSM_RD_BUSY       (0x00020000)
#define NFI_FSM_RD_DATA       (0x00030000)
#define NFI_FSM_PROG_BUSY     (0x00040000)
#define NFI_FSM_PROG_DATA     (0x00050000)
#define NFI_FSM_ERASE_BUSY    (0x00080000)
#define NFI_FSM_ERASE_DATA    (0x00090000)
#define NFI_FSM_CST_DATA      (0x000E0000)
#define NFI_FSM_CST           (0x000F0000)
#define STA_NFI_FSM_MASK      (0x000F0000)
#define STA_NFI_FSM_SHIFT     (16)
	
#define NAND_FSM_IDLE         (0x00000000)
#define NAND_FSM_CMD_WRRDY    (0x04000000)
#define NAND_FSM_CMD_WRST     (0x05000000)
#define NAND_FSM_CMD_WR       (0x06000000)
#define NAND_FSM_CMD_WRHD     (0x07000000)
#define NAND_FSM_ADDR_WRRDY   (0x08000000)
#define NAND_FSM_ADDR_WRST    (0x09000000)
#define NAND_FSM_ADDR_WR      (0x0A000000)
#define NAND_FSM_ADDR_WRHD    (0x0B000000)
#define NAND_FSM_CA2DEXT      (0x0C000000)
#define NAND_FSM_DATA_RDST    (0x11000000) 
#define NAND_FSM_DATA_RD      (0x12000000)
#define NAND_FSM_DATA_RDHD    (0x13000000)
#define NAND_FSM_DATA_WRRDY   (0x18000000)
#define NAND_FSM_DATA_WRST    (0x19000000)
#define NAND_FSM_DATA_WR      (0x1A000000)
#define NAND_FSM_DATA_WRHD    (0x1B000000)
#define NAND_FSM_PRECE        (0x1C000000)
#define STA_NAND_FSM_MASK     (0x1F000000)
#define STA_NAND_FSM_SHIFT    (24)
	
	/* NFI_FIFOSTA */
#define FIFO_RD_EMPTY         (0x00000040)
#define FIFO_RD_FULL          (0x00000080)
#define FIFO_WR_EMPTY         (0x00004000)
#define FIFO_WR_FULL          (0x00008000)
#define FIFO_RD_REMAIN(x)     (0x1F&(x))
#define FIFO_WR_REMAIN(x)     ((0x1F00&(x))>>8)

#endif //_ATC_NFI_H
