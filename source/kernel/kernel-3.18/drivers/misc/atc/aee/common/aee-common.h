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

#if !defined(AEE_COMMON_H)
#define AEE_COMMON_H

#define LOGD(fmt, msg...)	pr_notice(fmt, ##msg)
#define LOGV(fmt, msg...)
#define LOGI	LOGD
#define LOGE(fmt, msg...)	pr_err(fmt, ##msg)
#define LOGW	LOGE

int get_memory_size(void);

int in_fiq_handler(void);

int aee_dump_stack_top_binary(char *buf, int buf_len, unsigned long bottom, unsigned long top);

#endif				/* AEE_COMMON_H */
