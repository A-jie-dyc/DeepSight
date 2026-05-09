#include "Controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    //创建业务对象
    m_cam = new CameraCapture;
    m_pre = new FramePreprocessor;

    //创建线程
    m_camThread = new QThread(this);
    m_preThread = new QThread(this);

    //放置对应线程
    m_cam->moveToThread(m_camThread);
    m_pre->moveToThread(m_preThread);

    connect(m_cam,&CameraCapture::frameReady,m_pre,&FramePreprocessor::onFrameReady);

    connect(m_camThread,&QThread::finished,m_cam,&QObject::deleteLater);
    connect(m_preThread,&QThread::finished,m_pre,&QObject::deleteLater);
}

void Controller::start()
{
    m_camThread->start();
    m_preThread->start();

    QMetaObject::invokeMethod(m_cam,&CameraCapture::openCamera);
}

void Controller::stop()
{
    QMetaObject::invokeMethod(m_cam,&CameraCapture::stopCapture);

    m_camThread->quit();
    m_camThread->wait();

    m_preThread->quit();
    m_preThread->wait();
}

Controller::~Controller()
{
    stop();
}