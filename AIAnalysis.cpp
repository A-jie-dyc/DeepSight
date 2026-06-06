#include "AIAnalysis.h"
#include <QDebug>
#include <cstring>
#include <thread>

AIAnalysis::AIAnalysis(QObject *parent)
    : QObject{parent}
{}

void AIAnalysis::onAIInputReady(const cv::Mat &matForAI, const PreprocessParams &params)
{
    if(!m_isRunning)
        return;

    bool expected = false;
    if(!m_busy.compare_exchange_strong(expected, true)) {
        qDebug()<<"AI忙，丢一帧";
        return;
    }

    bool success = infer(matForAI, params);

    if(!success)
        qDebug()<<"AI异常分析失败";

    m_busy.store(false);
}

bool AIAnalysis::infer(const cv::Mat &mat, const PreprocessParams &params)
{
    if(!m_session || mat.empty() || mat.type() != CV_32F)
        return false;

    //直接拷贝
    std::memcpy(m_inputData.data(), mat.ptr<float>(0),
                m_inputData.size() * sizeof(float));

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        m_inputData.data(),
        m_inputData.size(),
        m_inputShape.data(),
        m_inputShape.size()
    );

    const char* inputNames[]  = {m_inputName.c_str()};
    const char* outputNames[] = {m_outputName.c_str()};

    try {
        Ort::RunOptions runOptions;
        std::vector<Ort::Value> outputTensors = m_session->Run(
            runOptions,
            inputNames,
            &inputTensor,
            1,
            outputNames,
            1
        );

        float *outputPtr = outputTensors[0].GetTensorMutableData<float>();
        size_t outputSize = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
        std::vector<float> output(outputPtr, outputPtr + outputSize);

        emit AIOutputReady(output, params);
        return true;
    } catch (...) {
        return false;
    }
}

void AIAnalysis::initModel()
{
    if(m_session)
        return;

    try {
        m_session.reset();
        Ort::SessionOptions session_options;
        //设置推理线程数
        unsigned int cpu_cores = std::thread::hardware_concurrency();
        int best_threads = 4;
        if(cpu_cores > 0) {
            best_threads = std::max(1, (int)(cpu_cores / 2));
            best_threads = std::min(best_threads, 8);
        }
        session_options.SetIntraOpNumThreads(best_threads);
        //设置主模型会话数
        session_options.SetInterOpNumThreads(1);
        //设置模型图优化
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        m_session = std::make_unique<Ort::Session>(g_ortEnv,MODEL_PATH,session_options);
        //创建临时内存分配器获取输入输出节点
        Ort::AllocatorWithDefaultOptions allocator;
        auto inputName = m_session->GetInputNameAllocated(0,allocator);
        auto outputName = m_session->GetOutputNameAllocated(0,allocator);
        m_inputName = inputName.get();
        m_outputName = outputName.get();

        // 预分配输入数据缓冲区 (1 x 3 x 640 x 640)
        m_inputData.resize(1 * 3 * MODEL_WIDTH * MODEL_HEIGHT);

        qDebug()<<"模型初始化完成";
        emit modelReady();
    } catch (const std::exception &e) {
        qDebug()<<"模型初始化失败："<<e.what();
    }
}

AIAnalysis::~AIAnalysis()
{
    m_session.reset();
}