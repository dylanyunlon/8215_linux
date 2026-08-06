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

#include <sys/mount.h>
#include <sys/ioctl.h>

#include "bootloader.h"
#include "atc_update.h"
#include "XMLFile.h"
#include "err_num.h"
#include "recovery.h"
#include "roots.h"
#include "atc_upgrade_common.h"


const char * devname = "/dev/mmcblk0";

#define XMLFileName "scatter.mmcboot.ext4.xml"

#define CONFIG_USRDATA_EXT4

/*const char *CUR_ROOT = "/ext_sdcard1/"; */
extern const char *CUR_ROOT;

partitionread *oldptbl = NULL;
partitionread *xmlptbl = NULL;
partitionread *newtblhead = NULL;
partitionread *newtblcur = NULL;
static bool _partition_changed = false;
static bool _required_updated_failed = false;
static int fdwp;


void dumpwriteprotectregion(int fdwp)
{
    struct wp_cmd_arg argdump = {0};
    int ret1;

    argdump.wp_action = WP_REGIONINFO_GET;
    argdump.wp_dump_info =(char *)malloc(MAX_DUMP_BUFF_SIZE);
    ret1=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&argdump);
    if(ret1){
        rec_err("dump wp fail,ret:%d\n",ret1);
    }else{
        rec_info("dump success ,result:%s\n",argdump.wp_dump_info);
    }
    free(argdump.wp_dump_info);

}

void dumppartitioninfo(partitionread *part)
{

    if (part){
        rec_info("Partition Info----------------------------------------------\r\n");
        rec_info("name : %s\r\n",  part->szPartName);
         rec_info("type : %s\r\n",  part->szType);
        rec_info("image: %s\r\n",  part->szImageFileName);
        rec_info("mount: %d\r\n",  part->u4Mount);

#ifdef NEW_PARTITION_DESIGN
        rec_info("flag : 0x%x\r\n", part->u4Flag);
#endif
        rec_info("last : %d\r\n",  part->u4LastPartition);
        rec_info("addr : 0x%llx\r\n", part->u8PartitionStartAddr);
        rec_info("size : 0x%llx\r\n", part->u8PartitionSize);
        rec_info("rsize: 0x%llx\r\n", part->u8RealDataSize);
        rec_info("----------------------------------------------------------\r\n");
    }
}

void dumpallpartitioninfo(partitionread *part)
{

    partitionread *pdump_table = part;
    if (part) {
        rec_err("dumo info part is not null\n");
    } else {
        rec_err("dumo info part is null\n");
    }
    while (pdump_table)
    {
        dumppartitioninfo(pdump_table);
        pdump_table = pdump_table->nextpartition;
    }
}
#ifdef NEW_PARTITION_DESIGN
unsigned int isEnable(enum Part_Attr attribute, unsigned int flag)
{
    unsigned int temp = 0;
    struct partitionflag *pflag = (struct partitionflag *)(&flag);
    switch(attribute)
    {
        case UPGRADE_ENABLE:
            temp = pflag->upgradable;
            break;
        case ERASE_ENABLE:
            temp = pflag->eraseable;
            break;
        case FASTBOOT_ENABLE:
            temp = pflag->fastbootable;
            break;
        case COPY_UPGRADE_EABLE:
            temp = pflag->copyupgradable;
            break;
        case WRITE_PROTECT_ENABLE:
            temp = pflag->write_protect;
            break;
        case MOUNT_ENABLE:
            temp = pflag->mountable;
            break;
        default:
            rec_err("ERR: Unknown Partition Attribute[%d]!!!\r\n", (unsigned int)(attribute));
            break;
    }
    if (temp)
        return ENABLE;
    else
        return DISABLE;
}
#endif
static int ptbl_update_one_partition(partitionread* part, const char *root)
{
    int status = 0;
    char  file[IMG_FULL_NAME_MAX];
    int ret = UPDATED_SUCCESS;

    if (!part || (part->u8PartitionStartAddr < 0x10000))
        return (ret);
#ifdef NEW_PARTITION_DESIGN
    if (DISABLE == part->u4Flag)
        return (ret);
#endif
    if (!strcmp(part->szType, "fat32")){
        if (part->u4Mount & 0x2){
            format_userdata_partition_fat32(part->u8PartitionStartAddr, part->u8PartitionSize);
        }
        if (part->u4Mount)
            part->u4Mount = 1;
        return (ret);
    }
    if (strlen(part->szImageFileName)){
        strcpy(file, root);
        strcat(file, part->szImageFileName);
        if (!is_file_exist(file)){
            return (UPDATED_SUCCESS);
        }
        if (!strcmp(part->szType, "raw")){
            ret = update_raw_partition_from_file(devname,  file, part->u8PartitionStartAddr , part->u8PartitionSize);
            if (ret >= 0){
                ret = (ret + 511) & (~(0x1FFUL)); //align 512
                part->u8RealDataSize = ret;
                part->u4LastPartition |= UPDATE_FLAG_DONE;
                ret = UPDATED_SUCCESS;
            }
        }
        else if (!strcmp(part->szType, "ext4")){
            if(strcmp("data4write.img.ext4",part->szImageFileName) == 0)
                format_volume("/data4write");
            if(strcmp("data.img.ext4",part->szImageFileName) == 0)
                format_volume("/data");
            ret = update_ext4_partition_from_file(devname,  file, part->u8PartitionStartAddr , part->u8PartitionSize);
            if (ret >= 0){
                part->u8RealDataSize = ret;
                part->u4LastPartition |= UPDATE_FLAG_DONE;
                ret = UPDATED_SUCCESS;
            }
        }
    }
    return (ret);
}


static int ptbl_update_all_auto(void)
{
    int ret = UPDATED_SUCCESS;

    newtblcur = newtblhead;
    while (newtblcur && (UPDATED_SUCCESS ==ret))
    {
        ret = ptbl_update_one_partition(newtblcur, CUR_ROOT);
        if (UPDATE_FLAG_REQUIRE & newtblcur->u4LastPartition ){
            if (UPDATE_FLAG_DONE & newtblcur->u4LastPartition){
                _partition_changed = true;
            }else{
                _required_updated_failed = true;
            }
        }
        newtblcur = newtblcur->nextpartition;
    }
#ifndef CONFIG_USRDATA_EXT4
    // Check if need to format user data or not.
    newtblcur = newtblhead;
    while (newtblcur && newtblcur->nextpartition)
        newtblcur = newtblcur->nextpartition;

    if ((UPDATE_FLAG_REQUIRE | UPDATE_FLAG_DONE) == newtblcur->u4LastPartition){
        // Last partition is changed and has been updated. Format user data partition.
        format_userdata_partition_fat32(newtblcur->u8PartitionStartAddr + newtblcur->u8PartitionSize, 0);
    }
#endif
    return (ret);
}

