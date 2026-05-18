#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QTimer>
#include <opencv2/opencv.hpp>

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

private:

    void grabFrame();
    cv::VideoCapture m_cap;     //摄像头对象
    QTimer *m_timer;
};

#endif // CAMERACAPTURE_H
