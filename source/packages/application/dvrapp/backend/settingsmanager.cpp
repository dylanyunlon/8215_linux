#include "settingsmanager.h"
#include "dvrlog.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QCoreApplication>
#include <fstream>
#include <sstream>
#include <unistd.h>

static const char* TAG = "SettingsManager";

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_currentUsbMode(UsbModeHost)
    , m_performanceTimer(new QTimer(this))
    , m_initialized(false)
{
    LOGI(TAG, "[SettingsManager] Created");

    // Initialize CPU/Process info structures
    memset(&m_lastCpuInfo, 0, sizeof(m_lastCpuInfo));
    memset(&m_lastProcessInfo, 0, sizeof(m_lastProcessInfo));

    // Read current USB mode from sysfs
    loadCurrentUsbMode();

    // Emit initial USB mode to QML
    emit usbModeChanged(m_currentUsbMode);
    LOGI(TAG, "[USB_MODE] Initial USB mode emitted: %d", m_currentUsbMode);

    // Setup performance monitoring timer
    connect(m_performanceTimer, &QTimer::timeout, this, &SettingsManager::updatePerformanceStats);
    m_startTime = QDateTime::currentDateTime();
}

SettingsManager::~SettingsManager()
{
    stopPerformanceMonitoring();
    LOGI(TAG, "[SettingsManager] Destroyed");
}

// ============================================================================
// USB Mode Switching (Pure mode switch only, no camera operations)
// ============================================================================

bool SettingsManager::switchUsbMode(int mode)
{
    LOGI(TAG, "[USB_MODE] switchUsbMode called, new mode: %d", mode);

    // Validate mode
    if (mode != UsbModeHost && mode != UsbModeDevice) {
        LOGE(TAG, "[USB_MODE] Invalid USB mode: %d", mode);
        emit usbModeSwitchFailed("Invalid USB mode");
        return false;
    }

    // Check if mode actually changed
    if (mode == m_currentUsbMode) {
        LOGI(TAG, "[USB_MODE] USB mode unchanged, no action needed");
        return true;
    }

    LOGI(TAG, "[USB_MODE] Switching USB mode from %d to %d", m_currentUsbMode, mode);

    // Execute mode switch (pure sysfs operation)
    bool success = false;
    if (mode == UsbModeHost) {
        success = switchToHostMode();
    } else {
        success = switchToDeviceMode();
    }

    if (!success) {
        LOGE(TAG, "[USB_MODE] USB mode switch failed");
        emit usbModeSwitchFailed("Failed to switch USB mode - check permissions");
        return false;
    }

    // Update state immediately after successful mode switch
    m_currentUsbMode = mode;

    // Notify success
    LOGI(TAG, "[USB_MODE] USB mode switched successfully to: %d", mode);
    emit usbModeChanged(mode);

    return true;
}

bool SettingsManager::switchToHostMode()
{
    LOGI(TAG, "[USB_MODE] Switching to USB Host mode...");

    // Step 1: Write "host" to /sys/atc2ctl/mode
    if (!writeSysfs("/sys/atc2ctl/mode", "host")) {
        LOGE(TAG, "[USB_MODE] Failed to write host mode to sysfs");
        return false;
    }

    // Step 2: Verify the write by reading back
    QString currentMode = readSysfs("/sys/atc2ctl/mode");
    if (currentMode.trimmed() == "host") {
        LOGI(TAG, "[USB_MODE] USB Host mode verified: %s", currentMode.toUtf8().constData());
        return true;
    } else {
        LOGE(TAG, "[USB_MODE] USB Host mode verification failed, read: %s", currentMode.toUtf8().constData());
        return false;
    }
}

bool SettingsManager::switchToDeviceMode()
{
    LOGI(TAG, "[USB_MODE] Switching to USB Device mode...");

    // Step 1: Write "peripheral" to /sys/atc2ctl/mode
    if (!writeSysfs("/sys/atc2ctl/mode", "peripheral")) {
        LOGE(TAG, "[USB_MODE] Failed to write peripheral mode to sysfs");
        return false;
    }

    // Step 2: Verify the write by reading back
    QString currentMode = readSysfs("/sys/atc2ctl/mode");
    if (currentMode.trimmed() == "peripheral") {
        LOGI(TAG, "[USB_MODE] USB Device mode verified: %s", currentMode.toUtf8().constData());
        return true;
    } else {
        LOGE(TAG, "[USB_MODE] USB Device mode verification failed, read: %s", currentMode.toUtf8().constData());
        return false;
    }
}

