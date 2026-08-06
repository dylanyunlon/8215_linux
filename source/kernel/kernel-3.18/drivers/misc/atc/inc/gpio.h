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

#ifndef _GPIO_H_
#define _GPIO_H_

//#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OUTPUT 		1
#define INPUT 		0
#define HIGH        1
#define LOW         0


extern INT32 BSP_PinSet(INT32 i4FuncSel, INT32 i4Func);
extern INT32 BSP_PinGet(INT32 i4FuncSel);
extern void GPIO_InOut_Sel(INT32 i4GpioNum, INT32 i4Output);
extern void GPIO_Output(INT32 i4GpioNum, INT32 i4High);
extern INT32 GPIO_Input(INT32 i4GpioNum);
extern void GPIO_Release(INT32 i4GpioNum);
extern void GPIO_Config(INT32 i4GpioNum, INT32 i4Output, INT32 i4High);

#ifdef __cplusplus
}
#endif

#endif    // _GPIO_H_

