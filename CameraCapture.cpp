#include "CameraCapture.h"
#include <QDebug>

CameraCapture::CameraCapture(QObject *parent)
    : QObject{parent}
    ,m_timer(new QTimer(this))
{
    connect(m_timer,&QTimer::timeout,this,&CameraCapture::grabFrame);
    //设置精确计时器
    m_timer->setTimerType(Qt::PreciseTimer);
}

//开启摄像头
void CameraCapture::openCamera()
{
    QMutexLocker lock(&m_mutex);
    if(!m_cap.isOpened()) {
        if(!m_cap.open(0, cv::CAP_DSHOW)) {
            qDebug()<<"摄像头状态异常";
            return;
        }
    }
    qDebug()<<"启动摄像头";
    m_timer->start(30);
}

//抓帧
void CameraCapture::grabFrame()
{
    QMutexLocker lock(&m_mutex);
    if(!m_cap.isOpened())
        return;

    cv::Mat frame;
    m_cap >> frame;

    if(frame.empty())
        return;
    //包装成QImage
    QImage img(frame.data,frame.cols,frame.rows,frame.step,QImage::Format_RGB888);

    emit frameReady(img.copy());
}

//关闭
void CameraCapture::stopCapture()
{
    QMutexLocker lock(&m_mutex);
    if(m_timer->isActive())
        m_timer->stop();

    qDebug()<<"摄像头已暂停";
}

CameraCapture::~CameraCapture()
{
    stopCapture();
    if(m_cap.isOpened())
        m_cap.release();
}