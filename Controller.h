#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include "CameraCapture.h"
#include "FramePreprocessor.h"
#include "AIAnalysis.h"
#include "OutputPostProcessor.h"
#include "VisionPainter.h"
#include "FrameImageProvider.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    Q_INVOKABLE FrameImageProvider* getProvider();

private:
    CameraCapture *m_camera;
    FramePreprocessor *m_pre;
    AIAnalysis *m_ai;
    OutputPostProcessor *m_post;
    VisionPainter *m_painter;
    FrameImageProvider *m_provider;

    QThread *m_camThread;
    QThread *m_preThread;
    QThread *m_aiThread;
    QThread *m_postThread;
    QThread *m_painThread;

    bool m_modelReady = false;
};

#endif // CONTROLLER_H
