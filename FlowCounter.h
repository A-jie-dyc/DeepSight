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

public slots:
    void onTrackReady(const std::vector<Track> &rawTracks);
    void setRunning(bool running) { m_isRunning.store(running); }
    void resetCountData();

signals:
    void flowDataChanged(int enter, int current);

private:
    void calcPeopleFlow(const std::vector<Track> &rawTracks);
    int m_countLineY = 320;
    int m_enterTotal = 0;
    int m_currentPeople = 0;

    QMap<int, float> m_trackLastY;
    QSet<int> m_finishCountId;

    std::atomic<bool> m_isRunning = false;
};

#endif // FLOWCOUNTER_H
