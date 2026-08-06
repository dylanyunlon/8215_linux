#include "clusterappperformance.h"
#include <QTime>
#include <QFile>
#include "unistd.h"
#include "clog.h"

const static char* TAG = "ClusterAppPerformance";
ClusterAppPerformance *ClusterAppPerformance::s_instance = nullptr;
ClusterAppPerformance::ClusterAppPerformance()
{

}

ClusterAppPerformance *ClusterAppPerformance::getInstance()
{
    if (s_instance == nullptr) {
        s_instance = new ClusterAppPerformance();
    }

    return s_instance;
}

QStringList ClusterAppPerformance::getPerformance()
{
    static int fps = 0;
    static int lastTime = QTime::currentTime().msecsSinceStartOfDay();
    static int frameCount = 0;
    static QString gpuLoding = "0%";
    //cpu
    static int cpuLoading  = 0;
    static int pidCpuLoading  = 0;
    static MxcSysInfo::CPU_OCCUPY lastCpu = {0};
    static unsigned int lastTotaltime = 0;
    static unsigned int lastPidTime = 0;
    //memory
    static unsigned int totalMemory = 0;
    static unsigned int pidMemory = 0;

    ++frameCount;
    int curTime = QTime::currentTime().msecsSinceStartOfDay();
    if (curTime - lastTime > 1000) {  //1s
        //FPS
        fps = frameCount;
        if(fps > 60) {
           fps = 60;
        }
        frameCount = 0;
        lastTime = curTime;
        //GPU
        gpuLoding = QString::fromStdString(MxcSysInfo::getGPULoading());
        //CPU
        MxcSysInfo::CPU_OCCUPY cpu;
        pid_t pid = getpid();
        unsigned int totalTime  = MxcSysInfo::get_cpuoccupy(&cpu);
        unsigned int pidTime = MxcSysInfo::get_cpu_process_occupy(pid);
        cpuLoading = MxcSysInfo::cal_cpuoccupy(&lastCpu, &cpu);
        pidCpuLoading = (((pidTime - lastPidTime) * 100) / (totalTime - lastTotaltime));
        //memory
        totalMemory = MxcSysInfo::get_total_mem();
        pidMemory = MxcSysInfo::get_phy_mem(pid);

        lastCpu = cpu;
        lastTotaltime = totalTime;
        lastPidTime = pidTime;
    }

    return QStringList({ QString("Fps: %1").arg(fps)
                       , QString("Gpu: %1").arg(gpuLoding)
                       , QString("Cluster Cpu:%1%").arg(pidCpuLoading)
                       , QString("Total Cpu:%1%").arg(cpuLoading)
                       , QString("Cluster Mem:%1MB").arg(QString::number(pidMemory / 1024.0, 'f', 2))
                       , QString("Total Mem:%1MB").arg(QString::number(totalMemory / 1024.0, 'f', 2))});

}

uint ClusterAppPerformance::getFps()
{
    static uint fps = 0;
    static uint lastTime = QTime::currentTime().msecsSinceStartOfDay();
    static uint frameCount = 0;

    ++frameCount;
    int curTime = QTime::currentTime().msecsSinceStartOfDay();
    if (curTime - lastTime > 1000) {
        fps = frameCount;
        frameCount = 0;
        lastTime = curTime;
    }

    return fps;
}