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
//#include <sysdep.h>

/* void *memset (dstpp, c, len) */

//ENTRY(memset)
	.text
	.global	memset
	.type	memset, %function
	.align	2
memset:
	mov	r3, r0
	cmp	r2, #8
	bcc	2f		@ less than 8 bytes to move

1:
	tst	r3, #3		@ aligned yet?
	strneb	r1, [r3], #1
	subne	r2, r2, #1
	bne	1b

	and	r1, r1, #255	@ clear any sign bits
	orr	r1, r1, r1, lsl $8
	orr	r1, r1, r1, lsl $16

1:
	subs	r2, r2, #8
	strcs	r1, [r3], #4	@ store up to 32 bytes per loop iteration
	strcs	r1, [r3], #4
	subcss	r2, r2, #8
	strcs	r1, [r3], #4
	strcs	r1, [r3], #4
	subcss	r2, r2, #8
	strcs	r1, [r3], #4
	strcs	r1, [r3], #4
	subcss	r2, r2, #8
	strcs	r1, [r3], #4
	strcs	r1, [r3], #4
	bcs	1b

	and	r2, r2, #7
2:
	subs	r2, r2, #1	@ store up to 4 bytes per loop iteration
	strcsb	r1, [r3], #1
	subcss	r2, r2, #1
	strcsb	r1, [r3], #1
	subcss	r2, r2, #1
	strcsb	r1, [r3], #1
	subcss	r2, r2, #1
	strcsb	r1, [r3], #1
	bcs	2b
	
//	DO_RET(lr)
	bx	lr

	.size	memset, . - memset
//END(memset)
//libc_hidden_builtin_def (memset)
