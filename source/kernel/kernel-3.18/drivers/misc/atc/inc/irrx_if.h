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
#ifndef IRRX_IF_H
#define IRRX_IF_H
#ifdef __cplusplus
extern "C"{
#endif

INT32 IRRX_InitMtkIr(void);
INT32 IRRX_PollMtkIr(UINT32 * pu4Key);


#ifdef __cplusplus
}
#endif
#endif
