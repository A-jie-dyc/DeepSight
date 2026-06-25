#include "Controller.h"
#include <QDateTime>

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

void Controller::initConnections()
{
    connect(m_ai, &AIAnalysis::modelReady,this,[this](){
        m_modelReady = true;
    }, Qt::QueuedConnection);

    connect(m_media, &MediaCapture::frameReady, m_pre, &FramePreprocessor::onFrameReady, Qt::QueuedConnection);
    connect(m_pre, &FramePreprocessor::previewFrameReady, this, &Controller::frameDeliver, Qt::QueuedConnection);
    connect(m_pre, &FramePreprocessor::sendFrame, m_painter, &VisionPainter::receiveFrame, Qt::QueuedConnection);
    connect(m_pre, &FramePreprocessor::AIInputReady, m_ai, &AIAnalysis::onAIInputReady, Qt::QueuedConnection);
    connect(m_ai, &AIAnalysis::AIOutputReady, m_post, &OutputPostprocessor::onOutputReady, Qt::QueuedConnection);
    connect(m_post, &OutputPostprocessor::postProcessReady,m_track, &TrackManager::onPostProcessReady, Qt::QueuedConnection);
    connect(m_track, &TrackManager::trackReady, m_painter, &VisionPainter::onTrackReady, Qt::QueuedConnection);
    connect(m_track, &TrackManager::trackReady, m_counter, &FlowCounter::onTrackReady, Qt::QueuedConnection);
    connect(m_painter, &VisionPainter::paintReady, this, &Controller::frameDeliver, Qt::QueuedConnection);
    connect(m_counter, &FlowCounter::flowDataChanged, this, [this](int enter, int people) {
        m_enterTotal = enter;
        m_currentPeople = people;
        emit flowDataUpdated();
    }, Qt::QueuedConnection);

    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setInterval(200);
    m_fpsTimer->setTimerType(Qt::PreciseTimer);
    connect(m_fpsTimer, &QTimer::timeout, this, &Controller::onFpsTimerTimeout);
    connect(this, &Controller::AIRunningChanged, this, [this](){
        if(m_AIRunning) {
            m_ai->resetAllCount();
            m_fpsElapsed.start();
            m_fpsTimer->start();
        }else {
            m_fpsTimer->stop();
        }
    });

    connect(this, &Controller::countLineChanged, m_counter,&FlowCounter::setCountLine, Qt::QueuedConnection);
    connect(this, &Controller::countLineChanged, m_painter,&VisionPainter::setCountLine, Qt::QueuedConnection);

    connect(m_media, &MediaCapture::errorOccurred, this, &Controller::handleError);
    connect(m_ai, &AIAnalysis::errorOccurred, this, &Controller::handleError);
}

void Controller::openCamera() { openMedia("0"); }
void Controller::openVideo(const QString &videoPath) { openMedia(videoPath); }

void Controller::openMedia(const QString &path)
{
    stop();
    QMetaObject::invokeMethod(m_media,"openMedia",Q_ARG(QString, path));
    m_mediaOpened = true;
    emit mediaOpenedChanged();
}

void Controller::startAI()
{
    if(!m_modelReady) {
        QMetaObject::invokeMethod(m_ai,&AIAnalysis::initModel,Qt::QueuedConnection);
        return;
    }
    if(!m_AIRunning) {
        m_counter->setRunning(true);
        m_painter->setRunning(true);
        m_track->setRunning(true);
        m_post->setRunning(true);
        m_ai->setRunning(true);
        m_pre->setRunning(true);
        m_pre->resetFrameId();
        m_AIRunning = true;
        emit AIRunningChanged();
    }
}

void Controller::stopAI()
{
    if(m_AIRunning) {
        m_pre->setRunning(false);
        m_ai->setRunning(false);
        m_post->setRunning(false);
        m_track->setRunning(false);
        m_painter->setRunning(false);
        m_counter->setRunning(false);
        m_AIRunning = false;
        emit AIRunningChanged();
    }
}

void Controller::start()
{
    if(!m_running) {
        startAI();
        QMetaObject::invokeMethod(m_media, &MediaCapture::startCapture, Qt::QueuedConnection);
        m_running = true;
        emit runningChanged();
    }
}

void Controller::stop()
{
    if(m_running) {
        QMetaObject::invokeMethod(m_media, &MediaCapture::stopCapture, Qt::BlockingQueuedConnection);
        stopAI();
        m_running = false;
        emit runningChanged();
    }
}

void Controller::closeMedia()
{
    stop();
    emit frameDeliver(QImage());
    resetFlowCount();
    m_mediaOpened = false;
    emit mediaOpenedChanged();
}

