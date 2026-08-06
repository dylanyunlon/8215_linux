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


/* Include header files */
//#include "typedefs.h"
//#include "platform.h"
//#include "dram_buffer.h"
#include "tz_init.h"
//#include "debug.h"
//#include "tz_sec_reg.h"
//#include "device_apc.h"
//#include "tz_mem.h"
//#include "sec_devinfo.h"
//#include "cust_sec_ctrl.h"
//#include "sec.h"
#include "x_printf.h"
#include "boot.h"

#define print   Printf
#define dprintf Printf

#ifdef CFG_GOOGLE_TRUSTY_SUPPORT
//#include "tz_trusty.h"
#endif
//#include "sec_devinfo.h"
//#include "emi_drv.h"

#if 0
void trustzone_pre_init_v7(void)
{
    sec_malloc_buf_reset();
    tz_dapc_sec_init();
}

void trustzone_post_init_v7(void)
{
    tz_dapc_sec_postinit();
}
#endif

#if 0
void platform_set_boot_args(void)
{
    int i;
    u32 sec_addr, sec_size = 0;
    tee_get_secmem_start(&sec_addr);
    tee_get_secmem_size(&sec_size);
    bootarg.tee_reserved_mem.start = sec_addr;
    bootarg.tee_reserved_mem.size = sec_size;

    print("\n%s boot to LK.\n", MOD, g_boot_reason);
    bootarg.magic = BOOT_ARGUMENT_MAGIC;
    bootarg.mode  = g_boot_mode;
    bootarg.e_flag = sp_check_platform();
    bootarg.log_port = CFG_UART_LOG;
    bootarg.log_baudrate = CFG_LOG_BAUDRATE;
    bootarg.log_enable = (u8)log_status();
    bootarg.boot_reason = g_boot_reason;
    bootarg.meta_com_type = (u32)g_meta_com_type;
    bootarg.meta_com_id = g_meta_com_id;
    bootarg.meta_uart_port = CFG_UART_META;
    bootarg.boot_time = get_timer(g_boot_time);

    bootarg.part_num =  g_dram_buf->part_num;
    bootarg.part_info = g_dram_buf->part_info;

    bootarg.ddr_reserve_enable = g_ddr_reserve_enable;
    bootarg.ddr_reserve_success= g_ddr_reserve_success;
    bootarg.dram_buf_size =  sizeof(dram_buf_t);

    bootarg.smc_boot_opt = g_smc_boot_opt;
    bootarg.lk_boot_opt = g_lk_boot_opt;
    bootarg.kernel_boot_opt = g_kernel_boot_opt;

    mt_set_ptp_info(&bootarg.ptp_volt_info);

    bootarg.non_secure_sram_addr = CFG_NON_SECURE_SRAM_ADDR;
    bootarg.non_secure_sram_size = CFG_NON_SECURE_SRAM_SIZE;

    print("%s NON SECURE SRAM ADDR: 0x%x\n", MOD, bootarg.non_secure_sram_addr);
    print("%s NON SECURE SRAM SIZE: 0x%x\n", MOD, bootarg.non_secure_sram_size);

    memcpy(bootarg.pl_version, PL_VERSION, sizeof(bootarg.pl_version));
    print("%s PL_VERSION = %s \n", MOD, bootarg.pl_version);


    print("\n%s boot reason: %d\n", MOD, g_boot_reason);
    print("%s boot mode: %d\n", MOD, g_boot_mode);
    print("%s META COM%d: %d\n", MOD, bootarg.meta_com_id, bootarg.meta_com_type);
    print("%s <0x%x>: 0x%x\n", MOD, &bootarg.e_flag, bootarg.e_flag);
    print("%s boot time: %dms\n", MOD, bootarg.boot_time);
    print("%s DDR reserve mode: enable = %d, success = %d\n", MOD, bootarg.ddr_reserve_enable, bootarg.ddr_reserve_success);
    print("%s dram_buf_size: 0x%x\n", MOD, bootarg.dram_buf_size);
    print("%s smc_boot_opt: 0x%x\n", MOD, bootarg.smc_boot_opt);
    print("%s lk_boot_opt: 0x%x\n", MOD, bootarg.lk_boot_opt);
    print("%s kernel_boot_opt: 0x%x\n", MOD, bootarg.kernel_boot_opt);
    print("%s tee_reserved_mem: 0x%llx, 0x%llx\n", MOD, bootarg.tee_reserved_mem.start, bootarg.tee_reserved_mem.size);

}
#endif


