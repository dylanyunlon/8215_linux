/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <iostream>
#include <cstdarg>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>
#include <syslog.h>

#include <sys/stat.h>
#include <sys/statfs.h>

#include "aee_internal.h"
#include "expdb.h"
#include "utils.h"

#ifdef ATC_OS_LINUX
#include "ATCMountClient.hpp"
#endif

static char gzfilename[MAX_PATH];
static u32 ipanic_iv = 0xaabbccdd;
static struct ipanic_header *header_buf = NULL;
static int expdb_dev = -1;

static long getDiskFreeSpace(char *filePath)
{
    struct statfs path_st;

    if (statfs(filePath, &path_st) < 0) {
        AEE_LOGE("Failed to get free size:%s, error is %d:%s", filePath, errno, strerror(errno));
        return 0;
    }
    AEE_LOGI("Free space is: %dM", int(path_st.f_bavail / 1024));
    return path_st.f_bavail / 1024; // unit: BLOCKSIZE * 1024
}

static void aed_ke_dal_report(FILE *fp)
{
    AE_DAL_DATA dal_show;
    char *pc_symbols = NULL;
    char symbol[MAX_PATH];
    ssize_t read_sz;
    size_t len = 0;

    int dal_fd = open(AE_EE_DEVICE_PATH, O_RDONLY);
    if (dal_fd == -1) {
        AEE_LOGD("open dev0 Failed, ignored");
        return ;
    }
    memset(dal_show.msg, 0x00, DAL_MSG_LEN);
    while( (read_sz = getline(&pc_symbols, &len, fp)) != -1) {
        if (read_sz < MAX_PATH) {
            /* memset is Required.  Otherwise, the ending of DAL is unreadabled. */
            memset(symbol, 0, MAX_PATH);
            strncpy(symbol, pc_symbols, read_sz );
            if (strlen(dal_show.msg) + strlen(symbol) >= DAL_MSG_LEN) {
                break;
            }
            sprintf(dal_show.msg, "%s%s.\n", dal_show.msg, symbol);
        }
    }
    if (pc_symbols) {
        free(pc_symbols);
    }

    if (strlen(dal_show.msg) > 0) {
        AE_DAL_SETCOLOR dal_color;
            dal_color.fgcolor = 0xFF00FF; // fg: purple
            dal_color.bgcolor = 0x00FF00; // bg: green
        dal_color.screencolor= 0xFF0000; // red

        if (ioctl(dal_fd, AEEIOCTL_DAL_SETCOLOR, &dal_color) < 0) {
                AEE_LOGD("setcolor ioctl failed(%s)\n", strerror(errno));
            }

        if (ioctl(dal_fd, AEEIOCTL_DAL_SHOW, &dal_show) < 0) {
                AEE_LOGD("ERROR: show ioctl failed(%s).\n", strerror(errno));
            }
        }
    if (dal_fd >= 0) {
        close(dal_fd);
    }
}

static void ipanic_block_scramble(char *buf, int buflen)
{
    int i;
    u32 *p = (u32 *)buf;
    for (i = 0; i < buflen; i += 4, p++) {
        *p = *p ^ ipanic_iv;
    }
}

static int ipanic_open(void)
{
    if (expdb_dev < 0) {
        expdb_dev = open(EXPDB_DEV, O_RDWR);
        if (expdb_dev < 0) {
        AEE_LOGE("Failed to open %s, error is %d:%s", EXPDB_DEV, errno, strerror(errno));
        return -1;
    }
    }
    return expdb_dev;
}