static int check_ptbl_completeness(partitionread * ptbl)
{
    int ret = UPDATED_SUCCESS;
    unsigned long long  u8Addr = 0;
    partitionread * ptmp = ptbl;
    while (ptbl)
    {
        if (ptbl->u8PartitionStartAddr < u8Addr)
            return (UPDATED_ERR_TABLE_BAD);
        u8Addr += ptbl->u8PartitionSize;
        ptbl = ptbl->nextpartition;
    }
    return (UPDATED_SUCCESS);
}


static int check_all_image_files_exist_for_update(partitionread * ptbl)
{
    int ret = UPDATED_SUCCESS;
    while (ptbl)
    {
        if (strlen(ptbl->szImageFileName) && (UPDATE_FLAG_REQUIRE == ptbl->u4LastPartition)){
            char  file[IMG_FULL_NAME_MAX];
            strcpy(file, CUR_ROOT);
            strcat(file, ptbl->szImageFileName);
            if (!is_file_exist(file)){
                printf("Required file %s \r\n", file);
                dumppartitioninfo(ptbl);
                ret = UPDATED_ERR_FILE_NOT_EXIST;
            }
        }
        ptbl = ptbl->nextpartition;
    }
    return (ret);
}
#ifdef NEW_PARTITION_DESIGN
static partitionread *search_partion_by_name(partitionread *ptbl,const char*name)
{
    if(ptbl==NULL||name==NULL){
        printf("Error:search_partion_by_name:ptbl or name is NULL!\n");
    }
    while(ptbl){
        if(strcmp(ptbl->szPartName,name)==0){
            printf("search_partion_by_name: found %s.\n");
            break;
        }else
            ptbl=ptbl->nextpartition;
        }
    return ptbl;
}

static int check_update_completeness(void)
{
    partitionread *ptblnewpre = newtblhead;
    partitionread *ptbloldtmp = oldptbl;
    partitionread *ptblxmltmp = xmlptbl;
    partitionread *ptbl_find = NULL;
    int ret = UPDATED_SUCCESS;
    int partition_changed = 0;

    ret = check_ptbl_completeness(xmlptbl);
    if (UPDATED_SUCCESS != ret){
        rec_err("xml partition table is wrong.\r\n");
        dumpallpartitioninfo(xmlptbl);
        return ret;
    }
    ret = check_ptbl_completeness(oldptbl);
    if (UPDATED_SUCCESS != ret){
        rec_err("old partition table is wrong.\r\n");
        dumpallpartitioninfo(oldptbl);
        return ret;
    }
    while (ptblxmltmp && ptbloldtmp)
    {
        //pick up one table entry from old table, then search in xml table.
        ptbl_find = search_partion_by_name(xmlptbl, ptbloldtmp->szPartName);
        if ((ptbl_find != NULL) && (ptbl_find->u8PartitionStartAddr == ptbloldtmp->u8PartitionStartAddr) &&
                       (ptbl_find->u8PartitionSize == ptbloldtmp->u8PartitionSize)){
            //this partition [start, end] is NOT change, copy xml partition as new one.
            partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
            if (pinfo == NULL){
                rec_err("Error: check_update_completeness: No memory-1!\n");
                return UPDATED_ERR_OUT_OF_MEMORY;
            }
            memcpy(pinfo, ptbl_find, sizeof(partitionread));
            pinfo->nextpartition = NULL;
            pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
            if (newtblhead == NULL)
                newtblhead = pinfo;
            else
                ptblnewpre->nextpartition = pinfo;
            ptblnewpre = pinfo;
            ptbloldtmp = ptbloldtmp->nextpartition;
            ptblxmltmp = ptbl_find->nextpartition;
        }else if (ptbl_find == NULL){
            //this partition can't be found in xml table, copy old partition as new one.
            partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
            if (pinfo == NULL){
                printf("Error: check_update_completeness: No memory-2!\n");
                return UPDATED_ERR_OUT_OF_MEMORY;
            }
            memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
            pinfo->nextpartition = NULL;
            pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
            if (newtblhead == NULL)
                newtblhead = pinfo;
            else
                ptblnewpre->nextpartition = pinfo;
            ptblnewpre = pinfo;
            ptbloldtmp = ptbloldtmp->nextpartition;
        }else{
/*
* if run here, it means partion [start end] changed.
*
* !!!important!!!
* merging xml and old partition is based the below assumption.
* if some xml partition changed, from this partion start, all the rest partitions should be provided in xml...
*
*/
            partition_changed = 1;
            ptbloldtmp = NULL;
            ptblxmltmp = NULL;
            while (ptbl_find)
            {
                partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
                if (pinfo == NULL){
                    printf("Error: check_update_completeness: No memory-3!\n");
                    return UPDATED_ERR_OUT_OF_MEMORY;
                }
                 memcpy(pinfo, ptbl_find, sizeof(partitionread));
                 pinfo->nextpartition = NULL;
                 pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;

                 if (newtblhead == NULL)
                     newtblhead = pinfo;
                 else
                     ptblnewpre->nextpartition = pinfo;

                ptblnewpre = pinfo;
                ptbl_find = ptbl_find->nextpartition;
           }
        }
    }
    if (partition_changed == 0) {
        while (ptblxmltmp)
        {
            partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
            if (pinfo == NULL){
                printf("Error: check_update_completeness: No memory-4!\n");
                return UPDATED_ERR_OUT_OF_MEMORY;
            }
            memcpy(pinfo, ptblxmltmp, sizeof(partitionread));
            pinfo->nextpartition = NULL;
            pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
            if (newtblhead == NULL)
                newtblhead = pinfo;
            else
                ptblnewpre->nextpartition = pinfo;
            ptblnewpre = pinfo;
            ptblxmltmp = ptblxmltmp->nextpartition;
        }
        while (ptbloldtmp)
        {
            partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
            if (pinfo == NULL){
                printf("Error: check_update_completeness: No memory-5!\n");
                return UPDATED_ERR_OUT_OF_MEMORY;
            }
            memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
            pinfo->nextpartition = NULL;
            pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
            if (newtblhead == NULL)
                newtblhead = pinfo;
            else
                ptblnewpre->nextpartition = pinfo;
            ptblnewpre = pinfo;
            ptbloldtmp = ptbloldtmp->nextpartition;              
        }
    }
                
    ret = check_all_image_files_exist_for_update(newtblhead);
    if (UPDATED_SUCCESS != ret){
        printf("Not all required image files exist!\r\n");
    }
    printf("check_update_completeness return %d.\r\n", ret);
    return ret;
}

