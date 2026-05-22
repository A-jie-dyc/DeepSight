#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QMetaType>
#include <vector>
#include <opencv2/core.hpp>

//模型常量
constexpr int MODEL_WIDTH = 640;
constexpr int MODEL_HEIGHT = 640;
constexpr int MODEL_CHANNELS = 3;
constexpr float NORMALIZE_DIVISOR = 255.0f;
constexpr const wchar_t* MODEL_PATH = L"models/yolov8n.onnx";
//跟踪常量
constexpr int MAX_LOST_FRAMES = 10;
constexpr float TRACK_IOU_THRES = 0.4f;

struct AIDataInput
{
    int width = 0;
    int height = 0;
    int channels = MODEL_CHANNELS;
    std::vector<float> normData;
};


struct DetectionBox
{
    float x1 = 0.0;
    float y1 = 0.0;
    float x2 = 0.0;
    float y2 = 0.0;
    float score = 0.0;
    int classId = 0;
};

struct Track
{
    int trackId = -1;
    DetectionBox box;
    int lostFrameCount = 0;
    cv::Point2f lastCenterPos;
    cv::Point2f predictCenterPos;
};

Q_DECLARE_METATYPE(AIDataInput)
Q_DECLARE_METATYPE(DetectionBox)
Q_DECLARE_METATYPE(Track)
Q_DECLARE_METATYPE(std::vector<DetectionBox>)
Q_DECLARE_METATYPE(std::vector<Track>)

#endif // COMMONDEF_H
