#include "clusterappwindow.h"
#include "clusterappanimationtest.h"
#include "clusterappTwoWheel.h"
#include "clusterCarplayTwoWheel.h"

#include "clusterapppainterdemo.h"
#include <fstream>
#include <QGuiApplication>
#include <iostream>
#include <QScreen>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "atcbootanicom.h"

#include "clog.h"
#include <QThread>
#ifdef ATC_SHOW_LOGO_BOOT
#include "BootAnimationDrv.h"
#endif

#include <QGuiApplication>
#include "GlobalScreenConfig.h"

using cluster_utils::CLog;

const static char *TAG = "ClusterAppWindow";
ClusterAppWindow::ClusterAppWindow()
    : m_config(ClusterAppConfigure::getInstance())
{
   setFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    int windowWidth = 1024;
    int windowHeight = 600;

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect rect = screen->availableGeometry();
    if (screen) {
        windowWidth = rect.width();
        windowHeight = rect.height();
    }

    connect(this, &ClusterAppWindow::frameSwapped, [this] () {
        update();
    });

    connect(this, &ClusterAppWindow::sigThemeSwitchChanged, this, &ClusterAppWindow::onThemeSwitchChanged);

    m_aninationTest = new ClusterAppAnimationTest();
    m_aninationTest->start();

   // initCLiReceiver();
    UTILS_LOGI(TAG, "windowWidth = %d, windowHeight =  %d",windowWidth, windowHeight);
    m_showRect = QRect(0, 0, windowWidth, windowHeight);
    if (windowWidth == 1024) {
        twoWheelLarge = new clusterappTwoWheelLargeScreen(m_showRect);
        m_painters.append(twoWheelLarge);
        GlobalScreenConfig::setLargeScreen(true);
    }else if (windowWidth == 800) {
        twoWheel = new clusterappTwoWheel(m_showRect);
        m_painters.append(twoWheel);
        GlobalScreenConfig::setLargeScreen(false);
    }

    UTILS_LOGD(TAG, "start setWindow");
    m_mediumManager.setWindow(this);
    UTILS_LOGD(TAG, "end setWindow");

  //  m_serviceAdapter.start();
}

ClusterAppWindow::~ClusterAppWindow()
{

}

void ClusterAppWindow::initializeGL()
{
    initializeOpenGLFunctions();
}

void ClusterAppWindow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

//play music rgb source
void ClusterAppWindow::paintGL()
{
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_firsetDraw) {
        writeBootProf("cluster app first draw start");
        m_painters[m_currentThemeIndex]->drawBackground(this);
        //m_painters[m_currentThemeIndex]->draw(this);
        m_firsetDraw = false;
        writeBootProf("cluster app first draw finshed");

        QThread *thread = new QThread();
        connect(thread, &QThread::started, [=](){
/*#ifdef ATC_SHOW_LOGO_BOOT
                int fd = OpenAnimationDrv();
                SendAnimationMsg(fd);
                CloseAnimationDriver(fd);
                writeBootProf("cluster app SendAnimationMsg");
#else
               // QThread::msleep(300);
                AniComm *aniComm = new AniComm();
                if (NULL != aniComm) {
                    if (!aniComm->sendCommand(ANI_COMM_CMD_EXIT, true)) {
                        //LOGE(TAG, "Notify Bootanimation to exit fail\n");
                    }
                    writeBootProf("cluster app send ANI_COMM_CMD_EXIT");
                    delete aniComm;
                    aniComm = NULL;
                }
#endif*/
                thread->quit();
            });
            thread->start();

    } else {
        m_painters[m_currentThemeIndex]->draw(this);
    }
    /**
    QOpenGLContext *context = QOpenGLContext::currentContext();
    context->swapBuffers(this);
    **/
}

void ClusterAppWindow::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    UTILS_LOGI(TAG, "mousePressEvent");
    if (GlobalScreenConfig::getLargeScreen()) {
        if (twoWheelLarge->m_dvrRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_DVR , 0);
        } else if (twoWheelLarge->m_gpsRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CARBIT_APP , 0);
        } else if (twoWheelLarge->m_dialerRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_BT , 0);
        } else if (twoWheelLarge->m_settingsRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            //GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_SETTING , 0);
        }
    }else {
        if (twoWheel->m_dvrRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_DVR , 0);
        } else if (twoWheel->m_gpsRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CARBIT_APP , 0);
        } else if (twoWheel->m_dialerRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_BT , 0);
        } else if (twoWheel->m_settingsRect.contains(pos)) {
            GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            //GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_SETTING , 0);
        }
    }
}

