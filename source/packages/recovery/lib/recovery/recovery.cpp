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


#define _LARGEFILE64_SOURCE     /* See feature_test_macros(7) */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>

#include "atc_upgrade_common.h"
#include "recovery.h"
#include "err_num.h"
#include "roots.h"
#include "bootloader.h"
#include "atc_update.h"
#include "atc_emptycore_upgrade.h"
#include "atc_safe_upgrade.h"

extern partitionread *newtblhead;

static partitionread *update_partiton_info = NULL;


void msdc_emmc_clear_write_protect(void)
{
	/*clear all write protect*/
		int fdwp,ret = 0;
		struct wp_cmd_arg arg = {0};
		arg.wp_action = WP_CLEAR_AND_SAVE;
		ret = ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
		if(ret){
			rec_err("clear and save all wp fail,ret:%d\n",ret);
			close(fdwp);
		}else {
			rec_info("clear and save all wp success\n");
	}
}

void msdc_emmc_restore_write_protect(void)
{
	int fdwp,ret = 0;
	struct wp_cmd_arg arg = {0};

	arg.wp_action = WP_RESTORE;
	ret=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
	if(ret){
		rec_err("restore all wp fail,ret:%d\n",ret);
		close(fdwp);
	}else {
		rec_info("restore all wp success\n");
	}

}

int update_ext4_partition_from_file(const char *devname,  const char *file, long long offset, int size)
{
    int fdev, fd,fdwp;
    size_t sizer, sizew, filelen;
    uchar *buffer = NULL;
    uint32_t chunk_cnt = 0;
    uint32_t block_size = 0;
    uchar *pfileBuffer = NULL;
    size_t memlen = DEF_CHUNK_SIZE;
    size_t datalen = 0;
    off64_t cur_offset;
    int is_auto_update = 0;
    int ret=-1;
    int ret1=-1;
    struct wp_cmd_arg arg = {0};

    sparse_header_t tSparseHeader;
    chunk_header_t  tChunkHeader;

    if (strcmp(devname, ATC_DEVNAME) == 0) {
/*
* auto update
*/
        is_auto_update = 1;
    }
    if(!is_auto_update){
        fdwp = open("/dev/misc-sd",O_RDONLY);
        if(fdwp < 0){
            printf("open failed\n");
            return -1;
        }
        printf("before update  dump\n");
        dumpwriteprotectregion(fdwp);
        /*clear all write protect*/
        arg.wp_action = WP_CLEAR_AND_SAVE;
        ret=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
        if(ret){
            printf("clear and save all wp fail,ret:%d\n",ret);
            close(fdwp);
        }else {
            printf("clear and save all  wp success\n");
        }
        printf("after clear  dump\n");
        dumpwriteprotectregion(fdwp);
    }
    printf("update_ext4_partition_from_file dev(%s) offset(0x%llx) file(%s)\n", devname, offset, file);
    fd = open(file, O_RDONLY, 0);
    if (fd < 0){
        return UPDATED_ERR_FILE_DEMAGED;
    }
    fdev = open(devname, O_RDWR|O_LARGEFILE);
    if (fdev < 0){
        close(fd);
        return UPDATED_ERR_PART_DAMAGED;
    }
    filelen = lseek(fd, 0, SEEK_END);

    buffer = (uchar *) malloc (memlen);
    if (!buffer){
        close(fd);
        close(fdev);
        return UPDATED_ERR_OUT_OF_MEMORY;
    }

    // Read file to buffer
    lseek(fd, 0, SEEK_SET);
    sizer = read(fd, &tSparseHeader, sizeof(sparse_header_t));

    block_size = tSparseHeader.blk_sz;
    chunk_cnt = tSparseHeader.total_chunks;

    filelen -= sizeof(sparse_header_t);
    while ((chunk_cnt > 0) && (filelen > 0))
    {
        sizer = read(fd, &tChunkHeader, CHUNK_HEADER_LEN);
        datalen = tChunkHeader.chunk_sz * block_size;

        if (CHUNK_TYPE_RAW == tChunkHeader.chunk_type){
            if (datalen > memlen){
                buffer = (uchar *)realloc((void *)buffer, datalen);
                if (!buffer){
                    close(fd);
                    close(fdev);
                    return (UPDATED_ERR_OUT_OF_MEMORY);
                }
                memlen = datalen;
            }
            pfileBuffer = buffer;

            sizer = read(fd, pfileBuffer, datalen);

            cur_offset = lseek64(fdev, offset, SEEK_SET);
            if (cur_offset != offset){
                printf ("update_ext4_partition_from_file lseek failed!\r\n");
                return (UPDATED_ERR_FILE_DEMAGED);
            }
            sizew = write(fdev, pfileBuffer, datalen);

            filelen -= tChunkHeader.total_sz;
            chunk_cnt--;
            pfileBuffer+= datalen;
            offset += datalen;
        }
        else if (CHUNK_TYPE_DONT_CARE == tChunkHeader.chunk_type){
            lseek(fd, tChunkHeader.total_sz - CHUNK_HEADER_LEN, SEEK_CUR);
            filelen -= tChunkHeader.total_sz;
            chunk_cnt--;
            offset += datalen;
        }else{
            printf ("mmc_write_ext4_image:tChunkHeader.chunk_type error\r\n");
            return (UPDATED_ERR_FILE_DEMAGED);
        }
    }

    close(fd);
    close(fdev);
    free(buffer);

    if (0 != chunk_cnt){
        printf ("mmc_write_ext4_image:chunk_cnt error\r\n");
        return (UPDATED_ERR_FILE_DEMAGED);
    }
    if(!is_auto_update){
        arg.wp_action = WP_RESTORE;
        ret1=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
        if(ret1){
            printf("restore all wp fail,ret:%d\n",ret1);
            close(fdwp);
        }else {
            printf("restore all wp success\n");
        }
        printf("after update dump\n");
        dumpwriteprotectregion(fdwp);
        close(fdwp);
    }
    printf("update_ext4_partition_from_file succeeded! memsize(0x%x)\n", memlen);
    return (UPDATED_SUCCESS);

}

