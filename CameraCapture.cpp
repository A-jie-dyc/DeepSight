#include "CameraCapture.h"
#include <QDebug>

CameraCapture::CameraCapture(QObject *parent)
    : QObject{parent}
{
    connect(this,&CameraCapture::started,this,&CameraCapture::captureLoop);
}

//开启摄像头
void CameraCapture::openCamera()
{
    //防止二次开启
    if(m_isRunning) return;
    //打开默认摄像头
    if(!m_cap.open(0)) return;

    m_isRunning = true;
    emit started();
}

//抓图循环
void CameraCapture::captureLoop()
{
    cv::Mat frame;

    while(m_isRunning)
    {
        m_cap >> frame;
        if(frame.empty()) {
            QThread::msleep(10);
            continue;
        }
        //包装成QImage
        QImage img(frame.data,frame.cols,frame.rows,frame.step,QImage::Format_RGB888);

        emit frameReady(img.copy());
    }
    //关闭摄像头
    m_cap.release();
    emit stopped();
}

//关闭
void CameraCapture::stopCapture()
{
    m_isRunning=false;
}

CameraCapture::~CameraCapture()
{
    stopCapture();
}