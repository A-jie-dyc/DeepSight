#ifndef VISIONPAINTER_H
#define VISIONPAINTER_H

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <atomic>
#include "CommonDef.h"

class VisionPainter : public QObject
{
    Q_OBJECT
public:
    explicit VisionPainter(QObject *parent = nullptr);

public slots:
    void ReceiveFrame(const QImage &img);
    void onPostProcessReady(const std::vector<DetectionBox> &boxes);
    void setRunning(bool running) { m_isRunning = running; }

signals:
    void paintReady(const QImage &processImg);

private:
    void draw(const QImage &img,const std::vector<DetectionBox> &boxes);
    QImage m_drawImg;

    std::atomic<bool> m_isRunning = false;
};

#endif // VISIONPAINTER_H
