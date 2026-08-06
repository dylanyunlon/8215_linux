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

//
//------------------------------------------------------------------------------
//
//  File:  image_cfg.h
//
//  Defines configuration parameters used to create the NK and Bootloader
//  program images.
//
#if (!defined __IMAGE_CFG_H)
#define __IMAGE_CFG_H



//------------------------------------------------------------------------------
//  RESTRICTION
//
//  This file is a configuration file. It should ONLY contain simple #define
//  directives defining constants. This file is included by other files that
//  only support simple substitutions.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  NAMING CONVENTION
//
//  The IMAGE_ naming convention ...
//
//  IMAGE_<NAME>_<SECTION>_<MEMORY_DEVICE>_[OFFSET|SIZE|START|END]
//
//      <NAME>          - WINCE, BOOT, SHARE
//      <SECTION>       - section name: user defined
//      <MEMORY_DEVICE> - the memory device the block resides on
//      OFFSET          - number of bytes from memory device start address
//      SIZE            - maximum size of the block
//      START           - start address of block    (device address + offset)
//      END             - end address of block      (start address  + size - 1)
//
//------------------------------------------------------------------------------
#if (defined BSP_ARM2)
#define IMAGE_SHARE_ARGS_UA_START       0xAC100000
#define IMAGE_SHARE_ARGS_CA_START       0x8C100000
#define IMAGE_SHARE_ARGS_SIZE           (0x00000800)
#define IMAGE_SHARE_METAZONE_PA_START   0x00000200
#else
#define IMAGE_SHARE_ARGS_UA_START       0xAC000000
#define IMAGE_SHARE_ARGS_CA_START       0x8C000000
#define IMAGE_SHARE_ARGS_SIZE           (0x00000800)
#define IMAGE_SHARE_METAZONE_PA_START   0x00000200
#endif 
//------------------------------------------------------------------------------

#define ARM2_ARGS_OFFSET   (0x800)
#define SDRAM_SIZE_OFFSET  (ARM2_ARGS_OFFSET + 0x0)
#define IMAGE_METAZONE_UA_START       0xAFFE6000
#define IMAGE_METAZONE_VA_START       0x8FFE6000
#define IMAGE_METAZONE_PA_START       0x3FFE6000

#if (defined BSP_NODVDMEMORY)
#define RESERVED_DVD_BASE_VA    0x8c000000   //this line is just for compile issue, it should not be used

#define RESERVED_AUDIO_BASE_VA  0x90000000
#define RESERVED_AUDIO_SIZE     0x00700000

#define RESERVED_VIDEO_BASE_VA  0x90700000
#if (defined MEMORY_SIZE_128M)
#define RESERVED_VIDEO_SIZE     0x00A00000
#else
#define RESERVED_VIDEO_SIZE     0x01700000
#endif

    
#define RESERVED_MEMORY_SIZE   (RESERVED_AUDIO_SIZE + RESERVED_VIDEO_SIZE)
#else

#if (defined DVP_CODE_IN_NAND)

#define RESERVED_DVD_BASE_VA    0x90000000
#if (defined MEMORY_SIZE_128M)

#define RESERVED_DVD_SIZE       0x00900000

#define RESERVED_AUDIO_BASE_VA  0x90900000 // must 1MB alignment for audio dsp
#define RESERVED_AUDIO_SIZE     0x00600000

#define RESERVED_VIDEO_BASE_VA  0x90F00000
#define RESERVED_VIDEO_SIZE     0x00650000

#else

#define RESERVED_DVD_SIZE       0x00B00000

#define RESERVED_AUDIO_BASE_VA  0x90B00000 // must 1MB alignment for audio dsp
#define RESERVED_AUDIO_SIZE     0x00700000

#define RESERVED_VIDEO_BASE_VA  0x91200000
#define RESERVED_VIDEO_SIZE     0x01f00000

#endif

#else

#define RESERVED_DVD_BASE_VA    0x90000000

#if (defined MEMORY_SIZE_128M)

#define RESERVED_DVD_SIZE       0x00800000

#define RESERVED_AUDIO_BASE_VA  0x90800000 // must 1MB alignment for audio dsp
#define RESERVED_AUDIO_SIZE     0x00600000

#define RESERVED_VIDEO_BASE_VA  0x90E00000
#define RESERVED_VIDEO_SIZE     0x00650000

#else

#define RESERVED_DVD_SIZE       0x00900000

#define RESERVED_AUDIO_BASE_VA  0x90900000 // must 1MB alignment for audio dsp
#define RESERVED_AUDIO_SIZE     0x00700000

#define RESERVED_VIDEO_BASE_VA  0x91000000
#define RESERVED_VIDEO_SIZE     0x01f00000

#endif

#endif


#define RESERVED_MEMORY_SIZE   (RESERVED_DVD_SIZE + RESERVED_AUDIO_SIZE + RESERVED_VIDEO_SIZE)

#endif

#define RESERVED_MEMORY_START  (RESERVED_VIDEO_BASE_VA + RESERVED_VIDEO_SIZE)

#if (defined MEMORY_SIZE_128M)
#define CE_MEMORY_SIZE (0x04000000 - RESERVED_MEMORY_SIZE)
#else
#define CE_MEMORY_SIZE (0x0C000000 - RESERVED_MEMORY_SIZE)
#endif


#endif
