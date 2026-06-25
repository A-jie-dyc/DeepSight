#ifndef VISIONPAINTER_H
#define VISIONPAINTER_H

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>
#include <atomic>
#include <QHash>
#include "CommonDef.h"

class VisionPainter : public QObject
{
    Q_OBJECT
public:
    explicit VisionPainter(QObject *parent = nullptr);
    //动态设置最大缓存帧
    static constexpr int RAW_MAX = (2 * 1000 + CAPTURE_INTERVAL_MS - 1) / CAPTURE_INTERVAL_MS;
    static constexpr int MAX_CACHE_FRAME = RAW_MAX < 5 ? 5 : (RAW_MAX > 100 ? 100 : RAW_MAX);
    void setRunning(bool running) { m_isRunning.store(running); }

public slots:
    void receiveFrame(uint64_t frameId, const cv::Mat &matForDraw);
    void onTrackReady(uint64_t frameId, const std::vector<Track> &rawTracks);
    void setCountLine(const Line &line) { m_countLine = line; }

signals:
    void paintReady(const QImage &processImg);

private:
    QImage convertMatToQImage(const cv::Mat &mat);
    void draw(QImage &drawImg,const std::vector<Track> &tracks);

    QHash<uint64_t, QImage> m_frameCache;
    Line m_countLine = {{0,0}, {0,0}, false, 1};
    std::atomic<bool> m_isRunning = false;
};

#endif // VISIONPAINTER_H
