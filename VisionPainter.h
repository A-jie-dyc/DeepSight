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
    void receiveFrame(const cv::Mat &matForDraw);
    void onTrackReady(const std::vector<Track> &rawTracks);
    void setRunning(bool running) { m_isRunning.store(running); }

signals:
    void paintReady(const QImage &processImg);

private:
    QImage convertMatToQImage(const cv::Mat &mat);
    void draw(QImage &drawImg,const std::vector<Track> &tracks);

    QImage m_currentImg;
    std::atomic<bool> m_isRunning = false;
};

#endif // VISIONPAINTER_H
