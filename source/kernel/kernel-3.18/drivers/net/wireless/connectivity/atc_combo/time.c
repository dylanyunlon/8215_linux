/*
* Copyright (c) 2023 AutoChips Inc.
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

/* single task time monitor
 *
   int foo(void)
   {
       unsigned long start_time = jiffies;
       unsigned long timeout = start_time + msecs_to_jiffies(500);
       int ret = 0;

       ret = do_something();
       if (time_is_before_jiffies(timeout)) {
           pr_err(DFT_TAG "[ERROR] %s,%d do_something ret %d time %u ms > 500 ms\n",
                   __func__, __LINE__, ret, jiffies_to_msecs(jiffies - start_time));
           dump_stack();
       }

       return ret;
   }
 */

/* atc_combo_time_check - nested task time monitor
 *
 * Usage:
 *
    // 声明
    void _atc_combo_time_check(const char *func, int line, bool end);

    #define atc_combo_time_check(end) \
        _atc_combo_time_check(__func__, __LINE__, end)

    // 用法
    static void foo(void)
    {
        atc_combo_time_check(false); // 记录开始时间
        do_something();
        atc_combo_time_check(true);  // 超时检查（参数false/true要成对）

        atc_combo_time_check(false);
        do_something();
        atc_combo_time_check(true);
    }


    static void do_something(void) // 可在子函数嵌套调用
    {
        atc_combo_time_check(false);
        atc_combo_time_check(true);
    }
 */

#include "wi_begin.h"
#include <linux/module.h>
#include "wi_end.h"

#include "common.h"

void _atc_combo_time_check(const char *func, int line, bool end)
{
#define ATC_COMBO_DEPTH    20
#define ATC_COMBO_TIMEOUT  500
	static atomic_t s_depth = ATOMIC_INIT(-1);
	static unsigned long s_start_time[ATC_COMBO_DEPTH];
	static unsigned long s_timeout[ATC_COMBO_DEPTH];
	static const char *s_func[ATC_COMBO_DEPTH];
	static int s_line[ATC_COMBO_DEPTH];
	static int s_dump = 0;

	int depth = 0;

	// 嵌套深度越界检查
	if (!end) {
		atomic_inc(&s_depth);
		depth = atomic_read(&s_depth);
		if (depth == 0) {
			s_dump = 0;
		} else if (depth >= ATC_COMBO_DEPTH) {
			COMBO_ERR("%s,%d end[%d] depth[%d] >= %d\n",
					func, line, end, depth, ATC_COMBO_DEPTH);
			//dump_stack();
			return;
		}
	} else {
		depth = atomic_read(&s_depth);
		atomic_dec(&s_depth);
		if (depth == 0) {
			s_dump = 0;
		} else if (depth >= ATC_COMBO_DEPTH) {
			COMBO_ERR("%s,%d end[%d] depth[%d] >= %d\n",
					func, line, end, depth, ATC_COMBO_DEPTH);
			//dump_stack();
			return;
		}
	}

	//COMBO_INFO("%s,%d end[%d] depth[%d]\n", func, line, end, depth);
	//dump_stack();

	if (!end) { // 记录开始时间和调用位置
		s_start_time[depth] = jiffies;
		s_timeout[depth] = jiffies + msecs_to_jiffies(ATC_COMBO_TIMEOUT);
		s_func[depth] = func;
		s_line[depth] = line;
	} else {    // 超时检查
		if (time_is_before_jiffies(s_timeout[depth])) {
			COMBO_ERR("%s,%d depth[%d] %u ms > %d ms\n",
					func, line, depth,
					jiffies_to_msecs(jiffies - s_start_time[depth]),
					ATC_COMBO_TIMEOUT);
			dump_stack();
		}
	}
	if (s_dump == 0 && depth > 0
			// 对嵌套调用最外层到当前时间的超时检查
			&& time_is_before_jiffies(s_timeout[0])) {
		int i;

		s_dump = 1;

		COMBO_ERR("%s,%d end[%d] depth[%d-0] %u ms > %d ms\n",
				func, line, end, depth,
				jiffies_to_msecs(jiffies - s_start_time[0]),
				ATC_COMBO_TIMEOUT);

		dump_stack();

		for (i = 0; i <= depth; i++) { // 打印嵌套路径
			COMBO_ERR("%s,%d depth[%d-0] %u ms\n",
					s_func[i], s_line[i], i,
					jiffies_to_msecs(s_start_time[i] - s_start_time[0]));
		}

		/* dump more debug info */
		//atc_combo_dump_cpu_task_trigger(true);
		//panic(__func__);
	}
}
EXPORT_SYMBOL(_atc_combo_time_check);

MODULE_DESCRIPTION("Check task executing time");
MODULE_ALIAS("atc_combo:time");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