#else
static int check_update_completeness(void)
{
    partitionread * ptblnewtmp = newtblhead;
    partitionread * ptblnewpre = newtblhead;
    partitionread * ptbloldtmp = oldptbl;
    int ret = UPDATED_SUCCESS;
    ret = check_ptbl_completeness(newtblhead);
    if (UPDATED_SUCCESS != ret){
        printf("New partition table is wrong.\r\n");
        dumpallpartitioninfo(newtblhead);
        return (ret);
    }
    ret = check_ptbl_completeness(oldptbl);

    if (UPDATED_SUCCESS != ret){
        printf("Old partition table is wrong.\r\n");
        dumpallpartitioninfo(oldptbl);
    }
    
    while (ptblnewtmp && ptbloldtmp)
    {
        if ((ptblnewtmp->u8PartitionStartAddr == ptbloldtmp->u8PartitionStartAddr) &&
            (ptblnewtmp->u8PartitionSize == ptbloldtmp->u8PartitionSize)){
            // This partitions are same in old and new table.  
            ptblnewtmp->u4LastPartition = 0;
            ptblnewpre = ptblnewtmp;
            ptblnewtmp = ptblnewtmp->nextpartition;
            ptbloldtmp = ptbloldtmp->nextpartition;
        }else if (ptblnewtmp->u8PartitionStartAddr >= 
                 (ptbloldtmp->u8PartitionStartAddr + ptbloldtmp->u8PartitionSize)){
            // The old partition is in front of new partition. Add it to new partition table
            partitionread * pinfo = (partitionread *) malloc (sizeof(partitionread));
            if (!pinfo){
                printf("No memory!.\n");
                return (UPDATED_ERR_OUT_OF_MEMORY);
            }
            memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
            pinfo->nextpartition = ptblnewtmp;
            pinfo->u4LastPartition = 0;
            if (ptblnewpre  == newtblhead)
                newtblhead = pinfo;
            else
                ptblnewpre->nextpartition = pinfo;
            ptblnewpre = pinfo;
            ptbloldtmp = ptbloldtmp->nextpartition;
        }else if (ptbloldtmp->u8PartitionStartAddr >= 
                 (ptblnewtmp->u8PartitionStartAddr + ptblnewtmp->u8PartitionSize)){
            // The old partition is in back of new partition.  Current partition in new table is new partition.
            ptblnewtmp->u4LastPartition = UPDATE_FLAG_REQUIRE;
            ptblnewpre = ptblnewtmp;
            ptblnewtmp = ptblnewtmp->nextpartition;
        }else{
            // The partition of new table is overlap with the partition of old table
            unsigned long long  u8NextAddr;
            ptblnewtmp->u4LastPartition = UPDATE_FLAG_REQUIRE;

            u8NextAddr = ptblnewtmp->u8PartitionStartAddr + ptblnewtmp->u8PartitionSize;
            if (u8NextAddr < (ptbloldtmp->u8PartitionStartAddr + ptbloldtmp->u8PartitionSize)){
                u8NextAddr = ptbloldtmp->u8PartitionStartAddr + ptbloldtmp->u8PartitionSize;
            }
            ptblnewpre = ptblnewtmp;
            ptblnewtmp = ptblnewtmp->nextpartition;
            
            ptbloldtmp = ptbloldtmp->nextpartition;
            while (ptblnewtmp && (ptblnewtmp->u8PartitionStartAddr < u8NextAddr))
            {
                ptblnewpre = ptblnewtmp;
                ptblnewtmp->u4LastPartition = UPDATE_FLAG_REQUIRE;
                ptblnewtmp = ptblnewtmp->nextpartition;
            }

            while (ptbloldtmp && (ptbloldtmp->u8PartitionStartAddr < u8NextAddr))
            {
                ptbloldtmp = ptbloldtmp->nextpartition;
            }
            
        }
        if (!ptblnewtmp){
            while(ptbloldtmp)
            {
                // No partition in new table. Add partition of old table to it.
                partitionread * pinfo = (partitionread *) malloc (sizeof(partitionread));
                if (!pinfo){
                    printf("No memory!.\n");
                    return (UPDATED_ERR_OUT_OF_MEMORY);
                }
                memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
                pinfo->nextpartition = NULL;
                pinfo->u4LastPartition = 0;
                ptblnewpre->nextpartition = pinfo;
                ptblnewpre = pinfo;
                ptbloldtmp = ptbloldtmp->nextpartition;
            }
        }

    }

    ret = check_all_image_files_exist_for_update(newtblhead);
    if (UPDATED_SUCCESS != ret){
        printf("Not all required image files exist!.\r\n");
    }
    printf("check_update_completeness return %d.\r\n", ret);
    return (ret);
    
}
#endif

void freenewtblmemory(void)
{
    while (newtblhead)
    {
        newtblcur = newtblhead;
        newtblhead = newtblhead->nextpartition;
        free(newtblcur);
    }
    newtblcur = NULL;

    return;
}

void deleteandfree_userdata_partition()
{
    partitionread *pre = newtblhead;
    newtblcur = newtblhead;
    while (newtblcur && newtblcur->nextpartition)
    {
        pre = newtblcur;
        newtblcur = newtblcur->nextpartition;
    }
    
    if (newtblcur ){
        if (!strcmp(newtblcur->szType, "fat32")){
            // found user data partition. 
            // Remove it from partition table.
            pre->nextpartition = NULL;
            // Free it.
            free(newtblcur);
        }
    }
    return;
}



