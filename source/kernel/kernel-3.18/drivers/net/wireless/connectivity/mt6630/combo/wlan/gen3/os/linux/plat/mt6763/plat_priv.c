/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#include "connectivity_build_in_adapter.h"
#include "gl_typedef.h"
#include "typedef.h"

#define MAX_CPU_FREQ     23400000

int kalBoostCpu(unsigned int level)
{
	unsigned long freq = MAX_CPU_FREQ;

	freq = level == 0 ? 0 : freq;

	if (level >= 1)
		spm_resource_req(SPM_RESOURCE_USER_CONN, SPM_RESOURCE_ALL); /* Disable deepidle/SODI */
	else
		spm_resource_req(SPM_RESOURCE_USER_CONN, 0); /* Enable deepidle/SODI */

	mt_ppm_sysboost_freq(BOOST_BY_WIFI, freq);

	return 0;
}


