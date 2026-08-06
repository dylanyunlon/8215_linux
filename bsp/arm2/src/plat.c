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

#include "armv6_mmu.h"
#include "83xx.inc"
#include "addr2func.h"

#define ARM2_SYS_STACK_SIZE (SVC_STACK_SIZE + SYS_STACK_SIZE + ARM2_IRQ_STACK_SIZE + ARM2_EXCPTION_STACK_SIZE)
#define ARM2_RW_BASE        (0x1000)
#define IO_SIZE             (0x400000)

extern unsigned long _start;
extern unsigned long __dram_rodata_end;
extern unsigned long __dram_data_start;
extern unsigned long __heap_end;
/* ==================== Memory Allocation for Page Tables ==================== */

/* L1 Page Table (16KB, must be 16KB aligned) */
static uint32_t l1_page_table[MMU_L1_TABLE_SIZE] __attribute__((section(".mmu.l1")));

/* L2 Page Tables (allocate pool of L2 tables) */
#define MAX_L2_TABLES 16
static mmu_l2_table_t l2_page_tables[MAX_L2_TABLES] __attribute__((section(".mmu.l2")));

static mmu_l2_table_info_t l2_page_table_info[MAX_L2_TABLES];

/* Define memory regions */
static mmu_region_t plat_regions[] = {
    /* Region 0: Vector Tlb < 0 ~ ARM2_RW_BASE > (Read-only, cacheable) */
    {
        .virt_addr = 0x00000000,
        .phys_addr = 0x00000000,
        .size = ARM2_RW_BASE,
        .attributes = {
            .type = MMU_REGION_NORMAL_WB,
            .permission = MMU_PERM_PRIV_RO_USER_RO,
            .domain = 0,
        }
    },

    /* Region 1: System Stack Region < ARM2_RW_BASE ~ ARM2_SYS_STACK_SIZE > (Read-write, cacheable) */
    {
        .virt_addr = ARM2_RW_BASE,
        .phys_addr = ARM2_RW_BASE,
        .size = ARM2_SYS_STACK_SIZE,
        .attributes = {
            .type = MMU_REGION_NORMAL_WB_WA,
            .permission = MMU_PERM_PRIV_RW_USER_RW,
            .domain = 0,
        }
    },

    /* Region 2: Peripheral registers < 0xF0000000 ~ 0xF0400000 > (Device memory) */
    {
        .virt_addr = 0xF0000000,
        .phys_addr = 0xF0000000,
        .size = IO_SIZE,  /* 1MB */
        .attributes = {
            .type = MMU_REGION_DEVICE,
            .permission = MMU_PERM_PRIV_RW_USER_RW,
            .domain = 0,
        }
    },

    /* Region 3: DTB Area < 0x2000000 ~ 0x2200000 > (Read-write, cacheable)
     * Note: it will be free after used up.
     */
    {
        .virt_addr = FDT_LOAD_ADDR - CFG_ARM2_RESERVED_ADDR,
        .phys_addr = FDT_LOAD_ADDR - CFG_ARM2_RESERVED_ADDR,
        .size = 0x200000,
        .attributes = {
            .type = MMU_REGION_NORMAL_WB_WA,
            .permission = MMU_PERM_PRIV_RW_USER_RW,
            .domain = 0,
        }
    },
};

#ifndef CFG_ARGS_RESERVED_ADDR
#define CFG_ARGS_RESERVED_ADDR 0
#define CFG_ARGS_RESERVED_SIZE 0
#endif

#ifndef CFG_METAZONE_RESERVED_ADDR
#define CFG_METAZONE_RESERVED_ADDR 0
#define CFG_METAZONE_RESERVED_SIZE 0
#endif

#ifndef CFG_DATAZONE_RESERVED_ADDR
#define CFG_DATAZONE_RESERVED_ADDR 0
#define CFG_DATAZONE_RESERVED_SIZE 0
#endif