int update_raw_partition_from_file(const char *devname,  const char *file, long long offset, int size)
{
    int fdev, fd,fdwp;
    size_t sizer, sizew, sizetotal;
    char *buffer = NULL;
    off64_t cur_offset;
    unsigned long long file_len;
    int is_auto_update = 0;
    int ret= -1;
    int ret1=-1;
    struct wp_cmd_arg arg = {0};

    if (strcmp(devname, ATC_DEVNAME) == 0) {
/*
* auto update
*/
        is_auto_update = 1;
    }
    if(!is_auto_update)
    {
        fdwp = open("/dev/misc-sd",O_RDONLY);
        if(fdwp < 0){
            printf("open failed\n");
            return -1;
        }
        dumpwriteprotectregion(fdwp);
        /*clear all write protect*/
        arg.wp_action = WP_CLEAR_AND_SAVE;
        ret=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
        if(ret){
            printf("clear and save all wp fail,ret:%d\n",ret);
            close(fdwp);
        }else {
            printf("clear and save all  wp success\n");
        }
        printf("after clear  dump\n");
        dumpwriteprotectregion(fdwp);
    }
    if(update_partiton_info == NULL){
        update_partiton_info = readpartitioninfofromflash();
    }

    ret = get_file_len(file, &file_len);
    if (ret < 0) {
        printf("ERROR: file_len get fail.\n");
        return UPDATED_ERR_FILE_DEMAGED;
    }
    printf("update_raw_partition_from_file dev(%s) offset(0x%llx) file(%s)\n", devname, offset, file);
    fd = open(file, O_RDONLY, 0);
    if (fd < 0){
        return (UPDATED_ERR_FILE_DEMAGED);
    }
    
    fdev = open(devname, O_RDWR|O_LARGEFILE);
    if (fdev < 0){
        close(fd);
        return (UPDATED_ERR_PART_DAMAGED);
    }
    buffer = (char *)malloc (FILE_RW_SIZE);
    if (!buffer){
        close(fd);
        close(fdev);
        return (UPDATED_ERR_OUT_OF_MEMORY);
    }
    cur_offset = lseek64(fdev, offset, SEEK_SET);
    if (cur_offset != offset){
        printf ("update_raw_partition_from_file lseek failed!\r\n");
        return (UPDATED_ERR_FILE_DEMAGED);
    }

    sizetotal = 0;
    while (sizer = read(fd, buffer, FILE_RW_SIZE))
    {
        sizew = write(fdev, buffer, sizer);
        sizetotal += sizew;
        if (sizew < sizer)
            break;
    }

    close(fd);
    close(fdev);
    free(buffer);

    if (sizetotal < file_len){
        printf("ERROR: update_raw_partition_from_file faild! the size of file(%s) is %d bytes, only %d bytes are write into emmc.\n",
        file, file_len, sizetotal);
        return UPDATED_ERR_FILE_DEMAGED;
    }

    if (update_partiton_info != NULL){
        updata_partition_len(update_partiton_info, file, file_len);
        if (writepartitioninfotoflash(update_partiton_info) < 0){
            printf("ERROR: update partition size failed \n");
        }
    }
    if(!is_auto_update){
        arg.wp_action = WP_RESTORE;
        ret1=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
        if(ret1){
            printf("restore all wp fail,ret:%d\n",ret1);
            close(fdwp);
        }else {
            printf("restore all wp success\n");
         }
        printf("after update dump\n");
        dumpwriteprotectregion(fdwp);
        close(fdwp);
    }
    printf("update_raw_partition_from_file succeeded!\n");
    return ((int)sizetotal);
}

