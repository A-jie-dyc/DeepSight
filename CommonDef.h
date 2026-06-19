#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QMetaType>
#include <vector>
#include <cstdint>
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
constexpr int MAX_LOST_FRAMES = 30;                                     //最大丢帧数
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
    uint64_t frameId = 0;
    float scale = 1.0f;
    int padLeft = 0;
    int padTop = 0;
};

struct Line
{
    cv::Point2f start;
    cv::Point2f end;
    bool isValid = false;
    int direction = 1;
};

namespace ErrorDef {
//给命名空间加元对象支持
Q_NAMESPACE
enum ErrorType {
    Error_Camera = 1,           //摄像头打开失败
    Error_VideoOpen = 2,        //视频文件打开失败
    Error_ModelLoad = 3,        //AI模型加载失败
    Error_LineInvalid = 4,      //统计线绘制无效
    Error_General               //通用错误
};
//注册枚举到元对象
Q_ENUM_NS(ErrorType);

enum ErrorLevel {
    ErrorLevel_Info = 0,
    ErrorLevel_Warning = 1,
    ErrorLevel_Critical = 2
};
Q_ENUM_NS(ErrorLevel);
}


struct ErrorInfo
{
    Q_GADGET
    Q_PROPERTY(int type READ type CONSTANT)
    Q_PROPERTY(int level READ level CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QString suggestion READ suggestion CONSTANT)

public:
    ErrorDef::ErrorType errorType = ErrorDef::Error_General;
    ErrorDef::ErrorLevel errorLevel = ErrorDef::ErrorLevel_Critical;
    QString errorTitle;
    QString errorMessage;
    QString errorSuggestion;

    int type() const { return static_cast<int>(errorType); }
    int level() const { return static_cast<int>(errorLevel); }
    QString title() const { return errorTitle; }
    QString message() const { return errorMessage; }
    QString suggestion() const { return errorSuggestion; }
};

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(DetectionBox)
Q_DECLARE_METATYPE(Track)
Q_DECLARE_METATYPE(PreprocessParams)
Q_DECLARE_METATYPE(std::vector<DetectionBox>)
Q_DECLARE_METATYPE(std::vector<Track>)
Q_DECLARE_METATYPE(Line)
Q_DECLARE_METATYPE(ErrorDef::ErrorType)
Q_DECLARE_METATYPE(ErrorDef::ErrorLevel)
Q_DECLARE_METATYPE(ErrorInfo)

#endif // COMMONDEF_H
