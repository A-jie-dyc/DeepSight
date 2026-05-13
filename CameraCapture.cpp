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
    //防止二次开启
    if(m_isRunning) return;
    //打开默认摄像头
    if(!m_cap.open(0)) return;

    qDebug()<<"启动摄像头";
    m_isRunning = true;
    m_timer->start(30);
}

//抓帧
void CameraCapture::grabFrame()
{
    if(!m_isRunning || !m_cap.isOpened())
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
    if(!m_isRunning)
        return;
    m_timer->stop();
    m_isRunning=false;
    if(m_cap.isOpened())
        m_cap.release();
    qDebug()<<"摄像头已关闭";
}

CameraCapture::~CameraCapture()
{
    stopCapture();
    m_timer->deleteLater();
}