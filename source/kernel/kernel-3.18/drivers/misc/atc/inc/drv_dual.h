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

#ifndef DRV_DUAL_H
#define DRV_DUAL_H

//============================================================================
// Include files
//============================================================================
#include "types.h"
#include <linux/interrupt.h>

//============================================================================
// Constant definitions
//============================================================================
#define DUAL_DRAMB_OFFSET_ALIGNMENT  0x400;

////////////////

#define INVALID_MESSAGE   (0)
#define ARM1TOARM2        (1)
#define ARM2TOARM1        (2)

#define MODULEIDMASK      (0xFF000000)
#define MODULEIDSHIFT     (24)

#define MESSAGEDIRMASK    (0x00FF0000)   //reserve
#define MESSAGEDIRSHIFT   (16)

#define MESSAGEIDMASK     (0x0000FFFF)

#define GETMODULEID(m)    ((m & MODULEIDMASK) >> MODULEIDSHIFT)
#define GETMESSAGEDIR(m)  ((m & MESSAGEDIRMASK) >> MESSAGEDIRSHIFT)
#define GETMESSAGEID(m)   (m & MESSAGEIDMASK)

#define DUAL_MAX_TASK        4

#define HW_SEM_MSG_UART      (1U<<0)  //Debugging UART0    
#define HW_SEM_BACKCAR_UART  (1U<<1)  //Back Car UART3
#define HW_SEM_BACKCAR_OSD   (1U<<2)  // Back Car OSD3


#define MODULE_AEC   (0x0)
#define MODULE_BCAR  (0x1)
#define MODULE_BOOTANIMATION (0x2)
#define MODULE_ARM2SYSTEMSERVICE  (0x3)
#define MODULE_TEST  (0x0)
#define MODULE_SMEM  (0x0)	// Share Memory
#define MOUDLE_REV2  (0x1)

//#define DIM(m, n) (sizeof(m)/sizeof(n))

//#define MSG_COMBINE(module, messdir, id) ((module<<24)|(messdir<<16)|(id))
#define MSG_COMBINE(module, id) ((module<<24)|(id))



//============================================================================
// Type definitions
//============================================================================


//============================================================================
// Public functions
//============================================================================

EXTERN BOOL fgDualHALInit(void);
EXTERN BOOL fgDualHALStart(void);
EXTERN BOOL fgDualHALStop(void);
EXTERN BOOL fgDualHALINTEachOther(void);

EXTERN BOOL fgDualHALSetOffset(UINT32 u4Offset);

EXTERN UINT32 u4DualHALOffsetAlignment(void);

EXTERN BOOL fgDualHALSetRemap(void);

EXTERN BOOL fgDualHALSetBootUpParameter(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4);
EXTERN BOOL fgDualHALGetBootUpParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);

EXTERN BOOL fgDualHALSetSendCommandParameter(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4);

EXTERN BOOL fgDualHALGetReturnParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);
EXTERN BOOL fgDualHALGetMessage(UINT32 GroupID, UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);
EXTERN void fgClearGroup(void);
EXTERN BOOL HWSendMessage(UINT32 u4MessageHeader, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4);
EXTERN BOOL HWGetMessage(UINT32 u4ModuleID, UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);
EXTERN   int  request_dualarm_irq(unsigned int module, irq_handler_t handler, unsigned long flags, const char *name, void *dev);
#endif  // DRV_DUAL_H

