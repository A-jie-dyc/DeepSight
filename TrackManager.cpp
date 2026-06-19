#include "TrackManager.h"

TrackManager::TrackManager(QObject *parent)
    : QObject{parent}
{}

std::vector<Track> TrackManager::processTracks(const std::vector<DetectionBox> &boxes)
{
    std::vector<Track> newTracks;
    std::vector<bool> matched(boxes.size(), false);

    for (Track &oldTrack : m_tracks) {
        // 基于跟踪器自身速度预测当前位置
        float vx = oldTrack.lastCenterPos.x - oldTrack.predictCenterPos.x;
        float vy = oldTrack.lastCenterPos.y - oldTrack.predictCenterPos.y;
        cv::Point2f predictCenter(oldTrack.lastCenterPos.x + vx,
                                  oldTrack.lastCenterPos.y + vy);

        float w = oldTrack.box.x2 - oldTrack.box.x1;
        float h = oldTrack.box.y2 - oldTrack.box.y1;

        DetectionBox predictBox;
        predictBox.x1 = predictCenter.x - w / 2;
        predictBox.y1 = predictCenter.y - h / 2;
        predictBox.x2 = predictCenter.x + w / 2;
        predictBox.y2 = predictCenter.y + h / 2;

        // 在所有未匹配的检测框中找最优 IOU 匹配
        float bestIou = TRACK_IOU_THRES;
        int bestIdx = -1;
        cv::Point2f bestCenter;
        for (int i = 0; i < (int)boxes.size(); i++) {
            if (matched[i])
                continue;
            float iouValue = iou(boxes[i], predictBox);
            if (iouValue > bestIou) {
                bestIou = iouValue;
                bestIdx = i;
                bestCenter = getCenterPos(boxes[i]);
            }
        }

        if (bestIdx >= 0) {
            // 匹配成功：更新跟踪器状态
            oldTrack.box = boxes[bestIdx];
            oldTrack.predictCenterPos = oldTrack.lastCenterPos;
            oldTrack.lastCenterPos = bestCenter;
            oldTrack.lostFrameCount = 0;
            newTracks.emplace_back(oldTrack);
            matched[bestIdx] = true;
        } else {
            // 失配：计数并决定是否保留
            oldTrack.lostFrameCount++;
            if (oldTrack.lostFrameCount <= MAX_LOST_FRAMES) {
                newTracks.emplace_back(oldTrack);
            }
        }
    }

    // 为未匹配的检测框创建新跟踪器
    for (int i = 0; i < (int)boxes.size(); i++) {
        if (!matched[i]) {
            Track newTrack;
            newTrack.trackId = m_nextId++;
            newTrack.box = boxes[i];
            newTrack.lostFrameCount = 0;
            cv::Point2f center = getCenterPos(boxes[i]);
            newTrack.lastCenterPos = center;
            newTrack.predictCenterPos = center; // 初始速度 = 0

            newTracks.emplace_back(newTrack);
        }
    }

    return newTracks;
}

void TrackManager::onPostProcessReady(uint64_t frameId, const std::vector<DetectionBox> &rawBoxes)
{
    if(!m_isRunning)
        return;

    m_tracks = processTracks(rawBoxes);

    emit trackReady(frameId, m_tracks);

    if(m_nextId > 300)
        m_nextId = 1;
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

cv::Point2f TrackManager::getCenterPos(const DetectionBox &box)
{
    return cv::Point2f((box.x1 + box.x2) * 0.5f,(box.y1 + box.y2) * 0.5f);
}