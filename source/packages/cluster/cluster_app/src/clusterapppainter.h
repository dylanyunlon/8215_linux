#ifndef CLUSTERAPPPAINTER_H
#define CLUSTERAPPPAINTER_H
#include <QPaintDevice>
#include <QPixmap>
#include <QPainter>
#include "clusterappiviinfo.h"
#include "clusterappvalue.h"
#include "clusterapppixmapmanager.h"
#include "clusterappperformance.h"
#include "clusterappconfigure.h"
#include <QVector>

class ClusterAppPainter
{
public:
    enum BottomFocus {
        FOCUS_NONE,
        FOCUS_DVR,
        FOCUS_GPS,
        FOCUS_DIALER,
        FOCUS_SETTINGS
    };

    ClusterAppPainter(const QRect &showRect);
    virtual void draw(QPaintDevice *device) = 0;
    virtual void drawBackground(QPaintDevice *device) {}
    bool checkFirstDraw();
    void drawInfo();
    void drawIVIInfo(int x, int yoff = 0);
    void drawCarLine(int x1, int y1, int x2, int y2);
    void drawCarDoorStatus(int x, int y);
    void drawUpdateInfo();
    void drawNavInfo(int x, int yoff);
    void drawFocusPath(const QPainterPath& path, bool focused);
    int m_focus = FOCUS_DVR;

private:
    double getFps();
    void drawCallInfo(int x, int yoff);
    void drawMusicInfo(int x, int yoff);

protected:
    QRect m_showRect;
    QPixmap m_backgroundPixmap;
    QPixmap m_pointerPixmap;
    QPixmap m_pointerShadePixmap;
    QPixmap m_pointerShadePixmap2;
    QPixmap m_transformedPixmap;

    ClusterAppIVIInfo *m_iviInfo = nullptr;
    ClusterAppValue *m_value  = nullptr;
    ClusterAppPixmapManager *m_pixmapManager  = nullptr;
    ClusterAppPerformance *m_performance  = nullptr;
    ClusterAppConfigure *m_configure  = nullptr;

    QPainter m_painter;
    bool m_needScale = false;
    QFont font;


};

#endif // CLUSTERAPPPAINTER_H
