#include "checksum.h"

uint32_t checksum32 (uint32_t chksum, char *buf, int len)
{
    char *end;

    for (end = buf + len; buf < end; ++buf)
        chksum += *buf;
    return chksum;
}

