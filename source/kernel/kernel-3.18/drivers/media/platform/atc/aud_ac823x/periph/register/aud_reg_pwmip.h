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

/**
 * @file aud_reg_pwmip.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_PWMIP_H
#define _AUD_REG_PWMIP_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif



#define AUD_REG_PWMIP_BASE          (0xF00)

//pwm data register
#define AUD_REG_PWMIP_PDATA         (AUD_REG_PWMIP_BASE + 0x00)

//pwm global control register 0
#define AUD_REG_PWMIP_PGCTRL0       (AUD_REG_PWMIP_BASE + 0x01)

//pwm global control register 1
#define AUD_REG_PWMIP_PGCTRL1       (AUD_REG_PWMIP_BASE + 0x02)

//pwm output enable register
#define AUD_REG_PWMIP_POE           (AUD_REG_PWMIP_BASE + 0x04)

//pwm interrupt control register
#define AUD_REG_PWMIP_PIC           (AUD_REG_PWMIP_BASE + 0x05)

//pwm status register
#define AUD_REG_PWMIP_PSTAT         (AUD_REG_PWMIP_BASE + 0x06)

//pwm gpio register
#define AUD_REG_PWMIP_PGDR          (AUD_REG_PWMIP_BASE + 0x07)

//pwm coefficient address register
#define AUD_REG_PWMIP_PCADDR        (AUD_REG_PWMIP_BASE + 0x08)

//pwm coefficient data register
#define AUD_REG_PWMIP_PCDATA        (AUD_REG_PWMIP_BASE + 0x09)

//pwm coefficient data register
#define AUD_REG_PWMIP_P0PIN         (AUD_REG_PWMIP_BASE + 0x0A)

//pwm coefficient data register
#define AUD_REG_PWMIP_P1PIN         (AUD_REG_PWMIP_BASE + 0x0B)

//pwm coefficient data register
#define AUD_REG_PWMIP_P2PIN         (AUD_REG_PWMIP_BASE + 0x0C)

//pwm coefficient data register
#define AUD_REG_PWMIP_P3PIN         (AUD_REG_PWMIP_BASE + 0x0D)

//pwm channel 0 control register
#define AUD_REG_PWMIP_P0CTRL        (AUD_REG_PWMIP_BASE + 0x0E)

//pwm channel 1 control register
#define AUD_REG_PWMIP_P1CTRL        (AUD_REG_PWMIP_BASE + 0x0F)

//pwm channel 2 control register
#define AUD_REG_PWMIP_P2CTRL        (AUD_REG_PWMIP_BASE + 0x10)

//pwm channel 3 control register
#define AUD_REG_PWMIP_P3CTRL        (AUD_REG_PWMIP_BASE + 0x11)

//pwm channel 4 control register
#define AUD_REG_PWMIP_P4CTRL        (AUD_REG_PWMIP_BASE + 0x12)

//pwm channel 5 control register
#define AUD_REG_PWMIP_P5CTRL        (AUD_REG_PWMIP_BASE + 0x13)

//pwm coefficient data register
#define AUD_REG_PWMIP_P4PIN         (AUD_REG_PWMIP_BASE + 0x14)

//pwm coefficient data register
#define AUD_REG_PWMIP_P5PIN         (AUD_REG_PWMIP_BASE + 0x15)

//pwm coefficient data register
#define AUD_REG_PWMIP_P6PIN         (AUD_REG_PWMIP_BASE + 0x16)

//pwm coefficient data register
#define AUD_REG_PWMIP_P7PIN         (AUD_REG_PWMIP_BASE + 0x17)

//pwm coefficient data register
#define AUD_REG_PWMIP_P8PIN         (AUD_REG_PWMIP_BASE + 0x18)

//pwm coefficient data register
#define AUD_REG_PWMIP_P9PIN         (AUD_REG_PWMIP_BASE + 0x19)

//pwm coefficient data register
#define AUD_REG_PWMIP_P10PIN        (AUD_REG_PWMIP_BASE + 0x1A)

//pwm coefficient data register
#define AUD_REG_PWMIP_P11PIN        (AUD_REG_PWMIP_BASE + 0x1B)

//pwm error testing word
#define AUD_REG_PWMIP_PERROR        (AUD_REG_PWMIP_BASE + 0x1C)

//pwm output to pad status
#define AUD_REG_PWMIP_PGDRMUX       (AUD_REG_PWMIP_BASE + 0x1D)

//pwm ramp function flag status
#define AUD_REG_PWMIP_RAMPFLAG      (AUD_REG_PWMIP_BASE + 0x1E)


#ifdef __cplusplus
        }
#endif
                            
#endif // _AUD_REG_PWMIP_H