int writepartitioninfotoflash(partitionread *ptbl)
{

    char  *buf,*p;
    int fdev;
    partitionhead parthead;
    int n = 0,err = 0;;
    partitionread *ptmppart;
    unsigned long u4PartionAddress = ATC_PART_TBL_ADDR - PTBL_BLOCK_SIZE;

    if (!ptbl){
        printf("No partition table to be written.\n");
        return (-1);
    }
        

    ptmppart = ptbl;
    while (ptmppart)
    {
        n ++;
        ptmppart = ptmppart->nextpartition;
    }
    
    parthead.blockcnt = (n * sizeof(partitionread) + (PTBL_BLOCK_SIZE -1)) / PTBL_BLOCK_SIZE;
    parthead.u4Signature = ATC_PTBL_SIGN;
    parthead.u4Version = ATC_PARTITION_VER;
    parthead.nextpartition = NULL;
    buf = (char *)malloc(parthead.blockcnt * PTBL_BLOCK_SIZE);
    memset(buf,0,parthead.blockcnt * PTBL_BLOCK_SIZE);

    memcpy(buf, &parthead,sizeof(parthead));

    fdev = open(devname, O_RDWR);
    if (fdev < 0) {
        printf("Open block device(%s) failed.\n", devname);
        return (-1);
    }

    lseek(fdev, u4PartionAddress, SEEK_SET);

    n =  write(fdev, buf, PTBL_BLOCK_SIZE);

    if (!n){
        printf("writepartioninfo write failed fail result=%d\r\n",n);
        close(fdev);
        return -1;
    }
    memset(buf,0, parthead.blockcnt * PTBL_BLOCK_SIZE);
    ptmppart= ptbl;
    p = buf;

    printf("writepartitioninfotoflash parthead.blockcnt=%d\r\n", parthead.blockcnt);
    while(ptmppart)
    {
//printf("<writepartitioninfotoflash> szPartName:%s,nextpartition:0x%X \r\n",pcurpart->szPartName,pcurpart->nextpartition);
        if (ptmppart->nextpartition)
            ptmppart->u4LastPartition = 0;
        else
            ptmppart->u4LastPartition = 1;
        memcpy(p,ptmppart,sizeof(partitionread));
        p += sizeof(partitionread);
        ptmppart= ptmppart->nextpartition;
   }
    u4PartionAddress -= parthead.blockcnt * PTBL_BLOCK_SIZE;
    lseek(fdev, u4PartionAddress, SEEK_SET);
    n =  write(fdev, buf, parthead.blockcnt * PTBL_BLOCK_SIZE);
    close(fdev);
    if ((parthead.blockcnt * PTBL_BLOCK_SIZE) != n){
        printf("writepartitioninfotoflash write failed. write size(%d) real size(%d)\r\n", 
            parthead.blockcnt * PTBL_BLOCK_SIZE, n);
        return (-1);
    }

 
    printf("---------------writepartitioninfotoflash succeed--------------\r\n");
    return 0;
}

void updata_partition_len(partitionread *ptbl ,const char *file ,unsigned long long len)
{
    partitionread * ptbl_search = ptbl;
    len = (len + 511) & (~(0x1FFULL));

    while(ptbl_search)
    {
        if (ptbl_search->szImageFileName && ptbl_search->szImageFileName[0] 
        && (strstr(file, ptbl_search->szImageFileName) != NULL)){
            ptbl_search->u8RealDataSize = len;
            break;
        }
        ptbl_search= ptbl_search->nextpartition;
    }
}

partitionread * readpartitioninfofromflash(void)
{

    int fdev = 0;
    int n;
    int err = 0;
    char  buf[PTBL_BLOCK_SIZE];
    char *bufpartinfo;
    partitionhead *pparthead;
    partitionread *ppartread,*pprepartition,*pcurpartition;
    unsigned long u4PartionAddress = ATC_PART_TBL_ADDR - PTBL_BLOCK_SIZE;

    unsigned long blknum;

    printf("---------------readpartitioninfofromflash start addr:0x%X-----------------\r\n",u4PartionAddress);

    fdev = open(devname, O_RDWR);
    if (fdev < 0) {
        printf("Open block device(%s) failed.\n", devname);
        return (NULL);
    }
    lseek(fdev, u4PartionAddress, SEEK_SET);

    n =  read(fdev, buf, PTBL_BLOCK_SIZE);

    if (!n){
        printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
        close(fdev);
        return NULL;
    }
    printf("readpartitioninfofromflash block_read success  result=%d\r\n",n);
    pparthead = (partitionhead *)buf;

    printf("readpartitioninfofromflash block_read  blockcnt=%d\r\n",pparthead->blockcnt);
    bufpartinfo =(char *) malloc(pparthead->blockcnt*PTBL_BLOCK_SIZE);
    u4PartionAddress -= pparthead->blockcnt*PTBL_BLOCK_SIZE;

    lseek(fdev, u4PartionAddress, SEEK_SET);

    n =  read(fdev, bufpartinfo, pparthead->blockcnt * PTBL_BLOCK_SIZE);
    if (!n){
        printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
        close(fdev);
        return (NULL);
    }

    ppartread = (partitionread *)bufpartinfo;
    pcurpartition = ppartread;
    pprepartition = pcurpartition;

    close(fdev);
    while(pcurpartition != NULL)
    {
        //printf("readpartitioninfofromflash partname:%s\r\n",pcurpartition->szPartName);
        //printf("readpartitioninfofromflash szType:%s\r\n",pcurpartition->szType);
        //printf("readpartitioninfofromflash u4Mount:%d\r\n",pcurpartition->u4Mount);
        //printf("readpartitioninfofromflash nextpartition:0x%X\r\n",pcurpartition->nextpartition);
        if (pcurpartition->u4LastPartition == 1){
            printf("readpartitioninfofromflash this is last partition\r\n");
            pcurpartition->nextpartition = NULL;
            break;
        }else{
            //printf("readpartitioninfofromflash part before addree:0x%X,partitionread,size=%d\r\n",pcurpartition,sizeof(partitionread));
            pcurpartition = pcurpartition + 1;
            //printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
            pprepartition->nextpartition = pcurpartition;
            pprepartition = pcurpartition;
        }
    }


    printf("---------------readpartitioninfofromflash end-------------------\r\n");

    return ppartread;

}


