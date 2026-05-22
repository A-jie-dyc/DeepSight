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
    void setRunning(bool running) { m_isRunning = running; }

signals:
    void trackReady(const std::vector<Track> &tracks);

private:
    float iou(const DetectionBox &a, const DetectionBox &b);
    cv::Point2f getCenterPos(const DetectionBox &box);
    cv::Point2f getPredictPos(const cv::Point2f &now, const cv::Point2f &last);

    std::vector<Track> m_tracks;
    int m_nextId = 1;

    std::atomic<bool> m_isRunning = false;
};

#endif // TRACKMANAGER_H
