#ifndef CLUSTERAPPTEST_H
#define CLUSTERAPPTEST_H
#include <QObject>
#include <QTimer>
#include "clusterappvalue.h"
#include "clusterappiviinfo.h"

#include <QElapsedTimer>
class ClusterAppAnimation;
class ClusterAppAnimationTest : public QObject
{
    Q_OBJECT
public:
    explicit ClusterAppAnimationTest();
    void start();
    void stop();

signals:
    void sigUpdateAnimation();

public slots:
    void slotUpdateValue();

private:
    QTimer m_timer;
    ClusterAppValue *m_value;
    ClusterAppIVIInfo *info;
    ClusterAppAnimation *m_animation;
    QElapsedTimer m_elaspedTimer;
    void testClusterAppIVIInfo();
    void setupStatusUpdater();
    QTimer *callStatusTimer = new QTimer();

};

#endif // CLUSTERAPPTEST_H
