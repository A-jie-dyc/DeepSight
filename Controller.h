#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include "CameraCapture.h"
#include "FramePreprocessor.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

private:
    CameraCapture *m_cam;
    FramePreprocessor *m_pre;

    QThread *m_camThread;
    QThread *m_preThread;
};

#endif // CONTROLLER_H
