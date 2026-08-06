#ifndef CLUSTERAPPANIMATION_H
#define CLUSTERAPPANIMATION_H

#include <QObject>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class ClusterAppAnimation : public QObject
{
    Q_OBJECT
public:
    explicit ClusterAppAnimation(QObject *parent = nullptr);
    void initAnimation();
    void start();
    void stop();

private:
    QPropertyAnimation *getPropertyAnimation(double startValue, double endValue, double duration, const QByteArray &target);
    QParallelAnimationGroup *getParallelAnimationGroup(double toSpeed, double toRpm, double toFuel, double duration);
signals:

public slots:

private:
    double m_startSpeed = 0;
    double m_startRpm = 1;
    double m_startFuel = 1;
    QSequentialAnimationGroup *m_group;
};

#endif // CLUSTERAPPANIMATION_H
