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



/*-----------------------------------------------------------------------------

* $RCSfile: cfa_mat_dec.h,v $

* $Date: 2016/03/17 $

* $Author: shuhui.zhang $

*

* Description:

*		The C header file of the interface for MAT Decoder

*---------------------------------------------------------------------------*/

#ifndef CFA_AUDIN_MATDEC_H

#define CFA_AUDIN_MATDEC_H

/* Old C header file */

#ifdef __cplusplus
extern "C" {

#endif	/*  */

#include "cfa_audin.h"

/* MLP defines*/
#define CFA_MATDEC_AU_MIN_SIZE	6	/* bytes*/
#define CFA_MATDEC_AU_MAX_SIZE	4000	/*bytes*/
#define MAT_PAYLOAD_SIZE		15344	/*words*/
#define FORMATSYNC_FBB			0xf8726fbb
#define FORMATSYNC_FBA			0xf8726fba

#if CFA_AUDIN_SUPPORT_MAT
	void vCfaAudInMatUnlockProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatCheckMainHeaderProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatCheckTocHeaderProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatCheckBocHeaderProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatLoadTocPayloadProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatLoadBocPayloadProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatCheckTocFooterProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vCfaAudInMatCheckBocFooterProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	s32 i4CfaAudInMatDecProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);

#endif	/*  */

/* Old C header file */

#ifdef __cplusplus
}
#endif	/*  */

#endif				/* _CFA_AUDIO_IN_MATDEC_H_*/
