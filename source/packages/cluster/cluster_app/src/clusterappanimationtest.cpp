#include "clusterappanimationtest.h"
#include "clusterappconstant.h"
#include "clusterappanimation.h"
#include <QTime>
#include "clusterappupdateinfo.h"
#include <QScopedPointer>


ClusterAppAnimationTest::ClusterAppAnimationTest()
    : m_value(ClusterAppValue::getInstance()), info(ClusterAppIVIInfo::getInstance())
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotUpdateValue()));
    m_animation = new ClusterAppAnimation(this);
    connect(&m_timer, &QTimer::timeout, this, &ClusterAppAnimationTest::slotUpdateValue);
    testClusterAppIVIInfo();
    setupStatusUpdater();
}

void ClusterAppAnimationTest::start()
{
    m_timer.start(TIME_UPDATE);
    m_animation->start();
}

void ClusterAppAnimationTest::stop()
{
    m_timer.stop();
    m_animation->stop();
}

void ClusterAppAnimationTest::slotUpdateValue()
{
    static int count = 0;
    count++;
    static double value = 0;
    static double valuedelta = 0.0025;
    value += valuedelta;

    if (value > 1) {
        value = 1;
        valuedelta = -valuedelta;
    } else if (value < 0) {
        value = 0;
        valuedelta = -valuedelta;
    }

   // m_value->setFuel(value);
    m_value->setTempareture(value);

    if (count > 200) {
        count = 0;
        for (int i = 1; i < ICONNAME_List.size() - 1; ++i) {
            m_value->setIndicationStatus(ICONNAME_List[i], qrand() % 2);
        }

        for (int i = 0; i < LIGHT_List.size(); ++i) {
            m_value->setLightStatus(LIGHT_List[i], qrand() % 2);
        }

        for (int i = 1; i < SMALLICONNAME_List.size() - 1; ++i) {
            m_value->setIndicationStatus(SMALLICONNAME_List[i], qrand() % 2);
        }

        for (int i = 0; i < SMALLLIGHT_List.size(); ++i) {
            m_value->setLightStatus(SMALLLIGHT_List[i], qrand() % 2);
        }

        m_value-> setGear(qrand() % 6);

        int s = m_value->getCardoorStatus();
        m_value->setCardoorStatus(++s);
        s % 2 ? m_value->setIndicationStatus(ICON_TRUN_LEFT, qrand() % 2)
              : m_value->setIndicationStatus(ICON_TRUN_RIGHT, qrand() % 2);

        s % 2 ? m_value->setIndicationStatus(SMALL_ICON_TRUN_LEFT, qrand() % 2)
              : m_value->setIndicationStatus(SMALL_ICON_TRUN_RIGHT, qrand() % 2);

        if (s > 15) {
           m_value->setCardoorStatus(0);
        }

        if (m_value->getTotalMilage() < 999999)
            m_value->setTotalMilage(m_value->getTotalMilage() + 1);
    }

    //ivi info
}

void ClusterAppAnimationTest::testClusterAppIVIInfo()
{
/*

    info->setPlayStatus(ClusterAppIVIInfo::Playing);
    info->setMediaName("Bohemian Rhapsody");
    info->setMediaPixmap(QPixmap(100, 100));

    info->setCallStatus(ClusterAppIVIInfo::Incoming);
    info->setCallPersonName("John Doe");
  //  info->setPersonPixmap(QPixmap(50, 50));
    info->setCallNumber("+1234567890");
    info->setCallTime("00:30");

    info->setNavgation(true);
    info->setDirectionStatus(ClusterAppIVIInfo::GOHEAD);
    **/
}

void ClusterAppAnimationTest::setupStatusUpdater() {
    ClusterAppIVIInfo *minfo = ClusterAppIVIInfo::getInstance();
    QObject::connect(callStatusTimer, &QTimer::timeout, [minfo]() {
        int randomsNumber = qrand() % 3 + 1;
        if(randomsNumber == 1) {
            /**
            int newCallStatus = qrand() % 3 + 1;
            minfo->setCallStatus(newCallStatus);
            minfo->setNavgation(false);
            minfo->setPlayStatus(ClusterAppIVIInfo::Stop);
            minfo->setCallNumber("+1234567890");
            minfo->setCallTime("00:30");
            minfo->setCallPersonName("John Doe");
**/
        }else if(randomsNumber == 2) {
            minfo->setNavgation(true);
            minfo->setDirectionStatus(ClusterAppIVIInfo::DirectionStatus(qrand() % 3));
         //   minfo->setCallStatus(ClusterAppIVIInfo::Idle);
         //   minfo->setPlayStatus(ClusterAppIVIInfo::Stop);
        }else {
            /**
            minfo->setPlayStatus(ClusterAppIVIInfo::Playing);
            minfo->setMediaName("Bohemian Rhapsody");
            minfo->setNavgation(false);
            minfo->setCallStatus(ClusterAppIVIInfo::Idle);
            **/
        }

    });

    callStatusTimer->start(1200);
}