#ifndef CFG_FRAMEBUFFER_RESERVED_ADDR
#define CFG_FRAMEBUFFER_RESERVED_ADDR 0
#define CFG_FRAMEBUFFER_RESERVED_SIZE 0
#endif

#ifndef CFG_IMAGERESIZE_RESERVED_ADDR
#define CFG_IMAGERESIZE_RESERVED_ADDR 0
#define CFG_IMAGERESIZE_RESERVED_SIZE 0
#endif

#ifndef CFG_MM_RESERVED_ADDR
#define CFG_MM_RESERVED_ADDR 0
#define CFG_MM_RESERVED_SIZE 0
#endif

#ifndef CFG_VBA_RESERVED_ADDR
#define CFG_VBA_RESERVED_ADDR 0
#define CFG_VBA_RESERVED_SIZE 0
#endif

#ifndef CFG_ANIMATION_RESERVED_ADDR
#define CFG_ANIMATION_RESERVED_ADDR 0
#define CFG_ANIMATION_RESERVED_SIZE 0
#endif

typedef struct {
    unsigned int start;
    unsigned int size;
} rsv_region;

/* define regions which could be access by ARM2 */
static rsv_region g_plat_rsv_region[] = {
    /* args for rsv info */
    { .start = CFG_ARGS_RESERVED_ADDR, .size = CFG_ARGS_RESERVED_SIZE, },
    /* metazone info */
    { .start = CFG_METAZONE_RESERVED_ADDR, .size = CFG_METAZONE_RESERVED_SIZE, },
    /* datazone info */
    { .start = CFG_DATAZONE_RESERVED_ADDR, .size = CFG_DATAZONE_RESERVED_SIZE, },
    /* framebuffer area */
    { .start = CFG_FRAMEBUFFER_RESERVED_ADDR, .size = CFG_FRAMEBUFFER_RESERVED_SIZE, },
    /* wch area */
    //{ .start = CFG_WCH_RESERVED_ADDR, .size = CFG_WCH_RESERVED_SIZE, },
    /* backcar UI area */
    //{ .start = CFG_ARM2_BACKCAR_UI_RESERVED_ADDR, .size = CFG_ARM2_BACKCAR_UI_RESERVED_SIZE, },
    /* Image resize area */
    { .start = CFG_IMAGERESIZE_RESERVED_ADDR, .size = CFG_IMAGERESIZE_RESERVED_SIZE, },
    /* multimedia area */
    { .start = CFG_MM_RESERVED_ADDR, .size = CFG_MM_RESERVED_SIZE, },
    /* vba area */
    { .start = CFG_VBA_RESERVED_ADDR, .size = CFG_VBA_RESERVED_SIZE, },
    /* audio dsp area */
    //{ .start = CFG_AUDIO_R1_ADDR, .size = CFG_AUDIO_R1_SIZE, },
    /* animation area */
    { .start = CFG_ANIMATION_RESERVED_ADDR, .size = CFG_ANIMATION_RESERVED_SIZE, },
};

