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
#ifndef _VDP_PULL_DOWN_DATA_H

#define _VDP_PULL_DOWN_DATA_H

/*pattern seq*/
__u32 _au4_22Pattern[] = {
	1, 0, 1, 0
};

__u32 _au4_32Pattern[] = {
	1, 1, 1, 1, 0, 1, 1, 1, 1, 0
};

__u32 _au4_2332Pattern[] = {
	1, 1, 0, 1, 1, 1, 1, 1, 1, 0
};

__u32 _au4_64Pattern[] = {
	1, 1, 0, 0, 0, 0, 1, 1, 0, 0
};

__u32 _au4_55Pattern[] = {
	1, 1, 0, 0, 0, 1, 1, 0, 0, 0
};

__u32 _au4_2224Pattern[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0
};

__u32 _au4_32322Pattern[] = {
	1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1
};


__u32 _au4_87Pattern[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0
};

/* Merge Patten*/
__u32 _au4MergeSeq_22[] = {
	1, 0, 1, 0
};

__u32 _au4MergeSeq_32[] = {
	1, 0, 1, 0,   0, 1, 0, 1,   0, 0
};

__u32 _au4MergeSeq_2332[] = {
	1, 0, 0, 1,   0, 1, 0, 1,   0, 0
};

__u32 _au4MergeSeq_64[] = {
	1, 0, 0, 0,   0, 0, 1, 0,   0, 0
};

__u32 _au4MergeSeq_55[] = {
	1, 0, 1, 0,   0, 1, 0, 0,   1, 0
};

__u32 _au4MergeSeq_2224[] = {
	1, 0, 1, 0,   1, 0, 1, 0,   1, 0
};

__u32 _au4MergeSeq_32322[] = {
	1, 0, 0, 1,   0, 1, 0, 0,   1, 0, 1, 0
};

__u32 _au4MergeSeq_87[] = {
	1, 0, 0, 0,   0, 0, 0, 0,   1, 0, 0, 0,   0, 0, 0,
	1, 0, 0, 0,   0, 0, 0, 0,   1, 0, 0, 0,   0, 0, 0
};


/* patten table*/

TABLE_T _r22Patern = {
	_au4_22Pattern,
	TABLE_SIZE(_au4_22Pattern),
};

TABLE_T _r32Patern = {
	_au4_32Pattern,
	TABLE_SIZE(_au4_32Pattern),
};

TABLE_T _r2332Patern = {
	_au4_2332Pattern,
	TABLE_SIZE(_au4_2332Pattern),
};

TABLE_T _r64Patern = {
	_au4_64Pattern,
	TABLE_SIZE(_au4_64Pattern),
};

TABLE_T _r55Patern = {
	_au4_55Pattern,
	TABLE_SIZE(_au4_55Pattern),
};

TABLE_T _r2224Patern = {
	_au4_2224Pattern,
	TABLE_SIZE(_au4_2224Pattern),
};

TABLE_T _r32322Patern = {
	_au4_32322Pattern,
	TABLE_SIZE(_au4_32322Pattern),
};

TABLE_T _r87Patern = {
	_au4_87Pattern,
	TABLE_SIZE(_au4_87Pattern),
};

/* merge order talble*/

TABLE_T *_aprPattern[] = {
	NULL,
	&_r22Patern,
	&_r32Patern,
	&_r2332Patern,
	&_r64Patern,
	&_r55Patern,
	&_r2224Patern,
	&_r32322Patern,
	&_r87Patern,
};

TABLE_T  _arMergeTable[] = {
	{
		0,
		0
	},
	{
		_au4MergeSeq_22,
		TABLE_SIZE(_au4MergeSeq_22),
	},
	{
		_au4MergeSeq_32,
		TABLE_SIZE(_au4MergeSeq_32),
	},
	{
		_au4MergeSeq_2332,
		TABLE_SIZE(_au4MergeSeq_2332),
	},
	{
		_au4MergeSeq_64,
		TABLE_SIZE(_au4MergeSeq_64),
	},
	{
		_au4MergeSeq_55,
		TABLE_SIZE(_au4MergeSeq_55),
	},/*5*/
	{
		_au4MergeSeq_2224,
		TABLE_SIZE(_au4MergeSeq_2224),
	},/*6*/

	{
		_au4MergeSeq_32322,
		TABLE_SIZE(_au4MergeSeq_32322),
	},/*7*/
	{
		_au4MergeSeq_87,
		TABLE_SIZE(_au4MergeSeq_87),
	},/*8*/
};


#endif