void Controller::onFpsTimerTimeout()
{
    FpsCount count = m_ai->fetchAndResetAllCount();
    //计算时间差
    qint64 elapsedMs = m_fpsElapsed.restart();
    float deltaSec = elapsedMs / 1000.0f;

    //防除零保护
    if(deltaSec < 0.001f) {
        return;
    }

    float instantInfer = static_cast<float>(count.inferFrame) / deltaSec;
    float instantTotal = static_cast<float>(count.totalFrame) / deltaSec;
    float instantDrop = (count.totalFrame > 0) ?
                            static_cast<float>(count.dropFrame) / static_cast<float>(count.totalFrame) * 100.0f
                            : 0.0f;

    //指数加权平滑（0.3旧值 + 0.7新值）
    float smoothedInfer = m_inferFps * 0.3f + instantInfer * 0.7f;
    float smoothedTotal = m_totalFps * 0.3f + instantTotal * 0.7f;
    float smoothedDrop = m_dropRate * 0.3f + instantDrop * 0.7f;

    float oldInferFps = m_inferFps;
    float oldTotalFps = m_totalFps;
    float oldDropRate = m_dropRate;
    m_inferFps = smoothedInfer;
    m_totalFps = smoothedTotal;
    m_dropRate = smoothedDrop;
    //变化超过阈值更新UI
    if(qAbs(oldInferFps - m_inferFps) > 0.1f) {
        emit inferFpsChanged();
    }
    if(qAbs(oldTotalFps - m_totalFps) > 0.1f) {
        emit totalFpsChanged();
    }
    if(qAbs(oldDropRate - m_dropRate) > 0.1f) {
        emit dropRateChanged();
    }
}

void Controller::handleError(ErrorDef::ErrorType type, const QString &msg)
{
    ErrorInfo err;
    err.errorType = type;
    err.errorMessage = msg;

    //构建完整的错误信息
    switch (type) {
    case ErrorDef::Error_Camera:
        err.errorLevel = ErrorDef::ErrorLevel_Critical;
        err.errorTitle = QStringLiteral("摄像头打开失败");
        err.errorSuggestion = QStringLiteral("请检查设备是否正常连接、是否被其他软件占用，可尝试插拔设备后重试");
        stop();
        break;
    case ErrorDef::Error_VideoOpen:
        err.errorLevel = ErrorDef::ErrorLevel_Critical;
        err.errorTitle = QStringLiteral("视频文件无法打开");
        err.errorSuggestion = QStringLiteral("请检查文件路径是否正确、文件是否损坏，或尝试更换其他视频文件");
        stop();
        break;
    case ErrorDef::Error_ModelLoad:
        err.errorLevel = ErrorDef::ErrorLevel_Critical;
        err.errorTitle = QStringLiteral("AI模型加载失败");
        err.errorSuggestion = QStringLiteral("请检查models目录下是否存在模型文件，或重新安装程序");
        m_modelReady = false;
        break;
    case ErrorDef::Error_LineInvalid:
        err.errorLevel = ErrorDef::ErrorLevel_Warning;
        err.errorTitle = QStringLiteral("统计线绘制无效");
        err.errorSuggestion = QStringLiteral("请按住鼠标左键拖动绘制更长的线段，确保跨越人员通行区域");
        break;
    default:
        err.errorLevel = ErrorDef::ErrorLevel_Critical;
        err.errorTitle = QStringLiteral("系统异常");
        err.errorSuggestion = QStringLiteral("请重启程序后重试，若问题持续请联系开发者");
        break;
    }

    m_lastError = err;
    emit errorHappened(err);
    emit errorInfoChanged();
}

void Controller::resetFlowCount()
{
    QMetaObject::invokeMethod(m_counter,&FlowCounter::resetCountData);
    m_enterTotal = 0;
    m_currentPeople = 0;
    emit flowDataUpdated();
}

int Controller::getEnterTotal() const { return m_enterTotal; }
int Controller::getCurrentPeople() const { return m_currentPeople; }
bool Controller::getRunning() const { return m_running; }
bool Controller::getAIRunning() const { return m_AIRunning; }
bool Controller::getMediaOpened() const { return m_mediaOpened; }
ErrorInfo Controller::lastError() const { return m_lastError; }
float Controller::getInferFps() const { return m_inferFps; }
float Controller::getTotalFps() const { return m_totalFps; }
float Controller::getDropRate() const { return m_dropRate; }

Controller::~Controller()
{
    if(m_fpsTimer && m_fpsTimer->isActive()) {
        m_fpsTimer->stop();
    }

    stop();

    m_media->deleteLater();
    m_pre->deleteLater();
    m_ai->deleteLater();
    m_post->deleteLater();
    m_track->deleteLater();
    m_painter->deleteLater();
    m_counter->deleteLater();

    m_mediaThread->quit();m_mediaThread->wait();
    m_preThread->quit();m_preThread->wait();
    m_aiThread->quit();m_aiThread->wait();
    m_postThread->quit();m_postThread->wait();
    m_trackThread->quit();m_trackThread->wait();
    m_painThread->quit();m_painThread->wait();
    m_countThread->quit();m_countThread->wait();

    delete m_mediaThread;
    delete m_preThread;
    delete m_aiThread;
    delete m_postThread;
    delete m_trackThread;
    delete m_painThread;
    delete m_countThread;
}