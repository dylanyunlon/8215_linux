#ifndef CLUSTERAPPPERFORMANCE_H
#define CLUSTERAPPPERFORMANCE_H
#include <QStringList>
#include "libsysinfo.h"
class ClusterAppPerformance
{
public:
    static ClusterAppPerformance *getInstance();
    QStringList getPerformance();

private:
    uint getFps();

    QString getFileContent(const QString &path);
    ClusterAppPerformance();
    static ClusterAppPerformance *s_instance;
};

#endif // CLUSTERAPPPERFORMANCE_H