static void ipanic_erase(void)
{
    if (ipanic_open() < 0) {
        return ;
    }

    char *data = (char *)malloc(EXPDB_ALLOCATE);
    if (data) {
        memset(data, 0x00, EXPDB_ALLOCATE);
        lseek(expdb_dev, 0, SEEK_SET);
        write(expdb_dev, data, EXPDB_ALLOCATE);
        free(data);
        AEE_LOGI("EXPDB partition erased.");
    } else {
        AEE_LOGE("Failed to malloc mem for erase expdb.");
    }
}
static bool ipanic_valid()
{
    if (ipanic_open() < 0) {
        return false;
    }

    lseek(expdb_dev, 0, SEEK_SET);
    read(expdb_dev, header_buf, sizeof(struct ipanic_header));
    if (header_buf->magic != AEE_IPANIC_MAGIC || header_buf->version < 0x10) {
        return false;
    }

    AEE_LOGI("Expdb Header Info:\
            \n    header_buf->magic 0x%x,\n    header_buf->version %d,\n    header_buf->size %d,\n    header_buf->datas %d,\
            \n    header_buf->dhblk %d,\n    header_buf->blksize %d,\n    header_buf->partsize%d,\n    header_buf->bufsize %d,\
            \n    header_buf->buf %lld\n",
        header_buf->magic, header_buf->version, header_buf->size, header_buf->datas,
        header_buf->dhblk, header_buf->blksize, header_buf->partsize, header_buf->bufsize,
        header_buf->buf);

    return true;
}

static bool ipanic_read_header(char * path)
{
    char *buf = NULL;
    int expdb = -1;

    if (ipanic_open() < 0) {
        return false;
    }

    char expdbpath[MAX_PATH];
    buf = (char *)malloc(EXPDB_BIN);
    if (!buf) {
        return false;
    }
    lseek(expdb_dev, 0, SEEK_CUR); // file pointer after header info.
    read(expdb_dev, buf, EXPDB_BIN);
    ipanic_block_scramble(buf, EXPDB_BIN);
    sprintf(expdbpath, "%s/expdb", path);
    expdb = open(expdbpath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (expdb < 0) {
        AEE_LOGE("Open file %s failed!--%d:%s!\n", expdbpath, errno, strerror(errno));
        free(buf);
        return false;
    }
    write(expdb, buf, EXPDB_BIN);
    close(expdb);
    free(buf);

    if (header_buf && header_buf->dhblk) {
        struct ipanic_data_header *dheader = (struct ipanic_data_header*)malloc(sizeof(struct ipanic_data_header));
        for (int i = IPANIC_DT_HEADER + 1; i < IPANIC_DT_RESERVED31; i++) {
            struct ipanic_data_header *dh = &header_buf->data_hdr[i];
            if (dh->total == 0) {
                continue;
            }
            lseek(expdb_dev, 40 + i * header_buf->dhblk / 8, SEEK_SET);
            read(expdb_dev, dheader, sizeof(struct ipanic_data_header));
            if (dh->type != dheader->type || dh->offset != dheader->offset) {
                AEE_LOGI("Unmatched data header, %x[%x], %x[%x]", dh->type, dh->offset, dheader->type, dheader->offset);
                continue;
            }
            dh->used = dheader->used;
            dh->valid = dheader->valid;
        }
        free(dheader);
    }
    return true;
}

static bool ipanic_read_data(struct ipanic_data_header *dheader, char * path)
{
    char filepath[MAX_PATH];

    if (!dheader || dheader->valid != 1) {
        return true;
    }
    if (dheader->used > dheader->total) {
        AEE_LOGI("Type(%d) size used(%x) greater than total(%x)", dheader->type, dheader->used, dheader->total);
        return false;
    }

    if (ipanic_open() < 0) {
        return false;
    }

    char *data = (char *)malloc(dheader->total);
    if (!data) {
        return false;
    }
    lseek(expdb_dev, dheader->offset, SEEK_SET);
    read(expdb_dev, data, dheader->used);

    if (dheader->encrypt) {
        ipanic_block_scramble(data, dheader->used);
    }

    sprintf(filepath, "%s/%s", path, dheader->name);
    int file = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file < 0) {
        AEE_LOGE("Open file %s failed--%d:%s!\n", filepath, errno, strerror(errno));
        free(data);
        return false;
    }
    write(file, data, dheader->used);
    close(file);
    free(data);
    AEE_LOGI("ipanic_read_data() Tyep(%d) end!", dheader->type);

    if (strncmp((const char*)dheader->name, IPANIC_CURRENT_TSK_NAME, sizeof(IPANIC_CURRENT_TSK_NAME)) == 0) {
        FILE * fp = fopen(filepath, "r");
        if (fp != NULL) {
            aed_ke_dal_report(fp);
            fclose(fp);
        }
    }

    return true;
}

