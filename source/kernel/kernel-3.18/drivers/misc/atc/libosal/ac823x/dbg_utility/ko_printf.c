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


#include "x_module.h"
#include "x_printf.h"
#include "dbg.h"
#include "x_common.h"

#include <linux/types.h>


//jie.tang
__u32 disable_print=0;

__s32 Printf(const char *ps_format, ...)
{
    __s32 i4_len;
    va_list t_ap;
    __u32 Layer_o=0x00;
    if((disable_print & 0x01U) == (__u32)1)
    {
  	return 0;
    }
    va_start(t_ap, ps_format);
  i4_len=vdbg_printf(Layer_o,(const char *)ps_format, t_ap);
 //   i4_len = vprintk((const char *)ps_format, t_ap);
    va_end(t_ap);

    return (__s32)i4_len;
}
__s32 SYS_Printf(const char *ps_format, ...)
{
    __s32 i4_len;
    va_list t_ap;
    __u32 Layer_o=(__u32)sys;
    if((disable_print & 0x01U) == (__u32)1)
    {
  	return 0;
    }
    va_start(t_ap, ps_format);
  i4_len=vdbg_printf(Layer_o,(const char *)ps_format, t_ap);
    va_end(t_ap);

    return (__s32)i4_len;
}

__s32 dbg_printf (__u32 Layer,const char *ps_format, ...)
{
    __s32 i4_len;
    va_list t_ap;
    __u32 Layer_o=Layer;
    if((disable_print & 0x01U) == (__u32)1)
	return 0;

    va_start(t_ap, ps_format);
    i4_len=vdbg_printf(Layer_o,(const char *)ps_format, t_ap);
    va_end(t_ap);

    return (__s32)i4_len;

}
__s32 vdbg_printf (__u32 Layer,const char *ps_format, va_list t_ap)
{
    __s32 i4_len;
    __u32 dis_layer =0x00;
    dis_layer=(disable_print & Layer);

   if (dis_layer)
   {
	return 0;
   }
    i4_len = vprintk((const char *)ps_format, t_ap);

    return (__s32)i4_len;

}


EXPORT_SYMBOL(Printf);
EXPORT_SYMBOL(dbg_printf);
EXPORT_SYMBOL(vdbg_printf);
EXPORT_SYMBOL(SYS_Printf);