unsigned int tee_entry_addr = 0x100000;


void trustzone_jump_v7(unsigned int addr, unsigned int arg1, unsigned int arg2)
{
    typedef void (*jump_func_type)(unsigned int addr, unsigned int arg1, unsigned int arg2) __attribute__ ((__noreturn__));
    jump_func_type jump_func;
    unsigned int full_memory_size = 0;
    unsigned int tee_secmem_size = 0x200000;
    sec_mem_arg_t sec_mem_arg;
    tee_v7_arg_t_ptr teearg = (tee_v7_arg_t_ptr)(tee_entry_addr + 0x100000); //trustzone_get_tee_boot_param_addr();

    /* Configure platform's security settings */
    //tee_sec_config();   // Need to implement
    /* prepare trustonic's TEE arguments */
    Printf("trustzone_jump_v7 addr: 0x%x\n", addr);
	
    teearg->magic        = TEE_ARGUMENT_MAGIC;       /* TEE magic number */
    teearg->NWEntry      = addr;                     /* NW Entry point after t-base */
    teearg->NWBootArgs   = arg1;                     /* NW boot args (propagated by t-base in r4 before jump) */
    teearg->NWBootArgsSize = arg2;                   /* NW boot args size (propagated by t-base in r5 before jump) */
    teearg->dRamBase     = 0x0;            /* DRAM base address */
    teearg->dRamSize     = 0x40000000;            /* Full DRAM size */
    teearg->secDRamBase  = 0x100000;           /* Secure DRAM base address */
    teearg->secDRamSize  = 0x200000;           /* Secure DRAM size */
    teearg->sRamBase     = 0x00000000;               /* SRAM base address */
    teearg->sRamSize     = 0x00000000;               /* SRAM size */
    teearg->secSRamBase  = 0x00000000;               /* Secure SRAM base address */
    teearg->secSRamSize  = 0x00000000;               /* Secure SRAM size */
    teearg->log_port     = 0x11002000;            /* UART logging : UART base address. Can be same as preloader's one or not */
    teearg->log_baudrate = 921600;      /* UART logging : UART baud rate */
    teearg->gicd_base    = 0xF1001000;                /* ARM GIC Distributor Interface Base Address */
    teearg->gicc_base    = 0xF1002000;                /* ARM GIC CPU Interface Base Address */


    //DBG_MSG("%s CFG_TEE_CORE_SIZE : 0x%x\n", MOD, CFG_TEE_CORE_SIZE);
    //DBG_MSG("%s tee_secmem_size : 0x%x\n", MOD, tee_secmem_size);
    //DBG_MSG("%s hwuid[0-3] : 0x%x 0x%x 0x%x 0x%x\n", MOD,
    //    teearg->hwuid[0], teearg->hwuid[1], teearg->hwuid[2], teearg->hwuid[3]);


    /* Jump to TEE */
    Printf("Start world switch from secure to non-secure world.\n");
    //Printf("SW jump addr is 0x%x.\n", tee_entry_addr);
    jump_func = (jump_func_type)tee_entry_addr;

    //Printf("NWd jump addr is 0x%x, args addr is 0x%x\n", teearg->NWEntry, teearg->NWBootArgs);
    //Printf("arg0(0x%x), arg1(0x%x), arg2(0x%x)\n", tee_secmem_size, (unsigned int)teearg, sizeof(tee_v7_arg_t));
    //dprintf(ALWAYS, "log_port(0x%x), log_baudrate(0x%x)\n",MOD, CFG_UART_LOG, CFG_LOG_BAUDRATE);
    (*jump_func)(tee_secmem_size, (unsigned int)teearg, sizeof(tee_v7_arg_t));
    // Never return.
}

