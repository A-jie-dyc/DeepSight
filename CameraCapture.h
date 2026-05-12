#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <atomic>

class CameraCapture : public QObject
{
    Q_OBJECT
public:
    explicit CameraCapture(QObject *parent = nullptr);
    ~CameraCapture() override;

public slots:
    void openCamera();
    void stopCapture();

signals:
    void frameReady(const QImage &img);
    void started();
    void stopped();

private:
    void captureLoop();
    cv::VideoCapture m_cap;     //摄像头对象
    std::atomic<bool> m_isRunning = false;
};

#endif // CAMERACAPTURE_H
