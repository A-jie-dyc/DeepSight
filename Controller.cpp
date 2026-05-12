#include "Controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    //创建业务对象
    m_cam = new CameraCapture;
    m_pre = new FramePreprocessor;
    m_ai = new AIAnalysis;

    //创建线程
    m_camThread = new QThread(this);
    m_preThread = new QThread(this);
    m_aiThread = new QThread(this);

    //放置对应线程
    m_cam->moveToThread(m_camThread);
    m_pre->moveToThread(m_preThread);
    m_ai->moveToThread(m_aiThread);

    connect(m_ai,&AIAnalysis::initModel,m_cam,&CameraCapture::openCamera);
    connect(m_cam,&CameraCapture::frameReady,m_pre,&FramePreprocessor::onFrameReady);
    connect(m_pre,&FramePreprocessor::AIInputReady,m_ai,&AIAnalysis::onAIInputReady);

    connect(m_camThread,&QThread::finished,m_cam,&QObject::deleteLater);
    connect(m_preThread,&QThread::finished,m_pre,&QObject::deleteLater);
    connect(m_aiThread,&QThread::finished,m_ai,&QObject::deleteLater);
}

void Controller::start()
{
    m_camThread->start();
    m_preThread->start();
    m_aiThread->start();

    //初始化AI模型
    QMetaObject::invokeMethod(m_ai,&AIAnalysis::initModel);
}

void Controller::stop()
{
    QMetaObject::invokeMethod(m_cam,&CameraCapture::stopCapture);

    m_camThread->quit();
    m_camThread->wait();

    m_preThread->quit();
    m_preThread->wait();

    m_aiThread->quit();
    m_aiThread->wait();
}

Controller::~Controller()
{
    stop();
}