#include "MediaCapture.h"
#include <opencv2/core.hpp>
#include <QUrl>
#include <QDebug>

MediaCapture::MediaCapture(QObject *parent)
    : QObject{parent}
{}

//开启摄像头
void MediaCapture::openMedia(const QString &path)
{
    stopCapture();
    if(m_cap.isOpened())
        m_cap.release();

    int cameraId = path.toInt(&m_isCamera);

    if(m_isCamera) {
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
        if(!m_isCamera) {
            stopCapture();
            qDebug()<<"视频播放完毕";
        } else {
            qDebug()<<"摄像头帧为空，跳过";
        }
        return;
    }

    emit frameReady(frame.clone());
}

//开启
void MediaCapture::startCapture()
{
    if(!m_cap.isOpened())
        return;
    if(!m_timer) {
        m_timer = new QTimer(this);
        //设置精确计时器
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(m_timer,&QTimer::timeout,this,&MediaCapture::grabFrame);
    }
    if(!m_timer->isActive())
        m_timer->start(50);
    qDebug()<<"开始采集";
}

//关闭
void MediaCapture::stopCapture()
{
    if(!m_timer)
        return;
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