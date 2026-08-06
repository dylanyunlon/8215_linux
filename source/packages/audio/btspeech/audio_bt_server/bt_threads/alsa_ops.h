/*
* Copyright (c) 2016 AutoChips Inc.
*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/


#ifndef ALSA_OPS_H_
#define ALSA_OPS_H_



#include <stdio.h>
#include <unistd.h>
#include <alsa/asoundlib.h>


int alsa_pcm_open(snd_pcm_t **pHandle, char* card, int stream, snd_output_t ** ppLog);
int alsa_pcm_close(snd_pcm_t *handle, snd_output_t *log);

int alsa_pcm_start(snd_pcm_t *handle);

int alsa_set_hw_params(snd_pcm_t *handle, int channels, int sample_rate, snd_pcm_format_t format);
int alsa_set_sw_params(snd_pcm_t *handle);

int alsa_pcm_read(snd_pcm_t *handle, unsigned char * buffer, int rcount);
int alsa_pcm_write(snd_pcm_t *handle, unsigned char * buffer, int rcount);


#endif

