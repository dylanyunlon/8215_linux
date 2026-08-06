#include <fstream>
#include <sstream>
#include <iomanip>
#include "libsysinfo.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int MxcSysInfo::get_memoccupy(MxcSysInfo::MEM_OCCUPY *mem)
{
	FILE *pf;
	int n;
	char buff[256];
	char temp[20];
	MxcSysInfo::MEM_OCCUPY *m = mem;

	pf = fopen ("/proc/meminfo", "r");
	if(pf == NULL)
		return -1;

	char *pstatus = NULL;
	pstatus = fgets (buff, sizeof(buff), pf);
	if( pstatus == NULL )
	{
		fclose(pf);
		return -1;
	}

	if(sscanf(buff, "%s %u %s", m->nameTotal, &m->total, temp) != 3)
	{
		fclose(pf);
		return -1;
	}

	pstatus = fgets (buff, sizeof(buff), pf);
	if( pstatus == NULL )
	{
		fclose(pf);
		return -1;
	}

	if(sscanf(buff, "%s %u %s", m->nameFree, &m->free, temp) != 3)
	{
		fclose(pf);
		return -1;
	}

	pstatus = fgets (buff, sizeof(buff), pf);
	if( pstatus == NULL )
	{
		fclose(pf);
		return -1;
	}

	if(sscanf (buff, "%s %u %s", m->nameAvailable, &m->available, temp) !=3)
	{
		fclose(pf);
		return -1;
	}

	fclose(pf);
	return 0;
}


unsigned int MxcSysInfo::get_cpuoccupy(MxcSysInfo::CPU_OCCUPY *cpust)
{
	FILE *fd;
	int n;
	char buff[256];
	char *pstatus;
	MxcSysInfo::CPU_OCCUPY *cpu_occupy;
	cpu_occupy=cpust;

	fd = fopen("/proc/stat", "r");
	if(fd == NULL)
		return -1;

	pstatus =fgets(buff, sizeof(buff), fd);
	if(pstatus == NULL)
	{
		fclose(fd);
		return -1;
	}


	if(sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name
				,&cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->sys, &cpu_occupy->idle
				,&cpu_occupy->iowait, &cpu_occupy->irq,&cpu_occupy->softirq) != 8)
	{
		fclose(fd);
		return -1;

	}
	fclose(fd);

    return (cpu_occupy->user + cpu_occupy->nice + cpu_occupy->sys + cpu_occupy->idle + cpu_occupy->iowait + cpu_occupy->softirq
        + cpu_occupy->irq);
}

int MxcSysInfo::cal_cpuoccupy(MxcSysInfo::CPU_OCCUPY *o, MxcSysInfo::CPU_OCCUPY *n)
{
	unsigned long od, nd;
	unsigned long id, sd;
	int cpu_use = 0;
    unsigned long preIdle = o->idle + o->iowait;
    unsigned long currIdle = n->idle + n->iowait;

	unsigned long preNonIdle = o->user + o->nice + o->sys  + o->irq + o->softirq;
	unsigned long currNonIdle = n->user + n->nice + n->sys + n->irq + n->softirq;

	unsigned long  preTotal = preIdle + preNonIdle;
    unsigned long  currTotal = currIdle + currNonIdle;

    double totalDiff = static_cast<double>(currTotal - preTotal);
    double idleDiff = static_cast<double>(currIdle - preIdle);


    return  static_cast<int>((totalDiff - idleDiff) / totalDiff * 100);
}


float MxcSysInfo::getTotalCpuPercent()
{
	MxcSysInfo::CPU_OCCUPY o;
	MxcSysInfo::CPU_OCCUPY n;
	memset(&o, 0 , sizeof(MxcSysInfo::CPU_OCCUPY));
	memset(&n, 0 , sizeof(MxcSysInfo::CPU_OCCUPY));

	if(get_cpuoccupy(&o) < 0)
		return 0.0;

	usleep(1000*500);

	if(get_cpuoccupy(&n) < 0)
		return 0.0;

    return cal_cpuoccupy(&o, &n) / 100.0;
}

