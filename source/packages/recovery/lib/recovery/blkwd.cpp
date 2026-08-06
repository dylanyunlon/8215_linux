#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/inotify.h>
#include <limits.h> 
#include <poll.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "err_num.h"
#include "recovery.h"
#include "blkwd.h"
#include "roots.h"
#include "atc_safe_upgrade.h"
#include "atc_upgrade_common.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
//extern const char *UDISK_ROOT;
extern const char *SD_ROOT;
//extern const char *SDCARD_ROOT;


int init_blkwd(struct wd_name *pwd)
{
    if (pwd == NULL) {
        rec_err( "invalid argument \n");
        return -EINVAL;
    }

    /*creating the INOTIFY instance*/
    pwd->fd = inotify_init();
    if (pwd->fd < 0) {
        rec_err("inotify_init fail\n");
        return -ESYSCALL;
    }

    /*adding the "/dev/" into watch list. Here, the suggestion is to validate the existence of the directory before adding into monitoring list.*/
    pwd->wd = inotify_add_watch(pwd->fd, pwd->name, IN_CREATE | IN_DELETE);

    if (pwd->wd < 0) {
        rec_err("Add watch for inotify failure \n");
        return -ESYSCALL;
    }

    return 0;
}

int destory_blkwd(struct wd_name *pwd)
{
    if (pwd == NULL) {
        rec_err("invalid argument\n");
        return -EINVAL;
    }

    inotify_rm_watch(pwd->fd, pwd->wd);

    /*closing the INOTIFY instance*/
    close(pwd->fd);
    pwd->fd = -1;
    pwd->wd = -1;
    return 0;
}

int blkwd_event(struct wd_name *pwd)
{
    if (pwd == NULL) {
        rec_err("invalid argument\n");
        return -EINVAL;
    }

    int length, i = 0;
    char buffer[EVENT_BUF_LEN];

    rec_info("begin to monitor blk device hotplug...\n");

/*read to determine the event change happens on"/dev/" directory. Actually this read blocks until the change event occurs*/
    struct pollfd pfd = { pwd->fd, POLLIN, 0 };
    int ret = poll(&pfd, 1, 10);  // timeout of 10ms
    if (ret < 0) {
        rec_err("poll failed: %s\n", strerror(errno));
        return  -ESYSCALL;
    } else if (ret == 0) {
    // Timeout with no events, move on.
        rec_warn("time out move on\n");
        return -ESYSCALL;
    } else {
    // Process the new event.
        struct inotify_event event;
        length = read(pwd->fd, buffer, EVENT_BUF_LEN);
        rec_info("begin to read\n");

        /*checking for error*/
        if (length < 0) {
            rec_err("read failure\n");
            return  -ESYSCALL;
        }

        ret = 0;
        /*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            rec_info("i=%d, length=&d\n", i, length);
            if (event->len) {
                rec_info("event->len=%d\n", event->len);
                if (event->mask & IN_CREATE) {
                    if (event->mask & IN_ISDIR) {
                        rec_note("New directory %s created.\n", event->name);
                    } else {
                        rec_info("New file %s created.\n", event->name );
                        if ((strlen(event->name) == 3) || (strlen(event->name) == 4)) {//name is "sdx or sdxy"
                            Volume *v = volume_for_path(UDISK_ROOT);
                            char dev_node[16]="/dev/";
                            strcat((char*)dev_node, event->name);
                            if ((v) && ((strlen(v->device) == 8) || (strlen(v->device) == 9)) &&
                             (strcmp(v->device, dev_node) != 0) ) {
                                strcpy((char*)v->device, (const char*)dev_node);
                                strncpy(ntfs_mount_device, (const char*)dev_node, NTFS_MOUNT_DEVICE_LEN);
                                rec_dbg("v->device=%s,ntfs_mount_device=%s.\n", v->device, ntfs_mount_device);
                                ret = UDISKPLUGIN;
                            }
                        }
                        // sdcard
                        if ((strlen(event->name) == 7) || (strlen(event->name) == 9)) {  //name is "mmcblk1 or mmcblk1p1"
                            rec_info("[qy]qiyun debug3\n");
                            Volume *v = volume_for_path(SD_ROOT);
                            char dev_node[16]="/dev/";
                            strcat(dev_node, event->name);
                            if ((v) && ((strlen(v->device) == 12) || (strlen(v->device) == 14)) &&
                             (strcmp(v->device, dev_node) != 0) ) {
                                strcpy((char *)v->device, (const char*)dev_node);
                                strncpy(ntfs_mount_device, (const char*)dev_node, NTFS_MOUNT_DEVICE_LEN);
                                rec_dbg("v->device=%s,ntfs_mount_device=%s.\n", v->device, ntfs_mount_device);
                                ret = SDCARDPLUGIN;
                             }
                        }
                    }
                }
                else if ( event->mask & IN_DELETE ) {
                    if ( event->mask & IN_ISDIR ) {
                        rec_info( "Directory %s deleted.\n", event->name );
                    }
                    else {
                        rec_info( "Old File %s deleted.\n", event->name );
                        if ((strlen(event->name) == 3) || (strlen(event->name) == 4)) {
                            ensure_path_unmounted(UDISK_ROOT);
                            rec_info(" umount the udisk for testing\n");
                            ret = UDISKPLUGOUT;
                        }
                        if (((strlen(event->name) == 7)&&(NULL != strstr(event->name,"mmcblk"))) || ((strlen(event->name) == 9)&&(NULL != strstr(event->name,"mmcblk1p")))) {
                            ensure_path_unmounted(SD_ROOT);
                            rec_info(" umount the sdcard for testing\n");
                            ret = SDCARDPLUGOUT;
                        }
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
        /*removing the /dev/block/sda1 file from the watch list.*/
    }
    rec_info("ret =%d\n", ret);
    return ret;
}


