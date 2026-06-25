#ifndef FLOWCOUNTER_H
#define FLOWCOUNTER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <atomic>
#include "CommonDef.h"

class FlowCounter : public QObject
{
    Q_OBJECT
public:
    explicit FlowCounter(QObject *parent = nullptr);
    ~FlowCounter() override;

    void setRunning(bool running) { m_isRunning.store(running); }

public slots:
    void onTrackReady(uint64_t, const std::vector<Track> &rawTracks);
    void resetCountData();
    void setCountLine(const Line &line);

signals:
    void flowDataChanged(int enter, int current);

private:
    void calcPeopleFlow(const std::vector<Track> &rawTracks);
    float pointSide(const cv::Point2f &p, const Line &line);

    Line m_countLine;
    int m_enterTotal = 0;
    int m_currentPeople = 0;
    QMap<int, cv::Point2f> m_trackLastPos;
    QSet<int> m_finishCountId;
    std::atomic<bool> m_isRunning = false;
};

#endif // FLOWCOUNTER_H
