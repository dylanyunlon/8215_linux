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
 
#ifndef ___VDP_RAI_H_
#define ___VDP_RAI_H_


typedef struct
{
  UINT32          Mask[0x100 / 4];
  UINT32          Val[0x100 / 4];
  UINT32          HwReg;
  UINT32          *pSwReg;
  UINT64          Mode;
  UINT32          AddrStart;  
} RAI_SHADOW_T;

#define RAI_IN_THE_RANGE(r_sh, addr)   (((r_sh).AddrStart <= (addr)) && (((r_sh).AddrStart + 256) > (addr)))

void  RAI_Init(void);
INT32 RAI_Read(UINT32 *val, UINT32 address);
INT32 RAI_Write(UINT32 val, UINT32 address);
INT32 RAI_GetAccess(RAI_SHADOW_T **ppAcc, UINT32 address);
INT32 RAI_LockWrite(RAI_SHADOW_T *pAcc, INT32 idx, UINT32 mask, UINT32 val);
INT32 RAI_UnlockWrite(RAI_SHADOW_T *pAcc, INT32 idx, UINT32 mask);
void  RAI_Update(RAI_SHADOW_T *pAcc);
void  RAI_UpdateFmt(void);
void  RAI_UpdateMvdo(void);

#endif
