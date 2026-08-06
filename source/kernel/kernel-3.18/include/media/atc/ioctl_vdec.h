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

#ifndef IOCTL_VDEC
#define IOCTL_VDEC


#include <linux/types.h>
#include <linux/ioctl.h>
#include "vdec_init.h"
#ifdef __KERNEL__
#include <generated/atc_project.h>
#endif
#ifndef CONFIG_ATC_PLATFORM_ac823x
#include "windev.h" 
#endif

typedef enum _VAL_MEM_TYPE_T
{
    VAL_MEM_TYPE_FOR_SW = 0,                    ///< External memory foe SW
    VAL_MEM_TYPE_FOR_HW_CACHEABLE,              ///< External memory for HW Cacheable
    VAL_MEM_TYPE_FOR_HW_CACHEABLE_MCI,          ///< External memory for HW Cacheable, with MCI port config
    VAL_MEM_TYPE_FOR_HW_NONCACHEABLE,           ///< External memory for HW Non-Cacheable
    VAL_MEM_TYPE_MAX = 0xFFFFFFFF               ///< Max memory type
} VAL_MEM_TYPE_T;

/**
 * @par Structure
 *  VAL_MEMORY_T
 * @par Description
 *  This is a parameter for memory usaged function
 */
typedef struct _VAL_MEMORY_T
{
    VAL_MEM_TYPE_T  eMemType;                   ///< [IN]     The allocation memory type
    __u32           u4MemSize;                  ///< [IN]     The size of memory allocation
    void            *pvMemVa;                   ///< [IN/OUT] The memory virtual address
    void            *pvMemPa;                   ///< [IN/OUT] The memory physical address
    __u32           u4Alignment;                ///< [IN]     The memory byte alignment setting
    __u64           u8RealMemPa;                   ///< [IN/OUT] The memory physical address
} VAL_MEMORY_T;

#ifdef CONFIG_ATC_PLATFORM_ac823x

#define VCODEC_IOCTL_MAGIC  'V'

#define VDEC_IOCTL_VPARSE_PROC          0x76646563

#if 1
#define VCODEC_ALLOC_NON_CACHE_BUFFER      _IOWR(VCODEC_IOCTL_MAGIC, 0, VAL_MEMORY_T)
#define VCODEC_FREE_NON_CACHE_BUFFER       _IOWR(VCODEC_IOCTL_MAGIC, 1, VAL_MEMORY_T)
#define VCODEC_WAITISR                     _IO(VCODEC_IOCTL_MAGIC, 3)
#define VCODEC_MB                          _IO(VCODEC_IOCTL_MAGIC, 4)
#define VCODEC_GET_DPB_SIZE                _IOR(VCODEC_IOCTL_MAGIC, 5, __u32)
#define VCODEC_HW_INIT                     _IOR(VCODEC_IOCTL_MAGIC, 6, VDEC_CODEC_INFO_T)
#define VCODEC_GET_SUPPORT_CODEC           _IOWR(VCODEC_IOCTL_MAGIC, 7, __u32)
#define VCODEC_REQUEST_HW                  _IOR(VCODEC_IOCTL_MAGIC, 12, __u32)
#define VCODEC_RELEASE_HW                  _IO(VCODEC_IOCTL_MAGIC, 13)
#define VCODEC_FLUSH_DCACHE                _IOWR(VCODEC_IOCTL_MAGIC, 14, VAL_MEMORY_T)
#define VCODEC_POWERON_HW                  _IOR(VCODEC_IOCTL_MAGIC, 15, __u32)
#define VCODEC_POWEROFF_HW                 _IOR(VCODEC_IOCTL_MAGIC, 16, __u32)
#else
#define VCODEC_ALLOC_NON_CACHE_BUFFER      _IOWR(VCODEC_IOCTL_MAGIC, 0, __u32)
#define VCODEC_FREE_NON_CACHE_BUFFER       _IOWR(VCODEC_IOCTL_MAGIC, 1, __u32)
#define VCODEC_WAITISR                     _IO(VCODEC_IOCTL_MAGIC, 3)
#define VCODEC_MB                          _IO(VCODEC_IOCTL_MAGIC, 4)
#define VCODEC_GET_DPB_SIZE                _IOR(VCODEC_IOCTL_MAGIC, 5, __u32)
#define VCODEC_HW_INIT                     _IOR(VCODEC_IOCTL_MAGIC, 6, __u32)
#define VCODEC_GET_SUPPORT_CODEC           _IOWR(VCODEC_IOCTL_MAGIC, 7, __u32)
#define VCODEC_REQUEST_HW                  _IOR(VCODEC_IOCTL_MAGIC, 12, __u32)
#define VCODEC_RELEASE_HW                  _IO(VCODEC_IOCTL_MAGIC, 13)
#define VCODEC_FLUSH_DCACHE                _IOWR(VCODEC_IOCTL_MAGIC, 14, __u32)
#endif


