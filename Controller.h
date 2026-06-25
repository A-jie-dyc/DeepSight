#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include "MediaCapture.h"
#include "FramePreprocessor.h"
#include "AIAnalysis.h"
#include "OutputPostprocessor.h"
#include "TrackManager.h"
#include "VisionPainter.h"
#include "FlowCounter.h"

class Controller : public QObject
{
    Q_OBJECT
    //属性
    Q_PROPERTY(int enterTotal READ getEnterTotal NOTIFY flowDataUpdated)
    Q_PROPERTY(int currentPeople READ getCurrentPeople NOTIFY flowDataUpdated)
    Q_PROPERTY(bool running READ getRunning NOTIFY runningChanged)
    Q_PROPERTY(bool AIRunning READ getAIRunning NOTIFY AIRunningChanged)
    Q_PROPERTY(bool mediaOpened READ getMediaOpened NOTIFY mediaOpenedChanged)
    Q_PROPERTY(ErrorInfo lastError READ lastError NOTIFY errorInfoChanged)
    Q_PROPERTY(float inferFps READ getInferFps NOTIFY inferFpsChanged)
    Q_PROPERTY(float totalFps READ getTotalFps NOTIFY totalFpsChanged)
    Q_PROPERTY(float dropRate READ getDropRate NOTIFY dropRateChanged)

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    //接口
    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void openVideo(const QString &videoPath);
    Q_INVOKABLE void closeMedia();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void startAI();
    Q_INVOKABLE void stopAI();
    Q_INVOKABLE void resetFlowCount();

    void handleError(ErrorDef::ErrorType type, const QString &msg);

    int getEnterTotal() const;
    int getCurrentPeople() const;
    bool getRunning() const;
    bool getAIRunning() const;
    bool getMediaOpened() const;
    ErrorInfo lastError() const;
    float getInferFps() const;
    float getTotalFps() const;
    float getDropRate() const;

signals:
    void flowDataUpdated();
    void runningChanged();
    void AIRunningChanged();
    void mediaOpenedChanged();
    void errorInfoChanged();
    void frameDeliver(const QImage &image);
    void countLineChanged(const Line &line);
    void errorHappened(const ErrorInfo &errorInfo);
    void inferFpsChanged();
    void totalFpsChanged();
    void dropRateChanged();

private slots:
    void onFpsTimerTimeout();

private:
    void initConnections();
    void openMedia(const QString &path);

    MediaCapture *m_media;
    FramePreprocessor *m_pre;
    AIAnalysis *m_ai;
    OutputPostprocessor *m_post;
    TrackManager *m_track;
    VisionPainter *m_painter;
    FlowCounter *m_counter;

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
    bool m_running = false;
    bool m_AIRunning = false;
    bool m_mediaOpened = false;

    ErrorInfo m_lastError;

    QTimer *m_fpsTimer = nullptr;
    QElapsedTimer m_fpsElapsed;
    float m_inferFps = 0.0f;
    float m_totalFps = 0.0f;
    float m_dropRate = 0.0f;
};

#endif // CONTROLLER_H
