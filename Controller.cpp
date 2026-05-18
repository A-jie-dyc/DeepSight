#include "Controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent}
{
    //创建业务对象
    m_camera = new CameraCapture;
    m_pre = new FramePreprocessor;
    m_ai = new AIAnalysis;
    m_post = new OutputPostProcessor;
    m_painter = new VisionPainter;
    m_provider = new FrameImageProvider(this);

    //创建线程
    m_camThread = new QThread(this);
    m_preThread = new QThread(this);
    m_aiThread = new QThread(this);
    m_postThread = new QThread(this);
    m_painThread = new QThread(this);

    //连接信号槽
    initConnections();

    //放置对应线程
    m_camera->moveToThread(m_camThread);
    m_pre->moveToThread(m_preThread);
    m_ai->moveToThread(m_aiThread);
    m_post->moveToThread(m_postThread);
    m_painter->moveToThread(m_painThread);

    //启动线程
    m_camThread->start();
    m_preThread->start();
    m_aiThread->start();
    m_postThread->start();
    m_painThread->start();
}

void Controller::start()
{
    emit runningChanged(true);

    if(m_modelReady)
        QMetaObject::invokeMethod(m_camera,&CameraCapture::openCamera);
    else
        QMetaObject::invokeMethod(m_ai,&AIAnalysis::initModel);
}

void Controller::stop()
{
    emit runningChanged(false);

    QMetaObject::invokeMethod(m_camera,&CameraCapture::stopCapture,Qt::BlockingQueuedConnection);
}

void Controller::initConnections()
{
    connect(m_ai,&AIAnalysis::modelReady,m_camera,&CameraCapture::openCamera,Qt::QueuedConnection);
    connect(m_ai,&AIAnalysis::modelReady,this,[this](){ m_modelReady = true; },Qt::QueuedConnection);

    connect(m_camera,&CameraCapture::frameReady,m_pre,&FramePreprocessor::onFrameReady,Qt::QueuedConnection);
    connect(m_pre,&FramePreprocessor::SendFrame,m_painter,&VisionPainter::ReceiveFrame,Qt::QueuedConnection);
    connect(m_pre,&FramePreprocessor::AIInputReady,m_ai,&AIAnalysis::onAIInputReady,Qt::QueuedConnection);
    connect(m_ai,&AIAnalysis::AIOutputReady,m_post,&OutputPostProcessor::onOutputReady,Qt::QueuedConnection);
    connect(m_post,&OutputPostProcessor::postProcessReady,m_painter,&VisionPainter::onPostProcessReady,Qt::QueuedConnection);
    connect(m_painter,&VisionPainter::paintReady,m_provider,&FrameImageProvider::onPaintReady,Qt::QueuedConnection);

    connect(this,&Controller::runningChanged,m_pre,&FramePreprocessor::setRunning);
    connect(this,&Controller::runningChanged,m_ai,&AIAnalysis::setRunning);
    connect(this,&Controller::runningChanged,m_post,&OutputPostProcessor::setRunning);
    connect(this,&Controller::runningChanged,m_painter,&VisionPainter::setRunning);
}

FrameImageProvider* Controller::getProvider()
{
    return m_provider;
}

Controller::~Controller()
{
    stop();


    m_camThread->quit();
    m_camThread->wait();

    m_preThread->quit();
    m_preThread->wait();

    m_aiThread->quit();
    m_aiThread->wait();

    m_postThread->quit();
    m_postThread->wait();

    m_painThread->quit();
    m_painThread->wait();

    m_provider->deleteLater();
    m_camera->deleteLater();
    m_pre->deleteLater();
    m_ai->deleteLater();
    m_post->deleteLater();
    m_painter->deleteLater();
}