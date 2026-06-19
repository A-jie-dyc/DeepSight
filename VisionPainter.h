#ifndef VISIONPAINTER_H
#define VISIONPAINTER_H

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <atomic>
#include <QHash>
#include <QMutex>
#include "CommonDef.h"

class VisionPainter : public QObject
{
    Q_OBJECT
public:
    explicit VisionPainter(QObject *parent = nullptr);
    //最大缓存
    static constexpr int MAX_CACHE_FRAME = 5;

public slots:
    void receiveFrame(uint64_t frameId, const cv::Mat &matForDraw);
    void onTrackReady(uint64_t frameId, const std::vector<Track> &rawTracks);
    void setRunning(bool running) { m_isRunning.store(running); }
    void setCountLine(const Line &line) { m_countLine = line; }

signals:
    void paintReady(const QImage &processImg);

private:
    QImage convertMatToQImage(const cv::Mat &mat);
    void draw(QImage &drawImg,const std::vector<Track> &tracks);

    QHash<uint64_t, QImage> m_frameCache;
    QMutex m_mutex;
    Line m_countLine {{0,0}, {0,0}, false, 1};
    std::atomic<bool> m_isRunning = false;
};

#endif // VISIONPAINTER_H
