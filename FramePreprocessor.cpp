#include "FramePreprocessor.h"
#include <opencv2/opencv.hpp>

FramePreprocessor::FramePreprocessor(QObject *parent)
    : QObject{parent}
{}

void FramePreprocessor::preProcess(const cv::Mat &mat)
{
    //BGR->RGB
    cv::cvtColor(mat,m_matForDraw,cv::COLOR_BGR2RGB);

    // Letterbox: 等比例缩放 + 灰色填充，保持宽高比
    float scale = std::min((float)MODEL_WIDTH / m_matForDraw.cols,
                           (float)MODEL_HEIGHT / m_matForDraw.rows);
    int newW = (int)(m_matForDraw.cols * scale);
    int newH = (int)(m_matForDraw.rows * scale);

    cv::Mat resized;
    cv::resize(m_matForDraw, resized, cv::Size(newW, newH));

    int padLeft = (MODEL_WIDTH - newW) / 2;
    int padTop  = (MODEL_HEIGHT - newH) / 2;

    cv::Mat letterboxed(MODEL_HEIGHT, MODEL_WIDTH, CV_8UC3, cv::Scalar(114, 114, 114));
    resized.copyTo(letterboxed(cv::Rect(padLeft, padTop, newW, newH)));

    m_params.scale = scale;
    m_params.padLeft = padLeft;
    m_params.padTop = padTop;

    //缩放尺寸+归一化+数据排序转换,输出4维Mat适配ONNX
    m_matForAI = cv::dnn::blobFromImage(
        letterboxed,
        1.0f / NORMALIZE_DIVISOR,
        cv::Size(MODEL_WIDTH, MODEL_HEIGHT),
        cv::Scalar(),
        false,
        false,
        CV_32F
    );
}

void FramePreprocessor::onFrameReady(const cv::Mat &rawMat)
{
    if(rawMat.empty() || !m_isRunning) return;

    preProcess(rawMat);

    emit AIInputReady(m_matForAI, m_params);
    emit sendFrame(m_matForDraw);
}