void ClusterAppWindow::keyPressEvent(QKeyEvent *e)
{
    UTILS_LOGI(TAG, "keyPressEvent %d", e->key());
    KeyAction action = KeyAction(e->key());

    switch (action) {

    case ACTION_LEFT:
        moveFocusLeft();
        break;

    case ACTION_RIGHT:
        moveFocusRight();
        break;

    case ACTION_UP:
        // 可扩展
        break;

    case ACTION_DOWN:
        // 可扩展
        break;

    case ACTION_OK:
        selectItem();
        break;

    case ACTION_VOICE:
      //  triggerSiri();
        break;

    default:
        break;
    }


    return QOpenGLWindow::keyPressEvent(e);
}

void ClusterAppWindow::switchTheme(int index)
{
    if (!m_thmemSwitching) {
        int oldIndex = m_currentThemeIndex;
        if (index == -1) {
            ++m_currentThemeIndex;
        } else if (index != m_currentThemeIndex) {
            m_currentThemeIndex = index;
        }

        if (oldIndex != m_currentThemeIndex) {
            m_thmemSwitching = true;
            m_serviceAdapter.stopIVIProjection();
          //  m_transitionPlayer->start();
            if (m_painters.size() < 2) {
                 m_painters.append(new clusterCarplayTwoWheel(m_showRect));
            }
            if (m_currentThemeIndex >= m_painters.size()) {
                m_currentThemeIndex = 0;
            }
        }
        m_thmemSwitching = false;
        //m_transitionPlayer->waitShowing(1000);
    }
}

void ClusterAppWindow::initCLiReceiver()
{
    m_cliReceiver = new ClusterAppCLIReceiver();
    connect(m_cliReceiver, &ClusterAppCLIReceiver::sigSwitchTestMode, this, &ClusterAppWindow::slotSwitchTestModel);
    connect(m_cliReceiver, &ClusterAppCLIReceiver::sigSetVolume, this, &ClusterAppWindow::setVolume);
    connect(m_cliReceiver, &ClusterAppCLIReceiver::sigCallControl, [this] (bool flag) {
        flag ? m_serviceAdapter.call() : m_serviceAdapter.hangup();
    });

    connect(m_cliReceiver, &ClusterAppCLIReceiver::sigMusicControl, [this] (int cmd) {
        enum {PlayPause = 1, MusicPre = 2, MusicNext = 3};
        if (cmd == PlayPause) {
            m_serviceAdapter.playpause();
        } else if (cmd == MusicPre) {
            m_serviceAdapter.musicPre();
        } else if (cmd == MusicNext) {
            m_serviceAdapter.musicNext();
        }
    });

    connect(m_cliReceiver, &ClusterAppCLIReceiver::sigSwitchTheme, [this] (int theme) {
        switchTheme(theme);
    });
}

QRect ClusterAppWindow::getScaleRect(int screenWidth, int screenHeight)
{
    double startPosX = 0;
    double startPosY = 0;
    double scale = 0;
    const int WIDTH = 800;
    const int HEIGHT = 480;
    //Case 1: Synchronous scaling, scaling to scale smaller
    if ((screenWidth <= WIDTH && screenHeight <= HEIGHT) || (screenWidth >= WIDTH && screenHeight >= HEIGHT)) {

        if ((double)screenWidth / WIDTH <= (double)screenHeight / HEIGHT) {
            scale = (double)screenWidth / WIDTH;
            startPosY = fabs(screenHeight - HEIGHT * scale ) / 2.0;
        } else {
            scale = (double)screenHeight / HEIGHT;
            startPosX = fabs(screenWidth - WIDTH * scale) / 2.0;
        }

        //Case 2: Zoom in while zooming in, zoom out to scale
    } else {
        if (screenWidth <= WIDTH && screenHeight >= HEIGHT) {
            scale = (double)screenWidth / WIDTH;
            startPosY = fabs(screenHeight - HEIGHT * scale ) / 2.0;
        } else {
            scale = (double)screenHeight / HEIGHT;
            startPosX = fabs(screenWidth - WIDTH * scale) / 2.0;
        }
    }

    return QRect(startPosX, startPosY, WIDTH * scale, HEIGHT * scale);
}

