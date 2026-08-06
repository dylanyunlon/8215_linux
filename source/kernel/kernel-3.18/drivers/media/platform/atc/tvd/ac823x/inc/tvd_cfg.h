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

#ifndef TVD_CFG_H
#define TVD_CFG_H

#include "chip_ver.h"

#define TVD_DRV_FPGA_BOARD             0
/*---------------Configuration Section about emulation arm2 tvd driver.*/
#define UNDERCE_EMUL_ARM2_TVD          0

#if (UNDERCE_EMUL_ARM2_TVD)
#define ARM2_TVD_DRV
#endif
/*---------------*/

#define TVD_EVT_NAME_MAX_LENGTH        20

/*---------------Configuration Section about emulation arm2 tvd driver.*/
/*---------------*/
/*---------------Configuration Section about video buffers alloc policy.*/
#if defined(__ARM2__)
/*for arm2*/
#define MAX_VDO_BUF_CNT                5
#elif defined(__UBOOT__)
/*for uboot*/
#else
/*for arm1*/
#endif

#if 0
#define MEM_RESERVE_SOLUTION           1
#define MEM_ALLOC_SOLUTION             2

#define CUR_VDOBUF_MEM_SOLUTION       (MEM_ALLOC_SOLUTION)

#if (CUR_VDOBUF_MEM_SOLUTION == MEM_RESERVE_SOLUTION)

#define VDO_BUF_START_ADDR             (0xB0000000)

#define TVD_VATOPA(x)                  (x - 0xAC000000)

#elif (CUR_VDOBUF_MEM_SOLUTION == MEM_ALLOC_SOLUTION)

#define TVD_VATOPA(x)                  (x)

#else
#endif

#endif
/*---------------*/


/*---------------Configuration Section about video frame size*/



/*#define PAL_VDO_YBUF_SIZE             (PAL_FRAME_WIDTH * PAL_FRAME_HEIGHT)   //0x65400*/
/*#define PAL_VDO_CBUF_SIZE             ((PAL_FRAME_WIDTH >> 1) * PAL_FRAME_HEIGHT)   //0x32A00*/

/*#define NTSC_VDO_YBUF_SIZE            (NTSC_FRAME_WIDTH * NTSC_FRAME_HEIGHT)   //0x54600*/
/*#define NTSC_VDO_CBUF_SIZE            ((NTSC_FRAME_WIDTH >> 1) * NTSC_FRAME_HEIGHT)   //0x2A300*/
/*---------------*/


#define ENSURE_CORRECT_TV_MODE        (1)

#define TVD_MAX_APP                   (5)

#define SUPPORT_PWR_MAN_FUNC          (0)
#define SUPPORT_CALIBRATE_BRIGHTNESS  (1)
#define SUPPORT_MODE_CHANGE_PROC      (1)
#if defined(__ARM2__)
/*for arm2*/
#define TVD_DRV_SUPPORT_DIGITAL            (1)
#define TVD_DRV_SUPPORT_ATV            (1)
#define TVD_DRV_ATV_PORT_NUM         (1)
#define TVD_DRV_CAMERA_PORT_NUM    (2)
#elif defined(__UBOOT__)
/*for uboot*/
#else
/*for arm1*/
#endif
#endif


