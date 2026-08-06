#ifndef CLUSTERAPPWINDOW_H
#define CLUSTERAPPWINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QPaintEvent>
#include <QPixmap>
#include <QVector>
#include <QKeyEvent>
#include "clusterapppainter.h"
#include "clusterappserviceadapter.h"
//#include "clustertransitionplayer.h"
#include "clusterappclireceiver.h"
#include <QTimer>
#include <QPainter>
#include <QRect>
#include <QTimer>
#include "clusterappTwoWheel.h"
#include "clusterappTwoWheelLargeScreen.h"

#include "globalbus.h"
#include "qobjlistener.h"
#include "appobj.h"

class ClusterAppAnimationTest;

class MediumManager : public CQObjListener
{
    Q_OBJECT
public:
    MediumManager();
    ~MediumManager();
    void setWindow(QWindow *topWindow);

public slots:
    int doShowFrontUI(void);
    int doExit (int param1, int param2);
};

class ClusterAppWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    enum FocusItem {
        FOCUS_NONE = 0,
        FOCUS_DVR,
        FOCUS_GPS,
        FOCUS_DIALER,
        FOCUS_SETTINGS
    };
    enum KeyAction {
        ACTION_NONE = 0,
        ACTION_LEFT   = Qt::Key_4,  // 52
        ACTION_RIGHT  = Qt::Key_6,  // 54
        ACTION_UP     = Qt::Key_2,  // 50
        ACTION_DOWN   = Qt::Key_5,  // 53

        ACTION_OK     = Qt::Key_1,  // 49
        ACTION_VOICE  = Qt::Key_3   // 54
    };

    ClusterAppWindow();
    ~ClusterAppWindow();
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void keyPressEvent(QKeyEvent * e);
    void switchTheme(int index = -1);

signals:
    void sigThemeSwitchChanged();
private:
    void initCLiReceiver();
    QRect getScaleRect(int screenWidth, int screenHeight);
    void addVolume();
    void decVolume();
    void setVolume(int value);
    void slotSwitchTestModel(bool flag);
    void onThemeSwitchChanged();
    void writeBootProf(const QString &text);
    void mousePressEvent(QMouseEvent *event);
    void moveFocusLeft();
    void moveFocusRight();
    void selectItem();

    ClusterAppServiceAdapter m_serviceAdapter;
    QVector<ClusterAppPainter*> m_painters;
    int m_currentThemeIndex = 0;
    QPixmap m_pixmap;
    QPainter m_painter;
 //   ClusterTransitionlayer *m_transitionPlayer;
    ClusterAppAnimationTest *m_aninationTest;
    ClusterAppConfigure *m_config;
    ClusterAppCLIReceiver *m_cliReceiver;
    QTimer m_timer;
    bool m_manualTest = false;
    bool m_firsetDraw = true;
    bool m_thmemSwitching = false;
    QRect m_showRect;
    clusterappTwoWheelLargeScreen* twoWheelLarge;
    clusterappTwoWheel* twoWheel;

    MediumManager m_mediumManager;

};
#endif // CLUSTERAPPWINDOW_H
