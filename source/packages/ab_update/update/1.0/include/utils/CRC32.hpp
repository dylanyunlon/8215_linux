/*
 * Used for crc32.c
 */

#ifndef __CRC_H_
#define __CRC_H_

#include <stdint.h>
#include <sys/types.h>

//uint32_t crc32 (uint32_t, const unsigned char *, uint);
uint32_t crc32_no_comp (uint32_t crc, const uint8_t *buf, uint32_t len);

#endif
