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
#include <linux/vermagic.h>
#include <linux/compiler.h>


MODULE_INFO(vermagic, VERMAGIC_STRING);


#define KBUILD_STR(s) #s

#ifdef CONFIG_MODULE_UNLOAD
#define DECLARE_MODULE(modname) \
module_init(modname##_init); \
module_exit(modname##_exit); \
struct module __this_module \
__attribute__((section(".gnu.linkonce.this_module"))) = { \
 .name = KBUILD_STR(modname), \
 .init = init_module, \
 .exit = cleanup_module,\
 .arch = MODULE_ARCH_INIT, \
};
#else
#define DECLARE_MODULE(modname) \
module_init(modname##_init); \
module_exit(modname##_exit); \
struct module __this_module \
__attribute__((section(".gnu.linkonce.this_module"))) = { \
 .name = KBUILD_STR(modname), \
 .init = init_module, \
 .arch = MODULE_ARCH_INIT, \
};
#endif


static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


//MODULE_AUTHOR("");
MODULE_LICENSE("Proprietary");


