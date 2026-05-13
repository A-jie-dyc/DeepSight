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
    m_provider = new FrameImageProvider;

    //创建线程
    m_camThread = new QThread(this);
    m_preThread = new QThread(this);
    m_aiThread = new QThread(this);
    m_postThread = new QThread(this);
    m_painThread = new QThread(this);

    //放置对应线程
    m_camera->moveToThread(m_camThread);
    m_pre->moveToThread(m_preThread);
    m_ai->moveToThread(m_aiThread);
    m_post->moveToThread(m_postThread);
    m_painter->moveToThread(m_painThread);

    connect(m_ai,&AIAnalysis::modelReady,m_camera,&CameraCapture::openCamera);
    connect(m_camera,&CameraCapture::frameReady,m_pre,&FramePreprocessor::onFrameReady);
    connect(m_pre,&FramePreprocessor::SendFrame,m_painter,&VisionPainter::ReceiveFrame);
    connect(m_pre,&FramePreprocessor::AIInputReady,m_ai,&AIAnalysis::onAIInputReady);
    connect(m_ai,&AIAnalysis::AIOutputReady,m_post,&OutputPostProcessor::onOutputReady);
    connect(m_post,&OutputPostProcessor::postProcessReady,m_painter,&VisionPainter::onPostProcessReady);
    connect(m_painter,&VisionPainter::paintReady,m_provider,&FrameImageProvider::onPaintReady);

    connect(m_camThread,&QThread::finished,m_camera,&QObject::deleteLater);
    connect(m_preThread,&QThread::finished,m_pre,&QObject::deleteLater);
    connect(m_aiThread,&QThread::finished,m_ai,&QObject::deleteLater);
    connect(m_postThread,&QThread::finished,m_post,&QObject::deleteLater);
    connect(m_painThread,&QThread::finished,m_painter,&QObject::deleteLater);
}

void Controller::start()
{
    m_camThread->start();
    m_preThread->start();
    m_aiThread->start();
    m_postThread->start();
    m_painThread->start();

    //初始化AI模型
    QMetaObject::invokeMethod(m_ai,&AIAnalysis::initModel);
}

void Controller::stop()
{
    QMetaObject::invokeMethod(m_camera,&CameraCapture::stopCapture);

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
}

FrameImageProvider* Controller::getProvider()
{
    return m_provider;
}

Controller::~Controller()
{
    stop();
}