static bool GetPartition(CXMLElement *pRoot)
{
    CXMLElement* pElement = NULL;
    pElement = pRoot->GetCurrentChild();
    partitionread * pinfo;
    pinfo = (partitionread *) malloc (sizeof(partitionread));
    //printf("GetPartition \r\n");
    if (!pinfo){
        printf("No memory!.\n");
        return (false);
    }

    memset(pinfo, 0, sizeof(partitionread));

    while(NULL != pElement)
    {
        if((XET_ATTRIBUTE == pElement->GetElementType())){
            if(!strcmp("name",pElement->GetElementName())){
            // save partition name

                strcpy(pinfo->szPartName, pElement->GetValue());
            }else if(!strcmp("type",pElement->GetElementName())){
            // Save partion type
                strcpy(pinfo->szType, pElement->GetValue());

            }else if(!strcmp("mount",pElement->GetElementName())){

                // Translate mount info
                pinfo->u4Mount= strtoll(pElement->GetValue(), NULL, 0);
            }else if(!strcmp("startaddress",pElement->GetElementName())){

                // Translate start address
                pinfo->u8PartitionStartAddr = strtoll(pElement->GetValue(), NULL, 0);

            }else if(!strcmp("size",pElement->GetElementName())){

                // Translate partition size
                pinfo->u8PartitionSize = strtoll(pElement->GetValue(), NULL, 0);
            }else if(!strcmp("imagename",pElement->GetElementName())){

                if (pElement->GetValue()) {
                    strcpy(pinfo->szImageFileName, pElement->GetValue());
                }
            }
#ifdef NEW_PARTITION_DESIGN
            else if(!strcmp("flag",pElement->GetElementName())){

                if (pElement->GetValue()) {
                    pinfo->u4Flag = (unsigned int)strtoll(pElement->GetValue(), NULL, 0);
                }
            }
#endif
        }
        pElement = pRoot->GetNextChild();
    }
#if 0/*remain and after  test success will drop*/
    if(!xmlptbl){
        xmlptbl = pinfo;
    }else{
        newtblcur->nextpartition = pinfo;
    }
    newtblcur = pinfo;
    return true;
#endif
	if (!newtblcur) {
		newtblhead = pinfo;
		xmlptbl = pinfo;
	} else {
		newtblcur->nextpartition = pinfo;
	}
	newtblcur = pinfo;

	return true;

}

static bool ParsePartitionInfo(CXMLElement *pInfo)
{

	CXMLElement *pElement = pInfo->GetCurrentChild();
//    printf("ParsePartitionInfo \n");
	while(NULL != pElement)
	{
//		printf("PP: name=%s ; value=%s; type=%d \r\n", pElement->GetElementName(), pElement->GetValue(), pElement->GetElementType());
		if(XET_TAG == pElement->GetElementType() && !strcmp("partition",pElement->GetElementName()))
		{
			GetPartition(pElement);
		}

		pElement = pInfo->GetNextChild();
	}

	return true;

}

static bool GetEnvInfo(CXMLElement *pRoot)
{
	CXMLElement* pElement = NULL;
	pElement = pRoot->GetCurrentChild();
    printf("GetEnvInfo \r\n");


	while(NULL != pElement)
	{

		printf("GE: name=%s ; value=%s; type=%d \r\n", pElement->GetElementName(), pElement->GetValue(), pElement->GetElementType());
		if((XET_ATTRIBUTE == pElement->GetElementType()))
		{
		}

		pElement = pRoot->GetNextChild();
	}

	return true;
}


static bool ParseEnvInfo(CXMLElement *pInfo)
{
 
	CXMLElement* pElement = pInfo->GetCurrentChild();
    printf("ParseEnvInfo \r\n");
    
	while(NULL != pElement)
	{

		if((XET_ATTRIBUTE == pElement->GetElementType()))// && !strcmp("partitionsize", pElement->GetElementName()))
		{
		    if (!strcmp("partitionsize", pElement->GetElementName())){
				printf("partitionsize is %s\r\n",pElement->GetValue());
		    }
			else if(!strcmp("startaddress", pElement->GetElementName())){
				printf("startaddress is %s\r\n",pElement->GetValue());
			    
			}		    
		}
		else if(XET_TAG == pElement->GetElementType() && !strcmp("item",pElement->GetElementName()))
		{
			printf("\r\n");
			GetEnvInfo(pElement);

		}

		pElement = pInfo->GetNextChild();
	}

	return true;

}


int read_partition_table_from_file(char *file)
{
     CXMLFile xmlFile;
     CXMLElement* root;
     CXMLElement* pElement = NULL;

     // search scatter file (.xml)
     BOOL bRet = xmlFile.LoadFromFile(file);
     if(bRet)
     {
         root = xmlFile.GetRoot();
         pElement = root->GetCurrentChild();

         while(NULL != pElement)
         {
//             printf("name=%s ; value=%s; type=%d \r\n", pElement->GetElementName(), pElement->GetValue(), pElement->GetElementType());
             
             if((XET_TAG == pElement->GetElementType())&& !strcmp("FLASH",pElement->GetElementName())){

                 ParsePartitionInfo(pElement);

             }else if((XET_TAG == pElement->GetElementType())&& !strcmp("NFLASH",pElement->GetElementName())){

                 ParsePartitionInfo(pElement);
             }else if (XET_TAG == pElement->GetElementType() && !strcmp("ENV",pElement->GetElementName())){

                 ParseEnvInfo(pElement);
             }
             pElement = root->GetNextChild();
         }
    }
//    dumppartitioninfo(newtblhead);
    
    return (0);
}

