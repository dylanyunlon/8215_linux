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

#ifndef _X_BUF_DEF_H_
#define _X_BUF_DEF_H_

//#include "dram_model.h"
//#include "section.h"

//
// Memory layout should be like this:
//
// | VFifo[0] | VFifo[1] | PbBuf[1] | PbBuf[0] |
// | <--   expand video fifo    --> |
//


//
// If overlap with frame buffer and no smooth slow reverse,
// Memory layout should be like this:
//
// | VFifo[0] | PbBuf[0] | VFifo[1] | PbBuf[1] |
//                       | <--   overlap   --> |
//


#define VFIFO_ALIGN         4096
#define PBBUF_ALIGN         4096
#define VIDPBBUF_ALIGN         4096

#define X_BUF_ALIGN_MASK(value, mask)			((((value) + ((mask) - 1)) / (mask)) * (mask))


//#define PBBUF_MAX_SIZE  (8 * 1024 * 1024)
#define PBBUF_MAX_SIZE  7864320    //7.5M bytes

#if 0
#define VFIFO_SIZE          X_BUF_ALIGN_MASK((6 * 1024 * 1024 + 12 * 1024),VFIFO_ALIGN)


#define PBBUF_SIZE          X_BUF_ALIGN_MASK(PBBUF_MAX_SIZE,PBBUF_ALIGN)

#define VIDPBBUF_SIZE       (VFIFO_SIZE * 2 + PBBUF_SIZE * 2)
#define EXPANDED_VFIFO_SIZE (VFIFO_SIZE * 2 + PBBUF_SIZE)
#endif

#endif // #ifndef _X_BUF_DEF_H_


