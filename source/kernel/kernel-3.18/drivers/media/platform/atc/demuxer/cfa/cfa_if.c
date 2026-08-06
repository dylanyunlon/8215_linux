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

#ifndef __linux__
#pragma warning(push)
#pragma warning(disable : 4115)	/* disable warning C4115: named type definition in parentheses */
#endif				/*
				 */

#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_cfa_def.h>
#else				/*
				 */
#include "dmx_define.h"
#include "dmx_cfa_def.h"
#endif				/* __linux__ */

#include "dmx_def.h"
#include "cfa_if.h"
#ifndef __linux__
#pragma warning(pop)
#endif

EXTERN void * CfaAsfGetInterface(void);

EXTERN void *CfaAviGetInterface(void);

EXTERN void *CfaFlvGetInterface(void);

EXTERN void *CfaMkvGetInterface(void);

EXTERN void *CfaMp4GetInterface(void);

EXTERN void *CfaMpgGetInterface(void);

EXTERN void *CfaOgmGetInterface(void);

EXTERN void *CfaRmGetInterface(void);

EXTERN void *CfaAudioGetInterface(void);

EXTERN void *CfaApeGetInterface(void);

EXTERN void *CfaTsGetInterface(void);

#if CONFIG_DRV_HDMI_RX
EXTERN void *CfaAudInGetInterface(void);

#endif				/* CONFIG_DRV_HDMI_RX */

void *CfaGetInterface(u32 u4CfaType)
{
	switch (u4CfaType) {

	case CFA_TYPE_ASF:
		return CfaAsfGetInterface();

	case CFA_TYPE_AVI:
		return CfaAviGetInterface();

	case CFA_TYPE_FLV:
		return CfaFlvGetInterface();

	case CFA_TYPE_MKV:
		return CfaMkvGetInterface();

	case CFA_TYPE_MP4:
		return CfaMp4GetInterface();

	case CFA_TYPE_MPG:
		return CfaMpgGetInterface();

	case CFA_TYPE_OGM:
		return CfaOgmGetInterface();

	case CFA_TYPE_RM:
		return CfaRmGetInterface();

	case CFA_TYPE_APE:
		return CfaApeGetInterface();

	case CFA_TYPE_AUDIO:
		return CfaAudioGetInterface();

	case CFA_TYPE_TS:
		return CfaTsGetInterface();

#if CONFIG_DRV_HDMI_RX
	case CFA_TYPE_AUDIN:
		return CfaAudInGetInterface();
#endif				/* CONFIG_DRV_HDMI_RX */

	default:
		break;

	}

	return NULL;

}
