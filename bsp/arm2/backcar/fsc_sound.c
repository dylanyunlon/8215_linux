/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#include "x_typedef.h"
#include "fsc_sound.h"
#include "arm2pcmplay_if.h"

static UINT16 asrc_sine_table_16bit[64]=
{
	0x0000,0x0C8B,0x18F8,0x2528,0x30FB,0x3C56,0x471C,0x5133,
	0x5A82,0x62F2,0x6A6D,0x70E2,0x7641,0x7A7D,0x7D8A,0x7F62,
	0x7FFF,0x7F62,0x7D8A,0x7A7D,0x7641,0x70E2,0x6A6D,0x62F2,
	0x5A82,0x5133,0x471C,0x3C56,0x30FB,0x2528,0x18F8,0x0C8B,
	0x0000,0xF374,0xE707,0xDAD7,0xCF04,0xC3A9,0xB8E3,0xAECC,
	0xA57D,0x9D0D,0x9592,0x8F1D,0x89BE,0x8582,0x8275,0x809D,
	0x8000,0x809D,0x8275,0x8582,0x89BE,0x8F1D,0x9592,0x9D0D,
	0xA57D,0xAECC,0xB8E3,0xC3A9,0xCF04,0xDAD7,0xE707,0xF374
};

#pragma pack(push, 1)
typedef struct {
    BYTE     riff[4];          // "RIFF"
    UINT32   filesize;         // File size - 8
    BYTE     wave[4];          // "WAVE"
    BYTE     fmt[4];           // "fmt "
    UINT32   fmtsize;          // 0x10
    UINT16   wFormatTag;       // 0x01 -> PCM
    UINT16   nChannels;        // Channels
    UINT32   nSamplesPerSec;  // Sampling Rate (samples per second)
    UINT32   nAvgBytesPerSec; // Average Bytes per second
    UINT16   nBlockAlign;
    UINT16   wBitsPerSample;   // Bits per sample
    BYTE     data[4];          // "data"
    UINT32   datasize;         // Data Size(Byte)
} WaveHeader;
#pragma pack (pop)


BOOL PlaySound(void* psndmem, UINT32 u4sndsz)
{
    WavHeader  *Header = NULL;
    WavFMT       *FMT  = NULL;
    WavDATA    *Data = NULL;
    WaveHeader  *pWavHeader = NULL;
    
    Header = (WavHeader *)psndmem;

    pWavHeader = (WaveHeader *)psndmem;

    FMT = (WavFMT *)((BYTE *)psndmem + sizeof(WavHeader));

    Data = (WavDATA *)((BYTE *)psndmem + sizeof(WavHeader) + sizeof(WavFMT));

    Printf("Sample Rate(%d),Bits Per Samples(%d) Num Channels %d,data size %d.\r\n",
        FMT->SampleRate, FMT->BitsPerSamples, FMT->NumChannels, pWavHeader->datasize);

    ARM2PCM_FMT rARM2PcmFmt;
    rARM2PcmFmt.u4SampleRate = FMT->SampleRate;
    rARM2PcmFmt.u4BitsPerSamples = FMT->BitsPerSamples;
    rARM2PcmFmt.u4Channels = FMT->NumChannels;
    rARM2PcmFmt.lpData = (LPBYTE)((BYTE *)psndmem + sizeof(WaveHeader));
    rARM2PcmFmt.u4Len = pWavHeader->datasize;
    rARM2PcmFmt.u4Loops = 1;
    
    ARM2PCM_Start(&rARM2PcmFmt);
    
    return TRUE;
}

