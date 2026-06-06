#ifndef FRAMEPREPROCESSOR_H
#define FRAMEPREPROCESSOR_H

#include <QObject>
#include <QImage>
#include <atomic>
#include "CommonDef.h"

class FramePreprocessor : public QObject
{
    Q_OBJECT
public:
    explicit FramePreprocessor(QObject *parent = nullptr);

public slots:
    void onFrameReady(const cv::Mat &rawMat);       //接收
    void setRunning(bool running) { m_isRunning.store(running); }

signals:
    void sendFrame(const cv::Mat &matForDraw);
    void AIInputReady(const cv::Mat &matForAI, const PreprocessParams &params);

private:
    void preProcess(const cv::Mat &mat);     //预处理

    cv::Mat m_matForDraw;
    cv::Mat m_matForAI;
    PreprocessParams m_params;
    std::atomic<bool> m_isRunning = false;
};

#endif // FRAMEPREPROCESSOR_H
