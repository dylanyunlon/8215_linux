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
#include <alsa/asoundlib.h>

#include <syslog.h>

#define BT_SCO_ALSA "btScoAlsa"



int alsa_set_hw_params(snd_pcm_t *handle, int channels, int sample_rate, snd_pcm_format_t format)
{
	snd_pcm_hw_params_t *hwparams;
	unsigned int exact_rate;
	unsigned int  buffer_time, period_time;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);

	/* Init hwparams with full configuration space */
	if (snd_pcm_hw_params_any(handle, hwparams) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_any\n");
		goto err;
	}

	if (snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_set_access\n");
		goto err;
	}

	/* Set sample format */
	if (snd_pcm_hw_params_set_format(handle, hwparams, format) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_set_format\n");
		goto err;
	}

	/* Set number of channels */
	if (snd_pcm_hw_params_set_channels(handle, hwparams, channels) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_set_channels\n");
		goto err;
	}
	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */
	exact_rate = sample_rate;
	if (snd_pcm_hw_params_set_rate_near(handle, hwparams, &exact_rate, 0) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_set_rate_near\n");
		goto err;
	}
	if (sample_rate != exact_rate) {
		printf(BT_SCO_ALSA, "The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n",
				sample_rate, exact_rate);
	}

	if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_get_buffer_time_max\n");
		goto err;
	}
	if (buffer_time > 500000) buffer_time = 500000;
	/*period_time = buffer_time / 4;*/
	period_time = 20000;/*20ms*/

	if (snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, 0) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_set_buffer_time_near\n");
		goto err;
	}

	if (snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, 0) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params_set_period_time_near\n");
		goto err;
	}

	/* Set hw params */
	if (snd_pcm_hw_params(handle, hwparams) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_hw_params(handle, params)\n");
		goto err;
	}


	return 0;

err:
	return -1;
}



int alsa_pcm_open(snd_pcm_t **pHandle, char* card, int stream, snd_output_t ** ppLog)
{
	if (snd_output_stdio_attach(ppLog, stderr, 0) < 0) {
		printf(BT_SCO_ALSA, "Error snd_output_stdio_attach\n");
		goto err;
	}

	if (snd_pcm_open(pHandle, card, stream, 0) < 0) {
		printf(BT_SCO_ALSA, "Error snd_pcm_open [ %s]\n", card);
		goto err;
	}

	return 0;

err:
	if (*ppLog)      snd_output_close(*ppLog);
	if (*pHandle)   snd_pcm_close(*pHandle);
	return -1;
}

int alsa_pcm_close(snd_pcm_t *handle, snd_output_t *log)
{
	if (log)      snd_output_close(log);
	if (handle)   snd_pcm_close(handle);
	return 0;
}

int alsa_pcm_start(snd_pcm_t *handle)
{
    int ret = snd_pcm_start(handle);

    return (ret);
}

int alsa_pcm_read(snd_pcm_t *handle, unsigned char * buffer, int rcount)
{
	int r;

	if(NULL == handle)
    {
        return 0;
    }
    snd_pcm_sframes_t avail = snd_pcm_avail(handle);



    if (avail < rcount)
    {
        snd_pcm_wait(handle, 20);
    }

    r = snd_pcm_readi(handle, buffer, rcount);

    if (r == -EPIPE)
    {
        printf(BT_SCO_ALSA, "alsa_pcm_read  handle: overrun occurred\n");
    }

	return r;
}

int alsa_pcm_write(snd_pcm_t *handle, unsigned char * buffer, int rcount)
{
    int r = 0;


    if(NULL == handle)
    {
        return 0;
    }

    int avail = snd_pcm_avail(handle);
    if (avail < rcount)
    {
        printf(BT_SCO_ALSA, "alsa_pcm_write snd_pcm_avail:%d, rcount:%d\n", avail, rcount);        
    }

    r = snd_pcm_writei(handle, buffer, rcount);

    if (r == -EPIPE) {
        printf(BT_SCO_ALSA, "alsa_pcm_write  handle: underrun occurred\n");
        snd_pcm_prepare(handle);
        printf(BT_SCO_ALSA, "alsa_pcm_write  snd_pcm_prepare: \n");
        //r = snd_pcm_writei(handle, buffer, rcount);

    } else if (r < 0) {
        printf(BT_SCO_ALSA, "set_pcm_play: error from writei: %s\n", snd_strerror(r));
    }


	return r;
}


int alsa_set_sw_params(snd_pcm_t *handle)
{
    snd_pcm_sw_params_t * swparams;


    snd_pcm_sw_params_alloca(&swparams);
    snd_pcm_sw_params_current(handle, swparams);

    //snd_pcm_sw_params_set_start_threshold(handle, swparams, 2000);
    snd_pcm_sw_params_set_stop_threshold(handle, swparams, -1);

    snd_pcm_sw_params(handle, swparams);
    return 0;
}