static bool remain_size_enough(const char * path) {
    struct statfs path_st;
    unsigned long long free_size;

    if (path && strlen(path) > 0) {
        if (statfs(path, &path_st) >= 0) {
            free_size = (path_st.f_bavail / 1024) * (path_st.f_bsize / 1024);
            AEE_LOGI("Free size of storage %s is : %lldM", path, free_size);
            if (free_size > MIN_STORAGE_SIZE) {
                return true;
            }
        } else {
            AEE_LOGE("Failed to stat storage folder:%s(%s).", path, strerror(errno));
        }
    }
    return false;
}

static char * storage_exist(void) {
    static char storagepath[MAX_PATH];
#ifdef CONFIG_ATC_NAND
    int retry_count = 5;
#else
    int retry_count = 1;
#endif
    memset(storagepath, 0x00, MAX_PATH);

#ifdef ATC_OS_LINUX
    ATCMountClient *mountclient = new ATCMountClient();
    std::vector<std::string> path;

    if (!mountclient) {
        AEE_LOGE("***Failed to get mount client!***");
        return NULL;
    }

    path.clear();
    path = mountclient->getUdiskMountedPaths();
    for (unsigned int i = 0; i < path.size(); i++) {
        AEE_LOGI("UDisk Mounted path[%d]: %s", i, path[i].c_str());
        if (remain_size_enough(path[i].c_str())) {
            strcpy(storagepath, path[i].c_str());
            break;
        }
    }

    if (!strlen(storagepath)) {
        path.clear();
        path = mountclient->getSdMountedPaths();
        for (unsigned int i = 0; i < path.size(); i++) {
            AEE_LOGI("SD Mounted path[%d]: %s", i, path[i].c_str());
            if (remain_size_enough(path[i].c_str())) {
                strcpy(storagepath, path[i].c_str());
                break;
            }
        }
    }
#endif

    while(retry_count--)
    {
        if (!strlen(storagepath)) {
            if (remain_size_enough(INT_SDCARD_PATH)) {
                strcpy(storagepath, INT_SDCARD_PATH);
                break;
            } else {
                if(retry_count <= 0)
                {
                    AEE_LOGE("****internal storage device insufficient space!*****");
                    return NULL;
                }
                else
                {
                    AEE_LOGE("****try to wait u pan mount!*****");
                    sleep(2);
                }
            }
        }
    }

    DIR * dir = opendir(storagepath);
    if (dir) {
        AEE_LOGI("************Storage:%s available ***********", storagepath);
        closedir(dir);
        return storagepath;
    }

    AEE_LOGE("****No storage device available!*****");
    return NULL;

}

// maybe name format is:  KE_xxx.tar.gz
// we can create 9999 file of KE_xxx.tar.gz max, overwrite while overflow.
static void getketargetname(char * path)
{
    int n = 0;

    DIR *dir = opendir(path);
    if (dir) {
        struct dirent *node = NULL;
        while ((node = readdir(dir)) != NULL) {
            if (strstr(node->d_name, "KE_") && strstr(node->d_name, ".tar.gz")) {
                n++;
            }
        }
        closedir(dir);
    }

    sprintf(gzfilename, "KE_%04d.tar.gz", n);
    AEE_LOGI("gz file is: %s.\n", gzfilename);
}

