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
#ifndef _TVE_IF_H_
#define _TVE_IF_H_

#define CCI_ASPECT           1
#define CCI_CGMS             (0x1 << 1)
#define CCI_APS              (0x1 << 2)

extern void vSetNtscVbiSignal(__u32 dwOption, __u32 dwCgms, __u32 dwMVType);
extern void vSetPalVbiSignal(__u32 dwOption, __u32 dwCgms, __u32 dwMVType);
extern void vCCXDSCGMSService(void);

#endif