float MxcSysInfo::getTotalMemPercent()
{
	MxcSysInfo::MEM_OCCUPY mem;
	memset(&mem, 0 , sizeof(mem));
	if(get_memoccupy(&mem)<0)
		return 0.0;
	return ((mem.total - mem.free)*10000 / mem.total)/100.0;
}


unsigned int MxcSysInfo::get_cpu_core_number()
{
	return sysconf(_SC_NPROCESSORS_CONF);
}

unsigned int MxcSysInfo::get_cpu_core_useage()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

unsigned int MxcSysInfo::get_phy_total_pages()
{
	return sysconf(_SC_PHYS_PAGES);
}

unsigned int MxcSysInfo::get_phy_available_pages()
{
	return sysconf(_SC_AVPHYS_PAGES);
}

unsigned int MxcSysInfo::get_phy_mem(const pid_t p)
{

	char file[128] = {0};
	char line_buff[256] = {0};
	sprintf(file, "/proc/%d/status", p);

	FILE *pf;
	pf = fopen(file, "r");
	if(pf == NULL)
		return 0;

	char name[32];
	unsigned int vmrss;
	while(fgets(line_buff, sizeof(line_buff), pf) != NULL)
	{
		sscanf(line_buff, "%s %d", name, &vmrss);
		if (strcmp("VmRSS:", name) == 0)
		{
			fclose(pf);
			return vmrss;
		}
	}
	fclose(pf);
	return 0;
}

unsigned int MxcSysInfo::get_total_mem()
{
	char file[128] = {0};
	char line_buff[256] = {0};
	sprintf(file, "/proc/meminfo");

	FILE *pf;
	pf = fopen(file, "r");
	if(pf == NULL)
		return 0;

	char name[32];
	unsigned int MemTotal;
	while(fgets(line_buff, sizeof(line_buff), pf) != NULL)
	{
		sscanf(line_buff, "%s %d", name, &MemTotal);
		if (strcmp("MemTotal:", name) == 0)
		{
			fclose(pf);
			return MemTotal;
		}
	}
	fclose(pf);
	return 0;
}


float MxcSysInfo::getProcessMemPercent(const pid_t p)
{
	unsigned int  phymem = get_phy_mem(p);
	if(phymem == 0)
		return 0.0;
	unsigned int  totalmem = get_total_mem();
	if(totalmem == 0)
		return 0.0;
	//    return (phymem*1.0)/(totalmem*1.0) * 100;
	return ((phymem*10000)/totalmem) / 100.0;
}


float MxcSysInfo::getProcessMemPercent(const char* ProcessName)
{
	pid_t pid = pidof(ProcessName);
	if(pid < 0)
		return 0.0;
	return getProcessMemPercent(pid);
}


unsigned int MxcSysInfo::get_cpu_process_occupy(const pid_t p)
{
	char file[128] = {0};
	char line_buff[1024] = {0};
	sprintf(file, "/proc/%d/stat", p);

	FILE *pf;
	pf = fopen(file, "r");
	if( pf == NULL ) return 0;

	if( fgets(line_buff, sizeof(line_buff), pf) == NULL )
	{
		fclose(pf);
		return 0;
	}


	char* q = get_items(line_buff, 14);
	if(q==NULL) return 0;


	MxcSysInfo::CPU_OCCUPY cpu_occupy;
	memset(&cpu_occupy, 0 , sizeof(cpu_occupy));


	if(sscanf(q, "%u %u %u %u",&cpu_occupy.user, &cpu_occupy.nice,&cpu_occupy.sys, &cpu_occupy.idle) != 4)
	{
		fclose(pf);
		return 0;
	}

	fclose(pf);

	return (cpu_occupy.user + cpu_occupy.nice + cpu_occupy.sys + cpu_occupy.idle);
}

