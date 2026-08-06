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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <cstdarg>
#include <stdarg.h>
#include <syslog.h>

#include <execinfo.h>

#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "utils.h"
#include "aee_internal.h"

/* Name of the output file.  */
static const char *fname;

extern bool enable_dal;
static int dal_fd = -1;

/* Some important info is outputed via console devices.    */
//#define ENABLE_BACKTRACE_TO_CONSOLE
/* DAL API is called here.            */

#define MAX_TOMBSTONES          10
#define TOMBSTONE_DIR           "/data/tombstones"
#define TOMBSTONE_TEMPLATE      (TOMBSTONE_DIR"/tombstone_%02d")
//#define AEE_NE_FULL_PATH        (INT_SDCARD_PATH"/atclog/aee/ne")
//#define AEE_NE_MAIN             (AEE_NE_PATH"/main_exp.txt")
#define AEE_TMP_FILE            "data/aee_tmp_file"

// find_and_open_tombstone - find an available tombstone slot, if any, of the
// form tombstone_XX where XX is 00 to MAX_TOMBSTONES-1, inclusive. If no
// file is available, we reuse the least-recently-modified file.
//
// Returns the path of the tombstone file, allocated using malloc().  Caller must free() it.
static char* find_and_open_tombstone(int* pfd)
{
    // In a single pass, find an available slot and, in case none
    // exist, find and record the least-recently-modified file.
    char path[128];
    int oldest = -1;
    struct stat oldest_sb;
    for (int i = 0; i < MAX_TOMBSTONES; i++) {
        snprintf(path, sizeof(path), TOMBSTONE_TEMPLATE, i);

        struct stat sb;
        if (!stat(path, &sb)) {
            if (oldest < 0 || sb.st_mtime < oldest_sb.st_mtime) {
                oldest = i;
                oldest_sb.st_mtime = sb.st_mtime;
            }
            continue;
        }
        if (errno != ENOENT)
            continue;

        *pfd = open(path, O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC, 0600);
        if (*pfd < 0)
            continue;   // raced ?

        fchown(*pfd, AID_SYSTEM, AID_SYSTEM);
        return strdup(path);
    }

    if (oldest < 0) {
        AEE_LOGE("NE: Failed to find a valid tombstone, default to using tombstone 0.\n");
        oldest = 0;
    }

    // we didn't find an available file, so we clobber the oldest one
    snprintf(path, sizeof(path), TOMBSTONE_TEMPLATE, oldest);
    *pfd = open(path, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (*pfd < 0) {
        AEE_LOGE("NE: failed to open tombstone file '%s': %s\n", path, strerror(errno));
        return NULL;
    }
    fchown(*pfd, AID_SYSTEM, AID_SYSTEM);
    return strdup(path);
}



/* read /proc/pid/cmdline by pid */
int getprocessname( char *name, int pid)
{
    FILE *file;
    char buf[MAX_PATH];

    sprintf(buf, "/proc/%d/cmdline", pid);
    file = fopen(buf, "r");
    if (!file)
        return 1;

    buf[0] = '\0';
    fgets(buf, MAX_PATH, file);
    fclose(file);

    if (strlen(buf) > 0) {
        strncpy(name, buf, MAX_PATH);
        name[MAX_PATH - 1] = 0;
    } else {
        name[0] = 0;
    }

    return 0;
}

int aee_dal_clean(void) {

    if (dal_fd < 0) {
        AEE_LOGE("%s: No DAL device available", __func__);
        return -1;
    }
    if (ioctl(dal_fd, AEEIOCTL_DAL_CLEAN, NULL) < 0) {
        AEE_LOGE("%s: clean DAL failed", __func__);
        return -1;
    }
    return 0;
}

/*
 * The new API is used for DAL(Display assert layer)
 * added by Joshua Wang.
 * Mergerd from Android debuggerd of MTK.
 * @ Input: AE_MSG Struct.
 * @ Output: Return 0 when Ok. Otherwise return 1.
*/
int aee_dal_report(int pid, const AE_Msg *p) {
    char processname[MAX_PATH];

    /*
     * Mode = 0, Most info is showed firstly.
     * SIG NUM & PID.
     */
    getprocessname(processname, pid);

    /*
     * Otherwise, Tombstones info is showed now.
     */
    AE_DAL_SETCOLOR dal_color;
    AE_DAL_DATA dal_show;
    dal_color.fgcolor = 0xFF00FF; // fg: purple
    dal_color.bgcolor = 0x00FF00; // bg: green

    switch (p->cmdId) {
    case AE_IND_FATAL_RAISED:  /* Tombstone Info.    */
        snprintf(dal_show.msg, sizeof(dal_show.msg), "%s", processname);
        dal_color.screencolor= 0xff0000; // red
        break;
    case AE_IND_EXP_RAISED:    /* VIP Info.                */
        snprintf(dal_show.msg, sizeof(dal_show.msg),
                " collect 'Exception: SIG[%d] <--%s' DB, please wait.\n", p->sig_no, processname);
        dal_color.screencolor= 0xff0000; // red
        break;
    case AE_IND_WRN_RAISED:
        snprintf(dal_show.msg, sizeof(dal_show.msg),
                " collect 'Warning %s' DB, please wait.\n", processname);
        dal_color.screencolor= 0xffff00; // yellow
        break;
    case AE_IND_REM_RAISED:
        snprintf(dal_show.msg, sizeof(dal_show.msg), " collect DB Dump, please wait.\n");
        dal_color.screencolor= 0xffff00; // yellow
        break;
    default:
        AEE_LOGE("NE: not support argument (%d)", p->cmdId);
        break;
    }

    if (ioctl(p->dal_fd, AEEIOCTL_DAL_SETCOLOR, &dal_color) < 0) {
        AEE_LOGE("NE: setcolor ioctl failed(%s)\n", strerror(errno));
        return 1;
    }

    if (ioctl(p->dal_fd, AEEIOCTL_DAL_SHOW, &dal_show) < 0) {
        AEE_LOGE("NE: ERROR: show ioctl failed(%s).\n", strerror(errno));
    }
    return 1;
}


/* We better should not use `strerror' since it can call far too many
   other functions which might fail.  Do it here ourselves.  */
static void
write_strsignal (int signal)
{
    char buf[32];
    if (signal < 0 || signal >= _NSIG || _sys_siglist[signal] == NULL) {
        sprintf(buf, "signal: (%d) -- Unknown", signal);
    } else {
        sprintf(buf, "signal: (%d) -- %s", signal, _sys_siglist[signal]);
    }
    WRITE_STRING_FD(ts_fd, buf);
    WRITE_STRING_FD(ts_fd, "\n");
}

static void show_crash_time(void) {
    time_t now;
    char *cnow = NULL;

    time(&now);
    cnow = asctime(localtime(&now));
    WRITE_STRING_FD(ts_fd, "Crash Time is : ");
    WRITE_STRING_FD(ts_fd, cnow);
    WRITE_STRING_FD(ts_fd, "\n");
}

static int readline(FILE *fd, char *buf)
{
    int i = 0;
    int ret = -1;

    memset(buf, 0x00, MAX_PATH);
    ret = fread(&buf[i], 1, 1, fd);
    while (ret != 0) {
        if ((buf[i] == 0x0D) || (buf[i] == 0x0A)) {
            buf[i + 1] = 0;
            return i;
        }
        i++;
        ret = fread(&buf[i], 1, 1, fd);
    }

    return i == 0 ? EOF : i;
}

static void show_process_info(int pid) {
    int statusfd;
    char status[MAX_PATH] = {0};
    char buf[MAX_PATH] = {0};

    if (pid) {
        WRITE_STRING_FD(ts_fd, "\nCrashed Process status:\n");
        snprintf(status, sizeof(status), "/proc/%d/status", pid);
        statusfd = open(status, O_RDONLY);
        if (statusfd) {
            while (read(statusfd, buf, sizeof(buf)) > 0) {
                WRITE_STRING_FD(ts_fd, buf);
                memset(buf, 0x00, sizeof(buf));
            }
            close(statusfd);
        }
        WRITE_STRING_FD(ts_fd, "\n");
    } else {
        AEE_LOGE("NE: Failed to get <<Process status>> because of invalid pid\n");
    }
}

// maybe name format is:  KE_xxx.tar.gz
// we can create 9999 file of KE_xxx.tar.gz max, overwrite while overflow.
static char * getnetargetname(char * path)
{
    static char gzname[MAX_PATH];
    int n = 0;

    memset(gzname, 0x00, MAX_PATH);
    DIR *dir = opendir(path);
    if (dir) {
        struct dirent *node = NULL;
        while ((node = readdir(dir)) != NULL) {
            if (strstr(node->d_name, "NE_") && strstr(node->d_name, ".tar.gz")) {
                n++;
            }
        }
        closedir(dir);
    }

    sprintf(gzname, "NE_%04d.tar.gz", n);
    AEE_LOGI("NE: gz file is: %s.\n", gzname);

    return gzname;
}


static void dump_process_info(char *path, int pid) {
    char cmd[MAX_PATH];

    sprintf(cmd, "cat /proc/%d/maps > %s/MAPS", pid, path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/%d/mountinfo > %s/MOUNTINFO", pid, path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/%d/mountstats > %s/MOUNTSTATS", pid, path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/%d/smaps > %s/SMAPS", pid, path); // this maybe need many seconds to do.
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/%d/stack > %s/STACK", pid, path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/%d/status > %s/STATUS", pid, path);
    cmd_execute(cmd);

    AEE_LOGI("NE: Complete dump process info.\n");
}

static void dump_system_info(char *path) {
    char cmd[MAX_PATH];

    sprintf(cmd, "cat /proc/buddyinfo > %s/PROC_BUDDYINFO", path);
    cmd_execute(cmd);

    #ifdef ATC_OS_LINUX
    sprintf(cmd, "cat /proc/cgroups > %s/PROC_CGROUPS", path);
    cmd_execute(cmd);
    #endif

    sprintf(cmd, "cat /proc/cpuinfo > %s/PROC_CPUINFO", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/diskstats > %s/PROC_DISKSTATS", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/interrupts > %s/PROC_INTERRUPTS", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/iomem > %s/PROC_IOMEM", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/kallsyms > %s/PROC_KALLSYMS", path); // this maybe need many seconds to do.
    cmd_execute(cmd);

    sprintf(cmd, "dmesg > %s/KMSG", path); // cat proc/kmsg will not end.
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/last_kmsg > %s/LAST_KMSG", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/meminfo > %s/PROC_MEMINFO", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/mounts > %s/PROC_MOUNTS", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/pagetypeinfo > %s/PROC_PAGETYPEINFO", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/slabinfo > %s/PROC_SLABINFO", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/softirqs > %s/PROC_SOFTIRQS", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/stat > %s/PROC_STAT", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/version > %s/PROC_VERSION", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/vmallocinfo > %s/PROC_VMALLOCINFO", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/vmstat > %s/PROC_VMSTAT", path);
    cmd_execute(cmd);

    sprintf(cmd, "cat /proc/zoneinfo > %s/PROC_ZONEINFO", path);
    cmd_execute(cmd);

    AEE_LOGI("NE: Complete dump system info.\n");
}