#ifdef NEW_PARTITION_DESIGN
static int update_region_writeprotect(void)
{
    struct wp_cmd_arg arg = {0};
    unsigned long long wp_start_sect = 0;
    unsigned long long wp_end_sect = 0;
    int find_wp_part = 0;
    int ret;

    newtblcur = newtblhead;
    printf("[qy]update_region_writeprotect");
    while (newtblcur) {
        if(ENABLE == isEnable(WRITE_PROTECT_ENABLE,newtblcur->u4Flag)) {
            if (find_wp_part == 0) {
                wp_start_sect = newtblcur->u8PartitionStartAddr/ PTBL_BLOCK_SIZE;
                wp_end_sect = (wp_start_sect*PTBL_BLOCK_SIZE + newtblcur->u8PartitionSize) / PTBL_BLOCK_SIZE;
                find_wp_part = 1;
            } 
            else
            {
                printf("[qy]continues region has been wirte protect\r\n");
            } 
        }
        else
        {
            if(find_wp_part)
            {
                arg.wp_action = WP_REGION_ENABLE;
                arg.wpg_size_of_xml=16;//MB
                arg.sect_start = wp_start_sect;
                /*because of alligment ,the last partion of continuous region is not continuous address */
                arg.sect_end = newtblcur->u8PartitionStartAddr/PTBL_BLOCK_SIZE;
                printf("<1>enable write protect on sect 0x%x~0x%x\n",arg.sect_start*PTBL_BLOCK_SIZE,arg.sect_end*PTBL_BLOCK_SIZE);
                ret = ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
                if(ret) {
                    printf("set wp fail\n");
                    return -1; 
                } 
                else 
                {
                    printf("set wp success\n");
                }

                printf("[qy]4debug\n");
                wp_start_sect = 0;
                wp_end_sect = 0;
                find_wp_part = 0;
            }
        }
        newtblcur = newtblcur->nextpartition;
    }

    if (find_wp_part)
    {
        //all parts are protect.
        arg.wp_action = WP_REGION_ENABLE;
        arg.wpg_size_of_xml=16;//MB
        arg.sect_start = wp_start_sect;
        /*
             * end sect aligns to 16M/512=0x8000
            */
        arg.sect_end = ((wp_end_sect + 0x8000) & (~0x7FFFU));

        printf("<2>enable write protect on sect 0x%x~0x%x\n",arg.sect_start*PTBL_BLOCK_SIZE,arg.sect_end*PTBL_BLOCK_SIZE);
        ret = ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
        if(ret)
        {
            printf("set wp fail\n");
            return -1; 
        }
        else
        {
            printf("set wp success\n");
        }	
	}

	return 0;
}
#endif
int update_image_for_auto()
{
    char xmlname[80];
    int ret = -1;
    unsigned long long emmc_size;
    int result = 0;
    partitionread *ptable = NULL;
    int ret1=-1;
    struct wp_cmd_arg arg = {0};

    if(CUR_ROOT == NULL){
        printf("Can't find the external stroage.\r\n");
        return (UPDATED_ERR_DISK_NOT_EXIST);
    }
#ifdef NEW_PARTITION_DESIGN
	//add by qiyun for wirte protect begin
	fdwp = open("/dev/misc-sd",O_RDONLY);
	if(fdwp < 0){
        printf("open failed\n");
		return -1;
	}
	printf("before upgrade auto dump info:\n");
	dumpwriteprotectregion(fdwp);
	/*clear all write protect*/
	arg.wp_action = WP_ALL_DISABLE;
	arg.partition_name = NULL;
	ret1=ioctl(fdwp,MSDC_EMMC_WRITE_PROTECT,&arg);
	if(ret1)
	{
	    printf("clear all wp fail,ret1:%d\n",ret1);
	    close(fdwp);
	}
	else 
	{
	    printf("clear all wp success");
	}
    //add by qiyun for write protect end
#endif
    strcpy(xmlname, CUR_ROOT);
    strcat(xmlname, XMLFileName);
    _partition_changed = false;
    _required_updated_failed = false;
    result = mkdir(CUR_ROOT,S_IRUSR|S_IWUSR|S_IXUSR);
    if((result < 0)&&((errno != EEXIST)))
    {
        printf("[qy]mkdir failed,return error\r\n");
        return INSTALL_ERROR;
    }
    ret = ensure_path_mounted(CUR_ROOT);
    printf("[qy]6ensure_path_mounted,ret=%d \n",ret);
    if ( 0 != ret)
    {
        printf("%s can't be accessed.\r\n", xmlname);
        return (UPDATED_ERR_DISK_NOT_EXIST);
    }
    if (!is_file_exist(xmlname))
    {
        printf("%s doesn't exist.\r\n", xmlname);
        return (UPDATED_ERR_FILE_NOT_EXIST);
    }
	printf("qiyundebug2\n");
    if (read_partition_table_from_file(xmlname) != 0)
    {
        printf("%s is incorrect.\r\n", xmlname);
        return (UPDATED_ERR_TABLE_BAD);
    }
#ifdef CONFIG_USRDATA_EXT4
    ret = get_emmc_total_size(&emmc_size);
    if (ret < 0) {
		printf("ERROR:get emmc total size failed.\n");
		return -8;//emmc size error
    }
	/*
	* pick up the last partition.
	*/
	//ptable = newtblhead;
	ptable = xmlptbl;
	while(ptable && ptable->nextpartition)
		ptable = ptable->nextpartition;
	
	/*the last partition shoule be usrdata with ext4 type*/
	if ((strcmp(ptable->szPartName, "usrdata") == 0)&&(strcmp(ptable->szType, "ext4") == 0))
	{
	     if(emmc_size < ptable->u8PartitionStartAddr)
	     {
             printf("ERROR:emmc_size less than userdate.startaddress\n");
	     }
	     else if(emmc_size < ptable->u8PartitionStartAddr+ptable->u8PartitionSize)
	     {
	         printf("ERROR:emmc_size less than userdate.startaddress+usrdate.size\n");
	     }
	     else if(emmc_size < ptable->u8PartitionStartAddr+(50+2)*512)
	     {
	       	 printf("ERROR:emmc_size less than userdate.startaddress+(50+2)*512)\n");
	      }
	      else
	      {
	         ptable->u8PartitionSize = emmc_size - ptable->u8PartitionStartAddr - (50+2)*512;
	      }
	}
        
#endif
    printf("xml partitions table information:\r\n");
    dumpallpartitioninfo(xmlptbl);

    oldptbl = readpartitioninfofromflash();

    printf("Old partitions table information:\r\n");
    dumpallpartitioninfo(oldptbl);

    ret = check_update_completeness();
    printf("after compare ,New partitions table information:\r\n");
    dumpallpartitioninfo(newtblhead);
    if (UPDATED_SUCCESS == ret)
    {
	    ret = ptbl_update_all_auto();
		//printf("[qy] table information begin:\r\n");
	    //dumpallpartitioninfo(newtblhead);
		writepartitioninfotoflash(newtblhead);	 
		printf("[qy]after write emmc flash  partitions table information:\r\n");
	    dumpallpartitioninfo(newtblhead);

#ifdef NEW_PARTITION_DESIGN
        update_region_writeprotect();
        printf("[qy]after upgrade auto dump info:\n");
        dumpwriteprotectregion(fdwp);
#endif

	    if (_partition_changed)
	    {
            printf("[qy]partition has been changed\n");
            deleteandfree_userdata_partition();
            writepartitioninfotoflash(newtblhead);
	     }
    }
    
    _partition_changed = 0;
    freetblmemory(xmlptbl);
    xmlptbl = NULL;
    freetblmemory(newtblhead);
    newtblhead = NULL;
    if (oldptbl){
        free(oldptbl);
        oldptbl = NULL;
    }

#ifdef NEW_PARTITION_DESIGN
    dumpwriteprotectregion(fdwp);
    close(fdwp);
    fdwp = 0;
#endif
    return (ret);

    
}