void ClusterAppWindow::addVolume()
{
    int value = m_config->getVolume();
    if (value < 20) {
        ++value;
        setVolume(value);
    }
}

void ClusterAppWindow::decVolume()
{
    int value = m_config->getVolume();
    if (value > 0) {
        --value;
        setVolume(value);
    }
}

void ClusterAppWindow::setVolume(int value)
{

}

void ClusterAppWindow::slotSwitchTestModel(bool flag)
{
 //   UTILS_LOGD(TAG, "switch test model %d", flag);
    if (m_manualTest != flag) {
        m_manualTest = flag;
        if (flag) {
            m_aninationTest->stop();
            ClusterAppValue::getInstance()->reset();
        } else {
            m_aninationTest->start();
        }
    }
}
void ClusterAppWindow::onThemeSwitchChanged()
{
    if (m_currentThemeIndex == 0)
        m_serviceAdapter.startIVIProjection(720, 103, 480, 264);
    else if (m_currentThemeIndex == 1)
        m_serviceAdapter.startIVIProjection(1350, 179, 480, 264);
    else if (m_currentThemeIndex == 2)
        m_serviceAdapter.startIVIProjection(656, 171, 608, 384);

    m_thmemSwitching = false;
}

void ClusterAppWindow::writeBootProf(const QString &text)
{
    int fd = open("/proc/bootprof", O_CREAT | O_RDWR | O_APPEND);
    if (fd < 0) {
     //   syslog(LOG_ERR, "open /proc/bootprof failed!\n");
        return;
    }
    write(fd, text.toStdString().c_str(), text.length());
    ::close(fd);
}

MediumManager::MediumManager(): CQObjListener(CAPPBaseObj::APPID_CLUSTER)
{

}

MediumManager::~MediumManager()
{

}

void MediumManager::setWindow(QWindow *topWindow)
{
    initListener(topWindow, NULL, NULL, NULL);
}

int MediumManager::doExit (int param1, int param2)
{
    UTILS_LOGI(TAG, "doExit");

    return 1;
}

int MediumManager::doShowFrontUI(void)
{
    UTILS_LOGI(TAG, "doShowFrontUI");

    int ret = CQObjListener::doShowFrontUI();

    return ret;
}

void ClusterAppWindow::moveFocusLeft()
{
    if (m_painters[m_currentThemeIndex]->m_focus == FOCUS_DVR) {
        m_painters[m_currentThemeIndex]->m_focus = FOCUS_SETTINGS;
    } else {
        m_painters[m_currentThemeIndex]->m_focus = static_cast<FocusItem>(m_painters[m_currentThemeIndex]->m_focus - 1);
    }

    update();
}

void ClusterAppWindow::moveFocusRight()
{
    if (m_painters[m_currentThemeIndex]->m_focus == FOCUS_SETTINGS) {
        m_painters[m_currentThemeIndex]->m_focus = FOCUS_DVR;
    } else {
        m_painters[m_currentThemeIndex]->m_focus = static_cast<FocusItem>(m_painters[m_currentThemeIndex]->m_focus + 1);
    }

    update();
}

void ClusterAppWindow::selectItem()
{
    switch (m_painters[m_currentThemeIndex]->m_focus) {

    case FOCUS_DVR:
        GlobalBus::applyFor(GlobalBus::ACTION_RUN,
                            CAPPBaseObj::APPID_DVR, 0);
        break;

    case FOCUS_GPS:
        GlobalBus::applyFor(GlobalBus::ACTION_RUN,
                            CAPPBaseObj::APPID_CARPLAY_APP, 0);
        GlobalBus::applyFor(GlobalBus::ACTION_RUN,
                            CAPPBaseObj::APPID_CARBIT_APP, 0);
        break;

    case FOCUS_DIALER:
        GlobalBus::applyFor(GlobalBus::ACTION_RUN,
                            CAPPBaseObj::APPID_BT, 0);
        break;

    case FOCUS_SETTINGS:
        GlobalBus::applyFor(GlobalBus::ACTION_RUN,
                            CAPPBaseObj::APPID_CARPLAY_APP, 0);
        // 或者 SETTINGS
        break;

    default:
        break;
    }
}