static int erase_volume(const char *volume)
{
    ensure_path_unmounted(volume);
    return format_volume(volume);
}

static void finish_recovery(const char *send_intent)
{
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    set_bootloader_message(&boot);
    sleep(1);
    system("reboot");
}


bool is_file_exist(const char *file)
{
    int fd;
    if (ensure_path_mounted(file) != 0){
        return (0);
    }
    fd = open(file, O_RDONLY, 0);
    if (fd < 0){
        return (0);
    }
    close(fd);
    return (1);
}

int filecopy( char *srcdir, char *dstdir)
{
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[]= {"/bin/cp", "-rfp", (char *)NULL, (char *)NULL, (char *)NULL};
        argv[2] = srcdir;
        argv[3] = dstdir;
        execv(argv[0], argv);
        fprintf(stdout, "E:Can't run %s (%s)\n", argv[0], strerror(errno));
        _exit(-1);
    }
    int status;
    waitpid(pid, &status, 0);
    return (0);
}

int dircopy(char *srcdir, char *dstdir)
{
    char srcdirm[20];
    strcpy(srcdirm, srcdir);
    int len = strlen(srcdirm);
    if ('*' == srcdirm[len-1]){
        struct dirent *entry;
        srcdirm[len-1] = 0;
        DIR *dirptr = opendir(srcdirm);
        if(dirptr){
            while (entry = readdir(dirptr))
            {
                struct stat buf;
                char fullname[255];
                if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")){
                    continue;
                }
                strcpy(fullname, srcdirm);
                strcat(fullname, "/");
                strcat(fullname, entry->d_name);
                filecopy(fullname, dstdir);
            }
            closedir(dirptr);
        }
        return (0);
    }else{
        return (filecopy(srcdirm, dstdir));
    }
}

static int fd;
void recovery_init()
{
   // int ret,arg;

    load_volume_table();
    printf("[qy]recovery_init\n");

}


int factory_reset()
{
    int result = 0;
    int status = INSTALL_SUCCESS;
    if (erase_volume("/app"))
        status = INSTALL_ERROR;
    //printf("factory_reset-->erase status : %d\n", status);
    result = mkdir("/appbk",S_IRUSR|S_IWUSR|S_IXUSR);
    //printf("factory_reset->mkdir status1 : %d\n", result);

    if(result >= 0)
        status = INSTALL_SUCCESS;
    else{
        if ((-1 == result) && (errno == EEXIST))
            status = INSTALL_SUCCESS;
        else{
            status = INSTALL_ERROR;
            printf("Oh dear, something went wrong with mkdir()! %s\n", strerror(errno));
        }
    }

    //printf("factory_reset->mkdir status2 : %d\n", status);
    ensure_path_mounted("/app");
    ensure_path_mounted("/appbk");
    if (status == INSTALL_SUCCESS){
        dircopy("/appbk/*", "/app/");
        //printf("factory_reset->copy appbk partition done\n");
    }
    return status;
}

void reboot_system()
{
    rec_info("begin to reboot\n");
    system("reboot");

}

