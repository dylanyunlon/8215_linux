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

#ifndef _NFI2_H
#define _NFI2_H
#include <mach/ac83xx_basic.h>
#include <mach/chip_ver.h>


#ifndef    AC83XX_YECON_BOARD
//#define   USE_60BIT
#endif


#define IO_VIRT 0xFD000000
//#define LINUX_ISR_ENABLE

#define FDM_BYTES  9
#define FDM_ECC_BYTES 9
#define MTD_NAND_DEFAULT_TIMEOUT	0x00FFFFFF
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

#define INT_WR_CLR
#ifdef  USE_60BIT
#define NFI_BASE_REG (IO_VIRT+0x1E000) 
#else
#define NFI_BASE_REG (IO_VIRT+0x1E400) 
#endif

#define WAIT_RDY_MASK         0x1000

#define NFCEN2_SEL_PWM0             0x8000

#define NFI2_MASK                            0x00400000

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
#define NOB_BYTE_8         0x0000//read 8 bytes
#define NOB_WORD           0x0040//read 2 bytes
#define NOB_DWORD          0x0080//read 4 bytes
#define BURST_RD           0x0100
#define BURST_WR           0x0200
#define NOB_BYTE           0x0020
#define SEC_NUM(x)                (((u32) x &0x0F) << 12)

/* NFI_ACCCON */
#define ACCCON        (LCD2NAND| PRECS|C2R | W2R | WH | WST | RLT)

#define PDN_NFI 0x00080000
#define NFI_CLK_144 0x00030000
#define NFI_CLK_27 0x0
#define NFI_CLK_200 0x00050000
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
#define RLT           0x5        // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=2
#define WST           (0x3 << 4)   // 0:0T, 1:1T, 2:2T, 3:3T ; recommanded value=1
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
#define RB_CS1                0x0010  // NFI2+0090[4]

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
	,S_PGM_FAILED
	,S_ERASE_FAILED
	,S_TIMEOUT
	,S_IN_PROGRESS
	,S_CMD_ERR
	,S_BLOCK_LOCKED_ERR
	,S_BLOCK_UNSTABLE
	,S_VPP_RANGE_ERR
	,S_INVALID_BEGIN_ADDR
	,S_INVALID_RANGE
	,S_PGM_AT_ODD_ADDR
	,S_PGM_WITH_ODD_LENGTH
	,S_BUFPGM_NO_SUPPORT
	,S_UNKNOWN_ERR
	,S_BAD_BLOCK
	,S_ECC_1BIT_CORRECT
	,S_ECC_2BITS_ERR
	,S_ECC_UNCORRECT_ERR
	,S_ECC_CORRECTABLE_ERR
	,S_SPARE_CHKSUM_ERR
	,S_HW_COPYBACK_ERR
	,S_INVALID_PAGE_INDEX
	,S_NFI_NOT_SUPPORT
	,S_NFI_CS1_NOT_SUPPORT
	,S_NFI_16BITS_IO_NOT_SUPPORT
	,S_NO_GOOD_BLOCK_FOUND
	,S_SETUP_PLL_ERR
	,S_MOBILE_RAM_NOT_SUPPORT
	,S_RAM_FLOARTING
	,S_RAM_UNACCESSABLE
	,S_RAM_ERROR
	,S_DEVICE_NOT_FOUND
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

#define UINT32 u32
#define BOOL unsigned int
#define UINT16 unsigned short
#define UINT8 unsigned char
#define FALSE false
#define TRUE true

#ifdef USE_60BIT

static const u16 _p2Randomizerseeds[128]= 
{
0x576A,0x05E8,0x629D,0x45A3,0x649C,0x4BF0,0x2342,0x272E,
0x7358,0x4FF3,0x73EC,0x5F70,0x7A60,0x1AD8,0x3472,0x3612,
0x224F,0x0454,0x030E,0x70A5,0x7809,0x2521,0x48F4,0x5A2D,
0x492A,0x043D,0x7F61,0x3969,0x517A,0x3B42,0x769D,0x0647,
0x7E2A,0x1383,0x79D9,0x07B8,0x2578,0x7EEC,0x4423,0x352F,
0x5B22,0x72B9,0x367B,0x24B6,0x7E8E,0x2318,0x6BD0,0x5519,
0x1783,0x18A7,0x7B6E,0x7602,0x4B7F,0x3648,0x2C53,0x6B99,
0x0C23,0x67CF,0x7E0E,0x4D8C,0x5079,0x209D,0x244A,0x747B,
0x350B,0x0E4D,0x7004,0x6AC3,0x7F3E,0x21F5,0x7A15,0x2379,
0x1517,0x1ABA,0x4E77,0x15A1,0x04FA,0x2D61,0x253A,0x1302,
0x1F63,0x5AB3,0x049A,0x5AE8,0x1CD7,0x4A00,0x30C8,0x3247,
0x729C,0x5034,0x2B0E,0x57F2,0x00E4,0x575B,0x6192,0x38F8,
0x2F6A,0x0C14,0x45FC,0x41DF,0x38DA,0x7AE1,0x7322,0x62DF,
0x5E39,0x0E64,0x6D85,0x5951,0x5937,0x6281,0x33A1,0x6A32,
0x3A5A,0x2BAC,0x743A,0x5E74,0x3B2E,0x7EC7,0x4FD2,0x5D28,
0x751F,0x3EF8,0x39B1,0x4E49,0x746B,0x6EF6,0x44BE,0x6DB7
};
#endif

#endif