void *dump_ke(void *p)
{
    char *rootpath = NULL;
    char kepath[MAX_PATH];
    char tmppath[MAX_PATH];
    char cmd[MAX_PATH];

    header_buf = (struct ipanic_header*)malloc(sizeof(struct ipanic_header));
    if (!header_buf) {
        AEE_LOGE("Failed to malloc ipanic header...");
        goto MAIN_ERROR;
    }

    if (!ipanic_valid()) {
        AEE_LOGI("No exception info in expdb...");
        goto MAIN_ERROR;
    }

    // Check storage exist and set rootpath.
    rootpath = storage_exist();
    if (!rootpath) {
        goto MAIN_ERROR;
    }

    if (getDiskFreeSpace(rootpath) < 10) {
        AEE_LOGW("Free space lower than 10M...");
        goto MAIN_ERROR;
    }

    sprintf(kepath, "%s/%s", rootpath, AEE_KE_PATH);
    AEE_LOGI("ke dump path is: %s", kepath);
    if (!create_dir(kepath)) {
        AEE_LOGE("Failed to create folder for dump ke info.");
        goto MAIN_ERROR;
    }

    sprintf(tmppath, "%s/tmp", kepath);
    if (mkdir(tmppath, S_IRUSR | S_IWUSR) < 0) {
        if (errno == EEXIST) {
            // remove files in atclog folder.
            sprintf(cmd, "rm -rf %s/*", tmppath);
            cmd_execute(cmd);
        } else {
            AEE_LOGE("Failed to mkdir: %s, err is %d:%s", tmppath, errno, strerror(errno));
            goto MAIN_ERROR;
        }
    }

    AEE_LOGI("(0)Start dump System Version...\n");
    sprintf(cmd, "cat /proc/version > %s/SYS_VERSION_INFO", tmppath);
    if (!cmd_execute(cmd)) {
        AEE_LOGE("Failed to dump system version.\n");
        goto MAIN_ERROR;
    }

    AEE_LOGI("(1)Start dump Last Kmsg...\n");
    sprintf(cmd, "cat /proc/last_kmsg > %s/SYS_LAST_KMSG", tmppath);
    if (!cmd_execute(cmd)) {
        AEE_LOGE("Failed to dump last kmsg.\n");
        goto MAIN_ERROR;
    }

    AEE_LOGI("(2)Start dump Expdb...\n");
    if (!ipanic_read_header(tmppath)) {
        AEE_LOGI("Failed to read ipanic header...");
        goto MAIN_ERROR;
    }
    for (int i = IPANIC_DT_HEADER + 1; i < IPANIC_DT_RESERVED31; i++) {
        if (!ipanic_read_data(&header_buf->data_hdr[i], tmppath)) {
            AEE_LOGE("Failed to Read ipanic data %d", i);
            //ipanic_erase();
            goto MAIN_ERROR;
        }
    }

    AEE_LOGI("(3)Compress to .tar.gz...\n");
    getketargetname(kepath);
    #ifdef ATC_OS_LINUX
    sprintf(cmd, "cd %s && tar -czvf ../%s ./* && cd -", tmppath, gzfilename);
    #else
    sprintf(cmd, "cd %s && tar -cvf ../%s ./* && cd -", tmppath, gzfilename);
    #endif
    if (!cmd_execute(cmd)) {
        AEE_LOGE("Failed to compress atclog folder.\n");
        goto MAIN_ERROR;
    }
    AEE_LOGI("(4)Delete temporary folder.");
    sprintf(tmppath, "%s/%s", kepath, gzfilename);
    if (access(tmppath, R_OK) == 0) {
        sprintf(cmd, "rm -rf %s/tmp", kepath);
        if (!cmd_execute(cmd)) {
            AEE_LOGE("Failed to delete atclog folder.\n");
            goto MAIN_ERROR;
        }

        ipanic_erase();
    } else {
        AEE_LOGE("Failed to access %s, %d--%s\n", tmppath, errno, strerror(errno));
        goto MAIN_ERROR;
    }

    AEE_LOGI("Successful!");

MAIN_ERROR:
    if (header_buf) {
        free(header_buf);
    }

    if (expdb_dev >= 0) {
        close(expdb_dev);
    }

    AEE_LOGI("Dump KE End %s!", (char*)p);
    return NULL;
}

