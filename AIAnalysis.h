#ifndef AIANALYSIS_H
#define AIANALYSIS_H

#include <QObject>
#include <atomic>
#include <onnxruntime_cxx_api.h>
#include "CommonDef.h"

static Ort::Env g_ortEnv(ORT_LOGGING_LEVEL_WARNING, "AIModel");     //ONNX环境配置

class AIAnalysis : public QObject
{
    Q_OBJECT
public:
    explicit AIAnalysis(QObject *parent = nullptr);
    ~AIAnalysis();

public slots:
    void initModel();
    void onAIInputReady(const cv::Mat &matForAI, const PreprocessParams &params);
    void setRunning(bool running) { m_isRunning = running; }

signals:
    void modelReady();
    void AIOutputReady(const std::vector<float> &output, const PreprocessParams &params);

private:
    bool infer(const cv::Mat &mat, const PreprocessParams &params);

    std::unique_ptr<Ort::Session> m_session;
    std::string m_inputName;
    std::string m_outputName;

    // 预分配输入数据缓冲区 (1 x 3 x 640 x 640)，避免每帧重新分配
    std::vector<float> m_inputData;
    std::array<int64_t, 4> m_inputShape{1, 3, MODEL_WIDTH, MODEL_HEIGHT};

    std::atomic<bool> m_isRunning = false;
};

#endif // AIANALYSIS_H