extern struct quickboot_param qb_param;

void trustzone_resume_v7(unsigned int addr, unsigned int arg1, unsigned int arg2)
{
	typedef void (*jump_func_type)(unsigned int addr, unsigned int arg1, unsigned int arg2) __attribute__ ((__noreturn__));
	jump_func_type jump_func;
	unsigned int full_memory_size = 0;
	unsigned int tee_secmem_size = 0x200000;
	sec_mem_arg_t sec_mem_arg;
	tee_v7_arg_t_ptr teearg = (tee_v7_arg_t_ptr)(tee_entry_addr + 0x100000); //trustzone_get_tee_boot_param_addr();

	/* Configure platform's security settings */
	//tee_sec_config();   // Need to implement
	/* prepare trustonic's TEE arguments */
	Printf("trustzone_resume_v7 addr: 0x%x\n", addr);
	
	teearg->magic		 = TEE_ARGUMENT_MAGIC;		 /* TEE magic number */
	teearg->NWEntry 	 = addr;					 /* NW Entry point after t-base */
	teearg->NWBootArgs	 = arg1;					 /* NW boot args (propagated by t-base in r4 before jump) */
	teearg->NWBootArgsSize = arg2;					 /* NW boot args size (propagated by t-base in r5 before jump) */
	teearg->dRamBase	 = 0x0; 		   /* DRAM base address */
	teearg->dRamSize	 = 0x40000000;			  /* Full DRAM size */
	teearg->secDRamBase  = 0x100000;		   /* Secure DRAM base address */
	teearg->secDRamSize  = 0x200000;		   /* Secure DRAM size */
	teearg->sRamBase	 = 0x00000000;				 /* SRAM base address */
	teearg->sRamSize	 = 0x00000000;				 /* SRAM size */
	teearg->secSRamBase  = 0x00000000;				 /* Secure SRAM base address */
	teearg->secSRamSize  = 0x00000000;				 /* Secure SRAM size */
	teearg->log_port	 = 0x11002000;			  /* UART logging : UART base address. Can be same as preloader's one or not */
	teearg->log_baudrate = 921600;		/* UART logging : UART baud rate */
	teearg->gicd_base	 = 0xF1001000;				  /* ARM GIC Distributor Interface Base Address */
	teearg->gicc_base	 = 0xF1002000;				  /* ARM GIC CPU Interface Base Address */


	//DBG_MSG("%s CFG_TEE_CORE_SIZE : 0x%x\n", MOD, CFG_TEE_CORE_SIZE);
	//DBG_MSG("%s tee_secmem_size : 0x%x\n", MOD, tee_secmem_size);
	//DBG_MSG("%s hwuid[0-3] : 0x%x 0x%x 0x%x 0x%x\n", MOD,
	//	  teearg->hwuid[0], teearg->hwuid[1], teearg->hwuid[2], teearg->hwuid[3]);


	/* Jump to TEE */
	Printf("Resume world switch from secure to non-secure world.\n");
	Printf("SW jump addr is 0x%x.\n", qb_param.sw_resume_entry);
	Printf("SW jump addr is fix  0x1002a8.\n");
	jump_func = (jump_func_type)qb_param.sw_resume_entry;

	//Printf("NWd jump addr is 0x%x, args addr is 0x%x\n", teearg->NWEntry, teearg->NWBootArgs);
	//Printf("arg0(0x%x), arg1(0x%x), arg2(0x%x)\n", tee_secmem_size, (unsigned int)teearg, sizeof(tee_v7_arg_t));
	//dprintf(ALWAYS, "log_port(0x%x), log_baudrate(0x%x)\n",MOD, CFG_UART_LOG, CFG_LOG_BAUDRATE);
	(*jump_func)(tee_secmem_size, (unsigned int)teearg, sizeof(tee_v7_arg_t));
	// Never return.
}

