#ifndef CLUSTERAPPICON_H
#define CLUSTERAPPICON_H

#include <QObject>
#include <QTimer>

class ClusterAppIcon : public QObject
{
public:
    explicit ClusterAppIcon(const QString& name);
    void init();
    QString getName() const;
    QPixmap* getPixmap() const;
    virtual void setStatus(bool status);
    bool getStatus() const;

protected:
    bool m_status;
    QString m_name;
    QString m_path;
    QPixmap *m_statusOnPixmap = nullptr;
    QPixmap *m_statusOffPixmap = nullptr;
    QPixmap *m_pixmap = nullptr;
};

class ClusterAppFlashIcon : public ClusterAppIcon
{
    Q_OBJECT
public:
    explicit ClusterAppFlashIcon(const QString& name);
    void setStatus(bool status);
signals:

private slots:
    void slotFlash();
private:
    QTimer m_flashTimer;
    bool m_flashing;
};


#endif // CLUSTERAPPICON_H