unsigned int MxcSysInfo::get_cpu_total_occupy()
{
	char line_buff[1024] = {0};
	FILE *pf;
	MxcSysInfo::CPU_OCCUPY cpu_occupy;

	pf = fopen("/proc/stat", "r");
	if(pf == NULL)
		return 0;

	if(fgets(line_buff, sizeof(line_buff), pf) == NULL)
	{
		fclose(pf);
		return 0;
	}

	if(sscanf(line_buff, "%s %u %u %u %u", cpu_occupy.name
				,&cpu_occupy.user, &cpu_occupy.nice
				,&cpu_occupy.sys, &cpu_occupy.idle) !=5)
	{
		fclose(pf);
		return 0;
	}

	fclose(pf);

	return (cpu_occupy.user + cpu_occupy.nice + cpu_occupy.sys + cpu_occupy.idle);
}

float MxcSysInfo::getProcessCpuPercent(const pid_t p)
{
	unsigned int totaltime1, totaltime2;
	unsigned int procetime1,  procetime2;

	totaltime1 = get_cpu_total_occupy();
	if(totaltime1 == 0)
		return 0.0;
	procetime1 = get_cpu_process_occupy(p);
	if(procetime1 == 0)
		return 0.0;

	usleep(500*1000);

	totaltime2 = get_cpu_total_occupy();
	if(totaltime2 == 0)
		return 0.0;
	procetime2 = get_cpu_process_occupy(p);
	if(procetime2 == 0)
		return 0.0;

	//    return 100.0*(procetime2 - procetime1)*1.0/((totaltime2-totaltime1)*1.0);
	return (((procetime2 - procetime1)*10000)/(totaltime2-totaltime1))/100.0;
}

float MxcSysInfo::getProcessCpuPercent(const  char* ProcessName)
{
	pid_t pid = pidof(ProcessName);
	if(pid < 0)
		return 0.0;
	return getProcessCpuPercent(pid);
}




char* MxcSysInfo::get_items(const char *buffer, int ie)
{
	if(buffer == NULL)
		return NULL;

	char *p =const_cast<char*>(buffer);
	int len = strlen(buffer);
	int count = 0;

	if( ie <= 1 )
		return p;

	int i;
	for (i = 0; i<len; i++)
	{
		if(' ' == *p)
		{
			count++;
			if (count == ie - 1)
			{
				p++;
				break;
			}
		}
		p++;
	}

	if(i >= (len-1))
		return NULL;
	else
        return p;
}

std::string MxcSysInfo::getGPULoading()
{
    FILE *pf = nullptr;
    char gpuLoadStr[16] = {0};

    pf = fopen("/sys/kernel/debug/mali/utilization_gp_pp", "r");
    if (pf == nullptr) {
        return "";
    }

    if (fscanf(pf, "%15s", gpuLoadStr) < 1) {
        fclose(pf);
        return "";
    }

    fclose(pf);

    int value = atoi(gpuLoadStr);

    if (value < 0) value = 0;
    if (value > 255) value = 255;
    int percent = value * 100 / 255;
    return std::to_string(percent) + "%";
}

pid_t MxcSysInfo::pidof(const char* ProcessName)
{
	DIR *dir;
	struct dirent *d;
	int pid,i = 0;
	char *s;

	int pnlen = strlen(ProcessName);
	dir = opendir("/proc");
	if(!dir)
		return -1;

	while( (d = readdir(dir)) != NULL )
	{
		char exe[PATH_MAX+1];
		char path[PATH_MAX+1];
		int len;
		int namelen;

		if( (pid = atoi(d->d_name)) ==0 )
			continue;

		snprintf(exe, sizeof(exe), "/proc/%s/exe",d->d_name);
		if ((len = readlink(exe,path,PATH_MAX)) < 0)
			continue;

		path[len] = '\0';

		s = strrchr(path, '/');

		if(s == NULL)
			continue;
		s++;

		namelen = strlen(s);

		if(namelen < pnlen)
			continue;

		if(!strncmp(ProcessName, s, pnlen))
		{
			if(s[pnlen] == ' ' || s[pnlen] == '\0')
			{
				closedir(dir);
				return pid;
			}
		}
	}
	closedir(dir);
	return -1;
}

unsigned int MxcSysInfo::getProcessMem(const pid_t p)
{
	return get_phy_mem(p);
}

unsigned int MxcSysInfo::getProcessMem(const char* ProcessName)
{
	pid_t pid = pidof(ProcessName);
	if(pid < 0)
		return 0.0;
	return get_phy_mem(pid);
}

