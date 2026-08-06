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



#ifndef DMX_CFA_AUDIO_IN_H
#define DMX_CFA_AUDIO_IN_H

/* Old C header file */
#ifdef __cplusplus
extern "C" {

#endif	/* 
 */

typedef struct {
	__u64 u8Sa;	/* < start file offset, 0-based */
	__u64 u8Ea;	/* < end file offset.    The byte of this offset is transferred. */
} CfaAudInPR;

typedef struct {
	__u32 u4AudioByteRate;
	bool fgAc3Type;
} CfaAudInCfgInf;

/* Old C header file */
#ifdef __cplusplus
}
#endif	/* 
 */

#endif				/* DMX_CFA_AUDIO_IN_H */