bool SettingsManager::writeSysfs(const QString &path, const QString &value)
{
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOGE(TAG, "[USB_MODE] Failed to open sysfs file: %s, Error: %s",
             path.toUtf8().constData(), file.errorString().toUtf8().constData());
        return false;
    }

    QTextStream out(&file);
    out << value;
    file.close();

    LOGD(TAG, "[USB_MODE] Written to %s: %s",
         path.toUtf8().constData(), value.toUtf8().constData());
    return true;
}

QString SettingsManager::readSysfs(const QString &path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOGW(TAG, "[USB_MODE] Failed to read sysfs file: %s",
             path.toUtf8().constData());
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll().trimmed();
    file.close();

    LOGD(TAG, "[USB_MODE] Read from %s: %s",
         path.toUtf8().constData(), content.toUtf8().constData());
    return content;
}

void SettingsManager::loadCurrentUsbMode()
{
    LOGI(TAG, "[USB_MODE] Reading current USB mode from sysfs...");

    QString currentMode = readSysfs("/sys/atc2ctl/mode");
    if (currentMode.isEmpty()) {
        LOGW(TAG, "[USB_MODE] Failed to read sysfs, defaulting to Host mode");
        m_currentUsbMode = UsbModeHost;
        return;
    }

    QString trimmedMode = currentMode.trimmed();
    if (trimmedMode == "host") {
        m_currentUsbMode = UsbModeHost;
        LOGI(TAG, "[USB_MODE] Current mode: Host (0)");
    } else if (trimmedMode == "peripheral") {
        m_currentUsbMode = UsbModeDevice;
        LOGI(TAG, "[USB_MODE] Current mode: Device (1)");
    } else if (trimmedMode == "otg") {
        // OTG mode maps to Device for compatibility
        m_currentUsbMode = UsbModeDevice;
        LOGI(TAG, "[USB_MODE] Current mode: OTG (treating as Device)");
    } else {
        LOGW(TAG, "[USB_MODE] Unknown mode '%s', defaulting to Host", trimmedMode.toUtf8().constData());
        m_currentUsbMode = UsbModeHost;
    }
}

// ============================================================================
// Performance Monitoring
// ============================================================================

void SettingsManager::startPerformanceMonitoring()
{
    LOGI(TAG, "[PERF] Starting performance monitoring...");

    // Initialize baseline readings (matching original app)
    pid_t pid = getpid();
    LOGI(TAG, "[PERF] Initializing monitoring for PID: %d", pid);

    bool cpuInitOk = readCpuInfo(m_lastCpuInfo);
    bool procInitOk = readProcessInfo(pid, m_lastProcessInfo);

    LOGI(TAG, "[PERF] CPU init: %d, Process init: %d", cpuInitOk, procInitOk);

    m_initialized = true;
    m_startTime = QDateTime::currentDateTime();
    m_performanceTimer->start(1000);  // Update every 1 second
}

void SettingsManager::stopPerformanceMonitoring()
{
    LOGI(TAG, "[PERF] Stopping performance monitoring...");
    m_performanceTimer->stop();
}

void SettingsManager::updatePerformanceStats()
{
    if (!m_initialized) {
        LOGW(TAG, "[PERF] Not initialized, skipping update");
        return;
    }

    int fps = calculateFPS();
    float dvrCpu = getDVRCpuUsage();
    float totalCpu = getTotalCpuUsage();
    float dvrMemory = getDVRMemoryUsage();
    float totalMemory = getTotalMemoryUsage();
    QString elapsed = getElapsedTime();

    LOGD(TAG, "[PERF] Stats - FPS:%d, DVR_CPU:%.1f%%, Total_CPU:%.1f%%, DVR_Mem:%.2fMB, Total_Mem:%.2fMB",
         fps, dvrCpu, totalCpu, dvrMemory, totalMemory);

    m_performanceStats = QString(
        "=== DVR Performance Monitor ===\n"
        "FPS: %1\n"
        "GPU: N/A\n"
        "DVR CPU: %2%\n"
        "Total CPU: %3%\n"
        "DVR Memory: %4 MB\n"
        "Total Memory: %5 MB\n"
        "Time: %6"
    ).arg(fps)
     .arg(dvrCpu, 0, 'f', 1)
     .arg(totalCpu, 0, 'f', 1)
     .arg(dvrMemory, 0, 'f', 2)
     .arg(totalMemory, 0, 'f', 2)
     .arg(elapsed);

    emit performanceStatsUpdated();
}

int SettingsManager::calculateFPS()
{
    // TODO: Integrate with actual frame callback from DVR library
    return 30;
}

