#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <vector>
#include <QString>

constexpr int MODEL_WIDTH = 640;
constexpr int MODEL_HEIGHT = 640;
constexpr int MODEL_CHANNELS = 3;
constexpr float NORMALIZE_DIVISOR = 255.0f;
constexpr const wchar_t* MODEL_PATH = L"models/yolov8n.onnx";

struct AIDataInput
{
    int width = 0;
    int height = 0;
    int channels = MODEL_CHANNELS;
    std::vector<uint8_t> rgbData;
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
#endif // COMMONDEF_H
