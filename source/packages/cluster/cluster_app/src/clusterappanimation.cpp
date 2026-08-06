#include "clusterappanimation.h"
#include "clusterappvalue.h"

ClusterAppAnimation::ClusterAppAnimation(QObject *parent) : QObject(parent)
{
    initAnimation();
}

void ClusterAppAnimation::initAnimation()
{
    m_group = new QSequentialAnimationGroup(this);
    m_group->addAnimation(getParallelAnimationGroup(120, 8, 150, 1000));
    m_group->addAnimation(getParallelAnimationGroup(0, 2, 100, 1000));
    m_group->addPause(500);
    m_group->addAnimation(getParallelAnimationGroup(30, 6.1, 80, 3000));
    m_group->addAnimation(getParallelAnimationGroup(26, 2.4, 40, 600));
    m_group->addAnimation(getParallelAnimationGroup(60, 4.3, 60, 3000));
    m_group->addAnimation(getParallelAnimationGroup(56, 3.9, 30, 600));
    m_group->addAnimation(getParallelAnimationGroup(100, 6.2, 20, 3000));
    m_group->addAnimation(getParallelAnimationGroup(96, 5.5,  80, 600));
    m_group->addAnimation(getParallelAnimationGroup(50, 3.4, 60, 3000));
    m_group->addAnimation(getParallelAnimationGroup(80, 6.2, 70, 3000));
    m_group->addAnimation(getParallelAnimationGroup(100, 6.3, 60, 5000));
    m_group->addAnimation(getParallelAnimationGroup(60, 7.2, 40, 3000));
    m_group->addAnimation(getParallelAnimationGroup(30, 5.6, 20, 2000));
    m_group->addAnimation(getParallelAnimationGroup(0, 0, 16, 1000));
    m_group->addPause(2000);
    m_group->setLoopCount(-1);
}

void ClusterAppAnimation::start()
{
    if (m_group->state() == QPropertyAnimation::Stopped) {
        m_group->start();
    }
}

void ClusterAppAnimation::stop()
{
    if (m_group->state() == QPropertyAnimation::Running) {
        m_group->stop();
    }
}

QPropertyAnimation *ClusterAppAnimation::getPropertyAnimation(double startValue, double endValue, double duration, const QByteArray &target)
{
    QPropertyAnimation *animation = new QPropertyAnimation(ClusterAppValue::getInstance(), target);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setDuration(duration);
    animation->setEasingCurve(QEasingCurve::InOutSine);

    return animation;
}

QParallelAnimationGroup *ClusterAppAnimation::getParallelAnimationGroup(double toSpeed, double toRpm, double toFuel, double duration)
{
    QParallelAnimationGroup *parallelAnimationGroup = new QParallelAnimationGroup(this);
    parallelAnimationGroup->addAnimation(getPropertyAnimation(m_startSpeed, toSpeed, duration, "speed"));
    parallelAnimationGroup->addAnimation(getPropertyAnimation(m_startRpm, toRpm, duration, "rpm"));
    parallelAnimationGroup->addAnimation(getPropertyAnimation(m_startFuel, toFuel, duration, "fuel"));

    m_startSpeed = toSpeed;
    m_startRpm = toRpm;
    m_startFuel = toFuel;

    return parallelAnimationGroup;
}
