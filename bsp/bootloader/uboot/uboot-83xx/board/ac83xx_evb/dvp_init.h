#include <common.h>
#include <asm/io.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <i2c.h>
#include <asm/mach-types.h>

#include <asm/arch/x_typedef.h>
#include <asm/arch/x_bim.h>
#include <ac83xx_gpio.h>

#include <asm/arch/ac83xx_part_tbl.h>
#include <asm/arch/ac83xx_upg_status.h>
#include <stdio_dev.h>
#include "drv_config_mem.h"
#include <asm/arch/args_to_uboot.h>
#include <upg_config.h>
#include <linux/mtd/atc_nand.h>

#include <chip_ver.h>
#include <display_uboot.h>

#ifndef _DVP_INIT_
#define _DVP_INIT_

//Funtions
#define bHiByte(arg)      (*((BYTE *)&arg + 1))
#define bLoByte(arg)      (*(BYTE *)&arg)

#define wHiWord(arg)      (*((UINT16 *)&arg + 1))
#define wLoWord(arg)      (*(UINT16 *)&arg)

#define Buf_GetPos8(addr, offset)   ((UINT8*)((UINT8 *)(addr) + (offset)))
#define Buf_GetData32(addr, offset) (*(UINT32 *)((UINT8*)(addr) + (offset)))
#define Buf_GetData8(addr, offset)  (*(UINT8 *)((UINT8*)(addr) + (offset)))

#define RESERVED_DVD_BASE_VA      0 //DVP_BUFFER_PA
#define RESERVED_AUDIO_BASE_VA  0   //AUDIO_DSP_MEM_PA

//Target Bin Parameters
#define CODE_INFO_START_ADDRESS (0x200)
#define CODE_INFO_LENGTH        (0x12)
#define DVP_TARGET_LENGTH_LIMIT (2 * 1024 * 1024) //8032 + risc should not exceed this size
#define ROMCODE_IN_RISC_FLASH_OFFSET (720 * 1024) //ROMCODE Offset Address in target bin

//DRAMB Parameters
#define DRAM_PARTITION_ADDR_BASE    (0x71F000L)
#define ROMCODE_IN_DRAMB_OFFSET     (0x70000) //ROMCODE Offset Address to DRAMB
#define ROM_CODE_LENGTH             (260 * 1024) //ROM CODE LENGTH

//DDR Paremeters
#define DVP_CODE_OFFSET_IN_RESERVED_DRAM    (0x900000)
#define DVP_RAM_IMAGE_START_ADDR            (RESERVED_DVD_BASE_VA + DVP_CODE_OFFSET_IN_RESERVED_DRAM) //this address is specified in config.bib in CE OS   
#define DRAMB_START_ADDR                    (RESERVED_DVD_BASE_VA + DRAM_PARTITION_ADDR_BASE)

//DVP Share Infomation Address
#define DVP_SHAREINFO_OFFSET_IN_RESERVED_DRAM    (0x800000)

#endif //_DVP_INIT_
