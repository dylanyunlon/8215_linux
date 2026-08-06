#ifndef LIB_SYSINFO_H
#define LIB_SYSINFO_H

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <string>
using namespace std;

#define MAX_LINE_LENGTH 1024

class MxcSysInfo {
public:
    typedef struct CPUPACKED{
        char name[20];
        unsigned int user;
        unsigned int nice;
        unsigned int sys;
        unsigned int idle;
        unsigned int iowait;
        unsigned int irq;
        unsigned int softirq;
    } CPU_OCCUPY;

    typedef struct MEMPACKED{
        char nameTotal[20];
        unsigned int total;
        char nameFree[20];
        unsigned int free;
        char nameAvailable[20];
        unsigned int available;
    } MEM_OCCUPY;

    typedef struct {
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    } CpuTimes;

    typedef struct {
        unsigned long long utime, stime, cutime, cstime;
    } ProcTimes;

    static int read_cpu_times(CpuTimes *cpu_times);
    static int read_proc_times(pid_t pid, ProcTimes *proc_times);
    static double calculate_cpu_usage(const CpuTimes *prev_cpu, const CpuTimes *curr_cpu,
                               const ProcTimes *prev_proc, const ProcTimes *curr_proc);

	static float getTotalMemPercent();
	static float getProcessMemPercent(const pid_t p);
	static float getProcessMemPercent(const char* ProcessName);
	static unsigned int getProcessMem(const pid_t p);
	static unsigned int getProcessMem(const char* ProcessName);
	static unsigned int getTotalMem();

	static float getTotalCpuPercent();
	static float getProcessCpuPercent(const pid_t p);
	static float getProcessCpuPercent(const char* ProcessName);

	static pid_t pidof(const char* ProcessName);

	static int get_memoccupy(MxcSysInfo::MEM_OCCUPY* mem);
    static unsigned int get_cpuoccupy(MxcSysInfo::CPU_OCCUPY* mem);
	static char *get_items(const char *buffer, int ie);
    //gpu
    static std::string getGPULoading();

	//cpu
    static int cal_cpuoccupy(MxcSysInfo::CPU_OCCUPY *o, MxcSysInfo::CPU_OCCUPY *n);
	static unsigned int get_cpu_process_occupy(const pid_t p);
	static unsigned int get_cpu_total_occupy();
	static unsigned int get_cpu_core_number();
	static unsigned int get_cpu_core_useage();
    static double get_cluster_app_cpu_usage();
    static void parseTopOutput(double& totalCpuUsage, double& processCpuUsage);

	//mem
	static unsigned int get_phy_mem(const pid_t p);   //获取进程占用物理内存
	static unsigned int get_total_mem();
	static unsigned int get_phy_total_pages();
	static unsigned int get_phy_available_pages();
};

#endif
