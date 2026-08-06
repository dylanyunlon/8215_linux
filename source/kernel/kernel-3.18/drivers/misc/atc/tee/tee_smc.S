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

#include <linux/linkage.h>

.text
.balign 4
.code 32

	/* void tee_smc_call(struct smc_param *param); */
	.globl	tee_smc_call
ENTRY(tee_smc_call)
    #b .    
	push	{r0, lr}
.arch_extension sec
	smc	#0
	pop	{r0, pc}
ENDPROC(tee_smc_call)
