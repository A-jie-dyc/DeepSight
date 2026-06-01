#include "FlowCounter.h"
#include <QDebug>

FlowCounter::FlowCounter(QObject *parent)
    : QObject{parent},
    m_enterTotal(0),
    m_currentPeople(0),
    m_countLineY(320)
{}

void FlowCounter::calcPeopleFlow(const std::vector<Track> &rawTracks)
{
    m_currentPeople = 0;

    for(const auto &track : rawTracks) {
        if(track.lostFrameCount > MAX_LOST_FRAMES)
            continue;

        m_currentPeople++;

        if(!m_trackLastY.contains(track.trackId)) {
            m_trackLastY[track.trackId] = track.lastCenterPos.y;
            continue;
        }

        float lastY = m_trackLastY[track.trackId];
        m_trackLastY[track.trackId] = track.lastCenterPos.y;

        if(m_finishCountId.contains(track.trackId))
            continue;

        if(lastY <= m_countLineY && track.lastCenterPos.y > m_countLineY) {
            m_enterTotal++;
            m_finishCountId.insert(track.trackId);
        }
    }
    QList<int> aliveIds;
    for(const auto &t : rawTracks)
        aliveIds << t.trackId;
    m_trackLastY.removeIf([&](const std::pair<const int, float> &pair) { return !aliveIds.contains(pair.first); });
    m_finishCountId.removeIf([&](int id) { return !aliveIds.contains(id); });
}

void FlowCounter::onTrackReady(const std::vector<Track> &rawTracks)
{
    if(!m_isRunning)
        return;
    calcPeopleFlow(rawTracks);
    emit flowDataChanged(m_enterTotal, m_currentPeople);
}

void FlowCounter::resetCountData()
{
    m_enterTotal = 0;
    m_currentPeople = 0;
    m_trackLastY.clear();
    m_finishCountId.clear();
}

FlowCounter::~FlowCounter()
{
    resetCountData();
}