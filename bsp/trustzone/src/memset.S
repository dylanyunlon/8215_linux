/*!
 * @file memset.S
 *
 * @par LEGAL DISCLAIMER
 *
 * (Header of MediaTek Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, USER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * ARE PROVIDED TO USER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED
 * IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND USER AGREES TO LOOK ONLY TO
 * SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO USER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * USER HEREBY ACKNOWLEDGES THE CONFIDENTIALITY OF MEDIATEK SOFTWARE AND AGREES
 * NOT TO DISCLOSE OR PERMIT DISCLOSURE OF ANY MEDIATEK SOFTWARE TO ANY THIRD
 * PARTY OR TO ANY OTHER PERSON, EXCEPT TO DIRECTORS, OFFICERS, EMPLOYEES OF
 * USER WHO ARE REQUIRED TO HAVE THE INFORMATION TO CARRY OUT THE PURPOSE OF
 * OPENING OR USING THIS FILE.
 *
 *
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
