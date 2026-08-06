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

#include <linux/module.h>
#include <asm/uaccess.h>
#include "mmisc.h"

int mm_copy_from_user(void *dest, const void *src, unsigned long count)
{
	unsigned long ret = 0;

	if ((NULL == dest) || (NULL == src) || (0 == count)) {
		pr_err("%s fail for invalid args, dest: %p, src: %p, count: %ld\r\n",
			__func__, dest, src, count);
		return -1;
	}
	if (!access_ok(VERIFY_READ, (void __user *)src, count)) {
		pr_err("%s failed in access_ok check read permission of src(%p)!\r\n",
			__func__, src);
		return -1;
	}
	ret = copy_from_user(dest, (void __user *)src, count);
	if (0 != ret) {
		pr_err("%s fail in copy_from_user -- dest: %p, src: %p, count: %ld, ret: %ld\r\n",
			__func__, dest, src, count, ret);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(mm_copy_from_user);

int mm_copy_to_user(void *dest, const void *src, unsigned long count)
{
	unsigned long ret = 0;

	if ((NULL == dest) || (NULL == src) || (0 == count)) {
		pr_err("%s fail for invalid args, dest: %p, src: %p, count: %ld\r\n",
			__func__, dest, src, count);
		return -1;
	}
	if (!access_ok(VERIFY_WRITE, (void __user *)dest, count)) {
		pr_err("%s failed in access_ok check write permission of dest(%p)!\r\n",
			__func__, dest);
		return -1;
	}
	ret = copy_to_user((void __user *)dest, src, count);
	if (0 != ret) {
		pr_err("%s fail in copy_to_user -- dest: %p, src: %p, count: %ld, ret: %ld\r\n",
			__func__, dest, src, count, ret);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(mm_copy_to_user);
