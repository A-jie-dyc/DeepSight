#include "FramePreprocessor.h"
#include <opencv2/opencv.hpp>

FramePreprocessor::FramePreprocessor(QObject *parent)
    : QObject{parent}
{}

AIDataInput FramePreprocessor::convertToAIInput(const QImage &img)
{
    AIDataInput input;
    input.data = img.bits();
    input.width = img.width();
    input.height = img.height();
    input.channels = 3;

    return input;
}

QImage FramePreprocessor::preprocess(const QImage &src)
{
    //包装成Mat
    cv::Mat mat(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());

    //BGR->RGB
    cv::cvtColor(mat,mat,cv::COLOR_BGR2RGB);

    //缩放尺寸
    cv::resize(mat,mat,cv::Size(640,640));

    QImage img(mat.data,mat.cols,mat.rows,mat.step,QImage::Format_RGB888);

    return img.copy();
}

void FramePreprocessor::onFrameReady(const QImage &img)
{
    if(img.isNull()) return;

    QImage processdImg = preprocess(img);

    AIDataInput input = convertToAIInput(processdImg);

    emit AIInputReady(input);
}
