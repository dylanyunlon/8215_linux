#ifndef __STORAGE_UTILS_H__
#define __STORAGE_UTILS_H__

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

int format_partition(const char *mount_path, const char *fs_type, bool force_format);

int format_partition2(const char *device_node, const char *fs_type, int force_format);

#endif