//=============================== Format FAT32 Partition ============================================
// Struct definition

/** fat boot section struct */


#pragma pack(push ,1)
typedef struct _IMAGE_BPB{
	u16 BPB_BytesPerSec;
	u8 	BPB_SecPerClus;
	u16	BPB_RsvdSecCnt;
	u8	BPB_NumFATs;
	u16 BPB_RootEntCnt;
	u16 BPB_ToSec16;
	u8 	BPB_Media;
	u16 BPB_FATSz16;
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32	BPB_HidSec;
	u32	BPB_ToSec32;
	u32	BPB_FATSz32;
	u16 BPB_Flags;
	u16 BPB_FSVer;
	u32 BPB_RootClus;
	u16	BPB_FSInfo;
	u16 BPB_Reserved;
	u8 	reserved[12];
} IMAGE_BPB, *PIMAGE_BPB;

typedef struct _IMAGE_EXTERN_BPB{
	u8 BS_DrvNum;
	u8 BS_Reserved1;
	u8 BS_BootSig;
	u32	BS_VSN;
	u8 BS_VolumeLabel[11];
	u8 BS_SystemID[8];
} IMAGE_EXTERN_BPB, *PIMAGE_EXTERN_BPB;

typedef struct _IMAGE_DBP_SECTOR{
	u8 JmpCnd[3];
	char OSVersion[8];
	IMAGE_BPB bpb;
	IMAGE_EXTERN_BPB bpb_extern;
} IMAGE_DBP_SECTOR, *PIMAGE_DBP_SECTOR;

typedef struct _IMAGE_FSINFO{
	u32 FI_BootSig;
	u8 	FI_Reserved1[480];
	u32 FI_Signature;
	u32 FI_FreeClus;
	u32 FI_NextClus;
	u8 	FI_Reserved2[14];
	u16 FI_EndSig;
} IMAGE_FSINFO, *PIMAGE_FSINFO;

#pragma pack(pop)

typedef struct _DSKSZTOSECPERCLUS {
    u32   DiskSize;
    u8    SecPerClusVal;
}DSKSZTOSECPERCLUS;


// Number of sectors to allocate in memory for read/write buffers
#define NUM_BLOCK_SECTORS 	256
#define VFAT_BLOCK_SIZE		512
#define FAT_FSINFO_SIG		0x41615252

static DSKSZTOSECPERCLUS DskTableFAT32 [] = {
    {    66600,  0},       // disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error 
    {   532480,  1},       // disks up to 260 MB,  .5k cluster 
    { 16777216,  8},       // disks up to     8 GB,    4k cluster
    { 33554432, 16},       // disks up to   16 GB,    8k cluster
    { 67108864, 32},       // disks up to   32 GB,  16k cluster
    { 0xFFFFFFFF, 64}      // disks greater than 32GB, 32k cluster
};


static void fat_fill_bpb_extern(IMAGE_EXTERN_BPB *bpb_extern)
{
	char type[] = "FAT32";
	char *p;
	bpb_extern->BS_DrvNum  = 0x80;
	bpb_extern->BS_Reserved1 = 0;
	bpb_extern->BS_BootSig = 0x29;
	bpb_extern->BS_VSN = 0x94525487;
	p = (char *)bpb_extern->BS_VolumeLabel;
	p[0] = 'N';
	p[1] = 'O';
	p[2] = ' ';
	p[3] = 'N';
	p[4] = 'A';
	p[5] = 'M';
	p[6] = 'E';
	p[7] = ' '; 
	p[8] = 'A';
	p[9] = 'T';
	p[10] = 'C';
	memcpy(bpb_extern->BS_SystemID, type, 6);
}


static u32 GetSectorsPerCluster (u32 dwTotalSectors)
{
	int i;
	u32 dwSectorsPerCluster = 0;

	for (i = 0; 1; i++) 
	{
		if (dwTotalSectors <= DskTableFAT32[i].DiskSize) 
		{
			dwSectorsPerCluster = DskTableFAT32[i].SecPerClusVal;
			break;
		}
	}

	return dwSectorsPerCluster;
}

static void fat_fill_bpb(IMAGE_BPB *bpb , u32 dwTotalSectors)
{
	bpb->BPB_BytesPerSec = VFAT_BLOCK_SIZE;
	bpb->BPB_SecPerClus = GetSectorsPerCluster(dwTotalSectors); //(1024*16)/VFAT_BLOCK_SIZE;// Ref to 'DskTableFAT32'
	bpb->BPB_RsvdSecCnt = 32;
	bpb->BPB_NumFATs = 2;
	bpb->BPB_RootEntCnt = 0;
	bpb->BPB_ToSec16 = 0;
	bpb->BPB_Media = 0xF8;
	bpb->BPB_FATSz16 = 0;
	bpb->BPB_SecPerTrk = 0x3F;
	bpb->BPB_NumHeads = 0xFF;
	bpb->BPB_HidSec = 0;
	bpb->BPB_ToSec32 = 0; //
	bpb->BPB_FATSz32 = 0; //
	bpb->BPB_Flags = 0;
	bpb->BPB_FSVer = 0;
	bpb->BPB_RootClus = 2;
	bpb->BPB_FSInfo = 1;
	bpb->BPB_Reserved = 0;
	memset(bpb->reserved, 0x00, 12);
}

static void fat_fill_dbp(IMAGE_DBP_SECTOR *dbp, u32 dwTotalSectors)
{
	char version[] = "MSDOS5.0";
	dbp->JmpCnd[0] = 0xEB;
	dbp->JmpCnd[1] = 0x58;
	dbp->JmpCnd[2] = 0x90;
	memcpy(dbp->OSVersion, version, 8);
	fat_fill_bpb(&(dbp->bpb), dwTotalSectors);
	fat_fill_bpb_extern(&(dbp->bpb_extern));
}

static void fat_fill_fsinfo(IMAGE_FSINFO *fsinfo)
{
	fsinfo->FI_BootSig = FAT_FSINFO_SIG;
	fsinfo->FI_Signature = 0x61417272;
	fsinfo->FI_NextClus = 2;
	fsinfo->FI_FreeClus = -1;
	fsinfo->FI_EndSig = 0x55AA;
}

