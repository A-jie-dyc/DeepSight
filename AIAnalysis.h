#ifndef AIANALYSIS_H
#define AIANALYSIS_H

#include <QObject>
#include <onnxruntime_cxx_api.h>
#include <atomic>
#include "CommonDef.h"

class AIAnalysis : public QObject
{
    Q_OBJECT
public:
    explicit AIAnalysis(QObject *parent = nullptr);
    ~AIAnalysis();

    void setRunning(bool running) { m_isRunning.store(running); }
    //读取并清零推理帧计数
    FpsCount fetchAndResetAllCount();
    void resetAllCount();

public slots:
    void initModel();
    void onAIInputReady(uint64_t frameId, const cv::Mat &matForAI, const PreprocessParams &params);

signals:
    void modelReady();
    void AIOutputReady(uint64_t frameId, const std::vector<float> &output, const PreprocessParams &params);
    void errorOccurred(ErrorDef::ErrorType type, const QString &msg);

private:
    //返回有效数组或空值
    std::optional<std::vector<float>> infer(const cv::Mat &mat, const PreprocessParams &params);
    //ONNX环境配置
    Ort::Env &getOrtEnv();

    std::unique_ptr<Ort::Session> m_session;
    std::string m_inputName;
    std::string m_outputName;
    //预分配输入数据缓冲区 (1 x 3 x 640 x 640)，避免每帧重新分配
    std::vector<float> m_inputData;
    std::array<int64_t, 4> m_inputShape{1, 3, MODEL_WIDTH, MODEL_HEIGHT};

    std::atomic<bool> m_isRunning = false;
    std::atomic<bool> m_busy = false;
    std::atomic<uint64_t> m_inferFrameCount = 0;
    std::atomic<uint64_t> m_dropFrameCount = 0;
    std::atomic<uint64_t> m_totalFrameCount = 0;
    std::atomic<uint64_t> m_countSeq = 0;
    uint64_t m_lastInfer = 0;
    uint64_t m_lastTotal = 0;
    uint64_t m_lastDrop = 0;
};

#endif // AIANALYSIS_H
