#include "MediaCapture.h"
#include <QUrl>
#include <QDebug>

MediaCapture::MediaCapture(QObject *parent)
    : QObject{parent}
    ,m_timer(new QTimer(this))
{
    connect(m_timer,&QTimer::timeout,this,&MediaCapture::grabFrame);
    //设置精确计时器
    m_timer->setTimerType(Qt::PreciseTimer);
}

//开启摄像头
void MediaCapture::openMedia(const QString &path)
{
    if(m_cap.isOpened())
        m_cap.release();

    bool isCamera;
    int cameraId = path.toInt(&isCamera);

    if(isCamera) {
        if(!m_cap.open(cameraId, cv::CAP_DSHOW)) {
            qDebug()<<"摄像头状态异常";
            return;
        }
    }
    else {
        QString localPath = QUrl(path).toLocalFile();
        if(!m_cap.open(localPath.toStdString())) {
            qDebug()<<"视频打开失败";
            return;
        }
    }
    qDebug()<<"媒体流设置成功";
}

//抓帧
void MediaCapture::grabFrame()
{
    if(!m_cap.isOpened())
        return;

    cv::Mat frame;
    m_cap >> frame;

    if(frame.empty()) {
        stopCapture();
        qDebug()<<"视频播放完毕/摄像头断开";
        return;
    }

    emit frameReady(frame);
}

//开启
void MediaCapture::startCapture()
{
    if(!m_cap.isOpened() || m_timer->isActive())
        return;
    m_timer->start(50);
    qDebug()<<"开始采集";
}

//关闭
void MediaCapture::stopCapture()
{
    if(m_timer->isActive()) {
        m_timer->stop();
        qDebug()<<"已暂停采集";
    }
}

MediaCapture::~MediaCapture()
{
    stopCapture();
    if(m_cap.isOpened())
        m_cap.release();
}