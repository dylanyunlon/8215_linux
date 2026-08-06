/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>

#include "roots.h"
#include "err_num.h"
#include  <mtdutils.h>
#include <mounts.h>
#include "recovery.h"
#include "make_ext4fs.h"


static int num_volumes = 0;
static Volume* device_volumes = NULL;
extern struct selabel_handle *sehandle;
char *UDISK_ROOT = "/udisk1/";
char *SD_ROOT = "/ext_sdcard1/";
static char *update_path = NULL;
const char *CUR_ROOT = SD_ROOT;


static int update_partition_from_file(Volume* v, const char *file)
{
    int status = UPDATED_SUCCESS;
    if (!v->imgname){
        return (UPDATED_SUCCESS);
    }

    if (!is_file_exist(file)){
        return (UPDATED_ERR_FILE_NOT_EXIST);
    }

    
    if ((!strcmp(v->fs_type, "emmc")) || (!strcmp(v->fs_type, "mtd"))){
        status = update_raw_partition_from_file(v->device, file, v->offset, v->length);
        if (status >= 0)
            status = UPDATED_SUCCESS;
            
    }else if (!strcmp(v->fs_type, "ext4")){
        status = update_ext4_partition_from_file(v->device, file, v->offset, v->length);
    }
    if (status < 0){
        printf("Failed to Update image from %s", file);
    }
    return (status);
}


static int update_all_partion(const char *dir)
{
    int status = 0;
    char  filem[50];
    char  *tmp;
    int i = 0;
    strcpy(filem, dir);
    tmp = filem + strlen(filem);
    for (i= 0; i < num_volumes; ++i)
    {
        Volume* v = &device_volumes[i];
        strcpy(tmp, v->imgname);
        status = update_partition_from_file(v, filem);
        if ((UPDATED_SUCCESS != status) && (UPDATED_ERR_DISK_NOT_EXIST != status)){
            break;
        }
    }
    return (status);
}

int try_update_by_partition_name(const char *parname, const char *dir)
{
    int i;
    int status = 0;
    char  filem[50];
    char  *tmp;
    strcpy(filem, dir);
    if (!strcmp(parname, "allpartition")){
        status = update_all_partion( dir);
    }else{
        for (i = 0; i < num_volumes; ++i) {
            Volume* v = &device_volumes[i];
            if (v->imgname && strstr(v->device, parname)){
                strcat(filem, v->imgname);
                status = update_partition_from_file(v, filem);
            }
        }
    }
    return (status);
}


int try_update_by_file_name(const char *file)
{
    int i;
    int status = 0;
    char  filem[50];
    char  *tmp;
    strcpy(filem, file);
    tmp = strstr(filem, "/updateall");
    if (tmp){
        tmp ++;
        *tmp = 0;
        status = update_all_partion(file);
        return (status);
    }else{
        for (i = 0; i < num_volumes; ++i) {
            Volume* v = &device_volumes[i];
            if (v->imgname && strstr(filem, v->imgname)){
                status = update_partition_from_file(v, filem);
            }
        }
    }
    return (status);
}



