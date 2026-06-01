#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include "MediaCapture.h"
#include "FramePreprocessor.h"
#include "AIAnalysis.h"
#include "OutputPostprocessor.h"
#include "TrackManager.h"
#include "VisionPainter.h"
#include "FlowCounter.h"
#include "FrameImageProvider.h"

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int enterTotal READ getEnterTotal NOTIFY flowDataUpdated)
    Q_PROPERTY(int currentPeople READ getCurrentPeople NOTIFY flowDataUpdated)

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void openVideo(const QString &videoPath);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE FrameImageProvider* getProvider();
    Q_INVOKABLE void resetFlowCount();

    int getEnterTotal() const;
    int getCurrentPeople() const;

signals:
    void runningChanged(bool running);
    void flowDataUpdated();

private:
    void initConnections();
    MediaCapture *m_media;
    FramePreprocessor *m_pre;
    AIAnalysis *m_ai;
    OutputPostprocessor *m_post;
    TrackManager *m_track;
    VisionPainter *m_painter;
    FlowCounter *m_counter;
    FrameImageProvider *m_provider;

    QThread *m_mediaThread;
    QThread *m_preThread;
    QThread *m_aiThread;
    QThread *m_postThread;
    QThread *m_trackThread;
    QThread *m_painThread;
    QThread *m_countThread;

    int m_enterTotal = 0;
    int m_currentPeople = 0;

    bool m_modelReady = false;
};

#endif // CONTROLLER_H
