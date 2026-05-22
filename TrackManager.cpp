#include "TrackManager.h"

TrackManager::TrackManager(QObject *parent)
    : QObject{parent}
{}

void TrackManager::onPostProcessReady(const std::vector<DetectionBox> &rawBoxes)
{
    if(!m_isRunning)
        return;

    std::vector<DetectionBox> boxes = rawBoxes;
    std::vector<Track> newTracks;
    std::vector<bool> matched(boxes.size(),false);

    for(Track &oldTrack : m_tracks) {
        bool isFound = false;

        for(int i = 0; i < boxes.size(); i++) {
            if(matched[i])
                continue;
            const DetectionBox &box = boxes[i];
            cv::Point2f centerPos = getCenterPos(box);
            cv::Point2f predictPos = getPredictPos(centerPos,oldTrack.lastCenterPos);
            float w = oldTrack.box.x2 - oldTrack.box.x1;
            float h = oldTrack.box.y2 - oldTrack.box.y1;

            DetectionBox predictBox;
            predictBox.x1 = predictPos.x - w / 2;
            predictBox.y1 = predictPos.y - h / 2;
            predictBox.x2 = predictPos.x + w / 2;
            predictBox.y2 = predictPos.y + h / 2;

            float iouValue = iou(box, predictBox);

            if(iouValue > TRACK_IOU_THRES) {
                oldTrack.box = box;
                oldTrack.lastCenterPos = centerPos;
                oldTrack.predictCenterPos = predictPos;
                oldTrack.lostFrameCount = 0;

                newTracks.emplace_back(oldTrack);
                matched[i] = true;
                isFound = true;
                break;
            }
        }

        if(!isFound) {
            oldTrack.lostFrameCount++;
            if(oldTrack.lostFrameCount <= MAX_LOST_FRAMES) {
                newTracks.emplace_back(oldTrack);
            }
        }
    }
    for(int i = 0; i < boxes.size(); i++) {
        if(!matched[i]) {
            Track newTrack;
            newTrack.trackId = m_nextId++;
            newTrack.box = boxes[i];
            newTrack.lostFrameCount = 0;
            newTrack.lastCenterPos = getCenterPos(boxes[i]);
            newTrack.predictCenterPos = newTrack.lastCenterPos;

            newTracks.emplace_back(newTrack);
        }
    }
    m_tracks = newTracks;

    emit trackReady(m_tracks);
}

float TrackManager::iou(const DetectionBox &a, const DetectionBox &b)
{
    cv::Rect2f rectA(a.x1, a.y1, a.x2 - a.x1, a.y2 - a.y1);
    cv::Rect2f rectB(b.x1, b.y1, b.x2 - b.x1, b.y2 - b.y1);

    cv::Rect2f interRect = rectA & rectB;
    float interArea = interRect.area();
    float unionArea = rectA.area() + rectB.area() - interArea;

    return unionArea > 0 ? interArea / unionArea : 0.0f;
}

cv::Point2f TrackManager::getPredictPos(const cv::Point2f &now, const cv::Point2f &last)
{
    float px = now.x - last.x;
    float py = now.y - last.y;
    return {
        now.x + px * 0.5f,
        now.y + py * 0.5f
    };
}

cv::Point2f TrackManager::getCenterPos(const DetectionBox &box)
{
    return cv::Point2f((box.x1 + box.x2) * 0.5f,(box.y1 + box.y2) * 0.5f);
}