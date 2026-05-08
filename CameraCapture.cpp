#include "CameraCapture.h"

CameraCapture::CameraCapture(QObject *parent)
    : QObject{parent}
{}

//开启摄像头
void CameraCapture::openCamera()
{
    //防止二次开启
    if(m_isRunning) return;
    //打开默认摄像头
    if(!m_cap.open(0)) return;

    m_isRunning = true;

    //创建线程并启动
    m_captureThread = new QThread;
    this->moveToThread(m_captureThread);
    connect(m_captureThread,&QThread::started,this,&CameraCapture::captureLoop);
    m_captureThread->start();

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

        //opencv BGR->Qt RGB
        cv::cvtColor(frame,frame,cv::COLOR_BGR2RGB);
        //Mat->QImage
        QImage img(frame.data,frame.cols,frame.rows,frame.step,QImage::Format_RGB888);

        emit frameReady(img.copy());
        QThread::msleep(30);
    }
    //关闭摄像头
    m_cap.release();
    emit stopped();
}

//关闭
void CameraCapture::stopCapture()
{
    m_isRunning=false;
    //清理线程
    if(m_captureThread) {
        m_captureThread->quit();
        m_captureThread->wait();
        m_captureThread->deleteLater();
        m_captureThread = nullptr;
    }
}

CameraCapture::~CameraCapture()
{
    stopCapture();
}