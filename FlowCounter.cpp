#include "FlowCounter.h"
#include <QDebug>

FlowCounter::FlowCounter(QObject *parent)
    : QObject{parent},
    m_enterTotal(0),
    m_currentPeople(0)
{}

void FlowCounter::calcPeopleFlow(const std::vector<Track> &rawTracks)
{
    m_currentPeople = 0;

    if(!m_countLine.isValid)
        return;

    for(const auto &track : rawTracks) {
        if(track.lostFrameCount > MAX_LOST_FRAMES)
            continue;

        m_currentPeople++;
        cv::Point2f currentPos = track.lastCenterPos;

        if(!m_trackLastPos.contains(track.trackId)) {
            m_trackLastPos[track.trackId] = currentPos;
            continue;
        }

        cv::Point2f lastPos = m_trackLastPos[track.trackId];
        m_trackLastPos[track.trackId] = currentPos;

        if(m_finishCountId.contains(track.trackId))
            continue;

        //叉乘法检测
        float lastSide = pointSide(lastPos, m_countLine);
        float currentSide = pointSide(currentPos, m_countLine);
        if(lastSide * currentSide < 0) {
            if(currentSide * m_countLine.direction > 0) {
                m_enterTotal++;
                m_finishCountId.insert(track.trackId);
            }
        }
    }
    //清理丢失目标
    QList<int> aliveIds;
    for(const auto &t : rawTracks)
        aliveIds << t.trackId;

    m_trackLastPos.removeIf([&](const std::pair<const int, cv::Point2f> &pair) { return !aliveIds.contains(pair.first); });
    m_finishCountId.removeIf([&](int id) { return !aliveIds.contains(id); });
}

void FlowCounter::onTrackReady(uint64_t, const std::vector<Track> &rawTracks)
{
    if(!m_isRunning)
        return;
    calcPeopleFlow(rawTracks);
    emit flowDataChanged(m_enterTotal, m_currentPeople);
}

float FlowCounter::pointSide(const cv::Point2f &p, const Line &line)
{
    return (line.end.x - line.start.x) * (p.y - line.start.y)
           - (line.end.y - line.start.y) * (p.x - line.start.x);
}

void FlowCounter::resetCountData()
{
    m_enterTotal = 0;
    m_currentPeople = 0;
    m_trackLastPos.clear();
    m_finishCountId.clear();
}

void FlowCounter::setCountLine(const Line &line)
{
    m_countLine = line;
    resetCountData();
    qDebug()<<"统计线已更新：起点（"<<line.start.x<<","<<line.start.y<<"）"
             <<"终点 （"<<line.end.x<<","<<line.end.y<<"）";
}

FlowCounter::~FlowCounter()
{
    resetCountData();
}