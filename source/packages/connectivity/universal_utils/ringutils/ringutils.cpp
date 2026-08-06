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


#include "ringutils.h"
#include <string.h>
#include "clog.h"

namespace universal_utils {

static const char *TAG = "RingUtils";

RingUtils::RingUtils()
{

}

RingUtils::~RingUtils()
{

}

void RingUtils::bufInit(RingBuf *ring, char buf[], int len)
{
    ring->pStart = (unsigned char*) buf;
    ring->pEnd = (unsigned char*) buf + len;    /* 'end' is 1 beyond of the buffer */
    bufFlush(ring);
}
void RingUtils::bufFlush(RingBuf *ring)
{
    ring->pRead = ring->pStart;
    ring->pWrite = ring->pStart;
}

int RingUtils::writeData(RingBuf *ring, const char buf[], int wanted_to_write_len)
{
    int actual_written_len = 0, freespace = 0;
    unsigned char *pPrevWrite = ring->pWrite;
    unsigned char *pPrevRead = ring->pRead;

    /* Directly using the following codes to compute the freespace instead of the function RING_FreeSpace() */
    if (pPrevWrite >= pPrevRead) {
        freespace = ring->pEnd - pPrevWrite;
        if (pPrevRead == ring->pStart) {
            freespace -= 1;
            if (freespace > wanted_to_write_len) {
                memcpy(pPrevWrite, buf, wanted_to_write_len);
                actual_written_len = wanted_to_write_len;
                pPrevWrite += actual_written_len;
            } else {
                /* freespace <= wanted_to_write_len */
                memcpy(pPrevWrite, buf, freespace);
                actual_written_len = freespace;
                /* pWrite pointer is wrapped to the start position of this Ring buffer */
                pPrevWrite += actual_written_len;
            }
        } else {
            if (freespace >= wanted_to_write_len) {
                memcpy(pPrevWrite, buf, wanted_to_write_len);
                actual_written_len = wanted_to_write_len;
                pPrevWrite += actual_written_len;
                if (pPrevWrite == ring->pEnd) {
                    pPrevWrite = ring->pStart;
                }
            } else {
                /* Two memcpy is necessary */
                int second_freespace = pPrevRead - ring->pStart - 1;
                int second_wanted_write_len = 0;

                memcpy(pPrevWrite, buf, freespace);
                actual_written_len = freespace;
                pPrevWrite += actual_written_len;

                /* Wrap to start position anc continous to write data! */
                pPrevWrite = ring->pStart;
                second_wanted_write_len = wanted_to_write_len - actual_written_len;
                if (second_freespace >= second_wanted_write_len) {
                    memcpy(pPrevWrite, buf + actual_written_len, second_wanted_write_len);
                    /* Add this time's written len */
                    actual_written_len += second_wanted_write_len;
                    pPrevWrite += second_wanted_write_len;
                } else {
                    memcpy(pPrevWrite, buf + actual_written_len, second_freespace);
                    /* Add this time's written len */
                    actual_written_len += second_freespace;
                    pPrevWrite += second_freespace;
                }
            }
        }
        ring->pWrite = pPrevWrite;
    } else {
        freespace = pPrevRead - pPrevWrite - 1;
        if (freespace > wanted_to_write_len) {
            memcpy(pPrevWrite, buf, wanted_to_write_len);
            actual_written_len = wanted_to_write_len;
            pPrevWrite += actual_written_len;
        } else {
            memcpy(pPrevWrite, buf, freespace);
            actual_written_len = freespace;
            /* pWrite pointer is moved forward, in case2: it is not necessary to wrap to start position */
            pPrevWrite += actual_written_len;
        }
        ring->pWrite = pPrevWrite;
    }
    return actual_written_len;

}

int RingUtils::readData(RingBuf *ring, char buf[], int wanted_to_read_len)
{
    int actual_read_len = 0;

    unsigned char *pPrevWrite = ring->pWrite;
    unsigned char *pPrevRead = ring->pRead;
    int total_data_len = 0;

    if (pPrevWrite >= pPrevRead) {
        int readable_len = 0;
        readable_len = pPrevWrite - pPrevRead;
        if (readable_len > wanted_to_read_len)
        {
            memcpy(buf, pPrevRead, wanted_to_read_len);
            actual_read_len = wanted_to_read_len;
            pPrevRead += actual_read_len;
        } else {
            memcpy(buf, pPrevRead, readable_len);
            actual_read_len = readable_len;
            pPrevRead += actual_read_len;
        }
        ring->pRead = pPrevRead;
    } else {
        int first_readable_len = 0;

        total_data_len = dataLen(ring);
        first_readable_len = ring->pEnd - pPrevRead;

        if (first_readable_len >= wanted_to_read_len) {
            memcpy(buf, pPrevRead, wanted_to_read_len);
            actual_read_len = wanted_to_read_len;
            pPrevRead += actual_read_len;

            if (pPrevRead == ring->pEnd) {
                pPrevRead = ring->pStart;
            }
        } else {
            int left_wanted_to_read_len = 0, left_available_read_len = 0;

            memcpy(buf, pPrevRead, first_readable_len);
            actual_read_len = first_readable_len;
            pPrevRead = ring->pStart;

            left_wanted_to_read_len = wanted_to_read_len - first_readable_len;
            left_available_read_len = total_data_len - first_readable_len;

            if (left_available_read_len < left_wanted_to_read_len) {
                memcpy(buf + actual_read_len, pPrevRead, left_available_read_len);
                pPrevRead += left_available_read_len;
                actual_read_len += left_available_read_len;
            } else {
                memcpy(buf + actual_read_len, pPrevRead, left_wanted_to_read_len);
                pPrevRead += left_wanted_to_read_len;
                actual_read_len += left_wanted_to_read_len;
            }
        }
        ring->pRead = pPrevRead;
    }

    return actual_read_len;
}
int RingUtils::size(RingBuf *ring)
{
    int size = 0;

    size = (ring->pEnd - ring->pStart);

    return size;
}

int RingUtils::freeSpace(RingBuf *ring)
{
    int freespace = 0;
    unsigned char *pPrevRead = ring->pRead;
    unsigned char *pPrevWrite = ring->pWrite;

    if (pPrevWrite >= pPrevRead) {
        freespace = (ring->pEnd - pPrevWrite) + (pPrevRead - ring->pStart) - 1;
    } else {
        freespace = pPrevRead - pPrevWrite - 1;
    }

    return freespace;
}

int RingUtils::dataLen(RingBuf *ring)
{
    int datalen = 0;
    unsigned char *pPrevRead = ring->pRead;
    unsigned char *pPrevWrite = ring->pWrite;

    if (pPrevWrite >= pPrevRead) {
        datalen = pPrevWrite - pPrevRead;
    } else {
        datalen = (ring->pEnd - pPrevRead) + (pPrevWrite - ring->pStart);
    }

    return datalen;
}

void RingUtils::bufDelete(RingBuf *ring, int len)
{
    unsigned char *pPrevRead = ring->pRead;

    if ((pPrevRead + len) >= ring->pEnd) {
        pPrevRead = ring->pStart + (pPrevRead + len - ring->pEnd);
    } else {
        pPrevRead += len;
    }

    ring->pRead = pPrevRead;
}

int RingUtils::readDataNoDelete(RingBuf *ring, char buf[], int wanted_to_read_len)
{
    int actual_read_len = 0;

    unsigned char *pPrevWrite = ring->pWrite;
    unsigned char *pPrevRead = ring->pRead;
    int total_data_len = dataLen(ring);

    if (pPrevWrite >= pPrevRead) {
        int readable_len = 0;

        readable_len = pPrevWrite - pPrevRead;
        if (readable_len > wanted_to_read_len){
            memcpy(buf, pPrevRead, wanted_to_read_len);
            actual_read_len = wanted_to_read_len;
            pPrevRead += actual_read_len;
        } else {
            memcpy(buf, pPrevRead, readable_len);
            actual_read_len = readable_len;
            pPrevRead += actual_read_len;
        }
    } else {
        int first_readable_len = 0;

        first_readable_len = ring->pEnd - pPrevRead;

        if (first_readable_len >= wanted_to_read_len)
        {
            memcpy(buf, pPrevRead, wanted_to_read_len);
            actual_read_len = wanted_to_read_len;
            pPrevRead += actual_read_len;

            if (pPrevRead == ring->pEnd) {
                pPrevRead = ring->pStart;
            }
        } else {
            int left_wanted_to_read_len = 0, left_available_read_len = 0;

            memcpy(buf, pPrevRead, first_readable_len);
            actual_read_len = first_readable_len;
            pPrevRead = ring->pStart;

            left_wanted_to_read_len = wanted_to_read_len - first_readable_len;
            left_available_read_len = total_data_len - first_readable_len;

            if (left_available_read_len < left_wanted_to_read_len) {
                memcpy(buf + actual_read_len, pPrevRead, left_available_read_len);
                pPrevRead += left_available_read_len;
                actual_read_len += left_available_read_len;
            } else {
                memcpy(buf + actual_read_len, pPrevRead, left_wanted_to_read_len);
                pPrevRead += left_wanted_to_read_len;
                actual_read_len += left_wanted_to_read_len;
            }
            /* pPrevRead pointer will be wrapped to start position of this buffer */

        }
    }

    return actual_read_len;
}

}

