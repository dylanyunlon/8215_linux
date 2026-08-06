#include <QTime>
#include <QtMath>
#include "clusterapppainterdemo.h"

ClusterAppPainterDemo::ClusterAppPainterDemo(const QRect &showRect)
     : ClusterAppPainter(showRect)
     , m_radius(280)

{
    m_backgroundPixmap.load(":/images/demo/background_1920x720.png");
    m_mapPixmap.load(":/images/demo/map_608x408.png");
}


double ClusterAppPainterDemo::toPixels(double percentage)
{
    return m_radius * percentage;
}

void ClusterAppPainterDemo::drawProgress(double process, int lineWidth, int radius)
{
    QColor color1;
    QColor color2;
    QStringList colorNames = {"white", "green", "yellow", "red"};
    color1.setNamedColor(colorNames.at(int (process * 2.9)));
    color1.setAlpha(128);
    color2.setNamedColor(colorNames.at(int (process * 3.9)));
    color2.setAlpha(128);
    QLinearGradient linearGradient(-radius + lineWidth / 2, -radius + lineWidth / 2, 2 * radius - lineWidth, 2 * radius - lineWidth);
    linearGradient.setColorAt(0.0, color1);
    linearGradient.setColorAt(1.0, color2);
    gradientArc(radius, -125,  -290 * process, lineWidth, QBrush(linearGradient));
}

void ClusterAppPainterDemo::gradientArc(int radius, int startAngle, int angleLength, int arcHeight, const QBrush& brush)
{
    m_painter.setBrush(brush);

    QRectF rect(-radius, -radius, radius << 1, radius << 1);
    QPainterPath path;
    path.arcTo(rect, startAngle, angleLength);

    QPainterPath subPath;
    subPath.addEllipse(rect.adjusted(arcHeight, arcHeight, -arcHeight, -arcHeight));

    path -= subPath;

    m_painter.setPen(Qt::NoPen);
    m_painter.drawPath(path);
}

void ClusterAppPainterDemo::drawText(int value)
{
    QFont font;
    font.setPixelSize(toPixels(0.3));
    m_painter.setFont(font);
    m_painter.setPen(Qt::white);
    m_painter.drawText(0, 0, 150, 100, Qt::AlignCenter, QString::number(value));
}

void ClusterAppPainterDemo::drawMapPixmap(int width, int height)
{
    m_painter.drawPixmap(-width / 2, -height / 2, width, height, m_mapPixmap);
}


void ClusterAppPainterDemo::draw(QPaintDevice * device)
{
    static QPixmap pix;
    if (m_needScale) {
        pix = m_backgroundPixmap;
        m_painter.begin(&pix);
    } else {
        m_painter.begin(device);
        m_painter.drawPixmap(0, 0, m_backgroundPixmap);
    }

    checkFirstDraw();

    m_painter.setRenderHint(QPainter::SmoothPixmapTransform);
    m_painter.setRenderHint(QPainter::HighQualityAntialiasing);
    m_painter.setRenderHint(QPainter::TextAntialiasing);
    m_painter.save();
    drawInfo();

    //m_painter.translate(1920 / 2, 720 / 2);
    //drawMapPixmap(608, 408);
    //m_painter.restore();
    //m_painter.save();
    drawIVIInfo(760);
    drawUpdateInfo();

    m_painter.translate(1920 / 2, 20);
    drawIcon();
    m_painter.restore();

    //draw gauge needle
    m_painter.translate(m_radius + 60, 720 / 2 - 99.8);
    m_painter.save();
    drawNeeder(GAUGE_NEEDLE);

    //draw temperture needle
    m_painter.restore();
    m_painter.save();
    m_painter.translate(1240, 0);
    drawNeeder(TEMPERATURE_NEEDLE);

    //draw speed progress
    m_painter.restore();
    m_painter.translate(0, 99.8);
    m_painter.save();
    drawProgress(m_value->getSpeed() / 280.0, 56, 245);

    //draw speed needle
    drawNeeder(SPEED_NEEDLE);

    //draw speed text
    m_painter.restore();
    m_painter.save();
    m_painter.translate(-75, 25);
    drawText(m_value->getSpeed());

    //draw rpm needle
    m_painter.restore();
    m_painter.translate(1240, 0);
    m_painter.save();
    drawProgress(m_value->getRpm() / 8.0, 56, 245);
    drawNeeder(RPM_NEEDLE);

    //draw rpm text
    m_painter.restore();
    m_painter.translate(-75, 25);
    drawText(m_value->getRpm());

    m_painter.end();

    if (m_needScale) {
        m_painter.begin(device);
        m_painter.drawPixmap(m_showRect, pix);
        m_painter.end();
    }
}