void platform_early_init(void)
{
    mmu_region_t dyn_region = {0};
    unsigned int *special_desc;
    int ret, i;
    int size;

    /* Configure MMU mapping tables */
    /* Step1: Initialize MMU context */
    ret = mmu_init(0, l1_page_table, l2_page_tables, l2_page_table_info, MAX_L2_TABLES);
    if (ret != MMU_ERR_SUCCESS) {
        printf("ERROR: MMU initialization failed: %d\n", ret);
        return;
    }
    printf("MMU context initialized successfully\n\n");

    /* Step2: Configure default regions */
    ret = mmu_configure_regions(0, plat_regions, sizeof(plat_regions) / sizeof(plat_regions[0]));
    if (ret != MMU_ERR_SUCCESS) {
        printf("ERROR: Region configuration failed: %d\n", ret);
        return;
    }

    /* Step3: Configure function table regions */
    size = ft_get_total_size();
    if (size) {
        size = (size + 1023) & ~0x3FF;
        memset(&dyn_region, 0, sizeof(dyn_region));
        dyn_region.virt_addr = (unsigned int)&__heap_end;
        dyn_region.phys_addr = (unsigned int)&__heap_end;
        dyn_region.size = size;
        dyn_region.attributes.type = MMU_REGION_NORMAL_WB;
        dyn_region.attributes.permission = MMU_PERM_PRIV_RO_USER_RO;

        ret = mmu_configure_region(0, &dyn_region);
        if (ret != MMU_ERR_SUCCESS) {
               printf("ERROR: Function table configuration failed: %d\n", ret);
            return;
        }
    }

    /* Step4: Configure dynamic regions */
    memset(&dyn_region, 0, sizeof(dyn_region));
    dyn_region.virt_addr = (unsigned int)&_start;
    dyn_region.phys_addr = (unsigned int)&_start;
    dyn_region.size = (unsigned int)&__dram_data_start - (unsigned int)&_start;
    dyn_region.attributes.type = MMU_REGION_NORMAL_WB;
    dyn_region.attributes.permission = MMU_PERM_PRIV_RO_USER_RO;

    ret = mmu_configure_region(0, &dyn_region);
    if (ret != MMU_ERR_SUCCESS) {
        printf("ERROR: Region Text configuration failed: %d\n", ret);
        return;
    }

    memset(&dyn_region, 0, sizeof(dyn_region));
    dyn_region.virt_addr = (unsigned int)&__dram_data_start;
    dyn_region.phys_addr = (unsigned int)&__dram_data_start;
    dyn_region.size = (unsigned int)&__heap_end - (unsigned int)&__dram_data_start;
    dyn_region.attributes.type = MMU_REGION_NORMAL_WB_WA;
    dyn_region.attributes.permission = MMU_PERM_PRIV_RW_USER_RW;

    ret = mmu_configure_region(0, &dyn_region);
    if (ret != MMU_ERR_SUCCESS) {
        printf("ERROR: Region Data configuration failed: %d\n", ret);
        return;
    }

    /* Add reserved memory for arm2: All rsv region set as RW defaultly */
    for (i = 0; i < sizeof(g_plat_rsv_region) / sizeof(g_plat_rsv_region[0]); i++) {
        if (g_plat_rsv_region[i].start < CFG_ARM2_RESERVED_ADDR) {
            printf("ERROR: Rsv Region-%d startaddress:0x%x failed.\n", i, g_plat_rsv_region[i].start);
            return;
        }
        dyn_region.virt_addr = g_plat_rsv_region[i].start - CFG_ARM2_RESERVED_ADDR;
        dyn_region.phys_addr = g_plat_rsv_region[i].start - CFG_ARM2_RESERVED_ADDR;
        dyn_region.size = g_plat_rsv_region[i].size;
        ret = mmu_configure_region(0, &dyn_region);
        if (ret != MMU_ERR_SUCCESS) {
            printf("ERROR: Rsv Region-%d configuration failed: %d\n", i, ret);
            return;
        }
    }

    /* complatible for some static libs */
    dyn_region.virt_addr = 0xFD000000;
    dyn_region.phys_addr = 0xF0000000;
    dyn_region.size = IO_SIZE;
    dyn_region.attributes.type = MMU_REGION_DEVICE;
    dyn_region.attributes.permission = MMU_PERM_PRIV_RW_USER_RW;
    ret = mmu_configure_region(0, &dyn_region);
    if (ret != MMU_ERR_SUCCESS) {
        printf("ERROR: Remap IO region failed: %d\n", ret);
        return;
    }

    /* protect first tiny page: vector table could not be writable */
    special_desc = (unsigned int *)l2_page_tables;
    *special_desc = *special_desc & ~(0xFF0) | 0xFC0;
    printf("Memory regions configured successfully\n");

    /* Enable MMU */
    mmu_enable(0);

    mmu_dump_page_table();
}