#else // CONFIG_ATC_PLATFORM_ac823x
#define VDEC_IOCTRL_ID_START             0x100
#define DEFINE_VID_IOCTRL(ID)                           \
    CTL_CODE(FILE_DEVICE_UNKNOWN, ID, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
//
//  Define:  VDEC_IOCTL_INIT
//
//  init vdecoder
//
#define VDEC_IOCTL_INIT                 DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x01)//_IO('p', 0x01) 

//------------------------------------------------------------------------------
//
//  Define:  VDEC_IOCTL_VPARSE_PROC
//
//  parser picture hdr
//
#define VDEC_IOCTL_VPARSE_PROC          DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x02)//_IO('p', 0x02) 

//------------------------------------------------------------------------------
//
//  Define:  VDEC_IOCTL_START_TO_DEC
//
//  after parser picture hdr, begin to dec valid data
//
#define VDEC_IOCTL_START_TO_DEC         DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x03)//_IO('p', 0x03)

//------------------------------------------------------------------------------
//
//  Define:  VDEC_IOCTL_INS_TO_OUTPUTBUF
//
//  set output info, include output buf and necceary info
//
#define VDEC_IOCTL_INS_TO_OUTPUTBUF     DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x04)//_IO('p', 0x04)  

//------------------------------------------------------------------------------
//
//  Define:  VDEC_IOCTL_VDEC
//
//  MSDK call to start dec
//
#define VDEC_IOCTL_VDEC                 DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x05)//_IO('p', 0x05)

//------------------------------------------------------------------------------
//
//  Define:  VDEC_IOCTL_VDEC
//
//  MSDK call to start dec
//
#define VDEC_IOCTL_SEEK                 DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x06)//_IO('p', 0x06)
//-------------------------------------------------------------------------------
//
//video decoder error code
//

#define VDEC_IOCTL_FREEBUF              DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x07)//_IO('p', 0x07) 

#define VDEC_IOCTL_FLUSH                DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x08)//_IO('p', 0x08)

#define VDEC_IOCTL_ALLOC_WCBUF          DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x09)//_IO('p', 0x09)

#define VDEC_IOCTL_GET_RGB_BUFFER       DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x0A)//_IO('p', 0x10)

#define VDEC_IOCTL_ALLOC_IN_BUFFER      DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x0B)//_IO('p', 0x0B)
#define VDEC_IOCTL_GET_RGB888_BUFFER    DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x0C)//_IO('p', 0x11)
#define VDEC_IOCTL_GET_RGB565_BUFFER    DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x0D)//_IO('p', 0x11)
#define VDEC_IOCTL_GET_ASPECT_RATIO     DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x0E)//_IO('p', 0x11)

#define VDEC_IOCTL_PARAM_CONFIG         DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x0F)
#define VDEC_IOCTL_SET_PARAM            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x10)
#define VDEC_IOCTL_GET_PARAM            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x11)

#define VCODEC_ALLOC_NON_CACHE_BUFFER         DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x12)
#define VCODEC_FREE_NON_CACHE_BUFFER            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x13)
#define VCODEC_WAITISR            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x14)
#define VCODEC_MB         DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x15)
#define VCODEC_HW_INIT            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x16)
#define VCODEC_PA_TO_VA            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x17)
#define VCODEC_GET_SUPPORT_CODEC            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x1C)
#define VCODEC_GET_DPB_SIZE            DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x1D)
#define VCODEC_REQUEST_HW               DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x1E)
#define VCODEC_RELEASE_HW               DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x1F)
#define VCODEC_POWERON_HW               DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x20)
#define VCODEC_POWEROFF_HW               DEFINE_VID_IOCTRL(VDEC_IOCTRL_ID_START + 0x21)

struct YC_TO_RGB_BUF
{
    __u32 ycbuf[2];
    void *vaddr;
    __s32 width;
    __s32 height;
};
#endif


#endif