static void dump_all_file(char * path, int pid) {
    char cmd[MAX_PATH];

    // Clean files in tmp folder.
    sprintf(cmd, "rm -rf %s/*", path);
    cmd_execute(cmd);

    // copy tombstone
    if (fname && strlen(fname)) {
        sprintf(cmd, "cp -f %s %s", fname, path);
        cmd_execute(cmd);
    }

    // dumpsys is a very very useful tool, but it's high coupling with android.
    // dumpstat is also a useful tool, but it's also only android's tool.:-(

    // Dump Process Info of Crashed process info.
    dump_process_info(path, pid);

    // Dump System Info from proc.
    dump_system_info(path);
}

static void notify_crash_proc(FILE *btfd) {
    AEE_LOGI("NE: Notify Crash Process.");
    if (btfd != NULL) {
        fclose(btfd);
        btfd = fopen(AEE_TMP_FILE, "w");
        fseek(btfd, 0, SEEK_SET);
        if (0 >= fwrite("END", 3, 1, btfd))
            AEE_LOGE("NE: Failed to Notify END for SegFault.\n");
        fflush(btfd);
        fsync(fileno(btfd));
        fclose(btfd);
        WRITE_STRING("**********Dump Exception Info End**********.\n");
    } else {
        WRITE_STRING("**********Dump Exception Info End without Feedback to SegFault**********.\n");
    }
}

