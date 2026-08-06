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

#ifndef PM_H_
#define PM_H_

struct pm_operations {
	int (*suspend)(void *param);
	int (*resume)(void *param);
};


void vPmInit(int argc,void *argv);
void vPmUninit(int argc,void *argv);
int register_pm_ops(struct pm_operations *ops);
int unregister_pm_ops(struct pm_operations *ops);

#endif /* PM_H_ */

