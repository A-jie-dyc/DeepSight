#ifndef FRAMEDISPLAY_H
#define FRAMEDISPLAY_H

#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>
#include <QImage>
#include <QMutex>

class FrameDisplay : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit FrameDisplay(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

public slots:
    void updateFrame(const QImage &image);

private:
    QImage m_image;
    QMutex m_mutex;
};

#endif // FRAMEDISPLAY_H