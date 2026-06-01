#ifndef MEDIACAPTURE_H
#define MEDIACAPTURE_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QTimer>
#include <QString>
#include <opencv2/opencv.hpp>

class MediaCapture : public QObject
{
    Q_OBJECT
public:
    explicit MediaCapture(QObject *parent = nullptr);
    ~MediaCapture() override;

public slots:
    void openMedia(const QString &path);
    void startCapture();
    void stopCapture();

signals:
    void frameReady(const cv::Mat &mat);

private:
    void grabFrame();
    cv::VideoCapture m_cap;     //摄像头对象
    QTimer *m_timer;
};

#endif // MEDIACAPTURE_H