bool SettingsManager::readCpuInfo(CpuInfo &cpu)
{
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        LOGE(TAG, "[PERF] Failed to open /proc/stat");
        return false;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string cpuLabel;
        iss >> cpuLabel >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle
            >> cpu.iowait >> cpu.irq >> cpu.softirq >> cpu.steal;
        return true;
    }

    return false;
}

bool SettingsManager::readProcessInfo(pid_t pid, ProcessInfo &proc)
{
    std::string filename = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOGE(TAG, "[PERF] Failed to open %s", filename.c_str());
        return false;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string dummy;

        // Skip first 13 fields to get to utime (field 14)
        for (int i = 0; i < 13; ++i) {
            iss >> dummy;
        }

        iss >> proc.utime >> proc.stime >> proc.cutime >> proc.cstime;
        return true;
    }

    return false;
}

double SettingsManager::calculateCpuUsage(const CpuInfo &prevCpu, const CpuInfo &currCpu,
                                         const ProcessInfo &prevProc, const ProcessInfo &currProc)
{
    unsigned long long totalTimeDiff = (currCpu.user + currCpu.nice + currCpu.system +
                                       currCpu.idle + currCpu.iowait + currCpu.irq +
                                       currCpu.softirq + currCpu.steal) -
                                      (prevCpu.user + prevCpu.nice + prevCpu.system +
                                       prevCpu.idle + prevCpu.iowait + prevCpu.irq +
                                       prevCpu.softirq + prevCpu.steal);

    unsigned long long processTimeDiff = (currProc.utime + currProc.stime +
                                         currProc.cutime + currProc.cstime) -
                                        (prevProc.utime + prevProc.stime +
                                         prevProc.cutime + prevProc.cstime);

    if (totalTimeDiff == 0) {
        return 0.0;
    }

    return (100.0 * processTimeDiff) / totalTimeDiff;
}

float SettingsManager::getDVRCpuUsage()
{
    pid_t pid = getpid();
    CpuInfo currentCpu;
    ProcessInfo currentProc;

    if (!readCpuInfo(currentCpu) || !readProcessInfo(pid, currentProc)) {
        return 0.0;
    }

    float cpuPercent = static_cast<float>(calculateCpuUsage(m_lastCpuInfo, currentCpu,
                                                            m_lastProcessInfo, currentProc));

    m_lastCpuInfo = currentCpu;
    m_lastProcessInfo = currentProc;

    return cpuPercent;
}

float SettingsManager::getTotalCpuUsage()
{
    CpuInfo currentCpu;
    if (!readCpuInfo(currentCpu)) {
        return 0.0;
    }

    // Calculate total CPU usage (matching original app)
    unsigned long long totalDiff = (currentCpu.user + currentCpu.nice + currentCpu.system +
                                   currentCpu.idle + currentCpu.iowait + currentCpu.irq +
                                   currentCpu.softirq + currentCpu.steal) -
                                  (m_lastCpuInfo.user + m_lastCpuInfo.nice + m_lastCpuInfo.system +
                                   m_lastCpuInfo.idle + m_lastCpuInfo.iowait + m_lastCpuInfo.irq +
                                   m_lastCpuInfo.softirq + m_lastCpuInfo.steal);

    unsigned long long idleDiff = (currentCpu.idle + currentCpu.iowait) -
                                  (m_lastCpuInfo.idle + m_lastCpuInfo.iowait);

    float cpuPercent = 0.0;
    if (totalDiff > 0) {
        cpuPercent = static_cast<float>(100.0 * (totalDiff - idleDiff) / totalDiff);
    }

    // Note: m_lastCpuInfo is also updated in getDVRCpuUsage(), which is called first
    // This ensures both functions use the same baseline

    return cpuPercent;
}

float SettingsManager::getDVRMemoryUsage()
{
    pid_t pid = getpid();
    std::string filename = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream file(filename);

    if (!file.is_open()) {
        LOGE(TAG, "[PERF] Failed to open %s", filename.c_str());
        return 0.0;
    }

    float memoryMB = 0.0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string label;
            unsigned int memoryKB;
            iss >> label >> memoryKB;
            memoryMB = memoryKB / 1024.0;  // Convert KB to MB
            break;
        }
    }

    return memoryMB;
}

float SettingsManager::getTotalMemoryUsage()
{
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        LOGE(TAG, "[PERF] Failed to open /proc/meminfo");
        return 0.0;
    }

    unsigned int memTotal = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string label;
            iss >> label >> memTotal;
            break;  // Found MemTotal, return it
        }
    }

    return memTotal / 1024.0;  // Convert KB to MB
}

QString SettingsManager::getElapsedTime()
{
    qint64 seconds = m_startTime.secsTo(QDateTime::currentDateTime());
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}
