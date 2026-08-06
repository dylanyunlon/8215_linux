/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SM_SM_H
#define SM_SM_H

typedef enum TEE_DRV_SMC_CALL_TYPE
{    
    TEE_SMC_CALL_Efuse_GetChipFeature,
    TEE_SMC_CALL_Efuse_FeatureInit,
    TEE_SMC_CALL_HDCP_DECRYPT_KEY,
    TEE_SMC_CALL_Efuse_UNKNOWN,
}TEE_DRV_SMC_CALL_TYPE;


struct sm_nsec_ctx_t {
	unsigned int usr_sp;
	unsigned int usr_lr;
	unsigned int irq_spsr;
	unsigned int irq_sp;
	unsigned int irq_lr;
	unsigned int svc_spsr;
	unsigned int svc_sp;
	unsigned int svc_lr;
	unsigned int abt_spsr;
	unsigned int abt_sp;
	unsigned int abt_lr;
	unsigned int und_spsr;
	unsigned int und_sp;
	unsigned int und_lr;
	unsigned int mon_lr;
	unsigned int mon_spsr;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	/* Only stored on FIQ entry */
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
};

struct sm_sec_ctx_t {
	unsigned int usr_sp;
	unsigned int usr_lr;
	unsigned int irq_spsr;
	unsigned int irq_sp;
	unsigned int irq_lr;
	unsigned int svc_spsr;
	unsigned int svc_sp;
	unsigned int svc_lr;
	unsigned int abt_spsr;
	unsigned int abt_sp;
	unsigned int abt_lr;
	unsigned int und_spsr;
	unsigned int und_sp;
	unsigned int und_lr;
	unsigned int mon_lr;
	unsigned int mon_spsr;
	unsigned int entry_reason;
};

struct smc_param {
	unsigned int cmd_id;
	unsigned int ret;
	unsigned int data[0];
	/*
	unsigned int a1;
	unsigned int a2;
	unsigned int a3;
	unsigned int a4;
	unsigned int a5;
	unsigned int a6;
	unsigned int a7;*/
};

#define TEE_SMC_OK 0

/* Returns storage location of non-secure context for current CPU */
struct sm_nsec_ctx_t *sm_get_nsec_ctx(void);

/* Returns storage location of secure context for current CPU */
struct sm_sec_ctx_t *sm_get_sec_ctx(void);

void sm_save_regs(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3);


#if 0
/* Returns stack pointer to use in monitor mode for current CPU */
void *sm_get_sp(void);


/*
 * Initializes secure monitor, must be called by each CPU
 */
void sm_init(vaddr_t stack_pointer);

void sm_set_entry_vector(void *entry_vector);
#endif

void smc_handle_func(unsigned long u4physAddr);


#endif /*SM_SM_H*/
