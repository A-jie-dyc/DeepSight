#include "FramePreprocessor.h"
#include <opencv2/opencv.hpp>

FramePreprocessor::FramePreprocessor(QObject *parent)
    : QObject{parent}
{}

AIDataInput FramePreprocessor::convertToAIInput(const QImage &img)
{
    AIDataInput input;
    std::vector<uint8_t> rgbData;

    input.width = img.width();
    input.height = img.height();

    int totalBytes = img.width() * img.height() * 3;

    rgbData.resize(totalBytes);
    memcpy(rgbData.data(),img.bits(),totalBytes);

    //转换+归一化处理
    input.normData.resize(rgbData.size());

    for(int c = 0; c < input.channels; c++) {
        for(int h = 0; h < input.height; h++) {
            for(int w = 0; w < input.width; w++) {

                int nhwc_idx = h * input.width * input.channels + w * input.channels + c;
                int nchw_idx = c * input.height * input.width + h * input.width + w;

                input.normData[nchw_idx] = static_cast<float>(rgbData[nhwc_idx]) / NORMALIZE_DIVISOR;
            }
        }
    }

    return input;
}

QImage FramePreprocessor::preProcess(const QImage &src)
{
    //包装成Mat
    cv::Mat mat(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());

    //BGR->RGB
    cv::cvtColor(mat,mat,cv::COLOR_BGR2RGB);

    //缩放尺寸
    cv::resize(mat,mat,cv::Size(MODEL_WIDTH,MODEL_HEIGHT));

    QImage img(mat.data,mat.cols,mat.rows,mat.step,QImage::Format_RGB888);

    return img.copy();
}

void FramePreprocessor::onFrameReady(const QImage &img)
{
    if(img.isNull() || !m_isRunning) return;

    QImage processdImg = preProcess(img);

    emit SendFrame(processdImg);

    AIDataInput input = convertToAIInput(processdImg);

    emit AIInputReady(input);
}