void load_volume_table() {
    int alloc = 16;
    device_volumes = (Volume*)malloc(alloc * sizeof(Volume));
    if(device_volumes == NULL){
        printf("malloc error \n");
    }
    printf("reload file from fstab \n");
    // Insert an entry for /tmp, which is the ramdisk and is always mounted.
    device_volumes[0].mount_point = "/tmp";
    device_volumes[0].fs_type = "ramdisk";
    device_volumes[0].device = NULL;
    device_volumes[0].imgname = NULL;
    device_volumes[0].length = 0;
    device_volumes[0].offset = 0;
    num_volumes = 1;

    printf("reload file from fstab 1 \n");
    FILE* fstab = fopen("/etc/recovery.fstab", "r");
    if (fstab == NULL) {
        printf("failed to open /etc/recovery.fstab (%s)\n", strerror(errno));
        return;
    }

    printf("reload file from fstab 2\n");
    char buffer[1024];
    int i;
    while (fgets(buffer, sizeof(buffer)-1, fstab)) {
        for (i = 0; buffer[i] && isspace(buffer[i]); ++i);
        if (buffer[i] == '\0' || buffer[i] == '#') continue;

        char* original = strdup(buffer);

        char* mount_point = strtok(buffer+i, " \t\n");
        char* fs_type = strtok(NULL, " \t\n");
        char* device = strtok(NULL, " \t\n");
        // lines may optionally have a second device, to use if
        // mounting the first one fails.
        char* strtmp = NULL;
        char* imgname = strtok(NULL, " \t\n");
        if (imgname) {
                strtmp = strtok(NULL, " \t\n");
        }
        device_volumes[num_volumes].imgname = NULL;

        if (mount_point && fs_type && device) {
            while (num_volumes >= alloc) 
            {
                printf("will realloc \n");

                alloc *= 2;
                device_volumes = (Volume*)realloc(device_volumes, alloc*sizeof(Volume));
                printf("realloc ok \n");
            }
            device_volumes[num_volumes].mount_point = strdup(mount_point);
            device_volumes[num_volumes].fs_type = strdup(fs_type);
            device_volumes[num_volumes].device = strdup(device);
            if (imgname){
                // Remove / in the image name. 
                if (imgname[0] == '/')
                    imgname ++;
                device_volumes[num_volumes].imgname =  strdup(imgname) ;
            }

            device_volumes[num_volumes].length = 0;
            device_volumes[num_volumes].offset = 0;
            if (strtmp){
                device_volumes[num_volumes].length = strtoll(strtmp, NULL, 0);
                strtmp = strtok(NULL, " \t\n");
                if (strtmp){
                    device_volumes[num_volumes].offset = strtoll(strtmp, NULL, 0);
                }
                
            }
            printf("  %d %s %s %s %s 0x%llx 0x%llx\n", num_volumes, device_volumes[num_volumes].mount_point, device_volumes[num_volumes].fs_type,
            device_volumes[num_volumes].device, device_volumes[num_volumes].imgname, device_volumes[num_volumes].length, device_volumes[num_volumes].offset);
            ++num_volumes;
            if(num_volumes > 16)
                break;
        } else {
            printf("skipping malformed recovery.fstab line: %s\n", original);
        }
        free(original);
    }

    fclose(fstab);

    printf("recovery filesystem table\n");
    printf("=========================\n");
    for (i = 0; i < num_volumes; ++i) {
        Volume* v = &device_volumes[i];
        printf("  %d %s %s %s %s 0x%llx 0x%llx\n", i, v->mount_point, v->fs_type,
               v->device, v->imgname, v->length, v->offset);
    }
    printf("\n");
}

Volume* volume_for_path(const char* path) {
    int i;
    for (i = 0; i < num_volumes; ++i) {
        Volume* v = device_volumes+i;
        int len = strlen(v->mount_point);
        if (strncmp(path, v->mount_point, len) == 0 &&
            (path[len] == '\0' || path[len] == '/')) {
            return v;
        }
    }
    return NULL;
}