static u32 caculate_fat_size(u32 total_sect, u32 resv_sect, u32 fat_num, u32 SecPerClus)
{
	u32 fat_sect = 0;
	u32 inval_sect = 0;
	u32 tmp = 0;
	while(1)
	{
		fat_sect++;
		inval_sect = total_sect - resv_sect - fat_sect*fat_num;
		tmp = (inval_sect/SecPerClus + 2)*4;
		if(tmp <= fat_sect * VFAT_BLOCK_SIZE)
		{
			return fat_sect;
		}
	}
}

int format_userdata_partition_fat32(u64 u8PartOffset, u64 u8PartSize)
{
   
	u32 i = 0;
    int fdev;
	u32 sector_id = 0;
    off_t offset;
	IMAGE_DBP_SECTOR *block_buffer = (PIMAGE_DBP_SECTOR)malloc(VFAT_BLOCK_SIZE);
	IMAGE_BPB *bpb = &(block_buffer->bpb);
	IMAGE_FSINFO *fsinfo;
	u_char* pWriteBuf = (u_char*)malloc(VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);

	u32 u4RemainWriteSector = 0;

    printf("format_userdata_partition_fat32 offset = 0x%llx \r\n", u8PartOffset);
	printf("format_userdata_partition_fat32 size   = 0x%llx \r\n", u8PartSize );

	fdev = open(devname, O_RDWR);

	if(block_buffer == NULL || pWriteBuf == NULL)
	{
		printf("Can not alloc memory for userdata format function\n");
		return -1;
	}

    offset = lseek(fdev, 0, SEEK_END);
    if (u8PartOffset >= offset)
    {
        close(fdev);
        return (-1);
    }
    
	if (!u8PartSize || (u8PartOffset + u8PartSize) > offset )
	{
		u8PartSize = offset - u8PartOffset;
	}
	printf("---> format_userdata_partition_fat32 device size = 0x%llx\r\n", offset );
	
	memset(block_buffer, 0x00, VFAT_BLOCK_SIZE);
	memset(pWriteBuf, 0x00, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
	
	fat_fill_dbp((IMAGE_DBP_SECTOR *)block_buffer, (u32)(u8PartSize/VFAT_BLOCK_SIZE) );
	bpb->BPB_ToSec32 = (u32)(u8PartSize / VFAT_BLOCK_SIZE);
	bpb->BPB_FATSz32 = caculate_fat_size(bpb->BPB_ToSec32, bpb->BPB_RsvdSecCnt, bpb->BPB_NumFATs, bpb->BPB_SecPerClus);
	((u_char *)block_buffer)[510] = 0x55;
	((u_char *)block_buffer)[511] = 0xAA;


	/** write boot section */
    offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
    lseek(fdev, offset, SEEK_SET);
    write(fdev, block_buffer, VFAT_BLOCK_SIZE);
	
	sector_id = 1;
	fsinfo = (IMAGE_FSINFO *)pWriteBuf;
	fat_fill_fsinfo(fsinfo); 

    offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
    lseek(fdev, offset, SEEK_SET);
    write(fdev, pWriteBuf, VFAT_BLOCK_SIZE);
    
	
	sector_id++;
	memset(pWriteBuf, 0x00, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);

    offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
    lseek(fdev, offset, SEEK_SET);
    write(fdev, pWriteBuf, VFAT_BLOCK_SIZE * (bpb->BPB_RsvdSecCnt - 2));

	
	/** write FAT table */
	((u32 *)pWriteBuf)[0] = 0x0FFFFFF8;
	((u32 *)pWriteBuf)[1] = 0x0FFFFFFF;
	((u32 *)pWriteBuf)[2] = 0x0FFFFFFF;
	sector_id = bpb->BPB_RsvdSecCnt;


    offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
    lseek(fdev, offset, SEEK_SET);
    write(fdev, pWriteBuf, VFAT_BLOCK_SIZE );
	sector_id++; //33
	memset(pWriteBuf, 0x00, 12); // Reset all data to 0

	u4RemainWriteSector = bpb->BPB_FATSz32 - 1;
	for(i = 1; i<bpb->BPB_FATSz32; )
	{
		if (u4RemainWriteSector < NUM_BLOCK_SECTORS)
		{
            offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
            lseek(fdev, offset, SEEK_SET);
            write(fdev, pWriteBuf, VFAT_BLOCK_SIZE * u4RemainWriteSector);
			sector_id += u4RemainWriteSector;
			i += u4RemainWriteSector;
			u4RemainWriteSector = 0;
		}
		else
		{
            offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
            lseek(fdev, offset, SEEK_SET);
            write(fdev, pWriteBuf, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
			sector_id += NUM_BLOCK_SECTORS;
			i += NUM_BLOCK_SECTORS;
			u4RemainWriteSector -= NUM_BLOCK_SECTORS;
		}
	}
	
	/** write 2th fat table */
	((u32 *)pWriteBuf)[0] = 0x0FFFFFF8;
	((u32 *)pWriteBuf)[1] = 0x0FFFFFFF;  
	((u32 *)pWriteBuf)[2] = 0x0FFFFFFF;

    offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
    lseek(fdev, offset, SEEK_SET);
    write(fdev, pWriteBuf, VFAT_BLOCK_SIZE );

	sector_id++;
	memset(pWriteBuf, 0x00, 12); // Reset all data to 0
	u4RemainWriteSector = bpb->BPB_FATSz32 - 1;
	for(i = 1; i<bpb->BPB_FATSz32; )
	{
		if (u4RemainWriteSector < NUM_BLOCK_SECTORS)
		{
            offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
            lseek(fdev, offset, SEEK_SET);
            write(fdev, pWriteBuf, VFAT_BLOCK_SIZE * u4RemainWriteSector);
			sector_id += u4RemainWriteSector;
			i += u4RemainWriteSector;
			u4RemainWriteSector = 0;
		}
		else
		{
            offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
            lseek(fdev, offset, SEEK_SET);
            write(fdev, pWriteBuf, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
			sector_id += NUM_BLOCK_SECTORS;
			i += NUM_BLOCK_SECTORS;
			u4RemainWriteSector -= NUM_BLOCK_SECTORS;
		}
		
	}
	
	// Root Directory, Max Size is 32 Sectors, now we earse 256 Sectors
    offset = u8PartOffset + sector_id * VFAT_BLOCK_SIZE;
    lseek(fdev, offset, SEEK_SET);
    write(fdev, pWriteBuf, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);

    close(fdev);
	free(block_buffer);
	free(pWriteBuf);
	
	printf("\r\n");
	return 0;
}




