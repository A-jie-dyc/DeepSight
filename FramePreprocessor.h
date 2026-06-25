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

    void setRunning(bool running) { m_isRunning.store(running); }
    void resetFrameId();

public slots:
    void onFrameReady(const cv::Mat &rawMat);       //接收

signals:
    void sendFrame(uint64_t frameId, const cv::Mat &matForDraw);
    void AIInputReady(uint64_t frameId, const cv::Mat &matForAI, const PreprocessParams &params);
    void previewFrameReady(const QImage &frame);

private:
    void preProcess(const cv::Mat &mat);     //预处理
    QImage matToQImage(const cv::Mat &rgbMat);

    cv::Mat m_matForDraw;
    cv::Mat m_matForAI;
    PreprocessParams m_params;
    std::atomic<uint64_t> m_frameId = 0;
    std::atomic<bool> m_isRunning = false;
};

#endif // FRAMEPREPROCESSOR_H
