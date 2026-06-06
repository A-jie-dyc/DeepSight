#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QMetaType>
#include <vector>
#include <opencv2/core.hpp>

//模型常量
constexpr int MODEL_WIDTH = 640;                                        //宽
constexpr int MODEL_HEIGHT = 640;                                       //高
constexpr float NORMALIZE_DIVISOR = 255.0f;                             //归一化
constexpr const wchar_t* MODEL_PATH = L"models/yolov8n.onnx";           //模型文件路径
//后处理常量
constexpr float CONF_THRESHOLD = 0.25f;                                 //置信度阈值
constexpr float NMS_THRESHOLD = 0.5f;                                   //NMS阈值
constexpr int OUT_DIM = 84;                                             //输出维度
constexpr int BOX_COUNT = 8400;                                         //输出候选框总数
//跟踪常量
constexpr int MAX_LOST_FRAMES = 10;                                     //最大丢帧数
constexpr float TRACK_IOU_THRES = 0.4f;                                 //跟踪判定IOU

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

struct PreprocessParams
{
    float scale = 1.0f;
    int padLeft = 0;
    int padTop = 0;
};

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(DetectionBox)
Q_DECLARE_METATYPE(Track)
Q_DECLARE_METATYPE(PreprocessParams)
Q_DECLARE_METATYPE(std::vector<DetectionBox>)
Q_DECLARE_METATYPE(std::vector<Track>)

#endif // COMMONDEF_H
