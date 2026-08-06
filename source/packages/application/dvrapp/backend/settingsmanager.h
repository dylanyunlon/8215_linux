#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int usbMode READ getUsbMode NOTIFY usbModeChanged)
    Q_PROPERTY(QString performanceStats READ getPerformanceStats NOTIFY performanceStatsUpdated)

public:
    enum UsbMode {
        UsbModeHost = 0,
        UsbModeDevice = 1
    };
    Q_ENUM(UsbMode)

    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager();

    // USB Mode control
    int getUsbMode() const { return m_currentUsbMode; }
    Q_INVOKABLE bool switchUsbMode(int mode);

    // Performance monitoring
    QString getPerformanceStats() const { return m_performanceStats; }
    Q_INVOKABLE void startPerformanceMonitoring();
    Q_INVOKABLE void stopPerformanceMonitoring();

signals:
    void usbModeChanged(int newMode);
    void usbModeSwitchFailed(QString errorMessage);
    void performanceStatsUpdated();

private slots:
    void updatePerformanceStats();

private:
    // USB Mode
    int m_currentUsbMode;
    bool switchToHostMode();
    bool switchToDeviceMode();
    bool writeSysfs(const QString &path, const QString &value);
    QString readSysfs(const QString &path);
    void loadCurrentUsbMode();

    // Performance monitoring
    QTimer *m_performanceTimer;
    QString m_performanceStats;
    QDateTime m_startTime;

    // CPU/Process tracking structures (matching original app)
    struct CpuInfo {
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    };

    struct ProcessInfo {
        unsigned long long utime, stime, cutime, cstime;
    };

    CpuInfo m_lastCpuInfo;
    ProcessInfo m_lastProcessInfo;
    bool m_initialized;

    // Performance data collection
    int calculateFPS();
    bool readCpuInfo(CpuInfo &cpu);
    bool readProcessInfo(pid_t pid, ProcessInfo &proc);
    double calculateCpuUsage(const CpuInfo &prevCpu, const CpuInfo &currCpu,
                            const ProcessInfo &prevProc, const ProcessInfo &currProc);
    float getDVRCpuUsage();
    float getTotalCpuUsage();
    float getDVRMemoryUsage();
    float getTotalMemoryUsage();
    QString getElapsedTime();
};

#endif // SETTINGSMANAGER_H
