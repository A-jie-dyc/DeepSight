#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include <QObject>
#include "CommonDef.h"

class TrackManager : public QObject
{
    Q_OBJECT
public:
    explicit TrackManager(QObject *parent = nullptr);

    void setRunning(bool running) { m_isRunning.store(running); }

public slots:
    void onPostProcessReady(uint64_t frameId, const std::vector<DetectionBox> &rawBoxes);

signals:
    void trackReady(uint64_t frameId, const std::vector<Track> &tracks);

private:
    std::vector<Track> processTracks(const std::vector<DetectionBox> &boxes);
    float iou(const DetectionBox &a, const DetectionBox &b);
    cv::Point2f getCenterPos(const DetectionBox &box);

    std::vector<Track> m_tracks;
    int m_nextId = 1;

    std::atomic<bool> m_isRunning = false;
};

#endif // TRACKMANAGER_H