unsigned int MxcSysInfo::getTotalMem()
{
	return get_total_mem();
}

int MxcSysInfo::read_cpu_times(CpuTimes *cpu_times) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        perror("fgets");
        fclose(fp);
        return -1;
    }

    sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
           &cpu_times->user, &cpu_times->nice, &cpu_times->system, &cpu_times->idle,
           &cpu_times->iowait, &cpu_times->irq, &cpu_times->softirq, &cpu_times->steal);

    fclose(fp);
    return 0;
}

int MxcSysInfo::read_proc_times(pid_t pid, ProcTimes *proc_times) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        perror("fgets");
        fclose(fp);
        return -1;
    }

    sscanf(buffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu %llu %llu",
           &proc_times->utime, &proc_times->stime, &proc_times->cutime, &proc_times->cstime);

    fclose(fp);
    return 0;
}

double MxcSysInfo::calculate_cpu_usage(const CpuTimes *prev_cpu, const CpuTimes *curr_cpu,
                           const ProcTimes *prev_proc, const ProcTimes *curr_proc) {
    unsigned long long prev_total_cpu = prev_cpu->user + prev_cpu->nice + prev_cpu->system +
                                        prev_cpu->idle + prev_cpu->iowait + prev_cpu->irq +
                                        prev_cpu->softirq + prev_cpu->steal;
    unsigned long long curr_total_cpu = curr_cpu->user + curr_cpu->nice + curr_cpu->system +
                                        curr_cpu->idle + curr_cpu->iowait + curr_cpu->irq +
                                        curr_cpu->softirq + curr_cpu->steal;

    unsigned long long total_cpu_diff = curr_total_cpu - prev_total_cpu;

    unsigned long long prev_proc_time = prev_proc->utime + prev_proc->stime +
                                        prev_proc->cutime + prev_proc->cstime;
    unsigned long long curr_proc_time = curr_proc->utime + curr_proc->stime +
                                        curr_proc->cutime + curr_proc->cstime;

    unsigned long long proc_time_diff = curr_proc_time - prev_proc_time;

    return (double)proc_time_diff / total_cpu_diff * 100.0;
}

double MxcSysInfo::get_cluster_app_cpu_usage() {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    double cpu_usage = 0.0;

    fp = popen("top -b -n 1 | grep 'cluster-app'", "r");
    if (fp == NULL) {
        perror("popen");
        return cpu_usage;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "cluster-app") != NULL) {
            sscanf(line, "%*d %*d %*s %*c %*d %*d %*d %*d %lf", &cpu_usage);
            break;
        }
    }

    fclose(fp);
    return cpu_usage;
}

void MxcSysInfo::parseTopOutput(double& totalCpuUsage, double& processCpuUsage) {
    char buffer[1024];
    totalCpuUsage = 0.0;
    processCpuUsage = 0.0;
    bool processFound = false;
    FILE* fp = popen("top -b -n 1", "r");
    if (fp == nullptr) {
        std::cerr << "Failed to run top command" << std::endl;
    }


    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        if (strncmp(buffer, "CPU:", 4) == 0) {
            int usr, sys, nic, idle, io, irq, sirq;
            if (sscanf(buffer, "CPU: %d%% usr   %d%% sys   %d%% nic  %d%% idle   %d%% io   %d%% irq   %d%% sirq",
                       &usr, &sys, &nic, &idle, &io, &irq, &sirq) == 7) {
                totalCpuUsage = usr + sys + nic + io + irq + sirq;
            }
        }

        if (strstr(buffer, "cluster-app") != nullptr) {
            int pid, ppid;
            char user[32], stat[2], command[256];
            unsigned long vsize;
            double cpuUsage;
            if (sscanf(buffer, "%d %d %s %s %lu %lf %s",
                       &pid, &ppid, user, stat, &vsize, &cpuUsage, command) >= 6) {
                processCpuUsage = cpuUsage;
                processFound = true;
            }
        }
    }

    if (!processFound) {
        std::cerr << "cluster-app process not found." << std::endl;
    }
}
