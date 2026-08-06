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

#if !defined(__MRDUMP_PRIVATE_H__)

struct pt_regs;

void mrdump_save_current_backtrace(struct pt_regs *regs);
void mrdump_print_crash(struct pt_regs *regs);

#endif
