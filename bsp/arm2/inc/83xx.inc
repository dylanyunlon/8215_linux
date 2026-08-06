/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
/*
 * CPU related
 */
 
#define LOCKOUT    0xC0               /*Interrupt lockout value      */
#define LOCK_MSK   0xC0               /*Interrupt lockout mask value */
#define MODE_USR     0x10
#define MODE_FIQ     0x11
#define MODE_IRQ     0x12
#define MODE_SVC     0x13
#define MODE_ABT     0x17
#define MODE_UDF     0x1B
#define MODE_MASK    0x1F 
 
#define BIT_I		   0x80
#define BIT_F		   0x40

//#define INIT_CPSR      0xD3   @( MODE_SVC | BIT_I | BIT_F )
#define INIT_CPSR      0xDF   @( MODE_SYS | BIT_I | BIT_F )

/*CP15 control register (r1) definitions*/

#define CR_MMU                               (1 << 0)           @ MMU enable
#define CR_ALIGNMENT                         (1 << 1)           @Alignment fault checking
#define CR_DCACHE                            (1 << 2)           @ D-cache enable
#define CR_ICACHE                            (1 << 12)          @I-cache enable
#define CR_UNALIGNMENT_SUPPORT               (1 << 22)          @ Unalignment Support


/*IO Base*/
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
	#define IO_BASE		     0xF0000000
#elif defined(CONFIG_ATC_PLATFORM_ac823x)
	#define IO_BASE		     0x10000000
#endif
#define CKGEN_BASE	   IO_BASE + 0x0000

#define ARGS_FROM_BOOTLOADER_ADDRESS    (0x400)

/*BIM*/


/*stack*/
#define ARM2_IRQ_STACK_SIZE     (64 * 1024)
#define ARM2_IRQ_STACK_BASE     (0x1000 + ARM2_IRQ_STACK_SIZE)

#define ARM2_EXCPTION_STACK_SIZE     (32 * 1024)
#define ARM2_EXCPTION_STACK_BASE     (ARM2_IRQ_STACK_BASE + ARM2_EXCPTION_STACK_SIZE)


#define SYS_STACK_SIZE        (64 * 1024)
#define SYS_STACK_BASE        (ARM2_EXCPTION_STACK_BASE + SYS_STACK_SIZE)

#define SVC_STACK_SIZE        (28 * 1024)
#define SVC_STACK_BASE        (SYS_STACK_BASE + SVC_STACK_SIZE)

/*arm2 memory offset*/
#define BIM_ARM2_OFFSET_REG_ADD    IO_BASE + 0x45020


#define FDT_LOAD_ADDR (0x2000000)
