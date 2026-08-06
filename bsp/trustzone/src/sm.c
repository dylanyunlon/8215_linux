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

#include "sm.h"
#include "sm_private.h"

static struct sm_nsec_ctx_t sm_nsec_ctx = {0};
static struct sm_sec_ctx_t sm_sec_ctx = {0};
static struct sm_reg_r0_to_r3 sm_regs = {0};

/*
 * Has to match layout of thread_vector_table. Some of the entries are
 * never used.
 *
 * We're using this layout to be able to used the same vector when this
 * secure monitor is used and when the secure monitor in ARM Trusted
 * Firmware is used.
 */
static struct {
	unsigned int std_smc_entry;
	unsigned int fast_smc_entry;
	unsigned int cpu_on_entry;
	unsigned int cpu_off_entry;
	unsigned int cpu_resume_entry;
	unsigned int cpu_suspend_entry;
	unsigned int fiq_entry;
	unsigned int system_off_entry;
	unsigned int system_reset_entry;
} *sm_entry_vector;

struct sm_nsec_ctx_t *sm_get_nsec_ctx(void)
{
	return &sm_nsec_ctx;
}

struct sm_sec_ctx_t *sm_get_sec_ctx(void)
{
	return &sm_sec_ctx;
}

extern void smc_entry_func(void);
void sm_set_sec_smc_entry(void)
{
#if 0

	struct sm_sec_ctx_t *sec_ctx = sm_get_sec_ctx();
	//int c;


	if (TEESMC_IS_FAST_CALL(regs->r0))
		sec_ctx->mon_lr = (unsigned int)&sm_entry_vector->fast_smc_entry;
	else
#endif
	//sec_ctx->mon_lr = (unsigned int)&smc_entry_func;
	smc_entry_func();
}

unsigned int sm_set_nsec_ret_vals(struct sm_reg_r0_to_r3 *regs, unsigned int r4)
{
#if 0
	if (regs->r0 == TEESMC_OPTEED_RETURN_FIQ_DONE) {
		/* On FIQ exit we're restoring r0-r3 from nsec context */
		struct sm_nsec_ctx_t *nsec_ctx = sm_get_nsec_ctx();

		regs->r0 = nsec_ctx->r0;
		regs->r1 = nsec_ctx->r1;
		regs->r2 = nsec_ctx->r2;
		regs->r3 = nsec_ctx->r3;
	} else {
#endif
		/* On all other exits we're shifting r1-r4 into r0-r3 */
		regs->r0 = sm_regs.r1;
		regs->r1 = sm_regs.r2;
		regs->r2 = sm_regs.r3;
		regs->r3 = r4;
	//}
	return sm_regs.r1;
}

void sm_save_regs(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3)
{
    sm_regs.r0 = r0;
    sm_regs.r1 = r1;
    sm_regs.r2 = r2;
    sm_regs.r3 = r3;
}

#define KEY_1_4_LENGTH              (308)
static char *strncpy(char *dest, const char *src, unsigned int count)
{
	char *tmp = dest;

	while (count) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}

static unsigned char bData[1248];

static void *memcpy(void *__dest, __const void *__src, unsigned int __n)
{
	int i = 0;
	unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

	for (i = __n >> 3; i > 0; i--) {
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
	}

	if (__n & 1 << 2) {
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
	}

	if (__n & 1 << 1) {
		*d++ = *s++;
		*d++ = *s++;
	}

	if (__n & 1)
		*d++ = *s++;

	return __dest;
}

void smc_handle_func(unsigned long u4physAddr)
{
    struct smc_param *param = (struct smc_param *)u4physAddr;

    switch (param->cmd_id){
        case TEE_SMC_CALL_Efuse_GetChipFeature:
        {
    	    param->ret = fgGetChipFeature(param->data[0]);
    	    break;
    	}
    	case TEE_SMC_CALL_Efuse_FeatureInit:
        {
    	    featureInit(param->data[0], param->data[1], param->data[2], param->data[3]);
    	    param->ret = TEE_SMC_OK;
    	    break;
    	}
    	case TEE_SMC_CALL_HDCP_DECRYPT_KEY:
        {            
           // memcpy(bData, param->data, 1248);
            
    	    LoadHDCPKeyToSRAM((unsigned char*)param->data);
    	    param->ret = TEE_SMC_OK;
    	    break;
    	}
    	default:
    	    break;
    }
    
    return;    
}


