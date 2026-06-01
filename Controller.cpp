#include "Controller.h"

Controller::Controller(QObject *parent)
    : QObject{parent},
    m_enterTotal(0),
    m_currentPeople(0)
{
    //创建业务对象
    m_media = new MediaCapture;
    m_pre = new FramePreprocessor;
    m_ai = new AIAnalysis;
    m_post = new OutputPostprocessor;
    m_track = new TrackManager;
    m_painter = new VisionPainter;
    m_counter = new FlowCounter;
    m_provider = new FrameImageProvider(this);

    //创建线程
    m_mediaThread = new QThread;
    m_preThread = new QThread;
    m_aiThread = new QThread;
    m_postThread = new QThread;
    m_trackThread = new QThread;
    m_painThread = new QThread;
    m_countThread = new QThread;

    //连接信号槽
    initConnections();

    //放置对应线程
    m_media->moveToThread(m_mediaThread);
    m_pre->moveToThread(m_preThread);
    m_ai->moveToThread(m_aiThread);
    m_post->moveToThread(m_postThread);
    m_track->moveToThread(m_trackThread);
    m_painter->moveToThread(m_painThread);
    m_counter->moveToThread(m_countThread);

    //启动线程
    m_mediaThread->start();
    m_preThread->start();
    m_aiThread->start();
    m_postThread->start();
    m_trackThread->start();
    m_painThread->start();
    m_countThread->start();

    QMetaObject::invokeMethod(m_ai,&AIAnalysis::initModel,Qt::QueuedConnection);
}

void Controller::openCamera()
{
    stop();
    QMetaObject::invokeMethod(m_media,"openMedia",Q_ARG(QString, "0"));
}

void Controller::openVideo(const QString &videoPath)
{
    stop();
    QMetaObject::invokeMethod(m_media,"openMedia",Q_ARG(QString, videoPath));
}

void Controller::start()
{
    emit runningChanged(true);

    if(m_modelReady)
        QMetaObject::invokeMethod(m_media,&MediaCapture::startCapture,Qt::QueuedConnection);
    else
        QMetaObject::invokeMethod(m_ai,&AIAnalysis::initModel,Qt::QueuedConnection);
}

void Controller::stop()
{
    emit runningChanged(false);

    QMetaObject::invokeMethod(m_media,&MediaCapture::stopCapture,Qt::BlockingQueuedConnection);
}

void Controller::initConnections()
{
    connect(m_ai,&AIAnalysis::modelReady,this,[this](){ m_modelReady = true; },Qt::QueuedConnection);

    connect(m_media,&MediaCapture::frameReady,m_pre,&FramePreprocessor::onFrameReady,Qt::QueuedConnection);
    connect(m_pre,&FramePreprocessor::sendFrame,m_painter,&VisionPainter::receiveFrame,Qt::QueuedConnection);
    connect(m_pre,&FramePreprocessor::AIInputReady,m_ai,&AIAnalysis::onAIInputReady,Qt::QueuedConnection);
    connect(m_ai,&AIAnalysis::AIOutputReady,m_post,&OutputPostprocessor::onOutputReady,Qt::QueuedConnection);
    connect(m_post,&OutputPostprocessor::postProcessReady,m_track,&TrackManager::onPostProcessReady,Qt::QueuedConnection);
    connect(m_track,&TrackManager::trackReady,m_painter,&VisionPainter::onTrackReady,Qt::QueuedConnection);
    connect(m_track,&TrackManager::trackReady,m_counter,&FlowCounter::onTrackReady,Qt::QueuedConnection);
    connect(m_painter,&VisionPainter::paintReady,m_provider,&FrameImageProvider::onPaintReady,Qt::QueuedConnection);
    connect(m_counter,&FlowCounter::flowDataChanged,this,[this](int enter, int people){
        m_enterTotal = enter;
        m_currentPeople = people;
        emit flowDataUpdated();
    });

    connect(this,&Controller::runningChanged,m_pre,&FramePreprocessor::setRunning,Qt::QueuedConnection);
    connect(this,&Controller::runningChanged,m_ai,&AIAnalysis::setRunning,Qt::QueuedConnection);
    connect(this,&Controller::runningChanged,m_post,&OutputPostprocessor::setRunning,Qt::QueuedConnection);
    connect(this,&Controller::runningChanged,m_track,&TrackManager::setRunning,Qt::QueuedConnection);
    connect(this,&Controller::runningChanged,m_painter,&VisionPainter::setRunning,Qt::QueuedConnection);
    connect(this,&Controller::runningChanged,m_counter,&FlowCounter::setRunning,Qt::QueuedConnection);
}

void Controller::resetFlowCount()
{
    QMetaObject::invokeMethod(m_counter,&FlowCounter::resetCountData);
    m_enterTotal = 0;
    m_currentPeople = 0;
    emit flowDataUpdated();
}

int Controller::getEnterTotal() const
{
    return m_enterTotal;
}

int Controller::getCurrentPeople() const
{
    return m_currentPeople;
}

FrameImageProvider* Controller::getProvider()
{
    return m_provider;
}

Controller::~Controller()
{
    stop();

    m_mediaThread->quit();m_mediaThread->wait();
    m_preThread->quit();m_preThread->wait();
    m_aiThread->quit();m_aiThread->wait();
    m_postThread->quit();m_postThread->wait();
    m_trackThread->quit();m_trackThread->wait();
    m_painThread->quit();m_painThread->wait();
    m_countThread->quit();m_countThread->wait();

    delete m_media;
    delete m_pre;
    delete m_ai;
    delete m_post;
    delete m_track;
    delete m_painter;
    delete m_counter;

    delete m_mediaThread;
    delete m_preThread;
    delete m_aiThread;
    delete m_postThread;
    delete m_trackThread;
    delete m_painThread;
    delete m_countThread;
}