int ensure_path_mounted(const char* path) {
    Volume* v = volume_for_path(path);
    int devnum = 1;
    if (v == NULL) {
        printf("unknown volume for path [%s]\n", path);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // the ramdisk is always mounted.
        return 0;
    }

    int result;
    result = scan_mounted_volumes();
    if (result < 0) {
        printf("failed to scan mounted volumes\n");
        return -1;
    }

    const MountedVolume* mv =
        find_mounted_volume_by_mount_point(v->mount_point);
    if (mv) {
        // volume is already mounted
        return 0;
    }

    mkdir(v->mount_point, 0755);  // in case it doesn't already exist

    if (strcmp(v->fs_type, "yaffs2") == 0) {
        // mount an MTD partition as a YAFFS2 filesystem.
        mtd_scan_partitions();
        const MtdPartition* partition;
        partition = mtd_find_partition_by_name(v->device);
        if (partition == NULL) {
            printf("failed to find \"%s\" partition to mount at \"%s\"\n",
                 v->device, v->mount_point);
            return -1;
        }
        return mtd_mount_partition(partition, v->mount_point, v->fs_type, 0);
    } else if (strcmp(v->fs_type, "ext4") == 0 ||
               strcmp(v->fs_type, "vfat") == 0) {
        result = mount(v->device, v->mount_point, v->fs_type,
                       MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");

        if (result == 0) 
            return 0;
        
        if (strstr(v->device, "sda")){
            char device[30];
            for (devnum = 1; devnum < 8; devnum ++)
            {
                sprintf(device, "%s%d", v->device, devnum);
                result = mount(device, v->mount_point, v->fs_type,
                               MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
                if (result == 0) 
                    return 0;
            }
        }


        printf("failed to mount %s (%s)\n", v->mount_point, strerror(errno));
        return -1;
    }

    printf("unknown fs_type \"%s\" for %s\n", v->fs_type, v->mount_point);
    return -1;
}

int ensure_path_unmounted(const char* path) {
    Volume* v = volume_for_path(path);
    if (v == NULL) {
        printf("unknown volume for path [%s]\n", path);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // the ramdisk is always mounted; you can't unmount it.
        return -1;
    }

    int result;
    result = scan_mounted_volumes();
    if (result < 0) {
        printf("failed to scan mounted volumes\n");
        return -1;
    }

    const MountedVolume* mv =
        find_mounted_volume_by_mount_point(v->mount_point);
    if (mv == NULL) {
        // volume is already unmounted
        return 0;
    }

    return unmount_mounted_volume(mv);
}


int format_ext4(const char *blockdev)
{
    pid_t pid = fork();
    if (pid == 0) {
        char dev[40];
        char *argv[]= {"/sbin/make_ext4fs", dev, (char *)NULL};
        strcpy(dev, blockdev);
        execv(argv[0], argv);
        fprintf(stdout, "E:Can't run %s (%s)\n", argv[0], strerror(errno));
        _exit(-1);
    }
    int status;
    waitpid(pid, &status, 0);
    return (0);
}



int format_volume(const char* volume) {
    time_t t;
    Volume* v = volume_for_path(volume);
    if (v == NULL) {
        printf("unknown volume \"%s\"\n", volume);
        return -1;
    }
    if (strcmp(v->fs_type, "ramdisk") == 0) {
        // you can't format the ramdisk.
        printf("can't format_volume \"%s\"", volume);
        return -1;
    }
    if (strcmp(v->mount_point, volume) != 0) {
        printf("can't give path \"%s\" to format_volume\n", volume);
        return -1;
    }

    if (ensure_path_unmounted(volume) != 0) {
        printf("format_volume failed to unmount \"%s\"\n", v->mount_point);
        return -1;
    }

    if (strcmp(v->fs_type, "yaffs2") == 0 || strcmp(v->fs_type, "mtd") == 0) {
        mtd_scan_partitions();
        const MtdPartition* partition = mtd_find_partition_by_name(v->device);
        if (partition == NULL) {
            printf("format_volume: no MTD partition \"%s\"\n", v->device);
            return -1;
        }

        MtdWriteContext *write = mtd_write_partition(partition);
        if (write == NULL) {
            printf("format_volume: can't open MTD \"%s\"\n", v->device);
            return -1;
        } else if (mtd_erase_blocks(write, -1) == (off_t) -1) {
            printf("format_volume: can't erase MTD \"%s\"\n", v->device);
            mtd_write_close(write);
            return -1;
        } else if (mtd_write_close(write)) {
            printf("format_volume: can't close MTD \"%s\"\n", v->device);
            return -1;
        }
        return 0;
    }


    if (strcmp(v->fs_type, "ext4") == 0) {
        t = time(&t); 
        printf("format_volume device(%s) len(%d) volume(%s) (%s)\n", v->device, v->length, volume, ctime(&t));
        int result = make_ext4fs(v->device, v->length, volume, NULL);
        if (result != 0) {
            printf("format_volume: make_extf4fs failed on %s\n", v->device);
            return -1;
        }
        t = time(&t); 
        printf("format_volume device(%s) len(%d) volume(%s) (%s) Done\n", v->device, v->length, volume, ctime(&t));
        return 0;
    }

    printf("format_volume: fs_type \"%s\" unsupported\n", v->fs_type);
    return -1;
}

/******************************************
 * path : 0  u-disk
 *        1  sd card
 *
 *****************************************/
void set_update_path(char path)
{
    if(path == 0){
        update_path = UDISK_ROOT;
    }else if(path == 1){
        update_path = SD_ROOT;
    }
    CUR_ROOT = update_path;
}


int update_by_file_name(const char *file)
{
    int i;
    int status = 0;
    int result = 0;
    char  filem[50];
    char  *tmp;
    if(update_path == NULL)
        return INSTALL_ERROR;
    strcpy(filem,update_path);
    strcat(filem, file);
    result = mkdir(update_path,S_IRUSR|S_IWUSR|S_IXUSR);
    printf("[qy]error: %s\n ", strerror(errno));
    printf("[qy]errno=%d\n",errno);
    if((result < 0)&&((errno != EEXIST))){
        printf("[qy]mkdir failed,return error.\r\n");
        return INSTALL_ERROR;
    }
    if(update_path != NULL){
        ensure_path_mounted(update_path);
    } else {
        return UPDATED_ERR_DISK_NOT_EXIST;
    }
    for (i = 0; i < num_volumes; ++i)
    {
        Volume* v = &device_volumes[i];
        if (v->imgname && strstr(filem, v->imgname)){
            status = update_partition_from_file(v, filem);
        }
    }
    return (status);
}