static void copy_coredump_file(int pid, char *path) {
    int cnt = 10;
    struct stat sb;
    int filesz = 0;
    char str_c[MAX_PATH];

    // coredump file will be generated after crashed process end.
    // current process is forked from crashed process. so can't copy it.
    // TODO: 
    return ;

    sprintf(str_c, "%s/core.%d", INT_COREDUMP_PATH, pid);
    stat(path, &sb);
    while (cnt--) {
        filesz = sb.st_size;
        sleep(2); // sleep 1s.
        //AEE_LOGI("NE: core dump file maybe not generated still, wait for a sec(%d).\n", cnt);
        stat(path, &sb);
        filesz = sb.st_size;
        AEE_LOGI("TEST NE: ret of stat is %d - %d.", cnt, filesz);
    }

    AEE_LOGI("TEST NE: cnt is %d.", cnt);

    if (cnt > 0) {
        sprintf(str_c, "cp -f %s/core.%d %s", INT_COREDUMP_PATH, pid, path);
        cmd_execute(str_c);
    }
}

static void synctostorage(char *path) {
    FILE *fp = fopen(path, "r+");
    if (fp) {
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }
}

void dump_ne(int signo, int pid) {
    char path[MAX_PATH];
    char str_c[MAX_PATH];
    char *gzname = NULL;

#ifdef ENABLE_BACKTRACE_TO_CONSOLE
    fd = open("dev/console", O_TRUNC | O_WRONLY | O_CREAT, 0666);
    if (fd == -1) {
        AEE_LOGE("NE: Failed to Open the console device.\n");
        fd = 2;
    }
#endif

    if ((mkdir(TOMBSTONE_DIR, 0755) == -1) && (errno != EEXIST)) {
        AEE_LOGE("NE: Failed to create %s: %s\n", TOMBSTONE_DIR, strerror(errno));
    } else {
        if (chown(TOMBSTONE_DIR, AID_SYSTEM, AID_SYSTEM) == -1) {
            AEE_LOGE("NE: Failed to change ownership of %s: %s\n", TOMBSTONE_DIR, strerror(errno));
        }
    }
    fname = find_and_open_tombstone(&ts_fd);

    if (enable_dal) {
        AE_Msg msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmdId  = AE_IND_EXP_RAISED;
        msg.sig_no = signo;

        if (dal_fd = -1)
            dal_fd = open(AE_EE_DEVICE_PATH, O_RDONLY);
        if (dal_fd != -1) {
            msg.dal_fd = dal_fd;
            aee_dal_report(pid, &msg);
        }
    }

    sprintf(str_c, "**********Dump Exception Info, signo is %d, pid is %d**********.\n", signo, pid);
    WRITE_STRING(str_c);

    show_crash_time();
    write_strsignal(signo);

    show_process_info(pid);

    //Dump Backtrace and memmap to tombstone file.
    memset(str_c, 0x00, sizeof(str_c));
    FILE *btfd = fopen(AEE_TMP_FILE, "r");
    if (btfd != NULL) {
        while (readline(btfd, str_c) != EOF) {
            if (strlen(str_c))
                WRITE_STRING_FD(ts_fd, str_c);
        }
    }

#ifdef CONFIG_ATC_NAND
    if (0 == access(INT_SDCARD_PATH, F_OK))
    {
        AEE_LOGI("UDISK is ready\n");
	} else {
        AEE_LOGI("UDISK is not mount\n");
        goto ERR_END;
    }
#endif

    #ifndef ATC_OS_LINUX
    // create dir for core dump.
    sprintf(path, "%s/coredump", INT_SDCARD_PATH);
    if (!create_dir(path)) {
        AEE_LOGE("Failed to create dir: %s", path);
    }
    #endif

    //AEE_LOGI("+++++++++ More info, Pls Check %s !! +++++++++\n", fname);
    // Copy all info file to xxx/atclog/aee/ne/tmp folder
    sprintf(path, "%s/%s/tmp", INT_SDCARD_PATH, AEE_NE_PATH);
    if (create_dir(path)) {
        dump_all_file(path, pid);
    } else {
        goto ERR_END;
    }

    // copy coredump file: core.$(PID) from default folder to tmp folder.
    copy_coredump_file(pid, path);

    // Compress xxx/atclog/aee/ne/tmp folder to NE.xx.tar.gz
    AEE_LOGI("NE: Compress Temp files to .tar.gz.");
    sprintf(path, "%s/%s", INT_SDCARD_PATH, AEE_NE_PATH);
    gzname = getnetargetname(path);
    #ifdef ATC_OS_LINUX
    sprintf(str_c, "cd %s/tmp && tar -czvf ../%s ./* && cd -", path, gzname);
    #else
    sprintf(str_c, "cd %s/tmp && tar -cvf ../%s ./* && cd -", path, gzname);
    #endif
    if (!cmd_execute(str_c)) {
        AEE_LOGE("NE: Failed to compress aee dump files, cmd line is:\n");
        AEE_LOGE(str_c);
    }

    sprintf(path, "%s/%s/%s", INT_SDCARD_PATH, AEE_NE_PATH, gzname);
    synctostorage(path);

    // delete temp file.
    AEE_LOGI("NE: Delete Temp files.");
    sprintf(str_c, "rm -rf %s/%s/tmp", INT_SDCARD_PATH, AEE_NE_PATH);
    if (!cmd_execute(str_c))
        AEE_LOGE("NE: Failed to delete files in folder: %s", str_c);

    // Notify Crash Process "Dump Info End."
    notify_crash_proc(btfd);

ERR_END:
    ;
#ifdef ENABLE_BACKTRACE_TO_CONSOLE
    close(fd);
#endif
}

