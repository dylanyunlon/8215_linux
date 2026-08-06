/*
* Copyright (c) 2016 AutoChips Inc.
*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/



#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "src_api.h"

static SRC_struct gSrcStructUp;
static int gSrcInitedUp = 0;

static SRC_struct gSrcStructDown;
static int gSrcInitedDown = 0;


static void bt_src_init_up()
{
    if(0 == gSrcInitedUp)
    {
	    SRC_init(&gSrcStructUp);
	    gSrcInitedUp = 1;
    }
}


static void bt_src_init_down()
{
    if(0 == gSrcInitedDown)
    {
	    SRC_init(&gSrcStructDown);
	    gSrcInitedDown = 1;
    }
}

void samples8kToSamples16k(unsigned char* input8k, unsigned char* output16k)
{
	bt_src_init_up();
	SRC_upsample_x2(&gSrcStructUp, input8k, output16k);
	return;
}

void samples16kToSamples8k(unsigned char* input16k, unsigned char* output8k)
{
	bt_src_init_down();
	SRC_downsample_x2(&gSrcStructDown, input16k, output8k);
	return;
}

