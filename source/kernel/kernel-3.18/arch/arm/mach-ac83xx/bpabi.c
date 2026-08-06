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

extern long long __divdi3 (long long, long long);
extern unsigned long long __udivdi3 (unsigned long long, 
				     unsigned long long);
extern long long __gnu_ldivmod_helper (long long, long long, long long *);
extern unsigned long long __gnu_uldivmod_helper (unsigned long long, 
						 unsigned long long, 
						 unsigned long long *);


long long
__gnu_ldivmod_helper (long long a, long long b, long long *remainder)
{
	long long quotient;

	quotient = __divdi3(a, b);
	*remainder = a - b * quotient;
	return quotient;
}


unsigned long long
__gnu_uldivmod_helper (unsigned long long a, 
		       unsigned long long b,
		       unsigned long long *remainder)
{
  unsigned long long quotient;

  quotient = __udivdi3 (a, b);
  *remainder = a - b * quotient;
  return quotient;
}
