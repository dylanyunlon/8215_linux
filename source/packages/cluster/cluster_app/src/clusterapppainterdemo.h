#ifndef CLUSTERAPPPAINTER_1920X720_H
#define CLUSTERAPPPAINTER_1920X720_H
#include "clusterapppainter.h"

class ClusterAppPainterDemo : public ClusterAppPainter
{
public:
    explicit ClusterAppPainterDemo(const QRect &showRect);
    void draw(QPaintDevice * paintDevice);

private:
    double toPixels(double percentage);
    void drawProgress(double process, int lineWidth, int radius);
    void drawText(int value);
    void drawMapPixmap(int width, int height);
    void gradientArc(int radius, int startAngle, int angleLength, int arcHeight, const QBrush& brush);

    void drawNeeder(ENUM_NEEDLE needle);
    void drawIcon();
    void drawBottom();

    int m_radius;
    QPixmap m_mapPixmap;
};

#endif // CLUSTERAPPPAINTER_1920X720_H
