/*
 * Utility functions needed for (some) EABI conformant tool chains.
 *
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 *
 * This program is Free Software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */


int raise (int signum)
{
	return 0;

}


void  __aeabi_unwind_cpp_pr0()
{

}

void  __aeabi_unwind_cpp_pr1()
{

}

void _Unwind_Resume()
{

}

void _Unwind_GetGR()
{

}

void _Unwind_VRS_Get()
{

}

void _Unwind_Backtrace()
{

}

void __gcc_personality_v0()
{

}