void ClusterAppPainterDemo::drawNeeder(ENUM_NEEDLE needle)
{
    double needleLen = 220;
    double rotateAngle = 0;
    double scale = 1;
    switch (needle) {
    case SPEED_NEEDLE:
        needleLen = 260;
        rotateAngle = 35 + 290 / 280.0 * m_value->getSpeed();
        break;
    case RPM_NEEDLE:
        needleLen = 220;
        rotateAngle = 35 + 290 / 8.0 * m_value->getRpm();
        break;
    case GAUGE_NEEDLE:
        needleLen = 55;
        rotateAngle = 120 + 120 * m_value->getFuel();
        scale = 0.35;
        break;
    case TEMPERATURE_NEEDLE:
        needleLen = 55;
        rotateAngle = 120 + 120 * m_value->getTempareture();
        scale = 0.35;
        break;
    }

    QVector<QPoint> points = {QPoint(0, 0), QPoint(0, needleLen), QPoint(-2.5 * scale, needleLen), QPoint(-7.5 * scale, 0)};
    QVector<QPoint> points1 = {QPoint(0, 0), QPoint(0, needleLen), QPoint(2.5 * scale, needleLen), QPoint(7.5 * scale, 0)};
    QPolygon pts1(points);
    QPolygon pts2(points1);
    m_painter.rotate(rotateAngle);
    m_painter.setPen(Qt::NoPen);
    m_painter.setBrush(QColor(168, 0, 0, 168));
    m_painter.drawConvexPolygon(pts1);
    m_painter.setBrush(QColor(168, 0, 0, 100));
    m_painter.drawConvexPolygon(pts2);

    // draw needle hat
    QLinearGradient linearGrad(QPointF(-16 * scale, -16 * scale), QPointF(32 * scale, 32 * scale));
    linearGrad.setColorAt(0, Qt::gray);
    linearGrad.setColorAt(1, Qt::black);
    m_painter.setPen(Qt::NoPen);
    m_painter.setBrush(linearGrad);
    m_painter.drawEllipse(-16 * scale , -16 * scale, 32 * scale, 32 * scale);
}

void ClusterAppPainterDemo::drawIcon()
{
    int span = 10;
    int width = 60;
    int height = 60;

    int len = ICONNAME_List.size() * width + (ICONNAME_List.size() - 1) * span;
    m_painter.translate(-len / 2.0, 0);
    m_painter.save();

    QPen pen;
    pen.setWidth(10);
    pen.setColor(QColor(180, 180, 180, 255));
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::FlatCap);
    m_painter.setPen(pen);
    m_painter.translate(0, 80);
    QVector<QPointF> points1 = {QPointF(-100, -120), QPointF(0, 0), QPointF(len, 0), QPointF(len + 100, -120)};
    m_painter.drawPolygon(points1);

    m_painter.translate(0, 530);
    QVector<QPointF> points2 = {QPointF(-100, 120), QPointF(0, 0), QPointF(len, 0), QPointF(len + 100, 120)};
    m_painter.drawPolygon(points2);


    m_painter.translate(len / 2.0, 0);
    drawBottom();

    m_painter.restore();

    foreach (QString iconKey, ICONNAME_List) {
        QPixmap *pix = m_value->getIndicationIcon(iconKey);
        if (pix)
            m_painter.drawPixmap(QPoint(0,0), *pix);
        m_painter.translate(span + width, 0);
    }
}

void ClusterAppPainterDemo::drawBottom()
{
    QFont font;
    font.setPixelSize(toPixels(0.13));
    m_painter.setFont(font);
    m_painter.setPen(Qt::white);
    QTime curTime =QTime::currentTime();
    QString curTimeStr = curTime.toString("hh : mm");
    m_painter.translate(0, 25);

    QStringList gearList = {"P", "R", "N", "D"};
    for (int i = 0; i < gearList.size(); i++) {
        double scale = 1;
        if (m_value->getGearPosition() == i) {
            scale = 1.2;
        }
        QRect rect(-75 + i * 50, 0, 40 * scale, 40 * scale);
        font.setPixelSize(toPixels(0.13 * scale));
        m_painter.setFont(font);
        m_painter.setPen(Qt::white);
        m_painter.drawText(rect, Qt::AlignCenter, gearList[i]);

        if (scale > 1) {
            m_painter.setPen(Qt::NoPen);
            m_painter.setBrush(QColor(0, 250, 0, 128));
            m_painter.drawRect(rect);
        }
    }

    font.setPixelSize(toPixels(0.11));
    m_painter.setPen(Qt::white);
    m_painter.setFont(font);
    m_painter.drawText(-350, 0, 160, 40, Qt::AlignCenter, curTimeStr);
    m_painter.drawText(110, 0, 350, 40, Qt::AlignCenter, QString("%1 km").arg(m_value->getTotalMilage()));
}


