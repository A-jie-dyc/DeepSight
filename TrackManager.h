#ifndef TRACKMANAGER_H
#define TRACKMANAGER_H

#include <QObject>
#include "CommonDef.h"

class TrackManager : public QObject
{
    Q_OBJECT
public:
    explicit TrackManager(QObject *parent = nullptr);

public slots:
    void onPostProcessReady(const std::vector<DetectionBox> &rawBoxes);
    void setRunning(bool running) { m_isRunning.store(running); }

signals:
    void trackReady(const std::vector<Track> &tracks);

private:
    std::vector<Track> processTracks(const std::vector<DetectionBox> &boxes);
    float iou(const DetectionBox &a, const DetectionBox &b);
    cv::Point2f getCenterPos(const DetectionBox &box);

    std::vector<Track> m_tracks;
    int m_nextId = 1;

    std::atomic<bool> m_isRunning = false;
};

#endif // TRACKMANAGER_H
